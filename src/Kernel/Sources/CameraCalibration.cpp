// =============================================================================
//  CameraCalibration.cpp
//
//  MIT License
//
//  Copyright (c) 2007-2018 Dairoku Sekiguchi
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.
// =============================================================================
/*!
	\file		CameraCalibration.cpp
	\author		Dairoku Sekiguchi
	\version	1.0
	\date		2007/04/05
	\brief		This file is a part of CalibraKernel
	
	This is a direct C++ port of "Camera Calibration Toolbox for Matlab"
	by Jean-Yves Bouguet.
	Original source can be found at:
	http://www.vision.caltech.edu/bouguetj/calib_doc/
*/

#pragma warning(disable:4244)	// __w64 intからconst intへの変換warningを消すため
#pragma warning(disable:4267)	// size_tからconst unsigned intへの変換warningを消すため
#pragma warning(disable:4278)
#pragma warning(disable:4996)	// std::uninitialized_copyのwarningを消すため

// -----------------------------------------------------------------------------
// 	include files
// -----------------------------------------------------------------------------
#include <math.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/bindings/lapack/gesv.hpp>
#include <boost/numeric/bindings/lapack/gesvd.hpp>
#include <boost/numeric/bindings/traits/ublas_matrix.hpp>

namespace ublas = boost::numeric::ublas;
namespace lapack = boost::numeric::bindings::lapack;

#include "CameraCalibration.hpp"

extern "C" {
//#define LAPACK_DGETRI dgetri
void    LAPACK_DGETRI(int *n,double *a,int *lda,int *ipiv,double *work,int *lwork,int *info);
}

bool	g_debug_enabled = false;


//  CameraCalibration class public member functions ===========================
// -----------------------------------------------------------------------------
//	CameraCalibration
// -----------------------------------------------------------------------------
//
CameraCalibration::CameraCalibration(int inImageWidth, int inImageHeight)
{
	//	いくつかのパラメータを初期化
	alpha_c = 0.0;
	thresh_cond = 1e6;

	mImageWidth = inImageWidth;
	mImageHeight = inImageHeight;
}


// -----------------------------------------------------------------------------
//	~CameraCalibration
// -----------------------------------------------------------------------------
//
CameraCalibration::~CameraCalibration()
{
}


// -----------------------------------------------------------------------------
//	ClearMesurementData
// -----------------------------------------------------------------------------
//
void	CameraCalibration::ClearMesurementData()
{
	x_list.clear();
	X_list.clear();
	H_list.clear();
	omc_list.clear();
	Tc_list.clear();
	Rc_list.clear();
	y_list.clear();
	ex_list.clear();
}


// -----------------------------------------------------------------------------
//	AddMesurementData
// -----------------------------------------------------------------------------
//
void	CameraCalibration::AddMesurementData(	const ublas::matrix<double, ublas::column_major> &x,
												const ublas::matrix<double, ublas::column_major> &X)
{
	ublas::matrix<double, ublas::column_major>	X_dash(3, X.size2());
	ublas::matrix<double, ublas::column_major>	H(3, 3);

	x_list.push_back(x);
	X_list.push_back(X);

	for (int i = 0; i < (int )X.size2(); i++)
	{
		X_dash(0, i) = X(0, i);
		X_dash(1, i) = X(1, i);
		X_dash(2, i) = 1.0;		//　この行の値がゼロだとまずいため
	}

	computeHomography(x, X_dash, H);

	H_list.push_back(H);
}


// -----------------------------------------------------------------------------
//	DoCalibration
// -----------------------------------------------------------------------------
//
void	CameraCalibration::DoCalibration()
{
	//	すでに計算してあるホモグラフィより，各パラメータの初期値を求める
	computeIntrisicParam();
	computeExtrinsicParam();
	mainOptimization();
}


// -----------------------------------------------------------------------------
//	CancelCalibrationProcess
// -----------------------------------------------------------------------------
//
void	CameraCalibration::CancelCalibrationProcess()
{
}


//  CameraCalibration class protected member functions =========================
// -----------------------------------------------------------------------------
//	computeHomography
// -----------------------------------------------------------------------------
//
void	CameraCalibration::computeHomography(
										const ublas::matrix<double, ublas::column_major> &x,
										const ublas::matrix<double, ublas::column_major> &X,
										ublas::matrix<double, ublas::column_major> &H)
{
	int	i, j, k;
	int	Np = (int )x.size2();

	ublas::matrix<double, ublas::column_major>	m(3, x.size2());
	ublas::matrix<double, ublas::column_major>	M(3, X.size2());

	for (int i = 0; i < Np; i++)
	{
		m(0, i) = x(0, i);
		m(1, i) = x(1, i);
		m(2, i) = 1.0;
		if (x.size1() == 3)	//	x(2, i)はゼロ以外でなければならない
		{
			m(0, i) /= x(2, i);
			m(1, i) /= x(2, i);
		}

		M(0, i) = X(0, i);
		M(1, i) = X(1, i);
		M(2, i) = 1.0;
		if (X.size1() == 3)	//	X(2, i)はゼロ以外でなければならない
		{
			M(0, i) /= X(2, i);
			M(1, i) /= X(2, i);
		}
	}

	ublas::vector<double>	ax(Np);
	ublas::vector<double>	ay(Np);

	double	mxx = 0;
	double	myy = 0;
	for (i = 0; i < Np; i++)
	{
		ax(i) = m(0, i);
		ay(i) = m(1, i);
		mxx += ax(i);
		myy += ay(i);
	}
	mxx /= (double )Np;
	myy /= (double )Np;

	double	scxx = 0;
	double	scyy = 0;
	for (i = 0; i < Np; i++)
	{
		ax(i) = fabs(ax(i) - mxx);
		ay(i) = fabs(ay(i) - myy);
		scxx += ax(i);
		scyy += ay(i);
	}
	scxx /= (double )Np;
	scyy /= (double )Np;

	ublas::matrix<double, ublas::column_major>	Hnorm(3, 3);
	ublas::matrix<double, ublas::column_major>	inv_Hnorm(3, 3);

	Hnorm(0, 0) = 1.0 / scxx;
	Hnorm(0, 1) = 0;
	Hnorm(0, 2) = -mxx / scxx;

	Hnorm(1, 0) = 0;
	Hnorm(1, 1) = 1.0 / scyy;
	Hnorm(1, 2) = -myy / scyy;

	Hnorm(2, 0) = 0;
	Hnorm(2, 1) = 0;
	Hnorm(2, 2) = 1.0;

	inv_Hnorm(0, 0) = scxx;
	inv_Hnorm(0, 1) = 0;
	inv_Hnorm(0, 2) = mxx;

	inv_Hnorm(1, 0) = 0;
	inv_Hnorm(1, 1) = scyy;
	inv_Hnorm(1, 2) = myy;

	inv_Hnorm(2, 0) = 0;
	inv_Hnorm(2, 1) = 0;
	inv_Hnorm(2, 2) = 1.0;

	ublas::matrix<double, ublas::column_major>	mn(3, Np);
	mn = ublas::prod(Hnorm, m);

	ublas::matrix<double, ublas::column_major>	L(2 * Np, 9);
	for (i = 0; i < Np; i++)
	{
		j = i * 2;
		L(j, 0) = M(0, i);
		L(j, 1) = M(1, i);
		L(j, 2) = M(2, i);
		L(j, 3) = 0.0;
		L(j, 4) = 0.0;
		L(j, 5) = 0.0;
		L(j, 6) = -1.0 * mn(0, i) * M(0, i);
		L(j, 7) = -1.0 * mn(0, i) * M(1, i);
		L(j, 8) = -1.0 * mn(0, i) * M(2, i);
	}

	for (i = 0; i < Np; i++)
	{
		j = i * 2 + 1;
		L(j, 0) = 0.0;
		L(j, 1) = 0.0;
		L(j, 2) = 0.0;
		L(j, 3) = M(0, i);
		L(j, 4) = M(1, i);
		L(j, 5) = M(2, i);
		L(j, 6) = -1.0 * mn(1, i) * M(0, i);
		L(j, 7) = -1.0 * mn(1, i) * M(1, i);
		L(j, 8) = -1.0 * mn(1, i) * M(2, i);
	}

	ublas::matrix<double, ublas::column_major>	U;
	ublas::matrix<double, ublas::column_major>	S;
	ublas::matrix<double, ublas::column_major>	V;

	if (Np > 4)
	{
		L = ublas::prod(ublas::trans(L), L);

		U = ublas::matrix<double, ublas::column_major>(9, 9);
		S = ublas::matrix<double, ublas::column_major>(9, 9);
		V = ublas::matrix<double, ublas::column_major>(9, 9);
	}
	else
	{
		U = ublas::matrix<double, ublas::column_major>(8, 8);
		S = ublas::matrix<double, ublas::column_major>(8, 9);
		V = ublas::matrix<double, ublas::column_major>(9, 9);
	}

	//	特異値分解
	mat_svd(L, U, S, V);

//std::cout << "L " << L << std::endl;
//std::cout << "U " << U << std::endl;
//std::cout << "S " << S << std::endl;
//std::cout << "V " << V << std::endl;

	ublas::vector<double>	hh(9);
	for (i = 0; i < 9; i++)
		hh(i) = V(i, 8);	// VではなくUを使うようである（←間違いだった？）

	for (i = 0; i < 9; i++)
		hh(i) = hh(i) / hh(8);

	ublas::matrix<double, ublas::column_major>	Hrem(3, 3);
	for (i = 0; i < 3; i++)
		for (j = 0; j < 3; j++)
			Hrem(i, j) = hh(i * 3 + j);

	H = ublas::prod(inv_Hnorm, Hrem);

	if (Np < 5)
		return;				// 最適化を行わない

	ublas::vector<double>	hhv(8);
	for (i = 0; i < 8; i++)
		hhv(i) = H((i / 3), (i % 3));

//std::cout << "Initial H:" << H << std::endl;

	//	非線形最小二乗法（Levenberg-Marquard法）Minpackを使ったほうがよいかも
	ublas::matrix<double, ublas::column_major>	mrep(3, Np);
	ublas::matrix<double, ublas::column_major>	J(2 * Np, 8);
	ublas::matrix<double, ublas::column_major>	MMM(3, Np);
	ublas::matrix<double, ublas::column_major>	MMM2(3, Np);
	ublas::matrix<double, ublas::column_major>	MMM3(3, Np);
	ublas::matrix<double, ublas::column_major>	temp_mat(8, 8);
	ublas::vector<double>	m_err(Np * 2);
	ublas::vector<double>	hh_innov(8);
	ublas::vector<int>	temp_vec(8);

	for (i = 0; i < 10; i++)
	{
		mrep = ublas::prod(H, M);

		for (j = 0; j < Np; j++)
		{
			MMM(0, j) = M(0, j) / mrep(2, j);
			MMM(1, j) = M(1, j) / mrep(2, j);
			MMM(2, j) = M(2, j) / mrep(2, j);

			mrep(0, j) = mrep(0, j) / mrep(2, j);
			mrep(1, j) = mrep(1, j) / mrep(2, j);
			mrep(2, j) = mrep(2, j) / mrep(2, j);

			m_err(j * 2) = m(0, j) - mrep(0, j);
			m_err(j * 2 + 1) = m(1, j) - mrep(1, j);

			MMM2(0, j) = MMM(0, j) * mrep(0, j);
			MMM2(1, j) = MMM(1, j) * mrep(0, j);
			MMM2(2, j) = MMM(2, j) * mrep(0, j);

			MMM3(0, j) = MMM(0, j) * mrep(1, j);
			MMM3(1, j) = MMM(1, j) * mrep(1, j);
			MMM3(2, j) = MMM(2, j) * mrep(1, j);
		}

		for (j = 0; j < Np; j++)
		{
			k = j * 2;

			J(k, 0) = -1.0 * MMM(0, j);
			J(k, 1) = -1.0 * MMM(1, j);
			J(k, 2) = -1.0 * MMM(2, j);

			J(k, 3) = 0.0;
			J(k, 4) = 0.0;
			J(k, 5) = 0.0;

			J(k, 6) = MMM2(0, j);
			J(k, 7) = MMM2(1, j);
		}

		for (j = 0; j < Np; j++)
		{
			k = j * 2 + 1;

			J(k, 0) = 0.0;
			J(k, 1) = 0.0;
			J(k, 2) = 0.0;

			J(k, 3) = -1.0 * MMM(0, j);
			J(k, 4) = -1.0 * MMM(1, j);
			J(k, 5) = -1.0 * MMM(2, j);

			J(k, 6) = MMM3(0, j);
			J(k, 7) = MMM3(1, j);
		}

		for (j = 0; j < Np; j++)
		{
			MMM(0, j) = M(0, j) / mrep(2, j);
			MMM(1, j) = M(1, j) / mrep(2, j);
			MMM(2, j) = M(2, j) / mrep(2, j);
		}

		temp_mat = ublas::prod(ublas::trans(J), J);
		mat_inv(temp_mat);
		hh_innov = ublas::prod(ublas::prod(temp_mat, ublas::trans(J)), m_err);
		hhv = hhv - hh_innov;

		for (j = 0; j < 8; j++)
			H((j / 3), (j % 3)) = hhv(j);
	}

//std::cout << "H:" << H << std::endl;
}


// -----------------------------------------------------------------------------
//	computeIntrisicParam
// -----------------------------------------------------------------------------
// Xが2行の制限がある... かも
void	CameraCalibration::computeIntrisicParam()
{
	int		i, j;
	ublas::vector<double>	c_init(2), k_init(5);

	//	initialize at the center of the image
	c_init(0) = mImageWidth	/ 2 - 0.5;
	c_init(1) = mImageHeight / 2 - 0.5;

	//	initialize to zero (no distortion)
	k_init.clear();


	//	matrix that subtract the principal point
	ublas::matrix<double, ublas::column_major>	Sub_cc(3, 3);
	Sub_cc(0, 0) = 1.0;
	Sub_cc(0, 1) = 0.0;
	Sub_cc(0, 2) = -1.0 * c_init(0);

	Sub_cc(1, 0) = 0.0;
	Sub_cc(1, 1) = 1.0;
	Sub_cc(1, 2) = -1.0 * c_init(1);

	Sub_cc(2, 0) = 0.0;
	Sub_cc(2, 1) = 0.0;
	Sub_cc(2, 2) = 1.0;

	//	ローカル計算用のホモグラフィ行列
	ublas::matrix<double, ublas::column_major>	H(3, 3);

	ublas::vector<double>	V_hori_pix(3), V_vert_pix(3), V_diag1_pix(3), V_diag2_pix(3);
	ublas::matrix<double, ublas::column_major>	A(getImageNum() * 2, 2);
	ublas::matrix<double, ublas::column_major>	b(getImageNum() * 2, 1);

	std::vector<ublas::matrix<double, ublas::column_major> >::const_iterator	it;

	for (it = H_list.begin(), i = 0; it != H_list.end(); ++it)
	{
		H = *it;

		H = ublas::prod(Sub_cc, H);

		// Extract vanishing points (direct and diagonals)
		for (j = 0; j < 3; j++)
		{
			V_hori_pix(j) = H(j, 0);
			V_vert_pix(j) = H(j, 1);
			V_diag1_pix(j) = (H(j, 0) + H(j, 1)) / 2.0;
			V_diag2_pix(j) = (H(j, 0) - H(j, 1)) / 2.0;
		}

		double	V_hori_pix_norm = mat_norm(V_hori_pix);
		double	V_vert_pix_norm = mat_norm(V_vert_pix);
		double	V_diag1_pix_norm = mat_norm(V_diag1_pix);
		double	V_diag2_pix_norm = mat_norm(V_diag2_pix);

		for (j = 0; j < 3; j++)
		{
			V_hori_pix(j) = V_hori_pix(j) / V_hori_pix_norm;
			V_vert_pix(j) = V_vert_pix(j) / V_vert_pix_norm;
			V_diag1_pix(j) = V_diag1_pix(j) / V_diag1_pix_norm;
			V_diag2_pix(j) = V_diag2_pix(j) / V_diag2_pix_norm;
		}

		A(i, 0) = V_hori_pix(0) * V_vert_pix(0);
		A(i, 1) = V_hori_pix(1) * V_vert_pix(1);
		b(i, 0) = -1.0 * V_hori_pix(2) * V_vert_pix(2);
		i++;

		A(i, 0) = V_diag1_pix(0) * V_diag2_pix(0);
		A(i, 1) = V_diag1_pix(1) * V_diag2_pix(1);
		b(i, 0) = -1.0 * V_diag1_pix(2) * V_diag2_pix(2);
		i++;
	}

	//	use all the vanishing points to estimate focal length:
	//	Select the model for the focal. (solution to Gerd's problem)

	bool	isTowFocalsInit = false;
	ublas::matrix<double, ublas::column_major>	sum_A(getImageNum() * 2, 1);
	ublas::matrix<double, ublas::column_major>	temp_value(1, 1);
	ublas::matrix<double, ublas::column_major>	temp_value2(1, 1);

	//	sum(A')
	for (i = 0; i < (int )A.size1(); i++)
		sum_A(i, 0) = A(i, 0) + A(i, 1);
	// b'*(sum(A')')
	temp_value = ublas::prod(ublas::trans(b), sum_A);

	if (!isTowFocalsInit)
	{
		//	towFocalsInitのあたりは，Jean-YvesさんのD論を読まないとわからない
		if (temp_value(0, 0) < 0)
			isTowFocalsInit = true;
	}

	ublas::vector<double>	f_init(2);

	if (isTowFocalsInit)
	{
		//	Use a two focals estimate
		//f_init = sqrt(abs(1./(inv(A'*A)*A'*b))); % if using a two-focal model for initial guess
	}
	else
	{
		//	Use a single focal estimate:
		temp_value2 = ublas::prod(ublas::trans(b), b);

		f_init(0) = sqrt(temp_value(0, 0) / temp_value2(0, 0));
		f_init(1) = f_init(0);
	}

	bool	isEstAspectRation = true;
	if (!isEstAspectRation)
	{
		//	two focal modelのときのみ意味あり
		f_init(0) = (f_init(0) + f_init(1)) / 2.0;
		f_init(1) = f_init(1);
	}

	KK = ublas::matrix<double, ublas::column_major>(3, 3);

	KK(0, 0) = f_init(0);
	KK(0, 1) = 0.0;
	KK(0, 2) = c_init(0);

	KK(1, 0) = 0.0;
	KK(1, 1) = f_init(1);
	KK(1, 2) = c_init(1);

	KK(2, 0) = 0.0;
	KK(2, 1) = 0.0;
	KK(2, 2) = 1.0;

	cc = c_init;
	fc = f_init;
	kc = k_init;
	alpha_c = 0.0;

	printf("\n\nCalibration parameters after initialization:\n\n");
	printf("Focal Length:          fc = [ %3.5f   %3.5f ]\n", KK(0, 0), KK(1, 1));
	printf("Principal point:       cc = [ %3.5f   %3.5f ]\n", KK(0, 2), KK(1, 2));
	printf("Skew:             alpha_c = [ %3.5f ]   => angle of pixel = %3.5f degrees\n",
																KK(0, 1), 90 - atan(KK(0, 1)) * 180 / 3.141592);
	printf("Distortion:            kc = [ %3.5f   %3.5f   %3.5f   %3.5f   %5.5f ]\n",
		k_init(0), k_init(1), k_init(2), k_init(3), k_init(4));   
}


// -----------------------------------------------------------------------------
//	computeExtrinsicParam
// -----------------------------------------------------------------------------
//
void	CameraCalibration::computeExtrinsicParam()
{
	int	i;
	//N_points_views = ublas::matrix<double, ublas::column_major>(1, getImageNum());

	ublas::matrix<double, ublas::column_major>	omckk(3, 1);
	ublas::matrix<double, ublas::column_major>	Tckk(3, 1);
	ublas::matrix<double, ublas::column_major>	Rckk(3, 3);

	//	iをkkに変えるとよいかも（他との整合性）
	for (i = 0; i < getImageNum(); i++)
	{
std::cout << "Image=" << i << std::endl;

		//N_points_views(0, i) = x_list[i].size2();
		ublas::matrix<double, ublas::column_major>	JJ_kk(2 * x_list[i].size2(), 6);

		computeExtrinsicInit(x_list[i], X_list[i], omckk, Tckk, Rckk);	// Rckkは使いません．せっかく計算するけど

//std::cout << "compute_extrinsic_init omckk" << omckk << std::endl;
//std::cout << "compute_extrinsic_init Tckk" << Tckk << std::endl;

		computeExtrinsicRefine(x_list[i], X_list[i], omckk, Tckk, Rckk, JJ_kk);

		//if (check_cond)	ここで計算結果（逆行列？）の精度をチェックしたほうがよい模様
		/*	MATLAB コード
			if (cond(JJ_kk)> thresh_cond),
				active_images(kk) = 0;
				omckk = NaN*ones(3,1);
				Tckk = NaN*ones(3,1);
				fprintf(1,'\nWarning: View #%d ill-conditioned. This image is now set inactive.\n',kk)
				desactivated_images = [desactivated_images kk];
			end;*/

//std::cout << "computeExtrinsicRefine omckk" << omckk << std::endl;
//std::cout << "computeExtrinsicRefine Tckk" << Tckk << std::endl;

		omc_list.push_back(omckk);
		Tc_list.push_back(Tckk);
	}
}


// -----------------------------------------------------------------------------
//	computeExtrinsicInit
// -----------------------------------------------------------------------------
//
void	CameraCalibration::computeExtrinsicInit(
										const ublas::matrix<double, ublas::column_major> &x,
										const ublas::matrix<double, ublas::column_major> &X,
										ublas::matrix<double, ublas::column_major> &omckk,
										ublas::matrix<double, ublas::column_major> &Tckk,
										ublas::matrix<double, ublas::column_major> &Rckk)
{
	int		i, j;
	ublas::matrix<double, ublas::column_major>	xn(2, x.size2());

	normalize_pixel(fc, cc, kc, alpha_c, x, xn);

	int	Np = (int )xn.size2();

	//	Check for planarity of the structure:
	//		X_mean = mean(X_kk')'...
	ublas::matrix<double, ublas::column_major>	X_mean(3, 1);
	X_mean.clear();
	for (i = 0; i < Np; i++)
		for (j = 0; j < 3; j++)
			X_mean(j, 0) += X(j, i);
	for (i = 0; i < 3; i++)
		X_mean(i, 0) /= (double )Np;

	ublas::matrix<double, ublas::column_major>	Y(3, Np);
	for (i = 0; i < Np; i++)
		for (j = 0; j < 3; j++)
			Y(j, i) = X(j, i) - X_mean(j, 0);

	ublas::matrix<double, ublas::column_major>	YY(3, 3);
	YY = ublas::prod(Y, ublas::trans(Y));

	//	特異値分解
	//ublas::vector<double>	S(3);
	ublas::matrix<double, ublas::column_major>	U(3, 3);
	ublas::matrix<double, ublas::column_major>	S(3, 3);
	ublas::matrix<double, ublas::column_major>	V(3, 3);

	mat_svd(YY, U, S, V);

	//lapack::gesvd('A', 'A', YY, S, U, V);	// lapackの呼び出し
	//V = ublas::trans(V);	// matlabと同じにするため

	double	r = S(2, 2) / S(1, 1);

	if (r < 1e-3 || Np < 5)
	{
//std::cout << "Planar structure detected: r=" << r << std::endl;

		//	Transform the plane to bring it in the Z=0 plane:
		ublas::matrix<double, ublas::column_major>	R_transform(3, 3);
		ublas::matrix<double, ublas::column_major>	R_transform2(3, 3);
		ublas::matrix<double, ublas::column_major>	T_transform(3, 1);

		//R_transform = ublas::trans(U);	// VではなくUを使うようである
		R_transform = ublas::trans(V);

//std::cout << "R_transform:" << R_transform << std::endl;

		ublas::vector<double>	temp_vec(2);
		temp_vec(0) = R_transform(0, 2);
		temp_vec(1) = R_transform(1, 2);

//std::cout << "mat_norm(temp_vec):" << mat_norm(temp_vec) << std::endl;

		if (mat_norm(temp_vec) < 1.0e-6)
			R_transform = ublas::identity_matrix<double>(3);

//std::cout << "mat_det(R_transform):" << mat_det(R_transform) << std::endl;

		if (mat_det(R_transform) < 0)
			R_transform = -1.0 * R_transform;

//std::cout << "R_transform2:" << R_transform << std::endl;

		T_transform = ublas::prod(-1.0 *R_transform, X_mean);

//std::cout << "R_transform:" << R_transform << std::endl;
//std::cout << "T_transform:" << T_transform << std::endl;
//std::cout << "X_mean:" << X_mean << std::endl;

		ublas::matrix<double, ublas::column_major>	X_new(X.size1(), X.size2());
		X_new = ublas::prod(R_transform, X);
		for (i = 0; i < Np; i++)
		{
			X_new(0, i) += T_transform(0, 0);
			X_new(1, i) += T_transform(1, 0);
			//X_new(2, i) += T_transform(2, 0);
			X_new(2, i) = 1.0;	// computeHomographyを呼び出すため
		}

		//	Compute the planar homography:
		ublas::matrix<double, ublas::column_major>	H(3, 3);
		computeHomography(xn, X_new, H);

//std::cout << "xn:" << xn << std::endl;
//std::cout << "X_new:" << X_new << std::endl;
//std::cout << "MyH:" << H << std::endl;

		//	De-embed the motion parameters from the homography:
		double	sc;
		ublas::vector<double>	temp_vec2(3);

		temp_vec2(0) = H(0, 0);
		temp_vec2(1) = H(1, 0);
		temp_vec2(2) = H(2, 0);
		sc = mat_norm(temp_vec2);

		temp_vec2(0) = H(0, 1);
		temp_vec2(1) = H(1, 1);
		temp_vec2(2) = H(2, 1);
		sc += mat_norm(temp_vec2);

		sc /= 2.0;
		H = H / sc;

		ublas::vector<double>	u1(3);
		ublas::vector<double>	u2(3);
		ublas::vector<double>	u3(3);
		ublas::matrix<double, ublas::column_major>	RRR(3, 3);

		u1(0) = H(0, 0);
		u1(1) = H(1, 0);
		u1(2) = H(2, 0);
		u1 = u1 / mat_norm(u1);

		u2(0) = H(0, 1);
		u2(1) = H(1, 1);
		u2(2) = H(2, 1);
		u2 = u2 - ublas::inner_prod(u1, u2) * u1;
		u2 = u2 / mat_norm(u2);

		mat_cross(u1, u2, u3);

//std::cout << "u1:" << u1 << std::endl;
//std::cout << "u2:" << u2 << std::endl;
//std::cout << "u3:" << u3 << std::endl;

		for (i = 0; i < 3; i++)
		{
			RRR(i, 0) = u1(i);
			RRR(i, 1) = u2(i);
			RRR(i, 2) = u3(i);
		}

		ublas::matrix<double, ublas::column_major>	jacobian(9, 3);

		rodrigues(RRR, omckk, jacobian);

//std::cout << "RRR:" << RRR << std::endl;
//std::cout << "omckk:" << omckk << std::endl;
//std::cout << "jacobian:" << jacobian << std::endl;

		rodrigues(omckk, Rckk, jacobian);

//std::cout << "Rckk:" << Rckk << std::endl;
//std::cout << "jacobian:" << jacobian << std::endl;

		for (i = 0; i < 3; i++)
			Tckk(i, 0) = H(i, 2);

		Tckk = Tckk + ublas::prod(Rckk, T_transform);
		Rckk = ublas::prod(Rckk, R_transform);

//std::cout << "Tckk:" << Tckk << std::endl;
//std::cout << "Rckk:" << Rckk << std::endl;

		rodrigues(Rckk, omckk, jacobian);
		rodrigues(omckk, Rckk, jacobian);

//std::cout << "LAST Rckk:" << Rckk << std::endl;
//std::cout << "jacobian:" << jacobian << std::endl;

		return;
	}

	//	Computes an initial guess for extrinsic parameters (works for general 3d structure, not planar!!!):
	//	The DLT method is applied here!!

std::cout << "Non planar structure detected: r=" << r << std::endl;
std::cout << "DLT法は未実装です" << std::endl;

	//	DLT法は未実装です
}

//	EXTRINSIC_REFINE_ITER_MAXを引数で指定できるようにしないとだめかも
#define	EXTRINSIC_REFINE_CHANGE_MIN	1e-10
#define	EXTRINSIC_REFINE_ITER_MAX	20

// -----------------------------------------------------------------------------
//	computeExtrinsicRefine
// -----------------------------------------------------------------------------
//
void	CameraCalibration::computeExtrinsicRefine(
										const ublas::matrix<double, ublas::column_major> &x,
										const ublas::matrix<double, ublas::column_major> &X,
										ublas::matrix<double, ublas::column_major> &omckk,
										ublas::matrix<double, ublas::column_major> &Tckk,
										ublas::matrix<double, ublas::column_major> &Rckk,
										ublas::matrix<double, ublas::column_major> &JJ_kk)
{
	ublas::matrix<double, ublas::column_major>	param(6, 1);
	for (int i = 0; i < 3; i++)
	{
		param(i, 0) = omckk(i, 0);
		param(i + 3, 0) = Tckk(i, 0);
	}

	double	change = 1.0;
	int		iter = 0;

	int	n = X.size2();
	ublas::matrix<double, ublas::column_major>	xn(2, n);
	ublas::matrix<double, ublas::column_major>	dxdom(2 * n, 3);
	ublas::matrix<double, ublas::column_major>	dxdT(2 * n, 3);
	ublas::matrix<double, ublas::column_major>	dxdf(2 * n, 2);
	ublas::matrix<double, ublas::column_major>	dxdc(2 * n, 2);
	ublas::matrix<double, ublas::column_major>	dxdk(2 * n, 5);
	ublas::matrix<double, ublas::column_major>	dxdalpha(2 * n, 1);

	ublas::matrix<double, ublas::column_major>	ex(2, n);
	ublas::matrix<double, ublas::column_major>	ex_vec(2 * n, 1);
	ublas::matrix<double, ublas::column_major>	JJ(2 * n, 6);
	ublas::matrix<double, ublas::column_major>	JJ2(6, 6);
	ublas::matrix<double, ublas::column_major>	param_innov(6, 1);
	ublas::matrix<double, ublas::column_major>	param_up(6, 1);


	while (	change > EXTRINSIC_REFINE_CHANGE_MIN &&
			iter < EXTRINSIC_REFINE_ITER_MAX)
	{
		// ToDo: mIsEstimateAspectRatio = falseのときの処理を考えたほうがよいかも
		project_points2(X, omckk, Tckk, fc, cc, kc, alpha_c, xn, dxdom, dxdT, dxdf, dxdc, dxdk, dxdalpha);

/*if (g_debug_enabled)
{
std::cout << "X:" << X << std::endl;
std::cout << "omckk:" << omckk << std::endl;
std::cout << "Tckk:" << Tckk << std::endl;
std::cout << "fc:" << fc << std::endl;
std::cout << "cc:" << cc << std::endl;
std::cout << "kc:" << kc << std::endl;
std::cout << "alpha_c:" << alpha_c << std::endl;

std::cout << "xn:" << xn << std::endl;
std::cout << "dxdom:" << dxdom << std::endl;
std::cout << "dxdT:" << dxdT << std::endl;
std::cout << "dxdf:" << dxdf << std::endl;
std::cout << "dxdc:" << dxdc << std::endl;
std::cout << "dxdk:" << dxdk << std::endl;
std::cout << "dxdalpha:" << dxdalpha << std::endl;
}*/
		ex = x - xn;

		for (int i = 0; i < (2 * n); i++)
			for (int j = 0; j < 3; j++)
			{
				JJ(i, j) = dxdom(i, j);
				JJ(i, j + 3) = dxdT(i, j);
			}
					
		if (mat_cond(JJ) > thresh_cond)
			break;

		JJ2 = ublas::prod(ublas::trans(JJ), JJ);

		mat_inv(JJ2);
		for (int i = 0; i < n; i++)
		{
			ex_vec(i * 2, 0)		= ex(0, i);
			ex_vec(i * 2 + 1, 0)	= ex(1, i);
		}
		param_innov = ublas::prod(
						ublas::matrix<double, ublas::column_major>(ublas::prod(JJ2, ublas::trans(JJ))),
						ex_vec);	
		param_up = param + param_innov;
		change = mat_norm(param_innov) / mat_norm(param_up);
		param = param_up;

		for (int i = 0; i < 3; i++)
		{
			omckk(i, 0) = param(i, 0);
			Tckk(i, 0) = param(i + 3, 0);
		}

		iter++;
	}

	ublas::matrix<double, ublas::column_major>	jacobian(9, 3);

	rodrigues(omckk, Rckk, jacobian);
}

#define	MAIN_OPTIMIZATION_CHANGE_MIN	1e-9
#define	MAIN_OPTIMIZATION_ITER_MAX		30

// -----------------------------------------------------------------------------
//	mainOptimization
// -----------------------------------------------------------------------------
//
void	CameraCalibration::mainOptimization()
{
	int	i, j;
	int	n_ima = getImageNum();

	ublas::vector<double>	init_param(15 + n_ima * 6);
	init_param.clear();
	init_param(0) = fc(0);
	init_param(1) = fc(1);
	init_param(2) = cc(0);
	init_param(3) = cc(1);
	init_param(4) = alpha_c;
	for (i = 0; i < 5; i++)
		init_param(5 + i) = kc(i);
	for (i = 0; i < getImageNum(); i++)
		for (j = 0; j < 3; j++)
		{
			init_param(15 + i * 6 + j) = omc_list[i](j, 0);
			init_param(15 + i * 6 + 3 + j) = Tc_list[i](j, 0);
		}

	//	Main Optimization
	ublas::vector<double>	param(15 + n_ima * 6);
	param = init_param;

//std::cout << "param:" << param << std::endl;

	double	change = 1.0;
	int		iter = 0;

	ublas::vector<double>	f(2);
	ublas::vector<double>	c(2);
	ublas::vector<double>	k(5);
	double					alpha;

	ublas::matrix<double, ublas::column_major>	omckk(3, 1);
	ublas::matrix<double, ublas::column_major>	Tckk(3, 1);

	//	MATLABではsparseで確保しているが，それほど巨大な行列
	//	でもないので普通に確保してみる
	ublas::matrix<double, ublas::column_major>	JJ3(15 + 6 * n_ima, 15 + 6 * n_ima);
	ublas::matrix<double, ublas::column_major>	ex3(15 + 6 * n_ima, 1);

	//	MATLABではループの中にあったもの
	ublas::vector<double>	selected_variables(15 + 6 * n_ima);


	while (	change > EXTRINSIC_REFINE_CHANGE_MIN &&
			iter < EXTRINSIC_REFINE_ITER_MAX)
	{

		f(0) = param(0);
		f(1) = param(1);
		c(0) = param(2);
		c(1) = param(3);
		alpha = param(4);
		for (i = 0; i < 5; i++)
			k(i) = param(5 + i);

		JJ3.clear();
		ex3.clear();

		//	must check active image first!
		for (int kk = 0; kk < n_ima; kk++)
		{
			for (i = 0; i < 3; i++)
			{
				omckk(i, 0) = param(15 + kk * 6 + i);
				Tckk(i, 0) = param(15 + kk * 6 + 3 + i);
			}

			int	Np = X_list[kk].size2();

			ublas::matrix<double, ublas::column_major>	x(2, Np);
			ublas::matrix<double, ublas::column_major>	exkk(2, Np);
			ublas::matrix<double, ublas::column_major>	dxdom(2 * Np, 3);
			ublas::matrix<double, ublas::column_major>	dxdT(2 * Np, 3);
			ublas::matrix<double, ublas::column_major>	dxdf(2 * Np, 2);
			ublas::matrix<double, ublas::column_major>	dxdc(2 * Np, 2);
			ublas::matrix<double, ublas::column_major>	dxdk(2 * Np, 5);
			ublas::matrix<double, ublas::column_major>	dxdalpha(2 * Np, 1);

			// ToDo: mIsEstimateAspectRatio = falseのときの処理を考えないとダメ
			project_points2(X_list[kk], omckk, Tckk, f, c, k, alpha, x, dxdom, dxdT, dxdf, dxdc, dxdk, dxdalpha);
			//[x,dxdom,dxdT,dxdf,dxdc,dxdk,dxdalpha] = project_points2(X_kk,omckk,Tckk,f(1),c,k,alpha);

			exkk = x_list[kk] - x;

			ublas::matrix<double, ublas::column_major>	A(10, 2 * Np);
			ublas::matrix<double, ublas::column_major>	B(6, 2 * Np);
			for (i = 0; i < 2 * Np; i++)
			{
				A(0, i) = dxdf(i, 0);
				A(1, i) = dxdf(i, 1);

				A(2, i) = dxdc(i, 0);
				A(3, i) = dxdc(i, 1);

				A(4, i) = dxdalpha(i, 0);

				for (j = 0; j < 5; j++)
					A(5 + j, i) = dxdk(i, j);

				for (j = 0; j < 3; j++)
					B(j, i) = dxdom(i, j);

				for (j = 0; j < 3; j++)
					B(3 + j, i) = dxdT(i, j);
			}

//std::cout << "A" << A << std::endl;
//std::cout << "B" << B << std::endl;

			//	_DEF_MAT_RANGE(JJ3_r1, JJ3, 0, 10, 0, 10)　みたいなマクロを作ったほうがよいかも
			ublas::matrix_range<ublas::matrix<double, ublas::column_major> >
				JJ3_r1(JJ3, ublas::range(0, 10), ublas::range(0, 10));
			JJ3_r1 = JJ3_r1 + ublas::prod(A, ublas::trans(A));

//std::cout << "JJ3_r1" << JJ3_r1 << std::endl;

			//	_GET_MAT_RANGE(JJ3, 0, 10, 0, 10)　みたいなマクロを作ったほうがよいかも
			ublas::matrix_range<ublas::matrix<double, ublas::column_major> >
				JJ3_r2(JJ3, ublas::range(15 + kk * 6, 15 + kk * 6 + 6),
							ublas::range(15 + kk * 6, 15 + kk * 6 + 6));
			JJ3_r2 = ublas::prod(B, ublas::trans(B));

			ublas::matrix<double, ublas::column_major>	AB(10, 6);
			AB = ublas::prod(A, ublas::trans(B));

			ublas::matrix_range<ublas::matrix<double, ublas::column_major> >
				JJ3_r3(JJ3, ublas::range(0, 10),
							ublas::range(15 + kk * 6, 15 + kk * 6 + 6));
			JJ3_r3 = AB;

			ublas::matrix_range<ublas::matrix<double, ublas::column_major> >
				JJ3_r4(JJ3, ublas::range(15 + kk * 6, 15 + kk * 6 + 6),
							ublas::range(0, 10));
			JJ3_r4 = ublas::trans(AB);

			ublas::matrix<double, ublas::column_major>	exkk_vec(2 * Np, 1);
			for (i = 0; i < Np; i++)
			{
				exkk_vec(i * 2, 0) = exkk(0, i);
				exkk_vec(i * 2 + 1, 0) = exkk(1, i);
			}

			ublas::matrix_range<ublas::matrix<double, ublas::column_major> >
				ex3_r1(ex3, ublas::range(0, 10),
							ublas::range(0, 1));
			ex3_r1 = ex3_r1 + ublas::prod(A, exkk_vec);

			ublas::matrix_range<ublas::matrix<double, ublas::column_major> >
				ex3_r2(ex3, ublas::range(15 + kk * 6, 15 + kk * 6 + 6),
							ublas::range(0, 1));
			ex3_r2 = ublas::prod(B, exkk_vec);

			//	Check if this view is ill-conditioned:
			//if check_cond,
			//	JJ_kk = B'; %[dxdom dxdT];
			//	if (cond(JJ_kk)> thresh_cond),
			//		active_images(kk) = 0;
			//		fprintf(1,'\nWarning: View #%d ill-conditioned. This image is now set inactive. (note: to disactivate this option, set check_cond=0)\n',kk)
			//		desactivated_images = [desactivated_images kk];
			//		param(15+6*(kk-1) + 1:15+6*(kk-1) + 6) = NaN*ones(6,1); 
			//	end;
			//end;
		}

//std::cout << "JJ3" << JJ3 << std::endl;
//std::cout << "ex3" << ex3 << std::endl;

		//	The following vector helps to select the variables to update (for only active images):
		//ublas::vector<double>	selected_variables(15 + 6 * n_ima);	ループの外に出した(2007/04/21)
		selected_variables.clear();
		selected_variables(0) = 1.0;	// est_fc
		selected_variables(1) = 1.0;	// est_fc
		selected_variables(2) = 1.0;	// center optimizaion x
		selected_variables(3) = 1.0;	// center optimizaion y
		selected_variables(4) = 0.0;	// est_alpha
		//selected_variables(4) = 1.0;	// est_alpha
		selected_variables(5) = 1.0;	// est_dist
		selected_variables(6) = 1.0;	// est_dist
		selected_variables(7) = 1.0;	// est_dist
		selected_variables(8) = 1.0;	// est_dist
		selected_variables(9) = 0.0;	// est_dist
		for (i = 0; i < 5; i++)
			selected_variables(10 + i) = 0.0;
		for (i = 0; i < 6 * n_ima; i++)
			selected_variables(15 + i) = 1.0;	//	activeイメージだけ1にしたほうがよい

		int	temp_num = 0;
		for (i = 0; i < (int )selected_variables.size(); i++)
			if (selected_variables(i) != 0.0)
				temp_num++;

		//	このあたりは冗長なのでまんとかする
		ublas::matrix<double, ublas::column_major>	JJ3_dash(temp_num, temp_num);
		int	i_dash, j_dash;
		for(i = 0, i_dash = 0; i < (int )JJ3.size1(); i++)
		{
			if (selected_variables(i) != 0.0)
			{
				for (j = 0, j_dash = 0; j < (int )JJ3.size2(); j++)
				{
					if (selected_variables(j) != 0.0)
					{
						JJ3_dash(i_dash, j_dash) = JJ3(i, j);
						j_dash++;
					}
				}
				i_dash++;
			}
		}
		ublas::matrix<double, ublas::column_major>	ex3_dash(temp_num, 1);
		for(i = 0, i_dash = 0; i < (int )ex3.size1(); i++)
		{
			if (selected_variables(i) != 0.0)
			{
				ex3_dash(i_dash, 0) = ex3(i, 0);
				i_dash++;
			}
		}

		ublas::matrix<double, ublas::column_major>	JJ2_inv(temp_num, temp_num);
		JJ2_inv = JJ3_dash;
		mat_inv(JJ2_inv); 

//std::cout << "JJ2_inv" << JJ2_inv << std::endl;

		// Smoothing coefficient:
		double	alpha_smooth	= 0.4;	// set alpha_smooth = 1; for steepest gradient descent
		double	alpha_smooth2	= 1.0 - pow((1.0 - alpha_smooth), iter + 1.0);	//	set to 1 to undo any smoothing!

		ublas::matrix<double, ublas::column_major>	param_innov(temp_num, 1);	
		param_innov = alpha_smooth2 * ublas::prod(JJ2_inv, ex3_dash);

		//ublas::matrix<double, ublas::column_major>	param_up(temp_num, 1);	
		for(i = 0, i_dash = 0; i < (int )param.size(); i++)
		{
			if (selected_variables(i) != 0.0)
			{
				param(i) = param(i) + param_innov(i_dash, 0);
				i_dash++;
			}
		}

		//	New intrinsic parameters
		ublas::vector<double>	fc_current(2);
		ublas::vector<double>	cc_current(2);
		ublas::vector<double>	kc_current(5);
		double					alpha_current;

		fc_current(0) = param(0);
		fc_current(1) = param(1);

		bool	center_optim = true;
		if (center_optim && (
			param(2) < 0 || param(2) > mImageWidth ||
			param(3) < 0 ||	param(3) > mImageHeight))
		{
std::cerr << "Warning: it appears that the principal point cannot be estimated. Setting center_optim = 0" << std::endl;
			center_optim = false;	// <- this dosen't take effect something 
			cc_current = c;
		}
		else
		{
			cc_current(0) = param(2);
			cc_current(1) = param(3);
		}

		alpha_current = param(4);
		for (i = 0; i < 5; i++)
			kc_current(i) = param(5 + i);

		bool	est_aspect_ratio = true;
		//if (est_aspect_ratio == false && est_fc(0) == 1 && est_fc(1) == 1)
		if (false)
		{
			fc_current(1) = fc_current(0);
			param(1) = param(0);
		}

		//	Change on the intrinsic parameters
		ublas::vector<double>	temp_vec(4);
		temp_vec(0) = fc_current(0);
		temp_vec(1) = fc_current(1);
		temp_vec(2) = cc_current(0);
		temp_vec(3) = cc_current(1);

		ublas::vector<double>	temp_vec2(4);
		temp_vec2(0) = fc_current(0) - f(0);
		temp_vec2(1) = fc_current(1) - f(1);
		temp_vec2(2) = cc_current(0) - c(0);
		temp_vec2(3) = cc_current(1) - c(1);

		change = mat_norm(temp_vec2) / mat_norm(temp_vec);

std::cout << "iter:" << iter << std::endl;
//std::cout << "mat_norm(temp_vec2)" << mat_norm(temp_vec2) << std::endl;
//std::cout << "mat_norm(temp_vec)" << mat_norm(temp_vec) << std::endl;
std::cout << "change" << change << std::endl;

		//	Second step: (optional) - It makes convergence faster, and the region of convergence LARGER!!!
		//	Recompute the extrinsic parameters only using compute_extrinsic.m (this may be useful sometimes)
		//	The complete gradient descent method is useful to precisely update the intrinsic parameters.
		bool	recompute_extrinsic = true;
		if (recompute_extrinsic)
		{
			double	MaxIter2 = 20;
			ublas::matrix<double, ublas::column_major>	omc_current(3, 1);
			ublas::matrix<double, ublas::column_major>	Tc_current(3, 1);
			ublas::matrix<double, ublas::column_major>	Rckk(3, 3);

			for (int kk = 0; kk < n_ima; kk++)
			{
				ublas::matrix<double, ublas::column_major>	JJ_kk(2 * x_list[kk].size2(), 6);

				//ToDo: このあたりをよく考え直す
				fc = fc_current;
				cc = cc_current;
				kc = kc_current;
				alpha_c = alpha_current;

				/*for (i = 0; i < 3; i++)	// MATLABのコードにはあるけど冗長
				{
					omc_current(i, 0) = param(15 + kk * 6 + i);
					Tc_current(i, 0) = param(15 + kk * 6 + i + 3);
				}*/
				computeExtrinsicInit(x_list[kk], X_list[kk], omc_current, Tc_current, Rckk);	// Rckkは使いません．せっかく計算するけど
//std::cout << "omc_current" << omc_current << std::endl;
//std::cout << "Tc_current" << Tc_current << std::endl;
				g_debug_enabled = true;
				computeExtrinsicRefine(x_list[kk], X_list[kk], omc_current, Tc_current, Rckk, JJ_kk);	// MaxIter2を引数で指定できるように...
//std::cout << "omc_current 2" << omc_current << std::endl;
//std::cout << "Tc_current 2" << Tc_current << std::endl;
				//if check_cond,
				//	if (cond(JJ_kk)> thresh_cond),
				//		active_images(kk) = 0;
				//		fprintf(1,'\nWarning: View #%d ill-conditioned. This image is now set inactive. (note: to disactivate this option, set check_cond=0)\n',kk);
				//		desactivated_images = [desactivated_images kk];
				//		omckk = NaN*ones(3,1);
				//		Tckk = NaN*ones(3,1);
				//	end;
				//end;

				for (i = 0; i < 3; i++)
				{
					param(15 + kk * 6 + i) = omc_current(i, 0);
					param(15 + kk * 6 + i + 3) = Tc_current(i, 0);
				}
			}
		}

		//param_list = [param_list param];は何らかの形でやっておいたほうが良いかも
		iter++;
	}

std::cout << "done" << std::endl;
std::cout << "Estimation of uncertainties..." << std::endl;

	//	冗長なコピー
	ublas::vector<double>	solution(15 + n_ima * 6);
	solution = param;

	//	Extraction of the paramters for computing the right reprojection error:
	fc(0) = solution(0);
	fc(1) = solution(1);
	cc(0) = solution(2);
	cc(1) = solution(3);
	alpha_c = solution(4);

	for (i = 0; i < 5; i++)
		kc(i) = solution(5 + i);

	ublas::matrix<double, ublas::column_major>	Rckk(3, 3);
	ublas::matrix<double, ublas::column_major>	jacobian(9, 3);

	for (int kk = 0; kk < n_ima; kk++)
	{
		for (i = 0; i < 3; i++)
		{
			for (i = 0; i < 3; i++)
			{
				omc_list[kk](i, 0) = solution(15 + kk * 6 + i);
				Tc_list[kk](i, 0) = solution(15 + kk * 6 + 3 + i);
			}
//std::cout << "kk=" << kk << std::endl;
//std::cout << "omckk" << omckk << std::endl;
//std::cout << "Tckk" << Tckk << std::endl;
			rodrigues(omc_list[kk], Rckk, jacobian);

			//omc_list[kk] = omckk;
			//Tc_list[kk] = Tckk;
			Rc_list.push_back(Rckk);	// <- これいらないような気がする．なぜなら、いつでも計算できるので
		}
	}

	//	Recompute the error (in the vector ex):
	//comp_error_calibの内容をここで処理
	for (int kk = 0; kk < n_ima; kk++)
	{
		int	n = X_list[kk].size2();
		ublas::matrix<double, ublas::column_major>	y(2, n);
		ublas::matrix<double, ublas::column_major>	dxdom(2 * n, 3);
		ublas::matrix<double, ublas::column_major>	dxdT(2 * n, 3);
		ublas::matrix<double, ublas::column_major>	dxdf(2 * n, 2);
		ublas::matrix<double, ublas::column_major>	dxdc(2 * n, 2);
		ublas::matrix<double, ublas::column_major>	dxdk(2 * n, 5);
		ublas::matrix<double, ublas::column_major>	dxdalpha(2 * n, 1);
		ublas::matrix<double, ublas::column_major>	ex(2, n);

		project_points2(X_list[kk], omc_list[kk], Tc_list[kk], fc, cc, kc, alpha_c, y, dxdom, dxdT, dxdf, dxdc, dxdk, dxdalpha);
		ex = x_list[kk] - y;

//std::cout << "y" << y << std::endl;
//std::cout << "ex" << ex << std::endl;

		y_list.push_back(y);
		ex_list.push_back(ex);
	}

	//err_std = std(ex')の実装
	//sigma_x = std(ex(:))の実装
	//	mat_stdを作ったほうがよいかも
	int		n = 0;
	int		n2 = 0;
	ublas::vector<double>	avg(2);
	avg.clear();
	double	avg2 = 0.0;
	for (int kk = 0; kk < n_ima; kk++)
	{
		for (i = 0; i < (int )ex_list[kk].size2(); i++)
		{
			avg(0) += ex_list[kk](0, i);
			avg(1) += ex_list[kk](1, i);
			n++;
			avg2 += ex_list[kk](0, i);
			avg2 += ex_list[kk](1, i);
			n2 += 2;
		}
	}
	avg(0) /= (double )n;
	avg(1) /= (double )n;
	avg2 /= (double )n2;

	err_std.resize(2);
	err_std.clear();
	double	sigma_x = 0.0;
	for (int kk = 0; kk < n_ima; kk++)
	{
		for (i = 0; i < (int )ex_list[kk].size2(); i++)
		{
			err_std(0) += pow((ex_list[kk](0, i) - avg(0)), 2);
			err_std(1) += pow((ex_list[kk](1, i) - avg(1)), 2);
			sigma_x += pow((ex_list[kk](0, i) - avg2), 2);
			sigma_x += pow((ex_list[kk](1, i) - avg2), 2);
		}
	}
	err_std(0) = sqrt(err_std(0) / (double )(n - 1));	// matlabの標準偏差は分母がn-1
	err_std(1) = sqrt(err_std(1) / (double )(n - 1));	// matlabの標準偏差は分母がn-1
	sigma_x = sqrt(sigma_x / (double )(n2 - 1));		// matlabの標準偏差は分母がn-1

//std::cout << "err_std" << err_std << std::endl;
//std::cout << "sigma_x" << sigma_x << std::endl;

	//	MATLABではここでもう一度，JJ3を求めている
	//	しかし冗長なので，すでに求めているJJ3を再利用
	int	temp_num = 0;
	for (i = 0; i < (int )selected_variables.size(); i++)
		if (selected_variables(i) != 0.0)
			temp_num++;

	//	前と同じだが，このあたりは冗長なのでまんとかする
	ublas::matrix<double, ublas::column_major>	JJ3_dash(temp_num, temp_num);
	int	i_dash, j_dash;
	for(i = 0, i_dash = 0; i < (int )JJ3.size1(); i++)
	{
		if (selected_variables(i) != 0.0)
		{
			for (j = 0, j_dash = 0; j < (int )JJ3.size2(); j++)
			{
				if (selected_variables(j) != 0.0)
				{
					JJ3_dash(i_dash, j_dash) = JJ3(i, j);
					j_dash++;
				}
			}
			i_dash++;
		}
	}
	ublas::matrix<double, ublas::column_major>	JJ2_inv(temp_num, temp_num);
	JJ2_inv = JJ3_dash;
	mat_inv(JJ2_inv);

	//	param_error(ind_Jac) =  3*sqrt(full(diag(JJ2_inv)))*sigma_x;を実装
	ublas::vector<double>	param_error(15 + n_ima * 6);
	param_error.clear();
	for (i = 0, i_dash = 0; i < (int )param_error.size(); i++)
	{
		if (selected_variables(i) != 0.0)
		{
//std::cout << "JJ2_inv" << JJ2_inv(i_dash, i_dash) << std::endl;
			param_error(i) = 3.0 * sqrt(JJ2_inv(i_dash, i_dash)) * sigma_x;
			i_dash++;
		}
	}



	//	冗長なコピー
	ublas::vector<double>	solution_error(15 + n_ima * 6);
	solution_error = param_error;

	bool	est_aspect_ratio = true;
	//if (est_aspect_ratio == false && est_fc(0) == 1 && est_fc(1) == 1)
	if (false)
	{
		solution_error(1) = solution_error(0);
	}

	//	extract_parametersの内容をここに実装
	fc_error = fc;
	cc_error = cc;
	alpha_c_error = alpha_c;
	kc_error = kc;
	fc_error(0) = solution_error(0);
	fc_error(1) = solution_error(1);
	cc_error(0) = solution_error(2);
	cc_error(1) = solution_error(3);
	alpha_c_error = solution_error(4);
	for (i = 0; i < 5; i++)
		kc_error(i) = solution_error(i + 5);

	KK = ublas::matrix<double, ublas::column_major>(3, 3);

	KK(0, 0) = fc(0);
	KK(0, 1) = fc(0) * alpha_c;
	KK(0, 2) = cc(0);

	KK(1, 0) = 0.0;
	KK(1, 1) = fc(1);
	KK(1, 2) = cc(1);

	KK(2, 0) = 0.0;
	KK(2, 1) = 0.0;
	KK(2, 2) = 1.0;

	// omckk_error
	// Tckk_error
	// Hの再計算 <- これやったほうが良いと思われる

	DumpResults();
}


// -----------------------------------------------------------------------------
//	dumpResults
// -----------------------------------------------------------------------------
//
void	CameraCalibration::DumpResults()
{
	printf("\n\nCalibration results after optimization (with uncertainties):\n\n");
	printf("Focal Length:          fc = [ %3.5f   %3.5f ] error [ %3.5f   %3.5f ]\n", fc(0), fc(1), fc_error(0), fc_error(1));
	printf("Principal point:       cc = [ %3.5f   %3.5f ] error [ %3.5f   %3.5f ]\n", cc(0), cc(1), cc_error(0), cc_error(1));
	printf("Skew:             alpha_c = [ %3.5f ] error [ %3.5f ]   => angle of pixel = %3.5f  error [ %3.5f ] degrees\n",
																alpha_c, alpha_c_error,
																90 - atan(alpha_c) * 180 / 3.141592,
																atan(alpha_c_error) * 180 / 3.141592);
	printf("Distortion:            kc = [ %3.5f   %3.5f   %3.5f   %3.5f   %5.5f ] error [ %3.5f   %3.5f   %3.5f   %3.5f   %5.5f ]\n",
		kc(0), kc(1), kc(2), kc(3), kc(4),
		kc_error(0), kc_error(1), kc_error(2), kc_error(3), kc_error(4));
	printf("Pixel error:          err = [ %3.5f   %3.5f ]\n", err_std(0), err_std(1));
	printf("Note: The numerical errors are approximately three times the standard deviations (for reference).\n");
	//printf("      For accurate (and stable) error estimates, it is recommended to run Calibration once again.\n");

	//	Some recommendations to the user to reject some of the difficult unkowns... Still in debug mode.
	double	alpha_c_min = alpha_c - alpha_c_error/2;
	double	alpha_c_max = alpha_c + alpha_c_error/2;

	if (alpha_c_min < 0 && alpha_c_max > 0)
	{
std::cout << "Recommendation: The skew coefficient alpha_c is found to be equal to zero (within its uncertainty)." << std::endl;
std::cout << "                You may want to reject it from the optimization by setting est_alpha=0 and run Calibration" << std::endl;
	}

	ublas::vector<double>	kc_min(5);
	ublas::vector<double>	kc_max(5);

	kc_min = kc - kc_error / 2;
	kc_max = kc + kc_error / 2;

	ublas::vector<double>	prob_kc(5);
	for (int i = 0; i < (int )kc.size(); i++)
		if (kc_min(i) < 0 && kc_max(i) > 0)
			prob_kc(i) = 1;
		else
			prob_kc(i) = 0;

	if (prob_kc(2) == 0 || prob_kc(3) == 0)
	{
		prob_kc(2) = 0;
		prob_kc(3) = 0;
	}

	double	sum = 0;
	for (int i = 0; i < (int )kc.size(); i++)
		sum += prob_kc(i);

	if (sum != 0)
	{
std::cout << "Recommendation: Some distortion coefficients are found equal to zero (within their uncertainties)." << std::endl;
std::cout << "                To reject them from the optimization set est_dist=[...] and run Calibration" << std::endl;
	//	本当はest_dist & ~prob_kcの結果もプリントしたほうがよい
	}
}


// -----------------------------------------------------------------------------
//	project_points2
// -----------------------------------------------------------------------------
//
void	CameraCalibration::project_points2(
										const ublas::matrix<double, ublas::column_major> &in_X,
										const ublas::matrix<double, ublas::column_major> &in_om,
										const ublas::matrix<double, ublas::column_major> &in_T,
										const ublas::vector<double> &in_fc,
										const ublas::vector<double> &in_cc,
										const ublas::vector<double> &in_kc,
										double	in_alpha_c,
										ublas::matrix<double, ublas::column_major> &out_xp,
										ublas::matrix<double, ublas::column_major> &out_dxpdom,
										ublas::matrix<double, ublas::column_major> &out_dxpdT,
										ublas::matrix<double, ublas::column_major> &out_dxpdf,
										ublas::matrix<double, ublas::column_major> &out_dxpdc,
										ublas::matrix<double, ublas::column_major> &out_dxpdk,
										ublas::matrix<double, ublas::column_major> &out_dxpdalpha)
{
	int	i, j;
	int	m = in_X.size1();
	int	n = in_X.size2();

	ublas::matrix<double, ublas::column_major>	Y(3, n);
	ublas::matrix<double, ublas::column_major>	dYdom(3 * n, 3);
	ublas::matrix<double, ublas::column_major>	dYdT(3 * n, 3);

	rigid_motion(in_X, in_om, in_T, Y, dYdom, dYdT);

/*if (g_debug_enabled) {
std::cout << "Y:" << Y << std::endl;
std::cout << "dYdom:" << dYdom << std::endl;
std::cout << "dYdT:" << dYdT << std::endl;
}*/
	ublas::matrix<double, ublas::column_major>	inv_Z(1, n);
	for (i = 0; i < n; i++)
		inv_Z(0, i) = 1.0 / Y(2, i);	// Y(2, i)がゼロでないことを確認しないとダメかも

	ublas::matrix<double, ublas::column_major>	x(2, n);
	for (i = 0; i < n; i++)
	{
		x(0, i) = Y(0, i) * inv_Z(0, i);
		x(1, i) = Y(1, i) * inv_Z(0, i);
	}

	ublas::matrix<double, ublas::column_major>	bb(n, 3);
	ublas::matrix<double, ublas::column_major>	cc(n, 3);	//	メンバー変数のccと重なっています．要変更 <-staticだからokなだけ
	for (i = 0; i < n; i++)
	{
		bb(i, 0) = -1.0 * x(0, i) * inv_Z(0, i);
		bb(i, 1) = bb(i, 0);
		bb(i, 2) = bb(i, 0);
		cc(i, 0) = -1.0 * x(1, i) * inv_Z(0, i);
		cc(i, 1) = cc(i, 0);
		cc(i, 2) = cc(i, 0);
	}

	ublas::matrix<double, ublas::column_major>	dxdom(2 * n, 3);
	for (i = 0; i < n; i++)
	{
		dxdom(i * 2, 0) = inv_Z(0, i) * dYdom(i * 3, 0) + bb(i, 0) * dYdom(i * 3 + 2, 0);
		dxdom(i * 2, 1) = inv_Z(0, i) * dYdom(i * 3, 1) + bb(i, 1) * dYdom(i * 3 + 2, 1);
		dxdom(i * 2, 2) = inv_Z(0, i) * dYdom(i * 3, 2) + bb(i, 2) * dYdom(i * 3 + 2, 2);

		dxdom(i * 2 + 1, 0) = inv_Z(0, i) * dYdom(i * 3 + 1, 0) + cc(i, 0) * dYdom(i * 3 + 2, 0);
		dxdom(i * 2 + 1, 1) = inv_Z(0, i) * dYdom(i * 3 + 1, 1) + cc(i, 1) * dYdom(i * 3 + 2, 1);
		dxdom(i * 2 + 1, 2) = inv_Z(0, i) * dYdom(i * 3 + 1, 2) + cc(i, 2) * dYdom(i * 3 + 2, 2);
	}

	ublas::matrix<double, ublas::column_major>	dxdT(2 * n, 3);
	for (i = 0; i < n; i++)
	{
		dxdT(i * 2, 0) = inv_Z(0, i) * dYdT(i * 3, 0) + bb(i, 0) * dYdT(i * 3 + 2, 0);
		dxdT(i * 2, 1) = inv_Z(0, i) * dYdT(i * 3, 1) + bb(i, 1) * dYdT(i * 3 + 2, 1);
		dxdT(i * 2, 2) = inv_Z(0, i) * dYdT(i * 3, 2) + bb(i, 2) * dYdT(i * 3 + 2, 2);

		dxdT(i * 2 + 1, 0) = inv_Z(0, i) * dYdT(i * 3 + 1, 0) + cc(i, 0) * dYdT(i * 3 + 2, 0);
		dxdT(i * 2 + 1, 1) = inv_Z(0, i) * dYdT(i * 3 + 1, 1) + cc(i, 1) * dYdT(i * 3 + 2, 1);
		dxdT(i * 2 + 1, 2) = inv_Z(0, i) * dYdT(i * 3 + 1, 2) + cc(i, 2) * dYdT(i * 3 + 2, 2);
	}

/*if (g_debug_enabled) {
//std::cout << "bb:" << bb << std::endl;
//std::cout << "cc:" << cc << std::endl;
std::cout << "dxdom:" << dxdom << std::endl;
std::cout << "dxdT:" << dxdT << std::endl;
}*/

	//	Add distortion
	ublas::matrix<double, ublas::column_major>	r2(1, n);
	for (i = 0; i < n; i++)
		r2(0, i) = x(0, i) * x(0, i) + x(1, i) * x(1, i);


	ublas::matrix<double, ublas::column_major>	dr2dom(n, 3);
	ublas::matrix<double, ublas::column_major>	dr2dT(n, 3);
	for (i = 0; i < n; i++)
	{
		dr2dom(i, 0) = 2.0 * x(0, i) * dxdom(i * 2, 0) + 2.0 * x(1, i) * dxdom(i * 2 + 1, 0);
		dr2dom(i, 1) = 2.0 * x(0, i) * dxdom(i * 2, 1) + 2.0 * x(1, i) * dxdom(i * 2 + 1, 1);
		dr2dom(i, 2) = 2.0 * x(0, i) * dxdom(i * 2, 2) + 2.0 * x(1, i) * dxdom(i * 2 + 1, 2);

		dr2dT(i, 0) = 2.0 * x(0, i) * dxdT(i * 2, 0) + 2.0 * x(1, i) * dxdT(i * 2 + 1, 0);
		dr2dT(i, 1) = 2.0 * x(0, i) * dxdT(i * 2, 1) + 2.0 * x(1, i) * dxdT(i * 2 + 1, 1);
		dr2dT(i, 2) = 2.0 * x(0, i) * dxdT(i * 2, 2) + 2.0 * x(1, i) * dxdT(i * 2 + 1, 2);
	}

	ublas::matrix<double, ublas::column_major>	r4(1, n);
	for (i = 0; i < n; i++)
		r4(0, i) = r2(0, i) * r2(0, i);

	ublas::matrix<double, ublas::column_major>	dr4dom(n, 3);
	ublas::matrix<double, ublas::column_major>	dr4dT(n, 3);
	for (i = 0; i < n; i++)
	{
		dr4dom(i, 0) = 2.0 * r2(0, i) * dr2dom(i, 0);
		dr4dom(i, 1) = 2.0 * r2(0, i) * dr2dom(i, 1);
		dr4dom(i, 2) = 2.0 * r2(0, i) * dr2dom(i, 2);

		dr4dT(i, 0) = 2.0 * r2(0, i) * dr2dT(i, 0);
		dr4dT(i, 1) = 2.0 * r2(0, i) * dr2dT(i, 1);
		dr4dT(i, 2) = 2.0 * r2(0, i) * dr2dT(i, 2);
	}

	ublas::matrix<double, ublas::column_major>	r6(1, n);
	for (i = 0; i < n; i++)
		r6(0, i) = r2(0, i) * r2(0, i) * r2(0, i);

	ublas::matrix<double, ublas::column_major>	dr6dom(n, 3);
	ublas::matrix<double, ublas::column_major>	dr6dT(n, 3);
	for (i = 0; i < n; i++)
	{
		double	r2_squre = r2(0, i) * r2(0, i);
		dr6dom(i, 0) = 3.0 * r2_squre * dr2dom(i, 0);
		dr6dom(i, 1) = 3.0 * r2_squre * dr2dom(i, 1);
		dr6dom(i, 2) = 3.0 * r2_squre * dr2dom(i, 2);

		dr6dT(i, 0) = 3.0 * r2_squre * dr2dT(i, 0);
		dr6dT(i, 1) = 3.0 * r2_squre * dr2dT(i, 1);
		dr6dT(i, 2) = 3.0 * r2_squre * dr2dT(i, 2);
	}


/*if (g_debug_enabled) {
std::cout << "dr4dom:" << dr4dom << std::endl;
std::cout << "dr4dT:" << dr4dT << std::endl;
std::cout << "dr6dom:" << dr6dom << std::endl;
std::cout << "dr6dT:" << dr6dT << std::endl;
}*/

	//	Radial distortion
	ublas::matrix<double, ublas::column_major>	cdist(1, n);
	for (i = 0; i < n; i++)
		cdist(0, i) = 1 + in_kc(0) * r2(0, i) + in_kc(1) * r4(0, i) + in_kc(4) * r6(0, i);

	ublas::matrix<double, ublas::column_major>	dcdistdom(n, 3);
	ublas::matrix<double, ublas::column_major>	dcdistdT(n, 3);
	ublas::matrix<double, ublas::column_major>	dcdistdk(n, 5);
	for (i = 0; i < n; i++)
	{
		dcdistdom(i, 0) = in_kc(0) * dr2dom(i, 0) + in_kc(1) * dr4dom(i, 0) + in_kc(4) * dr6dom(i, 0);
		dcdistdom(i, 1) = in_kc(0) * dr2dom(i, 1) + in_kc(1) * dr4dom(i, 1) + in_kc(4) * dr6dom(i, 1);
		dcdistdom(i, 2) = in_kc(0) * dr2dom(i, 2) + in_kc(1) * dr4dom(i, 2) + in_kc(4) * dr6dom(i, 2);

		dcdistdT(i, 0) = in_kc(0) * dr2dT(i, 0) + in_kc(1) * dr4dT(i, 0) + in_kc(4) * dr6dT(i, 0);
		dcdistdT(i, 1) = in_kc(0) * dr2dT(i, 1) + in_kc(1) * dr4dT(i, 1) + in_kc(4) * dr6dT(i, 1);
		dcdistdT(i, 2) = in_kc(0) * dr2dT(i, 2) + in_kc(1) * dr4dT(i, 2) + in_kc(4) * dr6dT(i, 2);

		dcdistdk(i, 0) = r2(0, i);
		dcdistdk(i, 1) = r4(0, i);
		dcdistdk(i, 2) = 0.0;
		dcdistdk(i, 3) = 0.0;
		dcdistdk(i, 4) = r6(0, i);
	}

	ublas::matrix<double, ublas::column_major>	xd1(2, n);
	for (i = 0; i < n; i++)
		for (j = 0; j < 2; j++)
			xd1(j, i) = x(j, i) * cdist(0, i);

	ublas::matrix<double, ublas::column_major>	dxd1dom(2 * n, 3);
	for (i = 0; i < n; i++)
		for (j = 0; j < 3; j++)
		{
			dxd1dom(i * 2, j) = x(0, i) * dcdistdom(i, j);
			dxd1dom(i * 2 + 1, j) = x(1, i) * dcdistdom(i, j);
		}

	ublas::matrix<double, ublas::column_major>	coeff(2 * n, 3);
	for (i = 0; i < n; i++)
		for (j = 0; j < 3; j++)
		{
			coeff(i * 2, j) = cdist(0, i);
			coeff(i * 2 + 1, j) = cdist(0, i);
		}

	for (i = 0; i < 2 * n; i++)
		for (j = 0; j < 3; j++)
			dxd1dom(i, j) = dxd1dom(i, j) + coeff(i, j) * dxdom(i, j);

	ublas::matrix<double, ublas::column_major>	dxd1dT(2 * n, 3);
	for (i = 0; i < n; i++)
		for (j = 0; j < 3; j++)
		{
			dxd1dT(i * 2, j) = x(0, i) * dcdistdT(i, j);
			dxd1dT(i * 2 + 1, j) = x(1, i) * dcdistdT(i, j);
		}

	for (i = 0; i < 2 * n; i++)
		for (j = 0; j < 3; j++)
			dxd1dT(i, j) = dxd1dT(i, j) + coeff(i, j) * dxdT(i, j);

	ublas::matrix<double, ublas::column_major>	dxd1dk(2 * n, 5);
	for (i = 0; i < n; i++)
		for (j = 0; j < 5; j++)
		{
			dxd1dk(i * 2, j) = x(0, i) * dcdistdk(i, j);
			dxd1dk(i * 2 + 1, j) = x(1, i) * dcdistdk(i, j);
		}

/*if (g_debug_enabled) {
std::cout << "dxd1dom:" << dxd1dom << std::endl;
std::cout << "dxd1dT:" << dxd1dT << std::endl;
std::cout << "dxd1dk:" << dxd1dk << std::endl;
}*/


	//	Tangential distortion
	ublas::matrix<double, ublas::column_major>	a1(1, n);
	ublas::matrix<double, ublas::column_major>	a2(1, n);
	ublas::matrix<double, ublas::column_major>	a3(1, n);
	for (i = 0; i < n; i++)
	{
		a1(0, i) = 2.0 * x(0, i) * x(1, i);
		a2(0, i) = r2(0, i) + 2.0 * x(0, i) * x(0, i);
		a3(0, i) = r2(0, i) + 2.0 * x(1, i) * x(1, i);
	}

	ublas::matrix<double, ublas::column_major>	delta_x(2, n);
	for (i = 0; i < n; i++)
	{
		delta_x(0, i) = in_kc(2) * a1(0, i) + in_kc(3) * a2(0, i);
		delta_x(1, i) = in_kc(2) * a3(0, i) + in_kc(3) * a1(0, i);
	}

	ublas::matrix<double, ublas::column_major>	aa(n, 3);
	//ublas::matrix<double, ublas::column_major>	bb(n, 3);
	//ublas::matrix<double, ublas::column_major>	cc(n, 3);
	for (i = 0; i < n; i++)
		for (j = 0; j < 3; j++)
		{
			aa(i, j) = 2.0 * in_kc(2) * x(1, i) + 6.0 * in_kc(3) * x(0, i);
			bb(i, j) = 2.0 * in_kc(2) * x(0, i) + 2.0 * in_kc(3) * x(1, i);
			cc(i, j) = 6.0 * in_kc(2) * x(1, i) + 2.0 * in_kc(3) * x(0, i);
		}

	ublas::matrix<double, ublas::column_major>	ddelta_xdom(2 * n, 3);
	for (i = 0; i < n; i++)
		for (j = 0; j < 3; j++)
		{
			ddelta_xdom(i * 2, j) = aa(i, j) * dxdom(i * 2, j) + bb(i, j) * dxdom(i * 2 + 1, j);
			ddelta_xdom(i * 2 + 1, j) = bb(i, j) * dxdom(i * 2, j) + cc(i, j) * dxdom(i * 2 + 1, j);
		}

	ublas::matrix<double, ublas::column_major>	ddelta_xdT(2 * n, 3);
	for (i = 0; i < n; i++)
		for (j = 0; j < 3; j++)
		{
			ddelta_xdT(i * 2, j) = aa(i, j) * dxdT(i * 2, j) + bb(i, j) * dxdT(i * 2 + 1, j);
			ddelta_xdT(i * 2 + 1, j) = bb(i, j) * dxdT(i * 2, j) + cc(i, j) * dxdT(i * 2 + 1, j);
		}

	ublas::matrix<double, ublas::column_major>	ddelta_xdk(2 * n, 5);
	ddelta_xdk.clear();
	for (i = 0; i < n; i++)
	{
		ddelta_xdk(i * 2, 2) = a1(0, i);
		ddelta_xdk(i * 2, 3) = a2(0, i);
		ddelta_xdk(i * 2 + 1, 2) = a3(0, i);
		ddelta_xdk(i * 2 + 1, 3) = a1(0, i);
	}

	ublas::matrix<double, ublas::column_major>	xd2(2, n);
	xd2 = xd1 + delta_x;

	ublas::matrix<double, ublas::column_major>	dxd2dom(2 * n, 3);
	ublas::matrix<double, ublas::column_major>	dxd2dT(2 * n, 3);
	ublas::matrix<double, ublas::column_major>	dxd2dk(2 * n, 5);
	dxd2dom = dxd1dom + ddelta_xdom;
	dxd2dT = dxd1dT + ddelta_xdT;
	dxd2dk = dxd1dk + ddelta_xdk;

/*if (g_debug_enabled) {
std::cout << "dxd2dom:" << dxd2dom << std::endl;
std::cout << "dxd2dT:" << dxd2dT << std::endl;
std::cout << "dxd2dk:" << dxd2dk << std::endl;
}*/

	//	Add Skew
	ublas::matrix<double, ublas::column_major>	xd3(2, n);
	for (i = 0; i < n; i++)
	{
		xd3(0, i) = xd2(0, i) + in_alpha_c * xd2(1, i);
		xd3(1, i) = xd2(1, i);
	}

	//	Compute: dxd3dom, dxd3dT, dxd3dk, dxd3dalpha
	ublas::matrix<double, ublas::column_major>	dxd3dom(2 * n, 3);
	for (i = 0; i < n; i++)
		for (j = 0; j < 3; j++)
		{
			dxd3dom(i * 2, j) = dxd2dom(i * 2, j) + in_alpha_c * dxd2dom(i * 2 + 1, j);
			dxd3dom(i * 2 + 1, j) = dxd2dom(i * 2 + 1, j);
		}

	ublas::matrix<double, ublas::column_major>	dxd3dT(2 * n, 3);
	for (i = 0; i < n; i++)
		for (j = 0; j < 3; j++)
		{
			dxd3dT(i * 2, j) = dxd2dT(i * 2, j) + in_alpha_c * dxd2dT(i * 2 + 1, j);
			dxd3dT(i * 2 + 1, j) = dxd2dT(i * 2 + 1, j);
		}

	ublas::matrix<double, ublas::column_major>	dxd3dk(2 * n, 5);
	dxd3dk.clear();
	for (i = 0; i < n; i++)
		for (j = 0; j < 5; j++)
		{
			dxd3dk(i * 2, j) = dxd2dk(i * 2, j) + in_alpha_c * dxd2dk(i * 2 + 1, j);
			dxd3dk(i * 2 + 1, j) = dxd2dk(i * 2 + 1, j);
		}

	ublas::matrix<double, ublas::column_major>	dxd3dalpha(2 * n, 1);
	dxd3dalpha.clear();
	for (i = 0; i < n; i++)
		dxd3dalpha(i * 2, 0) = xd2(1, i);

/*if (g_debug_enabled) {
std::cout << "xd1:" << xd1 << std::endl;
std::cout << "xd2:" << xd2 << std::endl;
std::cout << "xd3:" << xd3 << std::endl;
std::cout << "dxd3dom:" << dxd3dom << std::endl;
std::cout << "dxd3dT:" << dxd3dT << std::endl;
std::cout << "dxd3dk:" << dxd3dk << std::endl;
std::cout << "dxd3dalpha:" << dxd3dalpha << std::endl;
}*/


	//	Pixel coordinates
	if (in_fc.size() > 1)	//	基本的にin_fcの長さは2以上とみなす
	{
		for (i = 0; i < 2; i++)
			for (j = 0; j < n; j++)
				out_xp(i, j) = xd3(i, j) * in_fc(i) + in_cc(i);

		ublas::matrix<double, ublas::column_major>	fc_temp(2 * n, 1);
		for (i = 0; i < n; i++)
		{
			fc_temp(i * 2, 0) = in_fc(0);
			fc_temp(i * 2 + 1, 0) = in_fc(1);
		}

		for (i = 0; i < 2 * n; i++)
			for (j = 0; j < 3; j++)
				out_dxpdom(i, j) = fc_temp(i, 0) * dxd3dom(i, j);

		for (i = 0; i < 2 * n; i++)
			for (j = 0; j < 3; j++)
				out_dxpdT(i, j) = fc_temp(i, 0) * dxd3dT(i, j);

		for (i = 0; i < 2 * n; i++)
			for (j = 0; j < 5; j++)
				out_dxpdk(i, j) = fc_temp(i, 0) * dxd3dk(i, j);

		for (i = 0; i < 2 * n; i++)
			for (j = 0; j < 1; j++)
				out_dxpdalpha(i, j) = fc_temp(i, 0) * dxd3dalpha(i, j);

		out_dxpdf.clear();
		for (i = 0; i < n; i++)
		{
			out_dxpdf(i * 2, 0) = xd3(0, i);
			out_dxpdf(i * 2 + 1, 1) = xd3(1, i);
		}
	}
	else
	{
std::cout << "Warning:!! NO TEST PATH (project_points2)" << std::endl;
		//	ここのコードは通常使わないだろう
		for (i = 0; i < 2; i++)
			for (j = 0; j < n; j++)
				out_xp(i, j) = in_fc(0) * xd3(i, j) + in_cc(i);

		out_dxpdom = in_fc(0) * dxd3dom;
		out_dxpdT = in_fc(0) * dxd3dT;
		out_dxpdk = in_fc(0) * dxd3dk;
		out_dxpdalpha = in_fc(0) * dxd3dalpha;

		out_dxpdf.clear();
		for (i = 0; i < n; i++)
		{
			out_dxpdf(i * 2, 0) = xd3(0, i);
			out_dxpdf(i * 2 + 1, 0) = xd3(1, i);
		}
	}

	out_dxpdc.clear();
	for (i = 0; i < n; i++)
	{
		out_dxpdc(i * 2, 0) = 1.0;
		out_dxpdc(i * 2 + 1, 1) = 1.0;
	}

/*if (g_debug_enabled) {
std::cout << "out_xp:" << out_xp << std::endl;
std::cout << "out_dxpdom:" << out_dxpdom << std::endl;
std::cout << "out_dxpdT:" << out_dxpdT << std::endl;
std::cout << "out_dxpdk:" << out_dxpdk << std::endl;
std::cout << "out_dxpdalpha:" << out_dxpdalpha << std::endl;
std::cout << "out_dxpdf:" << out_dxpdf << std::endl;
std::cout << "out_dxpdc:" << out_dxpdc << std::endl;
}*/

}


// -----------------------------------------------------------------------------
//	rigid_motion
// -----------------------------------------------------------------------------
//
void	CameraCalibration::rigid_motion(
										const ublas::matrix<double, ublas::column_major> &in_X,
										const ublas::matrix<double, ublas::column_major> &in_om,
										const ublas::matrix<double, ublas::column_major> &in_T,
										ublas::matrix<double, ublas::column_major> &out_Y,
										ublas::matrix<double, ublas::column_major> &out_dYdom,
										ublas::matrix<double, ublas::column_major> &out_dYdT)
{
	ublas::matrix<double, ublas::column_major>	R(3, 3);
	ublas::matrix<double, ublas::column_major>	dRdom(9, 3);

	rodrigues(in_om, R, dRdom);

	int	m = in_X.size1();
	int	n = in_X.size2();

	ublas::matrix<double, ublas::column_major>	temp_mat(in_T.size1() * 1, in_T.size2() * n);

	mat_repmat(in_T, temp_mat, 1, n);
	out_Y = ublas::prod(R, in_X) + temp_mat;

	ublas::matrix<double, ublas::column_major>	dYdR(3 * n, 9);
	dYdR.clear();
	out_dYdT.clear();

	for (int i = 0; i < n; i++)
	{
		dYdR(i * 3, 0) = in_X(0, i);
		dYdR(i * 3, 3) = in_X(1, i);
		dYdR(i * 3, 6) = in_X(2, i);

		dYdR(i * 3 + 1, 1) = in_X(0, i);
		dYdR(i * 3 + 1, 4) = in_X(1, i);
		dYdR(i * 3 + 1, 7) = in_X(2, i);

		dYdR(i * 3 + 2, 2) = in_X(0, i);
		dYdR(i * 3 + 2, 5) = in_X(1, i);
		dYdR(i * 3 + 2, 8) = in_X(2, i);

		out_dYdT(i * 3, 0) = 1.0;
		out_dYdT(i * 3 + 1, 1) = 1.0;
		out_dYdT(i * 3 + 2, 2) = 1.0;
	}

	out_dYdom = ublas::prod(dYdR, dRdom);
}


// -----------------------------------------------------------------------------
//	normalize_pixel
// -----------------------------------------------------------------------------
void	CameraCalibration::normalize_pixel(
										const ublas::vector<double> &in_fc,
										const ublas::vector<double> &in_cc,
										const ublas::vector<double> &in_kc,
										double	in_alpha_c,
										const ublas::matrix<double, ublas::column_major> &in_x,
										ublas::matrix<double, ublas::column_major> &out_x)
{
	//	First: Subtract principal point, and divide by the focal length:
	for (int i = 0; i < (int )in_x.size2(); i++)
	{
		out_x(0, i) = (in_x(0, i) - in_cc(0)) / in_fc(0);
		out_x(1, i) = (in_x(1, i) - in_cc(1)) / in_fc(1);
	}

	//	Second: undo skew
	for (int i = 0; i < (int )in_x.size2(); i++)
		out_x(0, i) = out_x(0, i) - in_alpha_c * out_x(1, i);

	if (mat_norm(in_kc) != 0.0)
	{
		//	Third: Compensate for lens distortion:
		ublas::matrix<double, ublas::column_major>	x_distort = out_x;
		comp_distortion_oulu(in_kc, x_distort, out_x);
//std::cout << "out_x" << out_x << std::endl;
	}
}


// -----------------------------------------------------------------------------
//	comp_distortion_oulu
// -----------------------------------------------------------------------------
//
void	CameraCalibration::comp_distortion_oulu(
										const ublas::vector<double> &in_kc,
										const ublas::matrix<double, ublas::column_major> &in_x,
										ublas::matrix<double, ublas::column_major> &out_x)
{
	double	k1 = in_kc(0);
    double	k2 = in_kc(1);
    double	k3 = in_kc(4);
    double	p1 = in_kc(2);
    double	p2 = in_kc(3);
	double	r_2, k_radial, delta_x0, delta_x1;

	out_x = in_x;		//	initial guess

	for (int i = 0; i < 20; i++)
		for (int j = 0; j < (int )in_x.size2(); j++)
		{
			r_2 = out_x(0, j) * out_x(0, j) + out_x(1, j) * out_x(1, j);
			k_radial = 1 + k1 * r_2 + k2 * r_2 * r_2 + k3 * r_2 * r_2 * r_2;
			delta_x0 = 2 * p1 * out_x(0, j) * out_x(1, j) + p2 * (r_2 + 2 * out_x(0, j) * out_x(0, j));
			delta_x1 = p1 * (r_2 + 2 * out_x(1, j) * out_x(1, j)) + 2 * p2 * out_x(0, j) * out_x(1, j);
			out_x(0, j) = (in_x(0, j) - delta_x0) / k_radial;
			out_x(1, j) = (in_x(1, j) - delta_x1) / k_radial;
		}
}

#define	MATLAB_EPS	2.2204e-016

// -----------------------------------------------------------------------------
//	rodrigues
// -----------------------------------------------------------------------------
//	入力はベクトルであっても必ず行列として渡す
void	CameraCalibration::rodrigues(
									 const ublas::matrix<double, ublas::column_major> &in_mat,
									 ublas::matrix<double, ublas::column_major> &out_mat,
									 ublas::matrix<double, ublas::column_major> &out_jacobian)
{
	int	m = in_mat.size1();
	int	n = in_mat.size2();
	double	bigeps = MATLAB_EPS * 10e20;
	double	theta;

	ublas::matrix<double, ublas::column_major>	R(3, 3);
	ublas::matrix<double, ublas::column_major>	dRdin(9, 3);

	if ((m == 1 && n == 3) || (m == 3 && n== 1))
	{
		theta = mat_norm(in_mat);
		if (theta < MATLAB_EPS)
		{
			R = ublas::identity_matrix<double>(3);

			dRdin(0, 0) = 0;	dRdin(0, 1) = 0;	dRdin(0, 2) = 0;
			dRdin(1, 0) = 0;	dRdin(1, 1) = 0;	dRdin(1, 2) = 1;
			dRdin(2, 0) = 0;	dRdin(2, 1) = -1;	dRdin(2, 2) = 0;
			dRdin(3, 0) = 0;	dRdin(3, 1) = 0;	dRdin(3, 2) = -1;
			dRdin(4, 0) = 0;	dRdin(4, 1) = 0;	dRdin(4, 2) = 0;
			dRdin(5, 0) = 1;	dRdin(5, 1) = 0;	dRdin(5, 2) = 0;
			dRdin(6, 0) = 0;	dRdin(6, 1) = 1;	dRdin(6, 2) = 0;
			dRdin(7, 0) = -1;	dRdin(7, 1) = 0;	dRdin(7, 2) = 0;
			dRdin(8, 0) = 0;	dRdin(8, 1) = 0;	dRdin(8, 2) = 0;

std::cout << "Warning:!! NO TEST PATH in rodrigues01" << std::endl;
		}
		else
		{
			ublas::matrix<double, ublas::column_major>	in_dash(3, 1);
			if (n == 3)
				in_dash = ublas::trans(in_mat);
			else
				in_dash = in_mat;

			ublas::matrix<double, ublas::column_major>	dm3din(4, 3);
			dm3din(0, 0) = 1;	dm3din(0, 1) = 0;	dm3din(0, 2) = 0;
			dm3din(1, 0) = 0;	dm3din(1, 1) = 1;	dm3din(1, 2) = 0;
			dm3din(2, 0) = 0;	dm3din(2, 1) = 0;	dm3din(2, 2) = 1;
			dm3din(3, 0) = in_dash(0, 0) / theta;
			dm3din(3, 1) = in_dash(1, 0) / theta;
			dm3din(3, 2) = in_dash(2, 0) / theta;

//std::cout << "Rodrigues:dm3din" << dm3din << std::endl;

			ublas::matrix<double, ublas::column_major>	omega(3, 1);	//	vectorで定義したほうがよいかも
			omega(0, 0) = in_dash(0, 0) / theta;
			omega(1, 0) = in_dash(1, 0) / theta;
			omega(2, 0) = in_dash(2, 0) / theta;

//std::cout << "Rodrigues:omega" << omega << std::endl;

			ublas::matrix<double, ublas::column_major>	dm2dm3(4, 4);
			dm2dm3.clear();
			double	t = 1.0 / theta;
			dm2dm3(0, 0) = t;	dm2dm3(0, 1) = 0;	dm2dm3(0, 2) = 0;
			dm2dm3(1, 0) = 0;	dm2dm3(1, 1) = t;	dm2dm3(1, 2) = 0;
			dm2dm3(2, 0) = 0;	dm2dm3(2, 1) = 0;	dm2dm3(2, 2) = t;
			dm2dm3(0, 3) = -1 * in_dash(0, 0) / (theta * theta);
			dm2dm3(1, 3) = -1 * in_dash(1, 0) / (theta * theta);
			dm2dm3(2, 3) = -1 * in_dash(2, 0) / (theta * theta);
			dm2dm3(3, 0) = 0;
			dm2dm3(3, 1) = 0;
			dm2dm3(3, 2) = 0;
			dm2dm3(3, 3) = 1;

//std::cout << "Rodrigues:dm2dm3" << dm2dm3 << std::endl;

			double	alpha = cos(theta);
			double	beta = sin(theta);
			double	gamma = 1 - cos(theta);
			ublas::matrix<double, ublas::column_major>	omegav(3, 3);
			omegav(0, 0) = 0;			omegav(0, 1) = -omega(2, 0);	omegav(0, 2) = omega(1, 0);
			omegav(1, 0) = omega(2, 0);	omegav(1, 1) = 0;				omegav(1, 2) = -omega(0, 0);
			omegav(2, 0) = -omega(1, 0);omegav(2, 1) = omega(0, 0);		omegav(2, 2) = 0;

//std::cout << "Rodrigues:omegav" << omegav << std::endl;

			ublas::matrix<double, ublas::column_major>	A(3, 3);
			A = ublas::prod(omega, ublas::trans(omega));

//std::cout << "Rodrigues:A" << A << std::endl;

			ublas::matrix<double, ublas::column_major>	dm1dm2(21, 4);
			dm1dm2.clear();
			dm1dm2(0, 3) = -sin(theta);
			dm1dm2(1, 3) = cos(theta);
			dm1dm2(2, 3) = sin(theta);

			dm1dm2(3, 0) = 0;	dm1dm2(3, 1) = 0;	dm1dm2(3, 2) = 0;
			dm1dm2(4, 0) = 0;	dm1dm2(4, 1) = 0;	dm1dm2(4, 2) = 1;
			dm1dm2(5, 0) = 0;	dm1dm2(5, 1) = -1;	dm1dm2(5, 2) = 0;
			dm1dm2(6, 0) = 0;	dm1dm2(6, 1) = 0;	dm1dm2(6, 2) = -1;
			dm1dm2(7, 0) = 0;	dm1dm2(7, 1) = 0;	dm1dm2(7, 2) = 0;
			dm1dm2(8, 0) = 1;	dm1dm2(8, 1) = 0;	dm1dm2(8, 2) = 0;
			dm1dm2(9, 0) = 0;	dm1dm2(9, 1) = 1;	dm1dm2(9, 2) = 0;
			dm1dm2(10, 0) = -1;	dm1dm2(10, 1) = 0;	dm1dm2(10, 2) = 0;
			dm1dm2(11, 0) = 0;	dm1dm2(11, 1) = 0;	dm1dm2(11, 2) = 0;

			double	w1 = omega(0, 0);
			double	w2 = omega(1, 0);
			double	w3 = omega(2, 0);

			dm1dm2(12, 0) = 2*w1;	dm1dm2(12, 1) = 0;		dm1dm2(12, 2) = 0;
			dm1dm2(13, 0) = w2;		dm1dm2(13, 1) = w1;		dm1dm2(13, 2) = 0;
			dm1dm2(14, 0) = w3;		dm1dm2(14, 1) = 0;		dm1dm2(14, 2) = w1;
			dm1dm2(15, 0) = w2;		dm1dm2(15, 1) = w1;		dm1dm2(15, 2) = 0;
			dm1dm2(16, 0) = 0;		dm1dm2(16, 1) = 2*w2;	dm1dm2(16, 2) = 0;
			dm1dm2(17, 0) = 0;		dm1dm2(17, 1) = w3;		dm1dm2(17, 2) = w2;
			dm1dm2(18, 0) = w3;		dm1dm2(18, 1) = 0;		dm1dm2(18, 2) = w1;
			dm1dm2(19, 0) = 0;		dm1dm2(19, 1) = w3;		dm1dm2(19, 2) = w2;
			dm1dm2(20, 0) = 0;		dm1dm2(20, 1) = 0;		dm1dm2(20, 2) = 2*w3;

//std::cout << "Rodrigues:dm1dm2" << dm1dm2 << std::endl;

			R = ublas::identity_matrix<double>(3);
			R = R * alpha + omegav * beta + A * gamma;

			ublas::matrix<double, ublas::column_major>	dRdm1(9, 21);
			dRdm1.clear();
			dRdm1(0, 0) = 1;	dRdm1(4, 0) = 1;	dRdm1(8, 0) = 1;
			for (int i = 0; i < 9; i++)
			{
				dRdm1(i, 1) = omegav(i % 3, i / 3);
				dRdm1(i, 3 + i) = beta;
				dRdm1(i, 2) = A(i % 3, i / 3);
				dRdm1(i, 12 + i) = gamma;
			}

//std::cout << "Rodrigues:dRdm1" << dRdm1 << std::endl;

			dRdin = ublas::prod(
				ublas::matrix<double, ublas::column_major>(ublas::prod(
					ublas::matrix<double, ublas::column_major>(ublas::prod(dRdm1, dm1dm2)),
						dm2dm3)), dm3din);
		}

		out_mat = R;
		out_jacobian = dRdin;

		return;
	}

	R = ublas::identity_matrix<double>(3);

    // it is prob. a rot matr.

	if (m != n || m != 3 ||
		mat_norm(ublas::prod(ublas::trans(in_mat), in_mat) - R) > bigeps ||
		abs(mat_det(in_mat) - 1) > bigeps)
	{
std::cerr << "Neither a rotation matrix nor a rotation vector were provided" << std::endl;
		return;
	}

	//R = in_mat;

	//	特異値分解
	//ublas::vector<double>	S(3);
	ublas::matrix<double, ublas::column_major>	U(3, 3);
	ublas::matrix<double, ublas::column_major>	S(3, 3);
	ublas::matrix<double, ublas::column_major>	V(3, 3);

//std::cout << "Rodrigues:in_mat" << in_mat << std::endl;
//std::cout << "Rodrigues:R" << R << std::endl;

/*R(0, 0) = 0.0740;	R(0, 1) = 0.9897;	R(0, 2) = 0.1228;
R(1, 0) = 0.6396;	R(1, 1) = 0.0474;	R(1, 2) = -0.7673;
R(2, 0) = -0.7652;	R(2, 1) = 0.1353;	R(2, 2) = -0.6295;*/

	mat_svd(in_mat, U, S, V);

	//lapack::gesvd('A', 'A', R, S, U, V);	// lapackの呼び出し（MATLABと同じ結果が出てこない）
	//V = ublas::trans(V);	// matlabと同じにするため

/*U(0, 0) = 0.8835;	U(0, 1) = -0.4625;	U(0, 2) = -0.0740;
U(1, 0) = 0.3138;	U(1, 1) = 0.7018;	U(1, 2) = -0.6396;
U(2, 0) = 0.3477;	U(2, 1) = 0.5419;	U(2, 2) = 0.7652;

V(0, 0) = 0;		V(0, 1) = 0;		V(0, 2) = -1.0000;
V(1, 0) = 0.9363;	V(1, 1) = -0.3511;	V(1, 2) = 0;
V(2, 0) = -0.3511;	V(2, 1) = -0.9363;	V(2, 2) = 0;*/

//std::cout << "Rodrigues:U" << U << std::endl;
//std::cout << "Rodrigues:S" << S << std::endl;
//std::cout << "Rodrigues:V" << V << std::endl;
//std::cout << "Rodrigues:R" << R << std::endl;

	R = ublas::prod(U, ublas::trans(V));


	double	tr = (mat_trace(R) - 1) / 2.0;

//std::cout << "Rodrigues:tr" << tr << std::endl;

	ublas::matrix<double, ublas::column_major>	dtrdR(1, 9);
	dtrdR(0, 0) = 1.0/2.0;	dtrdR(0, 1) = 0;		dtrdR(0, 2) = 0;
	dtrdR(0, 3) = 0;		dtrdR(0, 4) = 1.0/2.0;	dtrdR(0, 5) = 0;
	dtrdR(0, 6) = 0;		dtrdR(0, 7) = 0;		dtrdR(0, 8) = 1.0/2.0;
	if (abs(tr) > 1)
		theta = 0;
	else
		theta = acos(tr);

	if (sin(theta) >= 1.0e-4)
	{
		double	dthetadtr = -1 / sqrt(1 - tr * tr);
		
		ublas::matrix<double, ublas::column_major>	dthetadR(1, 9);
		dthetadR = dthetadtr * dtrdR;

//std::cout << "Rodrigues:dthetadtr" << dthetadtr << std::endl;
//std::cout << "Rodrigues:dtrdR" << dtrdR << std::endl;

		double	vth = 1 / (2 * sin(theta));
		double	dvthdtheta = -vth * cos(theta) / sin(theta);

		ublas::matrix<double, ublas::column_major>	dvar1dtheta(2, 1);
		dvar1dtheta(0, 0) = dvthdtheta;
		dvar1dtheta(1, 0) = 1;
		
		ublas::matrix<double, ublas::column_major>	dvar1dR(2, 9);
		dvar1dR = ublas::prod(dvar1dtheta, dthetadR);

//std::cout << "Rodrigues:dvar1dtheta" << dvar1dtheta << std::endl;
//std::cout << "Rodrigues:dthetadR" << dthetadR << std::endl;
//std::cout << "Rodrigues:dvar1dR" << dvar1dR << std::endl;

		ublas::matrix<double, ublas::column_major>	om1(3, 1);
		om1(0, 0) = R(2, 1) - R(1, 2);
		om1(1, 0) = R(0, 2) - R(2, 0);
		om1(2, 0) = R(1, 0) - R(0, 1);

//std::cout << "Rodrigues:om1" << om1 << std::endl;

		ublas::matrix<double, ublas::column_major>	dom1dR(3, 9);
		dom1dR(0, 0) = 0;	dom1dR(0, 1) = 0;	dom1dR(0, 2) = 0;
		dom1dR(0, 3) = 0;	dom1dR(0, 4) = 0;	dom1dR(0, 5) = 1;
		dom1dR(0, 6) = 0;	dom1dR(0, 7) =-1;	dom1dR(0, 8) = 0;
		dom1dR(1, 0) = 0;	dom1dR(1, 1) = 0;	dom1dR(1, 2) =-1;
		dom1dR(1, 3) = 0;	dom1dR(1, 4) = 0;	dom1dR(1, 5) = 0;
		dom1dR(1, 6) = 1;	dom1dR(1, 7) = 0;	dom1dR(1, 8) = 0;
		dom1dR(2, 0) = 0;	dom1dR(2, 1) = 1;	dom1dR(2, 2) = 0;
		dom1dR(2, 3) =-1;	dom1dR(2, 4) = 0;	dom1dR(2, 5) = 0;
		dom1dR(2, 6) = 0;	dom1dR(2, 7) = 0;	dom1dR(2, 8) = 0;

		ublas::matrix<double, ublas::column_major>	dvardR(5, 9);
		for (int i = 0; i < 3; i++)
			for (int j = 0; j < 9; j++)
				dvardR(i, j) = dom1dR(i, j);
		for (int i = 0; i < 2; i++)
			for (int j = 0; j < 9; j++)
				dvardR(i + 3, j) = dvar1dR(i, j);

		ublas::matrix<double, ublas::column_major>	om(3, 1);
		om = vth * om1;

//std::cout << "Rodrigues:om" << om << std::endl;

		ublas::matrix<double, ublas::column_major>	domdvar(3, 5);
		domdvar.clear();
		for (int i = 0; i < 3; i++)
			domdvar(i, i) = vth;
		domdvar(0, 3) = om1(0, 0);
		domdvar(1, 3) = om1(1, 0);
		domdvar(2, 3) = om1(2, 0);

//std::cout << "Rodrigues:domdvar" << domdvar << std::endl;

		ublas::matrix<double, ublas::column_major>	dvar2dvar(4, 5);
		dvar2dvar.clear();
		for (int i = 0; i < 3; i++)
			for (int j = 0; j < 5; j++)
				dvar2dvar(i, j) = domdvar(i, j);
		dvar2dvar(3, 4) = 1;

//std::cout << "Rodrigues:dvar2dvar" << dvar2dvar << std::endl;

		out_mat = om * theta;

		ublas::matrix<double, ublas::column_major>	domegadvar2(3, 4);
		domegadvar2.clear();
		for (int i = 0; i < 3; i++)
		{
			domegadvar2(i, i) = theta;
			domegadvar2(i, 3) = om(i, 0);
		}

//std::cout << "Rodrigues:dvardR" << dvardR << std::endl;
//std::cout << "Rodrigues:domegadvar2" << domegadvar2 << std::endl;

	//out_jacobianの大きさをチェックすべきかも（9,3)であっているのだろうけど
	//もしくは，
	//out_jacobian = dRdin;をわざと入れる
		out_jacobian = ublas::prod(
						ublas::matrix<double, ublas::column_major>(ublas::prod(domegadvar2, dvar2dvar)),
						dvardR);

		return;
	}

std::cout << "Warning:!! NO TEST PATH in rodrigues02" << std::endl;

	if (tr > 0)
	{
		ublas::matrix<double, ublas::column_major>	out(3, 1);
		out.clear();
		out_mat = out;

		ublas::matrix<double, ublas::column_major>	dout(3, 9);
		dout(0, 0) = 0;		dout(0, 1) = 0;		dout(0, 2) = 0;
		dout(0, 3) = 0;		dout(0, 4) = 0;		dout(0, 5) = 1/2;
		dout(0, 6) = 0;		dout(0, 7) =-1/2;	dout(0, 8) = 0;
		dout(1, 0) = 0;		dout(1, 1) = 0;		dout(1, 2) =-1/2;
		dout(1, 3) = 0;		dout(1, 4) = 0;		dout(1, 5) = 0;
		dout(1, 6) = 1/2;	dout(1, 7) = 0;		dout(1, 8) = 0;
		dout(2, 0) = 0;		dout(2, 1) = 1/2;	dout(2, 2) = 0;
		dout(2, 3) =-1/2;	dout(2, 4) = 0;		dout(2, 5) = 0;
		dout(2, 6) = 0;		dout(2, 7) = 0;		dout(2, 8) = 0;

		out_jacobian = dout;

		return;
	}

	//	Solution by Mike Burl on Feb 27, 2007
	//	This is a better way to determine the signs of the
	//	entries of the rotation vector using a hash table on all
	//	the combinations of signs of a pairs of products (in the
	//	rotation matrix)

	//	Define hashvec and Smat
	ublas::matrix<double, ublas::column_major>	hashvec(11, 1);
	hashvec(0,0) = 0;
	hashvec(1,0) = -1;
	hashvec(2,0) = -3;
	hashvec(3,0) = -9;
	hashvec(4,0) = 9;
	hashvec(5,0) = 3;
	hashvec(6,0) = 1;
	hashvec(7,0) = 13;
	hashvec(8,0) = 5;
	hashvec(9,0) = -7;
	hashvec(10,0) = -11;

	ublas::matrix<double, ublas::column_major>	Smat(11, 3);
	Smat(0, 0) = 1;		Smat(0, 1) = 1;		Smat(0, 2) = 1;
	Smat(1, 0) = 1;		Smat(1, 1) = 0;		Smat(1, 2) = -1;
	Smat(2, 0) = 0;		Smat(2, 1) = 1;		Smat(2, 2) = -1;
	Smat(3, 0) = 1;		Smat(3, 1) = -1;	Smat(3, 2) = 0;
	Smat(4, 0) = 1;		Smat(4, 1) = 1;		Smat(4, 2) = 0;
	Smat(5, 0) = 0;		Smat(5, 1) = 1;		Smat(5, 2) = 1;
	Smat(6, 0) = 1;		Smat(6, 1) = 0;		Smat(6, 2) = 1;
	Smat(7, 0) = 1;		Smat(7, 1) = 1;		Smat(7, 2) = 1;
	Smat(8, 0) = 1;		Smat(8, 1) = 1;		Smat(8, 2) = -1;
	Smat(9, 0) = 1;		Smat(9, 1) = -1;	Smat(9, 2) = -1;
	Smat(10, 0) = 1;	Smat(10, 1) = -1;	Smat(10, 2) = 1;

	ublas::matrix<double, ublas::column_major>	M(3, 3);
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
		{
			if (i == j)
				M(i, j) = (R(i, j) + 1.0) / 2.0;
			else
				M(i, j) = R(i, j) / 2.0;
		}

	double	uabs = sqrt(M(0, 0));
	double	vabs = sqrt(M(1, 1));
	double	wabs = sqrt(M(2, 2));

	ublas::matrix<double, ublas::column_major>	mvec(1, 3);
	mvec(0, 0) = M(0, 1);
	mvec(0, 1) = M(1, 2);
	mvec(0, 2) = M(0, 2);

	//	robust sign() function
	ublas::matrix<double, ublas::column_major>	syn(1, 3);
	for (int i = 0; i < 3; i++)
		syn(0, i)  = ((mvec(0, i) > 1e-4) - (mvec(0, i) < -1e-4));

	ublas::matrix<double, ublas::column_major>	temp_mat(3, 1);
	mvec(0, 0) = 9;
	mvec(0, 1) = 3;
	mvec(0, 2) = 1;

	ublas::matrix<double, ublas::column_major>	hash(1, 1);
	hash = ublas::prod(syn ,temp_mat);

	ublas::matrix<double, ublas::column_major>	out(3, 1);
	out.clear();
	for (int i = 0; i < 11; i++)
		if (hash(0, 0) == hashvec(i, 0))
		{
			out(0, 0) = theta * uabs * Smat(i, 0);
			out(1, 0) = theta * vabs * Smat(i, 1);
			out(2, 0) = theta * wabs * Smat(i, 2);
		}
		//	見つからなかったときの処理を入れるべきかも

std::cerr << "WARNING!!!! Jacobian domdR undefined!!!" << std::endl;
	ublas::matrix<double, ublas::column_major>	dout(3, 9);
	dout.clear();

	out_mat = out;
	out_jacobian = dout;
}


// -----------------------------------------------------------------------------
//	rect_index
// -----------------------------------------------------------------------------
//
void	CameraCalibration::rect_index(
										int nc, int nr,	// xaxis, yaxis
										const ublas::matrix<double, ublas::column_major> &R,
										const ublas::vector<double> &f,
										const ublas::vector<double> &c,
										const ublas::vector<double> &k,
										double	alpha,
										const ublas::matrix<double, ublas::column_major> &KK_new,
										ublas::vector<double> &out_a1,
										ublas::vector<double> &out_a2,
										ublas::vector<double> &out_a3,
										ublas::vector<double> &out_a4,
										ublas::vector<int> &out_ind_new,
										ublas::vector<int> &out_ind_1,
										ublas::vector<int> &out_ind_2,
										ublas::vector<int> &out_ind_3,
										ublas::vector<int> &out_ind_4)
{
	//	Note: R is the motion of the points in space
	//	So: X2 = R*X where X: coord in the old reference frame, X2: coord in the new ref frame.

	// [mx,my] = meshgrid(1:nc, 1:nr);
	// px = reshape(mx',nc*nr,1);
	// py = reshape(my',nc*nr,1);
	// rays = inv(KK_new)*[(px - 1)';(py - 1)';ones(1,length(px))];
	ublas::vector<int>	px(nc * nr);
	ublas::vector<int>	py(nc * nr);

	for (int i = 0; i < nr; i++)
		for (int j = 0;j < nc; j++)
		{
			px(i * nc + j) = j;
			py(i * nc + j) = i;
		}

	ublas::matrix<double, ublas::column_major>	t(3, nr * nc);
	for (int i = 0; i < nr; i++)
		for (int j = 0; j < nc; j++)
		{
			t(0, i * nc + j) = j;
			t(1, i * nc + j) = i;
			t(2, i * nc + j) = 1;
		}

	ublas::matrix<double, ublas::column_major> KK_new_inv = KK_new;
	mat_inv(KK_new_inv);
	ublas::matrix<double, ublas::column_major>	rays(3, nr * nc);
	rays = ublas::prod(KK_new_inv, t);

	// Rotation: (or affine transformation):
	ublas::matrix<double, ublas::column_major>	rays2(3, nr * nc);
	rays2 = ublas::prod(ublas::trans(R), rays);

	ublas::matrix<double, ublas::column_major>	x(2, nr * nc);
	for (int i = 0; i < nc * nr; i++)
	{
		x(0, i) = rays2(0, i) / rays2(2, i);
		x(1, i) = rays2(1, i) / rays2(2, i);
	}

	// Add distortion:
	ublas::matrix<double, ublas::column_major>	xd(2, nr * nc);

	apply_distortion(x, k, xd);

	// Reconvert in pixels:
	// Interpolate between the closest pixels:
	ublas::vector<double>	px2(nc * nr);
	ublas::vector<double>	py2(nc * nr);
	ublas::vector<int>	px_0(nc * nr);
	ublas::vector<int>	py_0(nc * nr);
	ublas::vector<int>	good_points(nc * nr);

	// ind_new = (px(good_points)-1)*nr + py(good_points); も同時に処理．
	// オリジナルでは後の方に出てくる
	//ublas::vector<int>	ind_new(nc * nr);
	out_ind_new.resize(nc * nr);

	int	count = 0;
	for (int i = 0; i < nc * nr; i++)
	{
		px2(count) = f(0) * (xd(0, i) + alpha * xd(1, i)) + c(0);
		py2(count) = f(1) * xd(1, i) + c(1);
		px_0(count) = floor(px2(count));
		py_0(count) = floor(py2(count));

		out_ind_new(count) = px(i) * nr + py(i);

		if (px_0(count) >= 0 && px_0(count) <= (nc - 2) &&
			py_0(count) >= 0 && py_0(count) <= (nr - 2))
			count++;
	}
std::cout << "count:" << count << std::endl;

	px2.resize(count);
	py2.resize(count);
	px_0.resize(count);
	py_0.resize(count);
	out_ind_new.resize(count);

	ublas::vector<double>	alpha_x(count);
	ublas::vector<double>	alpha_y(count);

	alpha_x = px2 - px_0;
	alpha_y = py2 - py_0;

	out_a1.resize(count);
	out_a2.resize(count);
	out_a3.resize(count);
	out_a4.resize(count);

	for (int i = 0; i < count; i++)
	{
		out_a1(i) = (1 - alpha_y(i)) * (1 - alpha_x(i));
		out_a2(i) = (1 - alpha_y(i)) * alpha_x(i);
		out_a3(i) = alpha_y(i) * (1 - alpha_x(i));
		out_a4(i) = alpha_y(i) * alpha_x(i);
	}

	out_ind_1.resize(count);
	out_ind_2.resize(count);
	out_ind_3.resize(count);
	out_ind_4.resize(count);

	//	オリジナルはMatlabの座標系に変換しているので注意
	//	画像を一次元配列でアクセスする（Fortran配列）
	for (int i = 0; i < count; i++)
	{
		out_ind_1(i) = px_0(i) * nr + py_0(i);
		out_ind_2(i) = (px_0(i) + 1) * nr + py_0(i);
		out_ind_3(i) = px_0(i) * nr + (py_0(i) + 1);
		out_ind_4(i) = (px_0(i) + 1) * nr + (py_0(i) + 1);
	}

	// 元のコードには以下の部分があったがこれは無駄なのでここでは処理しない
	// Irec(ind_new) = a1 .* I(ind_1) + a2 .* I(ind_2) + a3 .* I(ind_3) + a4 .* I(ind_4);
}


// -----------------------------------------------------------------------------
//	rectify_image
// -----------------------------------------------------------------------------
//
void	CameraCalibration::rectify_image(
										int inWidth, int inHeight,
										const unsigned char *inImage,
										const ublas::vector<double> &in_a1,
										const ublas::vector<double> &in_a2,
										const ublas::vector<double> &in_a3,
										const ublas::vector<double> &in_a4,
										const ublas::vector<int> &in_ind_new,
										const ublas::vector<int> &in_ind_1,
										const ublas::vector<int> &in_ind_2,
										const ublas::vector<int> &in_ind_3,
										const ublas::vector<int> &in_ind_4,
										unsigned char *outImage)
{
	int	imageSize = inWidth * inHeight;
	int indexCount = in_ind_1.size();

	for (int i = 0; i < imageSize; i++)
		outImage[i] = 0;

	double	value;
	int	x, y, ind_new, ind1, ind2, ind3, ind4;
	for (int i = 0; i < indexCount; i++)
	{
		y = in_ind_new[i] % inHeight;
		x = in_ind_new[i] / inHeight;
		ind_new = x + y * inWidth;

		y = in_ind_1[i] % inHeight;
		x = in_ind_1[i] / inHeight;
		ind1 = x + y * inWidth;

		y = in_ind_2[i] % inHeight;
		x = in_ind_2[i] / inHeight;
		ind2 = x + y * inWidth;

		y = in_ind_3[i] % inHeight;
		x = in_ind_3[i] / inHeight;
		ind3 = x + y * inWidth;

		y = in_ind_4[i] % inHeight;
		x = in_ind_4[i] / inHeight;
		ind4 = x + y * inWidth;

		value = in_a1[i] * inImage[ind1];
		value += in_a2[i] * inImage[ind2];
		value += in_a3[i] * inImage[ind3];
		value += in_a4[i] * inImage[ind4];
		outImage[ind_new] = value;
	}
}


// -----------------------------------------------------------------------------
//	apply_distortion
// -----------------------------------------------------------------------------
//
void	CameraCalibration::apply_distortion(
									 const ublas::matrix<double, ublas::column_major> &x,
									 const ublas::vector<double> &k,
									 ublas::matrix<double, ublas::column_major> &out_xd)
{
	int	m = x.size1();
	int n = x.size2();

	ublas::vector<double>	x1(n);
	ublas::vector<double>	x2(n);

	for (int i = 0; i < n; i++)
	{
		x1(i) = x(0, i);
		x2(i) = x(1, i);
	}

	// Add distortion:
	ublas::vector<double>	r2(n);
	ublas::vector<double>	r4(n);
	ublas::vector<double>	r6(n);

	for (int i = 0; i < n; i++)
	{
		r2(i) = x1(i) * x1(i) + x2(i) * x2(i);
		r4(i) = r2(i) * r2(i);
		r6(i) = r2(i) * r2(i) * r2(i);
	}

	// Radial distortion:
	ublas::vector<double>	cdist(n);
	for (int i = 0; i < n; i++)
		cdist(i) = 1 + k(0) * r2(i) + k(1) * r4(i) + k(4) * r6(i);

	ublas::matrix<double, ublas::column_major>	xd1(m, n);

	for (int i = 0; i < n; i++)
	{
		xd1(0, i) = x(0, i) * cdist(i);
		xd1(1, i) = x(1, i) * cdist(i);
	}

	// tangential distortion:
	ublas::vector<double>	a1(n);
	ublas::vector<double>	a2(n);
	ublas::vector<double>	a3(n);

	for (int i = 0; i < n; i++)
	{
		a1(i) = 2 * x1(i) * x2(i);
		a2(i) = r2(i) + 2 * x1(i) * x1(i);
		a3(i) = r2(i) + 2 * x2(i) * x2(i);
	}

	ublas::matrix<double, ublas::column_major>	delta_x(2, n);

	for (int i = 0; i < n; i++)
	{
		delta_x(0, i) = k(2) * a1(i) + k(3) * a2(i);
		delta_x(1, i) = k(2) * a3(i) + k(3) * a1(i);
	}

	out_xd.resize(m, n);
	for (int i = 0; i < n; i++)
	{
		out_xd(0, i) = xd1(0, i) + delta_x(0, i);
		out_xd(1, i) = xd1(1, i) + delta_x(1, i);
	}
}


//  CameraCalibration class private member functions ===========================
//
//	mat_で始まる関数は，matlab互換用のための関数
// -----------------------------------------------------------------------------
//	mat_inv
// -----------------------------------------------------------------------------
//
void	CameraCalibration::mat_inv(ublas::matrix<double, ublas::column_major> &io_mat)
{
	ublas::vector<int>	temp_vec(io_mat.size1());

	//	io_matの逆行列を求める（正則な正方行列なハズなのでlapackのgetrfとgetriを使う）
	lapack::getrf(io_mat, temp_vec);	

	//	なぜかlapack::getrfが未実装(todo)になっているので直接呼出し
	int	n = io_mat.size1();
	int	lda = n, lwork = n, info;
	double *work = new double[io_mat.size1()];

	LAPACK_DGETRI(&n,
		boost::numeric::bindings::traits::matrix_storage(io_mat),
		&lda,
		boost::numeric::bindings::traits::vector_storage(temp_vec),
		work, &lwork, &info);

	delete work;
}

// -----------------------------------------------------------------------------
//	mat_det
// -----------------------------------------------------------------------------
//
double	CameraCalibration::mat_det(const ublas::matrix<double, ublas::column_major> &in_mat)
{
	int	n = in_mat.size1();
	ublas::matrix<double, ublas::column_major> A(n, n);
	ublas::vector<int>	piv(n);

	A = in_mat;
	lapack::getrf(A, piv);	

	double	det = 1.0;
	for (int i = 0; i < n; i++)
		if (piv(i) != ((double )i + 1.0))
			det = -1.0 * det * A(i, i);
		else
			det = det * A(i, i);

	return det;
}

// -----------------------------------------------------------------------------
//	mat_trace
// -----------------------------------------------------------------------------
//
double	CameraCalibration::mat_trace(const ublas::matrix<double, ublas::column_major> &in_mat)
{
	double	trace = 0;
	int		n;

	if (in_mat.size1() < in_mat.size2())
		n = in_mat.size1();
	else
		n = in_mat.size2();

	for (int i = 0; i < n; i++)
		trace += in_mat(i, i);

	return trace;
}


// -----------------------------------------------------------------------------
//	mat_cond
// -----------------------------------------------------------------------------
//
double	CameraCalibration::mat_cond(const ublas::matrix<double, ublas::column_major> &in_mat)
{
	//	特異値分解
	ublas::vector<double>	S(in_mat.size2());
	ublas::matrix<double, ublas::column_major>	A = in_mat;
	ublas::matrix<double, ublas::column_major>	U(in_mat.size1(), in_mat.size1());
	ublas::matrix<double, ublas::column_major>	V(in_mat.size2(), in_mat.size2());

	lapack::gesvd('A', 'A', A, S, U, V);	// lapackの呼び出し

	double	min = S(0);
	double	max = S(0);


	for (int i = 0; i < (int )S.size(); i++)
	{
		if (S(i) < min)
			min = S(i);
		if (S(i) > max)
			max = S(i);
	}

	if (min == 0)
		return DBL_MAX;

	//	in_matの最大の特異値と最小の特異値の比を求める
	return max / min;
}


// -----------------------------------------------------------------------------
//	mat_repmat
// -----------------------------------------------------------------------------
//
void	CameraCalibration::mat_repmat(
								const ublas::matrix<double, ublas::column_major> &in_mat,
								ublas::matrix<double, ublas::column_major> &out_mat,
								int in_m, int in_n)
{
	for (int i = 0; i < in_m; i++)
		for (int j = 0; j < in_n; j++)
			for (int k = 0; k < (int )in_mat.size1(); k++)
				for (int l = 0; l < (int )in_mat.size2(); l++)
					out_mat(i * in_mat.size1() + k, j * in_mat.size2()) = in_mat(k, l);
}


// -----------------------------------------------------------------------------
//	mat_cross
// -----------------------------------------------------------------------------
//	uBlasには外積は定義されていないため
//	なぜuBlasには，通常言うところの「外積」がないか？（おそらくFAQ）
//		uBlasは汎用の行列計算用ライブラリであるため，基本的に？長さ3のベクトルのときのみ
//		定義される外積を実装しないというのが設計者のポリシーの模様
void	CameraCalibration::mat_cross(
										const ublas::vector<double> &in_vec_A,
										const ublas::vector<double> &in_vec_B,
										ublas::vector<double> &out_vec_C)
{

	out_vec_C(0) = in_vec_A(1) * in_vec_B(2) - in_vec_B(1) * in_vec_A(2);
	out_vec_C(1) = in_vec_A(2) * in_vec_B(0) - in_vec_B(2) * in_vec_A(0);
	out_vec_C(2) = in_vec_A(0) * in_vec_B(1) - in_vec_B(0) * in_vec_A(1);
}


// -----------------------------------------------------------------------------
//	mat_norm
// -----------------------------------------------------------------------------
//
double	CameraCalibration::mat_norm(const ublas::matrix<double, ublas::column_major> &in_mat)
{
	int	m = in_mat.size1();
	int	n = in_mat.size2();
	int	len = 1;

	if (m != 1)
		len = m;
	if (n != 1)
		len = n;

	ublas::vector<double> vec(len);

	if (m != 1)
		for (int i = 0; i < len; i++)
			vec(i) = in_mat(i, 0);

	if (n != 1)
		for (int i = 0; i < len; i++)
			vec(i) = in_mat(0, i);

	return ublas::norm_2(vec);
}


// -----------------------------------------------------------------------------
//	mat_norm
// -----------------------------------------------------------------------------
//
double	CameraCalibration::mat_norm(const ublas::vector<double> &in_vec)
{
	return ublas::norm_2(in_vec);
}


// -----------------------------------------------------------------------------
//	mat_svd
// -----------------------------------------------------------------------------
//
void	CameraCalibration::mat_svd(
								const ublas::matrix<double, ublas::column_major> &in_mat,
								ublas::matrix<double, ublas::column_major> &out_U,
								ublas::matrix<double, ublas::column_major> &out_S,
								ublas::matrix<double, ublas::column_major> &out_V)
{	int	m = in_mat.size1();
	int	n = in_mat.size2();
	int	s_len;
	if (m < n)
		s_len = m;
	else
		s_len = n;

	ublas::vector<double>	S(s_len);
	ublas::matrix<double, ublas::column_major>	A(m, n);

	A = in_mat;

	lapack::gesvd('A', 'A', A, S, out_U, out_V);		// lapackの呼び出し
	out_V = ublas::trans(out_V);						// matlabと同じにするため

	out_S.clear();
	for (int i = 0; i < s_len; i++)
		out_S(i, i) = S(i);
}


// -----------------------------------------------------------------------------
//	mat_mean
// -----------------------------------------------------------------------------
//
double	CameraCalibration::mat_mean(const ublas::vector<double> &in_vec)
{
	double	result = 0;

	for (int i = 0; i < (int )in_vec.size(); i++)
		result += in_vec(i);
	result /= (double )in_vec.size();

	return result;
}


// -----------------------------------------------------------------------------
//	mat_mean_dim
// -----------------------------------------------------------------------------
//
void	CameraCalibration::mat_mean_dim(
				const ublas::matrix<double, ublas::column_major> &in_mat,
				ublas::matrix<double, ublas::column_major> &out_mat,
				int inDimension)
{
	int	m = in_mat.size1();
	int	n = in_mat.size2();

	if (inDimension == 1)
	{
		ublas::vector<double> vec(in_mat.size1());

		for (int i = 0; i < n; i++)
		{
			for (int j = 0; j < m; j++)
				vec(j) = in_mat(j, i);
			out_mat(0, i) = mat_mean(vec);
		}
	}
	else
	{
		ublas::vector<double> vec(in_mat.size2());

		for (int i = 0; i < m; i++)
		{
			for (int j = 0; j < n; j++)
				vec(j) = in_mat(i, j);
			out_mat(i, 0) = mat_mean(vec);
		}
	}
}


// -----------------------------------------------------------------------------
//	mat_sort
// -----------------------------------------------------------------------------
//
void	CameraCalibration::mat_sort(ublas::vector<double> &io_vec)
{
	std::sort(io_vec.begin(), io_vec.end());
}


// -----------------------------------------------------------------------------
//	mat_sort
// -----------------------------------------------------------------------------
//
void	CameraCalibration::mat_sort(ublas::matrix<double, ublas::column_major> &io_mat, int inDimension)
{
	int	i;

	if (inDimension == 1)	//	列ごとにソート
	{
		for (i = 0; i < (int )io_mat.size2(); i++)
		{
			ublas::matrix_column<ublas::matrix<double, ublas::column_major> >	vec(io_mat, i);

			std::sort(vec.begin(), vec.end());
		}

		return;
	}

	//	行ごとにソート（本当は，inDimension>=3のときの処理をしないとダメ）
	for (i = 0; i < (int )io_mat.size1(); i++)
	{
		ublas::matrix_row<ublas::matrix<double, ublas::column_major> >	vec(io_mat, i);

		std::sort(vec.begin(), vec.end());
	}
}


// -----------------------------------------------------------------------------
//	mat_median
// -----------------------------------------------------------------------------
//
void	CameraCalibration::mat_median(
								const ublas::matrix<double, ublas::column_major> &in_mat,
								ublas::matrix<double, ublas::column_major> &out_mat,
								int inDimension)
{
	int	i;
	ublas::matrix<double, ublas::column_major>	mat = in_mat;

	mat_sort(mat, inDimension);

	if (inDimension == 1)	//	列ごとに中央値を求める
	{
std::cout << "warning: no test path!" << std::endl;

		for (i = 0; i < (int )mat.size2(); i++)
		{
			ublas::matrix_column<ublas::matrix<double, ublas::column_major> >	vec(mat, i);

			out_mat(0, i) = _mat_median_sub(vec);
		}

		return;
	}

	//	行ごとに中央値を求める（本当は，inDimension>=3のときの処理をしないとダメ）
	for (i = 0; i < (int )mat.size1(); i++)
	{
		ublas::matrix_row<ublas::matrix<double, ublas::column_major> >	vec(mat, i);
//std::cout << "vec" << vec << std::endl;

		out_mat(i, 0) = _mat_median_sub(vec);
//std::cout << "out_mat(i, 0)" << out_mat(i, 0) << std::endl;

	}
}


// -----------------------------------------------------------------------------
//	mat_std
// -----------------------------------------------------------------------------
//
double	CameraCalibration::mat_std(const ublas::vector<double> &in_vec)
{
	int		i;
	double	avg = 0.0;
	double	sigma = 0.0;

	for (i = 0; i < (int )in_vec.size(); i++)
		avg += in_vec(i);
	avg /= (double )in_vec.size();

	for (i = 0; i < (int )in_vec.size(); i++)
		sigma += pow((in_vec(i) - avg), 2);
	sigma = sqrt(sigma / (double )(in_vec.size() - 1));		// matlabの標準偏差は分母がn-1

	return sigma;
}


// -----------------------------------------------------------------------------
//	mat_round
// -----------------------------------------------------------------------------
//
double	CameraCalibration::mat_round(double in_value)
{
	double	sign = 1.0;
	if (in_value < 0)
	{
		in_value *= -1.0;
		sign = -1.0;
	}

	double	result = floor(in_value);

	if (in_value - result < 0.5)
		return result * sign;

	return (result + 1) * sign;
}


// -----------------------------------------------------------------------------
//	mat_conv2_same
// -----------------------------------------------------------------------------
//
void	CameraCalibration::mat_conv2_same(
					const ublas::matrix<double, ublas::column_major> &in_A,
					const ublas::matrix<double, ublas::column_major> &in_B,
					ublas::matrix<double, ublas::column_major> &out_mat)
{
	int	out_rows = in_A.size1();
	int	out_columns = in_A.size2();
	int	edg_rows = (in_B.size1() - 1) / 2;
	int	edg_columns = (in_B.size2() - 1) / 2;

	for (int i = 0; i < out_rows; i++)
	{
		for (int j = 0; j < out_columns; j++)
		{
			double	sum = 0;
			int	ia = i - edg_rows;
			if (ia < 0)
				ia = 0;
			int	ib = edg_rows - i;
			if (ib < 0)
				ib = 0;
			ib = in_B.size1() - 1 - ib;

			for ( ; ia < (int )in_A.size1() && ib >= 0; ia++, ib--)
			{
				int	ja = j - edg_columns;
				if (ja < 0)
					ja = 0;
				int	jb = edg_columns - j;
				if (jb < 0)
					jb = 0;
				jb = in_B.size2() - 1 - jb;

				for ( ; ja < (int )in_A.size2() && jb >= 0; ja++, jb--)
					sum += in_A(ia, ja) * in_B(ib, jb);
			}
			out_mat(i, j) = sum;
		}
	}
}


// -----------------------------------------------------------------------------
//	mat_gradient
// -----------------------------------------------------------------------------
//
void	CameraCalibration::mat_gradient(
					const ublas::matrix<double, ublas::column_major> &in_mat,
					ublas::matrix<double, ublas::column_major> &out_x,
					ublas::matrix<double, ublas::column_major> &out_y)
{
	int	i, j, count;
	double	dF;

	for (i = 0; i < (int )in_mat.size1() ; i++)
	{
		for (j = 0; j < (int )in_mat.size2(); j++)
		{
			dF = 0;
			count = 0;

			if (j > 0)
			{
				dF = in_mat(i, j) - in_mat(i, j - 1);
				count++;
			}

			if (j + 1 < (int )in_mat.size2())
			{
				dF += in_mat(i, j + 1) - in_mat(i, j);
				count++;
			}

			out_x(i, j) = dF / (double )count;
		}
	}

	for (j = 0; j < (int )in_mat.size2(); j++)
	{
		for (i = 0; i < (int )in_mat.size1() ; i++)
		{
			dF = 0;
			count = 0;

			if (i > 0)
			{
				dF = in_mat(i, j) - in_mat(i - 1, j);
				count++;
			}

			if (i + 1 < (int )in_mat.size1())
			{
				dF += in_mat(i + 1, j) - in_mat(i, j);
				count++;
			}

			out_y(i, j) = dF / (double )count;
		}
	}
}


// -----------------------------------------------------------------------------
//	mat_sum
// -----------------------------------------------------------------------------
//
double	CameraCalibration::mat_sum(const ublas::vector<double> &in_vec)
{
	double	result = 0;

	for (int i = 0; i < (int )in_vec.size(); i++)
		result += in_vec(i);

	return result;
}


// -----------------------------------------------------------------------------
//	mat_sum
// -----------------------------------------------------------------------------
//
void	CameraCalibration::mat_sum(
					const ublas::matrix<double, ublas::column_major> &in_mat,
					ublas::vector<double> &out_vec)
{
	//	matlabのDimensionが1のときのみ

	out_vec.clear();
	for (int i = 0; i < (int )in_mat.size1(); i++)
		for (int j = 0; j < (int )in_mat.size2(); j++)
			out_vec(j) += in_mat(i, j);
}


// -----------------------------------------------------------------------------
//	_mat_median_sub
// -----------------------------------------------------------------------------
//
double	CameraCalibration::_mat_median_sub(const ublas::vector<double> &in_vec)
{
	int	m = in_vec.size() / 2;

	if (in_vec.size() % 2 != 0)	//	長さが奇数のとき
		return in_vec(m);

	//	長さが偶数のとき
	return (in_vec(m - 1) + in_vec(m)) / 2.0;
}

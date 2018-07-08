// =============================================================================
//  StereoCalibration.cpp
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
	\file		StereoCalibration.cpp
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

#include "StereoCalibration.hpp"


//bool	g_debug_enabled = false;


//  StereoCalibration class public member functions ===========================
// -----------------------------------------------------------------------------
//	StereoCalibration
// -----------------------------------------------------------------------------
//
StereoCalibration::StereoCalibration(int inImageWidth, int inImageHeight)
	: CameraCalibration(inImageWidth, inImageHeight)
{
	//	いくつかのパラメータを初期化
}


// -----------------------------------------------------------------------------
//	~StereoCalibration
// -----------------------------------------------------------------------------
//
StereoCalibration::~StereoCalibration()
{
}


// -----------------------------------------------------------------------------
//	doCalibration
// -----------------------------------------------------------------------------
//
void	StereoCalibration::DoCalibration()
{
	int	i, j;
	ublas::matrix<double, ublas::column_major>	R_left(3, 3);
	ublas::matrix<double, ublas::column_major>	R_right(3, 3);
	ublas::matrix<double, ublas::column_major>	R_ref(3, 3);
	ublas::matrix<double, ublas::column_major>	jacobian(9, 3);
	ublas::matrix<double, ublas::column_major>	T_ref(3, 1);
	ublas::matrix<double, ublas::column_major>	om_ref(3, 1);

	T = ublas::matrix<double, ublas::column_major>(3, 1);
	om = ublas::matrix<double, ublas::column_major>(3, 1);
	R = ublas::matrix<double, ublas::column_major>(3, 3);

	ublas::matrix<double, ublas::column_major>	T_ref_list(3, omc_left_list.size());
	ublas::matrix<double, ublas::column_major>	om_ref_list(3, omc_left_list.size());

	for (i = 0; i < (int )omc_left_list.size(); i++)
	{
		//	Align the structure from the first view:
		rodrigues(omc_left_list[i], R_left, jacobian);
		rodrigues(omc_right_list[i], R_right, jacobian);
		R_ref = ublas::prod(R_right, ublas::trans(R_left));
		T_ref = Tc_right_list[i] - ublas::prod(R_ref, Tc_left_list[i]);
		rodrigues(R_ref, om_ref, jacobian);

		for (j = 0; j < 3; j++)
		{
			om_ref_list(j, i) = om_ref(j, 0);
			T_ref_list(j, i) = T_ref(j, 0);
		}
	}

	//	Robust estimate of the initial value for rotation and translation between the two views:
	mat_median(om_ref_list, om, 2);
	mat_median(T_ref_list, T, 2);

	rodrigues(om, R, jacobian);

//std::cout << "om_ref_list" << om_ref_list << std::endl;
//std::cout << "T_ref_list" << T_ref_list << std::endl;

//std::cout << "om" << om << std::endl;
//std::cout << "T" << T << std::endl;
//std::cout << "R" << R << std::endl;

	//	some initializations
	mainOptimization();
}


// -----------------------------------------------------------------------------
//	CalcRectifyIndex
// -----------------------------------------------------------------------------
//
void	StereoCalibration::CalcRectifyIndex()
{
	ublas::matrix<double, ublas::column_major>	R(3, 3);
	ublas::matrix<double, ublas::column_major>	jacobian(9, 3);

	rodrigues(om, R, jacobian);

	// Bring the 2 cameras in the same orientation by rotating them "minimally": 
	ublas::matrix<double, ublas::column_major>	om_dash = om;
	ublas::matrix<double, ublas::column_major>	r_r(3, 3);

	om_dash = om / -2;
	rodrigues(om_dash, r_r, jacobian);
	
	//ublas::matrix<double, ublas::column_major>	r_l(3, 3);
	ublas::matrix<double, ublas::column_major>	r_l;
	r_l = ublas::trans(r_r);

	ublas::matrix<double, ublas::column_major>	t;
	t = ublas::prod(r_r, T);

	//	Rotate both cameras so as to bring the translation vector in alignment with the (1;0;0) axis:
	int	type_stereo;
	ublas::vector<double>	uu(3);
	ublas::vector<double>	tt(3);

	for (int i = 0; i < 3; i++)
		tt(i) = t(i, 0);

	if (fabs(tt(0)) > fabs(tt(1)))
	{
		type_stereo = 0;
		uu(0) = 1; uu(1) = 0; uu(2) = 0;	// Horizontal epipolar lines
	}
	else
	{
		type_stereo = 1;
		uu(0) = 0; uu(1) = 1; uu(2) = 0;	// Vertical epipolar lines
	}

	if (ublas::inner_prod(uu, tt) < 0)
		uu = -1 * uu;	//	Swtich side of the vector

	ublas::vector<double>	ww(3);
	mat_cross(tt, uu, ww);
	ww = ww / mat_norm(ww);
	ww = acos(fabs(ublas::inner_prod(tt, uu)) / (mat_norm(tt) * mat_norm(uu))) * ww;

	ublas::matrix<double, ublas::column_major>	ww_mat(3, 1);
	for (int i = 0; i < 3; i++)
		ww_mat(i, 0) = ww(i);

	ublas::matrix<double, ublas::column_major>	R2(3, 3);
	rodrigues(ww_mat, R2, jacobian);

//std::cout << "ww_mat:" << ww_mat << std::endl;
//std::cout << "R2:" << R2 << std::endl;

	// Global rotations to be applied to both views:
	ublas::matrix<double, ublas::column_major>	R_R(3, 3);
	ublas::matrix<double, ublas::column_major>	R_L(3, 3);

	R_R = ublas::prod(R2, r_r);
	R_L = ublas::prod(R2, r_l);

//std::cout << "R_R:" << R_R << std::endl;
//std::cout << "R_L:" << R_L << std::endl;

	// The resulting rigid motion between the two cameras after image rotations (substitutes of om, R and T):
	ublas::matrix<double, ublas::column_major>	R_new(3, 3);
	R_new = ublas::identity_matrix<double>(3);

	ublas::matrix<double, ublas::column_major>	om_new(3, 1);
	om_new.clear();

	ublas::matrix<double, ublas::column_major>	T_new;
	T_new = ublas::prod(R_R, T);

//std::cout << "R_new:" << R_new << std::endl;
//std::cout << "T_new:" << T_new << std::endl;

	// Computation of the *new* intrinsic parameters for both left and right cameras:
	// Vertical focal length *MUST* be the same for both images (here, we are trying to find a focal length that retains as much information contained in the original distorted images):
	double	fc_y_left_new, fc_y_right_new, fc_y_new;
	if (kc_left(0) < 0)
		fc_y_left_new = fc_left(1) * (1 + kc_left(0) * (mImageWidth * mImageWidth + mImageHeight * mImageHeight)
							/ (4 * fc_left(1) * fc_left(1)));
	else
		fc_y_left_new = fc_left(1);

	if (kc_right(0) < 0)
		fc_y_right_new = fc_right(1) * (1 + kc_right(0) * (mImageWidth * mImageWidth + mImageHeight * mImageHeight)
							/ (4 * fc_right(1) * fc_right(1)));
	else
		fc_y_right_new = fc_right(1);

	fc_y_new = __min(fc_y_left_new, fc_y_right_new);

//std::cout << "fc_y_left_new:" << fc_y_left_new << std::endl;
//std::cout << "fc_y_right_new:" << fc_y_right_new << std::endl;
//std::cout << "fc_y_new:" << fc_y_new << std::endl;

	// For simplicity, let's pick the same value for the horizontal focal length as the vertical focal length (resulting into square pixels):
	ublas::vector<double>	fc_left_new(2);
	ublas::vector<double>	fc_right_new(2);

	fc_left_new(0) = mat_round(fc_y_new);
	fc_left_new(1) = mat_round(fc_y_new);

	fc_right_new(0) = mat_round(fc_y_new);
	fc_right_new(1) = mat_round(fc_y_new);

//std::cout << "fc_left_new:" << fc_left_new << std::endl;
//std::cout << "fc_right_new:" << fc_right_new << std::endl;

	// Select the new principal points to maximize the visible area in the rectified images
	ublas::vector<double>	temp_vec(2);

	temp_vec(0) = (mImageWidth - 1) / 2; temp_vec(1) = (mImageHeight - 1) / 2;

	int	n = 4;
	ublas::matrix<double, ublas::column_major>	x(2, n);
	ublas::matrix<double, ublas::column_major>	xn(2, n);
	ublas::matrix<double, ublas::column_major>	xnn(3, n);
	ublas::matrix<double, ublas::column_major>	xnnn(2, n);
	ublas::matrix<double, ublas::column_major>	dxdom(2 * n, 3);
	ublas::matrix<double, ublas::column_major>	dxdT(2 * n, 3);
	ublas::matrix<double, ublas::column_major>	dxdf(2 * n, 2);
	ublas::matrix<double, ublas::column_major>	dxdc(2 * n, 2);
	ublas::matrix<double, ublas::column_major>	dxdk(2 * n, 5);
	ublas::matrix<double, ublas::column_major>	dxdalpha(2 * n, 1);

	x(0, 0) = 0; x(0, 1) = mImageWidth - 1; x(0, 2) = mImageWidth - 1; x(0, 3) = 0;
	x(1, 0) = 0; x(1, 1) = 0; x(1, 2) = mImageHeight - 1; x(1, 3) = mImageHeight - 1;
	normalize_pixel(fc_left, cc_left, kc_left, alpha_c_left, x, xn);

//std::cout << "xn:" << xn << std::endl;

	xnn(0, 0) = xn(0, 0); xnn(0, 1) = xn(0, 1); xnn(0, 2) = xn(0, 2); xnn(0, 3) = xn(0, 3);
	xnn(1, 0) = xn(1, 0); xnn(1, 1) = xn(1, 1); xnn(1, 2) = xn(1, 2); xnn(1, 3) = xn(1, 3);
	xnn(2, 0) = 1; xnn(2, 1) = 1; xnn(2, 2) = 1; xnn(2, 3) = 1;

	ublas::matrix<double, ublas::column_major>	R_Ln(3, 3);
	rodrigues(R_L, R_Ln, jacobian);

	ublas::vector<double>	z2(2); z2.clear();
	ublas::matrix<double, ublas::column_major>	z3(3, 1); z3.clear();
	ublas::vector<double>	z5(5); z5.clear();

	project_points2(xnn, R_Ln, z3, fc_left_new, z2, z5, 0, xnnn, dxdom, dxdT, dxdf, dxdc, dxdk, dxdalpha);

//std::cout << "xnnn:" << xnnn << std::endl;

	ublas::matrix<double, ublas::column_major>	mean_out(2, 1);
	mat_mean_dim(xnnn, mean_out, 2);

	ublas::vector<double>	cc_left_new(2);
	cc_left_new(0) = (mImageWidth - 1.0) / 2.0 - mean_out(0, 0);
	cc_left_new(1) = (mImageHeight - 1.0) / 2.0 - mean_out(1, 0);

//std::cout << "mean_out:" << mean_out << std::endl;
//std::cout << "cc_left_new:" << cc_left_new << std::endl;

	x(0, 0) = 0; x(0, 1) = mImageWidth - 1; x(0, 2) = mImageWidth - 1; x(0, 3) = 0;
	x(1, 0) = 0; x(1, 1) = 0; x(1, 2) = mImageHeight - 1; x(1, 3) = mImageHeight - 1;
	normalize_pixel(fc_right, cc_right, kc_right, alpha_c_right, x, xn);

	xnn(0, 0) = xn(0, 0); xnn(0, 1) = xn(0, 1); xnn(0, 2) = xn(0, 2); xnn(0, 3) = xn(0, 3);
	xnn(1, 0) = xn(1, 0); xnn(1, 1) = xn(1, 1); xnn(1, 2) = xn(1, 2); xnn(1, 3) = xn(1, 3);
	xnn(2, 0) = 1; xnn(2, 1) = 1; xnn(2, 2) = 1; xnn(2, 3) = 1;

	ublas::matrix<double, ublas::column_major>	R_Rn(3, 3);

	rodrigues(R_R, R_Rn, jacobian);
	project_points2(xnn, R_Rn, z3, fc_right_new, z2, z5, 0, xnnn, dxdom, dxdT, dxdf, dxdc, dxdk, dxdalpha);
	mat_mean_dim(xnnn, mean_out, 2);

	ublas::vector<double>	cc_right_new(2);
	cc_right_new(0) = (mImageWidth - 1) / 2.0 - mean_out(0, 0);
	cc_right_new(1) = (mImageHeight - 1) / 2.0 - mean_out(1, 0);

//std::cout << "cc_left_new:" << cc_left_new << std::endl;
//std::cout << "cc_right_new:" << cc_right_new << std::endl;

	// For simplivity, set the principal points for both cameras to be the average of the two principal points.
	if (type_stereo == 0)
	{
		// Horizontal stereo
		double	cc_y_new = (cc_left_new(1) + cc_right_new(1)) / 2;
		cc_left_new(0) = cc_left_new(0);
		cc_left_new(1) = cc_y_new;
		cc_right_new(0) = cc_right_new(0);
		cc_right_new(1) = cc_y_new;
	}
	else
	{
		// Vertical stereo
		double	cc_x_new = (cc_left_new(0) + cc_right_new(0)) / 2;
		cc_left_new(0) = cc_x_new;
		cc_left_new(1) = cc_left_new(1);
		cc_right_new(0) = cc_x_new;
		cc_right_new(1) = cc_right_new(1);
	}

//std::cout << "cc_left_new:" << cc_left_new << std::endl;
//std::cout << "cc_right_new:" << cc_right_new << std::endl;

	// Of course, we do not want any skew or distortion after rectification:
	double	alpha_c_left_new = 0;
	double	alpha_c_right_new = 0;
	ublas::vector<double>	kc_left_new(5); kc_left_new.clear();
	ublas::vector<double>	kc_right_new(5); kc_right_new.clear();

	// The resulting left and right camera matrices:
	ublas::matrix<double, ublas::column_major>	KK_left_new(3, 3);
	KK_left_new(0, 0) = fc_left_new(0);
	KK_left_new(0, 1) = fc_left_new(0) * alpha_c_left_new;
	KK_left_new(0, 2) = cc_left_new(0);
	KK_left_new(1, 0) = 0;
	KK_left_new(1, 1) = fc_left_new(1);
	KK_left_new(1, 2) = cc_left_new(1);
	KK_left_new(2, 0) = 0;
	KK_left_new(2, 1) = 0;
	KK_left_new(2, 2) = 1;

	ublas::matrix<double, ublas::column_major>	KK_right_new(3, 3);
	KK_right_new(0, 0) = fc_right_new(0);
	KK_right_new(0, 1) = fc_right_new(0) * alpha_c_right;
	KK_right_new(0, 2) = cc_right_new(0);
	KK_right_new(1, 0) = 0;
	KK_right_new(1, 1) = fc_right_new(1);
	KK_right_new(1, 2) = cc_right_new(1);
	KK_right_new(2, 0) = 0;
	KK_right_new(2, 1) = 0;
	KK_right_new(2, 2) = 1;

//std::cout << "KK_left_new:" << KK_left_new << std::endl;
//std::cout << "KK_right_new:" << KK_right_new << std::endl;

	// The sizes of the images are the same:
	double	nx_right_new = mImageWidth;
	double	ny_right_new = mImageHeight;
	double	nx_left_new = mImageWidth;
	double	ny_left_new = mImageHeight;

	// Let's rectify the entire set of calibration images:
	printf("Pre-computing the necessary data to quickly rectify the images (may take a while depending on the image resolution, but needs to be done only once - even for color images)...\n\n");

	// Pre-compute the necessary indices and blending coefficients to enable quick rectification:
	rect_index(mImageWidth, mImageHeight, R_L, fc_left, cc_left, kc_left, alpha_c_left, KK_left_new,
				a1_left, a2_left, a3_left, a4_left, ind_new_left, ind_1_left, ind_2_left, ind_3_left, ind_4_left);

	rect_index(mImageWidth, mImageHeight, R_R, fc_right, cc_right, kc_right, alpha_c_right, KK_right_new,
				a1_right, a2_right, a3_right, a4_right, ind_new_right, ind_1_right, ind_2_right, ind_3_right, ind_4_right);
}


// -----------------------------------------------------------------------------
//	DumpResults
// -----------------------------------------------------------------------------
void	StereoCalibration::DumpResults()
{
	//	すでにキャリブレーションされているかどうか，チェックすべき
	
	std::cout << "Stereo calibration parameters after optimization:" << std::endl;

	printf("\n\nCalibration results after optimization (with uncertainties):\n\n");
	printf("Focal Length:          fc_left = [ %3.5f   %3.5f ] error [ %3.5f   %3.5f ]\n", fc_left(0), fc_left(1), fc_left_error(0), fc_left_error(1));
	printf("Principal point:       cc_left = [ %3.5f   %3.5f ] error [ %3.5f   %3.5f ]\n", cc_left(0), cc_left(1), cc_left_error(0), cc_left_error(1));
	printf("Skew:             alpha_c_left = [ %3.5f ] error [ %3.5f ]   => angle of pixel = %3.5f  error [ %3.5f ] degrees\n",
																alpha_c_left, alpha_c_left_error,
																90 - atan(alpha_c_left) * 180 / 3.141592,
																atan(alpha_c_left_error) * 180 / 3.141592);
	printf("Distortion:            kc_left = [ %3.5f   %3.5f   %3.5f   %3.5f   %5.5f ] error [ %3.5f   %3.5f   %3.5f   %3.5f   %5.5f ]\n",
		kc_left(0), kc_left(1), kc_left(2), kc_left(3), kc_left(4),
		kc_left_error(0), kc_left_error(1), kc_left_error(2), kc_left_error(3), kc_left_error(4));
	
	printf("\n\nCalibration results after optimization (with uncertainties):\n\n");
	printf("Focal Length:          fc_right = [ %3.5f   %3.5f ] error [ %3.5f   %3.5f ]\n", fc_right(0), fc_right(1), fc_right_error(0), fc_right_error(1));
	printf("Principal point:       cc_right = [ %3.5f   %3.5f ] error [ %3.5f   %3.5f ]\n", cc_right(0), cc_right(1), cc_right_error(0), cc_right_error(1));
	printf("Skew:             alpha_c_right = [ %3.5f ] error [ %3.5f ]   => angle of pixel = %3.5f  error [ %3.5f ] degrees\n",
																alpha_c_right, alpha_c_right_error,
																90 - atan(alpha_c_right) * 180 / 3.141592,
																atan(alpha_c_right_error) * 180 / 3.141592);
	printf("Distortion:            kc_right = [ %3.5f   %3.5f   %3.5f   %3.5f   %5.5f ] error [ %3.5f   %3.5f   %3.5f   %3.5f   %5.5f ]\n",
		kc_right(0), kc_right(1), kc_right(2), kc_right(3), kc_right(4),
		kc_right_error(0), kc_right_error(1), kc_right_error(2), kc_right_error(3), kc_right_error(4));

	printf("\n\nExtrinsic parameters (position of right camera wrt left camera):\n\n");
	printf("Rotation vector:             om = [ %3.5f   %3.5f  %3.5f ] error [ %3.5f   %3.5f  %3.5f ]\n",
		om(0, 0), om(1, 0), om(2, 0),
		om_error(0, 0), om_error(1, 0), om_error(2, 0));
	printf("Translation vector:           T = [ %3.5f   %3.5f  %3.5f ] error [ %3.5f   %3.5f  %3.5f ]\n",
		T(0, 0), T(1, 0), T(2, 0),
		T_error(0, 0), T_error(1, 0), T_error(2, 0));

std::cout << "Note: The numerical errors are approximately three times the standard deviations (for reference)." << std::endl;
//std::cout << "Suggested threshold = " << std::endl;
}


#define	MAIN_OPTIMIZATION_CHANGE_MIN	5e-6
#define	MAIN_OPTIMIZATION_ITER_MAX		100


//  StreoCalibration class protected member functions ==========================
// -----------------------------------------------------------------------------
//	mainOptimization
// -----------------------------------------------------------------------------
//
void	StereoCalibration::mainOptimization()
{
	int		i, j, kk, i_dash, j_dash;
	int	n_ima = omc_left_list.size();

	//	This threshold is used only to automatically
	//	identify non-consistant image pairs (set to Infinity to not reject pairs)
	double	threshold = 50;
	bool	recompute_intrinsic_right = true;
	bool	recompute_intrinsic_left = true;

	//	%- Set to zero the entries of the distortion vectors that are not attempted to be estimated.
	//kc_right = kc_right .* ~~est_dist_right;
	//kc_left = kc_left .* ~~est_dist_left;

	//	Main Optimization
	//	Jに必要な行数の計算...
	int	J_rows = 0;
	for (kk = 0; kk < n_ima; kk++)
		J_rows += X_left_list[kk].size2() * 4;

	//	MATLABではsparseで確保しているが，それほど巨大な行列
	//	でもないので普通に確保してみる
	ublas::matrix<double, ublas::column_major>	J(J_rows, 20 + (1 + n_ima) * 6);
	ublas::matrix<double, ublas::column_major>	e(J_rows, 1);
	ublas::vector<double>	selected_variables(J.size2());
	ublas::vector<double>	param(J.size2());

	J.clear();
	e.clear();
	selected_variables.clear();
	param.clear();

	ublas::matrix<double, ublas::column_major>	J2;
	ublas::matrix<double, ublas::column_major>	J2_inv;

	double	change = 1.0;
	int		iter = 0;

	while (	change > MAIN_OPTIMIZATION_CHANGE_MIN &&
			iter < MAIN_OPTIMIZATION_ITER_MAX)
	{
		param(0) = fc_left(0);
		param(1) = fc_left(1);
		param(2) = cc_left(0);
		param(3) = cc_left(1);
		param(4) = alpha_c_left;
		for (i = 0; i < 5; i++)
			param(5 + i) = kc_left(i);
		param(10) = fc_right(0);
		param(11) = fc_right(1);
		param(12) = cc_right(0);
		param(13) = cc_right(1);
		param(14) = alpha_c_right;
		for (i = 0; i < 5; i++)
			param(15 + i) = kc_right(i);
		for (i = 0; i < 3; i++)
			param(20 + i) = om(i, 0);
		for (i = 0; i < 3; i++)
			param(23 + i) = T(i, 0);

		//	must check active image first!
		int	row_offset = 0;
		for (kk = 0; kk < n_ima; kk++)
		{
			//	Project the structure onto the left view:
			for (i = 0; i < 3; i++)
				param(26 + kk * 6 + i) = omc_left_list[kk](i, 0);
			for (i = 0; i < 3; i++)
				param(29 + kk * 6 + i) = Tc_left_list[kk](i, 0);

			int	Nckk = X_left_list[kk].size2();

			ublas::matrix<double, ublas::column_major>	Jkk(4 * Nckk, J.size2());
			ublas::matrix<double, ublas::column_major>	ekk(4 * Nckk, 1);

			Jkk.clear();
			ekk.clear();

			ublas::matrix<double, ublas::column_major>	xl(2, Nckk);
			ublas::matrix<double, ublas::column_major>	dxldomckk(2 * Nckk, 3);
			ublas::matrix<double, ublas::column_major>	dxldTckk(2 * Nckk, 3);
			ublas::matrix<double, ublas::column_major>	dxldfl(2 * Nckk, 2);
			ublas::matrix<double, ublas::column_major>	dxldcl(2 * Nckk, 2);
			ublas::matrix<double, ublas::column_major>	dxldkl(2 * Nckk, 5);
			ublas::matrix<double, ublas::column_major>	dxldalphal(2 * Nckk, 1);

			// ToDo: mIsEstimateAspectRatio = falseのときの処理を考えないとダメ
			project_points2(
				X_left_list[kk], omc_left_list[kk], Tc_left_list[kk],
				fc_left, cc_left, kc_left, alpha_c_left,
				xl, dxldomckk, dxldTckk, dxldfl, dxldcl, dxldkl, dxldalphal);

			for (i = 0; i < Nckk; i++)
			{
				ekk(i * 2, 0) = x_left_list[kk](0, i) - xl(0, i);
				ekk(i * 2 + 1, 0) = x_left_list[kk](1, i) - xl(1, i);
			}

			//	_DEF_MAT_RANGE(JJ3_r1, JJ3, 0, 10, 0, 10)　みたいなマクロを作ったほうがよいかも
			ublas::matrix_range<ublas::matrix<double, ublas::column_major> >
				Jkk_r1(Jkk, ublas::range(0, 2 * Nckk), ublas::range(6 * kk + 6 + 20, 6 * kk + 6 + 20 + 3));
			Jkk_r1 = dxldomckk;

			ublas::matrix_range<ublas::matrix<double, ublas::column_major> >
				Jkk_r2(Jkk, ublas::range(0, 2 * Nckk), ublas::range(6 * kk + 6 + 20 + 3, 6 * kk + 6 + 20 + 3 + 3));
			Jkk_r2 = dxldTckk;

			ublas::matrix_range<ublas::matrix<double, ublas::column_major> >
				Jkk_r3(Jkk, ublas::range(0, 2 * Nckk), ublas::range(0, 2));
			Jkk_r3 = dxldfl;

			ublas::matrix_range<ublas::matrix<double, ublas::column_major> >
				Jkk_r4(Jkk, ublas::range(0, 2 * Nckk), ublas::range(2, 4));
			Jkk_r4 = dxldcl;

			ublas::matrix_range<ublas::matrix<double, ublas::column_major> >
				Jkk_r5(Jkk, ublas::range(0, 2 * Nckk), ublas::range(4, 5));
			Jkk_r5 = dxldalphal;

			ublas::matrix_range<ublas::matrix<double, ublas::column_major> >
				Jkk_r6(Jkk, ublas::range(0, 2 * Nckk), ublas::range(5, 10));
			Jkk_r6 = dxldkl;

			//	Project the structure onto the right view:
			ublas::matrix<double, ublas::column_major>	omr(3, 1);
			ublas::matrix<double, ublas::column_major>	Tr(3, 1);
			ublas::matrix<double, ublas::column_major>	domrdomckk(3, 3);
			ublas::matrix<double, ublas::column_major>	domrdTckk(3, 3);
			ublas::matrix<double, ublas::column_major>	domrdom(3, 3);
			ublas::matrix<double, ublas::column_major>	domrdT(3, 3);
			ublas::matrix<double, ublas::column_major>	dTrdomckk(3, 3);
			ublas::matrix<double, ublas::column_major>	dTrdTckk(3, 3);
			ublas::matrix<double, ublas::column_major>	dTrdom(3, 3);
			ublas::matrix<double, ublas::column_major>	dTrdT(3, 3);

			compose_motion(omc_left_list[kk], Tc_left_list[kk], om, T,
				omr, Tr, domrdomckk, domrdTckk, domrdom, domrdT, dTrdomckk, dTrdTckk, dTrdom, dTrdT);

/*std::cout << "omc_left_list[kk]" << omc_left_list[kk] << std::endl;
std::cout << "Tc_left_list[kk]" << Tc_left_list[kk] << std::endl;
std::cout << "om" << om << std::endl;
std::cout << "T" << T << std::endl;

std::cout << "omr" << omr << std::endl;
std::cout << "Tr" << Tr << std::endl;
std::cout << "domrdomckk" << domrdomckk << std::endl;
std::cout << "domrdTckk" << domrdTckk << std::endl;
std::cout << "domrdom" << domrdom << std::endl;
std::cout << "domrdT" << domrdT << std::endl;
std::cout << "dTrdomckk" << dTrdomckk << std::endl;
std::cout << "dTrdTckk" << dTrdTckk << std::endl;
std::cout << "dTrdom" << dTrdom << std::endl;
std::cout << "dTrdT" << dTrdT << std::endl;*/

			ublas::matrix<double, ublas::column_major>	xr(2, Nckk);
			ublas::matrix<double, ublas::column_major>	dxrdomr(2 * Nckk, 3);
			ublas::matrix<double, ublas::column_major>	dxrdTr(2 * Nckk, 3);
			ublas::matrix<double, ublas::column_major>	dxrdfr(2 * Nckk, 2);
			ublas::matrix<double, ublas::column_major>	dxrdcr(2 * Nckk, 2);
			ublas::matrix<double, ublas::column_major>	dxrdkr(2 * Nckk, 5);
			ublas::matrix<double, ublas::column_major>	dxrdalphar(2 * Nckk, 1);

			// ToDo: mIsEstimateAspectRatio = falseのときの処理を考えないとダメ
			project_points2(
				X_left_list[kk], omr, Tr,
				fc_right, cc_right, kc_right, alpha_c_right,
				xr, dxrdomr, dxrdTr, dxrdfr, dxrdcr, dxrdkr, dxrdalphar);

			for (i = 0; i < Nckk; i++)
			{
				ekk(2 * Nckk + i * 2, 0) = x_right_list[kk](0, i) - xr(0, i);
				ekk(2 * Nckk + i * 2 + 1, 0) = x_right_list[kk](1, i) - xr(1, i);
			}

			ublas::matrix<double, ublas::column_major>	dxrdom(2 * Nckk, 3);
			ublas::matrix<double, ublas::column_major>	dxrdT(2 * Nckk, 3);

			dxrdom = ublas::prod(dxrdomr, domrdom) + ublas::prod(dxrdTr, dTrdom);
			dxrdT = ublas::prod(dxrdomr, domrdT) + ublas::prod(dxrdTr, dTrdT);

			ublas::matrix<double, ublas::column_major>	dxrdomckk(2 * Nckk, 3);
			ublas::matrix<double, ublas::column_major>	dxrdTckk(2 * Nckk, 3);

			dxrdomckk = ublas::prod(dxrdomr, domrdomckk) + ublas::prod(dxrdTr, dTrdomckk);
			dxrdTckk = ublas::prod(dxrdomr, domrdTckk) + ublas::prod(dxrdTr, dTrdTckk);

			ublas::matrix_range<ublas::matrix<double, ublas::column_major> >
				Jkk_r7(Jkk, ublas::range(2 * Nckk, 4 * Nckk), ublas::range(20, 20 + 3));
			Jkk_r7 = dxrdom;

			ublas::matrix_range<ublas::matrix<double, ublas::column_major> >
				Jkk_r8(Jkk, ublas::range(2 * Nckk, 4 * Nckk), ublas::range(23, 23 + 3));
			Jkk_r8 = dxrdT;

			ublas::matrix_range<ublas::matrix<double, ublas::column_major> >
				Jkk_r9(Jkk, ublas::range(2 * Nckk, 4 * Nckk), ublas::range(6 * kk + 6 + 20, 6 * kk + 6 + 20 + 3));
			Jkk_r9 = dxrdomckk;

//std::cout << "dxrdomckk.size1()" << dxrdomckk.size1() << std::endl;
//std::cout << "dxrdomckk.size2()" << dxrdomckk.size2() << std::endl;
//std::cout << "Jkk_r9.size1()" << Jkk_r9.size1() << std::endl;
//std::cout << "Jkk_r9.size2()" << Jkk_r9.size2() << std::endl;


			ublas::matrix_range<ublas::matrix<double, ublas::column_major> >
				Jkk_r10(Jkk, ublas::range(2 * Nckk, 4 * Nckk), ublas::range(6 * kk + 6 + 20 + 3, 6 * kk + 6 + 20 + 3 + 3));
			Jkk_r10 = dxrdTckk;

			ublas::matrix_range<ublas::matrix<double, ublas::column_major> >
				Jkk_r11(Jkk, ublas::range(2 * Nckk, 4 * Nckk), ublas::range(10, 12));
			Jkk_r11 = dxrdfr;

			ublas::matrix_range<ublas::matrix<double, ublas::column_major> >
				Jkk_r12(Jkk, ublas::range(2 * Nckk, 4 * Nckk), ublas::range(12, 14));
			Jkk_r12 = dxrdcr;

			ublas::matrix_range<ublas::matrix<double, ublas::column_major> >
				Jkk_r13(Jkk, ublas::range(2 * Nckk, 4 * Nckk), ublas::range(14, 15));
			Jkk_r13 = dxrdalphar;

			ublas::matrix_range<ublas::matrix<double, ublas::column_major> >
				Jkk_r14(Jkk, ublas::range(2 * Nckk, 4 * Nckk), ublas::range(15, 20));
			Jkk_r14 = dxrdkr;

			//	以下のロジックを今は実装していない
			//emax = max(abs(ekk));
			//if emax >= threshold,
			//	fprintf(1,'Disabling view %d - Reason: the left and right images are found inconsistent (try help calib_stereo for more information)\n',kk);

//std::cout << "Jkk:" << Jkk << std::endl;
//std::cout << "ekk:" << ekk << std::endl;

			ublas::matrix_range<ublas::matrix<double, ublas::column_major> >
				J_r(J, ublas::range(row_offset, row_offset + 4 * Nckk), ublas::range(0, J.size2()));
			J_r = Jkk;
			
			ublas::matrix_range<ublas::matrix<double, ublas::column_major> >
				e_r(e, ublas::range(row_offset, row_offset + 4 * Nckk), ublas::range(0, 1));
			e_r = ekk;

			row_offset += 4 * Nckk;
		}

//std::cout << "J:" << J << std::endl;
//std::cout << "e:" << e << std::endl;

		//	The following vector helps to select the variables to update (for only active images):
		selected_variables.clear();
		selected_variables(0) = 1.0;	// est_fc left
		selected_variables(1) = 1.0;	// est_fc left
		selected_variables(2) = 1.0;	// center optimizaion x left
		selected_variables(3) = 1.0;	// center optimizaion y left
		selected_variables(4) = 0.0;	// est_alpha left
		//selected_variables(4) = 1.0;	// est_alpha left
		selected_variables(5) = 1.0;	// est_dist left
		selected_variables(6) = 1.0;	// est_dist left
		selected_variables(7) = 1.0;	// est_dist left
		selected_variables(8) = 1.0;	// est_dist left
		selected_variables(9) = 0.0;	// est_dist left

		selected_variables(10) = 1.0;	// est_fc right
		selected_variables(11) = 1.0;	// est_fc right
		selected_variables(12) = 1.0;	// center optimizaion x right
		selected_variables(13) = 1.0;	// center optimizaion y right
		selected_variables(14) = 0.0;	// est_alpha right
		//selected_variables(14) = 1.0;	// est_alpha right
		selected_variables(15) = 1.0;	// est_dist right
		selected_variables(16) = 1.0;	// est_dist right
		selected_variables(17) = 1.0;	// est_dist right
		selected_variables(18) = 1.0;	// est_dist right
		selected_variables(19) = 0.0;	// est_dist right

		for (i = 0; i < 6; i++)
			selected_variables(20 + i) = 1.0;

		for (i = 0; i < 6 * n_ima; i++)
			selected_variables(26 + i) = 1.0;	//	activeイメージだけ1にしたほうがよい

		int	temp_num = 0;
		for (i = 0; i < (int )selected_variables.size(); i++)
			if (selected_variables(i) != 0.0)
				temp_num++;

		//	このあたりは冗長なのでまんとかする
		ublas::matrix<double, ublas::column_major>	J_dash(J.size1(), temp_num);
		for(i = 0; i < (int )J.size1(); i++)
		{
			for (j = 0, j_dash = 0; j < (int )J.size2(); j++)
			{
				if (selected_variables(j) != 0.0)
				{
					J_dash(i, j_dash) = J(i, j);
					j_dash++;
				}
			}
		}

		J2 = ublas::matrix<double, ublas::column_major>(temp_num, temp_num);
		J2_inv = ublas::matrix<double, ublas::column_major>(temp_num, temp_num);

		J2 = ublas::prod(ublas::trans(J_dash), J_dash);
		J2_inv = J2;
		mat_inv(J2_inv); 

//std::cout << "J2:" << J2 << std::endl;
//std::cout << "J2_inv:" << J2_inv << std::endl;

		ublas::matrix<double, ublas::column_major>	param_update(temp_num, 1);

		param_update = ublas::prod(
			ublas::matrix<double, ublas::column_major>(ublas::prod(J2_inv, ublas::trans(J_dash))), e);

		for(i = 0, i_dash = 0; i < (int )param.size(); i++)
		{
			if (selected_variables(i) != 0.0)
			{
				param(i) = param(i) + param_update(i_dash, 0);
				i_dash++;
			}
		}

		//	最初の代入と含めてこのあたりは冗長（最適化できると思う）
		fc_left(0) = param(0);
		fc_left(1) = param(1);
		cc_left(0) = param(2);
		cc_left(1) = param(3);
		alpha_c_left = param(4);
		for (i = 0; i < 5; i++)
			kc_left(i) = param(5 + i);
		fc_right(0) = param(10);
		fc_right(1) = param(11);
		cc_right(0) = param(12);
		cc_right(1) = param(13);
		alpha_c_right = param(14);
		for (i = 0; i < 5; i++)
			kc_right(i) = param(15 + i);
		
		bool	est_aspect_ratio_left_st = true;
		if (est_aspect_ratio_left_st == false)
			fc_left(1) = fc_left(0);
		bool	est_aspect_ratio_right_st = true;
		if (est_aspect_ratio_right_st == false)
			fc_right(1) = fc_right(0);

		ublas::matrix<double, ublas::column_major>	om_old(3, 1);
		ublas::matrix<double, ublas::column_major>	T_old(3, 1);
		
		om_old = om;
		T_old = T;

		for (i = 0; i < 3; i++)
			om(i, 0) = param(20 + i);
		for (i = 0; i < 3; i++)
			T(i, 0) = param(23 + i);

		for (kk = 0; kk < n_ima; kk++)
		{
			//	Project the structure onto the left view:
			for (i = 0; i < 3; i++)
				omc_left_list[kk](i, 0) = param(26 + kk * 6 + i);
			for (i = 0; i < 3; i++)
				Tc_left_list[kk](i, 0) = param(29 + kk * 6 + i);
		}

		ublas::vector<double>	temp_vec(6);
			for (i = 0; i < 3; i++)
				temp_vec(i) = T(i, 0);
			for (i = 0; i < 3; i++)
				temp_vec(i + 3) = om(i, 0);

		ublas::vector<double>	temp_vec2(6);
			for (i = 0; i < 3; i++)
				temp_vec2(i) = T(i, 0) - T_old(i, 0);
			for (i = 0; i < 3; i++)
				temp_vec2(i + 3) = om(i, 0) - om_old(i, 0);

		change = mat_norm(temp_vec2) / mat_norm(temp_vec);

std::cout << "iter:" << iter << std::endl;
std::cout << "change" << change << std::endl;

		iter++;
	}

std::cout << "done" << std::endl;

std::cout << "Estimation of uncertainties..." << std::endl;

	ublas::matrix_column<ublas::matrix<double, ublas::column_major> >	e_vec(e, 0);
	double	sigma_x = mat_std(e_vec);

	ublas::vector<double>	param_error(J.size2());
	param_error.clear();
	for (i = 0, i_dash = 0; i < (int )param_error.size(); i++)
	{
		if (selected_variables(i) != 0.0)
		{
			param_error(i) = 3.0 * sqrt(J2_inv(i_dash, i_dash)) * sigma_x;
			i_dash++;
		}
	}

	//	omckk_error, Tckk, omc_left_error_list, Tc_left_error_listはとりあえずスキップ

	fc_left_error = ublas::vector<double>(2);
	cc_left_error = ublas::vector<double>(2);
	kc_left_error = ublas::vector<double>(5);
	fc_right_error = ublas::vector<double>(2);
	cc_right_error = ublas::vector<double>(2);
	kc_right_error = ublas::vector<double>(5);

	fc_left_error(0) = param_error(0);
	fc_left_error(1) = param_error(1);
	cc_left_error(0) = param_error(2);
	cc_left_error(1) = param_error(3);
	alpha_c_left_error = param_error(4);
	for (i = 0; i < 5; i++)
		kc_left_error(i) = param_error(5 + i);
	fc_right_error(0) = param_error(10);
	fc_right_error(1) = param_error(11);
	cc_right_error(0) = param_error(12);
	cc_right_error(1) = param_error(13);
	alpha_c_right_error = param_error(14);
	for (i = 0; i < 5; i++)
		kc_right_error(i) = param_error(15 + i);
	
/*	if (est_aspect_ratio_left_st == false)
		fc_left_error(1) = fc_left_error(0);
	if (est_aspect_ratio_right_st == false)
		fc_right_error(1) = fc_right_error(0);*/

	om_error = ublas::matrix<double, ublas::column_major>(3, 1);
	T_error = ublas::matrix<double, ublas::column_major>(3, 1);

	for (i = 0; i < 3; i++)
		om_error(i, 0) = param_error(20 + i);
	for (i = 0; i < 3; i++)
		T_error(i, 0) = param_error(23 + i);

	ublas::matrix<double, ublas::column_major>	jacobian(9, 3);

	rodrigues(om, R, jacobian);

std::cout << "done" << std::endl;

	DumpResults();
}


// -----------------------------------------------------------------------------
//	compose_motion
// -----------------------------------------------------------------------------
void	StereoCalibration::compose_motion(
								const ublas::matrix<double, ublas::column_major> &in_om1,
								const ublas::matrix<double, ublas::column_major> &in_T1,
								const ublas::matrix<double, ublas::column_major> &in_om2,
								const ublas::matrix<double, ublas::column_major> &in_T2,
								ublas::matrix<double, ublas::column_major> &out_om3,
								ublas::matrix<double, ublas::column_major> &out_T3,
								ublas::matrix<double, ublas::column_major> &out_dom3dom1,
								ublas::matrix<double, ublas::column_major> &out_dom3dT1,
								ublas::matrix<double, ublas::column_major> &out_dom3dom2,
								ublas::matrix<double, ublas::column_major> &out_dom3dT2,
								ublas::matrix<double, ublas::column_major> &out_dT3dom1,
								ublas::matrix<double, ublas::column_major> &out_dT3dT1,
								ublas::matrix<double, ublas::column_major> &out_dT3dom2,
								ublas::matrix<double, ublas::column_major> &out_dT3dT2)
{
	//	Rotations
	ublas::matrix<double, ublas::column_major>	R1(3, 3);
	ublas::matrix<double, ublas::column_major>	dR1dom1(9, 3);
	ublas::matrix<double, ublas::column_major>	R2(3, 3);
	ublas::matrix<double, ublas::column_major>	dR2dom2(9, 3);

	rodrigues(in_om1, R1, dR1dom1);
	rodrigues(in_om2, R2, dR2dom2);

	ublas::matrix<double, ublas::column_major>	R3(3, 3);

	R3 = ublas::prod(R2, R1);

	ublas::matrix<double, ublas::column_major>	dR3dR2(3 * 3, 3 * 3);
	ublas::matrix<double, ublas::column_major>	dR3dR1(3 * 3, 3 * 3);

	dAB(R2, R1, dR3dR2, dR3dR1);

	ublas::matrix<double, ublas::column_major>	dom3dR3(3, 9);
	rodrigues(R3, out_om3, dom3dR3);

//std::cout << "R3" << R3 << std::endl;
//std::cout << "out_om3" << out_om3 << std::endl;
//std::cout << "dom3dR3" << dom3dR3 << std::endl;

	out_dom3dom1 = ublas::prod(
		ublas::matrix<double, ublas::column_major>(ublas::prod(dom3dR3, dR3dR1)), dR1dom1);
	out_dom3dom2 = ublas::prod(
		ublas::matrix<double, ublas::column_major>(ublas::prod(dom3dR3, dR3dR2)), dR2dom2);

	out_dom3dT1.clear();
	out_dom3dT2.clear();

	//	Translations
	ublas::matrix<double, ublas::column_major>	T3t(3, 1);

	T3t = ublas::prod(R2, in_T1);

	ublas::matrix<double, ublas::column_major>	dT3tdR2(3 * 1, 3 * 3);	// p * q, p * n where A(p,n), B(n2,q)
	ublas::matrix<double, ublas::column_major>	dT3tdT1(3 * 1, 1 * 3);	// p * q, q * n

	dAB(R2, in_T1, dT3tdR2, dT3tdT1);

	ublas::matrix<double, ublas::column_major>	dT3tdom2(3, 3);

	dT3tdom2 = ublas::prod(dT3tdR2, dR2dom2);

	out_T3 = T3t + in_T2;

	out_dT3dT1 = dT3tdT1;

	out_dT3dT2.clear();
	for (int i = 0; i < 3; i++)
		out_dT3dT2(i, i) = 1.0;

	out_dT3dom2 = dT3tdom2;
	out_dT3dom1.clear();
}


// -----------------------------------------------------------------------------
//	dAB
// -----------------------------------------------------------------------------
void	StereoCalibration::dAB(
								const ublas::matrix<double, ublas::column_major> &in_A,
								const ublas::matrix<double, ublas::column_major> &in_B,
								ublas::matrix<double, ublas::column_major> &out_dABdA,
								ublas::matrix<double, ublas::column_major> &out_dABdB)
{
	int	p = in_A.size1();
	int	n = in_A.size2();
	int	n2 = in_B.size1();
	int	q = in_B.size2();
	int	i, j, k, ij, kj;

	if (n2 != n)
	{
std::cout << "A and B must have equal inner dimensions" << std::endl;
		return;
	}

	out_dABdA.clear();

	for (i = 0; i < q; i++)
		for (j = 0; j < p; j++)
		{
			ij = j + i * p;
			for (k = 0; k < n; k++)
			{
				kj = j + k * p;
				out_dABdA(ij, kj) = in_B(k, i);
			}
		}

	out_dABdB.clear();

	for (i = 0; i < q; i++)
		for (j = 0; j < p; j++)
		{
			ij = j + i * p;
			for (k = 0; k < n; k++)
			{
				kj = k + i * n;
				out_dABdB(ij, kj) = in_A(j, k);
			}
		}
}

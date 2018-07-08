// =============================================================================
//  CornerFinder.cpp
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
	\file		CornerFinder.cpp
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

#include "CornerFinder.hpp"


//bool	g_debug_enabled = false;


//  CornerFinder class public member functions ===========================
// -----------------------------------------------------------------------------
//	CornerFinder
// -----------------------------------------------------------------------------
//
CornerFinder::CornerFinder(int inImageWidth, int inImageHeight)
	: CameraCalibration(inImageWidth, inImageHeight)
{
	//	いくつかのパラメータを初期化
}


// -----------------------------------------------------------------------------
//	~CornerFinder
// -----------------------------------------------------------------------------
//
CornerFinder::~CornerFinder()
{
}

#define	CORNER_FINDER_RESOLUTION	0.005
#define	CORNER_FINDER_ITER_MAX		10


// -----------------------------------------------------------------------------
//	findGrid
// -----------------------------------------------------------------------------
//
//	入力と出力で座標系が変わっているので要注意（これは元のMatlabのコードがそうなっているため）
//	入力はMatlabの座標系　原点が（1, 1）
//	出力は普通の座標系　原点が（0, 0）　後段のキャリブレーションをこちらの座標系で行うため
//	count_squares, findCorner, findCornerはまた違う仕様なので要注意
//
void	CornerFinder::findGrid(
		const ublas::matrix<unsigned char, ublas::column_major> &in_I,
		const ublas::matrix<double, ublas::column_major> &in_x,
		int	in_wintx, int in_winty,
		double in_dX, double in_dY,
		int in_n_sq_x, double in_n_sq_y,
		ublas::matrix<double, ublas::column_major> &out_XX,
		ublas::matrix<double, ublas::column_major> &out_x,
		ublas::matrix<double, ublas::column_major> &out_X,
		ublas::vector<int> &out_result)
{
	int	i, j;
	int	n = in_x.size2();

	ublas::vector<double>	x(n);
	ublas::vector<double>	y(n);

	for (i = 0; i < n; i++)
	{
		x(i) = in_x(0, i);
		y(i) = in_x(1, i);
	}

	//	Compute the inside points through computation of the planar homography (collineation)
	ublas::matrix<double, ublas::column_major>	hx(3, 4);
	ublas::matrix<double, ublas::column_major>	X(3, 4);
	ublas::matrix<double, ublas::column_major>	Homo(3, 3);

	hx(0, 0) = x(0); hx(1, 0) = y(0); hx(2, 0) = 1.0;
	hx(0, 1) = x(1); hx(1, 1) = y(1); hx(2, 1) = 1.0;
	hx(0, 2) = x(2); hx(1, 2) = y(2); hx(2, 2) = 1.0;
	hx(0, 3) = x(3); hx(1, 3) = y(3); hx(2, 3) = 1.0;

	X(0, 0) = 0; X(0, 1) = 1; X(0, 2) = 1; X(0, 3) = 0;
	X(1, 0) = 0; X(1, 1) = 0; X(1, 2) = 1; X(1, 3) = 1;
	X(2, 0) = 1; X(2, 1) = 1; X(2, 2) = 1; X(2, 3) = 1;

std::cout << "hx " << hx << std::endl;
std::cout << "X " << X << std::endl;

	CameraCalibration	cameraObject(in_I.size2(), in_I.size1());	//	本当はここで作る必要はない
																	//（引数も気持ち悪い．マントかする）

	//	Compute the planar collineation: (return the normalization matrix as well)
	cameraObject.computeHomography(hx, X, Homo);

std::cout << "Homo " << Homo << std::endl;

	//	Build the grid using the planar collineation:
	ublas::matrix<double, ublas::column_major>	x_l(in_n_sq_x + 1, in_n_sq_y + 1);
	ublas::matrix<double, ublas::column_major>	y_l(in_n_sq_x + 1, in_n_sq_y + 1);
	for (i = 0; i < (int )x_l.size1(); i++)
		for (j = 0; j < (int )x_l.size2(); j++)
		{
			x_l(i, j) = (double )i / (double )in_n_sq_x;
			y_l(i, j) = (double )j / (double )in_n_sq_y;
		}

	ublas::matrix<double, ublas::column_major>	pts(3, x_l.size1() * x_l.size2());
	for (i = 0; i < (int )pts.size2(); i++)
	{
		pts(0, i) = x_l(i % x_l.size1(), i / x_l.size1());
		pts(1, i) = y_l(i % x_l.size1(), i / x_l.size1());
		pts(2, i) = 1;
	}

	ublas::matrix<double, ublas::column_major>	XX(3, x_l.size1() * x_l.size2());
	XX = ublas::prod(Homo, pts);
	for (i = 0; i < (int )XX.size2(); i++)
	{
		XX(0, i) = XX(0, i) / XX(2, i);
		XX(1, i) = XX(1, i) / XX(2, i);
	}

	//	Complete size of the rectangle
	double	W = in_n_sq_x * in_dX;
	double	L = in_n_sq_y * in_dY;

//std::cout << "pts " << pts << std::endl;
//std::cout << "XX " << XX << std::endl;
//std::cout << "W " << W << std::endl;
//std::cout << "L " << L << std::endl;

	//	レンズひずみが無視できないときはここで処理を行う（要実装）


	int	Np = (in_n_sq_x + 1) * (in_n_sq_y + 1);
	double	grid_x, grid_y;
	bool	isSuccess;
	//ublas::matrix<double, ublas::column_major>	grid_pts(2, XX.size2());
	//ublas::vector<bool>	find_result(XX.size2());

	for (i = 0; i < Np; i++)
	{
		isSuccess = findCorner(in_I, XX(0, i), XX(1, i), in_wintx, in_winty, &grid_x, &grid_y);
		//	matlabのコードで行っている処理とあわせるために-1する
		//	subtract 1 to bring the origin to (0,0) instead of (1,1) 
		//	in matlab (not necessary in C)
		out_XX(0, i) = XX(0, i) - 1;
		out_XX(1, i) = XX(1, i) - 1;
		out_x(0, i) = grid_x - 1;
		out_x(1, i) = grid_y - 1;
		out_result(i) = isSuccess;
	}

	//	原点やX,Y軸を求める処理．外側でやるべきであろう
	//ind_corners = [1 n_sq_x+1 (n_sq_x+1)*n_sq_y+1 (n_sq_x+1)*(n_sq_y+1)]; % index of the 4 corners
	//ind_orig = (n_sq_x+1)*n_sq_y + 1;
	//xorig = grid_pts(1,ind_orig);
	//yorig = grid_pts(2,ind_orig);
	//dxpos = mean([grid_pts(:,ind_orig) grid_pts(:,ind_orig+1)]');
	//dypos = mean([grid_pts(:,ind_orig) grid_pts(:,ind_orig-n_sq_x-1)]');

	out_X.clear();
	for (i = 0; i < in_n_sq_y + 1; i++)
		for (j = 0; j < in_n_sq_x + 1; j++)
		{
			out_X(0, j + i * (in_n_sq_x + 1)) = j * in_dX;	
			out_X(1, j + i * (in_n_sq_x + 1)) = (in_n_sq_y - i) * in_dY;
			out_X(2, j + i * (in_n_sq_x + 1)) = 0.0;	//	Z Axis is always 0
		}
}



// -----------------------------------------------------------------------------
//	findRectangle
// -----------------------------------------------------------------------------
//
//	入力・出力ともにMatlabの座標系　原点が（1, 1）
//
void	CornerFinder::findRectangle(
		const ublas::matrix<unsigned char, ublas::column_major> &in_I,
		ublas::matrix<double, ublas::column_major> &io_x,
		int	in_wintx, int in_winty,
		int	*out_n_sq_x1, int *out_n_sq_x2,
		int	*out_n_sq_y1, int *out_n_sq_y2)
{
	int	i, j;
	int	n = io_x.size2();

	ublas::vector<double>	x(n);
	ublas::vector<double>	y(n);

	for (i = 0; i < n; i++)
	{
		//x(i) = io_x(0, i);
		//y(i) = io_x(1, i);

		findCorner(in_I,
			io_x(0, i), io_x(1, i),
			in_wintx, in_winty,
			&(x(i)), &(y(i)));
	}

	//	Sort the corners:
	double	x_mean = mat_mean(x);
	double	y_mean = mat_mean(y);

	ublas::vector<double>	x_v(n);
	ublas::vector<double>	y_v(n);
	ublas::vector<double>	theta(n);
	for (i = 0; i < n; i++)
	{
		x_v(i) = x(i) - x_mean;
		y_v(i) = y(i) - y_mean;
		theta(i) = atan2(-1.0 * y_v(i),x_v(i));
	}
//std::cout << "x_v " << x_v << std::endl;
//std::cout << "y_v " << y_v << std::endl;
//std::cout << "theta " << theta << std::endl;

	//mat_sort(theta); MATLABのコードにあるけど意味のない操作なのでスキップ
	double	temp = theta(0), temp_x, temp_y;
	for (i = 0; i < n; i++)
	{
		temp_x = theta(i) - temp;
		temp_y = 2 * MAT_PI;
		theta(i) = temp_x - temp_y * floor(temp_x / temp_y);	// emulate matlabs's mod
	}

	//インデックスつきのソートを行う
	ublas::vector<double>	theta_original(n);
	ublas::vector<int>	index(n);
	theta_original = theta;
	mat_sort(theta);
	for (i = 0; i < n; i++)
		for (j = 0; j < n; j++)
			if (theta(i) == theta_original(j))
				index(i) = j;

//std::cout << "theta_original " << theta_original << std::endl;
//std::cout << "index " << index << std::endl;

	//	the Z axis is pointing uppward
	ublas::vector<int>	ind(4);
	for (i = 0; i < 4; i++)
		ind(i) = index(3 - i);

	double	x1 = x(ind(0));
	double	x2 = x(ind(1));
	double	x3 = x(ind(2));
	double	x4 = x(ind(3));
	double	y1 = y(ind(0));
	double	y2 = y(ind(1));
	double	y3 = y(ind(2));
	double	y4 = y(ind(3));

	//	Find center:
	ublas::vector<double>	vec_a(3);
	ublas::vector<double>	vec_b(3);
	ublas::vector<double>	vec_c(3);
	ublas::vector<double>	vec_d(3);
	ublas::vector<double>	vec_e(3);
	ublas::vector<double>	vec_f(3);
	ublas::vector<double>	p_center(3);

	vec_a(0) = x1; vec_a(1) = y1; vec_a(2) = 1.0;
	vec_b(0) = x3; vec_b(1) = y3; vec_b(2) = 1.0;	
	mat_cross(vec_a, vec_b, vec_c);

	vec_d(0) = x2; vec_d(1) = y2; vec_d(2) = 1.0;
	vec_e(0) = x4; vec_e(1) = y4; vec_e(2) = 1.0;	
	mat_cross(vec_d, vec_e, vec_f);

	mat_cross(vec_c, vec_f, p_center);

	double	x5 = p_center(0) / p_center(2);
	double	y5 = p_center(1) / p_center(2);

	//	center on the X axis:
	double	x6 = (x3 + x4) / 2;
	double	y6 = (y3 + y4) / 2;

	//	center on the Y axis:
	double	x7 = (x1 + x4) / 2;
	double	y7 = (y1 + y4) / 2;

	//	Direction of displacement for the X axis:
	ublas::vector<double>	vX(2);
	vX(0) = x6 - x5; vX(1) = y6 - y5;
	vX = vX / mat_norm(vX);

	//	Direction of displacement for the Y axis:
	ublas::vector<double>	vY(2);
	vY(0) = x7 - x5; vY(1) = y7 - y5;
	vY = vY / mat_norm(vY);

	//	Direction of diagonal:
	ublas::vector<double>	vO(2);
	vO(0) = x4 - x5; vO(1) = y4 - y5;
	vO = vO / mat_norm(vO);

	double	delta = 30;

/*std::cout << "x1 " << x1 << std::endl;
std::cout << "x2 " << x2 << std::endl;
std::cout << "x3 " << x3 << std::endl;
std::cout << "x4 " << x4 << std::endl;
std::cout << "x5 " << x5 << std::endl;
std::cout << "x6 " << x6 << std::endl;
std::cout << "x7 " << x7 << std::endl;
std::cout << "y1 " << y1 << std::endl;
std::cout << "y2 " << y2 << std::endl;
std::cout << "y3 " << y3 << std::endl;
std::cout << "y4 " << y4 << std::endl;
std::cout << "y5 " << y5 << std::endl;
std::cout << "y6 " << y6 << std::endl;
std::cout << "y7 " << y7 << std::endl;
std::cout << "vX " << vX << std::endl;
std::cout << "vY " << vY << std::endl;
std::cout << "vO " << vO << std::endl;
std::cout << "p_center" << p_center << std::endl;*/

	*out_n_sq_x1 = count_squares(in_I, x1, y1, x2, y2, in_wintx);
	*out_n_sq_x2 = count_squares(in_I, x3, y3, x4, y4, in_wintx);
	*out_n_sq_y1 = count_squares(in_I, x2, y2, x3, y3, in_wintx);
	*out_n_sq_y2 = count_squares(in_I, x4, y4, x1, y1, in_wintx);

std::cout << "out_n_sq_x1 " << *out_n_sq_x1 << std::endl;
std::cout << "out_n_sq_x2 " << *out_n_sq_x2 << std::endl;
std::cout << "out_n_sq_y1 " << *out_n_sq_y1 << std::endl;
std::cout << "out_n_sq_y2 " << *out_n_sq_y2 << std::endl;

	for (i = 0; i < 4; i++)
	{
		io_x(0, i) = x(ind(i));
		io_x(1, i) = y(ind(i));
	}
}


// -----------------------------------------------------------------------------
//	count_squares
// -----------------------------------------------------------------------------
//
//	入力・出力ともにMatlabの座標系　原点が（1, 1）
//
int	CornerFinder::count_squares(
		const ublas::matrix<unsigned char, ublas::column_major> &in_I,
		double in_x1, double in_y1,
		double in_x2, double in_y2,
		int	in_win)
{
	int	i, j;
	int	ny = in_I.size1();
	int	nx = in_I.size2();

	ublas::vector<double>	lamda(3);
	lamda(0) = in_y1 - in_y2;
	lamda(1) = in_x2 - in_x1;
	lamda(2) = in_x1 * in_y2 - in_x2 * in_y1;

	double t = sqrt(lamda(0) * lamda(0) + lamda(1) * lamda(1));
	for (i = 0; i < 3; i++)
		lamda(i) = 1.0 / t * lamda(i);

	ublas::vector<double>	l1(3);
	ublas::vector<double>	l2(3);
	l1 = lamda;
	l1(2) += in_win;
	l2 = lamda;
	l2(2) -= in_win;

	double	dx = in_x2 - in_x1;
	double	dy = in_y2 - in_y1;

	ublas::vector<double>	xs;
	ublas::vector<double>	ys;
	if (fabs(dx) > fabs(dy))
	{
		xs = ublas::vector<double>(floor(fabs(dx)) + 1);
		ys = ublas::vector<double>(floor(fabs(dx)) + 1);

		if (in_x2 > in_x1)
		{
			for (i = 0, t = in_x1; i < (int )xs.size(); i++, t++) 
				xs(i) = t;
		}
		else
		{
			for (i = 0, t = in_x1; i < (int )xs.size(); i++, t--) 
				xs(i) = t;
		}

		for (i = 0; i < (int )ys.size(); i++) 
			ys(i) = -1.0 * (lamda(2) + lamda(0) * xs(i)) / lamda(1);
	}
	else
	{
		xs = ublas::vector<double>(floor(fabs(dy)) + 1);
		ys = ublas::vector<double>(floor(fabs(dy)) + 1);

		if (in_y2 > in_y1)
		{
			for (i = 0, t = in_y1; i < (int )ys.size(); i++, t++) 
				ys(i) = t;
		}
		else
		{
			for (i = 0, t = in_y1; i < (int )ys.size(); i++, t--) 
				ys(i) = t;
		}

		for (i = 0; i < (int )xs.size(); i++) 
			xs(i) = -1.0 * (lamda(2) + lamda(1) * ys(i)) / lamda(0);
	}

//std::cout << "lamda " << lamda << std::endl;
//std::cout << "xs " << xs << std::endl;
//std::cout << "ys " << ys << std::endl;

	int	Np = xs.size();

	ublas::matrix<double, ublas::column_major>	xs_mat(2 * in_win + 1, Np);
	ublas::matrix<double, ublas::column_major>	ys_mat(2 * in_win + 1, Np);
	for (i = 0; i < (int )xs_mat.size1(); i++)
		for (j = 0; j < (int )xs_mat.size2(); j++)
		{
			xs_mat(i, j) = xs(j);
			ys_mat(i, j) = ys(j);
		}

	ublas::matrix<double, ublas::column_major>	win_mat(2 * in_win + 1, Np);
	for (i = 0, t = -in_win; i < (int )win_mat.size1(); i++, t++)
		for (j = 0; j < (int )win_mat.size2(); j++)
			win_mat(i, j) = t;

	ublas::matrix<int, ublas::column_major>	xs_mat2(2 * in_win + 1, Np);
	ublas::matrix<int, ublas::column_major>	ys_mat2(2 * in_win + 1, Np);
	for (i = 0; i < (int )xs_mat2.size1(); i++)
		for (j = 0; j < (int )xs_mat2.size2(); j++)
		{
			xs_mat2(i, j) = mat_round(xs_mat(i, j) - win_mat(i, j) * lamda(0));
			ys_mat2(i, j) = mat_round(ys_mat(i, j) - win_mat(i, j) * lamda(1));
		}

	ublas::matrix<int, ublas::column_major>	ind_mat(2 * in_win + 1, Np);
	for (i = 0; i < (int )ind_mat.size1(); i++)
		for (j = 0; j < (int )ind_mat.size2(); j++)
			ind_mat(i, j) = (xs_mat2(i, j) - 1) * ny + ys_mat2(i, j);

//std::cout << "ind_mat " << ind_mat << std::endl;

	ublas::matrix<double, ublas::column_major>	ima_patch(2 * in_win + 1, Np);
	ima_patch.clear();
	for (i = 0; i < (int )ind_mat.size1(); i++)
		for (j = 0; j < (int )ind_mat.size2(); j++)
		{
			int	index = ind_mat(i, j) - 1;	// -1はMATLABとの差(1次元アクセスなので少しややこしい)
			int	i_index = index % (int )in_I.size1();
			int	j_index = index / (int )in_I.size1();
			if (j_index < (int )in_I.size2())
				ima_patch(i, j) = in_I(i_index, j_index);
			else
				ima_patch(i, j) = 0;
		}

//std::cout << "ima_patch " << ima_patch << std::endl;

	ublas::matrix<double, ublas::column_major>	filtk(2 * in_win + 1, Np);
	filtk.clear();
	for (i = 0; i < in_win; i++)
		for (j = 0; j < (int )filtk.size2(); j++)
		{
			filtk(i, j) = 1;
			filtk(i + in_win + 1, j) = -1;
		}

	ublas::matrix<double, ublas::column_major>	out_f_mat(1, Np);
	ublas::matrix_row<ublas::matrix<double, ublas::column_major> >	out_f(out_f_mat, 0);

	for (i = 0; i < (int )ind_mat.size1(); i++)
		for (j = 0; j < (int )ind_mat.size2(); j++)
			ima_patch(i, j) = filtk(i, j) * ima_patch(i, j);

	ublas::vector<double>	temp_vec(Np);		//	アドホックな解決方法．何とかする
	mat_sum(ima_patch, temp_vec);
	out_f = temp_vec;

//std::cout << "out_f_mat " << out_f_mat << std::endl;

	ublas::matrix<double, ublas::column_major>	out_f_f(1, Np);
	ublas::matrix<double, ublas::column_major>	filter(1, 3);
	filter(0, 0) = 1 / 4.0; filter(0, 1) = 1 / 2.0; filter(0, 2) = 1 / 4.0;
	mat_conv2_same(out_f_mat, filter, out_f_f);

//std::cout << "filter " << filter << std::endl;
//std::cout << "out_f_f " << out_f_f << std::endl;

	ublas::vector<double>	out_f_f2(Np - 2 * in_win);
	for (i = 0; i < (int )out_f_f2.size(); i++)
		out_f_f2(i) = out_f_f(0, i + in_win);

	int	result = 1;
	for (i = 0; i < (int )out_f_f2.size() - 1; i++)
		if ((out_f_f2(i + 1) >= 0 && out_f_f2(i) < 0) ||
			(out_f_f2(i + 1) <= 0 && out_f_f2(i) > 0))
		{
			result++;
		}

//std::cout << "out_f_f2 " << out_f_f2 << std::endl;
//std::cout << "result " << result << std::endl;

	return result;
}


// -----------------------------------------------------------------------------
//	findCorner
// -----------------------------------------------------------------------------
//
//	入力・出力ともにMatlabの座標系　原点が（1, 1）
//
bool	CornerFinder::findCorner(
		const ublas::matrix<unsigned char, ublas::column_major> &in_I,
		double in_x, double in_y,
		int	in_wintx, int in_winty,
		double *out_x, double *out_y)
{
	//int wx2 = -1;	//	元のコードでは引数でも指定可．だだし使用していない
	//int wy2 = -1;

	ublas::vector<double>	xt(2);
	xt(0) = in_y;	//	x,yを反転させているので要注意
	xt(1) = in_x;	//	in_Iの行（縦方向）がx軸として定義されているため

	int	i, j;
	int	a, b;
	ublas::matrix<double, ublas::column_major>	mask_a(in_wintx * 2 + 1, 1);
	ublas::matrix<double, ublas::column_major>	mask_b(1, in_winty * 2 + 1);

	for (i = 0, a = -in_wintx; i < (int )mask_a.size1(); i++, a++)
		mask_a(i, 0) = exp(-1.0 * pow((double )a / (double )in_wintx, 2));
	for (i = 0, a = -in_winty; i < (int )mask_b.size2(); i++, a++)
		mask_b(0, i) = exp(-1.0 * pow((double )a / (double )in_winty, 2));

	ublas::matrix<double, ublas::column_major>	mask(mask_a.size1(), mask_b.size2());
	mask = ublas::prod(mask_a, mask_b);

	ublas::matrix<double, ublas::column_major>	offx(mask_a.size1(), mask_b.size2());
	ublas::matrix<double, ublas::column_major>	offy(mask_a.size1(), mask_b.size2());

	for (i = 0, a = -in_wintx; i < (int )offx.size1(); i++, a++)
		for (j = 0, b = -in_winty; j < (int )offx.size2(); j++, b++)
		{
			offx(i, j) = a;
			offy(i, j) = b;
		}

//std::cout << "offx" << offx << std::endl;
//std::cout << "offy" << offy << std::endl;

	int	nx = in_I.size1();
	int	ny = in_I.size2();

	//	first guess... they don't move !!!
	ublas::vector<double>	xc(2);
	xc = xt;

	ublas::vector<double>	v_extra(2);
	v_extra(0) = 1 + CORNER_FINDER_RESOLUTION;
	v_extra(1) = 1 + CORNER_FINDER_RESOLUTION;

	int	compt = 0;

	while (mat_norm(v_extra) > CORNER_FINDER_RESOLUTION &&
			compt < CORNER_FINDER_ITER_MAX)
	{
		double	cIx = xc(0);
		double	cIy = xc(1);
		double	crIx = mat_round(cIx);
		double	crIy = mat_round(cIy);
		double	itIx = cIx - crIx;
		double	itIy = cIy - crIy;
		
		ublas::matrix<double, ublas::column_major>	vIx(3, 1);
		ublas::matrix<double, ublas::column_major>	vIy(1, 3);

		if (itIx > 0.0)	// the sub pixel
		{
			vIx(0, 0) = itIx;
			vIx(1, 0) = 1 - itIx;
			vIx(2, 0) = 0;
		}
		else
		{
			vIx(0, 0) = 0;
			vIx(1, 0) = 1 + itIx;
			vIx(2, 0) = -itIx;
		}
		if (itIy > 0.0)	// the sub pixel
		{
			vIy(0, 0) = itIy;
			vIy(0, 1) = 1 - itIy;
			vIy(0, 2) = 0;
		}
		else
		{
			vIy(0, 0) = 0;
			vIy(0, 1) = 1 + itIy;
			vIy(0, 2) = -itIy;
		}

/*std::cout << "cIx" << cIx << std::endl;
std::cout << "cIy" << cIy << std::endl;
std::cout << "crIx" << crIx << std::endl;
std::cout << "crIy" << crIy << std::endl;
std::cout << "vIx" << vIx << std::endl;
std::cout << "vIy" << vIy << std::endl;*/

		//	What if the sub image is not in?
		int	xmin, xmax;
		if (crIx - in_wintx - 2 < 1)
		{
			xmin = 1;
			xmax = 2 * in_wintx + 5;
		}
		else if (crIx + in_wintx + 2 > nx)
		{
			xmax = nx;
			xmin = nx - 2 * in_wintx - 4;
		}
		else
		{
			xmin = crIx - in_wintx - 2;
			xmax = crIx + in_wintx + 2;
		}

		int	ymin, ymax;
		if (crIy - in_winty - 2 < 1)
		{
			ymin = 1;
			ymax = 2 * in_winty + 5;
		}
		else if (crIy + in_winty + 2 > ny)
		{
			ymax = ny;
			ymin = ny - 2 * in_winty - 4;
		}
		else
		{
			ymin = crIy - in_winty - 2;
			ymax = crIy + in_winty + 2;
		}

/*std::cout << "xmin" << xmin << std::endl;
std::cout << "xmax" << xmax << std::endl;
std::cout << "ymin" << ymin << std::endl;
std::cout << "ymax" << ymax << std::endl;*/

		//	matlab indexからの変換
		//	この処理があるため，この関数内の処理がすべてmatlabの座標系で進められる
		xmin--; xmax--;
		ymin--; ymax--;	

		//	The necessary neighborhood
		ublas::matrix<double, ublas::column_major>	SI(xmax - xmin + 1, ymax - ymin + 1);
		for (i = 0, a = xmin; i < (int )SI.size1(); i++, a++)
			for (j = 0, b = ymin; j < (int )SI.size2(); j++, b++)
				SI(i, j) = in_I(a, b);

//std::cout << "SI" << SI << std::endl;
//std::cout << "vIx" << vIx << std::endl;

		ublas::matrix<double, ublas::column_major>	temp_mat(SI.size1(), SI.size2());
		mat_conv2_same(SI, vIx, temp_mat);
//std::cout << "temp_mat" << temp_mat << std::endl;
		mat_conv2_same(temp_mat, vIy, SI);

//std::cout << "vIy" << vIy << std::endl;
//std::cout << "SI" << SI << std::endl;

		//	The subpixel interpolated neighborhood
		ublas::matrix_range<ublas::matrix<double, ublas::column_major> >
						SI_r(SI,	ublas::range(1, 2 * in_wintx + 4),
									ublas::range(1, 2 * in_winty + 4));

//std::cout << "SI_r" << SI_r << std::endl;

		ublas::matrix<double, ublas::column_major>	gx(SI_r.size1(), SI_r.size2());
		ublas::matrix<double, ublas::column_major>	gy(SI_r.size1(), SI_r.size2());
		mat_gradient(SI_r, gy, gx);

		//	extraction of the useful parts only of the gradients
		ublas::matrix_range<ublas::matrix<double, ublas::column_major> >
						gx_r(gx,	ublas::range(1, 2 * in_wintx + 2),
									ublas::range(1, 2 * in_winty + 2));
		ublas::matrix_range<ublas::matrix<double, ublas::column_major> >
						gy_r(gy,	ublas::range(1, 2 * in_wintx + 2),
									ublas::range(1, 2 * in_winty + 2));

		ublas::matrix<double, ublas::column_major>	px(mask_a.size1(), mask_b.size2());
		ublas::matrix<double, ublas::column_major>	py(mask_a.size1(), mask_b.size2());

		for (i = 0; i < (int )px.size1(); i++)
			for (j = 0; j < (int )px.size2(); j++)
			{
				px(i, j) = cIx + offx(i, j);
				py(i, j) = cIy + offy(i, j);
			}

		ublas::matrix<double, ublas::column_major>	gxx(gx_r.size1(), gx_r.size2());
		ublas::matrix<double, ublas::column_major>	gyy(gy_r.size1(), gy_r.size2());
		ublas::matrix<double, ublas::column_major>	gxy(gx_r.size1(), gx_r.size2());

		for (i = 0; i < (int )mask.size1(); i++)
			for (j = 0; j < (int )mask.size2(); j++)
			{
				gxx(i, j) = gx_r(i, j) * gx_r(i, j) * mask(i, j);
				gyy(i, j) = gy_r(i, j) * gy_r(i, j) * mask(i, j);
				gxy(i, j) = gx_r(i, j) * gy_r(i, j) * mask(i, j);
			}

		ublas::matrix<double, ublas::column_major>	gxx_temp(gx_r.size1(), gx_r.size2());
		ublas::matrix<double, ublas::column_major>	gxy_temp(gy_r.size1(), gy_r.size2());

		for (i = 0; i < (int )mask.size1(); i++)
			for (j = 0; j < (int )mask.size2(); j++)
			{
				gxx_temp(i, j) = gxx(i, j) * px(i, j) + gxy(i, j) * py(i, j);
				gxy_temp(i, j) = gxy(i, j) * px(i, j) + gyy(i, j) * py(i, j);
			}

		ublas::vector<double>	temp_vec(gxx.size2());

		mat_sum(gxx_temp, temp_vec);
		double	bb_1 = mat_sum(temp_vec);
		mat_sum(gxy_temp, temp_vec);
		double	bb_2 = mat_sum(temp_vec);

		mat_sum(gxx, temp_vec);
		double	a = mat_sum(temp_vec);
		mat_sum(gxy, temp_vec);
		double	b = mat_sum(temp_vec);
		mat_sum(gyy, temp_vec);
		double	c = mat_sum(temp_vec);

		double	dt = a * c - b * b;

		ublas::vector<double>	xc2(2);
		xc2(0) = (c * bb_1 - b * bb_2) / dt;
		xc2(1) = (a * bb_2 - b * bb_1) / dt;

		ublas::matrix<double, ublas::column_major>	G(2, 2);
		ublas::matrix<double, ublas::column_major>	U(2, 2);
		ublas::matrix<double, ublas::column_major>	S(2, 2);
		ublas::matrix<double, ublas::column_major>	V(2, 2);

		G(0, 0) = a; G(0, 1) = b;
		G(1, 0) = b; G(1, 1) = c;

		mat_svd(G, U, S, V);

		//	If non-invertible, then project the point onto the edge orthogonal:
		if (S(0, 0) / S(1, 1) > 50.0)
		{
			//	projection operation:
			double	t = (xc(0) - xc2(0)) * V(0, 1) + (xc(1) - xc2(1)) * V(1, 1);

			xc2(0) = xc2(0) + t * V(0, 1);
			xc2(1) = xc2(1) + t * V(1, 1);
			//type(i) = 1;	使っていないようなので無視
//std::cout << "Warning:!! NO TEST PATH (CornerFinder)" << std::endl;
		}

		v_extra = xc - xc2;
		xc = xc2;
		compt++;

//std::cout << "compt" << compt << std::endl;
//std::cout << "v_extra" << v_extra << std::endl;
//std::cout << "norm(v_extra)" << mat_norm(v_extra) << std::endl;

	}

	bool	result = true;

	if (fabs(xc(0) - xt(0)) > in_wintx || fabs(xc(1) - xt(1)) > in_winty)
	{
//std::cout << "can't detect corner" << std::endl;
		xc = xt;
		result = false;
	}

	*out_x = xc(1);	//	反転しているので注意
	*out_y = xc(0);
	return result;
}

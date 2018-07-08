// =============================================================================
//	CameraCalibration.hpp

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
#ifndef __CAMERA_CALIBRATION_HPP
#define __CAMERA_CALIBRATION_HPP


// -----------------------------------------------------------------------------
// 	include files
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// 	macros
// -----------------------------------------------------------------------------
#define	MAT_PI	3.14159265

// -----------------------------------------------------------------------------
// 	CameraCalibration class
// -----------------------------------------------------------------------------
class	CameraCalibration
{
public:
	//	constructor/destructor
							CameraCalibration(int inImageWidth, int inImageHeight);
	virtual					~CameraCalibration();

	//	member functions
	void					ClearMesurementData();
	void					AddMesurementData(	const ublas::matrix<double, ublas::column_major> &m,
												const ublas::matrix<double, ublas::column_major> &M);

	virtual void			DoCalibration();
	virtual void			CancelCalibrationProcess();
	virtual void			DumpResults();


//protected:
	//	member variables
	double	mImageWidth;	// doubleÇ…ÇµÇ»Ç¢Ç∆åvéZÇ™âˆÇµÇ¢Ç∆Ç±ÇÎÇ™Ç†ÇÈÇΩÇﬂÅidoubleÇ÷ÇÃÉLÉÉÉXÉgñYÇÍñhé~Åj
	double	mImageHeight;

	std::vector<ublas::matrix<double, ublas::column_major> >	x_list;
	std::vector<ublas::matrix<double, ublas::column_major> >	X_list;

	std::vector<ublas::matrix<double, ublas::column_major> >	H_list;

	std::vector<ublas::matrix<double, ublas::column_major> >	omc_list;
	std::vector<ublas::matrix<double, ublas::column_major> >	Tc_list;
	std::vector<ublas::matrix<double, ublas::column_major> >	Rc_list;

	std::vector<ublas::matrix<double, ublas::column_major> >	y_list;
	std::vector<ublas::matrix<double, ublas::column_major> >	ex_list;

	ublas::vector<double>	fc;
	ublas::vector<double>	cc;
	ublas::vector<double>	kc;
	double					alpha_c;

	ublas::vector<double>	err_std;
	ublas::vector<double>	fc_error;
	ublas::vector<double>	cc_error;
	ublas::vector<double>	kc_error;
	double					alpha_c_error;

	double					thresh_cond;

	ublas::matrix<double, ublas::column_major>	KK;

	ublas::matrix<double, ublas::column_major>	N_points_views;

	//	member functions
	void					computeHomography(
										const ublas::matrix<double, ublas::column_major> &x,
										const ublas::matrix<double, ublas::column_major> &X,
										ublas::matrix<double, ublas::column_major> &H);
	void					computeIntrisicParam();

	void					computeExtrinsicParam();
	void					computeExtrinsicInit(
										const ublas::matrix<double, ublas::column_major> &x,
										const ublas::matrix<double, ublas::column_major> &X,
										ublas::matrix<double, ublas::column_major> &omckk,
										ublas::matrix<double, ublas::column_major> &Tckk,
										ublas::matrix<double, ublas::column_major> &Rckk);
	void					computeExtrinsicRefine(
										const ublas::matrix<double, ublas::column_major> &x,
										const ublas::matrix<double, ublas::column_major> &X,
										ublas::matrix<double, ublas::column_major> &omckk,
										ublas::matrix<double, ublas::column_major> &Tckk,
										ublas::matrix<double, ublas::column_major> &Rckk,
										ublas::matrix<double, ublas::column_major> &JJ_kk);
	void					mainOptimization();

	static void				project_points2(
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
										ublas::matrix<double, ublas::column_major> &out_dxpdalpha);
	static void				rigid_motion(
										const ublas::matrix<double, ublas::column_major> &in_X,
										const ublas::matrix<double, ublas::column_major> &in_om,
										const ublas::matrix<double, ublas::column_major> &in_T,
										ublas::matrix<double, ublas::column_major> &out_Y,
										ublas::matrix<double, ublas::column_major> &out_dYdom,
										ublas::matrix<double, ublas::column_major> &out_dYdT);
	static void				normalize_pixel(
										const ublas::vector<double> &in_fc,
										const ublas::vector<double> &in_cc,
										const ublas::vector<double> &in_kc,
										double	in_alpha_c,
										const ublas::matrix<double, ublas::column_major> &in_x,
										ublas::matrix<double, ublas::column_major> &out_x);
	static void				comp_distortion_oulu(
										const ublas::vector<double> &in_kc,
										const ublas::matrix<double, ublas::column_major> &in_x,
										ublas::matrix<double, ublas::column_major> &out_x);
	static void				rodrigues(
									 const ublas::matrix<double, ublas::column_major> &in_mat,
									 ublas::matrix<double, ublas::column_major> &out_mat,
									 ublas::matrix<double, ublas::column_major> &out_jacobian);

	static void				rect_index(
										int nc, int nr,
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
										ublas::vector<int> &ind_1,
										ublas::vector<int> &ind_2,
										ublas::vector<int> &ind_3,
										ublas::vector<int> &ind_4);
	static void				rectify_image(
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
										unsigned char *outImage);

	static void				apply_distortion(
									 const ublas::matrix<double, ublas::column_major> &x,
									 const ublas::vector<double> &k,
									 ublas::matrix<double, ublas::column_major> &out_xd);

//private:
	int						getImageNum() { return (int )H_list.size(); };

	static void				mat_inv(ublas::matrix<double, ublas::column_major> &io_mat);
	static double			mat_det(const ublas::matrix<double, ublas::column_major> &in_mat);
	static double			mat_trace(const ublas::matrix<double, ublas::column_major> &in_mat);

	static double			mat_cond(const ublas::matrix<double, ublas::column_major> &in_mat);
	static void				mat_repmat(
								const ublas::matrix<double, ublas::column_major> &in_mat,
								ublas::matrix<double, ublas::column_major> &out_mat,
								int in_m, int in_n);

	static void				mat_cross(
										const ublas::vector<double> &in_vec_A,
										const ublas::vector<double> &in_vec_B,
										ublas::vector<double> &out_vec_C);

	static double			mat_norm(const ublas::matrix<double, ublas::column_major> &in_mat);
	static double			mat_norm(const ublas::vector<double> &in_vec);

	static void				mat_svd(
								const ublas::matrix<double, ublas::column_major> &in_mat,
								ublas::matrix<double, ublas::column_major> &out_U,
								ublas::matrix<double, ublas::column_major> &out_S,
								ublas::matrix<double, ublas::column_major> &out_V);

	static double			mat_mean(const ublas::vector<double> &in_vec);
	static void				mat_mean_dim(
								const ublas::matrix<double, ublas::column_major> &in_mat,
								ublas::matrix<double, ublas::column_major> &out_mat,
								int inDimension);

	static void				mat_sort(ublas::vector<double> &io_vec);
	static void				mat_sort(ublas::matrix<double, ublas::column_major> &io_mat, int inDimension);

	static void				mat_median(
								const ublas::matrix<double, ublas::column_major> &in_mat,
								ublas::matrix<double, ublas::column_major> &out_mat,
								int inDimension);
	static double			mat_std(const ublas::vector<double> &in_vec);

	static double			mat_round(double in_value);

	static void				mat_conv2_same(
								const ublas::matrix<double, ublas::column_major> &in_A,
								const ublas::matrix<double, ublas::column_major> &in_B,
								ublas::matrix<double, ublas::column_major> &out_mat);

	static void				mat_gradient(
								const ublas::matrix<double, ublas::column_major> &in_mat,
								ublas::matrix<double, ublas::column_major> &out_x,
								ublas::matrix<double, ublas::column_major> &out_y);

	static double			mat_sum(const ublas::vector<double> &in_vec);
	static void				mat_sum(
								const ublas::matrix<double, ublas::column_major> &in_mat,
								ublas::vector<double> &out_vec);

	static double			_mat_median_sub(const ublas::vector<double> &in_vec);
};


#endif	// #ifdef __CAMERA_CALIBRATION_HPP

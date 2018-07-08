// =============================================================================
//  StereoCalibration.hpp
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
	\file		CameraCalibration.hpp
	\author		Dairoku Sekiguchi
	\version	1.0
	\date		2007/04/05
	\brief		This file is a part of CalibraKernel
	
	This is a direct C++ port of "Camera Calibration Toolbox for Matlab"
	by Jean-Yves Bouguet.
	Original source can be found at:
	http://www.vision.caltech.edu/bouguetj/calib_doc/
*/

#ifndef __STEREO_CALIBRATION_HPP
#define __STEREO_CALIBRATION_HPP


// -----------------------------------------------------------------------------
// 	include files
// -----------------------------------------------------------------------------
#include "CameraCalibration.hpp"


// -----------------------------------------------------------------------------
// 	StereoCalibration class
// -----------------------------------------------------------------------------
class	StereoCalibration : public CameraCalibration
{
public:
	//	constructor/destructor
							StereoCalibration(int inImageWidth, int inImageHeight);
	virtual					~StereoCalibration();

	//	member functions
	virtual void			DoCalibration();
	virtual void			DumpResults();

	void					CalcRectifyIndex();

	//	member variables
	std::vector<ublas::matrix<double, ublas::column_major> >	X_left_list;
	std::vector<ublas::matrix<double, ublas::column_major> >	x_left_list;
	std::vector<ublas::matrix<double, ublas::column_major> >	omc_left_list;
//	std::vector<ublas::matrix<double, ublas::column_major> >	Rc_left_list;
	std::vector<ublas::matrix<double, ublas::column_major> >	Tc_left_list;

	std::vector<ublas::matrix<double, ublas::column_major> >	X_right_list;
	std::vector<ublas::matrix<double, ublas::column_major> >	x_right_list;
	std::vector<ublas::matrix<double, ublas::column_major> >	omc_right_list;
//	std::vector<ublas::matrix<double, ublas::column_major> >	Rc_right_list;
	std::vector<ublas::matrix<double, ublas::column_major> >	Tc_right_list;

//	std::vector<ublas::matrix<double, ublas::column_major> >	y_list;
//	std::vector<ublas::matrix<double, ublas::column_major> >	ex_list;

	ublas::vector<double>	fc_left;
	ublas::vector<double>	cc_left;
	ublas::vector<double>	kc_left;
	double					alpha_c_left;

	ublas::vector<double>	fc_right;
	ublas::vector<double>	cc_right;
	ublas::vector<double>	kc_right;
	double					alpha_c_right;

	ublas::matrix<double, ublas::column_major>	T;
	ublas::matrix<double, ublas::column_major>	om;
	ublas::matrix<double, ublas::column_major>	R;

	ublas::vector<double>	fc_left_error;
	ublas::vector<double>	cc_left_error;
	ublas::vector<double>	kc_left_error;
	double					alpha_c_left_error;

	ublas::vector<double>	fc_right_error;
	ublas::vector<double>	cc_right_error;
	ublas::vector<double>	kc_right_error;
	double					alpha_c_right_error;

	ublas::matrix<double, ublas::column_major>	T_error;
	ublas::matrix<double, ublas::column_major>	om_error;

	ublas::vector<double>	a1_left;
	ublas::vector<double>	a2_left;
	ublas::vector<double>	a3_left;
	ublas::vector<double>	a4_left;

	ublas::vector<int>		ind_new_left;
	ublas::vector<int>		ind_1_left;
	ublas::vector<int>		ind_2_left;
	ublas::vector<int>		ind_3_left;
	ublas::vector<int>		ind_4_left;

	ublas::vector<double>	a1_right;
	ublas::vector<double>	a2_right;
	ublas::vector<double>	a3_right;
	ublas::vector<double>	a4_right;

	ublas::vector<int>		ind_new_right;
	ublas::vector<int>		ind_1_right;
	ublas::vector<int>		ind_2_right;
	ublas::vector<int>		ind_3_right;
	ublas::vector<int>		ind_4_right;


protected:
	void					mainOptimization();

	static void				compose_motion(
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
								ublas::matrix<double, ublas::column_major> &out_dT3dT2);

	static void				dAB(
								const ublas::matrix<double, ublas::column_major> &in_A,
								const ublas::matrix<double, ublas::column_major> &in_B,
								ublas::matrix<double, ublas::column_major> &out_dABdA,
								ublas::matrix<double, ublas::column_major> &out_dABdB);

};


#endif	// #ifdef __STEREO_CALIBRATION_HPP

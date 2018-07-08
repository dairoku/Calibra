// =============================================================================
//  MultiCameraCalibration.hpp
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
	\file		MultiCameraCalibration.hpp
	\author		Dairoku Sekiguchi
	\version	1.0
	\date		2007/04/05
	\brief		This file is a part of CalibraKernel
	
	This is a direct C++ port of "Camera Calibration Toolbox for Matlab"
	by Jean-Yves Bouguet.
	Original source can be found at:
	http://www.vision.caltech.edu/bouguetj/calib_doc/
*/

#ifndef __MULTI_CAMERA_CALIBRATION_HPP
#define __MULTI_CAMERA_CALIBRATION_HPP


// -----------------------------------------------------------------------------
// 	include files
// -----------------------------------------------------------------------------
#include "CameraCalibration.hpp"
#include "StereoCalibration.hpp"


// -----------------------------------------------------------------------------
// 	SingleCameraResult class
// -----------------------------------------------------------------------------
class	SingleCameraResult
{
public:
	//	member variables
	std::vector<ublas::matrix<double, ublas::column_major> >	X_list;
	std::vector<ublas::matrix<double, ublas::column_major> >	x_list;
	std::vector<ublas::matrix<double, ublas::column_major> >	omc_list;
	std::vector<ublas::matrix<double, ublas::column_major> >	Tc_list;

	ublas::vector<double>	fc;
	ublas::vector<double>	cc;
	ublas::vector<double>	kc;
	double					alpha_c;
};


// -----------------------------------------------------------------------------
// 	MultiCameraCalibration class
// -----------------------------------------------------------------------------
class	MultiCameraCalibration : public CameraCalibration
{
public:
	//	constructor/destructor
							MultiCameraCalibration(int inImageWidth, int inImageHeight);
	virtual					~MultiCameraCalibration();

	//	member functions
	virtual void			DoCalibration();
	virtual void			DumpResults();

	//	member variables
	int									mCenterCameraIndex;
	std::vector<SingleCameraResult>		mCalibrationResults;

	std::vector<ublas::matrix<double, ublas::column_major> >	T_list;
	std::vector<ublas::matrix<double, ublas::column_major> >	om_list;
	std::vector<ublas::matrix<double, ublas::column_major> >	R_list;

	std::vector<ublas::vector<double> >							fc_left_list;
	std::vector<ublas::vector<double> >							cc_left_list;
	std::vector<ublas::vector<double> >							kc_left_list;
	ublas::vector<double>										alpha_c_left_list;

	std::vector<ublas::vector<double> >							fc_right_list;
	std::vector<ublas::vector<double> >							cc_right_list;
	std::vector<ublas::vector<double> >							kc_right_list;
	ublas::vector<double>										alpha_c_right_list;

	std::vector<ublas::vector<double> >							fc_left_error_list;
	std::vector<ublas::vector<double> >							cc_left_error_list;
	std::vector<ublas::vector<double> >							kc_left_error_list;
	ublas::vector<double>										alpha_c_left_error_list;

	std::vector<ublas::vector<double> >							fc_right_error_list;
	std::vector<ublas::vector<double> >							cc_right_error_list;
	std::vector<ublas::vector<double> >							kc_right_error_list;
	ublas::vector<double>										alpha_c_right_error_list;

	std::vector<ublas::matrix<double, ublas::column_major> >	T_error_list;
	std::vector<ublas::matrix<double, ublas::column_major> >	om_error_list;


protected:
private:
	void					dumpOnePairResults(int inIndex);
};


#endif	// #ifdef __MULTI_CAMERA_CALIBRATION_HPP

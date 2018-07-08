// =============================================================================
//  MultiCameraCalibration.cpp
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
	\file		MultiCameraCalibration.cpp
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

#include "MultiCameraCalibration.hpp"
#include "StereoCalibration.hpp"

//bool	g_debug_enabled = false;


//  MultiCameraCalibration class public member functions ===========================
// -----------------------------------------------------------------------------
//	MultiCameraCalibration
// -----------------------------------------------------------------------------
//
MultiCameraCalibration::MultiCameraCalibration(int inImageWidth, int inImageHeight)
	: CameraCalibration(inImageWidth, inImageHeight)
{
	//	いくつかのパラメータを初期化
}


// -----------------------------------------------------------------------------
//	~MultiCameraCalibration
// -----------------------------------------------------------------------------
//
MultiCameraCalibration::~MultiCameraCalibration()
{
}


// -----------------------------------------------------------------------------
//	DoCalibration
// -----------------------------------------------------------------------------
//
void	MultiCameraCalibration::DoCalibration()
{
	int	cameraNum = mCalibrationResults.size();
	if (mCenterCameraIndex >= cameraNum)
	{
		printf("ASSERT: mCenterCameraIndex >= cameraNum MultiCameraCalibration::doCalibration()\n");
		return;
	}

	T_list.resize(cameraNum - 1);
	om_list.resize(cameraNum - 1);
	R_list.resize(cameraNum - 1);

	fc_left_error_list.resize(cameraNum - 1);
	cc_left_error_list.resize(cameraNum - 1);
	kc_left_error_list.resize(cameraNum - 1);
	alpha_c_left_error_list.resize(cameraNum - 1);

	fc_right_error_list.resize(cameraNum - 1);
	cc_right_error_list.resize(cameraNum - 1);
	kc_right_error_list.resize(cameraNum - 1);
	alpha_c_right_error_list.resize(cameraNum - 1);

	T_error_list.resize(cameraNum - 1);
	om_error_list.resize(cameraNum - 1);

	StereoCalibration	calibrationPair(mImageWidth, mImageHeight);
	int	i, count;

	if (mCenterCameraIndex != 0)
		count = 0;
	else
		count = 1;

	for (i = 0; i < cameraNum - 1; i++, count++)
	{
		printf("Calibrating Camera Pair %d and %d\n", mCenterCameraIndex, count);

		calibrationPair.x_left_list = mCalibrationResults[mCenterCameraIndex].x_list;
		calibrationPair.X_left_list = mCalibrationResults[mCenterCameraIndex].X_list;
		calibrationPair.omc_left_list = mCalibrationResults[mCenterCameraIndex].omc_list;
		calibrationPair.Tc_left_list = mCalibrationResults[mCenterCameraIndex].Tc_list;
		calibrationPair.fc_left = mCalibrationResults[mCenterCameraIndex].fc;
		calibrationPair.cc_left = mCalibrationResults[mCenterCameraIndex].cc;
		calibrationPair.kc_left = mCalibrationResults[mCenterCameraIndex].kc;
		calibrationPair.alpha_c_left = mCalibrationResults[mCenterCameraIndex].alpha_c;

		calibrationPair.x_right_list = mCalibrationResults[count].x_list;
		calibrationPair.X_right_list = mCalibrationResults[count].X_list;
		calibrationPair.omc_right_list = mCalibrationResults[count].omc_list;
		calibrationPair.Tc_right_list = mCalibrationResults[count].Tc_list;
		calibrationPair.fc_right = mCalibrationResults[count].fc;
		calibrationPair.cc_right = mCalibrationResults[count].cc;
		calibrationPair.kc_right = mCalibrationResults[count].kc;
		calibrationPair.alpha_c_right = mCalibrationResults[count].alpha_c;

		calibrationPair.DoCalibration();

		T_list[i] = calibrationPair.T;
		om_list[i] = calibrationPair.om;
		R_list[i] = calibrationPair.R;

		fc_left_list[i] = calibrationPair.fc_left;
		cc_left_list[i] = calibrationPair.cc_left;
		kc_left_list[i] = calibrationPair.kc_left;
		alpha_c_left_list[i] = calibrationPair.alpha_c_left;

		fc_right_list[i] = calibrationPair.fc_right;
		cc_right_list[i] = calibrationPair.cc_right;
		kc_right_list[i] = calibrationPair.kc_right;
		alpha_c_right_list[i] = calibrationPair.alpha_c_right;

		fc_left_error_list[i] = calibrationPair.fc_left_error;
		cc_left_error_list[i] = calibrationPair.cc_left_error;
		kc_left_error_list[i] = calibrationPair.kc_left_error;
		alpha_c_left_error_list[i] = calibrationPair.alpha_c_left_error;

		fc_right_error_list[i] = calibrationPair.fc_right_error;
		cc_right_error_list[i] = calibrationPair.cc_right_error;
		kc_right_error_list[i] = calibrationPair.kc_right_error;
		alpha_c_right_error_list[i] = calibrationPair.alpha_c_right_error;

		T_error_list[i] = calibrationPair.T_error;
		om_error_list[i] = calibrationPair.om_error;

		if (i + 1 != mCenterCameraIndex)
			count++;
		else
			count+=2;
	}

	DumpResults();
}


// -----------------------------------------------------------------------------
//	DumpResults
// -----------------------------------------------------------------------------
void	MultiCameraCalibration::DumpResults()
{
	//	すでにキャリブレーションされているかどうか，チェックすべき
	
	int	cameraNum = mCalibrationResults.size();
	int	i, count;

	if (mCenterCameraIndex != 0)
		count = 0;
	else
		count = 1;

	for (i = 0; i < cameraNum - 1; i++, count++)
	{
		printf("Calibrating Camera Pair %d and %d\n", mCenterCameraIndex, count);

		dumpOnePairResults(i);

		if (i + 1 != mCenterCameraIndex)
			count++;
		else
			count+=2;
	}
}


// -----------------------------------------------------------------------------
//	dumpOnePairResults
// -----------------------------------------------------------------------------
void	MultiCameraCalibration::dumpOnePairResults(int inIndex)
{
	//	すでにキャリブレーションされているかどうか，チェックすべき
	
	std::cout << "Stereo calibration parameters after optimization:" << std::endl;

	printf("\n\nCalibration results after optimization (with uncertainties):\n\n");
	printf("Focal Length:          fc_left = [ %3.5f   %3.5f ] error [ %3.5f   %3.5f ]\n",
										fc_left_list[inIndex](0), fc_left_list[inIndex](1),
										fc_left_error_list[inIndex](0), fc_left_error_list[inIndex](1));
	printf("Principal point:       cc_left = [ %3.5f   %3.5f ] error [ %3.5f   %3.5f ]\n",
										cc_left_list[inIndex](0), cc_left_list[inIndex](1),
										cc_left_error_list[inIndex](0), cc_left_error_list[inIndex](1));
	printf("Skew:             alpha_c_left = [ %3.5f ] error [ %3.5f ]   => angle of pixel = %3.5f  error [ %3.5f ] degrees\n",
										alpha_c_left_list[inIndex], alpha_c_left_error_list[inIndex],
										90 - atan(alpha_c_left_list[inIndex]) * 180 / 3.141592,
										atan(alpha_c_left_error_list[inIndex]) * 180 / 3.141592);
	printf("Distortion:            kc_left = [ %3.5f   %3.5f   %3.5f   %3.5f   %5.5f ] error [ %3.5f   %3.5f   %3.5f   %3.5f   %5.5f ]\n",
		kc_left_list[inIndex](0), kc_left_list[inIndex](1), kc_left_list[inIndex](2),
		kc_left_list[inIndex](3), kc_left_list[inIndex](4),
		kc_left_error_list[inIndex](0), kc_left_error_list[inIndex](1), kc_left_error_list[inIndex](2),
		kc_left_error_list[inIndex](3), kc_left_error_list[inIndex](4));
	
	printf("\n\nCalibration results after optimization (with uncertainties):\n\n");
	printf("Focal Length:          fc_right = [ %3.5f   %3.5f ] error [ %3.5f   %3.5f ]\n",
										fc_right_list[inIndex](0), fc_right_list[inIndex](1),
										fc_right_error_list[inIndex](0), fc_right_error_list[inIndex](1));
	printf("Principal point:       cc_right = [ %3.5f   %3.5f ] error [ %3.5f   %3.5f ]\n",
										cc_right_list[inIndex](0), cc_right_list[inIndex](1),
										cc_right_error_list[inIndex](0), cc_right_error_list[inIndex](1));
	printf("Skew:             alpha_c_right = [ %3.5f ] error [ %3.5f ]   => angle of pixel = %3.5f  error [ %3.5f ] degrees\n",
										alpha_c_right_list[inIndex], alpha_c_right_error_list[inIndex],
										90 - atan(alpha_c_right_list[inIndex]) * 180 / 3.141592,
										atan(alpha_c_right_error_list[inIndex]) * 180 / 3.141592);
	printf("Distortion:            kc_right = [ %3.5f   %3.5f   %3.5f   %3.5f   %5.5f ] error [ %3.5f   %3.5f   %3.5f   %3.5f   %5.5f ]\n",
		kc_right_list[inIndex](0), kc_right_list[inIndex](1), kc_right_list[inIndex](2),
		kc_right_list[inIndex](3), kc_right_list[inIndex](4),
		kc_right_error_list[inIndex](0), kc_right_error_list[inIndex](1), kc_right_error_list[inIndex](2),
		kc_right_error_list[inIndex](3), kc_right_error_list[inIndex](4));

	printf("\n\nExtrinsic parameters (position of right camera wrt left camera):\n\n");
	printf("Rotation vector:             om = [ %3.5f   %3.5f  %3.5f ] error [ %3.5f   %3.5f  %3.5f ]\n",
		om_list[inIndex](0, 0), om_list[inIndex](1, 0), om_list[inIndex](2, 0),
		om_error_list[inIndex](0, 0), om_error_list[inIndex](1, 0), om_error_list[inIndex](2, 0));
	printf("Translation vector:           T = [ %3.5f   %3.5f  %3.5f ] error [ %3.5f   %3.5f  %3.5f ]\n",
		T_list[inIndex](0, 0), T_list[inIndex](1, 0), T_list[inIndex](2, 0),
		T_error_list[inIndex](0, 0), T_error_list[inIndex](1, 0), T_error_list[inIndex](2, 0));

std::cout << "Note: The numerical errors are approximately three times the standard deviations (for reference)." << std::endl;
//std::cout << "Suggested threshold = " << std::endl;
}

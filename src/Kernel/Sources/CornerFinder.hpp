// =============================================================================
//  CornerFinder.hpp
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
	\file		CornerFinder.hpp
	\author		Dairoku Sekiguchi
	\version	1.0
	\date		2007/04/05
	\brief		This file is a part of CalibraKernel
	
	This is a direct C++ port of "Camera Calibration Toolbox for Matlab"
	by Jean-Yves Bouguet.
	Original source can be found at:
	http://www.vision.caltech.edu/bouguetj/calib_doc/
*/

#ifndef __CORNER_FINDER_HPP
#define __CORNER_FINDER_HPP


// -----------------------------------------------------------------------------
// 	include files
// -----------------------------------------------------------------------------
#include "CameraCalibration.hpp"


// -----------------------------------------------------------------------------
// 	CornerFinder class
// -----------------------------------------------------------------------------
class	CornerFinder : public CameraCalibration
{
public:
	//	constructor/destructor
							CornerFinder(int inImageWidth, int inImageHeight);
	virtual					~CornerFinder();

	//	member variables
	//	member functions
	static void		findGrid(
								const ublas::matrix<unsigned char, ublas::column_major> &in_I,
								const ublas::matrix<double, ublas::column_major> &in_x,
								int	in_wintx, int in_winty,
								double in_dX, double in_dY,
								int in_n_sq_x, double in_n_sq_y,
								ublas::matrix<double, ublas::column_major> &out_XX,
								ublas::matrix<double, ublas::column_major> &out_x,
								ublas::matrix<double, ublas::column_major> &out_X,
								ublas::vector<int> &out_result);

	static void		findRectangle(
								const ublas::matrix<unsigned char, ublas::column_major> &in_I,
								ublas::matrix<double, ublas::column_major> &io_x,
								int	in_wintx, int in_winty,
								int	*out_n_sq_x1, int *out_n_sq_x2,
								int	*out_n_sq_y1, int *out_n_sq_y2);

	static int		count_squares(
								const ublas::matrix<unsigned char, ublas::column_major> &in_I,
								double in_x1, double in_y1,
								double in_x2, double in_y2,
								int	in_win);

	static bool		findCorner(
								const ublas::matrix<unsigned char, ublas::column_major> &in_I,
								double in_x, double in_y,
								int	in_wintx, int in_winty,
								double *out_x, double *out_y);
protected:
};


#endif	// #ifdef __CORNER_FINDER_HPP

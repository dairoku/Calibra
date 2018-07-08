// =============================================================================
//  SingleCameraResultNode.hpp
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
	\file		SingleCameraResultNode.hpp
	\author		Dairoku Sekiguchi
	\version	1.0
	\date		2007/11/14
	\brief

	Description...
*/
#ifndef __SINGLE_CAMERA_RESULT_NODE_H
#define __SINGLE_CAMERA_RESULT_NODE_H


// -----------------------------------------------------------------------------
// 	include files
// -----------------------------------------------------------------------------
#include <string>
#include <vector>
#include "CalibrationResultNode.hpp"

// -----------------------------------------------------------------------------
// 	macros
// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------
//	SingleCameraResultNode class
// -----------------------------------------------------------------------------
//
class SingleCameraResultNode : public CalibrationResultNode
{
public:

	SingleCameraResultNode()
		: mCameraCalibration(DEFAULT_IMAGE_WIDTH, DEFAULT_IMAGE_HEIGHT)
	{
	}

	SingleCameraResultNode(int inWidth, int inHeight, const std::wstring &inCalibrationResultName)
		:	mCameraCalibration(inWidth, inHeight),
			CalibrationResultNode(inCalibrationResultName)
	{
	}

	virtual bool	ChildNodeCheck(CalibraNode *inNode) const
	{
		//if (inNode is CalibrationResultNode)
		//	return true;	must enable RTTI!

		return true;
	}

	virtual void	ReadFromStream(std::istream &ioIStream)
	{
		unsigned int	size, objectID;
		bool	isSuperclass;

		ReadObjectDataHeader(ioIStream, &size, &objectID, &isSuperclass);

		if (objectID != SINGLE_CAMERA_RESULT_NODE_OBJECT_ID)
			throw std::runtime_error("OBJECT_ID dose not mutch: in SingleCameraResultNode::ReadFromStream");

		CalibraFileUtil::ReadDoubleFromStream(ioIStream, &mCameraCalibration.mImageWidth);
		CalibraFileUtil::ReadDoubleFromStream(ioIStream, &mCameraCalibration.mImageHeight);

		CalibraFileUtil::ReadMatrixListFromStream(ioIStream, mCameraCalibration.x_list);
		CalibraFileUtil::ReadMatrixListFromStream(ioIStream, mCameraCalibration.X_list);
		CalibraFileUtil::ReadMatrixListFromStream(ioIStream, mCameraCalibration.H_list);
		CalibraFileUtil::ReadMatrixListFromStream(ioIStream, mCameraCalibration.omc_list);
		CalibraFileUtil::ReadMatrixListFromStream(ioIStream, mCameraCalibration.Tc_list);
		CalibraFileUtil::ReadMatrixListFromStream(ioIStream, mCameraCalibration.Rc_list);
		CalibraFileUtil::ReadMatrixListFromStream(ioIStream, mCameraCalibration.y_list);
		CalibraFileUtil::ReadMatrixListFromStream(ioIStream, mCameraCalibration.ex_list);

		CalibraFileUtil::ReadDoubleVectorFromStream(ioIStream, mCameraCalibration.fc);
		CalibraFileUtil::ReadDoubleVectorFromStream(ioIStream, mCameraCalibration.cc);
		CalibraFileUtil::ReadDoubleVectorFromStream(ioIStream, mCameraCalibration.kc);
		CalibraFileUtil::ReadDoubleFromStream(ioIStream, &mCameraCalibration.alpha_c);

		CalibraFileUtil::ReadDoubleVectorFromStream(ioIStream, mCameraCalibration.err_std);
		CalibraFileUtil::ReadDoubleVectorFromStream(ioIStream, mCameraCalibration.fc_error);
		CalibraFileUtil::ReadDoubleVectorFromStream(ioIStream, mCameraCalibration.cc_error);
		CalibraFileUtil::ReadDoubleVectorFromStream(ioIStream, mCameraCalibration.kc_error);
		CalibraFileUtil::ReadDoubleFromStream(ioIStream, &mCameraCalibration.alpha_c_error);	

		CalibraFileUtil::ReadDoubleFromStream(ioIStream, &mCameraCalibration.thresh_cond);

		CalibraFileUtil::ReadMatrixFromStream(ioIStream, mCameraCalibration.KK);

		CalibraFileUtil::ReadMatrixFromStream(ioIStream, mCameraCalibration.N_points_views);

		CalibrationResultNode::ReadFromStream(ioIStream);
	}

	virtual void	WriteToStream(std::ostream &ioOStream, bool inIsSuperclass) const
	{
		WriteObjectDataHeader(ioOStream, CalcStreamDataSectionSize(),
									SINGLE_CAMERA_RESULT_NODE_OBJECT_ID, inIsSuperclass);

		CalibraFileUtil::WriteDoubleToStream(ioOStream, mCameraCalibration.mImageWidth);
		CalibraFileUtil::WriteDoubleToStream(ioOStream, mCameraCalibration.mImageHeight);

		CalibraFileUtil::WriteMatrixListToStream(ioOStream, mCameraCalibration.x_list);
		CalibraFileUtil::WriteMatrixListToStream(ioOStream, mCameraCalibration.X_list);
		CalibraFileUtil::WriteMatrixListToStream(ioOStream, mCameraCalibration.H_list);
		CalibraFileUtil::WriteMatrixListToStream(ioOStream, mCameraCalibration.omc_list);
		CalibraFileUtil::WriteMatrixListToStream(ioOStream, mCameraCalibration.Tc_list);
		CalibraFileUtil::WriteMatrixListToStream(ioOStream, mCameraCalibration.Rc_list);
		CalibraFileUtil::WriteMatrixListToStream(ioOStream, mCameraCalibration.y_list);
		CalibraFileUtil::WriteMatrixListToStream(ioOStream, mCameraCalibration.ex_list);

		CalibraFileUtil::WriteDoubleVectorToStream(ioOStream, mCameraCalibration.fc);
		CalibraFileUtil::WriteDoubleVectorToStream(ioOStream, mCameraCalibration.cc);
		CalibraFileUtil::WriteDoubleVectorToStream(ioOStream, mCameraCalibration.kc);
		CalibraFileUtil::WriteDoubleToStream(ioOStream, mCameraCalibration.alpha_c);

		CalibraFileUtil::WriteDoubleVectorToStream(ioOStream, mCameraCalibration.err_std);
		CalibraFileUtil::WriteDoubleVectorToStream(ioOStream, mCameraCalibration.fc_error);
		CalibraFileUtil::WriteDoubleVectorToStream(ioOStream, mCameraCalibration.cc_error);
		CalibraFileUtil::WriteDoubleVectorToStream(ioOStream, mCameraCalibration.kc_error);
		CalibraFileUtil::WriteDoubleToStream(ioOStream, mCameraCalibration.alpha_c_error);	

		CalibraFileUtil::WriteDoubleToStream(ioOStream, mCameraCalibration.thresh_cond);

		CalibraFileUtil::WriteMatrixToStream(ioOStream, mCameraCalibration.KK);

		CalibraFileUtil::WriteMatrixToStream(ioOStream, mCameraCalibration.N_points_views);

		CalibrationResultNode::WriteToStream(ioOStream, true);
	}

	CameraCalibration	mCameraCalibration;

private:


	unsigned int	CalcStreamDataSectionSize() const
	{
		unsigned int	size = OBJECT_HEADER_LEN;

		size += CalibraFileUtil::CalcDoubleStreamSize();	// mImageWidth
		size += CalibraFileUtil::CalcDoubleStreamSize();	// mImageHeight

		size += CalibraFileUtil::CalcMatrixListStreamSize(mCameraCalibration.x_list);
		size += CalibraFileUtil::CalcMatrixListStreamSize(mCameraCalibration.X_list);
		size += CalibraFileUtil::CalcMatrixListStreamSize(mCameraCalibration.H_list);
		size += CalibraFileUtil::CalcMatrixListStreamSize(mCameraCalibration.omc_list);
		size += CalibraFileUtil::CalcMatrixListStreamSize(mCameraCalibration.Tc_list);
		size += CalibraFileUtil::CalcMatrixListStreamSize(mCameraCalibration.Rc_list);
		size += CalibraFileUtil::CalcMatrixListStreamSize(mCameraCalibration.y_list);
		size += CalibraFileUtil::CalcMatrixListStreamSize(mCameraCalibration.ex_list);

		size += CalibraFileUtil::CalcDoubleVectorStreamSize(mCameraCalibration.fc);
		size += CalibraFileUtil::CalcDoubleVectorStreamSize(mCameraCalibration.cc);
		size += CalibraFileUtil::CalcDoubleVectorStreamSize(mCameraCalibration.kc);
		size += CalibraFileUtil::CalcDoubleStreamSize();	// alpha_c

		size += CalibraFileUtil::CalcDoubleVectorStreamSize(mCameraCalibration.err_std);
		size += CalibraFileUtil::CalcDoubleVectorStreamSize(mCameraCalibration.fc_error);
		size += CalibraFileUtil::CalcDoubleVectorStreamSize(mCameraCalibration.cc_error);
		size += CalibraFileUtil::CalcDoubleVectorStreamSize(mCameraCalibration.kc_error);
		size += CalibraFileUtil::CalcDoubleStreamSize();	// alpha_c_error

		size += CalibraFileUtil::CalcDoubleStreamSize();	// thresh_cond

		size += CalibraFileUtil::CalcMatrixStreamSize(mCameraCalibration.KK);

		size += CalibraFileUtil::CalcMatrixStreamSize(mCameraCalibration.N_points_views);

		return size;
	}
};

#endif	// #ifdef __SINGLECAMERA_RESULT_NODE_H

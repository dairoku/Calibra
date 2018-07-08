// =============================================================================
//  StereoCameraCalibrationNode.hpp
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
	\file		StereoCameraCalibrationNode.hpp
	\author		Dairoku Sekiguchi
	\version	1.0
	\date		2007/11/14
	\brief

	Description...
*/
#ifndef __STEREO_CAMERA_RESULT_NODE_H
#define __STEREO_CAMERA_RESULT_NODE_H


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
//	StereoCameraCalibrationNode class
// -----------------------------------------------------------------------------
//
class StereoCameraResultNode : public CalibrationResultNode
{
public:
	StereoCameraResultNode()
		:	mStereoCalibration(DEFAULT_IMAGE_WIDTH, DEFAULT_IMAGE_HEIGHT)
	{
	}

	StereoCameraResultNode(int inWidth, int inHeight, const std::wstring &inCalibrationResultName)
		:	mStereoCalibration(inWidth, inHeight),
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

		if (objectID != STEREO_CAMERA_RESULT_NODE_OBJECT_ID)
			throw std::runtime_error("OBJECT_ID dose not mutch: in StereoCameraResultNode::ReadFromStream");

		CalibraFileUtil::ReadDoubleFromStream(ioIStream, &mStereoCalibration.mImageWidth);
		CalibraFileUtil::ReadDoubleFromStream(ioIStream, &mStereoCalibration.mImageHeight);

		CalibraFileUtil::ReadMatrixListFromStream(ioIStream, mStereoCalibration.X_left_list);
		CalibraFileUtil::ReadMatrixListFromStream(ioIStream, mStereoCalibration.x_left_list);
		CalibraFileUtil::ReadMatrixListFromStream(ioIStream, mStereoCalibration.omc_left_list);
		//CalibraFileUtil::ReadMatrixListFromStream(ioIStream, mStereoCalibration.Rc_left_list);
		CalibraFileUtil::ReadMatrixListFromStream(ioIStream, mStereoCalibration.Tc_left_list);

		CalibraFileUtil::ReadMatrixListFromStream(ioIStream, mStereoCalibration.X_right_list);
		CalibraFileUtil::ReadMatrixListFromStream(ioIStream, mStereoCalibration.x_right_list);
		CalibraFileUtil::ReadMatrixListFromStream(ioIStream, mStereoCalibration.omc_right_list);
		//CalibraFileUtil::ReadMatrixListFromStream(ioIStream, mStereoCalibration.Rc_right_list);
		CalibraFileUtil::ReadMatrixListFromStream(ioIStream, mStereoCalibration.Tc_right_list);

		CalibraFileUtil::ReadDoubleVectorFromStream(ioIStream, mStereoCalibration.fc_left);
		CalibraFileUtil::ReadDoubleVectorFromStream(ioIStream, mStereoCalibration.cc_left);
		CalibraFileUtil::ReadDoubleVectorFromStream(ioIStream, mStereoCalibration.kc_left);
		CalibraFileUtil::ReadDoubleFromStream(ioIStream, &mStereoCalibration.alpha_c_left);

		CalibraFileUtil::ReadDoubleVectorFromStream(ioIStream, mStereoCalibration.fc_right);
		CalibraFileUtil::ReadDoubleVectorFromStream(ioIStream, mStereoCalibration.cc_right);
		CalibraFileUtil::ReadDoubleVectorFromStream(ioIStream, mStereoCalibration.kc_right);
		CalibraFileUtil::ReadDoubleFromStream(ioIStream, &mStereoCalibration.alpha_c_right);

		CalibraFileUtil::ReadMatrixFromStream(ioIStream, mStereoCalibration.T);
		CalibraFileUtil::ReadMatrixFromStream(ioIStream, mStereoCalibration.om);
		CalibraFileUtil::ReadMatrixFromStream(ioIStream, mStereoCalibration.R);

		CalibraFileUtil::ReadDoubleVectorFromStream(ioIStream, mStereoCalibration.fc_left_error);
		CalibraFileUtil::ReadDoubleVectorFromStream(ioIStream, mStereoCalibration.cc_left_error);
		CalibraFileUtil::ReadDoubleVectorFromStream(ioIStream, mStereoCalibration.kc_left_error);
		CalibraFileUtil::ReadDoubleFromStream(ioIStream, &mStereoCalibration.alpha_c_left_error);

		CalibraFileUtil::ReadDoubleVectorFromStream(ioIStream, mStereoCalibration.fc_right_error);
		CalibraFileUtil::ReadDoubleVectorFromStream(ioIStream, mStereoCalibration.cc_right_error);
		CalibraFileUtil::ReadDoubleVectorFromStream(ioIStream, mStereoCalibration.kc_right_error);
		CalibraFileUtil::ReadDoubleFromStream(ioIStream, &mStereoCalibration.alpha_c_right_error);

		CalibraFileUtil::ReadMatrixFromStream(ioIStream, mStereoCalibration.T_error);
		CalibraFileUtil::ReadMatrixFromStream(ioIStream, mStereoCalibration.om_error);

		CalibraFileUtil::ReadDoubleVectorFromStream(ioIStream, mStereoCalibration.a1_left);
		CalibraFileUtil::ReadDoubleVectorFromStream(ioIStream, mStereoCalibration.a2_left);
		CalibraFileUtil::ReadDoubleVectorFromStream(ioIStream, mStereoCalibration.a3_left);
		CalibraFileUtil::ReadDoubleVectorFromStream(ioIStream, mStereoCalibration.a4_left);

		CalibraFileUtil::ReadIntVectorFromStream(ioIStream, mStereoCalibration.ind_new_left);
		CalibraFileUtil::ReadIntVectorFromStream(ioIStream, mStereoCalibration.ind_1_left);
		CalibraFileUtil::ReadIntVectorFromStream(ioIStream, mStereoCalibration.ind_2_left);
		CalibraFileUtil::ReadIntVectorFromStream(ioIStream, mStereoCalibration.ind_3_left);
		CalibraFileUtil::ReadIntVectorFromStream(ioIStream, mStereoCalibration.ind_4_left);

		CalibraFileUtil::ReadDoubleVectorFromStream(ioIStream, mStereoCalibration.a1_right);
		CalibraFileUtil::ReadDoubleVectorFromStream(ioIStream, mStereoCalibration.a2_right);
		CalibraFileUtil::ReadDoubleVectorFromStream(ioIStream, mStereoCalibration.a3_right);
		CalibraFileUtil::ReadDoubleVectorFromStream(ioIStream, mStereoCalibration.a4_right);

		CalibraFileUtil::ReadIntVectorFromStream(ioIStream, mStereoCalibration.ind_new_right);
		CalibraFileUtil::ReadIntVectorFromStream(ioIStream, mStereoCalibration.ind_1_right);
		CalibraFileUtil::ReadIntVectorFromStream(ioIStream, mStereoCalibration.ind_2_right);
		CalibraFileUtil::ReadIntVectorFromStream(ioIStream, mStereoCalibration.ind_3_right);
		CalibraFileUtil::ReadIntVectorFromStream(ioIStream, mStereoCalibration.ind_4_right);

		CalibrationResultNode::ReadFromStream(ioIStream);
	}

	virtual void	WriteToStream(std::ostream &ioOStream, bool inIsSuperclass) const
	{
		WriteObjectDataHeader(ioOStream, CalcStreamDataSectionSize(),
									STEREO_CAMERA_RESULT_NODE_OBJECT_ID, inIsSuperclass);

		CalibraFileUtil::WriteDoubleToStream(ioOStream, mStereoCalibration.mImageWidth);
		CalibraFileUtil::WriteDoubleToStream(ioOStream, mStereoCalibration.mImageHeight);

		CalibraFileUtil::WriteMatrixListToStream(ioOStream, mStereoCalibration.X_left_list);
		CalibraFileUtil::WriteMatrixListToStream(ioOStream, mStereoCalibration.x_left_list);
		CalibraFileUtil::WriteMatrixListToStream(ioOStream, mStereoCalibration.omc_left_list);
		//CalibraFileUtil::WriteMatrixListToStream(ioOStream, mStereoCalibration.Rc_left_list);
		CalibraFileUtil::WriteMatrixListToStream(ioOStream, mStereoCalibration.Tc_left_list);

		CalibraFileUtil::WriteMatrixListToStream(ioOStream, mStereoCalibration.X_right_list);
		CalibraFileUtil::WriteMatrixListToStream(ioOStream, mStereoCalibration.x_right_list);
		CalibraFileUtil::WriteMatrixListToStream(ioOStream, mStereoCalibration.omc_right_list);
		//CalibraFileUtil::WriteMatrixListToStream(ioOStream, mStereoCalibration.Rc_right_list);
		CalibraFileUtil::WriteMatrixListToStream(ioOStream, mStereoCalibration.Tc_right_list);

		CalibraFileUtil::WriteDoubleVectorToStream(ioOStream, mStereoCalibration.fc_left);
		CalibraFileUtil::WriteDoubleVectorToStream(ioOStream, mStereoCalibration.cc_left);
		CalibraFileUtil::WriteDoubleVectorToStream(ioOStream, mStereoCalibration.kc_left);
		CalibraFileUtil::WriteDoubleToStream(ioOStream, mStereoCalibration.alpha_c_left);

		CalibraFileUtil::WriteDoubleVectorToStream(ioOStream, mStereoCalibration.fc_right);
		CalibraFileUtil::WriteDoubleVectorToStream(ioOStream, mStereoCalibration.cc_right);
		CalibraFileUtil::WriteDoubleVectorToStream(ioOStream, mStereoCalibration.kc_right);
		CalibraFileUtil::WriteDoubleToStream(ioOStream, mStereoCalibration.alpha_c_right);

		CalibraFileUtil::WriteMatrixToStream(ioOStream, mStereoCalibration.T);
		CalibraFileUtil::WriteMatrixToStream(ioOStream, mStereoCalibration.om);
		CalibraFileUtil::WriteMatrixToStream(ioOStream, mStereoCalibration.R);

		CalibraFileUtil::WriteDoubleVectorToStream(ioOStream, mStereoCalibration.fc_left_error);
		CalibraFileUtil::WriteDoubleVectorToStream(ioOStream, mStereoCalibration.cc_left_error);
		CalibraFileUtil::WriteDoubleVectorToStream(ioOStream, mStereoCalibration.kc_left_error);
		CalibraFileUtil::WriteDoubleToStream(ioOStream, mStereoCalibration.alpha_c_left_error);

		CalibraFileUtil::WriteDoubleVectorToStream(ioOStream, mStereoCalibration.fc_right_error);
		CalibraFileUtil::WriteDoubleVectorToStream(ioOStream, mStereoCalibration.cc_right_error);
		CalibraFileUtil::WriteDoubleVectorToStream(ioOStream, mStereoCalibration.kc_right_error);
		CalibraFileUtil::WriteDoubleToStream(ioOStream, mStereoCalibration.alpha_c_right_error);

		CalibraFileUtil::WriteMatrixToStream(ioOStream, mStereoCalibration.T_error);
		CalibraFileUtil::WriteMatrixToStream(ioOStream, mStereoCalibration.om_error);

		CalibraFileUtil::WriteDoubleVectorToStream(ioOStream, mStereoCalibration.a1_left);
		CalibraFileUtil::WriteDoubleVectorToStream(ioOStream, mStereoCalibration.a2_left);
		CalibraFileUtil::WriteDoubleVectorToStream(ioOStream, mStereoCalibration.a3_left);
		CalibraFileUtil::WriteDoubleVectorToStream(ioOStream, mStereoCalibration.a4_left);

		CalibraFileUtil::WriteIntVectorToStream(ioOStream, mStereoCalibration.ind_new_left);
		CalibraFileUtil::WriteIntVectorToStream(ioOStream, mStereoCalibration.ind_1_left);
		CalibraFileUtil::WriteIntVectorToStream(ioOStream, mStereoCalibration.ind_2_left);
		CalibraFileUtil::WriteIntVectorToStream(ioOStream, mStereoCalibration.ind_3_left);
		CalibraFileUtil::WriteIntVectorToStream(ioOStream, mStereoCalibration.ind_4_left);

		CalibraFileUtil::WriteDoubleVectorToStream(ioOStream, mStereoCalibration.a1_right);
		CalibraFileUtil::WriteDoubleVectorToStream(ioOStream, mStereoCalibration.a2_right);
		CalibraFileUtil::WriteDoubleVectorToStream(ioOStream, mStereoCalibration.a3_right);
		CalibraFileUtil::WriteDoubleVectorToStream(ioOStream, mStereoCalibration.a4_right);

		CalibraFileUtil::WriteIntVectorToStream(ioOStream, mStereoCalibration.ind_new_right);
		CalibraFileUtil::WriteIntVectorToStream(ioOStream, mStereoCalibration.ind_1_right);
		CalibraFileUtil::WriteIntVectorToStream(ioOStream, mStereoCalibration.ind_2_right);
		CalibraFileUtil::WriteIntVectorToStream(ioOStream, mStereoCalibration.ind_3_right);
		CalibraFileUtil::WriteIntVectorToStream(ioOStream, mStereoCalibration.ind_4_right);

		CalibrationResultNode::WriteToStream(ioOStream, true);
	}

	StereoCalibration	mStereoCalibration;

private:


	unsigned int	CalcStreamDataSectionSize() const
	{
		unsigned int	size = OBJECT_HEADER_LEN;

		size += CalibraFileUtil::CalcDoubleStreamSize();	// mImageWidth
		size += CalibraFileUtil::CalcDoubleStreamSize();	// mImageHeight

		size += CalibraFileUtil::CalcMatrixListStreamSize(mStereoCalibration.X_left_list);
		size += CalibraFileUtil::CalcMatrixListStreamSize(mStereoCalibration.x_left_list);
		size += CalibraFileUtil::CalcMatrixListStreamSize(mStereoCalibration.omc_left_list);
		//size += CalibraFileUtil::CalcMatrixListStreamSize(mStereoCalibration.Rc_left_list);
		size += CalibraFileUtil::CalcMatrixListStreamSize(mStereoCalibration.Tc_left_list);

		size += CalibraFileUtil::CalcMatrixListStreamSize(mStereoCalibration.X_right_list);
		size += CalibraFileUtil::CalcMatrixListStreamSize(mStereoCalibration.x_right_list);
		size += CalibraFileUtil::CalcMatrixListStreamSize(mStereoCalibration.omc_right_list);
		//size += CalibraFileUtil::CalcMatrixListStreamSize(mStereoCalibration.Rc_right_list);
		size += CalibraFileUtil::CalcMatrixListStreamSize(mStereoCalibration.Tc_right_list);

		size += CalibraFileUtil::CalcDoubleVectorStreamSize(mStereoCalibration.fc_left);
		size += CalibraFileUtil::CalcDoubleVectorStreamSize(mStereoCalibration.cc_left);
		size += CalibraFileUtil::CalcDoubleVectorStreamSize(mStereoCalibration.kc_left);
		size += CalibraFileUtil::CalcDoubleStreamSize();	// alpha_c_left

		size += CalibraFileUtil::CalcDoubleVectorStreamSize(mStereoCalibration.fc_right);
		size += CalibraFileUtil::CalcDoubleVectorStreamSize(mStereoCalibration.cc_right);
		size += CalibraFileUtil::CalcDoubleVectorStreamSize(mStereoCalibration.kc_right);
		size += CalibraFileUtil::CalcDoubleStreamSize();	// alpha_c_right

		size += CalibraFileUtil::CalcMatrixStreamSize(mStereoCalibration.T);
		size += CalibraFileUtil::CalcMatrixStreamSize(mStereoCalibration.om);
		size += CalibraFileUtil::CalcMatrixStreamSize(mStereoCalibration.R);

		size += CalibraFileUtil::CalcDoubleVectorStreamSize(mStereoCalibration.fc_left_error);
		size += CalibraFileUtil::CalcDoubleVectorStreamSize(mStereoCalibration.cc_left_error);
		size += CalibraFileUtil::CalcDoubleVectorStreamSize(mStereoCalibration.kc_left_error);
		size += CalibraFileUtil::CalcDoubleStreamSize();	// alpha_c_left_error

		size += CalibraFileUtil::CalcDoubleVectorStreamSize(mStereoCalibration.fc_right_error);
		size += CalibraFileUtil::CalcDoubleVectorStreamSize(mStereoCalibration.cc_right_error);
		size += CalibraFileUtil::CalcDoubleVectorStreamSize(mStereoCalibration.kc_right_error);
		size += CalibraFileUtil::CalcDoubleStreamSize();	// alpha_c_right_error

		size += CalibraFileUtil::CalcMatrixStreamSize(mStereoCalibration.T_error);
		size += CalibraFileUtil::CalcMatrixStreamSize(mStereoCalibration.om_error);

		size += CalibraFileUtil::CalcDoubleVectorStreamSize(mStereoCalibration.a1_left);
		size += CalibraFileUtil::CalcDoubleVectorStreamSize(mStereoCalibration.a2_left);
		size += CalibraFileUtil::CalcDoubleVectorStreamSize(mStereoCalibration.a3_left);
		size += CalibraFileUtil::CalcDoubleVectorStreamSize(mStereoCalibration.a4_left);

		size += CalibraFileUtil::CalcIntVectorStreamSize(mStereoCalibration.ind_new_left);
		size += CalibraFileUtil::CalcIntVectorStreamSize(mStereoCalibration.ind_1_left);
		size += CalibraFileUtil::CalcIntVectorStreamSize(mStereoCalibration.ind_2_left);
		size += CalibraFileUtil::CalcIntVectorStreamSize(mStereoCalibration.ind_3_left);
		size += CalibraFileUtil::CalcIntVectorStreamSize(mStereoCalibration.ind_4_left);

		size += CalibraFileUtil::CalcDoubleVectorStreamSize(mStereoCalibration.a1_right);
		size += CalibraFileUtil::CalcDoubleVectorStreamSize(mStereoCalibration.a2_right);
		size += CalibraFileUtil::CalcDoubleVectorStreamSize(mStereoCalibration.a3_right);
		size += CalibraFileUtil::CalcDoubleVectorStreamSize(mStereoCalibration.a4_right);

		size += CalibraFileUtil::CalcIntVectorStreamSize(mStereoCalibration.ind_new_right);
		size += CalibraFileUtil::CalcIntVectorStreamSize(mStereoCalibration.ind_1_right);
		size += CalibraFileUtil::CalcIntVectorStreamSize(mStereoCalibration.ind_2_right);
		size += CalibraFileUtil::CalcIntVectorStreamSize(mStereoCalibration.ind_3_right);
		size += CalibraFileUtil::CalcIntVectorStreamSize(mStereoCalibration.ind_4_right);

		return size;
	}
};

#endif	// #ifdef __STEREO_CAMERA_RESULT_NODE_H

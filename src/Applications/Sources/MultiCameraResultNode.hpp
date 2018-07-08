// =============================================================================
//  MultiCameraResultNode.hpp
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
	\file		MultiCameraResultNode.hpp
	\author		Dairoku Sekiguchi
	\version	1.0
	\date		2007/11/14
	\brief

	Description...
*/
#ifndef __MULTI_CAMERA_RESULT_NODE_H
#define __MULTI_CAMERA_RESULT_NODE_H


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
//	MultiCameraResultNode class
// -----------------------------------------------------------------------------
//
class MultiCameraResultNode : public CalibrationResultNode
{
public:
	MultiCameraResultNode()
		:	mMultiCameraCalibration(DEFAULT_IMAGE_WIDTH, DEFAULT_IMAGE_HEIGHT)
	{
	}

	MultiCameraResultNode(int inWidth, int inHeight, const std::wstring &inCalibrationResultName)
		:	mMultiCameraCalibration(inWidth, inHeight),
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

		if (objectID != MULTI_CAMERA_RESULT_NODE_OBJECT_ID)
			throw std::runtime_error("OBJECT_ID dose not mutch: in MultiCameraResultNode::ReadFromStream");

		CalibraFileUtil::ReadDoubleFromStream(ioIStream, &(mMultiCameraCalibration.mImageWidth));
		CalibraFileUtil::ReadDoubleFromStream(ioIStream, &(mMultiCameraCalibration.mImageHeight));

		CalibraFileUtil::ReadIntFromStream(ioIStream, &(mMultiCameraCalibration.mCenterCameraIndex));

		ReadSingleCameraResultListFromStream(ioIStream, mMultiCameraCalibration.mCalibrationResults);

		CalibraFileUtil::ReadMatrixListFromStream(ioIStream, mMultiCameraCalibration.T_list);
		CalibraFileUtil::ReadMatrixListFromStream(ioIStream, mMultiCameraCalibration.om_list);
		CalibraFileUtil::ReadMatrixListFromStream(ioIStream, mMultiCameraCalibration.R_list);

		CalibraFileUtil::ReadDoubleVectorListFromStream(ioIStream, mMultiCameraCalibration.fc_left_list);
		CalibraFileUtil::ReadDoubleVectorListFromStream(ioIStream, mMultiCameraCalibration.cc_left_list);
		CalibraFileUtil::ReadDoubleVectorListFromStream(ioIStream, mMultiCameraCalibration.kc_left_list);
		CalibraFileUtil::ReadDoubleVectorFromStream(ioIStream, mMultiCameraCalibration.alpha_c_left_list);

		CalibraFileUtil::ReadDoubleVectorListFromStream(ioIStream, mMultiCameraCalibration.fc_right_list);
		CalibraFileUtil::ReadDoubleVectorListFromStream(ioIStream, mMultiCameraCalibration.cc_right_list);
		CalibraFileUtil::ReadDoubleVectorListFromStream(ioIStream, mMultiCameraCalibration.kc_right_list);
		CalibraFileUtil::ReadDoubleVectorFromStream(ioIStream, mMultiCameraCalibration.alpha_c_right_list);

		CalibraFileUtil::ReadDoubleVectorListFromStream(ioIStream, mMultiCameraCalibration.fc_left_error_list);
		CalibraFileUtil::ReadDoubleVectorListFromStream(ioIStream, mMultiCameraCalibration.cc_left_error_list);
		CalibraFileUtil::ReadDoubleVectorListFromStream(ioIStream, mMultiCameraCalibration.kc_left_error_list);
		CalibraFileUtil::ReadDoubleVectorFromStream(ioIStream, mMultiCameraCalibration.alpha_c_left_error_list);

		CalibraFileUtil::ReadDoubleVectorListFromStream(ioIStream, mMultiCameraCalibration.fc_right_error_list);
		CalibraFileUtil::ReadDoubleVectorListFromStream(ioIStream, mMultiCameraCalibration.cc_right_error_list);
		CalibraFileUtil::ReadDoubleVectorListFromStream(ioIStream, mMultiCameraCalibration.kc_right_error_list);
		CalibraFileUtil::ReadDoubleVectorFromStream(ioIStream, mMultiCameraCalibration.alpha_c_right_error_list);

		CalibraFileUtil::ReadMatrixListFromStream(ioIStream, mMultiCameraCalibration.T_error_list);
		CalibraFileUtil::ReadMatrixListFromStream(ioIStream, mMultiCameraCalibration.om_error_list);

		CalibrationResultNode::ReadFromStream(ioIStream);
	}

	virtual void	WriteToStream(std::ostream &ioOStream, bool inIsSuperclass) const
	{
		WriteObjectDataHeader(ioOStream, CalcStreamDataSectionSize(),
									MULTI_CAMERA_RESULT_NODE_OBJECT_ID, inIsSuperclass);

		CalibraFileUtil::WriteDoubleToStream(ioOStream, mMultiCameraCalibration.mImageWidth);
		CalibraFileUtil::WriteDoubleToStream(ioOStream, mMultiCameraCalibration.mImageHeight);

		CalibraFileUtil::WriteIntToStream(ioOStream, mMultiCameraCalibration.mCenterCameraIndex);

		WriteSingleCameraResultListToStream(ioOStream, mMultiCameraCalibration.mCalibrationResults);

		CalibraFileUtil::WriteMatrixListToStream(ioOStream, mMultiCameraCalibration.T_list);
		CalibraFileUtil::WriteMatrixListToStream(ioOStream, mMultiCameraCalibration.om_list);
		CalibraFileUtil::WriteMatrixListToStream(ioOStream, mMultiCameraCalibration.R_list);

		CalibraFileUtil::WriteDoubleVectorListToStream(ioOStream, mMultiCameraCalibration.fc_left_list);
		CalibraFileUtil::WriteDoubleVectorListToStream(ioOStream, mMultiCameraCalibration.cc_left_list);
		CalibraFileUtil::WriteDoubleVectorListToStream(ioOStream, mMultiCameraCalibration.kc_left_list);
		CalibraFileUtil::WriteDoubleVectorToStream(ioOStream, mMultiCameraCalibration.alpha_c_left_list);

		CalibraFileUtil::WriteDoubleVectorListToStream(ioOStream, mMultiCameraCalibration.fc_right_list);
		CalibraFileUtil::WriteDoubleVectorListToStream(ioOStream, mMultiCameraCalibration.cc_right_list);
		CalibraFileUtil::WriteDoubleVectorListToStream(ioOStream, mMultiCameraCalibration.kc_right_list);
		CalibraFileUtil::WriteDoubleVectorToStream(ioOStream, mMultiCameraCalibration.alpha_c_right_list);

		CalibraFileUtil::WriteDoubleVectorListToStream(ioOStream, mMultiCameraCalibration.fc_left_error_list);
		CalibraFileUtil::WriteDoubleVectorListToStream(ioOStream, mMultiCameraCalibration.cc_left_error_list);
		CalibraFileUtil::WriteDoubleVectorListToStream(ioOStream, mMultiCameraCalibration.kc_left_error_list);
		CalibraFileUtil::WriteDoubleVectorToStream(ioOStream, mMultiCameraCalibration.alpha_c_left_error_list);

		CalibraFileUtil::WriteDoubleVectorListToStream(ioOStream, mMultiCameraCalibration.fc_right_error_list);
		CalibraFileUtil::WriteDoubleVectorListToStream(ioOStream, mMultiCameraCalibration.cc_right_error_list);
		CalibraFileUtil::WriteDoubleVectorListToStream(ioOStream, mMultiCameraCalibration.kc_right_error_list);
		CalibraFileUtil::WriteDoubleVectorToStream(ioOStream, mMultiCameraCalibration.alpha_c_right_error_list);

		CalibraFileUtil::WriteMatrixListToStream(ioOStream, mMultiCameraCalibration.T_error_list);
		CalibraFileUtil::WriteMatrixListToStream(ioOStream, mMultiCameraCalibration.om_error_list);

		CalibrationResultNode::WriteToStream(ioOStream, true);
	}

	MultiCameraCalibration	mMultiCameraCalibration;

private:
	static void	ReadSingleCameraResultListFromStream(std::istream &ioIStream,
					std::vector<SingleCameraResult > &outList)
	{
		unsigned int	size;

		ioIStream.read((char *)&size, sizeof(unsigned int));
		outList.resize(size);

		std::vector<SingleCameraResult >::iterator	it;
		for (it = outList.begin(); it != outList.end(); ++it)
			ReadSingleCameraResultFromStream(ioIStream, *it);
	}

	static void	ReadSingleCameraResultFromStream(std::istream &ioIStream, SingleCameraResult &outResult)
	{
		CalibraFileUtil::ReadMatrixListFromStream(ioIStream, outResult.X_list);
		CalibraFileUtil::ReadMatrixListFromStream(ioIStream, outResult.x_list);
		CalibraFileUtil::ReadMatrixListFromStream(ioIStream, outResult.omc_list);
		CalibraFileUtil::ReadMatrixListFromStream(ioIStream, outResult.Tc_list);

		CalibraFileUtil::ReadDoubleVectorFromStream(ioIStream, outResult.fc);
		CalibraFileUtil::ReadDoubleVectorFromStream(ioIStream, outResult.cc);
		CalibraFileUtil::ReadDoubleVectorFromStream(ioIStream, outResult.kc);
		CalibraFileUtil::ReadDoubleFromStream(ioIStream, &outResult.alpha_c);
	}

	static void	WriteSingleCameraResultListToStream(std::ostream &ioOStream,
					const std::vector<SingleCameraResult > &inList)
	{
		unsigned int	size = (unsigned int )inList.size();

		ioOStream.write((char *)&size, sizeof(unsigned int));

		std::vector<SingleCameraResult >::const_iterator	it;
		for (it = inList.begin(); it != inList.end(); ++it)
			WriteSingleCameraResultToStream(ioOStream, *it);
	}

	static void	WriteSingleCameraResultToStream(std::ostream &ioOStream, const SingleCameraResult &inResult)
	{
		CalibraFileUtil::WriteMatrixListToStream(ioOStream, inResult.X_list);
		CalibraFileUtil::WriteMatrixListToStream(ioOStream, inResult.x_list);
		CalibraFileUtil::WriteMatrixListToStream(ioOStream, inResult.omc_list);
		CalibraFileUtil::WriteMatrixListToStream(ioOStream, inResult.Tc_list);

		CalibraFileUtil::WriteDoubleVectorToStream(ioOStream, inResult.fc);
		CalibraFileUtil::WriteDoubleVectorToStream(ioOStream, inResult.cc);
		CalibraFileUtil::WriteDoubleVectorToStream(ioOStream, inResult.kc);
		CalibraFileUtil::WriteDoubleToStream(ioOStream, inResult.alpha_c);
	}

	static unsigned int	CalcSingleCameraResultListStreamSize(
				const std::vector<SingleCameraResult > &inList)
	{
		unsigned int	size;
		std::vector<SingleCameraResult >::const_iterator	it;

		size = sizeof(unsigned int);
		for (it = inList.begin(); it != inList.end(); ++it)
			size += CalcSingleCameraResultStreamSize(*it);

		return size;
	}

	static unsigned int	CalcSingleCameraResultStreamSize(const SingleCameraResult &inResult)
	{
		unsigned int	size = 0;

		size += CalibraFileUtil::CalcMatrixListStreamSize(inResult.X_list);
		size += CalibraFileUtil::CalcMatrixListStreamSize(inResult.x_list);
		size += CalibraFileUtil::CalcMatrixListStreamSize(inResult.omc_list);
		size += CalibraFileUtil::CalcMatrixListStreamSize(inResult.Tc_list);

		size += CalibraFileUtil::CalcDoubleVectorStreamSize(inResult.fc);
		size += CalibraFileUtil::CalcDoubleVectorStreamSize(inResult.cc);
		size += CalibraFileUtil::CalcDoubleVectorStreamSize(inResult.kc);
		size += CalibraFileUtil::CalcDoubleStreamSize();	// alpha_c

		return size;
	}

	unsigned int	CalcStreamDataSectionSize() const
	{
		unsigned int	size = OBJECT_HEADER_LEN;

		size += CalibraFileUtil::CalcDoubleStreamSize();	// mImageWidth
		size += CalibraFileUtil::CalcDoubleStreamSize();	// mImageHeight

		size += CalibraFileUtil::CalcIntStreamSize();		// mCenterCameraIndex

		size += CalcSingleCameraResultListStreamSize(mMultiCameraCalibration.mCalibrationResults);

		size += CalibraFileUtil::CalcMatrixListStreamSize(mMultiCameraCalibration.T_list);
		size += CalibraFileUtil::CalcMatrixListStreamSize(mMultiCameraCalibration.om_list);
		size += CalibraFileUtil::CalcMatrixListStreamSize(mMultiCameraCalibration.R_list);

		size += CalibraFileUtil::CalcDoubleVectorListStreamSize(mMultiCameraCalibration.fc_left_list);
		size += CalibraFileUtil::CalcDoubleVectorListStreamSize(mMultiCameraCalibration.cc_left_list);
		size += CalibraFileUtil::CalcDoubleVectorListStreamSize(mMultiCameraCalibration.kc_left_list);
		size += CalibraFileUtil::CalcDoubleVectorStreamSize(mMultiCameraCalibration.alpha_c_left_list);

		size += CalibraFileUtil::CalcDoubleVectorListStreamSize(mMultiCameraCalibration.fc_right_list);
		size += CalibraFileUtil::CalcDoubleVectorListStreamSize(mMultiCameraCalibration.cc_right_list);
		size += CalibraFileUtil::CalcDoubleVectorListStreamSize(mMultiCameraCalibration.kc_right_list);
		size += CalibraFileUtil::CalcDoubleVectorStreamSize(mMultiCameraCalibration.alpha_c_right_list);

		size += CalibraFileUtil::CalcDoubleVectorListStreamSize(mMultiCameraCalibration.fc_left_error_list);
		size += CalibraFileUtil::CalcDoubleVectorListStreamSize(mMultiCameraCalibration.cc_left_error_list);
		size += CalibraFileUtil::CalcDoubleVectorListStreamSize(mMultiCameraCalibration.kc_left_error_list);
		size += CalibraFileUtil::CalcDoubleVectorStreamSize(mMultiCameraCalibration.alpha_c_left_error_list);

		size += CalibraFileUtil::CalcDoubleVectorListStreamSize(mMultiCameraCalibration.fc_right_error_list);
		size += CalibraFileUtil::CalcDoubleVectorListStreamSize(mMultiCameraCalibration.cc_right_error_list);
		size += CalibraFileUtil::CalcDoubleVectorListStreamSize(mMultiCameraCalibration.kc_right_error_list);
		size += CalibraFileUtil::CalcDoubleVectorStreamSize(mMultiCameraCalibration.alpha_c_right_error_list);

		size += CalibraFileUtil::CalcMatrixListStreamSize(mMultiCameraCalibration.T_error_list);
		size += CalibraFileUtil::CalcMatrixListStreamSize(mMultiCameraCalibration.om_error_list);

		return size;
	}
};

#endif	// #ifdef __SINGLECAMERA_RESULT_NODE_H

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
	\date		2007/10/31
	\brief

	Description...
*/
#ifndef __STEREO_CAMERA_CALIBRATION_NODE_H
#define __STEREO_CAMERA_CALIBRATION_NODE_H


// -----------------------------------------------------------------------------
// 	include files
// -----------------------------------------------------------------------------
#include <string>
#include <vector>
#include "CalibrationNode.hpp"

// -----------------------------------------------------------------------------
// 	macros
// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------
//	StereoCameraCalibrationNode class
// -----------------------------------------------------------------------------
//
class StereoCameraCalibrationNode : public CalibrationNode
{
public:
	StereoCameraCalibrationNode()
	{
	}

	StereoCameraCalibrationNode(const std::wstring &inCalibrationName)
		: CalibrationNode(inCalibrationName)
	{
	}

	virtual bool	ChildNodeCheck(CalibraNode *inNode) const
	{
//			if (inNode is CalibrationNode)
//				return true;
//
		return true;
	}

	virtual void	ReadFromStream(std::istream &ioIStream)
	{
		unsigned int	size, objectID;
		bool	isSuperclass;

		ReadObjectDataHeader(ioIStream, &size, &objectID, &isSuperclass);

		if (objectID != STEREO_CAMERA_CALIBRATION_NODE_OBJECT_ID)
			throw std::runtime_error("OBJECT_ID dose not mutch: in StereoCameraCalibrationNode::ReadFromStream");

		CalibrationNode::ReadFromStream(ioIStream);
	}

	virtual void	WriteToStream(std::ostream &ioOStream, bool inIsSuperclass) const
	{
		WriteObjectDataHeader(ioOStream, CalcStreamDataSectionSize(),
									STEREO_CAMERA_CALIBRATION_NODE_OBJECT_ID, inIsSuperclass);

		CalibrationNode::WriteToStream(ioOStream, true);
	}


private:
	//std::vector<CornerFinder>	mCornerFinderList;

	unsigned int	CalcStreamDataSectionSize() const
	{
		unsigned int	size = OBJECT_HEADER_LEN;

		//size += CalibraFileUtil::CalcStringStreamSize(mImageName);
		return size;
	}
};

#endif	// #ifdef __STEREO_CAMERA_CALIBRATION_NODE_H

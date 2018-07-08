// =============================================================================
//  CalibrationResultNode.hpp
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
	\file		CalibrationResultNode.hpp
	\author		Dairoku Sekiguchi
	\version	1.0
	\date		2007/10/31
	\brief

	Description...
*/
#ifndef __CALIBRATION_RESULT_NODE_H
#define __CALIBRATION_RESULT_NODE_H


// -----------------------------------------------------------------------------
// 	include files
// -----------------------------------------------------------------------------
#include <string>
#include <vector>
#include "CalibraNode.hpp"

// -----------------------------------------------------------------------------
// 	macros
// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------
//	CalibrationResultNode class
// -----------------------------------------------------------------------------
//
class CalibrationResultNode : public CalibraNode
{
public:
	const static int		DEFAULT_IMAGE_WIDTH		=	640;
	const static int		DEFAULT_IMAGE_HEIGHT	=	480;

	CalibrationResultNode()
	{
	}

	CalibrationResultNode(const std::wstring &inCalibrationResultName)
	{
		mCalibrationResultName = inCalibrationResultName;
	}

	virtual bool	IsLeafNode() const
	{
		return false;
	}

	virtual const std::wstring	&GetName() const
	{
		return mCalibrationResultName;
	}

	virtual void	ReadFromStream(std::istream &ioIStream)
	{
		unsigned int	size, objectID;
		bool	isSuperclass;

		ReadObjectDataHeader(ioIStream, &size, &objectID, &isSuperclass);

		if (objectID != CALIBRATION_RESULT_NODE_OBJECT_ID)
			throw std::runtime_error("OBJECT_ID dose not mutch: in CalibrationNode::ReadFromStream");

		CalibraFileUtil::ReadStringFromStream(ioIStream, mCalibrationResultName);
		CalibraNode::ReadFromStream(ioIStream);
	}

	virtual void	WriteToStream(std::ostream &ioOStream, bool inIsSuperclass) const
	{
		WriteObjectDataHeader(ioOStream, CalcStreamDataSectionSize(),
									CALIBRATION_RESULT_NODE_OBJECT_ID, inIsSuperclass);

		CalibraFileUtil::WriteStringToStream(ioOStream, mCalibrationResultName);
		CalibraNode::WriteToStream(ioOStream, true);
	}

private:
	std::wstring		mCalibrationResultName;

	unsigned int	CalcStreamDataSectionSize() const
	{
		unsigned int	size = OBJECT_HEADER_LEN;

		size += CalibraFileUtil::CalcStringStreamSize(mCalibrationResultName);
		return size;
	}
};

#endif	// #ifdef __CALIBRATION_RESULT_NODE_H

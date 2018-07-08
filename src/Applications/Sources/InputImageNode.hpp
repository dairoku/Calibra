// =============================================================================
//  InputImageNode.hpp
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
	\file		InputImageNode.hpp
	\author		Dairoku Sekiguchi
	\version	1.0
	\date		2007/10/31
	\brief

	Description...
*/
#ifndef __INPUT_IMAGE_NODE_H
#define __INPUT_IMAGE_NODE_H


// -----------------------------------------------------------------------------
// 	include files
// -----------------------------------------------------------------------------
#include <string>
#include <vector>
#include "ImageNode.hpp"
#include "ImageData.hpp"
#include "BoostIncludes.hpp"
#include "CornerFinder.hpp"


// -----------------------------------------------------------------------------
// 	macros
// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------
//	InputImageNode class
// -----------------------------------------------------------------------------
//
class InputImageNode : public ImageNode
{
public:
	const static int		INVALID_METHOD				= 0;
	const static int		MANUAL_EXTRACTION_METHOD	= 1;
	const static int		GRID_EXTRACTION_METHOD		= 10;
	const static int		CORNER_FIND_METHOD			= 11;
	const static int		REPROJECTION_METHOD			= 12;

	InputImageNode()
	{
		Initialize();
	}

	InputImageNode(const std::wstring &inImageName, const std::wstring &inImageFilePath)
		: ImageNode(inImageName, inImageFilePath)
	{
		Initialize();
	}

	virtual ~InputImageNode()
	{
		Destroy();
	}

	void	EnableGridNumAutoDetector(bool inEnable)
	{
		mEnableGridNumAutoDetector = inEnable;
	}

	bool	IsGridNumAutoDetectorEnabled()
	{
		return mEnableGridNumAutoDetector;
	}

	//	Final Window size used in the conrer finder will be:@finalSize = input * 2 + 1
	void	SetCornerFinderWindowSize(int inX, int inY)
	{
		mCornerFinderWindowX = inX;
		mCornerFinderWindowY = inY;
	}

	void	GetCornerFinderWindowSize(int *outX, int *outY)
	{
		*outX = mCornerFinderWindowX;
		*outY = mCornerFinderWindowY;
	}

	void	SetGridRealSize(double inX, double inY)
	{
		mGridRealSizeX = inX;
		mGridRealSizeY = inY;
	}

	void	GetGridRealSize(double *outX, double *outY)
	{
		*outX = mGridRealSizeX;
		*outY = mGridRealSizeY;
	}

	void	SetGridExtractorInputNum(int inNum) { mGridInputNum = inNum; };
	int		GetGridExtractorInputNum() { return mGridInputNum; };

	void	SetGridExtractorInput(int inIndex, double inX, double inY)
	{
		//	must check inIndex
		mGridExtractorInput(0, inIndex) = inX;
		mGridExtractorInput(1, inIndex) = inY;
	}

	void	GetGridExtractorInput(int inIndex, double *outX, double *outY)
	{
		//	must check inIndex
		*outX = mGridExtractorInput(0, inIndex);
		*outY = mGridExtractorInput(1, inIndex);
	}

	void	ClearAllExtractedCorner()
	{
		mCornerFinderCenter.resize(0, 0);
		mExtractedCorner.resize(0, 0);
		mExtractedCornerWorldCoordinate.resize(0, 0);
		mCornerFinderWindowSize.resize(0, 0);
		mExtractionResult.resize(0);
		mExtractionMethod.resize(0);
	}

	int		GetExtractedCornerNum() { return (int )(mExtractionResult.size()); }

	void	SetCornerFinderCenter(int inIndex, double inX, double inY, int inMethod)
	{
		if (inIndex >= GetExtractedCornerNum())
			return;

		mCornerFinderCenter(0, inIndex) = inX;
		mCornerFinderCenter(1, inIndex) = inY;
		mExtractionMethod(inIndex) = inMethod;
	}

	void	GetCornerFinderCenter(int inIndex, double *outX, double *outY)
	{
		if (inIndex >= GetExtractedCornerNum())
			return;

		*outX = mCornerFinderCenter(0, inIndex);
		*outY = mCornerFinderCenter(1, inIndex);
	}

	void	GetExtractedCorner(int inIndex, double *outX, double *outY)
	{
		if (inIndex >= GetExtractedCornerNum())
			return;

		*outX = mExtractedCorner(0, inIndex);
		*outY = mExtractedCorner(1, inIndex);
	}

	void	GetExtractedCornerWindowSize(int inIndex, int *outX, int *outY)
	{
		if (inIndex >= GetExtractedCornerNum())
			return;

		*outX = (int )mCornerFinderWindowSize(0, inIndex);
		*outY = (int )mCornerFinderWindowSize(1, inIndex);
	}

	int	GetExtractionResult(int inIndex)
	{
		if (inIndex >= GetExtractedCornerNum())
			return false;

		return mExtractionResult(inIndex);
	}

	int		GetExtractionMethod(int inIndex)
	{
		if (inIndex >= GetExtractedCornerNum())
			return INVALID_METHOD;

		return mExtractionMethod(inIndex);
	}

	void	ExecCornerFinder(ImageData &inImage, int inIndex)
	{
		if (inIndex >= GetExtractedCornerNum())
			return;

		double	x, y;
		ublas::matrix<unsigned char, ublas::column_major>	I;
		inImage.GetuBLASMatrix(I);

		//	Caution!! this function expect Matlab corrdinate system as input and output
		mExtractionResult(inIndex) = CornerFinder::findCorner(
			I,
			mCornerFinderCenter(0, inIndex), mCornerFinderCenter(1, inIndex),
			(int )mCornerFinderWindowSize(0, inIndex), (int )mCornerFinderWindowSize(1, inIndex),
			&x, &y);

		mExtractedCorner(0, inIndex) = x - 1;	// Be carefull!! 
		mExtractedCorner(1, inIndex) = y - 1;
	}

	void	ExecGridExtractor(ImageData &inImage)
	{
		if (GetGridExtractorInputNum()!= 4)
			return;

		ublas::matrix<double, ublas::column_major>	input = mGridExtractorInput;
		ublas::matrix<unsigned char, ublas::column_major>	I;
		inImage.GetuBLASMatrix(I);

		int	n_sq_x1, n_sq_x2, n_sq_y1, n_sq_y2;

		if (mEnableGridNumAutoDetector)
		{
			CornerFinder::findRectangle(I, 
				input,
				5, 5, &n_sq_x1, &n_sq_x2, &n_sq_y1, &n_sq_y2);
		}
		else
		{
			n_sq_x1 = mGridNumX;
			n_sq_y1 = mGridNumY;
		}
		
		int	extractedCornerNum = (n_sq_x1 + 1) * (n_sq_y1 + 1);
		mCornerFinderCenter.resize(2, extractedCornerNum);
		mExtractedCorner.resize(2, extractedCornerNum);
		mExtractedCornerWorldCoordinate.resize(3, extractedCornerNum);
		mCornerFinderWindowSize.resize(2, extractedCornerNum);
		mExtractionResult.resize(extractedCornerNum);
		mExtractionMethod.resize(extractedCornerNum);

		CornerFinder::findGrid(
			I,
			input,
			mCornerFinderWindowX, mCornerFinderWindowY,
			mGridRealSizeX, mGridRealSizeY,
			n_sq_x1, n_sq_y1,
			mCornerFinderCenter, mExtractedCorner,
			mExtractedCornerWorldCoordinate, mExtractionResult);

		for (int i = 0; i < extractedCornerNum; i++)
		{
			mExtractionMethod(i) = GRID_EXTRACTION_METHOD;
			mCornerFinderWindowSize(0, i) = mCornerFinderWindowX;
			mCornerFinderWindowSize(1, i) = mCornerFinderWindowY;
		}
	}

	virtual void	ReadFromStream(std::istream &ioIStream)
	{
		unsigned int	size, objectID;
		bool	isSuperclass;

		ReadObjectDataHeader(ioIStream, &size, &objectID, &isSuperclass);

		if (objectID != INPUT_IMAGE_NODE_OBJECT_ID)
			throw std::runtime_error("OBJECT_ID dose not mutch: in InputImageNode::ReadFromStream");

		CalibraFileUtil::ReadMatrixFromStream(ioIStream, mGridExtractorInput);
		CalibraFileUtil::ReadMatrixFromStream(ioIStream, mCornerFinderCenter);
		CalibraFileUtil::ReadMatrixFromStream(ioIStream, mExtractedCorner);
		CalibraFileUtil::ReadMatrixFromStream(ioIStream, mExtractedCornerWorldCoordinate);
		CalibraFileUtil::ReadMatrixFromStream(ioIStream, mCornerFinderWindowSize);

		CalibraFileUtil::ReadIntVectorFromStream(ioIStream, mExtractionMethod);
		CalibraFileUtil::ReadIntVectorFromStream(ioIStream, mExtractionResult);

		CalibraFileUtil::ReadIntFromStream(ioIStream, &mGridInputNum);
		CalibraFileUtil::ReadIntFromStream(ioIStream, &mGridNumX);
		CalibraFileUtil::ReadIntFromStream(ioIStream, &mGridNumY);
		CalibraFileUtil::ReadIntFromStream(ioIStream, &mCornerFinderWindowX); 
		CalibraFileUtil::ReadIntFromStream(ioIStream, &mCornerFinderWindowY); 

		CalibraFileUtil::ReadDoubleFromStream(ioIStream, &mGridRealSizeX);
		CalibraFileUtil::ReadDoubleFromStream(ioIStream, &mGridRealSizeY);

		CalibraFileUtil::ReadBoolFromStream(ioIStream, &mEnableGridNumAutoDetector);

		ImageNode::ReadFromStream(ioIStream);
	}

	virtual void	WriteToStream(std::ostream &ioOStream, bool inIsSuperclass) const
	{
		WriteObjectDataHeader(ioOStream, CalcStreamDataSectionSize(),
									INPUT_IMAGE_NODE_OBJECT_ID, inIsSuperclass);

		CalibraFileUtil::WriteMatrixToStream(ioOStream, mGridExtractorInput);
		CalibraFileUtil::WriteMatrixToStream(ioOStream, mCornerFinderCenter);
		CalibraFileUtil::WriteMatrixToStream(ioOStream, mExtractedCorner);
		CalibraFileUtil::WriteMatrixToStream(ioOStream, mExtractedCornerWorldCoordinate);
		CalibraFileUtil::WriteMatrixToStream(ioOStream, mCornerFinderWindowSize);

		CalibraFileUtil::WriteIntVectorToStream(ioOStream, mExtractionMethod);
		CalibraFileUtil::WriteIntVectorToStream(ioOStream, mExtractionResult);

		CalibraFileUtil::WriteIntToStream(ioOStream, mGridInputNum);
		CalibraFileUtil::WriteIntToStream(ioOStream, mGridNumX);
		CalibraFileUtil::WriteIntToStream(ioOStream, mGridNumY);
		CalibraFileUtil::WriteIntToStream(ioOStream, mCornerFinderWindowX); 
		CalibraFileUtil::WriteIntToStream(ioOStream, mCornerFinderWindowY); 

		CalibraFileUtil::WriteDoubleToStream(ioOStream, mGridRealSizeX);
		CalibraFileUtil::WriteDoubleToStream(ioOStream, mGridRealSizeY);

		CalibraFileUtil::WriteBoolToStream(ioOStream, mEnableGridNumAutoDetector);

		ImageNode::WriteToStream(ioOStream, true);
	}

	const ublas::matrix<double, ublas::column_major> &GetExtractedCornerMatrix() { return mExtractedCorner; };
	const ublas::matrix<double, ublas::column_major> &GetExtractedCornerWorldCoordinateMatrix() { return mExtractedCornerWorldCoordinate; };

private:
	ublas::matrix<double, ublas::column_major>	mGridExtractorInput;
	ublas::matrix<double, ublas::column_major>	mCornerFinderCenter;
	ublas::matrix<double, ublas::column_major>	mExtractedCorner;
	ublas::matrix<double, ublas::column_major>	mExtractedCornerWorldCoordinate;
	ublas::matrix<double, ublas::column_major>	mCornerFinderWindowSize;
	ublas::vector<int>	mExtractionMethod;
	ublas::vector<int>	mExtractionResult;
	int	mGridInputNum;
	int	mGridNumX, mGridNumY;
	int	mCornerFinderWindowX, mCornerFinderWindowY;
	double	mGridRealSizeX, mGridRealSizeY;
	bool	mEnableGridNumAutoDetector;

	void	Initialize()
	{
		mGridInputNum = 0;
		mGridExtractorInput.resize(2, 4);

		mGridNumX = 12;
		mGridNumY = 12;
		mGridRealSizeX = 30.0;
		mGridRealSizeY = 30.0;
		mCornerFinderWindowX = 5;
		mCornerFinderWindowY = 5;

		mEnableGridNumAutoDetector = true;
	}

	void	Destroy()
	{
	}

	unsigned int	CalcStreamDataSectionSize() const
	{
		unsigned int	size = OBJECT_HEADER_LEN;

		size += CalibraFileUtil::CalcMatrixStreamSize(mGridExtractorInput);
		size += CalibraFileUtil::CalcMatrixStreamSize(mCornerFinderCenter);
		size += CalibraFileUtil::CalcMatrixStreamSize(mExtractedCorner);
		size += CalibraFileUtil::CalcMatrixStreamSize(mExtractedCornerWorldCoordinate);
		size += CalibraFileUtil::CalcMatrixStreamSize(mCornerFinderWindowSize);

		size += CalibraFileUtil::CalcIntVectorStreamSize(mExtractionMethod);
		size += CalibraFileUtil::CalcIntVectorStreamSize(mExtractionResult);

		size += CalibraFileUtil::CalcIntStreamSize();	// mGridInputNum
		size += CalibraFileUtil::CalcIntStreamSize();	// mGridNumX
		size += CalibraFileUtil::CalcIntStreamSize();	// mGridNumY
		size += CalibraFileUtil::CalcIntStreamSize();	// mCornerFinderWindowX
		size += CalibraFileUtil::CalcIntStreamSize();	// mCornerFinderWindowY

		size += CalibraFileUtil::CalcDoubleStreamSize();	// mGridRealSizeX
		size += CalibraFileUtil::CalcDoubleStreamSize();	// mGridRealSizeY

		size += CalibraFileUtil::CalcBoolStreamSize();	// mEnableGridNumAutoDetector

		return size;
	}
};

#endif	// #ifdef __INPUT_IMAGE_NODE_H

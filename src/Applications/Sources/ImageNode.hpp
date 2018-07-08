// =============================================================================
//  ImageNode.hpp
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
	\file		ImageNode.hpp
	\author		Dairoku Sekiguchi
	\version	1.0
	\date		2007/10/31
	\brief

	Description...
*/
#ifndef __IMAGE_NODE_H
#define __IMAGE_NODE_H


// -----------------------------------------------------------------------------
// 	include files
// -----------------------------------------------------------------------------
#include <string>
#include <vector>
#include "FilePath.hpp"
#include "CalibraNode.hpp"
#include "ProjectNode.hpp"

// -----------------------------------------------------------------------------
// 	macros
// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------
//	ImageNode class
// -----------------------------------------------------------------------------
//
class ImageNode : public CalibraNode
{
public:
	ImageNode()
	{
	}

	ImageNode(const std::wstring &inImageName, const std::wstring &inImageFilePath)
	{
		mImageName = inImageName;
		mImageFilePath = inImageFilePath;
	}

	virtual bool	IsLeafNode() const
	{
		return true;
	}

	virtual bool	ChildNodeCheck(CalibraNode *inNode) const
	{
		return false;
	}

	virtual void	SetName(const std::wstring &inImageName)
	{
		mImageName = inImageName;
	}

	virtual const std::wstring	&GetName() const
	{
		return mImageName;
	}

	virtual void	SetFilePath(const std::wstring &inImageFilePath)
	{
		mImageFilePath = inImageFilePath;
	}

	virtual const std::wstring	&GetFilePath() const
	{
		return mImageFilePath;
	}

	virtual void	SetCachedFilePath(const std::wstring &inImageFilePath)
	{
		mCachedImageFilePath = inImageFilePath;
	}

	virtual const std::wstring	&GetCachedFilePath() const
	{
		return mCachedImageFilePath;
	}

	virtual const std::wstring	&GetDescription() const
	{
		return mImageDescription;
	}

	virtual void	ReadPostProcess()
	{
		const std::wstring	&projectFilePath = ((ProjectNode *)GetRootNode())->GetFilePath();
		std::wstring	projectPath = FilePath::ExtractPath(projectFilePath.c_str());
		mCachedImageFilePath = FilePath::BuildAbsoluteFilePath(
										projectPath.c_str(), mImageFilePath.c_str());
	}

	virtual void	WritePreprocess()
	{
		const std::wstring	&projectFilePath = ((ProjectNode *)GetRootNode())->GetFilePath();
		std::wstring	projectPath = FilePath::ExtractPath(projectFilePath.c_str());
		mImageFilePath = FilePath::BuildRelativeFilePath(
										projectPath.c_str(), mCachedImageFilePath.c_str());
	}

	virtual void	ReadFromStream(std::istream &ioIStream)
	{
		unsigned int	size, objectID;
		bool	isSuperclass;

		ReadObjectDataHeader(ioIStream, &size, &objectID, &isSuperclass);

		if (objectID != IMAGE_NODE_OBJECT_ID)
			throw std::runtime_error("OBJECT_ID dose not mutch: in ImageNode::ReadFromStream");

		CalibraFileUtil::ReadStringFromStream(ioIStream, mImageName);
		CalibraFileUtil::ReadStringFromStream(ioIStream, mImageFilePath);
		CalibraFileUtil::ReadStringFromStream(ioIStream, mImageDescription);

		CalibraNode::ReadFromStream(ioIStream);
	}

	virtual void	WriteToStream(std::ostream &ioOStream, bool inIsSuperclass) const
	{
		WriteObjectDataHeader(ioOStream, CalcStreamDataSectionSize(),
									IMAGE_NODE_OBJECT_ID, inIsSuperclass);

		CalibraFileUtil::WriteStringToStream(ioOStream, mImageName);
		CalibraFileUtil::WriteStringToStream(ioOStream, mImageFilePath);
		CalibraFileUtil::WriteStringToStream(ioOStream, mImageDescription);

		CalibraNode::WriteToStream(ioOStream, true);
	}

private:
	std::wstring		mImageName;
	std::wstring		mImageFilePath;
	std::wstring		mImageDescription;

	std::wstring		mCachedImageFilePath;

	unsigned int	CalcStreamDataSectionSize() const
	{
		unsigned int	size = OBJECT_HEADER_LEN;

		size += CalibraFileUtil::CalcStringStreamSize(mImageName);
		size += CalibraFileUtil::CalcStringStreamSize(mImageFilePath);
		size += CalibraFileUtil::CalcStringStreamSize(mImageDescription);
		return size;
	}
};

#endif	// #ifdef __IMAGE_NODE_H

// =============================================================================
//  CalibraNode.hpp
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
	\file		CalibraNode.hpp
	\author		Dairoku Sekiguchi
	\version	1.0
	\date		2007/10/31
	\brief

	Description...
*/
#ifndef __CALIBRA_NODE_H
#define __CALIBRA_NODE_H


// -----------------------------------------------------------------------------
// 	include files
// -----------------------------------------------------------------------------
#include <string>
#include <vector>
#include "BoostIncludes.hpp"
#include "CameraCalibration.hpp"
#include "StereoCalibration.hpp"
#include "MultiCameraCalibration.hpp"
#include "CalibraFileUtil.hpp"


// -----------------------------------------------------------------------------
// 	macros
// -----------------------------------------------------------------------------
#ifndef null
#define	null	0
#endif


// -----------------------------------------------------------------------------
//	CalibraNode class
// -----------------------------------------------------------------------------
//
class CalibraNode
{
public:
	//	File Data Section
	const static unsigned int		DATA_SECTION_TYPE_LEN				= 4;
	const static unsigned int		DATA_SECTION_SIZE_LEN				= 4;
	const static unsigned int		DATA_SECTION_HEADER_LEN				= DATA_SECTION_TYPE_LEN + DATA_SECTION_SIZE_LEN;

	//	File Object Data Section
	const static unsigned int		OBJECT_ID_LEN						= 4;
	const static unsigned int		NODE_ID_LEN							= 4;
	const static unsigned int		PARENT_NODE_ID_LEN					= 4;
	const static unsigned int		OBJECT_HEADER_LEN					= DATA_SECTION_HEADER_LEN + OBJECT_ID_LEN + NODE_ID_LEN + PARENT_NODE_ID_LEN;

	const static unsigned int		CALIBRA_NODE_SECTION_LEN			= OBJECT_HEADER_LEN;

	enum DataSectionType
	{
									NULL_TYPE			= 0,
									OBJECT_DATA_TYPE,
									OBJECT_SUPERCLASS_DATA_TYPE
	};

	enum ObjectID
	{
									NULL_ID				= 0,
									CALIBRA_NODE_OBJECT_ID,
									CALIBRATION_NODE_OBJECT_ID,
									CALIBRATION_RESULT_NODE_OBJECT_ID,
									IMAGE_FOLDER_NODE_OBJECT_ID,
									IMAGE_NODE_OBJECT_ID,
									INPUT_IMAGE_NODE_OBJECT_ID,
									PROJECT_NODE_OBJECT_ID,
									SINGLE_CAMERA_CALIBRATION_NODE_OBJECT_ID,
									SINGLE_CAMERA_RESULT_NODE_OBJECT_ID,
									STEREO_CAMERA_CALIBRATION_NODE_OBJECT_ID,
									STEREO_CAMERA_RESULT_NODE_OBJECT_ID,
									MULTI_CAMERA_CALIBRATION_NODE_OBJECT_ID,
									MULTI_CAMERA_RESULT_NODE_OBJECT_ID
	};

	CalibraNode()
	{
		mID = GetUniqueID();
		mParentNode = null;
	};

	virtual ~CalibraNode() {};

	virtual bool				IsLeafNode() const = 0;
	virtual bool				ChildNodeCheck(CalibraNode *inNode) const = 0;
	virtual const std::wstring	&GetName() const = 0;

	int	GetID()	const
	{
		return mID;
	}

	CalibraNode	*GetParentNode()	const
	{
		return mParentNode;
	}

	bool HasParentNode()	const
	{
		if (mParentNode == null)
			return false;

		return true;
	}

	bool HasChildNode()	const
	{
		if (IsLeafNode() || mChildNodeList.size() == 0)
			return false;

		return true;
	}

	int GetChildNodeNum()	const
	{
		if (IsLeafNode() || mChildNodeList.size() == 0)
			return 0;

		return (int )mChildNodeList.size();
	}

	CalibraNode *AddChildNode(CalibraNode *inNode)
	{
		PrepareForNewChildNode(inNode);

		mChildNodeList.push_back(inNode);
		inNode->mParentNode = this;

		return inNode;
	}

	CalibraNode *InsertChildNode(int inIndex, CalibraNode *inNode)
	{
		PrepareForNewChildNode(inNode);

		mChildNodeList.insert(mChildNodeList.begin() + inIndex, inNode);
		inNode->mParentNode = this;

		return inNode;
	}

	CalibraNode *RemoveChildNode(CalibraNode *inNode)
	{
		if (HasChildNode() == false)
			return null;

		int	index;
		if (GetChildNodeIndex(inNode, &index) == false)
			return null;
	
		return RemoveChildNodeAt(index);
	}

	CalibraNode *RemoveChildNodeAt(int inIndex)
	{
		if (HasChildNode() == false)
			return null;

		CalibraNode *node = GetChildNode(inIndex);

		mChildNodeList.erase(mChildNodeList.begin() + inIndex);
		node->mParentNode = null;

		return node;
	}

	void RemoveAllChildNodes()
	{
		if (HasChildNode() == false)
			return;

		for (int i = GetChildNodeNum() - 1; i >= 0; i--)
			RemoveChildNodeAt(i);
	}

	std::vector<CalibraNode *>::const_iterator begin()	const
	{
		return mChildNodeList.begin();
	}

	std::vector<CalibraNode *>::const_iterator end()	const
	{
		return mChildNodeList.end();
	}

	CalibraNode *GetRootNode()
	{
		if (HasParentNode())
			return mParentNode->GetRootNode();

		return this;
	}

	CalibraNode *GetElderBrotherNode()
	{
		if (HasParentNode() == false)
			return null;
		return GetParentNode()->GetNextChildNode(this);
	}

	CalibraNode *GetYoungerBrotherNode()
	{
		if (HasParentNode() == false)
			return null;
		return GetParentNode()->GetPreviousChildNode(this);
	}

	CalibraNode *GetChildNode(int inIndex)
	{
		if (HasChildNode() == false)
			return null;

		if (inIndex < 0 || inIndex >= (int )mChildNodeList.size())
			return null;

		return mChildNodeList[inIndex];
	}

	CalibraNode *GetPreviousChildNode(CalibraNode *inNode)
	{
		int	index;

		if (GetChildNodeIndex(inNode, &index) == false)
			return null;

		return GetChildNode(index + 1);
	}

	CalibraNode *GetNextChildNode(CalibraNode *inNode)
	{
		int	index;

		if (GetChildNodeIndex(inNode, &index) == false)
			return null;

		return GetChildNode(index + 1);
	}

	CalibraNode	*GetChildNodeByName(const std::wstring &inName)
	{
		if (HasChildNode() == false)
			return null;

		std::vector<CalibraNode *>::const_iterator		it;

		for (it = mChildNodeList.begin(); it != mChildNodeList.end(); ++it)
			if ((*it)->GetName().compare(inName) == 0)
				return (*it);

		return null;
	}

	CalibraNode *GetChildNodeByID(int inID)
	{
		if (HasChildNode() == false)
			return null;

		std::vector<CalibraNode *>::const_iterator		it;

		for (it = mChildNodeList.begin(); it != mChildNodeList.end(); ++it)
			if ((*it)->GetID() == inID)
				return (*it);

		return null;
	}

	bool	CheckIfChildNode(CalibraNode *inNode)
	{
		int	index;

		return GetChildNodeIndex(inNode, &index);
	}

	bool	GetChildNodeIndex(CalibraNode *inNode, int *outIndex)
	{
		if (HasChildNode() == false)
			return false;

		std::vector<CalibraNode *>::const_iterator		it;
		
		*outIndex = 0;
		for (it = mChildNodeList.begin(); it != mChildNodeList.end(); ++it, (*outIndex)++)
			if ((*it) == inNode)
				return true;

		return false;
	}

	CalibraNode *FindNodeByID(int inID)
	{
		if (GetID() == inID)
			return this;

		std::vector<CalibraNode *>::const_iterator		it;

		for (it = mChildNodeList.begin(); it != mChildNodeList.end(); ++it)
		{
			CalibraNode	*node = (*it)->FindNodeByID(inID);
			if (node != null)
				return node;
		}

		return null;
	}

	virtual void	ReadPostProcess()
	{
	}

	virtual void	WritePreprocess()
	{
	}

	virtual void	ReadFromStream(std::istream &ioIStream)
	{
		unsigned int	size, objectID;
		bool	isSuperclass;

		ReadObjectDataHeader(ioIStream, &size, &objectID, &isSuperclass);
		if (objectID != CALIBRA_NODE_OBJECT_ID)
			throw std::runtime_error("OBJECT_ID dose not mutch: in CalibraNode::ReadFromStream");
	}

	virtual void	WriteToStream(std::ostream &ioOStream, bool inIsSuperclass) const
	{
		WriteObjectDataHeader(ioOStream, CALIBRA_NODE_SECTION_LEN, CALIBRA_NODE_OBJECT_ID, inIsSuperclass);
	}

	static void	CallReadPostProcessRecursively(CalibraNode *inNode)
	{
		if (inNode != null)
		{
			inNode->CallAllChildNodesReadPostProcessRecursively();
			inNode->ReadPostProcess();
		}
	}

	static void	CallWritePreprocessRecursively(CalibraNode *inNode)
	{
		if (inNode != null)
		{
			inNode->CallAllChildNodesWritePreprocessRecursively();
			inNode->WritePreprocess();
		}
	}

	static void	RenumberAllNodesIDRecursively(CalibraNode *inNode)
	{
		if (inNode != null)
		{
			inNode->RenumberAllChildNodesIDRecursively();
			inNode->RenumberID();
		}
	}

	static void	DeleteAllNodesRecursively(CalibraNode *inNode)
	{
		if (inNode != null)
		{
			if (inNode->HasParentNode())
				inNode->GetParentNode()->RemoveChildNode(inNode);

			inNode->DeleteAllChildNodesRecursively();
			delete inNode;
		}
	}

protected:
	void	ReadObjectDataHeader(std::istream &ioIStream, unsigned int *outDataSectionSize,
											unsigned int *outObjectID, bool *outIsSuperclass)
	{
		unsigned int	data;

		ioIStream.read((char *)&data, DATA_SECTION_TYPE_LEN);
		if (data == OBJECT_DATA_TYPE)
			*outIsSuperclass = false;
		else
			*outIsSuperclass = true;

		ioIStream.read((char *)outDataSectionSize, DATA_SECTION_SIZE_LEN);
		ioIStream.read((char *)outObjectID, OBJECT_ID_LEN);
		ioIStream.read((char *)&data, NODE_ID_LEN);
		SetTempID(data);
		ioIStream.read((char *)&data, PARENT_NODE_ID_LEN);
	}

	void	WriteObjectDataHeader(std::ostream &ioOStream, unsigned int inDataSectionSize,
											unsigned int inObjectID, bool inIsSuperclass) const
	{
		unsigned int	data;

		if (inIsSuperclass)
			data = OBJECT_SUPERCLASS_DATA_TYPE;
		else
			data = OBJECT_DATA_TYPE;
		ioOStream.write((char *)&data, DATA_SECTION_TYPE_LEN);
		data = inDataSectionSize;
		ioOStream.write((char *)&data, DATA_SECTION_SIZE_LEN);
		ioOStream.write((char *)&inObjectID, OBJECT_ID_LEN);
		data = GetID();
		ioOStream.write((char *)&data, NODE_ID_LEN);
		data = GetParentID();
		ioOStream.write((char *)&data, PARENT_NODE_ID_LEN);
	}

private:
	int	mID;
	CalibraNode	*mParentNode;
	std::vector<CalibraNode *>	mChildNodeList;

	void CallAllChildNodesReadPostProcessRecursively()
	{
		if (HasChildNode() == false)
			return;

		std::vector<CalibraNode *>::const_iterator		it;

		for (it = mChildNodeList.begin(); it != mChildNodeList.end(); ++it)
		{
			(*it)->ReadPostProcess();
			(*it)->CallAllChildNodesReadPostProcessRecursively();
		}
	}

	void CallAllChildNodesWritePreprocessRecursively()
	{
		if (HasChildNode() == false)
			return;

		std::vector<CalibraNode *>::const_iterator		it;

		for (it = mChildNodeList.begin(); it != mChildNodeList.end(); ++it)
		{
			(*it)->WritePreprocess();
			(*it)->CallAllChildNodesWritePreprocessRecursively();
		}
	}

	void RenumberAllChildNodesIDRecursively()
	{
		if (HasChildNode() == false)
			return;

		std::vector<CalibraNode *>::const_iterator		it;

		for (it = mChildNodeList.begin(); it != mChildNodeList.end(); ++it)
		{
			(*it)->RenumberID();
			(*it)->RenumberAllChildNodesIDRecursively();
		}
	}

	void DeleteAllChildNodesRecursively()
	{
		if (HasChildNode() == false)
			return;

		for (int i = GetChildNodeNum() - 1; i >= 0; i--)
		{
			CalibraNode	*node = RemoveChildNodeAt(i);
			node->DeleteAllChildNodesRecursively();
			delete node;
		}
	}

	void	PrepareForNewChildNode(CalibraNode *inNode)
	{
		ASSERT(IsLeafNode() == false);

		ASSERT(inNode->mParentNode == null);

		ASSERT(ChildNodeCheck(inNode) != false);
	}

	void	SetTempID(int inID)
	{
		mID = inID;
	}

	void	RenumberID()
	{
		mID = GetUniqueID();
	}

	int		GetParentID()	const
	{
		if (GetParentNode() == null)
			return 0;

		return GetParentNode()->GetID();
	}

	int		GetUniqueID()
	{
		static int	sNewUniqueID = 0;
		sNewUniqueID++;
		return sNewUniqueID;
	}
};

#endif	// #ifdef __CALIBRA_NODE_H

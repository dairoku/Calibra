// =============================================================================
//  CalibraFile.hpp
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
	\file		CalibraFile.hpp
	\author		Dairoku Sekiguchi
	\version	1.0
	\date		2008/01/25
	\brief

	Description...
*/
#ifndef __CALIBRA_FILE_H
#define __CALIBRA_FILE_H


// -----------------------------------------------------------------------------
// 	include files
// -----------------------------------------------------------------------------
#include "CalibraData.hpp"
#include <fstream>
#include <string>
#include <vector>


// -----------------------------------------------------------------------------
// 	macros
// -----------------------------------------------------------------------------
#define	CALIBRA_FILE_MAGIC_WORD		"CALIBRA0"


// -----------------------------------------------------------------------------
//	CalibraNode class
// -----------------------------------------------------------------------------
//
class CalibraFile
{
public:
	//	File Header Section
	const static unsigned int		VERSION_NUMBER						= 0;

	const static unsigned int		MAGIC_WORD_LEN						= 8;
	const static unsigned int		VERSION_NUMBER_LEN					= 4;

	//	Data Section
	const static unsigned int		DATA_SECTION_TYPE_LEN				= 4;
	const static unsigned int		DATA_SECTION_SIZE_LEN				= 4;
	const static unsigned int		DATA_SECTION_HEADER_LEN				= DATA_SECTION_TYPE_LEN + DATA_SECTION_SIZE_LEN;


	static CalibraNode	*ReadFromFile(const wchar_t *inFileName)
	{
		std::ifstream	inputStream(inFileName, std::ios::in | std::ios::binary);
		if (inputStream.fail())
			throw std::runtime_error("inputStream.fail(): in CalibraFile::ReadFromFile");

		//	We assume that the root node is ProjectNode, but we should check that before use it.
		ProjectNode	*projectNode = (ProjectNode *)ReadFromStream(inputStream);
		projectNode->SetFilePath(inFileName);
		CalibraNode::CallReadPostProcessRecursively(projectNode);
		return projectNode;
	}

	static CalibraNode	*ReadFromStream(std::istream &ioIStream)
	{
		char	buf[MAGIC_WORD_LEN + 1];

		ioIStream.read(buf, MAGIC_WORD_LEN);
		buf[MAGIC_WORD_LEN] = 0;
		if (::strcmp(buf, CALIBRA_FILE_MAGIC_WORD) != 0)
			throw std::runtime_error("Invalid File Header: in CalibraFile::ReadFromStream");

		unsigned int	data;
		ioIStream.read((char *)&data, VERSION_NUMBER_LEN);
		if (data != VERSION_NUMBER)
			throw std::runtime_error("Invalid File Format Version: in CalibraFile::ReadFromStream");

		CalibraNode	*node = ReadNodeFromStream(ioIStream);
		CalibraNode::RenumberAllNodesIDRecursively(node);

		return node;
	}

	static void	WriteToFile(const wchar_t *inFileName, CalibraNode *inNode)
	{
		std::ofstream	outputStream(inFileName, std::ios::out | std::ios::binary);
		if (outputStream.fail())
			throw std::runtime_error("outputStream.fail(): in CalibraFile::WriteToFile");

		//	We assume that the root node is ProjectNode, but we should check that before use it.
		((ProjectNode *)inNode)->SetFilePath(inFileName);
		CalibraNode::CallWritePreprocessRecursively(inNode);
		WriteToStream(outputStream, inNode);
	}

	static void	WriteToStream(std::ostream &ioOStream, const CalibraNode *inNode)
	{
		int number = VERSION_NUMBER;

		ioOStream.write(CALIBRA_FILE_MAGIC_WORD, MAGIC_WORD_LEN);
		ioOStream.write((char *)&number, VERSION_NUMBER_LEN);

		WriteNodeToStream(ioOStream, inNode);

		unsigned int	data;

		data = CalibraNode::NULL_TYPE;
		ioOStream.write((char *)&data, DATA_SECTION_TYPE_LEN);
		data = CalibraNode::DATA_SECTION_HEADER_LEN;
		ioOStream.write((char *)&data, DATA_SECTION_SIZE_LEN);
	}
private:
	static CalibraNode	*ReadNodeFromStream(std::istream &ioIStream, CalibraNode *inNode = null)
	{
		unsigned int	data, objectID;
		int	nodeID, parentNodeID;
		CalibraNode	*newNode = null;

		try
		{
			ioIStream.read((char *)&data, CalibraNode::DATA_SECTION_TYPE_LEN);
			if (data == CalibraNode::NULL_TYPE)	// This means the end of the file
				return null;
			if (data != CalibraNode::OBJECT_DATA_TYPE)
				throw std::runtime_error("Invalid file format: in CalibraFile::ReadNodeFromStream");

			ioIStream.read((char *)&data, CalibraNode::DATA_SECTION_SIZE_LEN);
			ioIStream.read((char *)&objectID, CalibraNode::OBJECT_ID_LEN);
			ioIStream.read((char *)&nodeID, CalibraNode::NODE_ID_LEN);
			ioIStream.read((char *)&parentNodeID, CalibraNode::PARENT_NODE_ID_LEN);
			ioIStream.seekg(-1 * (int )CalibraNode::OBJECT_HEADER_LEN, std::ios_base::cur);

			newNode = CalibraNodeFactory(objectID);
			if (newNode == null)
				throw std::runtime_error("Invalid ObjectID: in CalibraFile::ReadNodeFromStream");
			newNode->ReadFromStream(ioIStream);

			if (parentNodeID != 0 && inNode != null)
			{
				CalibraNode	*rootNode = inNode->GetRootNode();
				CalibraNode	*parentNode = rootNode->FindNodeByID(parentNodeID);
				if (parentNode != null)
					parentNode->AddChildNode(newNode);
			}
		}

		catch (std::exception)
		{
			if (newNode != null)
				CalibraNode::DeleteAllNodesRecursively(newNode);

			if (inNode != null)
				CalibraNode::DeleteAllNodesRecursively(inNode->GetRootNode());

			throw;
		}

		ReadNodeFromStream(ioIStream, newNode);
		return newNode;
	}

	static void	WriteNodeToStream(std::ostream &ioOStream, const CalibraNode *inNode)
	{
		inNode->WriteToStream(ioOStream, false);

		std::vector<CalibraNode *>::const_iterator		it;

		for (it = inNode->begin(); it != inNode->end(); ++it)
			WriteNodeToStream(ioOStream, (*it));
	}

	static CalibraNode	*CalibraNodeFactory(int inObjectID)
	{
		switch (inObjectID)
		{
			case CalibraNode::IMAGE_FOLDER_NODE_OBJECT_ID:
				return new ImageFolderNode();
			case CalibraNode::IMAGE_NODE_OBJECT_ID:
				return new ImageNode();
			case CalibraNode::INPUT_IMAGE_NODE_OBJECT_ID:
				return new InputImageNode();
			case CalibraNode::PROJECT_NODE_OBJECT_ID:
				return new ProjectNode();
			case CalibraNode::SINGLE_CAMERA_CALIBRATION_NODE_OBJECT_ID:
				return new SingleCameraCalibrationNode();
			case CalibraNode::SINGLE_CAMERA_RESULT_NODE_OBJECT_ID:
				return new SingleCameraResultNode();
			case CalibraNode::STEREO_CAMERA_CALIBRATION_NODE_OBJECT_ID:
				return new StereoCameraCalibrationNode();
			case CalibraNode::STEREO_CAMERA_RESULT_NODE_OBJECT_ID:
				return new StereoCameraResultNode();
			case CalibraNode::MULTI_CAMERA_CALIBRATION_NODE_OBJECT_ID:
				return new MultiCameraCalibrationNode();
			case CalibraNode::MULTI_CAMERA_RESULT_NODE_OBJECT_ID:
				return new MultiCameraResultNode();
		}
		return null;
	}
};

#endif	// #ifdef __CALIBRA_FILE_H

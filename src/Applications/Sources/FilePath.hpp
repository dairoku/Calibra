// =============================================================================
//  FilePath.hpp
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
	\file		FilePath.hpp
	\author		Dairoku Sekiguchi
	\version	1.0
	\date		2008/02/19
	\brief

	Description...
*/
#ifndef __FILE_PATH_H
#define __FILE_PATH_H


// -----------------------------------------------------------------------------
// 	include files
// -----------------------------------------------------------------------------
#include <string>
#include <algorithm>
#include <ctype.h>

// -----------------------------------------------------------------------------
// 	macros
// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------
//	FilePath class
// -----------------------------------------------------------------------------
//
class FilePath
{
public:
	static std::wstring	ExtractFileName(const wchar_t *inFilePath)
	{
		size_t	pos = ::wcslen(inFilePath);

		while (pos != 0)
		{
			if (inFilePath[pos - 1] == '\\')
				return std::wstring(&inFilePath[pos]);
			pos--;
		}

		return std::wstring(inFilePath);
	}

	static std::wstring	ExtractPath(const wchar_t *inFilePath)
	{
		size_t	pos = ::wcslen(inFilePath), len = 0;

		while (pos != 0)
		{
			pos--;
			if (inFilePath[pos] == '\\')
				break;
			len++;
		}

		if (pos == 0)
			return std::wstring(L"");

		std::wstring	buf(inFilePath);
		buf.erase(pos + 1, len);

		return buf;
	}

	static bool	IsAbsolutePath(const wchar_t *inFilePath)
	{
		size_t	len = ::wcslen(inFilePath);
		if (len < 2)
			false;
		if (inFilePath[1] != ':')
			return false;

		return true;
	}

	static bool	IsUNCName(const wchar_t *inFilePath)
	{
		size_t	len = ::wcslen(inFilePath);
		if (len < 2)
			false;
		if (inFilePath[0] != '\\' || inFilePath[1] != '\\')
			return false;

		return true;
	}

	//	This function assumes that all inputs will satisfy the regular path expression.
	//	The regular path express means:
	//		must contain drive letter followed by ':' at first
	//		dose not contain '..'
	//		dose not contain '.'
	//		dose not contain multiple '\' as one delimiter
	//		path will always end with '\'
	//	I am thinking that we should provide a function, which will convert non-regular to regular one.
	static std::wstring	BuildRelativeFilePath(const wchar_t *inBasePath, const wchar_t *inAbsFilePath)
	{
		if (IsAbsolutePath(inBasePath) == false)
			return std::wstring(inAbsFilePath);
		if (IsAbsolutePath(inAbsFilePath) == false)
			return std::wstring(inAbsFilePath);

		std::wstring	baseDriveLetter;
		std::wstring	inputDriveLetter;
		baseDriveLetter += inBasePath[0];
		inputDriveLetter += inAbsFilePath[0];

		if (FileSystemStrCompare(baseDriveLetter, inputDriveLetter) != 0)	// Different drive letter
			return std::wstring(inAbsFilePath);

		std::wstring	inputPath = ExtractPath(inAbsFilePath);
		std::wstring	inputFileName = ExtractFileName(inAbsFilePath);
		std::wstring	buf, basePathDir, inputPathDir;
		int	basePathDirDepth = GetAbsDirectoryDepth(inBasePath);
		int	inputPathDirDepth = GetAbsDirectoryDepth(inputPath.c_str());
		int	i, matchedPathDepth, dirDepth;

		if (basePathDirDepth > inputPathDirDepth)
			dirDepth = inputPathDirDepth;
		else
			dirDepth = basePathDirDepth;

		for (i = 0; i < dirDepth; i++)
		{
			basePathDir = ExtractAbsDirectoryByIndex(inBasePath, i);
			inputPathDir = ExtractAbsDirectoryByIndex(inputPath.c_str(), i);

			if (FileSystemStrCompare(basePathDir, inputPathDir) != 0)
				break;
		}
		matchedPathDepth = i;

		if (matchedPathDepth >= basePathDirDepth)
		{
			for (i = matchedPathDepth; i < inputPathDirDepth; i++)
			{
				buf += ExtractAbsDirectoryByIndex(inputPath.c_str(), i);
				buf += L"\\";
			}
			buf += inputFileName;
			return buf;
		}

		for (i = 0; i < basePathDirDepth - matchedPathDepth; i++)
		{
			buf += L"..";
			buf += L"\\";
		}

		for (i = matchedPathDepth; i < inputPathDirDepth; i++)
		{
			buf += ExtractAbsDirectoryByIndex(inputPath.c_str(), i);
			buf += L"\\";
		}

		buf += inputFileName;
		return buf;
	}

	static std::wstring	BuildAbsoluteFilePath(const wchar_t *inBasePath, const wchar_t *inFilePath)
	{
		if (IsAbsolutePath(inBasePath) == false)
			return std::wstring(inFilePath);
		if (IsAbsolutePath(inFilePath) == true)
			return std::wstring(inFilePath);

		std::wstring	inputPath = ExtractPath(inFilePath);
		std::wstring	inputFileName = ExtractFileName(inFilePath);
		std::wstring	buf, basePathDir, inputPathDir;
		int	basePathDirDepth = GetAbsDirectoryDepth(inBasePath);
		int	inputPathDirDepth = GetRelativeDirectoryDepth(inputPath.c_str());
		int	i, dirMinusNum = 0;

		for (i = 0; i < inputPathDirDepth; i++)
		{
			inputPathDir = ExtractRelativeDirectoryByIndex(inputPath.c_str(), i);
			if (inputPathDir == L"..")
				dirMinusNum++;
		}

		basePathDirDepth -= dirMinusNum;
		if (basePathDirDepth < 0)
			return std::wstring(inFilePath);

		buf += inBasePath[0];	// Drive Letter
		buf += L":\\";
		for (i = 0; i < basePathDirDepth; i++)
		{
			buf += ExtractAbsDirectoryByIndex(inBasePath, i);
			buf += L"\\";
		}

		for (i = dirMinusNum; i < inputPathDirDepth; i++)
		{
			buf += ExtractRelativeDirectoryByIndex(inputPath.c_str(), i);
			buf += L"\\";
		}

		buf += inputFileName;
		return buf;
	}

private:
	static int	GetAbsDirectoryDepth(const wchar_t *inPath)
	{
		size_t	len = ::wcslen(inPath);
		int	depth = 0;

		for (size_t i = 0; i < len; i++)
			if (inPath[i] == '\\')
				depth++;

		if (inPath[len - 1] == '\\' && depth != 0)
			depth--;

		return depth;
	}

	static int	GetRelativeDirectoryDepth(const wchar_t *inPath)
	{
		size_t	len = ::wcslen(inPath);
		if (len == 0)
			return 0;

		int	depth = 1;

		for (size_t i = 0; i < len; i++)
			if (inPath[i] == '\\')
				depth++;

		if (inPath[len - 1] == '\\' && depth != 0)
			depth--;

		return depth;
	}

	static std::wstring	ExtractAbsDirectoryByIndex(const wchar_t *inPath, int inIndex)
	{
		size_t	i, j, len = ::wcslen(inPath);
		int	depth = 0;

		for (i = 0; i < len; i++)
		{
			if (inPath[i] == '\\')
				depth++;
			if (depth == inIndex + 1)
			{
				for (j = i + 1; j < len; j++)
				{
					if (inPath[j] == '\\')
						break;
				}
				std::wstring	buf(inPath);
				buf.erase(j, len - j);
				buf.erase(0, i + 1);

				return buf;
			}
		}

		return std::wstring(L"");
	}

	static std::wstring	ExtractRelativeDirectoryByIndex(const wchar_t *inPath, int inIndex)
	{
		size_t	i, j, len = ::wcslen(inPath);
		int	depth = 0;

		for (i = 0; i < len; i++)
		{
			if (depth == inIndex)
			{
				for (j = i; j < len; j++)
				{
					if (inPath[j] == '\\')
						break;
				}
				std::wstring	buf(inPath);
				buf.erase(j, len - j);
				buf.erase(0, i);

				return buf;
			}
			if (inPath[i] == '\\')
				depth++;
		}

		return std::wstring(L"");
	}

	static int	FileSystemStrCompare(std::wstring inA, std::wstring inB)
	{
		std::transform(inA.begin(), inA.end(), inA.begin(), towlower);
		std::transform(inB.begin(), inB.end(), inB.begin(), towlower);
		return inA.compare(inB);
	}
};

#endif	// #ifdef __FILE_PATH_H

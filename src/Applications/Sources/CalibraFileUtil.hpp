// =============================================================================
//  CalibraFileUtil.hpp
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
	\file		CalibraFileUtil.hpp
	\author		Dairoku Sekiguchi
	\version	1.0
	\date		2008/03/03
	\brief

	Description...
*/
#ifndef __CALIBRA_FILE_UTIL_H
#define __CALIBRA_FILE_UTIL_H


// -----------------------------------------------------------------------------
// 	include files
// -----------------------------------------------------------------------------
#include <fstream>
#include <string>
#include <vector>


// -----------------------------------------------------------------------------
// 	macros
// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------
//	CalibraFileUtil class
// -----------------------------------------------------------------------------
//
class CalibraFileUtil
{
public:
	static void	ReadBoolFromStream(std::istream &ioIStream, bool *outValue)
	{
		ioIStream.read((char *)outValue, sizeof(bool));
	}

	static void	WriteBoolToStream(std::ostream &ioOStream, bool inValue)
	{
		ioOStream.write((char *)&inValue, sizeof(bool));
	}

	static unsigned int	CalcBoolStreamSize()
	{
		return sizeof(bool);
	}

	static void	ReadIntFromStream(std::istream &ioIStream, int *outValue)
	{
		ioIStream.read((char *)outValue, sizeof(int));
	}

	static void	WriteIntToStream(std::ostream &ioOStream, int inValue)
	{
		ioOStream.write((char *)&inValue, sizeof(int));
	}

	static unsigned int	CalcIntStreamSize()
	{
		return sizeof(int);
	}

	static void	ReadDoubleFromStream(std::istream &ioIStream, double *outValue)
	{
		ioIStream.read((char *)outValue, sizeof(double));
	}

	static void	WriteDoubleToStream(std::ostream &ioOStream, double inValue)
	{
		ioOStream.write((char *)&inValue, sizeof(double));
	}

	static unsigned int	CalcDoubleStreamSize()
	{
		return sizeof(double);
	}

	static void	ReadIntVectorFromStream(std::istream &ioIStream,
					ublas::vector<int> &outVector)
	{
		unsigned int	size;

		ioIStream.read((char *)&size, sizeof(unsigned int));
		outVector.resize(size);
		
		if (size != 0)
		{
			int	*dataPtr = traits::vector_storage(outVector);
			ioIStream.read((char *)dataPtr, size * sizeof(int));
		}
	}

	static void	WriteIntVectorToStream(std::ostream &ioOStream,
					const ublas::vector<int> &inVector)
	{
		unsigned int	size;

		size = (unsigned int )inVector.size();
		ioOStream.write((char *)&size, sizeof(unsigned int));
		
		if (size != 0)
		{
			const int	*dataPtr = traits::vector_storage(inVector);
			ioOStream.write((char *)dataPtr, size * sizeof(int));
		}
	}

	static unsigned int	CalcIntVectorStreamSize(const ublas::vector<int> &inVector)
	{
		unsigned int	size;

		size = sizeof(int);
		size += (unsigned int )inVector.size() * sizeof(int);
		return size;
	}

	static void	ReadDoubleVectorFromStream(std::istream &ioIStream,
					ublas::vector<double> &outVector)
	{
		unsigned int	size;

		ioIStream.read((char *)&size, sizeof(unsigned int));
		outVector.resize(size);

		if (size != 0)
		{
			double	*dataPtr = traits::vector_storage(outVector);
			ioIStream.read((char *)dataPtr, size * sizeof(double));
		}
	}

	static void	WriteDoubleVectorToStream(std::ostream &ioOStream,
					const ublas::vector<double> &inVector)
	{
		unsigned int	size;

		size = (unsigned int )inVector.size();
		ioOStream.write((char *)&size, sizeof(unsigned int));

		if (size != 0)
		{
			const double	*dataPtr = traits::vector_storage(inVector);
			ioOStream.write((char *)dataPtr, size * sizeof(double));
		}
	}

	static unsigned int	CalcDoubleVectorStreamSize(const ublas::vector<double> &inVector)
	{
		unsigned int	size;

		size = sizeof(int);
		size += (unsigned int )inVector.size() * sizeof(double);
		return size;
	}

	static void	ReadDoubleVectorListFromStream(std::istream &ioIStream,
					std::vector<ublas::vector<double> > &outList)
	{
		unsigned int	size;

		ioIStream.read((char *)&size, sizeof(unsigned int));
		outList.resize(size);

		std::vector<ublas::vector<double> >::iterator	it;
		for (it = outList.begin(); it != outList.end(); ++it)
			ReadDoubleVectorFromStream(ioIStream, *it);
	}

	static void	WriteDoubleVectorListToStream(std::ostream &ioOStream,
					const std::vector<ublas::vector<double> > &inList)
	{
		unsigned int	size = (unsigned int )inList.size();

		ioOStream.write((char *)&size, sizeof(unsigned int));

		std::vector<ublas::vector<double> >::const_iterator	it;
		for (it = inList.begin(); it != inList.end(); ++it)
			WriteDoubleVectorToStream(ioOStream, *it);
	}

	static unsigned int	CalcDoubleVectorListStreamSize(
				const std::vector<ublas::vector<double> > &inList)
	{
		unsigned int	size;
		std::vector<ublas::vector<double> >::const_iterator	it;

		size = sizeof(unsigned int);
		for (it = inList.begin(); it != inList.end(); ++it)
			size += CalcDoubleVectorStreamSize(*it);

		return size;
	}

	static void	ReadMatrixFromStream(std::istream &ioIStream,
					ublas::matrix<double, ublas::column_major> &outMatrix)
	{
		unsigned int	size1, size2;

		ioIStream.read((char *)&size1, sizeof(unsigned int));
		ioIStream.read((char *)&size2, sizeof(unsigned int));
		outMatrix.resize(size1, size2);

		if (size1 * size2 != 0)
		{
			double	*dataPtr = traits::matrix_storage(outMatrix);
			ioIStream.read((char *)dataPtr, size1 * size2 * sizeof(double));
		}
	}

	static void	WriteMatrixToStream(std::ostream &ioOStream,
					const ublas::matrix<double, ublas::column_major> &inMatrix)
	{
		unsigned int	size1, size2;

		size1 = (unsigned int )inMatrix.size1();
		size2 = (unsigned int )inMatrix.size2();
		ioOStream.write((char *)&size1, sizeof(unsigned int));
		ioOStream.write((char *)&size2, sizeof(unsigned int));

		if (size1 * size2 != 0)
		{
			const double	*dataPtr = traits::matrix_storage(inMatrix);
			ioOStream.write((char *)dataPtr, size1 * size2 * sizeof(double));
		}
	}

	static unsigned int	CalcMatrixStreamSize(const ublas::matrix<double, ublas::column_major> &inMatrix)
	{
		unsigned int	size;

		size = sizeof(unsigned int) * 2;
		size += (unsigned int )inMatrix.size1() * (unsigned int )inMatrix.size2() * sizeof(double);
		return size;
	}

	static void	ReadMatrixListFromStream(std::istream &ioIStream,
					std::vector<ublas::matrix<double, ublas::column_major> > &outList)
	{
		unsigned int	size;

		ioIStream.read((char *)&size, sizeof(unsigned int));
		outList.resize(size);

		std::vector<ublas::matrix<double, ublas::column_major> >::iterator	it;
		for (it = outList.begin(); it != outList.end(); ++it)
			ReadMatrixFromStream(ioIStream, *it);
	}

	static void	WriteMatrixListToStream(std::ostream &ioOStream,
					const std::vector<ublas::matrix<double, ublas::column_major> > &inList)
	{
		unsigned int	size = (unsigned int )inList.size();

		ioOStream.write((char *)&size, sizeof(unsigned int));

		std::vector<ublas::matrix<double, ublas::column_major> >::const_iterator	it;
		for (it = inList.begin(); it != inList.end(); ++it)
			WriteMatrixToStream(ioOStream, *it);
	}

	static unsigned int	CalcMatrixListStreamSize(
				const std::vector<ublas::matrix<double, ublas::column_major> > &inList)
	{
		unsigned int	size;
		std::vector<ublas::matrix<double, ublas::column_major> >::const_iterator	it;

		size = sizeof(unsigned int);
		for (it = inList.begin(); it != inList.end(); ++it)
			size += CalcMatrixStreamSize(*it);

		return size;
	}

	static void	ReadStringFromStream(std::istream &ioIStream, std::wstring &outString)
	{
		unsigned int	size;
		char	*buf;

		ioIStream.read((char *)&size, sizeof(unsigned int));
		buf = new char[size];
		ioIStream.read(buf, size);
		outString.assign((wchar_t *)buf, size / sizeof(wchar_t));
		delete buf;
	}

	static void	WriteStringToStream(std::ostream &ioOStream, const std::wstring &inString)
	{
		unsigned int	size;

		size = (unsigned int )(inString.length() * sizeof(wchar_t));
		ioOStream.write((char *)&size, sizeof(unsigned int));
		ioOStream.write((char *)inString.c_str(), size);
	}

	static unsigned int	CalcStringStreamSize(const std::wstring &inString)
	{
		unsigned int	size;

		size = (unsigned int )(inString.length() * sizeof(wchar_t) + sizeof(unsigned int));
		return size;
	}
};

#endif	// #ifdef __CALIBRA_FILE_UTIL_H

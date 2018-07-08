// =============================================================================
//  ImageData.hpp
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
	\file		ImageData.hpp
	\author		Dairoku Sekiguchi
	\version	1.0
	\date		2007/11/13
	\brief		Simple Class for displaying a image

	Simple Class for displaying a image
*/
#ifndef __IMAGE_DATA_H
#define __IMAGE_DATA_H


// -----------------------------------------------------------------------------
// 	include files
// -----------------------------------------------------------------------------
#include <windows.h>
#include <string.h>
#include <process.h>
#include <vector>
#include <commctrl.h>
#include "BoostIncludes.hpp"


// -----------------------------------------------------------------------------
// 	macros
// -----------------------------------------------------------------------------
#define	IMAGE_PALLET_SIZE_8BIT		256
#define	IMAGE_FILE_NAME_BUF_LEN		256
#define	IMAGE_STR_BUF_SIZE			256

// -----------------------------------------------------------------------------
// 	typedefs
// -----------------------------------------------------------------------------
typedef struct
{
	BITMAPINFOHEADER		Header;
	RGBQUAD					RGBQuad[IMAGE_PALLET_SIZE_8BIT];
} ImageBitmapInfoMono8;


// -----------------------------------------------------------------------------
//	ImageData class
// -----------------------------------------------------------------------------
//
class ImageData
{
public:

	ImageData()
	{
		mBitmapInfo = NULL;
		mBitmapInfoSize = 0;
		mBitmapBits = NULL;
		mBitmapBitsSize = 0;
		mAllocatedImageBuffer = NULL;
	}

	virtual ~ImageData()
	{
		if (mAllocatedImageBuffer != NULL)
			delete mAllocatedImageBuffer;
	}

	void	SetImageBufferPtr(int inWidth, int inHeight, unsigned char *inImagePtr, bool inIsColor)
	{
		bool	doUpdateSize;

		if (inIsColor)
			doUpdateSize = CreateColorBitmapInfo(inWidth, inHeight);
		else
			doUpdateSize = CreateMonoBitmapInfo(inWidth, inHeight);

		if (mAllocatedImageBuffer != NULL)
		{
			delete mAllocatedImageBuffer;
			mAllocatedImageBuffer = NULL;
		}
		mBitmapBits = inImagePtr;
	}

	void	AllocateImageBuffer(int inWidth, int inHeight, bool inIsColor)
	{
		bool	doUpdateSize;

		if (inIsColor)
			doUpdateSize = CreateColorBitmapInfo(inWidth, inHeight);
		else
			doUpdateSize = CreateMonoBitmapInfo(inWidth, inHeight);

		if (mAllocatedImageBuffer != NULL && doUpdateSize != false)
		{
			delete mAllocatedImageBuffer;
			mAllocatedImageBuffer = NULL;
		}

		if (mAllocatedImageBuffer == NULL)
		{
			mAllocatedImageBuffer = new unsigned char[mBitmapBitsSize];
			if (mAllocatedImageBuffer == NULL)
			{
				printf("Error: Can't allocate mBitmapBits (AllocateMonoImageBuffer)\n");
				return;
			}
			mBitmapBits = mAllocatedImageBuffer;
		}
	}

	void	CopyImageBuffer(int inWidth, int inHeight, unsigned char *inImage, bool inIsColor)
	{
		bool	doUpdateSize;

		if (inIsColor)
			doUpdateSize = CreateMonoBitmapInfo(inWidth, inHeight);
		else
			doUpdateSize = CreateMonoBitmapInfo(inWidth, inHeight);

		if (mAllocatedImageBuffer != NULL && doUpdateSize != false)
		{
			delete mAllocatedImageBuffer;
			mAllocatedImageBuffer = NULL;
		}

		if (mAllocatedImageBuffer == NULL)
		{
			mAllocatedImageBuffer = new unsigned char[mBitmapBitsSize];
			if (mAllocatedImageBuffer == NULL)
			{
				printf("Error: Can't allocate mBitmapBits (AllocateMonoImageBuffer)\n");
				return;
			}
			mBitmapBits = mAllocatedImageBuffer;
		}
		CopyMemory(mBitmapBits, inImage, mBitmapBitsSize);
	}

	void	SetMonoImageBufferPtr(int inWidth, int inHeight, unsigned char *inImagePtr)
	{
		SetImageBufferPtr(inWidth, inHeight, inImagePtr, false);
	}

	void	AllocateMonoImageBuffer(int inWidth, int inHeight)
	{
		AllocateImageBuffer(inWidth, inHeight, false);
	}

	void	CopyMonoImageBuffer(int inWidth, int inHeight, unsigned char *inImage)
	{
		CopyImageBuffer(inWidth, inHeight, inImage, false);
	}

	void	SetColorImageBufferPtr(int inWidth, int inHeight, unsigned char *inImagePtr)
	{
		SetImageBufferPtr(inWidth, inHeight, inImagePtr, true);
	}

	void	AllocateColorImageBuffer(int inWidth, int inHeight)
	{
		AllocateImageBuffer(inWidth, inHeight, true);
	}

	void	CopyColorImageBuffer(int inWidth, int inHeight, unsigned char *inImage)
	{
		CopyImageBuffer(inWidth, inHeight, inImage, true);
	}

	void	GetuBLASMatrix(ublas::matrix<unsigned char, ublas::column_major> &outMat)
	{
		unsigned char	*dataPtr = GetImageBufferPtr();

		outMat.resize(GetImageHeight(), GetImageWidth());
		for (int i = 0; i < GetImageHeight(); i++)
			for (int j = 0; j < GetImageWidth(); j++, dataPtr++)
				outMat(i, j) = *dataPtr;
	}

	void	DumpBitmapInfo()
	{
		printf("[Dump BitmapInfo]\n");

		if (mBitmapInfo == NULL)
		{
			printf("BitmapInfo is NULL \n");
			return;
		}

		printf(" biSize: %d\n", mBitmapInfo->biSize);
		printf(" biWidth: %d\n", mBitmapInfo->biWidth);
		if (mBitmapInfo->biHeight < 0)
			printf(" biHeight: %d (Top-down DIB)\n", mBitmapInfo->biHeight);
		else
			printf(" biHeight: %d (Bottom-up DIB)\n", mBitmapInfo->biHeight);
		printf(" biPlanes: %d\n", mBitmapInfo->biPlanes);
		printf(" biBitCount: %d\n", mBitmapInfo->biBitCount);
		printf(" biCompression: %d\n", mBitmapInfo->biCompression);
		printf(" biSizeImage: %d\n", mBitmapInfo->biSizeImage);
		printf(" biXPelsPerMeter: %d\n", mBitmapInfo->biXPelsPerMeter);
		printf(" biYPelsPerMeter: %d\n", mBitmapInfo->biYPelsPerMeter);
		printf(" biClrUsed: %d\n", mBitmapInfo->biClrUsed);
		printf(" biClrImportant: %d\n", mBitmapInfo->biClrImportant);
		printf("\n");

		printf(" mBitmapInfoSize =  %d bytes\n", mBitmapInfoSize);
		printf(" RGBQUAD number  =  %d\n", (mBitmapInfoSize - mBitmapInfo->biSize) / sizeof(RGBQUAD));
		printf(" mBitmapBitsSize =  %d bytes\n", mBitmapBitsSize);
	}

	unsigned char	*CreateDIB(bool inForceConvertBottomUp = true)
	{
		if (mBitmapInfo == NULL)
			return NULL;

		unsigned char	*buf = new unsigned char[mBitmapInfoSize + mBitmapBitsSize];
		CopyMemory(buf, mBitmapInfo, mBitmapInfoSize);
		CopyMemory(&(buf[mBitmapInfoSize]), mBitmapBits, mBitmapBitsSize);

		if (inForceConvertBottomUp)
			if (mBitmapInfo->biHeight > 0)
				FlipBitmap((BITMAPINFOHEADER *)buf, &(buf[mBitmapInfoSize]));

		return buf;
	}

	void	FlipImageBuffer()
	{
		FlipBitmap(mBitmapInfo, mBitmapBits);
	}

	bool	OpenBitmapFile(const wchar_t *inFileName)
	{	
		FILE	*fp;
		BITMAPFILEHEADER	fileHeader;

		if (_wfopen_s(&fp, inFileName, L"rb") != 0)
		{
			printf("Error: Can't open file %s (OpenBitmapFile)\n", inFileName);
			return false;
		}

		if (fread(&fileHeader, sizeof(BITMAPFILEHEADER), 1, fp) != 1)
		{
			printf("Error: fread failed (OpenBitmapFile)\n");
			fclose(fp);
			return false;
		}
	
		if (mBitmapInfo != NULL)
		{
			delete mBitmapInfo;
			if (mAllocatedImageBuffer != NULL)
				delete mAllocatedImageBuffer;
		}

		mBitmapInfoSize = fileHeader.bfOffBits - sizeof(BITMAPFILEHEADER);
		mBitmapInfo = (BITMAPINFOHEADER *)(new unsigned char[mBitmapInfoSize]);
		if (mBitmapInfo == NULL)
		{
			printf("Error: Can't allocate mBitmapInfo (OpenBitmapFile)\n");
			fclose(fp);
			return false;
		}

		mBitmapBitsSize = fileHeader.bfSize - fileHeader.bfOffBits;
		mAllocatedImageBuffer = new unsigned char[mBitmapBitsSize];
		if (mAllocatedImageBuffer == NULL)
		{
			printf("Error: Can't allocate mBitmapBits (OpenBitmapFile)\n");
			delete mBitmapInfo;
			mBitmapInfo = NULL;
			fclose(fp);
			return false;
		}
		mBitmapBits = mAllocatedImageBuffer;

		if (fread(mBitmapInfo, mBitmapInfoSize, 1, fp) != 1)
		{
			printf("Error: fread failed (OpenBitmapFile)\n");
			fclose(fp);
			return false;
		}
		if (fread(mBitmapBits, mBitmapBitsSize, 1, fp) != 1)
		{
			printf("Error: fread failed (OpenBitmapFile)\n");
			fclose(fp);
			return false;
		}
		fclose(fp);

		if (mBitmapInfo->biHeight > 0)
			FlipImageBuffer();

		wprintf(L"Bitmap File opened: %s\n", inFileName);
		return true;
	}

	bool	CopyToClipboard(HWND inWindowH)
	{
		if (mBitmapInfo == NULL || mBitmapBits == NULL)
		{
			printf("Error: mBitmapInfo == NULL || mBitmapBits == NULL CopyToClipboard()\n");
			return false;
		}

		unsigned char	*buf = CreateDIB(true);

		OpenClipboard(inWindowH);
		EmptyClipboard();
		SetClipboardData(CF_DIB, buf);
		CloseClipboard();

		delete buf;

		return true;
	}

	bool	SaveBitmapFile(const wchar_t *inFileName)
	{
		if (mBitmapInfo == NULL || mBitmapBits == NULL)
		{
			printf("Error: mBitmapInfo == NULL || mBitmapBits == NULL SaveBitmap()\n");
			return false;
		}

		DumpBitmapInfo();

		unsigned char	*buf = CreateDIB(true);

		FILE	*fp;
		char	fileName[IMAGE_FILE_NAME_BUF_LEN];

		BITMAPFILEHEADER	fileHeader;
		fileHeader.bfType = 'MB';
		fileHeader.bfSize = sizeof(BITMAPFILEHEADER) + mBitmapInfoSize + mBitmapBitsSize;
		fileHeader.bfReserved1 = 0;
		fileHeader.bfReserved2 = 0;
		fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + mBitmapInfoSize;
		
		if (_wfopen_s(&fp, (wchar_t *)inFileName, L"wb") != 0)
		{
			printf("Error: Can't create file %s (SaveBitmapFile)\n", fileName);
			return false;
		}

		if (fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, fp) != 1)
		{
			printf("Error: Can't write file (SaveBitmapFile)\n");
			fclose(fp);
			return false;
		}
		if (fwrite(buf, mBitmapInfoSize + mBitmapBitsSize, 1, fp) != 1)
		{
			printf("Error: Can't write file (SaveBitmapFile)\n");
			fclose(fp);
			return false;
		}
		fclose(fp);

		wprintf(L"Bitmap File saved: %s\n", inFileName);

		delete buf;

		return true;
	}

	bool	HasValidData()
	{
		if (mBitmapInfo == NULL || mBitmapBits == NULL)
			return false;
		return true;
	}

	int	GetImageWidth()
	{
		if (mBitmapInfo == NULL)
			return 0;
		return mBitmapInfo->biWidth;
	}

	int	GetImageHeight()
	{
		if (mBitmapInfo == NULL)
			return 0;
		return abs(mBitmapInfo->biHeight);
	}

	int	GetImageBitCount()
	{
		if (mBitmapInfo == NULL)
			return 0;
		return mBitmapInfo->biBitCount;
	}
	unsigned char	*GetImageBufferPtr()
	{
		return mBitmapBits;
	}
	
	unsigned int	GetImageBufferSize()
	{
		return mBitmapBitsSize;
	}

	BITMAPINFO *GetBitmapInfoPtr()
	{
		return (BITMAPINFO *)mBitmapInfo;
	}

	static void	FlipBitmap(BITMAPINFOHEADER *inBitmapInfo, unsigned char *inBitmapBits)
	{
		if (inBitmapInfo == NULL)
			return;
		if (inBitmapBits == NULL)
			return;

		unsigned char	*srcPtr, *dstPtr, tmp;
		int	width = inBitmapInfo->biWidth;
		int	height = abs(inBitmapInfo->biHeight);

		if (inBitmapInfo->biBitCount == 8)
		{
			for (int y = 0; y < height / 2; y++)
			{
				srcPtr = &(inBitmapBits[width * y]);
				dstPtr = &(inBitmapBits[width * (height - y - 1)]);
				for (int x = 0; x < width; x++)
				{
					tmp = *dstPtr;
					*dstPtr = *srcPtr;
					*srcPtr = tmp;
					srcPtr++;
					dstPtr++;
				}
			}
		}
		else
		{
			for (int y = 0; y < height / 2; y++)
			{
				srcPtr = &(inBitmapBits[width * y]);
				dstPtr = &(inBitmapBits[width * (height - y - 1)]);
				for (int x = 0; x < width * 3; x++)
				{
					tmp = *dstPtr;
					*dstPtr = *srcPtr;
					*srcPtr = tmp;
					srcPtr++;
					dstPtr++;
				}
			}			
		}

		inBitmapInfo->biHeight *= -1;
	}

private:

	BITMAPINFOHEADER	*mBitmapInfo;
	unsigned int		mBitmapInfoSize;
	unsigned char		*mBitmapBits;
	unsigned int		mBitmapBitsSize;

	unsigned char		*mAllocatedImageBuffer;

	bool	CreateMonoBitmapInfo(int inWidth, int inHeight)
	{
		bool	doCreateBitmapInfo = false;

		if (mBitmapInfo != NULL)
		{
			if (mBitmapInfo->biBitCount		!= 8 ||
				mBitmapInfo->biWidth		!= inWidth ||
				abs(mBitmapInfo->biHeight)	!= inHeight)
			{
				doCreateBitmapInfo = true;
			}
		}
		else
		{
			doCreateBitmapInfo = true;
		}

		if (doCreateBitmapInfo)
		{
			if (mBitmapInfo != NULL)
				delete mBitmapInfo;

			mBitmapInfoSize = sizeof(ImageBitmapInfoMono8);
			mBitmapInfo = (BITMAPINFOHEADER *)(new unsigned char[mBitmapInfoSize]);
			if (mBitmapInfo == NULL)
			{
				printf("Error: Can't allocate mBitmapInfo (CreateMonoBitmapInfo)\n");
				return false;
			}
			ImageBitmapInfoMono8	*bitmapInfo = (ImageBitmapInfoMono8 *)mBitmapInfo;
			bitmapInfo->Header.biSize			= sizeof(BITMAPINFOHEADER);
			bitmapInfo->Header.biWidth			= inWidth;
			bitmapInfo->Header.biHeight			= -1 * abs(inHeight);
			bitmapInfo->Header.biPlanes			= 1;
			bitmapInfo->Header.biBitCount		= 8;
			bitmapInfo->Header.biCompression	= BI_RGB;
			bitmapInfo->Header.biSizeImage		= 0;
			bitmapInfo->Header.biXPelsPerMeter	= 100;
			bitmapInfo->Header.biYPelsPerMeter	= 100;
			bitmapInfo->Header.biClrUsed		= IMAGE_PALLET_SIZE_8BIT;
			bitmapInfo->Header.biClrImportant	= IMAGE_PALLET_SIZE_8BIT;

			for (int i = 0; i < IMAGE_PALLET_SIZE_8BIT; i++)
			{
				bitmapInfo->RGBQuad[i].rgbBlue		= i;
				bitmapInfo->RGBQuad[i].rgbGreen		= i;
				bitmapInfo->RGBQuad[i].rgbRed		= i;
				bitmapInfo->RGBQuad[i].rgbReserved	= 0;
			}

			mBitmapBitsSize = abs(inWidth * inHeight);
		}

		return doCreateBitmapInfo;
	}

	bool	CreateColorBitmapInfo(int inWidth, int inHeight)
	{
		bool	doCreateBitmapInfo = false;

		if (mBitmapInfo != NULL)
		{
			if (mBitmapInfo->biBitCount		!= 24 ||
				mBitmapInfo->biWidth		!= inWidth ||
				abs(mBitmapInfo->biHeight)	!= inHeight)
			{
				doCreateBitmapInfo = true;
			}
		}
		else
		{
			doCreateBitmapInfo = true;
		}

		if (doCreateBitmapInfo)
		{
			if (mBitmapInfo != NULL)
				delete mBitmapInfo;

			mBitmapInfoSize = sizeof(BITMAPINFOHEADER);
			mBitmapInfo = (BITMAPINFOHEADER *)(new unsigned char[mBitmapInfoSize]);
			if (mBitmapInfo == NULL)
			{
				printf("Error: Can't allocate mBitmapInfo (CreateColorBitmapInfo)\n");
				return false;
			}
			mBitmapInfo->biSize				= sizeof(BITMAPINFOHEADER);
			mBitmapInfo->biWidth			= inWidth;
			mBitmapInfo->biHeight			= -1 * abs(inHeight);
			mBitmapInfo->biPlanes			= 1;
			mBitmapInfo->biBitCount			= 24;
			mBitmapInfo->biCompression		= BI_RGB;
			mBitmapInfo->biSizeImage		= 0;
			mBitmapInfo->biXPelsPerMeter	= 100;
			mBitmapInfo->biYPelsPerMeter	= 100;
			mBitmapInfo->biClrUsed			= 0;
			mBitmapInfo->biClrImportant		= 0;

			mBitmapBitsSize = abs(inWidth * inHeight * 3);
		}

		return doCreateBitmapInfo;
	}
};
#endif	// #ifdef __IMAGE_DATA_H

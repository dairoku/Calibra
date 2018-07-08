// =============================================================================
//  ImageView.cpp
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

#include "stdafx.h"
#include "Calibra.h"
#include "ImageView.h"

// CImageView

IMPLEMENT_DYNCREATE(CImageView, CScrollView)

CImageView::CImageView()
{
	mImageDispScale = 100;
	mImageData = NULL;
}

CImageView::~CImageView()
{
}


BEGIN_MESSAGE_MAP(CImageView, CScrollView)
END_MESSAGE_MAP()


// CImageView 描画

void CImageView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	UpdateImageSize();
}


void CImageView::SetImageData(ImageData *inImageData)
{
	mImageData = inImageData;

	UpdateImageSize();
}


void CImageView::UpdateImageSize()
{
	if (mImageData == NULL)
		return;

	if (mImageData->HasValidData() == false)
		return;

	CSize sizeTotal;
	int	width = mImageData->GetImageWidth();
	int	height = mImageData->GetImageHeight();

	if (mImageDispScale == 100)
	{
		sizeTotal.cx = width;
		sizeTotal.cy = height;
	}
	else
	{
		double	scale = mImageDispScale / 100.0;
		sizeTotal.cx = (int )(width * scale);
		sizeTotal.cy = (int )(height * scale);
	}	
	SetScrollSizes(MM_TEXT, sizeTotal);
}


void CImageView::SetDispScale(int inScale)
{
	int	prevScale = mImageDispScale;

	if (inScale <= 0)
		inScale = 1;
	mImageDispScale = inScale;
	UpdateImageSize();
	Invalidate(false);
}

void CImageView::ZoomIn()
{
	int	scale = (GetDispScale() + 50) / 50;
	SetDispScale(scale * 50);
}

void CImageView::ZoomOut()
{
	int	scale = (GetDispScale() - 50) / 50;
	SetDispScale(scale * 50);
}

void CImageView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	if (mImageData == NULL)
		return;

	if (mImageData->GetImageBufferPtr() == NULL ||
		mImageData->GetBitmapInfoPtr() == NULL)
		return;

	int	width = mImageData->GetImageWidth();
	int	height = mImageData->GetImageHeight();

	if (mImageDispScale == 100)
	{
		::SetDIBitsToDevice(*pDC,
			0, 0,
			width, height,
			0, 0,
			0, height,
			mImageData->GetImageBufferPtr(),
			mImageData->GetBitmapInfoPtr(),
			DIB_RGB_COLORS);
	}
	else
	{
		double	scale = mImageDispScale / 100.0;
		::StretchDIBits(*pDC,
			0, 0,
			(int )(width * scale),
			(int )(height * scale),
			0, 0,
			width, height,
			mImageData->GetImageBufferPtr(),
			mImageData->GetBitmapInfoPtr(),
			DIB_RGB_COLORS, SRCCOPY);
	}
}

// CImageView 診断

#ifdef _DEBUG
void CImageView::AssertValid() const
{
	CScrollView::AssertValid();
}

#ifndef _WIN32_WCE
void CImageView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
#endif
#endif //_DEBUG


// CImageView メッセージ ハンドラ

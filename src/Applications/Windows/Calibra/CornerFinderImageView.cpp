// =============================================================================
//  CornerFinderImageView.cpp
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

#include "StdAfx.h"
#include "Resource.h"
#include "CalibraDoc.h"
#include "CalibraData.hpp"
#include "CornerFinderImageView.h"

CCornerFinderImageView::CCornerFinderImageView(void)
{
	mIsLMouseDown = false;
	mInputImageNode = NULL;
	mIsMouseCursorTracked = false;
}

CCornerFinderImageView::~CCornerFinderImageView(void)
{
	if (GetImageData() != NULL)
		delete GetImageData();
	printf("CCornerFinderImageView deleted\n");
}

BEGIN_MESSAGE_MAP(CCornerFinderImageView, CImageView)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_CREATE()
END_MESSAGE_MAP()

#define	TARGET_SIZE			5
#define	TARGET_WHISKER		TARGET_SIZE + 2
#define	TARGET_STR_SPACING	10


void CCornerFinderImageView::OnDraw(CDC* pDC)
{
	CImageView::OnDraw(pDC);
	if (mInputImageNode == NULL)
		return;

	CPaintDC	dc(this); // device context for painting
	CSize	totalSize = GetTotalSize();
	CPen	redPen(PS_SOLID, 1, RGB(255, 0, 0));
	CPen	darkRedPen(PS_SOLID, 1, RGB(128, 0, 0));
	CPen	greenPen(PS_SOLID, 1, RGB(0, 255, 0));
	CPen	darkGreenPen(PS_SOLID, 1, RGB(0, 128, 0));
	CPen	bluePen(PS_SOLID, 1, RGB(0, 0, 255));
	CPen	darkBluePen(PS_SOLID, 1, RGB(0, 0, 128));
	CPen	skyBluePen(PS_SOLID, 1, RGB(0, 255, 255));
	CPen	darkSkyBluePen(PS_SOLID, 1, RGB(0, 128, 128));
	CPen	magentaPen(PS_SOLID, 1, RGB(255, 0, 255));
	CPen	darkMagentaPen(PS_SOLID, 1, RGB(128, 0, 128));
	CPen	*oldPen = pDC->SelectObject(&redPen);
	std::wstringstream		buf;
	double	x, y;
	int	w, h;
	int	dX, dY, dW, dH;

	for (int i = 0; i < mInputImageNode->GetExtractedCornerNum(); i++)
	{
		if (mInputImageNode->GetExtractionMethod(i) != InputImageNode::MANUAL_EXTRACTION_METHOD)
		{
			mInputImageNode->GetExtractedCornerWindowSize(i, &w, &h);
			ConvertToDispScale(w, h, &dW, &dH, false);
			mInputImageNode->GetCornerFinderCenter(i, &x, &y);
			ConvertToDispScale(x, y, &dX, &dY);

			switch (mInputImageNode->GetExtractionMethod(i))
			{
				case InputImageNode::GRID_EXTRACTION_METHOD:
					if (mInputImageNode->GetExtractionResult(i))
						pDC->SelectObject(&darkGreenPen);
					else
						pDC->SelectObject(&darkRedPen);
					break;
				case InputImageNode::CORNER_FIND_METHOD:
					pDC->SelectObject(&darkBluePen);
					break;
				case InputImageNode::REPROJECTION_METHOD:
					pDC->SelectObject(&darkSkyBluePen);
					break;
			}
			pDC->MoveTo(dX - dW, dY - dH);
			pDC->LineTo(dX + dW, dY - dH);
			pDC->LineTo(dX + dW, dY + dH);
			pDC->LineTo(dX - dW, dY + dH);
			pDC->LineTo(dX - dW, dY - dH);
		}
		
		switch (mInputImageNode->GetExtractionMethod(i))
		{
			case InputImageNode::GRID_EXTRACTION_METHOD:
				if (mInputImageNode->GetExtractionResult(i))
					pDC->SelectObject(&greenPen);
				else
					pDC->SelectObject(&redPen);
				break;
			case InputImageNode::CORNER_FIND_METHOD:
				if (mInputImageNode->GetExtractionResult(i))
					pDC->SelectObject(&bluePen);
				else
					pDC->SelectObject(&redPen);
				break;
			case InputImageNode::REPROJECTION_METHOD:
				pDC->SelectObject(&skyBluePen);
				break;
			default:
				pDC->SelectObject(&magentaPen);
				break;
		}

		mInputImageNode->GetExtractedCorner(i, &x, &y);
		ConvertToDispScale(x, y, &dX, &dY);

		pDC->MoveTo(dX, dY - dH);
		pDC->LineTo(dX, dY + dH);
		pDC->MoveTo(dX - dW, dY);
		pDC->LineTo(dX + dW, dY);
	}

	pDC->SelectObject(&darkMagentaPen);
	if (mInputImageNode->GetExtractedCornerNum() == 0)
	{
		mInputImageNode->GetGridExtractorInput(0, &x, &y);
		ConvertToDispScale(x, y, &dX, &dY);
		pDC->MoveTo(dX, dY);
		for (int i = 1; i < mInputImageNode->GetGridExtractorInputNum(); i++)
		{
			mInputImageNode->GetGridExtractorInput(i, &x, &y);
			ConvertToDispScale(x, y, &dX, &dY);
			pDC->LineTo(dX, dY);
		}
		if (mInputImageNode->GetGridExtractorInputNum() == 4)
		{
			mInputImageNode->GetGridExtractorInput(0, &x, &y);
			ConvertToDispScale(x, y, &dX, &dY);
			pDC->LineTo(dX, dY);
		}
	}

	for (int i = 0; i < mInputImageNode->GetGridExtractorInputNum(); i++)
	{
		mInputImageNode->GetGridExtractorInput(i, &x, &y);
		ConvertToDispScale(x, y, &dX, &dY);
		ConvertToDispScale(TARGET_SIZE, TARGET_SIZE, &dW, &dH, false);
		pDC->Arc(dX - dW, dY - dH, dX + dW, dY + dH, dX - dW, 0, dX - dW, 0);

		ConvertToDispScale(TARGET_WHISKER, TARGET_WHISKER, &dW, &dH);
		pDC->MoveTo(dX, dY - dH);
		pDC->LineTo(dX, dY + dH);
		pDC->MoveTo(dX - dW, dY);
		pDC->LineTo(dX + dW, dY);

		ConvertToDispScale(TARGET_STR_SPACING, TARGET_STR_SPACING, &dW, &dH, false);
		buf << i;
		pDC->TextOutW(dX - dW, dY - dH, buf.str().c_str());
		buf.str(L"");
	}
	
	if (mIsMouseCursorTracked)
	{
		pDC->MoveTo(dX, dY);
		pDC->LineTo(mMouseTrackedPos);
	}

	pDC->SelectObject(oldPen);
}

void CCornerFinderImageView::OnLButtonDown(UINT nFlags, CPoint point)
{
	CCalibraDoc	*doc = (CCalibraDoc *)GetDocument();
	if (doc == NULL)
		return;

	if (doc->GetImageMode() != CCalibraDoc::IMAGE_GRID_EXTRACTOR_MODE &&
		doc->GetImageMode() != CCalibraDoc::IMAGE_ARROW_MODE)
	{
		CImageView::OnLButtonDown(nFlags, point);
		return;
	}

	if (doc->GetImageMode() == CCalibraDoc::IMAGE_ARROW_MODE)
	{
		if (mInputImageNode->GetGridExtractorInputNum() != 4)
		{
			CImageView::OnLButtonDown(nFlags, point);
			return;
		}

		int	index;
		if (GetClickedCornerMarker(point, &index) == false)
		{
			CImageView::OnLButtonDown(nFlags, point);
			return;
		}

		mIsLMouseDown = true;
		SetCapture();

		mInputImageNode->ClearAllExtractedCorner();
		mCapturedCornerMarkerIndex = index;
	}
	else
	{
		mIsLMouseDown = true;
		SetCapture();

		int	num = mInputImageNode->GetGridExtractorInputNum();
		if (num < 4)
			num++;

		mInputImageNode->SetGridExtractorInputNum(num);
		mCapturedCornerMarkerIndex = num - 1;
	}

	mIsMouseCursorTracked = false;
	MoveCornerMarker(mCapturedCornerMarkerIndex, point);
	this->Invalidate(false);
}

void CCornerFinderImageView::OnMouseMove(UINT nFlags, CPoint point)
{
	CCalibraDoc	*doc = (CCalibraDoc *)GetDocument();
	bool	isProcessed = false;

	if (doc->GetImageMode() == CCalibraDoc::IMAGE_ARROW_MODE)
	{
		if (mIsLMouseDown)
		{
			mIsMouseCursorTracked = false;
			MoveCornerMarker(mCapturedCornerMarkerIndex, point);
			isProcessed = true;
		}
	}

	if (doc->GetImageMode() == CCalibraDoc::IMAGE_GRID_EXTRACTOR_MODE)
	{
		isProcessed = true;
		if (mIsLMouseDown)
		{
			mIsMouseCursorTracked = false;
			MoveCornerMarker(mCapturedCornerMarkerIndex, point);
		}
		else
		{
			if (mInputImageNode->GetGridExtractorInputNum() == 0 ||
				mInputImageNode->GetGridExtractorInputNum() == 4)
			{
				mIsMouseCursorTracked = false;
				CImageView::OnMouseMove(nFlags, point);
				return;
			}
			mIsMouseCursorTracked = true;
			mMouseTrackedPos = point;
		}
	}

	if (isProcessed == false)
	{
		CImageView::OnMouseMove(nFlags, point);
		return;
	}

	this->Invalidate(false);
}

void CCornerFinderImageView::OnLButtonUp(UINT nFlags, CPoint point)
{
	CCalibraDoc	*doc = (CCalibraDoc *)GetDocument();
	if (doc == NULL)
		return;

	if (doc->GetImageMode() != CCalibraDoc::IMAGE_GRID_EXTRACTOR_MODE &&
		doc->GetImageMode() != CCalibraDoc::IMAGE_ARROW_MODE ||
		mIsLMouseDown == false)
	{
		CImageView::OnLButtonUp(nFlags, point);
		return;
	}

	MoveCornerMarker(mCapturedCornerMarkerIndex, point);

	if (mInputImageNode->GetGridExtractorInputNum() == 4)
	{
		mInputImageNode->ExecGridExtractor(*GetImageData());
		doc->SetImageMode(CCalibraDoc::IMAGE_ARROW_MODE);
	}

	mIsLMouseDown = false;
	ReleaseCapture();
	this->Invalidate(false);
}

bool	CCornerFinderImageView::GetClickedCornerMarker(CPoint inPoint, int *outIndex)
{
	double	x, y, cx, cy;

	ConvertToImageScale(inPoint.x, inPoint.y, &cx, &cy);

	for (int i = 0; i < mInputImageNode->GetGridExtractorInputNum(); i++)
	{
		mInputImageNode->GetGridExtractorInput(i, &x, &y);
		if (cx >= x - TARGET_SIZE && cx <= x + TARGET_SIZE &&
			cy >= y - TARGET_SIZE && cy <= y + TARGET_SIZE)
		{
			*outIndex = i;
			return true;
		}
	}

	return false;
}

void	CCornerFinderImageView::MoveCornerMarker(int inIndex, CPoint inPoint)
{
	CPoint	offset = GetScrollPosition();

	double	x = inPoint.x + offset.x;
	double	y = inPoint.y + offset.y;
	if (GetDispScale() != 100)
	{
		double	scale = GetDispScale() / 100.0;
		x = x / scale;
		y = y / scale;
	}

	mInputImageNode->SetGridExtractorInput(inIndex, x, y);
}

int	CCornerFinderImageView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CImageView::OnCreate(lpCreateStruct) == -1)
		return -1;

	CCalibraDoc	*doc = (CCalibraDoc *)GetDocument();
	if (doc == NULL)
		return -1;

	mInputImageNode = (InputImageNode *)doc->GetSelectedImageNode();
	if (mInputImageNode == NULL)
	{
		TRACE0("Failed to get selected image node\n");
		return -1;      // failed to create
	}

	SetImageData(new ImageData());
	GetImageData()->OpenBitmapFile(mInputImageNode->GetCachedFilePath().c_str());

	return 0;
}

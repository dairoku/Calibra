// =============================================================================
//  CornerFinderImageView.h
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

#pragma once
#include "ImageView.h"

class CCornerFinderImageView :
	public CImageView
{
public:
	CCornerFinderImageView(void);
	virtual ~CCornerFinderImageView(void);
	DECLARE_MESSAGE_MAP()
protected:
	virtual void OnDraw(CDC* /*pDC*/);
public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);

	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

private:
	bool	mIsLMouseDown;
	bool	mIsMouseCursorTracked;
	InputImageNode	*mInputImageNode;
	CPoint	mMouseTrackedPos;

	void	ConvertToDispScale(double inX, double inY, int *outX, int *outY, bool inIsAbsolute = true)
	{
		if (GetDispScale() == 100)
		{
			*outX = (int )inX;
			*outY = (int )inY;
		}
		else
		{
			double	scale = GetDispScale() / 100.0;
			if (GetDispScale() >= 300 && inIsAbsolute)
			{
				inX += 0.5;
				inY += 0.5;
			}
			*outX = (int )(inX * scale);
			*outY = (int )(inY * scale);
		}
	}

	void	ConvertToImageScale(int inX, int inY, double *outX, double *outY)
	{
		CPoint	offset = GetScrollPosition();

		*outX = inX + offset.x;
		*outY = inY + offset.y;

		if (GetDispScale() != 100)
		{
			double	scale = GetDispScale() / 100.0;
			*outX = *outX / scale;
			*outY = *outY / scale;
			if (GetDispScale() >= 300)
			{
				*outX -= 0.5;
				*outY -= 0.5;
			}
		}
	}

	int		mCapturedCornerMarkerIndex;

	bool	GetClickedCornerMarker(CPoint inPoint, int *outIndex);
	void	MoveCornerMarker(int inIndex, CPoint inPoint);
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};

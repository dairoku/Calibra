// =============================================================================
//  ImageView.h
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

#include "ImageData.hpp"


// CImageView ビュー

class CImageView : public CScrollView
{
	DECLARE_DYNCREATE(CImageView)

protected:
	CImageView();           // 動的生成で使用される protected コンストラクタ
	virtual ~CImageView();

public:
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

public:
	ImageData *GetImageData()
	{
		return mImageData;
	}
	void	SetImageData(ImageData *inImageData);
	void	UpdateImageSize();

	int		GetDispScale()
	{
		return mImageDispScale;
	}
	void	SetDispScale(int inScale);

	void	ZoomIn();
	void	ZoomOut();

private:
	ImageData	*mImageData;
	int			mImageDispScale;

protected:
	virtual void OnDraw(CDC* pDC);      // このビューを描画するためにオーバーライドされます。
	virtual void OnInitialUpdate();     // 構築後 1 回目

	DECLARE_MESSAGE_MAP()
public:
};



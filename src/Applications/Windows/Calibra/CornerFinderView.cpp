// =============================================================================
//  CornerFinderView.cpp
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

// CornerFinderView.cpp : 実装ファイル
//

#include "stdafx.h"
#include "Calibra.h"
#include "CalibraData.hpp"
#include "CalibraDoc.h"
#include "CornerFinderView.h"
#include "afxpriv.h"

// CCornerFinderView

IMPLEMENT_DYNCREATE(CCornerFinderView, CView)

CCornerFinderView::CCornerFinderView()
{
	mImageView = NULL;
}

CCornerFinderView::~CCornerFinderView()
{
}

BEGIN_MESSAGE_MAP(CCornerFinderView, CView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_NOTIFY(TCN_SELCHANGE, AFX_IDW_PANE_FIRST, OnTabSelChanged)
//	ON_WM_PAINT()
END_MESSAGE_MAP()


// CCornerFinderView 描画

void CCornerFinderView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: 描画コードをここに追加してください。
}


// CCornerFinderView 診断

#ifdef _DEBUG
void CCornerFinderView::AssertValid() const
{
	CView::AssertValid();
}

#ifndef _WIN32_WCE
void CCornerFinderView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif
#endif //_DEBUG


// CCornerFinderView メッセージ ハンドラ

int CCornerFinderView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	CCalibraDoc	*doc = (CCalibraDoc *)GetDocument();
	if (doc == NULL)
		return -1;

	CRect	rect;
	GetClientRect(rect);

	mTabCtrl.Create(WS_CHILD | WS_VISIBLE, rect, this, AFX_IDW_PANE_FIRST);
	//mTabCtrl.Create(TCS_BUTTONS  | TCS_FLATBUTTONS | WS_CHILD | WS_VISIBLE, rect, this, AFX_IDW_PANE_FIRST);
	//mTabCtrl.Create(TCS_OWNERDRAWFIXED  | WS_CHILD | WS_VISIBLE, rect, this, AFX_IDW_PANE_FIRST);
	mTabCtrl.InsertItem(0, TEXT("CornerFinder"));
	mTabCtrl.InsertItem(1, TEXT("Test"));
	mTabCtrl.AdjustRect(false, &rect);
	//mTabCtrl.SetExtendedStyle(TCS_EX_FLATSEPARATORS );

	LOGFONTW	logFont;
	GetObject(GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONTW), &logFont);
	logFont.lfHeight = 12;
	logFont.lfWidth = 0;
	mFont.CreateFontIndirectW(&logFont);
	mTabCtrl.SetFont(&mFont);

	if (!mToolBar.CreateEx(&mTabCtrl, TBSTYLE_FLAT) ||
		!mToolBar.LoadToolBar(IDR_CORNER_BAR))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // failed to create
	}
	//mToolBar.SetBarStyle(CBRS_ALIGN_TOP | CBRS_BORDER_RIGHT);


	CCreateContext context;
	ASSERT(context.m_pCurrentFrame == NULL);
	context.m_pLastView = this;
	context.m_pCurrentDoc = GetDocument();
	if (context.m_pCurrentDoc != NULL)
		context.m_pNewDocTemplate = context.m_pCurrentDoc->GetDocTemplate();

	mImageView = new CCornerFinderImageView();
	mImageView->Create(NULL, NULL, AFX_WS_DEFAULT_VIEW, rect, &mTabCtrl, AFX_IDW_PANE_FIRST, &context);
	mImageView->SendMessage(WM_INITIALUPDATE);
	//mImageView.ShowWindow(SW_HIDE);

	return 0;
}

void CCornerFinderView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	if (mTabCtrl.GetSafeHwnd())
 	{
		CRect	rect, toolBarRect;
		CSize	toolBarSize;

		GetClientRect(rect);
		if (rect.Size().cx <= 0 || rect.Size().cy <= 0)
			return;

		mTabCtrl.MoveWindow(rect, true);
		mTabCtrl.AdjustRect(false, rect);

		CToolBarCtrl &toolBarCtrl = mToolBar.GetToolBarCtrl();
		toolBarCtrl.GetMaxSize(&toolBarSize);
		toolBarRect = rect;
		//toolBarRect.right = rect.left + toolBarSize.cx;
		toolBarRect.bottom = rect.top + toolBarSize.cy;
		rect.top += toolBarSize.cy;
		mToolBar.MoveWindow(toolBarRect, true);
		mImageView->MoveWindow(rect, true);
	}
}

void CCornerFinderView::OnTabSelChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	if (mTabCtrl.GetCurSel() == 0)
	{
		mImageView->ShowWindow(SW_SHOW);
		mToolBar.ShowWindow(SW_SHOW);
	}
	else
	{
		mImageView->ShowWindow(SW_HIDE);
		mToolBar.ShowWindow(SW_HIDE);
	}
}

//BOOL CCornerFinderView::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
//{
//	if (mImageView->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo) != false)
//		return true;
//
//	return CView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
//}

//void CCornerFinderView::OnPaint()
//{
//	CPaintDC dc(this); // device context for painting
//	CRect	rect;
//
//	GetClientRect(rect);
//	dc.FillSolidRect(rect, RGB(255, 0, 0));
//}

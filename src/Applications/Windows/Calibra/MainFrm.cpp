// =============================================================================
//  MainFrm.cpp
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

#include "MainFrm.h"
#include "ContainerView.h"
#include "ProjectView.h"
#include "ImageListView.h"
#include "ImageView.h"
#include "PrintfView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
//	ON_WM_SIZING()
ON_WM_DESTROY()
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};


// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	mIgnoreOnSize = true;
}

CMainFrame::~CMainFrame()
{
}


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Delete these three lines if you don't want the toolbar to be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	return 0;
}

void CMainFrame::OnDestroy()
{
	WINDOWPLACEMENT	wp;
	if (GetWindowPlacement(&wp))
	{
		if(IsZoomed())
			wp.flags |= WPF_RESTORETOMAXIMIZED;

		CWinApp*  app = AfxGetApp();
		app->WriteProfileBinary(TEXT("AppSettings"), TEXT("WindowPlacement"), (LPBYTE )&wp, sizeof(WINDOWPLACEMENT));
		app->WriteProfileInt(TEXT("AppSettings"), TEXT("PrintfViewHeight"), mPrintfViewHeight);
	}

	__super::OnDestroy();

	//	‚»‚Ì‚Ù‚©‚Ìˆ—
}


BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT /*lpcs*/,
	CCreateContext* pContext)
{
	CWinApp*  app = AfxGetApp();
	mPrintfViewHeight = app->GetProfileInt(TEXT("AppSettings"), TEXT("PrintfViewHeight"), 150);

	// create splitter window
	if (!mWndSplitter0.CreateStatic(this, 2, 1))
		return FALSE;

	if (!mWndSplitter1.CreateStatic(&mWndSplitter0, 1, 2,
		WS_CHILD | WS_VISIBLE, mWndSplitter0.IdFromRowCol(0, 0)))
		return FALSE;

	if (!mWndSplitter2.CreateStatic(&mWndSplitter1, 2, 1,
		WS_CHILD | WS_VISIBLE, mWndSplitter1.IdFromRowCol(0, 1)))
		return FALSE;

	if (!mWndSplitter0.CreateView(1, 0, RUNTIME_CLASS(CPrintfView), CSize(10, 10), pContext) ||
		!mWndSplitter1.CreateView(0, 0, RUNTIME_CLASS(CProjectView), CSize(200, 100), pContext) ||
		!mWndSplitter2.CreateView(0, 0, RUNTIME_CLASS(CImageListView), CSize(100, 100), pContext) ||
		!mWndSplitter2.CreateView(1, 0, RUNTIME_CLASS(CContainerView), CSize(100, 100), pContext))
	{
		mWndSplitter2.DestroyWindow();
		mWndSplitter1.DestroyWindow();
		mWndSplitter0.DestroyWindow();
		return FALSE;
	}
	CRect	rect;

	GetClientRect(rect);
	mWndSplitter0.SetRowInfo(0, rect.bottom - mPrintfViewHeight, 50);
	mWndSplitter0.SetCallback(this);

	mIgnoreOnSize = false;

	return TRUE;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}


// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

void CMainFrame::CMySplitterWndCallbackFunc(CMySplitterWnd *inSplitterObject)
{
	int	value, min;
	CRect	rect;

	GetClientRect(rect);
	mWndSplitter0.GetRowInfo(0, value, min);

	mPrintfViewHeight = rect.bottom - value;
}

void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
	CFrameWnd::OnSize(nType, cx, cy);

	if (mIgnoreOnSize)
		return;

	if (cy - mPrintfViewHeight > 0)
	{
		mWndSplitter0.SetRowInfo(0, cy - mPrintfViewHeight, 50);
		mWndSplitter0.RecalcLayout();
	}
}

//void CMainFrame::OnSizing(UINT fwSide, LPRECT pRect)
//{
//	CFrameWnd::OnSizing(fwSide, pRect);
//
//	if (mIgnoreOnSize)
//		return;
//
//	if (mIsWindowSizing)
//		return;
//
//	int	min;
//	mWndSplitter0.GetRowInfo(1, mPrintfViewHeight, min);
//	mIsWindowSizing = true;
//	printf("mPrintfViewHeight:%d\n", mPrintfViewHeight);
//}



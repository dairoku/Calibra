// =============================================================================
//  ContainerView.cpp
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
#include "ContainerView.h"
#include "CalibraDoc.h"
#include "afxpriv.h"

// CContainerView

IMPLEMENT_DYNCREATE(CContainerView, CView)

CContainerView::CContainerView()
{
	mChildWnd = NULL;
}

CContainerView::~CContainerView()
{
}

BEGIN_MESSAGE_MAP(CContainerView, CView)
	ON_WM_SIZE()
	ON_WM_CREATE()
END_MESSAGE_MAP()


// CContainerView 描画

void CContainerView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: 描画コードをここに追加してください。
}


// CContainerView 診断

#ifdef _DEBUG
void CContainerView::AssertValid() const
{
	CView::AssertValid();
}

#ifndef _WIN32_WCE
void CContainerView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif
#endif //_DEBUG

// CContainerView メッセージ ハンドラ


BOOL CContainerView::CreateChildView(CRuntimeClass *pViewClass, CCreateContext* pContext)
{
#ifdef _DEBUG
	ASSERT_VALID(this);
	ASSERT(pViewClass != NULL);
	ASSERT(pViewClass->IsDerivedFrom(RUNTIME_CLASS(CWnd)));
	ASSERT(AfxIsValidAddress(pViewClass, sizeof(CRuntimeClass), FALSE));
#endif

	if (GetChildView() != NULL)
		DeleteView();

	BOOL bSendInitialUpdate = FALSE;

	CCreateContext contextT;
	if (pContext == NULL)
	{
		// if no context specified, generate one from the currently selected
		//  client if possible
		// set info about last pane
		ASSERT(contextT.m_pCurrentFrame == NULL);
		contextT.m_pLastView = this;
		contextT.m_pCurrentDoc = GetDocument();
		if (contextT.m_pCurrentDoc != NULL)
			contextT.m_pNewDocTemplate =
			  contextT.m_pCurrentDoc->GetDocTemplate();

		pContext = &contextT;
		bSendInitialUpdate = TRUE;
	}

	TRY
	{
		mChildWnd = (CWnd *)pViewClass->CreateObject();
		if (mChildWnd == NULL)
			AfxThrowMemoryException();
	}
	CATCH_ALL(e)
	{
		TRACE(traceAppMsg, 0, "Out of memory creating a container.\n");
		// Note: DELETE_EXCEPTION(e) not required
		return FALSE;
	}
	END_CATCH_ALL

	ASSERT_KINDOF(CWnd, mChildWnd);
	ASSERT(mChildWnd->m_hWnd == NULL);       // not yet created

	DWORD dwStyle = AFX_WS_DEFAULT_VIEW & ~WS_BORDER;

	// Create with the right size (wrong position)
	CRect rect(CPoint(0,0), CPoint(0,0));
	if (!mChildWnd->Create(NULL, NULL, dwStyle, rect, this, AFX_IDW_PANE_FIRST, pContext))
	{
		TRACE(traceAppMsg, 0, "Warning: couldn't create client pane for splitter.\n");
			// pWnd will be cleaned up by PostNcDestroy
		return FALSE;
	}
	//ASSERT((int)_AfxGetDlgCtrlID(pWnd->m_hWnd) == IdFromRowCol(row, col));

	// send initial notification message
	if (bSendInitialUpdate)
	{
		mChildWnd->SendMessage(WM_INITIALUPDATE);

		CRect	rect;
		GetClientRect(rect);
		mChildWnd->MoveWindow(rect);
	}

	return TRUE;
}

void CContainerView::DeleteView()
{
	ASSERT_VALID(this);

	if (mChildWnd == NULL)
		return;

	// default implementation assumes view will auto delete in PostNcDestroy
	mChildWnd->DestroyWindow();
	mChildWnd = NULL;
}

void CContainerView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	if (mChildWnd == NULL)
		return;

	mChildWnd->MoveWindow(0, 0, cx, cy);
}

int CContainerView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	((CCalibraDoc *)GetDocument())->SetContainerView(this);

	return 0;
}

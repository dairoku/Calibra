// =============================================================================
//  PrintfView.cpp
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
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <fstream>
#include "Calibra.h"
#include "PrintfView.h"


// CPrintfView
// This code assumes that:
//	MFC(VC++ Project) Character code setting is "UNICODE"
//	standard stream was opend as "ASCII" stream

IMPLEMENT_DYNCREATE(CPrintfView, CView)

CPrintfView::CPrintfView()
{

}

CPrintfView::~CPrintfView()
{
}

BEGIN_MESSAGE_MAP(CPrintfView, CView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CPrintfView 描画

void CPrintfView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: 描画コードをここに追加してください。
}


// CPrintfView 診断

#ifdef _DEBUG
void CPrintfView::AssertValid() const
{
	CView::AssertValid();
}

#ifndef _WIN32_WCE
void CPrintfView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif
#endif //_DEBUG


// CPrintfView メッセージ ハンドラ

int CPrintfView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect	rect;
	GetClientRect(rect);

	mEdit.Create(WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
		rect, this, AFX_IDW_PANE_FIRST);
	mEdit.ShowScrollBar(SB_VERT);

	int	hConHandle;
/*	AllocConsole();
	hConHandle = ::_open_osfhandle((intptr_t)GetStdHandle(STD_OUTPUT_HANDLE), _O_WTEXT);
	fp = ::_fdopen(hConHandle, "w");
	freopen_s(&fp, "CONOUT$", "w", stdout);
*/
	// https://stackoverflow.com/questions/311955/redirecting-cout-to-a-console-in-windows/424236#424236
	/// Re-initialize the C runtime "FILE" handles with clean handles bound to "nul".We do this because it has been
	// observed that the file number of our standard handle file objects can be assigned internally to a value of -2
	// when not bound to a valid target, which represents some kind of unknown internal invalid state. In this state our
	// call to "_dup2" fails, as it specifically tests to ensure that the target file number isn't equal to this value
	// before allowing the operation to continue. We can resolve this issue by first "re-opening" the target files to
	// use the "nul" device, which will place them into a valid state, after which we can redirect them to our target
	// using the "_dup2" function.
	mDummyFile;
	freopen_s(&mDummyFile, "nul", "w", stdout);//<- This is important

	// The following code is based on:
	// http://www.halcyon.com/~ast/dload/guicon.htm
	BOOL	result;
	result = ::CreatePipe(&mConsoleHandle, &mConsoleWriteHandle, NULL, 0);
	mOriginalStdHandle = ::GetStdHandle(STD_OUTPUT_HANDLE);
	result = ::SetStdHandle(STD_OUTPUT_HANDLE, mConsoleWriteHandle);

	hConHandle = ::_open_osfhandle((intptr_t )mConsoleWriteHandle, _O_TEXT);
	mRedirectFile = ::_fdopen(hConHandle, "w");
	int dup2Result = _dup2(_fileno(mRedirectFile), _fileno(stdout));//<- This is important
	::setvbuf(stdout, NULL, _IONBF, 0); //<- This is important
	std::wcout.clear();
	std::cout.clear();

	//freopen_s(&fp, "CONOUT$", "w", stdout);
	//*stdout = *fp;
	//::setvbuf(stdout, NULL, _IONBF, 0); //<- This is important
	//setvbuf( stdout, mBuffer, _IOFBF, 10240);
	//std::ios::sync_with_stdio();

	mLoop = true;
	mThread = ::AfxBeginThread(Run, (LPVOID)this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED, NULL);
	mThread->m_bAutoDelete = FALSE;
	mThread->ResumeThread();

	//LOGFONTW	logFont;
	//GetObject(GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONTW), &logFont);
	//logFont.lfHeight = 12;
	//mFont.CreateFontIndirectW(&logFont);
	//mFont.CreatePointFont(120, TEXT("Arial"));	// Propotional Font seems to be slow...
	mFont.CreatePointFont(100, TEXT("Terminal"));
	mEdit.SetFont(&mFont);

	return 0;
}

void CPrintfView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	if (mEdit.GetSafeHwnd())
	{
		CRect	rect;
		GetClientRect(rect);
		if (rect.Size().cx <= 0 || rect.Size().cy <= 0)
			return;

		mEdit.MoveWindow(rect, true);
	}
}

UINT CPrintfView::Run(LPVOID pParam)
{
	CPrintfView	*view = (CPrintfView *)pParam;
	char	buf[257];
	DWORD	size;
	BOOL	result;

	while (view->mLoop)
	{
		result = ::ReadFile(view->mConsoleHandle,
			buf,
			256,
			&size,
			NULL);
		if (result == 0)
		{
			size = ::GetLastError();
		}
		else
		{
			buf[size] = 0;
			view->mQueue.push(std::string(buf));
			view->PostMessage(WM_TIMER);
		}
	}

	return 0;
}

void CPrintfView::OnTimer(UINT_PTR nIDEvent)
{
	while (mQueue.empty() == false)
	{
		int	limit, len;
		std::string	&str = mQueue.front();
		int wcharsize = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
		WCHAR	*wideString = new WCHAR[wcharsize];
		MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, wideString, wcharsize);

		limit =mEdit.GetLimitText();
		len = mEdit.GetWindowTextLength();
		if (limit - len < 10)
		{
			mEdit.SetSel(0, limit / 2);
			mEdit.ReplaceSel(TEXT(""));
			len = mEdit.GetWindowTextLength();
		}
		mEdit.SetSel(len, len);
		mEdit.ReplaceSel(wideString);
		mQueue.pop();

		delete wideString;
	}

	CView::OnTimer(nIDEvent);
}

void CPrintfView::OnDestroy()
{
	CView::OnDestroy();

	mLoop = false;

	BOOL	result = ::SetStdHandle(STD_OUTPUT_HANDLE, mOriginalStdHandle);
	fclose(mRedirectFile);
	fclose(mDummyFile);

	::CloseHandle(mConsoleHandle);
	::CloseHandle(mConsoleWriteHandle);
	//::WaitForSingleObject(mThread->m_hThread, INFINITE);
	//delete mThread;
}

HBRUSH CPrintfView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	return (HBRUSH )GetStockObject(WHITE_BRUSH);
}

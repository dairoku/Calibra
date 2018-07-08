// =============================================================================
//  MyTabCtrl.cpp
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
#include "MyTabCtrl.h"


// CMyTabCtrl

IMPLEMENT_DYNAMIC(CMyTabCtrl, CTabCtrl)

CMyTabCtrl::CMyTabCtrl()
{
	mParentView = NULL;
}

CMyTabCtrl::~CMyTabCtrl()
{
}


BEGIN_MESSAGE_MAP(CMyTabCtrl, CTabCtrl)
//	ON_WM_CTLCOLOR()
//	ON_WM_PAINT()
END_MESSAGE_MAP()



// CMyTabCtrl メッセージ ハンドラ


BOOL CMyTabCtrl::Create(DWORD dwStyle, const RECT& rect, CView* pParentView, UINT nID)
{
	mParentView = pParentView;

	return CTabCtrl::Create(dwStyle, rect, pParentView, nID);
}

BOOL CMyTabCtrl::CreateEx(DWORD dwExStyle, DWORD dwStyle, const RECT& rect, CView* pParentView, UINT nID)
{
	mParentView = pParentView;

	return CTabCtrl::CreateEx(dwExStyle, dwStyle, rect, pParentView, nID);
}

BOOL CMyTabCtrl::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if (mParentView->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return true;

	return CTabCtrl::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

//HBRUSH CMyTabCtrl::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
//{
//	//HBRUSH hbr = CTabCtrl::OnCtlColor(pDC, pWnd, nCtlColor);
//
//	// TODO:  ここで DC の属性を変更してください。
//
//	// TODO:  既定値を使用したくない場合は別のブラシを返します。
//	return (HBRUSH )GetStockObject(BLACK_BRUSH);;
//}

//void CMyTabCtrl::OnPaint()
//{
//	CPaintDC dc(this); // device context for painting
//	// TODO: ここにメッセージ ハンドラ コードを追加します。
//	// 描画メッセージで CTabCtrl::OnPaint() を呼び出さないでください。
//
//	// 最初にデフォルトの描画処理を行うため、以下の処理を記述する
//	Default();
//		//または以下の記述
//		//const MSG *msg = GetCurrentMessage();
//		//DefWindowProc( msg->message, msg->wParam, msg->lParam );
//
//	// TODO: この位置にメッセージ ハンドラ用のコードを追加してください
//	CRect rect;
//	GetItemRect(GetCurSel(), &rect);  // 選択されているページのタブに外接する四角形の取得
//
//	// タブの背景をウィンドウの色で描画（レイアウト上、四角形は少し小さくする）
//	rect.InflateRect(-2, -2);
//	dc.FillSolidRect(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
//		GetSysColor(COLOR_WINDOW));
//
//
//	CTabCtrl::OnPaint();
///*
//
//
//	// 最初にデフォルトの描画処理を行うため、以下の処理を記述する
//	Default();
//		//または以下の記述
//		//const MSG *msg = GetCurrentMessage();
//		//DefWindowProc( msg->message, msg->wParam, msg->lParam );
//
//	// デバイスコンテキストはCClientDCを使用する
//	//CPaintDC dc(this); // 描画用のデバイス コンテキスト
//	CClientDC dc(this);
//
//	// TODO: この位置にメッセージ ハンドラ用のコードを追加してください
//	CRect rect;
//	GetItemRect(GetCurSel(), &rect);  // 選択されているページのタブに外接する四角形の取得
//
//	// タブの背景をウィンドウの色で描画（レイアウト上、四角形は少し小さくする）
//	rect.InflateRect(-2, -2);
//	dc.FillSolidRect(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
//		GetSysColor(COLOR_WINDOW));
//
//	// タブに描画する文字のフォントを設定
//	// （親ウィンドウであるプロパティシートと同じフォントにする）
//	// m_Fontはコントロールクラスのメンバ変数(CFont型)
//	LOGFONT lf, parentlf;
//	GetFont()->GetLogFont(&lf);
//	GetParent()->GetFont()->GetLogFont(&parentlf);
//	lf.lfHeight = parentlf.lfHeight;
//	strcpy(lf.lfFaceName, parentlf.lfFaceName);
//	m_Font.CreateFontIndirect(&parentlf);
//	CFont* pOldFont = dc.SelectObject(&m_Font);
//
//	// 文字を描画する
//	CString str;
//	CPropertyPage* pPage = ((CPropertySheet*)GetParent())->GetPage(GetCurSel());
//	if( pPage->GetSafeHwnd() ) {
//		pPage->GetWindowText(str);  // 現在開いているページのタブの文字を取得する
//		dc.DrawText(str, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
//	}
//
//	// フォントを元に戻す
//	dc.SelectObject(pOldFont);
//	m_Font.DeleteObject();
//*/
//	// 描画用メッセージとして CTabCtrl::OnPaint() を呼び出してはいけません
//
//
//}

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



// CMyTabCtrl ���b�Z�[�W �n���h��


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
//	// TODO:  ������ DC �̑�����ύX���Ă��������B
//
//	// TODO:  ����l���g�p�������Ȃ��ꍇ�͕ʂ̃u���V��Ԃ��܂��B
//	return (HBRUSH )GetStockObject(BLACK_BRUSH);;
//}

//void CMyTabCtrl::OnPaint()
//{
//	CPaintDC dc(this); // device context for painting
//	// TODO: �����Ƀ��b�Z�[�W �n���h�� �R�[�h��ǉ����܂��B
//	// �`�惁�b�Z�[�W�� CTabCtrl::OnPaint() ���Ăяo���Ȃ��ł��������B
//
//	// �ŏ��Ƀf�t�H���g�̕`�揈�����s�����߁A�ȉ��̏������L�q����
//	Default();
//		//�܂��͈ȉ��̋L�q
//		//const MSG *msg = GetCurrentMessage();
//		//DefWindowProc( msg->message, msg->wParam, msg->lParam );
//
//	// TODO: ���̈ʒu�Ƀ��b�Z�[�W �n���h���p�̃R�[�h��ǉ����Ă�������
//	CRect rect;
//	GetItemRect(GetCurSel(), &rect);  // �I������Ă���y�[�W�̃^�u�ɊO�ڂ���l�p�`�̎擾
//
//	// �^�u�̔w�i���E�B���h�E�̐F�ŕ`��i���C�A�E�g��A�l�p�`�͏�������������j
//	rect.InflateRect(-2, -2);
//	dc.FillSolidRect(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
//		GetSysColor(COLOR_WINDOW));
//
//
//	CTabCtrl::OnPaint();
///*
//
//
//	// �ŏ��Ƀf�t�H���g�̕`�揈�����s�����߁A�ȉ��̏������L�q����
//	Default();
//		//�܂��͈ȉ��̋L�q
//		//const MSG *msg = GetCurrentMessage();
//		//DefWindowProc( msg->message, msg->wParam, msg->lParam );
//
//	// �f�o�C�X�R���e�L�X�g��CClientDC���g�p����
//	//CPaintDC dc(this); // �`��p�̃f�o�C�X �R���e�L�X�g
//	CClientDC dc(this);
//
//	// TODO: ���̈ʒu�Ƀ��b�Z�[�W �n���h���p�̃R�[�h��ǉ����Ă�������
//	CRect rect;
//	GetItemRect(GetCurSel(), &rect);  // �I������Ă���y�[�W�̃^�u�ɊO�ڂ���l�p�`�̎擾
//
//	// �^�u�̔w�i���E�B���h�E�̐F�ŕ`��i���C�A�E�g��A�l�p�`�͏�������������j
//	rect.InflateRect(-2, -2);
//	dc.FillSolidRect(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
//		GetSysColor(COLOR_WINDOW));
//
//	// �^�u�ɕ`�悷�镶���̃t�H���g��ݒ�
//	// �i�e�E�B���h�E�ł���v���p�e�B�V�[�g�Ɠ����t�H���g�ɂ���j
//	// m_Font�̓R���g���[���N���X�̃����o�ϐ�(CFont�^)
//	LOGFONT lf, parentlf;
//	GetFont()->GetLogFont(&lf);
//	GetParent()->GetFont()->GetLogFont(&parentlf);
//	lf.lfHeight = parentlf.lfHeight;
//	strcpy(lf.lfFaceName, parentlf.lfFaceName);
//	m_Font.CreateFontIndirect(&parentlf);
//	CFont* pOldFont = dc.SelectObject(&m_Font);
//
//	// ������`�悷��
//	CString str;
//	CPropertyPage* pPage = ((CPropertySheet*)GetParent())->GetPage(GetCurSel());
//	if( pPage->GetSafeHwnd() ) {
//		pPage->GetWindowText(str);  // ���݊J���Ă���y�[�W�̃^�u�̕������擾����
//		dc.DrawText(str, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
//	}
//
//	// �t�H���g�����ɖ߂�
//	dc.SelectObject(pOldFont);
//	m_Font.DeleteObject();
//*/
//	// �`��p���b�Z�[�W�Ƃ��� CTabCtrl::OnPaint() ���Ăяo���Ă͂����܂���
//
//
//}

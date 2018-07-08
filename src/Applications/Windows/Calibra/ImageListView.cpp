// =============================================================================
//  ImageListView.cpp
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
#include <sstream>
#include "Calibra.h"

#include "CalibraDoc.h"
#include "ImageListView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define	STR_BUF_LEN			256

// CImageListView

IMPLEMENT_DYNCREATE(CImageListView, CListView)

BEGIN_MESSAGE_MAP(CImageListView, CListView)
	ON_WM_STYLECHANGED()
//	ON_NOTIFY_REFLECT(LVN_ODSTATECHANGED, &CImageListView::OnLvnOdstatechanged)
//ON_NOTIFY_REFLECT(LVN_ITEMACTIVATE, &CImageListView::OnLvnItemActivate)
ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, &CImageListView::OnLvnItemchanged)
ON_WM_CREATE()
END_MESSAGE_MAP()

// CImageListView construction/destruction

CImageListView::CImageListView()
{
	mListCtrl = NULL;
	mImageFolderNode = NULL;
}

CImageListView::~CImageListView()
{
}

BOOL CImageListView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CListView::PreCreateWindow(cs);
}

void CImageListView::OnInitialUpdate()
{
	CListView::OnInitialUpdate();
}


// CImageListView diagnostics

#ifdef _DEBUG
void CImageListView::AssertValid() const
{
	CListView::AssertValid();
}

void CImageListView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}

CCalibraDoc* CImageListView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CCalibraDoc)));
	return (CCalibraDoc*)m_pDocument;
}
#endif //_DEBUG


// CImageListView message handlers
void CImageListView::OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct)
{
	//TODO: add code to react to the user changing the view style of your window
	CListView::OnStyleChanged(nStyleType,lpStyleStruct);
}

void CImageListView::OnLvnItemchanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	*pResult = 0;

	if ((pNMLV->uNewState & LVIS_SELECTED) != LVIS_SELECTED)
		return;

	int	index = mListCtrl->GetNextItem(-1, LVNI_SELECTED);
	ImageNode	*node = (InputImageNode *)mImageFolderNode->GetChildNode(index);

	GetDocument()->ImageListViewSelChanged(node);
}

int CImageListView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CListView::OnCreate(lpCreateStruct) == -1)
		return -1;

	mListCtrl = &(GetListCtrl());

	//DWORD dwExStyle = LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES;
	DWORD dwExStyle = LVS_EX_FULLROWSELECT;
	mListCtrl->SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, (WPARAM)0, (LPARAM)dwExStyle);
	ModifyStyle(false, LVS_SINGLESEL | LVS_SHOWSELALWAYS);
	ModifyStyle(LVS_TYPEMASK, LVS_REPORT);

	GetDocument()->SetImageListView(this);

	mListCtrl->InsertColumn(0, TEXT("No."));
	mListCtrl->InsertColumn(1, TEXT("Name"));
	mListCtrl->InsertColumn(2, TEXT("Relative Path"));
	mListCtrl->InsertColumn(3, TEXT("Absolute Path"));

	mListCtrl->SetColumnWidth(0, 50);
	mListCtrl->SetColumnWidth(1, 150);
	mListCtrl->SetColumnWidth(2, 350);
	mListCtrl->SetColumnWidth(3, 550);

	return 0;
}

void	CImageListView::SelectImageNode(ImageNode *inNode)
{
	if (inNode->GetParentNode() == NULL)
		return;

	int	index;
	if (inNode->GetParentNode()->GetChildNodeIndex(inNode, &index) == false)
		return;

	mListCtrl->SetItemState(index, LVIS_SELECTED, LVIS_SELECTED);
}

// -----------------------------------------------------------------------------
//	SetImageFolderNode
// -----------------------------------------------------------------------------
//
void	CImageListView::SetImageFolderNode(ImageFolderNode *inNode)
{
	mImageFolderNode = inNode;
	if (mImageFolderNode == NULL)
	{
		mListCtrl->DeleteAllItems();
		return;
	}

	std::vector<CalibraNode *>::const_iterator	it;
	int i = 0;
	std::wstringstream		buf;

	for (it = inNode->begin(); it != inNode->end(); ++it, i++)
	{
		buf << i;
		mListCtrl->InsertItem(i, buf.str().c_str());
		mListCtrl->SetItemText(i, 1, (*it)->GetName().c_str());
		mListCtrl->SetItemText(i, 2, ((InputImageNode *)*it)->GetFilePath().c_str());
		mListCtrl->SetItemText(i, 3, ((InputImageNode *)*it)->GetCachedFilePath().c_str());
		buf.str(L"");
		buf.clear();
	}
}
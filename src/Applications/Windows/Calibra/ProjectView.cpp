// =============================================================================
//  ProjectView.cpp
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
/*!
	\file		ProjectView.cpp
	\author		Dairoku Sekiguchi
	\version	1.0
	\date		2007/10/31
	\brief

	Description...
*/

// -----------------------------------------------------------------------------
// 	include files
// -----------------------------------------------------------------------------
#include "stdafx.h"
#include "Calibra.h"
#include "CalibraDoc.h"
#include "ProjectView.h"


// -----------------------------------------------------------------------------
// 	macros
// -----------------------------------------------------------------------------
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


//	CProjectView class ---------------------------------------------------------
IMPLEMENT_DYNCREATE(CProjectView, CTreeView)

BEGIN_MESSAGE_MAP(CProjectView, CTreeView)
	ON_WM_CREATE()
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, &CProjectView::OnTvnSelChanged)
END_MESSAGE_MAP()


// -----------------------------------------------------------------------------
//	CProjectView
// -----------------------------------------------------------------------------
//
CProjectView::CProjectView()
{
	// TODO: add construction code here
}


// -----------------------------------------------------------------------------
//	~CProjectView
// -----------------------------------------------------------------------------
//
CProjectView::~CProjectView()
{
}


// -----------------------------------------------------------------------------
//	PreCreateWindow
// -----------------------------------------------------------------------------
//
BOOL	CProjectView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying the CREATESTRUCT cs

	return CTreeView::PreCreateWindow(cs);
}


// -----------------------------------------------------------------------------
//	OnInitialUpdate
// -----------------------------------------------------------------------------
//
void	CProjectView::OnInitialUpdate()
{
	CTreeView::OnInitialUpdate();

	printf("CProjectView::OnInitialUpdate() is called\n");
}


//	CProjectView diagnostics ---------------------------------------------------
#ifdef _DEBUG
// -----------------------------------------------------------------------------
//	AssertValid
// -----------------------------------------------------------------------------
//
void	CProjectView::AssertValid() const
{
	CTreeView::AssertValid();
}


// -----------------------------------------------------------------------------
//	Dump
// -----------------------------------------------------------------------------
//
void	CProjectView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}


// -----------------------------------------------------------------------------
//	GetDocument
// -----------------------------------------------------------------------------
//
CCalibraDoc	*CProjectView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CCalibraDoc)));
	return (CCalibraDoc*)m_pDocument;
}
#endif //_DEBUG
// CProjectView message handlers



// -----------------------------------------------------------------------------
//	OnCreate
// -----------------------------------------------------------------------------
//
int	CProjectView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CTreeView::OnCreate(lpCreateStruct) == -1)
		return -1;

	mTreeCtrl = &(GetTreeCtrl());
	mTreeCtrl->ModifyStyle(false, TVS_HASLINES + TVS_HASBUTTONS + TVS_SHOWSELALWAYS);

	GetDocument()->SetProjectView(this);

	return 0;
}


// -----------------------------------------------------------------------------
//	OnTvnSelChanged
// -----------------------------------------------------------------------------
//
void	CProjectView::OnTvnSelChanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW treeViewPtr = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	*pResult = 0;
	HTREEITEM	selectedItem = mTreeCtrl->GetSelectedItem();
	int	id = (int )mTreeCtrl->GetItemData(selectedItem);
	CalibraNode	*rootNode = GetDocument()->GetCalibraRootNode();

	GetDocument()->ProjectViewSelChanged(rootNode->FindNodeByID(id));
}


// -----------------------------------------------------------------------------
//	SetCalibraNode
// -----------------------------------------------------------------------------
//
void	CProjectView::SetCalibraNode(CalibraNode *inNode, HTREEITEM inParentItem)
{
	HTREEITEM	item;
	const type_info	&info = typeid(*inNode);

	if (info == typeid(ProjectNode) ||
		info == typeid(SingleCameraCalibrationNode) ||
		info == typeid(StereoCameraCalibrationNode) ||
		info == typeid(MultiCameraCalibrationNode) ||
		info == typeid(ImageFolderNode) ||
		info == typeid(SingleCameraResultNode) ||
		info == typeid(StereoCameraResultNode) ||
		info == typeid(MultiCameraResultNode))
	{
		item = mTreeCtrl->InsertItem(inNode->GetName().c_str(), inParentItem);
		mTreeCtrl->SetItemData(item, inNode->GetID());
	}
	else
	{
		return;
	}

	if (inNode->HasChildNode() == false)
		return;

	std::vector<CalibraNode *>::const_iterator	it;
	for (it = inNode->begin(); it != inNode->end(); ++it)
		SetCalibraNode((*it), item);
}

void CProjectView::OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/)
{
	mTreeCtrl->DeleteAllItems();
	SetCalibraNode(GetDocument()->GetCalibraRootNode());

	printf("CProjectView::OnUpdate() is called\n");
}

// -----------------------------------------------------------------------------
//	SelectNode
// -----------------------------------------------------------------------------
//
void	CProjectView::SelectNode(CalibraNode *inNode)
{
	HTREEITEM	item = FindTreeItem(mTreeCtrl->GetRootItem(), inNode->GetID());
	if (item != NULL)
	{
		mTreeCtrl->SelectItem(item);
		mTreeCtrl->Expand(item, TVE_EXPAND);
	}
}

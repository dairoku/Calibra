// =============================================================================
//  ProjectView.h
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
	\file		CProjectView.h
	\author		Dairoku Sekiguchi
	\version	1.0
	\date		2007/10/31
	\brief

	Description...
*/
#pragma once

class CCalibraDoc;
class CalibraNode;

// -----------------------------------------------------------------------------
//	CProjectView class
// -----------------------------------------------------------------------------
//
class CProjectView : public CTreeView
{
// Attributes
public:
	virtual ~CProjectView();

	CCalibraDoc* GetDocument();
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

	void	SetCalibraNode(CalibraNode *inNode, HTREEITEM inParentItem = TVI_ROOT);
	void	SelectNode(CalibraNode *inNode);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	HTREEITEM	FindTreeItem(HTREEITEM item, int inID)
	{
		if (item == NULL)
			return NULL;

		if (mTreeCtrl->GetItemData(item) == inID)
			return item;

		HTREEITEM	nextItem = mTreeCtrl->GetNextItem(item, TVGN_NEXT);
		if (nextItem != NULL)
		{
			HTREEITEM	resultItem = FindTreeItem(nextItem, inID);
			if (resultItem != NULL)
				return resultItem;
		}

		nextItem = mTreeCtrl->GetNextItem(item, TVGN_CHILD);
		if (nextItem != NULL)
			return FindTreeItem(nextItem, inID);

		return NULL;
	}

protected:
	CProjectView();
	DECLARE_DYNCREATE(CProjectView)

	virtual void OnInitialUpdate(); // called first time after construct

	CTreeCtrl	*mTreeCtrl;

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnTvnSelChanged(NMHDR *pNMHDR, LRESULT *pResult);
protected:
	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
};

#ifndef _DEBUG  // debug version in ProjectView.cpp
inline CCalibraDoc* CProjectView::GetDocument()
   { return reinterpret_cast<CCalibraDoc*>(m_pDocument); }
#endif


// =============================================================================
//  CalibraDoc.h
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
	\file		CalibraDoc.h
	\author		Dairoku Sekiguchi
	\version	1.0
	\date		2007/10/31
	\brief

	Description...
*/
#pragma once


// -----------------------------------------------------------------------------
// 	include files
// -----------------------------------------------------------------------------
#include "CalibraData.hpp"
#include "CalibraWorkerThread.h"


class	CProjectView;
class	CImageListView;
class	CContainerView;
class	CCornerFinderImageView;

// -----------------------------------------------------------------------------
//	CCalibraDoc class
// -----------------------------------------------------------------------------
//
class CCalibraDoc : public CDocument
{
public:
	enum
	{
		IMAGE_ARROW_MODE = 0,
		IMAGE_SCROLL_MODE,
		IMAGE_GRID_EXTRACTOR_MODE,
		IMAGE_CORNER_FINDER_MODE,
		IMAGE_MANUAL_CORNER_MODE
	};

	virtual ~CCalibraDoc();

	virtual BOOL OnNewDocument();

	void	SetProjectView(CProjectView *inProjectView) { mProjectView = inProjectView; };
	void	SetImageListView(CImageListView *inImageListView) { mImageListView = inImageListView; };
	void	SetContainerView(CContainerView *inContainerView) { mContainerView = inContainerView; };

	CalibraNode	*GetRootNode() { return mRootNode; };
	CalibraNode	*GetSelectedNode() { return mSelectedNode; };
	ImageNode	*GetSelectedImageNode() { return mSelectedImageNode; };

	CProjectView	*GetProjectView() { return mProjectView; };
	CImageListView	*GetImageListView() { return mImageListView; };
	CContainerView	*GetContainerView() { return mContainerView; };

	int		GetImageMode() { return mImageMode; };
	void	SetImageMode(int inMode);

	void	ProjectViewSelChanged(CalibraNode *inNode);
	void	ImageListViewSelChanged(ImageNode *inNode);

	CalibraNode		*GetCalibraRootNode() { return mRootNode; };

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CCalibraDoc();
	DECLARE_DYNCREATE(CCalibraDoc)

private:

	bool	IsImageFolderNodeSelected()
	{
		if (mSelectedNode == NULL)
			return false;
		const type_info	&info = typeid(*mSelectedNode);
		if (info != typeid(ImageFolderNode))
			return false;
		return true;
	}

	bool	IsImageListViewSelected()
	{
		if (mSelectedImageNode == NULL)
			return false;
		return true;
	}

	CalibraNode		*mRootNode;
	CalibraNode		*mSelectedNode;
	ImageNode		*mSelectedImageNode;
	
	//ImageFolderNode	*mSelectedImageFolderNode;
	//ImageNode		*mSelectedImageNode;

	CProjectView	*mProjectView;
	CImageListView	*mImageListView;
	CContainerView	*mContainerView;

	CCornerFinderImageView	*GetCornerFinderImageView();

	int	mImageMode;

	CalibraWorkerThread	mWorkerThread;

public:
	afx_msg void OnUpdateImageModeCommand(CCmdUI *pCmdUI);
	afx_msg void OnUpdateImageZoomCommand(CCmdUI *pCmdUI);
	afx_msg void OnImageModeSelect(UINT nID);
	afx_msg void OnImageZoom(UINT nID);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual void DeleteContents();

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTestAddtestproject();
	afx_msg void OnTestAddsinglecameracalibration();
	afx_msg void OnTestAddstereocameracalibration();
	afx_msg void OnAddImages();
	afx_msg void OnDeleteImage();
	afx_msg void OnTestRunSingleCameraCalibration();
	afx_msg void OnTestReprojection();
	afx_msg void OnTestRunStereoCameraCalibration();
	afx_msg void OnTestRectifyimages();
	afx_msg void OnTestAddmulticameracalibration();
	afx_msg void OnTestRunmulticameracalibration();
	afx_msg void OnTestDumpsinglecameraresults();
	afx_msg void OnTestDumpstereocameraresults();
	afx_msg void OnTestDumpmulticameraresults();
};

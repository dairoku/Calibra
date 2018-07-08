// =============================================================================
//  CalibraDoc.cpp
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
	\file		CalibraDoc.cpp
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
#include <sstream>
#include "Calibra.h"
#include "CalibraDoc.h"
#include "ProjectView.h"
#include "ImageListView.h"
#include "ContainerView.h"
#include "CornerFinderView.h"
#include "StereoCalibration.hpp"
#include "CalibraFile.hpp"
#include "FilePath.hpp"


// -----------------------------------------------------------------------------
// 	macros
// -----------------------------------------------------------------------------
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define	STR_BUF_LEN			256


//	CCalibraDoc class ----------------------------------------------------------
IMPLEMENT_DYNCREATE(CCalibraDoc, CDocument)

BEGIN_MESSAGE_MAP(CCalibraDoc, CDocument)
	ON_UPDATE_COMMAND_UI_RANGE(ID_IMAGE_ARROW_MODE, ID_MANUAL_CORNER_MODE, &CCalibraDoc::OnUpdateImageModeCommand)
	ON_UPDATE_COMMAND_UI_RANGE(ID_IMAGE_ZOOM_IN, ID_IMAGE_ZOOM_OUT, &CCalibraDoc::OnUpdateImageZoomCommand)
	ON_COMMAND_RANGE(ID_IMAGE_ARROW_MODE, ID_MANUAL_CORNER_MODE, &CCalibraDoc::OnImageModeSelect)
	ON_COMMAND_RANGE(ID_IMAGE_ZOOM_IN, ID_IMAGE_ZOOM_OUT, &CCalibraDoc::OnImageZoom)
	ON_COMMAND(ID_TEST_ADDTESTPROJECT, &CCalibraDoc::OnTestAddtestproject)
	ON_COMMAND(ID_TEST_ADDSINGLECAMERACALIBRATION, &CCalibraDoc::OnTestAddsinglecameracalibration)
	ON_COMMAND(ID_IMAGES_ADDIMAGES, &CCalibraDoc::OnAddImages)
	ON_COMMAND(ID_IMAGES_DELETEIMAGE, &CCalibraDoc::OnDeleteImage)
	ON_COMMAND(ID_TEST_RUNSINGLECAMERACALIBRATION, &CCalibraDoc::OnTestRunSingleCameraCalibration)
	ON_COMMAND(ID_TEST_REPROJECTION, &CCalibraDoc::OnTestReprojection)
	ON_COMMAND(ID_TEST_ADDSTEREOCAMERACALIBRATION, &CCalibraDoc::OnTestAddstereocameracalibration)
	ON_COMMAND(ID_TEST_RUNSTEREOCAMERACALIBRATION, &CCalibraDoc::OnTestRunStereoCameraCalibration)
	ON_COMMAND(ID_TEST_RECTIFYIMAGES, &CCalibraDoc::OnTestRectifyimages)
	ON_COMMAND(ID_TEST_ADDMULTICAMERACALIBRATION, &CCalibraDoc::OnTestAddmulticameracalibration)
	ON_COMMAND(ID_TEST_RUNMULTICAMERACALIBRATION, &CCalibraDoc::OnTestRunmulticameracalibration)
	ON_COMMAND(ID_TEST_DUMPSINGLECAMERARESULTS, &CCalibraDoc::OnTestDumpsinglecameraresults)
	ON_COMMAND(ID_TEST_DUMPSTEREOCAMERARESULTS, &CCalibraDoc::OnTestDumpstereocameraresults)
	ON_COMMAND(ID_TEST_DUMPMULTICAMERARESULTS, &CCalibraDoc::OnTestDumpmulticameraresults)
END_MESSAGE_MAP()



// -----------------------------------------------------------------------------
//	CCalibraDoc
// -----------------------------------------------------------------------------
//
CCalibraDoc::CCalibraDoc()
{
	mRootNode = NULL;
	mSelectedNode = NULL;
	mSelectedImageNode = NULL;

	mProjectView = NULL;
	mImageListView = NULL;
	mContainerView = NULL;

	mImageMode = IMAGE_ARROW_MODE;
}


// -----------------------------------------------------------------------------
//	~CCalibraDoc
// -----------------------------------------------------------------------------
//
CCalibraDoc::~CCalibraDoc()
{
}


// -----------------------------------------------------------------------------
//	OnNewDocument
// -----------------------------------------------------------------------------
//
BOOL CCalibraDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	//	Caution!!: SDI documents will reuse this document)
	mRootNode = new ProjectNode(L"New Project");

	return TRUE;
}

void CCalibraDoc::DeleteContents()
{
	printf("DeleteContents\n");

	if (mRootNode == NULL)
		return;

	CalibraNode::DeleteAllNodesRecursively(mRootNode);
	mRootNode = NULL;
}

void CCalibraDoc::SetImageMode(int inMode)
{
	mImageMode = inMode;
	theApp.m_pMainWnd->PostMessage(WM_COMMAND, inMode + ID_IMAGE_ARROW_MODE);
}

// -----------------------------------------------------------------------------
//	ProjectViewSelChanged
// -----------------------------------------------------------------------------
//
void CCalibraDoc::ProjectViewSelChanged(CalibraNode *inNode)
{
	if (mSelectedNode == NULL)
	{
		mSelectedNode = inNode;
		return;
	}

	const type_info	&info = typeid(*inNode);
	if (info == typeid(ImageFolderNode))
	{
		ImageListViewSelChanged(NULL);
		GetImageListView()->SetImageFolderNode(NULL);
		GetImageListView()->SetImageFolderNode((ImageFolderNode *)inNode);
	}
	else
	{
		ImageListViewSelChanged(NULL);
		GetImageListView()->SetImageFolderNode(NULL);
	}

	mSelectedNode = inNode;

	printf("I am here\n");
};


// -----------------------------------------------------------------------------
//	ImageListViewSelChanged
// -----------------------------------------------------------------------------
//
void CCalibraDoc::ImageListViewSelChanged(ImageNode *inNode)
{
	if (inNode == NULL)
	{
		mSelectedImageNode = NULL;
		GetContainerView()->DeleteView();
	}
	else
	{
		mSelectedImageNode = inNode;
		GetContainerView()->CreateChildView(RUNTIME_CLASS(CCornerFinderView));
	}
};


//	CCalibraDoc diagnostics ----------------------------------------------------
#ifdef _DEBUG
// -----------------------------------------------------------------------------
//	AssertValid
// -----------------------------------------------------------------------------
//
void CCalibraDoc::AssertValid() const
{
	CDocument::AssertValid();
}


// -----------------------------------------------------------------------------
//	Dump
// -----------------------------------------------------------------------------
//
void CCalibraDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

// CCalibraDoc commands

void CCalibraDoc::OnUpdateImageModeCommand(CCmdUI *pCmdUI)
{
	if (GetCornerFinderImageView() == NULL)
	{
		pCmdUI->Enable(false);
		return;
	}

	pCmdUI->Enable(true);
	switch (pCmdUI->m_nID)
	{
		case ID_IMAGE_ARROW_MODE:
			if (mImageMode == IMAGE_ARROW_MODE)
				pCmdUI->SetCheck(true);
			else
				pCmdUI->SetCheck(false);
			break;
		case ID_IMAGE_SCROLL_MODE:
			if (mImageMode == IMAGE_SCROLL_MODE)
				pCmdUI->SetCheck(true);
			else
				pCmdUI->SetCheck(false);
			break;
		case ID_GRID_EXTRACTOR_MODE:
			if (mImageMode == IMAGE_GRID_EXTRACTOR_MODE)
				pCmdUI->SetCheck(true);
			else
				pCmdUI->SetCheck(false);
			break;
		case ID_CORNER_FINDER_MODE:
			if (mImageMode == IMAGE_CORNER_FINDER_MODE)
				pCmdUI->SetCheck(true);
			else
				pCmdUI->SetCheck(false);
			break;
		case ID_MANUAL_CORNER_MODE:
			if (mImageMode == IMAGE_MANUAL_CORNER_MODE)
				pCmdUI->SetCheck(true);
			else
				pCmdUI->SetCheck(false);
			break;
	}
}

void CCalibraDoc::OnUpdateImageZoomCommand(CCmdUI *pCmdUI)
{
	if (GetCornerFinderImageView() == NULL)
		pCmdUI->Enable(false);
	else
		pCmdUI->Enable(true);
}


CCornerFinderImageView	*CCalibraDoc::GetCornerFinderImageView()
{
	if (GetContainerView() == NULL)
		return NULL;

	CWnd	*view = GetContainerView()->GetChildView();
	if (view == NULL)
		return NULL;

	const type_info	&info = typeid(*view);
	if (info != typeid(CCornerFinderView))
		return NULL;

	return ((CCornerFinderView *)view)->GetCornerFinderImageView();
}

void CCalibraDoc::OnImageModeSelect(UINT nID)
{
	CCornerFinderImageView	*view = GetCornerFinderImageView();
	InputImageNode	*imageNode = (InputImageNode *)GetSelectedImageNode();

	if (view == NULL)
		return;

	switch (nID)
	{
		case ID_IMAGE_ARROW_MODE:
			mImageMode = IMAGE_ARROW_MODE;
			break;
		case ID_IMAGE_SCROLL_MODE:
			mImageMode = IMAGE_SCROLL_MODE;
			break;
		case ID_GRID_EXTRACTOR_MODE:
			mImageMode = IMAGE_GRID_EXTRACTOR_MODE;
			imageNode->ClearAllExtractedCorner();
			imageNode->SetGridExtractorInputNum(0);
			view->Invalidate(false);
			break;
		case ID_CORNER_FINDER_MODE:
			mImageMode = IMAGE_CORNER_FINDER_MODE;
			break;
		case ID_MANUAL_CORNER_MODE:
			mImageMode = IMAGE_MANUAL_CORNER_MODE;
			break;
	}
}

void CCalibraDoc::OnImageZoom(UINT nID)
{
	CCornerFinderImageView	*view = GetCornerFinderImageView();
	if (view == NULL)
		return;

	if (nID == ID_IMAGE_ZOOM_IN)
		view->ZoomIn();
	else
		view->ZoomOut();
}

BOOL CCalibraDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	try
	{
		mRootNode = CalibraFile::ReadFromFile(lpszPathName);
	}

	catch (std::exception &ex)
	{
		wprintf(L"Caught exception while opening the file:%s\n", lpszPathName);
		printf("%s\n", ex.what());
		printf("Type:%s\n", typeid(ex).name());
		return false;
	}

	return true;
}

BOOL CCalibraDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	try
	{
		CalibraFile::WriteToFile(lpszPathName, mRootNode);
	}

	catch (std::exception &ex)
	{
		wprintf(L"Caught exception while saving the file:%s\n", lpszPathName);
		printf("%s\n", ex.what());
		return false;
	}

	return true;
}

void CCalibraDoc::OnTestAddtestproject()
{
	SingleCameraCalibrationNode	*calibrationNode = (SingleCameraCalibrationNode *)
		mRootNode->AddChildNode(new SingleCameraCalibrationNode(L"Single Camera Calibration"));

	ImageFolderNode	*imageFolderNode = (ImageFolderNode *)
		calibrationNode->AddChildNode(new ImageFolderNode(L"Input Images"));

	std::wstringstream		buf1;
	std::wstringstream		buf2;

	for (int i = 1; i < 21; i++)
	{
		buf1 << "image" << i;
		buf2 << "image" << i << ".bmp";
		imageFolderNode->AddChildNode(new InputImageNode(buf1.str(), buf2.str()));
		buf1.str(L"");
		buf1.clear();
		buf2.str(L"");
		buf2.clear();
	}

	SingleCameraResultNode	*calibrationResultNode = (SingleCameraResultNode *)
		calibrationNode->AddChildNode(
			new SingleCameraResultNode(
					CalibrationResultNode::DEFAULT_IMAGE_WIDTH,		// <- Should fix this
					CalibrationResultNode::DEFAULT_IMAGE_HEIGHT,	// <- Should fix this
					L"Results"));

	UpdateAllViews(NULL);
}

void CCalibraDoc::OnTestAddsinglecameracalibration()
{
	SingleCameraCalibrationNode	*calibrationNode = (SingleCameraCalibrationNode *)
		mRootNode->AddChildNode(new SingleCameraCalibrationNode(L"Single Camera Calibration"));

	ImageFolderNode	*imageFolderNode = (ImageFolderNode *)
		calibrationNode->AddChildNode(new ImageFolderNode(L"Input Images"));

	UpdateAllViews(NULL);
	GetProjectView()->SelectNode(imageFolderNode);
}

void CCalibraDoc::OnTestAddstereocameracalibration()
{
	StereoCameraCalibrationNode	*calibrationNode = (StereoCameraCalibrationNode *)
		mRootNode->AddChildNode(new StereoCameraCalibrationNode(L"Stereo Camera Calibration"));

	SingleCameraCalibrationNode	*leftCameraNode = (SingleCameraCalibrationNode *)
		calibrationNode->AddChildNode(new SingleCameraCalibrationNode(L"Left Camera"));

	SingleCameraCalibrationNode	*rightCameraNode = (SingleCameraCalibrationNode *)
		calibrationNode->AddChildNode(new SingleCameraCalibrationNode(L"Right Camera"));

	ImageFolderNode	*imageFolderNode = (ImageFolderNode *)
		leftCameraNode->AddChildNode(new ImageFolderNode(L"Input Images"));
		
	rightCameraNode->AddChildNode(new ImageFolderNode(L"Input Images"));

	UpdateAllViews(NULL);
	GetProjectView()->SelectNode(imageFolderNode);
}

void CCalibraDoc::OnAddImages()
{
	if (IsImageFolderNodeSelected() == false)
	{
		printf("Internal Error: ImageFolder is not selected\n");
		return;
	}

	CFileDialog	fileDialog(true, TEXT("bmp"), NULL, OFN_ALLOWMULTISELECT,
		TEXT("Bitmap Files (*.bmp)|*.bmp|PPM Files (*.ppm)|*.ppm|All Files(*.*)|*.*||"));
	CString	fileNameList;

	fileDialog.GetOFN().lpstrFile = fileNameList.GetBuffer(1024 * 256);
	fileDialog.GetOFN().nMaxFile = 1024;

	INT_PTR	result = fileDialog.DoModal();
	if (result == IDCANCEL)
	{
		fileNameList.ReleaseBuffer();
		return;
	}

	ImageFolderNode	*imageFolderNode = (ImageFolderNode *)GetSelectedNode();

	POSITION	position = fileDialog.GetStartPosition();
	while (position != NULL)
	{
		CString	filePath = fileDialog.GetNextPathName(position);
		std::wstring	fileName = FilePath::ExtractFileName(filePath.GetBuffer());

		InputImageNode	*imageNode = new InputImageNode(fileName.c_str(), L"");
		imageNode->SetCachedFilePath(filePath.GetBuffer());
		imageFolderNode->AddChildNode(imageNode);

		/*wprintf(L"%s\n", filePath.GetBuffer());
		wprintf(L"%s\n", fileName.c_str());

		std::wstring	relativePath = FilePath::BuildRelativeFilePath(
			L"c:\\temp\\hoge1\\hoge2\\hoge3\\", filePath.GetBuffer());
		wprintf(L"%s\n", relativePath.c_str());*/
	}

	GetImageListView()->SetImageFolderNode(imageFolderNode);	// Update List
	//UpdateAllViews(NULL); <- This will be better. But have implement OnUpate Handler

	fileNameList.ReleaseBuffer();
}

void CCalibraDoc::OnDeleteImage()
{
	if (IsImageFolderNodeSelected() == false)
	{
		printf("Internal Error: ImageFolder is not selected\n");
		return;
	}

	if (IsImageListViewSelected() == false)
	{
		printf("Internal Error: ImageListView is not selected\n");
		return;
	}

	ImageNode	*imageNode = GetSelectedImageNode();
	ImageNode	*nextImageNode = (ImageNode *)imageNode->GetElderBrotherNode();
	CalibraNode	*parentNode = imageNode->GetParentNode();

	ImageListViewSelChanged(NULL);
	parentNode->RemoveChildNode(imageNode);

	delete imageNode;

	ProjectViewSelChanged(GetSelectedNode());
	if (nextImageNode != NULL)
		GetImageListView()->SelectImageNode(nextImageNode);
}

void CCalibraDoc::OnTestRunSingleCameraCalibration()
{
	if (IsImageFolderNodeSelected() == false)
	{
		printf("Internal Error: ImageFolder is not selected\n");
		return;
	}

	ImageFolderNode	*imageFolderNode = (ImageFolderNode *)GetSelectedNode();
	SingleCameraResultNode	*resultNode;

	if (imageFolderNode->GetParentNode()->GetChildNodeNum() == 1)
	{
		resultNode = new SingleCameraResultNode(
								CalibrationResultNode::DEFAULT_IMAGE_WIDTH,		// <- Should fix this
								CalibrationResultNode::DEFAULT_IMAGE_HEIGHT,	// <- Should fix this
								L"Results");
		imageFolderNode->GetParentNode()->AddChildNode(resultNode);
	}
	else
	{
		resultNode = (SingleCameraResultNode *)imageFolderNode->GetParentNode()->GetChildNode(1);
	}

	resultNode->mCameraCalibration.ClearMesurementData();

	std::vector<CalibraNode *>::const_iterator	it;
	for (it = imageFolderNode->begin(); it != imageFolderNode->end(); ++it)
	{
		InputImageNode	*node = (InputImageNode *)*it;

		resultNode->mCameraCalibration.AddMesurementData(
			node->GetExtractedCornerMatrix(),
			node->GetExtractedCornerWorldCoordinateMatrix());	
	}

	mWorkerThread.WaitForWorkerThread();
	mWorkerThread.SetCameraCalibrationObject(&(resultNode->mCameraCalibration));

	printf("Start calibration...\n");
	mWorkerThread.StartWorkerThread();
}

void CCalibraDoc::OnTestDumpsinglecameraresults()
{
	if (IsImageFolderNodeSelected() == false)
	{
		printf("Internal Error: ImageFolder is not selected\n");
		return;
	}

	if (GetSelectedNode()->GetParentNode()->GetChildNodeNum() == 1)
		return;

	ImageFolderNode	*imageFolderNode = (ImageFolderNode *)GetSelectedNode();
	SingleCameraResultNode	*resultNode;
	resultNode = (SingleCameraResultNode *)imageFolderNode->GetParentNode()->GetChildNode(1);

	resultNode->mCameraCalibration.DumpResults();
}

void CCalibraDoc::OnTestReprojection()
{
	if (IsImageFolderNodeSelected() == false)
	{
		printf("Internal Error: ImageFolder is not selected\n");
		return;
	}

	ImageFolderNode	*imageFolderNode = (ImageFolderNode *)GetSelectedNode();
	SingleCameraResultNode	*resultNode
		= (SingleCameraResultNode *)imageFolderNode->GetParentNode()->GetChildNode(1);

	for (int i = 0; i < imageFolderNode->GetChildNodeNum(); i++)
	{
		InputImageNode	*node = (InputImageNode *)(imageFolderNode->GetChildNode(i));
		ImageData	image;
		image.OpenBitmapFile(node->GetCachedFilePath().c_str());

		for (int j = 0; j < node->GetExtractedCornerNum(); j++)
		{
			node->SetCornerFinderCenter(
				j,
				resultNode->mCameraCalibration.y_list[i](0, j),
				resultNode->mCameraCalibration.y_list[i](1, j),
				InputImageNode::REPROJECTION_METHOD);
			node->ExecCornerFinder(image, j);
		}
	}
}

void CCalibraDoc::OnTestRunStereoCameraCalibration()
{
	if (mSelectedNode == NULL)
		return;

	const type_info	&info = typeid(*mSelectedNode);
	if (info != typeid(StereoCameraCalibrationNode))
		return;

	StereoCameraCalibrationNode	*calibrationNode = (StereoCameraCalibrationNode *)mSelectedNode;
	SingleCameraResultNode	*leftResult
		= (SingleCameraResultNode *)calibrationNode->GetChildNode(0)->GetChildNode(1);
	SingleCameraResultNode	*rightResult
		= (SingleCameraResultNode *)calibrationNode->GetChildNode(1)->GetChildNode(1);

	StereoCameraResultNode	*node;

	if (calibrationNode->GetChildNodeNum() < 3)
	{
		node = new StereoCameraResultNode(
								CalibrationResultNode::DEFAULT_IMAGE_WIDTH,		// <- Should fix this
								CalibrationResultNode::DEFAULT_IMAGE_HEIGHT,	// <- Should fix this
								L"Results");
		calibrationNode->AddChildNode(node);
	}
	else
	{
		node = (StereoCameraResultNode *)calibrationNode->GetChildNode(2);
	}

	node->mStereoCalibration.x_left_list = leftResult->mCameraCalibration.x_list;
	node->mStereoCalibration.X_left_list = leftResult->mCameraCalibration.X_list;
	node->mStereoCalibration.omc_left_list = leftResult->mCameraCalibration.omc_list;
	node->mStereoCalibration.Tc_left_list = leftResult->mCameraCalibration.Tc_list;
	//node->mStereoCalibration.Rc_left_list = leftResult->mCameraCalibration.Rc_list;
	node->mStereoCalibration.fc_left = leftResult->mCameraCalibration.fc;
	node->mStereoCalibration.cc_left = leftResult->mCameraCalibration.cc;
	node->mStereoCalibration.kc_left = leftResult->mCameraCalibration.kc;
	node->mStereoCalibration.alpha_c_left = leftResult->mCameraCalibration.alpha_c;

	node->mStereoCalibration.x_right_list = rightResult->mCameraCalibration.x_list;
	node->mStereoCalibration.X_right_list = rightResult->mCameraCalibration.X_list;
	node->mStereoCalibration.omc_right_list = rightResult->mCameraCalibration.omc_list;
	node->mStereoCalibration.Tc_right_list = rightResult->mCameraCalibration.Tc_list;
	//node->mStereoCalibration.Rc_right_list = rightResult->mCameraCalibration.Rc_list;
	node->mStereoCalibration.fc_right = rightResult->mCameraCalibration.fc;
	node->mStereoCalibration.cc_right = rightResult->mCameraCalibration.cc;
	node->mStereoCalibration.kc_right = rightResult->mCameraCalibration.kc;
	node->mStereoCalibration.alpha_c_right = rightResult->mCameraCalibration.alpha_c;

	node->mStereoCalibration.DoCalibration();
	node->mStereoCalibration.CalcRectifyIndex();
}

void CCalibraDoc::OnTestDumpstereocameraresults()
{
	if (mSelectedNode == NULL)
		return;

	const type_info	&info = typeid(*mSelectedNode);
	if (info != typeid(StereoCameraCalibrationNode))
		return;

	if (mSelectedNode->GetChildNodeNum() < 3)
		return;

	StereoCameraResultNode	*node = (StereoCameraResultNode *)mSelectedNode->GetChildNode(2);

	node->mStereoCalibration.DumpResults();
}

void CCalibraDoc::OnTestRectifyimages()
{
	if (mSelectedNode == NULL)
		return;

	const type_info	&info = typeid(*mSelectedNode);
	if (info != typeid(StereoCameraCalibrationNode))
		return;

	if (mSelectedNode->GetChildNodeNum() < 3)
		return;

	StereoCameraResultNode	*node = (StereoCameraResultNode *)mSelectedNode->GetChildNode(2);

	CFileDialog	fileDialog(true, TEXT("bmp"), NULL, 0,
		TEXT("Bitmap Files (*.bmp)|*.bmp|PPM Files (*.ppm)|*.ppm|All Files(*.*)|*.*||"));

	INT_PTR	result = fileDialog.DoModal();
	if (result == IDCANCEL)
		return;

	CString	leftImageFilePathName = fileDialog.GetPathName();
	std::wstring	leftOutputFilePathName = FilePath::ExtractPath(leftImageFilePathName)
						+ L"left_rectified.bmp";

	result = fileDialog.DoModal();
	if (result == IDCANCEL)
		return;

	CString	rightImageFilePathName = fileDialog.GetPathName();
	std::wstring	rightOutputFilePath = FilePath::ExtractPath(rightImageFilePathName)
						+ L"right_rectified.bmp";

	ImageData	inputImage, outputImage;

	inputImage.OpenBitmapFile(leftImageFilePathName);
	outputImage.OpenBitmapFile(leftImageFilePathName);

	node->mStereoCalibration.rectify_image(
		inputImage.GetImageWidth(), inputImage.GetImageHeight(),
		inputImage.GetImageBufferPtr(),
		node->mStereoCalibration.a1_left, node->mStereoCalibration.a2_left,
		node->mStereoCalibration.a3_left, node->mStereoCalibration.a4_left,
		node->mStereoCalibration.ind_new_left,
		node->mStereoCalibration.ind_1_left, node->mStereoCalibration.ind_2_left,
		node->mStereoCalibration.ind_3_left, node->mStereoCalibration.ind_4_left,
		outputImage.GetImageBufferPtr());
	outputImage.SaveBitmapFile(leftOutputFilePathName.c_str());

	inputImage.OpenBitmapFile(rightImageFilePathName);

	node->mStereoCalibration.rectify_image(
		inputImage.GetImageWidth(), inputImage.GetImageHeight(),
		inputImage.GetImageBufferPtr(),
		node->mStereoCalibration.a1_right, node->mStereoCalibration.a2_right,
		node->mStereoCalibration.a3_right, node->mStereoCalibration.a4_right,
		node->mStereoCalibration.ind_new_right,
		node->mStereoCalibration.ind_1_right, node->mStereoCalibration.ind_2_right,
		node->mStereoCalibration.ind_3_right, node->mStereoCalibration.ind_4_right,
		outputImage.GetImageBufferPtr());
	outputImage.SaveBitmapFile(rightOutputFilePath.c_str());
}

#define	MULTI_CAMERA_NUM	25
#define	CENTER_CAMERA_INDEX	12

void CCalibraDoc::OnTestAddmulticameracalibration()
{
	MultiCameraCalibrationNode	*calibrationNode = (MultiCameraCalibrationNode *)
		mRootNode->AddChildNode(new MultiCameraCalibrationNode(L"ProFUSION 25 Calibration"));

	for (int i = 0; i < MULTI_CAMERA_NUM; i++)
	{
		std::wstringstream		buf;

		buf << "Camera " << i;
		SingleCameraCalibrationNode	*cameraNode = (SingleCameraCalibrationNode *)
			calibrationNode->AddChildNode(new SingleCameraCalibrationNode(buf.str()));

		ImageFolderNode	*imageFolderNode = (ImageFolderNode *)
			cameraNode->AddChildNode(new ImageFolderNode(L"Input Images"));
	}

	UpdateAllViews(NULL);
	//GetProjectView()->SelectNode(imageFolderNode);
}

void CCalibraDoc::OnTestRunmulticameracalibration()
{
	if (mSelectedNode == NULL)
		return;

	const type_info	&info = typeid(*mSelectedNode);
	if (info != typeid(MultiCameraCalibrationNode))
		return;

	MultiCameraCalibrationNode	*calibrationNode = (MultiCameraCalibrationNode *)mSelectedNode;

	MultiCameraResultNode	*node;

	if (calibrationNode->GetChildNodeNum() <= MULTI_CAMERA_NUM)
	{
		node = new MultiCameraResultNode(
								CalibrationResultNode::DEFAULT_IMAGE_WIDTH,		// <- Should fix this
								CalibrationResultNode::DEFAULT_IMAGE_HEIGHT,	// <- Should fix this
								L"Results");
		calibrationNode->AddChildNode(node);
	}
	else
	{
		node = (MultiCameraResultNode *)calibrationNode->GetChildNode(MULTI_CAMERA_NUM);
	}

	node->mMultiCameraCalibration.mCenterCameraIndex = CENTER_CAMERA_INDEX;
	node->mMultiCameraCalibration.mCalibrationResults.resize(MULTI_CAMERA_NUM);

	for (int i = 0; i < MULTI_CAMERA_NUM; i++)
	{
		SingleCameraResultNode	*singleResult
			= (SingleCameraResultNode *)calibrationNode->GetChildNode(i)->GetChildNode(1);

		node->mMultiCameraCalibration.mCalibrationResults[i].x_list = singleResult->mCameraCalibration.x_list;
		node->mMultiCameraCalibration.mCalibrationResults[i].X_list = singleResult->mCameraCalibration.X_list;
		node->mMultiCameraCalibration.mCalibrationResults[i].omc_list = singleResult->mCameraCalibration.omc_list;
		node->mMultiCameraCalibration.mCalibrationResults[i].Tc_list = singleResult->mCameraCalibration.Tc_list;
		node->mMultiCameraCalibration.mCalibrationResults[i].fc = singleResult->mCameraCalibration.fc;
		node->mMultiCameraCalibration.mCalibrationResults[i].cc = singleResult->mCameraCalibration.cc;
		node->mMultiCameraCalibration.mCalibrationResults[i].kc = singleResult->mCameraCalibration.kc;
		node->mMultiCameraCalibration.mCalibrationResults[i].alpha_c = singleResult->mCameraCalibration.alpha_c;
	}

	node->mMultiCameraCalibration.DoCalibration();
}

void CCalibraDoc::OnTestDumpmulticameraresults()
{
	if (mSelectedNode == NULL)
		return;

	const type_info	&info = typeid(*mSelectedNode);
	if (info != typeid(MultiCameraCalibrationNode))
		return;

	if (mSelectedNode->GetChildNodeNum() <= MULTI_CAMERA_NUM)
		return;

	MultiCameraResultNode	*node = (MultiCameraResultNode *)mSelectedNode->GetChildNode(MULTI_CAMERA_NUM);

	node->mMultiCameraCalibration.DumpResults();
}

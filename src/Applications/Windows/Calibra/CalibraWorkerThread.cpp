// =============================================================================
//  CalibraWorkerThread.cpp
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
	\file		CalibraWorkerThread.cpp
	\author		Dairoku Sekiguchi
	\version	1.0
	\date		2008/03/31
	\brief		This file is a part of Calibra

	This file is a part of Calibra
*/


// -----------------------------------------------------------------------------
// 	include files
// -----------------------------------------------------------------------------
#include "stdafx.h"
#include "CalibraWorkerThread.h"


//  CalibraWorkerThread class public member functions ===========================
// -----------------------------------------------------------------------------
//	CalibraWorkerThread
// -----------------------------------------------------------------------------
//
CalibraWorkerThread::CalibraWorkerThread()
{
	mCameraCalibration = NULL;
	mThread = NULL;
}


// -----------------------------------------------------------------------------
//	~CalibraWorkerThread
// -----------------------------------------------------------------------------
//
CalibraWorkerThread::~CalibraWorkerThread()
{
}


// -----------------------------------------------------------------------------
//	StartWorkerThread
// -----------------------------------------------------------------------------
//
void	CalibraWorkerThread::StartWorkerThread()
{
	ASSERT(mThread == NULL);

	mLoop = true;
	mThread = ::AfxBeginThread(WorkerFunc, (LPVOID)this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED, NULL);
	mThread->m_bAutoDelete = FALSE;
	mThread->ResumeThread();
}


// -----------------------------------------------------------------------------
//	WaitForWorkerThread
// -----------------------------------------------------------------------------
//
bool	CalibraWorkerThread::WaitForWorkerThread(DWORD inTimeout)
{
	if (mThread == NULL)
		return true;

	if (::WaitForSingleObject(mThread->m_hThread, inTimeout) == WAIT_OBJECT_0)
	{
		delete mThread;
		mThread = NULL;
		return true;
	}

	return false;
}


// -----------------------------------------------------------------------------
//	WorkerFunc
// -----------------------------------------------------------------------------
//
UINT	CalibraWorkerThread::WorkerFunc(LPVOID pParam)
{
	CalibraWorkerThread	*worker	= (CalibraWorkerThread *)pParam;

	if (worker->mCameraCalibration == NULL)
	{
		printf("ASSERT: worker->mCameraCalibration == NULL in CalibraWorkerThread::StartWorkerThread\n");
		return -1;
	}

	worker->mCameraCalibration->DoCalibration();

	return 0;
}

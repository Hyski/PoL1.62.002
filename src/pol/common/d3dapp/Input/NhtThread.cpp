#include "Precomp.h"
#include <process.h>
#include "NhtThread.h"

unsigned int __stdcall NhtThread(PVOID pvoid);

//=====================================================================================//
//                              CNhtThread::CNhtThread()                               //
//=====================================================================================//
CNhtThread::CNhtThread()
{
	m_hThread = 0;
	m_bThread = FALSE;
}

//=====================================================================================//
//                              CNhtThread::~CNhtThread()                              //
//=====================================================================================//
CNhtThread::~CNhtThread()
{
	Stop();
}

//=====================================================================================//
//                               BOOL CNhtThread::Run()                                //
//=====================================================================================//
BOOL CNhtThread::Run(void)
{
	unsigned int id;

	if(!m_hThread)
	{
		m_bThread = TRUE;
		if(m_hThread = (HANDLE)_beginthreadex(NULL,0,NhtThread,this,0,&id))
		{
			SetThreadPriority(m_hThread,THREAD_PRIORITY_HIGHEST);
			return TRUE;
		}
	}

	return FALSE;
}

//=====================================================================================//
//                               void CNhtThread::Stop()                               //
//=====================================================================================//
void CNhtThread::Stop(void)
{
	if(m_hThread)
	{
		m_bThread = FALSE;
		WaitForSingleObject(m_hThread,INFINITE);
		m_hThread = NULL;
	}
}

//=====================================================================================//
//                           unsigned int CNhtThread::Main()                           //
//=====================================================================================//
unsigned int  CNhtThread::Main(void)
{
	return 0;
}

//=====================================================================================//
//                         unsigned int __stdcall NhtThread()                          //
//=====================================================================================//
unsigned int __stdcall NhtThread(PVOID pvoid)
{
	return ((CNhtThread*)pvoid)->Main();
}
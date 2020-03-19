#include "precomp.h"
#include "WorkingThread.h"

#include <process.h>

namespace PoL
{

//=====================================================================================//
//                           WorkingThread::WorkingThread()                            //
//=====================================================================================//
WorkingThread::WorkingThread()
:	m_thread(0),
	m_stop(false),
	m_threadId(0),
	m_updateObjects(1)
{
	InitializeCriticalSection(&m_changeSection);

	m_stopEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	m_resumeEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	m_syncStopEvent = CreateEvent(NULL,FALSE,FALSE,NULL);

	m_events.push_back(m_stopEvent);
	m_eventInfos.push_back(this);

	startThread();
}

//=====================================================================================//
//                           WorkingThread::~WorkingThread()                           //
//=====================================================================================//
WorkingThread::~WorkingThread()
{
	stopThread();
	CloseHandle(m_stopEvent);
	CloseHandle(m_resumeEvent);
	CloseHandle(m_syncStopEvent);
	DeleteCriticalSection(&m_changeSection);
}

//=====================================================================================//
//                           void WorkingThread::addWorker()                           //
//=====================================================================================//
void WorkingThread::addWorker(HANDLE evt, Worker *worker)
{
	EnterCriticalSection(&m_changeSection);

	suspendThread();

	Events_t::iterator i = std::find(m_events.begin(),m_events.end(),evt);
	if(i == m_events.end())
	{
		m_events.push_back(evt);
		m_eventInfos.push_back(worker);
		++m_updateObjects;
	}

	resumeThread();

	LeaveCriticalSection(&m_changeSection);
}

//=====================================================================================//
//                         void WorkingThread::removeWorker()                          //
//=====================================================================================//
void WorkingThread::removeWorker(HANDLE evt)
{
	EnterCriticalSection(&m_changeSection);

	suspendThread();

	Events_t::iterator i = std::find(m_events.begin(),m_events.end(),evt);
	if(i != m_events.end())
	{
		m_events.erase(i);
		m_eventInfos.erase(m_eventInfos.begin() + (i-m_events.begin()));
		++m_updateObjects;
	}

	resumeThread();

	LeaveCriticalSection(&m_changeSection);
}

//=====================================================================================//
//                         void WorkingThread::suspendThread()                         //
//=====================================================================================//
void WorkingThread::suspendThread()
{
	SetEvent(m_stopEvent);
	WaitForSingleObject(m_syncStopEvent,INFINITE);
}

//=====================================================================================//
//                         void WorkingThread::resumeThread()                          //
//=====================================================================================//
void WorkingThread::resumeThread()
{
	SetEvent(m_resumeEvent);
}

//=====================================================================================//
//                          void WorkingThread::startThread()                          //
//=====================================================================================//
void WorkingThread::startThread()
{
	assert( m_thread == 0 );

	m_thread = _beginthreadex(0,0,threadFuncProxy,this,0,reinterpret_cast<unsigned int *>(&m_threadId));
	SetThreadPriority((HANDLE)m_thread,THREAD_PRIORITY_HIGHEST);
}

//=====================================================================================//
//                          void WorkingThread::stopThread()                           //
//=====================================================================================//
void WorkingThread::stopThread()
{
	m_stop = true;

    suspendThread();
	resumeThread();
	WaitForSingleObject((HANDLE)m_thread,INFINITE);
	m_thread = 0;
	m_threadId = 0;
}

//=====================================================================================//
//                    unsigned int WorkingThread::threadFuncProxy()                    //
//=====================================================================================//
unsigned int WorkingThread::threadFuncProxy(PVOID ptr)
{
	WorkingThread *This = reinterpret_cast<WorkingThread *>(ptr);
	This->threadFunc();
	return 0;
}

//=====================================================================================//
//                          void WorkingThread::threadFunc()                           //
//=====================================================================================//
void WorkingThread::threadFunc()
{
	HANDLE handles[MAXIMUM_WAIT_OBJECTS];
	Worker *workers[MAXIMUM_WAIT_OBJECTS];
	DWORD count;

	while(!m_stop)
	{
		if(m_updateObjects)
		{
			m_updateObjects = 0;
			count = m_events.size();
			std::copy(m_events.begin(),m_events.end(),handles);
			std::copy(m_eventInfos.begin(),m_eventInfos.end(),workers);
		}

		DWORD retval = WaitForMultipleObjects(count,handles,FALSE,INFINITE);

		if(retval >= WAIT_OBJECT_0 && retval < WAIT_OBJECT_0 + count)
		{
			const int index = retval - WAIT_OBJECT_0;
			workers[index]->work();
		}
	}
}

//=====================================================================================//
//                             void WorkingThread::work()                              //
//=====================================================================================//
void WorkingThread::work()
{
	SetEvent(m_syncStopEvent);
	WaitForSingleObject(m_resumeEvent,INFINITE);
}

}
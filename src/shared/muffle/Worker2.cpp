#include "precomp.h"
#include "Worker2.h"
#include <process.h>
#include <boost/utility.hpp>

namespace Muffle2
{

static Worker *g_inst = 0;

//=====================================================================================//
//                                  Worker::Worker()                                   //
//=====================================================================================//
Worker::Worker()
:	m_timer(NULL),
	m_thread(NULL)
{
	assert( g_inst == 0 );
	g_inst = this;

	timeBeginPeriod(1);

	m_timer = CreateWaitableTimer(NULL,FALSE,NULL);
	m_exitThread = CreateEvent(NULL,FALSE,FALSE,NULL);
	startThread();
	setTimer();
}

//=====================================================================================//
//                                  Worker::~Worker()                                  //
//=====================================================================================//
Worker::~Worker()
{
	MUFFLE_SYNCHRONIZED_THIS;

	finish();
	timeEndPeriod(1);

	assert( g_inst == this );
	g_inst = 0;
}

//=====================================================================================//
//                                void Worker::finish()                                //
//=====================================================================================//
void Worker::finish()
{
	MUFFLE_SYNCHRONIZED_THIS;

	if(m_thread)
	{
		SetEvent(m_exitThread);
		WaitForSingleObject(m_thread,INFINITE);

		CloseHandle(m_timer);
		CloseHandle(m_exitThread);

		m_thread = 0;
		m_exitThread = 0;
	}
}

//=====================================================================================//
//                               void Worker::addWork()                                //
//=====================================================================================//
void Worker::addWork(HWork work, unsigned int period, bool alone)
{
	MUFFLE_SYNCHRONIZED_THIS;

	WorkItem item = {0,period,work};
	if(alone) item.holder = work;
	m_workItems.push_front(item);
}

//=====================================================================================//
//                               void Worker::setTimer()                               //
//=====================================================================================//
void Worker::setTimer()
{
	LARGE_INTEGER period;
	period.QuadPart = 0;

	// Будем "тикать" каждые 20 миллисекунд
	SetWaitableTimer(m_timer,&period,20,NULL,NULL,FALSE);
}

//=====================================================================================//
//                                void Worker::worker()                                //
//=====================================================================================//
void Worker::worker()
{
	HANDLE handles[2] = {m_exitThread,m_timer};
	bool running = true;

	while(running)
	{
		DWORD wait_result = WaitForMultipleObjects(2,handles,FALSE,INFINITE);
		
		switch(wait_result)
		{
		case WAIT_OBJECT_0:		// Выход из потока
			running = false;
			break;
		case WAIT_OBJECT_0+1:	// Таймер
			executeWork();
			break;
		}
	}
}

//=====================================================================================//
//                         unsigned int Worker::threadProxy()                          //
//=====================================================================================//
unsigned int Worker::threadProxy(void *worker)
{
	Worker *This = reinterpret_cast<Worker *>(worker);
	This->worker();
	return 0;
}

//=====================================================================================//
//                             void Worker::executeWork()                              //
//=====================================================================================//
void Worker::executeWork()
{
	typedef std::list< HWork, SafeAlloc<HWork> > WorkToDo_t;
	WorkToDo_t workToDo;
	WorkItems_t workItCopy;

	{
		MUFFLE_SYNCHRONIZED_THIS;
		m_workItems.swap(workItCopy);
	}

	unsigned long curTime = timeGetTime();

	for(WorkItems_t::iterator i = workItCopy.begin(); i != workItCopy.end();)
	{
		bool remove = true;

		if(HWork lockedWork = i->work.acquire())
		{
			if(!lockedWork->expired())
			{
				if(i->lastRun + i->period <= curTime)
				{
					i->lastRun = curTime;
					workToDo.push_back(lockedWork);
				}

				remove = false;
			}
		}

		if(remove)
		{
			i = workItCopy.erase(i);
		}
		else
		{
			++i;
		}
	}

	{
		MUFFLE_SYNCHRONIZED_THIS;
		m_workItems.splice(m_workItems.end(),workItCopy);
	}

	for(WorkToDo_t::iterator i = workToDo.begin(); i != workToDo.end(); ++i)
	{
		(*i)->execute();
	}
}

//=====================================================================================//
//                             void Worker::startThread()                              //
//=====================================================================================//
void Worker::startThread()
{
	unsigned int tid = 0;
	m_thread = (HANDLE)_beginthreadex(NULL, 0, threadProxy, this, CREATE_SUSPENDED, &tid);
	SetThreadPriority(m_thread,THREAD_PRIORITY_ABOVE_NORMAL);
	ResumeThread(m_thread);
}

//=====================================================================================//
//                               Worker *Worker::inst()                                //
//=====================================================================================//
Worker *Worker::inst()
{
	return g_inst;
}

}

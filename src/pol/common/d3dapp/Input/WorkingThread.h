#pragma once

#include "Worker.h"

namespace PoL
{

//=====================================================================================//
//                                 class WorkingThread                                 //
//=====================================================================================//
class WorkingThread : Worker
{
	CRITICAL_SECTION m_changeSection;


	typedef std::vector<HANDLE> Events_t;
	typedef std::vector<Worker *> EventInfos_t;

	Events_t m_events;
	EventInfos_t m_eventInfos;

	HANDLE m_stopEvent;
	HANDLE m_resumeEvent;
	HANDLE m_syncStopEvent;

	int m_updateObjects;
	bool m_stop;
	uintptr_t m_thread;
	HANDLE m_threadId;

public:
	WorkingThread();
	~WorkingThread();

	/// Добавить "рабочую" функцию
	void addWorker(HANDLE, Worker *);
	/// Убрать "рабочую" функцию
	void removeWorker(HANDLE);

private:
	void work();

	void suspendThread();
	void resumeThread();
	void startThread();
	void stopThread();

	void threadFunc();

	static unsigned int __stdcall threadFuncProxy(PVOID);
};

}

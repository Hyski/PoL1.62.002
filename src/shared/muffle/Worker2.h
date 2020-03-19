#pragma once

#include "Work2.h"
#include "SafeAlloc2.h"
#include "Synchronized2.h"

namespace Muffle2
{

//=====================================================================================//
//                                    class Worker                                     //
//=====================================================================================//
class Worker : public Synchronized
{
	struct WorkItem
	{
		unsigned long lastRun;		///< ����� ���������� �������
		unsigned long period;		///< ������������� ������������
		WHWork work;				///< ��������
		HWork holder;				///< ��������, ���� �������� ����� ���������
	};

private:
	static const int DefaultPoolSize = 1024;
	
	typedef std::list< WorkItem, SafeAlloc<WorkItem> > WorkItems_t;

	HANDLE m_timer;
	HANDLE m_thread;
	HANDLE m_exitThread;
	WorkItems_t m_workItems;

public:
	Worker();
	~Worker();

	/// ��������� ������ � �������
	void addWork(HWork work, unsigned int period, bool alone = false);
	/// ������ ��� alone ������
	void finish();

	/// ���������� ��������� ������
	static Worker *inst();

private:
	void setTimer();
	void startThread();
	void executeWork();

	void worker();
	static unsigned int __stdcall threadProxy(void *);
};

}
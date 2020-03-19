/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description:
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)

************************************************************************/
#include "precomp.h"
#include <ctime>
#include "Timer.h"

bool  Timer::m_fsuspend;
float Timer::m_counter;
float Timer::m_freq;
ULARGE_INTEGER Timer::m_initial;   

void Timer::Init(void)
{
    //������� �������
    m_counter = 0;
    
    //�������� ���������� ����� ���������� � ���.
    ULARGE_INTEGER a, b, s;
    
    int tp, pp;
	HANDLE t, p;

	t = GetCurrentThread();
	tp = GetThreadPriority(t);
	SetThreadPriority(t, THREAD_PRIORITY_TIME_CRITICAL);
	p = GetCurrentProcess();
	pp = GetPriorityClass(p);
	SetPriorityClass(p, REALTIME_PRIORITY_CLASS);

	Sleep(1);
	GetCPUTicks(&a);   // read the performance counter 
	Sleep(100);        // pause 100 ms 
	GetCPUTicks(&b);   // read the performance counter 
    
    //!!! ~~~~~~~~~ �������� ~~~~~~~~~~~ !!!
    // �������� � ������������� �������������� �������������
    // �������� ������, ��� ��� � ��������� unsigned ����. 
    // ��������� � ��� �� ������ ������������?
    //
  	s.QuadPart = b.QuadPart - a.QuadPart;
		
    SetThreadPriority(t, tp);
	SetPriorityClass(p, pp);

    //�������� � �������� �������� ��cm���
    m_freq = (__int64)(10 * s.QuadPart);

    //�������� ������� ��������� ��������
    GetCPUTicks(&m_initial);	
}

void Timer::Update(void)
{
    ULARGE_INTEGER  cur, delta;

    GetCPUTicks(&cur);    

    //!!! ~~~~~~~~~ �������� ~~~~~~~~~~~ !!!
    // �������� � ������������� �������������� �������������
    // �������� ������, ��� ��� � ��������� unsigned ����. 
    // ��������� � ��� �� ������ ������������?
    //
    delta.QuadPart = cur.QuadPart - m_initial.QuadPart;
    
    //�������� ������� ��������� ��������
    m_initial = cur;
    
    //���� ������ ���������� �� ���
    if(m_fsuspend) return;

    m_counter += ((__int64) delta.QuadPart) / m_freq;
}

void Timer::Suspend()
{
    //��������� ������� ������
    m_fsuspend = true;
}

void Timer::Resume()
{
    //������� ������� ��������� ��������
    Update();

    //��������� ������� ������
    m_fsuspend = false;
}

tm* Timer::GetSysTime(void)
{
	time_t long_time;

	time(&long_time);          

	return localtime(&long_time);
}



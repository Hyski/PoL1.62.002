/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description:
   --------------------------------------------------------------------
    Author:			Pavel A.Duvanov   (Naughty)
	Modified by:	Alexander Garanin (Punch)

************************************************************************/
#ifndef _TIMER_H_
#define _TIMER_H_

struct tm;

class Timer{
public:
    static void Init(void);
    static void Update(void);
	
    //������� ������� �����
    static float GetSeconds(void);

    //���������� ������
    static void Suspend();
    //��������� ������ ������
    static void Resume();

	//	--------------------
	static tm* GetSysTime(void);

private:

    static void GetCPUTicks(ULARGE_INTEGER* ptr);

private:
	Timer();

    static float  m_counter;
    static float  m_freq;           //cpu ticks per second
    static ULARGE_INTEGER m_initial;//��������� �������� ��������

    static bool   m_fsuspend;       //���������
};

//
//~~~~~~~~~inlines~~~~~~~~~~~~
//

inline float Timer::GetSeconds(void)
{
    return m_counter;
}

inline void Timer::GetCPUTicks(ULARGE_INTEGER* _ptr)
{
    __asm{
	    _emit 0x0f   ; ������� �������� ��������
        _emit 0x31   

        mov ebx, _ptr ; ��������� ����� 

	    mov [ebx],     eax ; ������� �����  
        mov [ebx + 4], edx ; ������� ����� 
	}
}

#endif	//_TIMER_H_

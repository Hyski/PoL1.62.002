#include "precomp.h"

namespace mll
{

namespace utils
{
//---------------------------------------------------------------------------

namespace
{

const int timeCheck = 110;

#pragma warning(push)
#if defined(__ICL)
#pragma warning(disable:1011)
#elif defined(_MSC_VER)
#endif
/*=====================================================================================*\
 *                                       rdtsc()                                       * 
\*=====================================================================================*/
inline __int64 rdtsc()
{
	__asm _emit 0x0f;
	__asm _emit 0x31;
}
#pragma warning(pop)

__int64 getTscEvalTime()
{
	return rdtsc()-rdtsc();
}

} // anonymus namespace

//---------------------------------------------------------------------------
// Класс определяющий частоту процессора. Используется для инициализации таймера.
_time_init::_time_init() 
{
    int               thread_prior, proc_prior;
	HANDLE            cur_thread, cur_proc;

	// выставить высокие приоритеты
	cur_thread   = GetCurrentThread();
	thread_prior = GetThreadPriority(cur_thread);

	SetThreadPriority(cur_thread, THREAD_PRIORITY_TIME_CRITICAL);

	cur_proc   = GetCurrentProcess();
	proc_prior = GetPriorityClass(cur_proc);

	SetPriorityClass(cur_proc, REALTIME_PRIORITY_CLASS);

	__int64 tscEval = getTscEvalTime();

	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);

	__int64 frequency;

	{
		__int64 begin, end;
		LARGE_INTEGER start,tmp;

		QueryPerformanceCounter(&tmp);
		QueryPerformanceCounter(&start); begin = rdtsc();
		for(int j = 0; j < 100000; ++j)
		{
			QueryPerformanceCounter(&tmp);
		}
		end = rdtsc();

		frequency = __int64(long double(freq.QuadPart) / long double(tmp.QuadPart - start.QuadPart)
			* long double(end-begin+tscEval));
	}

	// восстановить исходные приоритеты
	SetThreadPriority(cur_thread, thread_prior);
	SetPriorityClass(cur_proc, proc_prior);

	// 1 / Кол-во тактовых импульсов за 1 секунду
	m_invFrequency = static_cast<float>(1.0 / static_cast<long double>(frequency));
}
//---------------------------------------------------------------------------

_time_init::~_time_init() 
{}
//---------------------------------------------------------------------------



_time_init time::m_initter;

//---------------------------------------------------------------------------

timer::timer()
:	m_startTime(rdtsc())
{
}
//---------------------------------------------------------------------------

timer::~timer()
{}
//---------------------------------------------------------------------------

time timer::get() const
{
	__int64 t = rdtsc();
	t -= m_startTime;
	return t;
}
//---------------------------------------------------------------------------

} // namespace utils

} // namespace mll


#ifndef _MLL_TIMER_H_
#define _MLL_TIMER_H_

#include <mll/_choose_lib.h>
#include <mll/_export_rules.h>

namespace mll
{

namespace utils
{


// ����� ��� ������������� �������
class MLL_EXPORT _time_init
{
public:
	_time_init();
	~_time_init();

	inline float getInvFreq() const { return m_invFrequency; };

private:
    float m_invFrequency; // ���-�� �������� ��������� �� 1 �������
}; // _time_init

// ��������! ��� ������� �� ����������� (��. MSDN __int64)

//---------------------------------------------------------------------------

// �����
class MLL_EXPORT time
{
public:
	time() : m_time(0i64) {}
	time(const __int64& t) : m_time(t) {}
	time(const time& t) : m_time(t.m_time) {}
//	time(float t) { m_time = static_cast<__int64>(t/m_initter.getInvFreq()); }
	//
//	operator float() const { return static_cast<float>(m_time) * m_initter.getInvFreq(); }
	//
	time& operator=(const time& t)
	{ m_time = t.m_time; return *this; }
	//
	bool operator==(const time& t) const
	{ return m_time==t.m_time; }
	//
	bool operator!=(const time& t) const
	{ return !((*this) == t); }
	//
	inline time& operator+=(const time& t)
	{ m_time += t.m_time; return *this; }
	//
	inline time operator+(const time& t) const
	{ time temp(*this); temp += t; return temp; }
	//
	inline time& operator-=(const time& t)
	{ m_time -= t.m_time; return *this; }
	//
	inline time operator-(const time& t) const
	{ time temp(*this); temp -= t; return temp; }
	//	��������� ������ �� float'��
	float seconds(void) const 
	{ return static_cast<float>(m_time) * m_initter.getInvFreq(); }

private:
	__int64 m_time;
	static _time_init m_initter;
}; // time

//---------------------------------------------------------------------------

// ������. 
// ��������! ����������� ������������� ������� �������� ����� 100 �����������
class MLL_EXPORT timer
{
public:
	//
	timer();
	//
	~timer();
	//
	time get() const;
private:
	__int64 m_startTime;
}; // timer

} // namespace utils

} // namespace mll

#endif	// _MLL_TIMER_H_
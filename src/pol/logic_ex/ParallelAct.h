#pragma once

#include "ComplexAct.h"

namespace PoL
{

//=====================================================================================//
//                        class ParallelAct : public ComplexAct                        //
//=====================================================================================//
class ParallelAct : public ComplexAct
{
	bool m_started;
	Acts_t::iterator m_lastAct;
	size_t m_endedCount;
	size_t m_endableCount;

public:
	template<typename It> ParallelAct(It,It);

	/// ��������� ���, �������� ��� ��� �������� �������
	virtual void skip();
	/// ������� �� ���
	virtual bool isStarted() const { return m_started; }
	/// ������ ���
	virtual void start();
	/// ���� ���������� ����, ����� �� ������� ������� ��������
	virtual void update(float tau);
	/// ���������� true, ���� ��� ��������
	virtual bool isEnded() const { return m_endableCount && m_endedCount == m_endableCount; }
	/// ����� �� �������� �����������
	virtual bool isEndable() const { return m_endableCount != 0; }

private:
	/// ���������� ���������� ����������� �����
	size_t calcEndable() const;
};

//=====================================================================================//
//                          inline ParallelAct::ParallelAct()                          //
//=====================================================================================//
template<typename It>
inline ParallelAct::ParallelAct(It begin,It end)
:	ComplexAct(begin,end),
	m_lastAct(actsEnd()),
	m_endedCount(0),
	m_started(false),
	m_endableCount(calcEndable())
{
}

}
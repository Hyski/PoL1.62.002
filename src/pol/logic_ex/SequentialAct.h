#pragma once

#include "ComplexAct.h"

namespace PoL
{

//=====================================================================================//
//                       class SequentialAct : public ComplexAct                       //
//=====================================================================================//
class SequentialAct : public ComplexAct
{
	bool m_started;
	bool m_endable;
	Acts_t::iterator m_currentAct;

public:
	template<typename It> SequentialAct(It begin, It end);

	/// ��������� ���, �������� ��� ��� �������� �������
	virtual void skip();
	/// ������� �� ���
	virtual bool isStarted() const { return m_started; }
	/// ������ ���
	virtual void start();
	/// ���� ���������� ����, ����� �� ������� ������� ��������
	virtual void update(float tau);
	/// ���������� true, ���� ��� ��������
	virtual bool isEnded() const { return m_currentAct == actsEnd(); }
	/// ����� �� �������� �����������
	virtual bool isEndable() const { return m_endable; }

private:
	/// ���������� ����������� ��������� ��������
	bool calcEndable() const;
	/// ������� �� ��������
	bool isValid() const;
};

//=====================================================================================//
//                        inline SequentialAct::SequentialAct()                        //
//=====================================================================================//
template<typename It>
inline SequentialAct::SequentialAct(It begin, It end)
:	ComplexAct(begin,end),
	m_started(false),
	m_endable(calcEndable()),
	m_currentAct(actsBegin())
{
}

}
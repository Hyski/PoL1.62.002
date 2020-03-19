#pragma once

#include "Act.h"
#include "Counter.h"

namespace PoL
{

//=====================================================================================//
//                             class PulseAct : public Act                             //
//=====================================================================================//
class PulseAct : public Act
{
	bool m_started;
	bool m_ended;
	HCounter m_ctr;
	int m_amount;


public:
	PulseAct(const std::string &, int);

	/// ��������� ���, �������� ��� ��� �������� �������
	virtual void skip() {}
	/// ������� �� ���
	virtual bool isStarted() const { return m_started; }
	/// ������ ���
	virtual void start();
	/// ���� ���������� ����, ����� �� ������� ������� ��������
	virtual void update(float tau);
	/// ���������� true, ���� ��� ��������
	virtual bool isEnded() const { return m_ended; }
	/// ����� �� �������� �����������
	virtual bool isEndable() const { return true; }
};

}
#pragma once

#include "Act.h"
#include "Counter.h"

namespace PoL
{

//=====================================================================================//
//                         class AddToCounterAct : public Act                          //
//=====================================================================================//
class AddToCounterAct : public Act
{
	bool m_started;
	bool m_ended;
	HCounter m_ctr;
	int m_delta;

public:
	AddToCounterAct(const std::string &, int delta);

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
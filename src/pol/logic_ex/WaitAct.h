#pragma once

#include "Act.h"

namespace PoL
{

//=====================================================================================//
//                             class WaitAct : public Act                              //
//=====================================================================================//
class WaitAct : public Act
{
	bool m_started;
	bool m_ended;
	float m_delay;

public:
	WaitAct(float);

	/// ��������� ���, �������� ��� ��� �������� �������
	virtual void skip() { m_ended = true; }
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

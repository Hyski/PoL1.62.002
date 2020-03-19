#pragma once

#include "Act.h"

namespace PoL
{

//=====================================================================================//
//                            class GiveExpAct : public Act                            //
//=====================================================================================//
class GiveExpAct : public Act
{
	bool m_started;
	bool m_ended;
	int m_expAmount;

public:
	GiveExpAct(int exp);

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
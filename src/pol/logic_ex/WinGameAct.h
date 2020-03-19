#pragma once

#include "Act.h"

namespace PoL
{

//=====================================================================================//
//                            class WinGameAct : public Act                            //
//=====================================================================================//
class WinGameAct : public Act
{
	bool m_started;
	bool m_ended;
	int m_phase;

public:
	WinGameAct();

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
	/// �������� �� ��� ������� ��� ��������
	virtual bool hasEndGame() const { return true; }
};

}

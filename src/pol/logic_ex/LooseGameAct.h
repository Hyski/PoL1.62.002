#pragma once

#include "Act.h"

namespace PoL
{

//=====================================================================================//
//                           class LooseGameAct : public Act                           //
//=====================================================================================//
class LooseGameAct : public Act
{
	bool m_started;
	bool m_ended;

public:
	LooseGameAct();

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
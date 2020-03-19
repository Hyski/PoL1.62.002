#pragma once

#include "Act.h"

namespace PoL
{

//=====================================================================================//
//                         class ResetCounterAct : public Act                          //
//=====================================================================================//
class ResetCounterAct : public Act
{
	bool m_started;
	bool m_ended;
	HCounter m_ctr;
	HQuest m_quest;

public:
	ResetCounterAct(const std::string &);

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

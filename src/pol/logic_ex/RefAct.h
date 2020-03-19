#pragma once

#include "Act.h"

namespace PoL
{

//=====================================================================================//
//                              class RefAct : public Act                              //
//=====================================================================================//
class RefAct : public Act
{
	HAct m_act;

public:
	RefAct(const std::string &act);

	/// ��������� ���, �������� ��� ��� �������� �������
	virtual void skip() { m_act->skip(); }
	/// ������� �� ���
	virtual bool isStarted() const { return m_act->isStarted(); }
	/// ������ ���
	virtual void start() { m_act->start(); }
	/// ���� ���������� ����, ����� �� ������� ������� ��������
	virtual void update(float tau) { m_act->update(tau); }
	/// ���������� true, ���� ��� ��������
	virtual bool isEnded() const { return m_act->isEnded(); }
	/// ����� �� �������� �����������
	virtual bool isEndable() const { return m_act->isEndable(); }
};

}

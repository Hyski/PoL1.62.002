#pragma once

#include "Act.h"

namespace PoL
{

//=====================================================================================//
//                             class PrintAct : public Act                             //
//=====================================================================================//
class PrintAct : public Act
{
	bool m_started;
	bool m_ended;
	std::string m_strid;

public:
	PrintAct(const std::string &);

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
#pragma once

#include "Act.h"

namespace PoL
{

//=====================================================================================//
//                              class SayAct : public Act                              //
//=====================================================================================//
class SayAct : public Act
{
	bool m_started;
	bool m_ended;
	std::string m_who;
	std::string m_strid;

public:
	SayAct(const std::string &who, const std::string &strid);

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

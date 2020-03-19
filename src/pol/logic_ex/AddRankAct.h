#pragma once

#include "Act.h"

namespace PoL
{

//=====================================================================================//
//                            class AddRankAct : public Act                            //
//=====================================================================================//
class AddRankAct : public Act
{
	bool m_started;
	bool m_ended;
	std::string m_entity;

public:
	AddRankAct(const std::string &);

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

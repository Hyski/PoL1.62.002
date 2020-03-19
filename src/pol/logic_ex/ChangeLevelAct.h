#pragma once

#include "Act.h"

namespace PoL
{

//=====================================================================================//
//                          class ChangeLevelAct : public Act                          //
//=====================================================================================//
class ChangeLevelAct : public Act
{
	std::string m_level;
	bool m_started;
	bool m_ended;

public:
	ChangeLevelAct(const std::string &);

	/// ��������� ���, �������� ��� ��� �������� �������
	virtual void skip() {}
	/// ������� �� ���
	virtual bool isStarted() const { return m_started; }
	/// ������ ���
	virtual void start();
	/// ���� ���������� ����, ����� �� ������� ������� ��������
	virtual void update(float tau);
	/// ���������� true, ���� ��� ��������
	virtual bool isEnded() const;
	/// ����� �� �������� �����������
	virtual bool isEndable() const { return true; }
};

}
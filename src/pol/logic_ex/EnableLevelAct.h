#pragma once

#include "Act.h"

namespace PoL
{

//=====================================================================================//
//                          class EnableLevelAct : public Act                          //
//=====================================================================================//
class EnableLevelAct : public Act
{
	bool m_started;
	bool m_ended;
	std::string m_level;
	bool m_enable;

public:
	EnableLevelAct(const std::string &level, bool enable);

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

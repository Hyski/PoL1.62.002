#pragma once

#include "Act.h"

namespace PoL
{

//=====================================================================================//
//                             class HeroAct : public Act                              //
//=====================================================================================//
class HeroAct : public Act
{
	bool m_ask;
	bool m_changeCounters;
	bool m_started;
	bool m_ended;
	std::string m_hero;
	bool m_add;

public:
	HeroAct(const std::string &, bool);
	HeroAct(const std::string &, bool, bool special);
	HeroAct(const std::string &, bool, bool ask, bool chc);

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

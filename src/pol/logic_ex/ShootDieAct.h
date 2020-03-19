#pragma once

#include "Act.h"

namespace PoL
{

//=====================================================================================//
//                           class ShootDieAct : public Act                            //
//=====================================================================================//
class ShootDieAct : public Act
{
	std::string m_who, m_target;
	bool m_die, m_shoot;

	std::auto_ptr<::Activity> m_shootact;

	bool m_shootCreated;

	bool m_started;
	bool m_ended;

public:
	ShootDieAct(const std::string &who, const std::string &target, bool shoot, bool die);

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

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

	/// Закончить акт, применив все его побочные эффекты
	virtual void skip() {}
	/// Начался ли акт
	virtual bool isStarted() const { return m_started; }
	/// Начать акт
	virtual void start();
	/// Дать управление акту, чтобы он изменил игровую ситуацию
	virtual void update(float tau);
	/// Возвращает true, если акт закончен
	virtual bool isEnded() const { return m_ended; }
	/// Может ли действие закончиться
	virtual bool isEndable() const { return true; }
};

}

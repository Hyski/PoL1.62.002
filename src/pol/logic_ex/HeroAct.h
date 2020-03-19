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

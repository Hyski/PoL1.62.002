#pragma once

#include "Condition.h"
#include "TaskDatabase.h"

namespace PoL
{

//=====================================================================================//
//                       class HeroCondition : public Condition                        //
//=====================================================================================//
class HeroCondition : public Condition
{
	std::string m_hero;
	bool m_inteam;

	TaskDatabase::Connection m_htcConn, m_spawnConn, m_deadConn;

public:
	HeroCondition(Id_t id, const std::string &, bool inteam);

	/// Возвращает тип объекта
	virtual SizerTypes getType() const { return stHeroCond; }

	/// Установить приоритет
	virtual void setPriority(int);

	virtual void reset();

private:
	void onHeroDead();
	void onHeroTeamChanged();
};

}

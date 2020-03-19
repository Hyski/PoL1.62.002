#include "precomp.h"
#include "HeroCondition.h"

namespace PoL
{

//=====================================================================================//
//                             static bool isHeroInTeam()                              //
//=====================================================================================//
static bool isHeroInTeam(::eid_t eid)
{
	::BaseEntity * const e = getEntityById(eid);
	return (e->Cast2Human() && e->Cast2Human()->GetPlayer() == PT_PLAYER);
}

//=====================================================================================//
//                           HeroCondition::HeroCondition()                            //
//=====================================================================================//
HeroCondition::HeroCondition(Id_t id, const std::string &hero, bool inteam)
:	Condition(!inteam,id),
	m_hero(hero),
	m_inteam(inteam)
{
	m_htcConn = TaskDatabase::instance()->attach(boost::bind(onHeroTeamChanged,this),hero,TaskDatabase::mtHeroTeamChanged);
	m_spawnConn = TaskDatabase::instance()->attach(boost::bind(onHeroTeamChanged,this),hero,TaskDatabase::mtNamedEntitySpawned);
	m_deadConn = TaskDatabase::instance()->attach(boost::bind(onHeroDead,this),hero,TaskDatabase::mtDead);
}

//=====================================================================================//
//                          void HeroCondition::onHeroDead()                           //
//=====================================================================================//
void HeroCondition::onHeroDead()
{
	setState(!m_inteam);
}

//=====================================================================================//
//                          void HeroCondition::setPriority()                          //
//=====================================================================================//
void HeroCondition::setPriority(int p)
{
	m_htcConn.setPriority(p);
	m_spawnConn.setPriority(p);
	m_deadConn.setPriority(p);
}

//=====================================================================================//
//                       void HeroCondition::onHeroTeamChanged()                       //
//=====================================================================================//
void HeroCondition::onHeroTeamChanged()
{
	setState(isHeroInTeam(TaskDatabase::instance()->getMessageInProcess().m_subject_id) == m_inteam);
}

//=====================================================================================//
//                             void HeroCondition::reset()                             //
//=====================================================================================//
void HeroCondition::reset()
{
	Condition::reset();
}

}

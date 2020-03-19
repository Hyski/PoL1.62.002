#include "precomp.h"
#include "HeroAct.h"
#include "ActFactory.h"

namespace PoL
{

//=====================================================================================//
//                                 HeroAct::HeroAct()                                  //
//=====================================================================================//
HeroAct::HeroAct(const std::string &hero, bool add)
:	m_started(false),
	m_ended(false),
	m_hero(hero),
	m_add(add),
	m_ask(true),
	m_changeCounters(true)
{
}

//=====================================================================================//
//                                 HeroAct::HeroAct()                                  //
//=====================================================================================//
HeroAct::HeroAct(const std::string &hero, bool add, bool spec)
:	m_started(false),
	m_ended(false),
	m_hero(hero),
	m_add(add),
	m_ask(!spec),
	m_changeCounters(!spec)
{
}

//=====================================================================================//
//                                 HeroAct::HeroAct()                                  //
//=====================================================================================//
HeroAct::HeroAct(const std::string &hero, bool add, bool ask, bool chc)
:	m_started(false),
	m_ended(false),
	m_hero(hero),
	m_add(add),
	m_ask(ask),
	m_changeCounters(chc)
{
}

//=====================================================================================//
//                                void HeroAct::start()                                //
//=====================================================================================//
void HeroAct::start()
{
	m_started = true;
}

//=====================================================================================//
//                               void HeroAct::update()                                //
//=====================================================================================//
void HeroAct::update(float)
{
	assert( m_started );
	assert( !m_ended );

	if(!m_hero.empty())
	{
		if(m_add)
		{
			if(m_changeCounters) ActFactory::instance()->addAvailableHero(m_hero);
			if(BaseEntity *hero = getEntityByName(m_hero))
			{
				addHeroToTeam(hero,!m_ask);
			}
		}
		else
		{
			if(m_changeCounters) ActFactory::instance()->removeAvailableHero(m_hero);
			if(BaseEntity *hero = getEntityByName(m_hero))
			{
				removeHeroFromTeam(hero,!m_ask);
			}
		}
	}

	m_ended = true;
}


}

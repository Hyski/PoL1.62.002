#include "precomp.h"
#include "ActFactory.h"
#include "DisappearAct.h"

namespace PoL
{

//=====================================================================================//
//                            DisappearAct::DisappearAct()                             //
//=====================================================================================//
DisappearAct::DisappearAct(const std::string &hero)
:	m_started(false),
	m_ended(false),
	m_hero(hero)
{
}

//=====================================================================================//
//                             void DisappearAct::start()                              //
//=====================================================================================//
void DisappearAct::start()
{
	m_started = true;
}

//=====================================================================================//
//                             void DisappearAct::update()                             //
//=====================================================================================//
void DisappearAct::update(float)
{
	assert( m_started );
	assert( !m_ended );

	if(!m_hero.empty())
	{
		while(BaseEntity *entity = getEntityByName(m_hero))
		{
			disappearEntity(entity);
		}
	}

	m_ended = true;
}


}

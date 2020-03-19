#include "precomp.h"
#include "DestructAct.h"
#include <undercover/GameLevel/ExplosionManager.h>

namespace PoL
{

//=====================================================================================//
//                             DestructAct::DestructAct()                              //
//=====================================================================================//
DestructAct::DestructAct(const std::string &obj)
:	m_started(false),
	m_ended(false),
	m_obj(obj)
{
}

//=====================================================================================//
//                              void DestructAct::start()                              //
//=====================================================================================//
void DestructAct::start()
{
	m_started = true;
}

//=====================================================================================//
//                             void DestructAct::update()                              //
//=====================================================================================//
void DestructAct::update(float)
{
	assert( m_started );
	assert( !m_ended );

	::ExplosionManager::Get()->BurnObject(m_obj,10000);

	m_ended = true;
}

}
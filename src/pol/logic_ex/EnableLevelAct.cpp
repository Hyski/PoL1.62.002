#include "precomp.h"
#include "EnableLevelAct.h"

namespace PoL
{

//=====================================================================================//
//                          EnableLevelAct::EnableLevelAct()                           //
//=====================================================================================//
EnableLevelAct::EnableLevelAct(const std::string &lev, bool e)
:	m_started(false),
	m_ended(false),
	m_level(lev),
	m_enable(e)
{
}

//=====================================================================================//
//                            void EnableLevelAct::start()                             //
//=====================================================================================//
void EnableLevelAct::start()
{
	m_started = true;
}

//=====================================================================================//
//                            void EnableLevelAct::update()                            //
//=====================================================================================//
void EnableLevelAct::update(float)
{
	assert( m_started );
	assert( !m_ended );

	::Spawner::GetInst()->EnableExit(m_level,m_enable);

	m_ended = true;
}

}

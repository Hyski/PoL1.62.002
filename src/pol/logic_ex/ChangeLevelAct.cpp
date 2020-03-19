#include "precomp.h"
#include "ChangeLevelAct.h"

namespace PoL
{

//=====================================================================================//
//                          ChangeLevelAct::ChangeLevelAct()                           //
//=====================================================================================//
ChangeLevelAct::ChangeLevelAct(const std::string &name)
:	m_level(name),
	m_started(false),
	m_ended(false)
{
}

//=====================================================================================//
//                            void ChangeLevelAct::start()                             //
//=====================================================================================//
void ChangeLevelAct::start()
{
	m_started = true;
}

//=====================================================================================//
//                            void ChangeLevelAct::update()                            //
//=====================================================================================//
void ChangeLevelAct::update(float)
{
	assert( !m_ended );
	assert( m_started );

	Spawner::GetInst()->ExitLevel(m_level);
	m_ended = true;
}

//=====================================================================================//
//                        bool ChangeLevelAct::isEnded() const                         //
//=====================================================================================//
bool ChangeLevelAct::isEnded() const
{
	return m_ended;
}

}
#include "precomp.h"
#include "PhaseAct.h"

namespace PoL
{

//=====================================================================================//
//                                PhaseAct::PhaseAct()                                 //
//=====================================================================================//
PhaseAct::PhaseAct(int p)
:	m_started(false),
	m_ended(false),
	m_phase(p)
{
}

//=====================================================================================//
//                               void PhaseAct::start()                                //
//=====================================================================================//
void PhaseAct::start()
{
	m_started = true;
}

//=====================================================================================//
//                               void PhaseAct::update()                               //
//=====================================================================================//
void PhaseAct::update(float)
{
	assert( m_started );
	assert( !m_ended );

	::Spawner::GetInst()->SetPhase(m_phase);

	m_ended = true;
}

}
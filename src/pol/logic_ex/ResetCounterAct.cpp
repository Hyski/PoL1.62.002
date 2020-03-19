#include "precomp.h"
#include "Counter.h"
#include "ActFactory.h"
#include "ResetCounterAct.h"

namespace PoL
{

//=====================================================================================//
//                         ResetCounterAct::ResetCounterAct()                          //
//=====================================================================================//
ResetCounterAct::ResetCounterAct(const std::string &ctr)
:	m_started(false),
	m_ended(false)
{
	m_quest = ActFactory::instance()->getQuestByName(ctr);
	m_ctr = ActFactory::instance()->getCounterByName(ctr);
}

//=====================================================================================//
//                            void ResetCounterAct::start()                            //
//=====================================================================================//
void ResetCounterAct::start()
{
	m_started = true;

	if(m_ctr) m_ctr->setCounter(m_ctr->getInitialValue());
}

//=====================================================================================//
//                           void ResetCounterAct::update()                            //
//=====================================================================================//
void ResetCounterAct::update(float)
{
	assert( m_started );
	assert( !m_ended );

	if(m_quest)
	{
		m_quest->resetTasks();
		m_quest->reset();
	}

	m_ended = true;
}


}

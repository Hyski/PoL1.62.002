#include "precomp.h"

#include "ActFactory.h"
#include "AddToCounterAct.h"

namespace PoL
{

//=====================================================================================//
//                         AddToCounterAct::AddToCounterAct()                          //
//=====================================================================================//
AddToCounterAct::AddToCounterAct(const std::string &ctr, int delta)
:	m_started(false),
	m_ended(false),
	m_ctr(ActFactory::instance()->getCounterByName(ctr)),
	m_delta(delta)
{
}

//=====================================================================================//
//                            void AddToCounterAct::start()                            //
//=====================================================================================//
void AddToCounterAct::start()
{
	m_started = true;
}

//=====================================================================================//
//                           void AddToCounterAct::update()                            //
//=====================================================================================//
void AddToCounterAct::update(float)
{
	assert( m_started );
	assert( !m_ended );

	if(m_ctr) m_ctr->setCounter(m_ctr->getCounter()+m_delta);

	m_ended = true;
}

}
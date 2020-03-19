#include "precomp.h"

#include "PulseAct.h"
#include "ActFactory.h"

namespace PoL
{

//=====================================================================================//
//                                PrintAct::PrintAct()                                 //
//=====================================================================================//
PulseAct::PulseAct(const std::string &ctr, int amount)
:	m_started(false),
	m_ended(false),
	m_amount(amount)
{
	m_ctr = ActFactory::instance()->getCounterByName(ctr);
}

//=====================================================================================//
//                               void PrintAct::start()                                //
//=====================================================================================//
void PulseAct::start()
{
	m_started = true;
}

//=====================================================================================//
//                               void PrintAct::update()                               //
//=====================================================================================//
void PulseAct::update(float)
{
	assert( m_started );
	assert( !m_ended );

	if(m_ctr)
	{
		int curval = m_ctr->getCounter();
		m_ctr->setCounter(curval+m_amount);
		m_ctr->setCounter(curval);
	}

	m_ended = true;
}


}
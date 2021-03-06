#include "precomp.h"
#include "WaitAct.h"

namespace PoL
{

//=====================================================================================//
//                                 WaitAct::WaitAct()                                  //
//=====================================================================================//
WaitAct::WaitAct(float tau)
:	m_started(false),
	m_ended(false),
	m_delay(tau)
{
}

//=====================================================================================//
//                                void WaitAct::start()                                //
//=====================================================================================//
void WaitAct::start()
{
	m_started = true;
}

//=====================================================================================//
//                               void WaitAct::update()                                //
//=====================================================================================//
void WaitAct::update(float tau)
{
	assert( isStarted() );
	assert( !isEnded() );

	m_delay -= tau;

	if(m_delay <= 0.0f)
	{
		m_ended = true;
	}
}

}
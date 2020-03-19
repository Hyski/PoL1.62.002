#include "precomp.h"
#include "SequentialAct.h"

namespace PoL
{

//=====================================================================================//
//                            void SequentialAct::update()                             //
//=====================================================================================//
void SequentialAct::update(float tau)
{
	assert( isStarted() );
	assert( !isEnded() );

	if(!(*m_currentAct)->isEnded())
	{
		(*m_currentAct)->update(tau);
	}

	if((*m_currentAct)->isEnded())
	{
		++m_currentAct;
		if(!isEnded())
		{
			(*m_currentAct)->start();
		}
	}
}

//=====================================================================================//
//                             void SequentialAct::start()                             //
//=====================================================================================//
void SequentialAct::start()
{
	assert( !isStarted() );

	m_started = true;

	if(!isValid())
	{
		m_currentAct = actsEnd();
	}
	else
	{
		m_currentAct = actsBegin();
		(*m_currentAct)->start();
	}
}

//=====================================================================================//
//                             void SequentialAct::skip()                              //
//=====================================================================================//
void SequentialAct::skip()
{
	assert( isStarted() );
	assert( !isEnded() );

	for(; m_currentAct != actsEnd(); ++m_currentAct)
	{
		(*m_currentAct)->skip();
	}

	m_started = false;
}

//=====================================================================================//
//                         bool SequentialAct::isValid() const                         //
//=====================================================================================//
bool SequentialAct::isValid() const
{
	size_t cnt = actsCount() - std::count_if(actsBegin(),actsEnd(),boost::bind(Act::isEndable,_1));
	if((cnt > 1) || (cnt == 1 && !(*(actsEnd()-1))->isEndable())) return false;
	return true;
}

//=====================================================================================//
//                       bool SequentialAct::calcEndable() const                       //
//=====================================================================================//
bool SequentialAct::calcEndable() const
{
	return std::count_if(actsBegin(),actsEnd(),boost::bind(Act::isEndable,_1)) != 0;
}

}
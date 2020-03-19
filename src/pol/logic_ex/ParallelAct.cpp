#include "precomp.h"
#include "ParallelAct.h"

namespace PoL
{

namespace ParallelActDetail
{

struct IsActNotEnded
{
	bool operator()(const HAct &a) const
	{
		return !a->isEnded();
	}
};

}

using namespace ParallelActDetail;

//=====================================================================================//
//                              void ParallelAct::start()                              //
//=====================================================================================//
void ParallelAct::start()
{
	assert( !isStarted() );
	m_started = true;

	for(Acts_t::iterator i = actsBegin(); i != actsEnd(); ++i)
	{
		(*i)->start();
	}
}

//=====================================================================================//
//                             void ParallelAct::update()                              //
//=====================================================================================//
void ParallelAct::update(float tau)
{
	assert( isStarted() );
	assert( !isEnded() );

	if(!isEnded())
	{
		for(Acts_t::iterator i = actsBegin(); i != m_lastAct; ++i)
		{
			(*i)->update(tau);
		}
	}

	m_lastAct = std::stable_partition(actsBegin(),actsEnd(),IsActNotEnded());
	m_endedCount = std::distance(m_lastAct,actsEnd());
}

//=====================================================================================//
//                              void ParallelAct::skip()                               //
//=====================================================================================//
void ParallelAct::skip()
{
	for(Acts_t::iterator i = actsBegin(); i != m_lastAct; ++i)
	{
		(*i)->skip();
	}
	m_lastAct = actsBegin();
}

//=====================================================================================//
//                       size_t ParallelAct::calcEndable() const                       //
//=====================================================================================//
size_t ParallelAct::calcEndable() const
{
	return std::count_if(actsBegin(),actsEnd(),boost::bind(Act::isEndable,_1));
}

}
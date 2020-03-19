#include "precomp.h"
#include "QuestContextAct.h"

namespace PoL
{

//=====================================================================================//
//                         QuestContextAct::QuestContextAct()                          //
//=====================================================================================//
QuestContextAct::QuestContextAct(HAct act, const std::string &holder,
								 const std::string &actor)
:	m_started(false),
	m_ended(false),
	m_endable(act->isEndable()),
	m_act(act),
	m_actor(actor),
	m_holder(holder),
	m_contextId(-1)
{
}

//=====================================================================================//
//                         QuestContextAct::~QuestContextAct()                         //
//=====================================================================================//
QuestContextAct::~QuestContextAct()
{
}

//=====================================================================================//
//                             void QuestReportAct::skip()                             //
//=====================================================================================//
void QuestContextAct::skip()
{
	assert( isStarted() );
	assert( !isEnded() );

	m_act->skip();

	m_started = false;
}

//=====================================================================================//
//                            void QuestReportAct::start()                             //
//=====================================================================================//
void QuestContextAct::start()
{
	assert( !isStarted() );
	m_started = true;

	m_contextId = QuestContext::createContext(m_holder,m_actor);
	m_act->start();
}

//=====================================================================================//
//                           void QuestContextAct::update()                            //
//=====================================================================================//
void QuestContextAct::update(float tau)
{
	assert( isStarted() );
	assert( !isEnded() );

	if(!m_act->isEnded()) m_act->update(tau);
	if(m_act->isEnded())
	{
		m_ended = true;
		QuestContext::removeContext(m_contextId);
	}
}


}
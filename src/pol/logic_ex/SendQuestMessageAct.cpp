#include "precomp.h"

#include "Task.h"
#include "ActFactory.h"
#include "SendQuestMessageAct.h"

namespace PoL
{

//=====================================================================================//
//                     SendQuestMessageAct::SendQuestMessageAct()                      //
//=====================================================================================//
SendQuestMessageAct::SendQuestMessageAct(const std::string &quest, QuestMessage msg)
:	m_started(false),
	m_ended(false),
	m_msg(msg)
{
	m_quest = ActFactory::instance()->getQuestByName(quest);
}

//=====================================================================================//
//                          void SendQuestMessageAct::start()                          //
//=====================================================================================//
void SendQuestMessageAct::start()
{
	m_started = true;
}

//=====================================================================================//
//                         void SendQuestMessageAct::update()                          //
//=====================================================================================//
void SendQuestMessageAct::update(float tau)
{
	assert( m_started );
	assert( !m_ended );

	if(m_quest) m_quest->sendMessage(m_msg);

	m_ended = true;
}

}
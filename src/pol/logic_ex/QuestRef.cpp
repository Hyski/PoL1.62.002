#include "precomp.h"
#include "QuestRef.h"

namespace PoL
{

//=====================================================================================//
//                                QuestRef::QuestRef()                                 //
//=====================================================================================//
QuestRef::QuestRef()
:	m_quest(0)
{
}

//=====================================================================================//
//                              void QuestRef::setQuest()                              //
//=====================================================================================//
void QuestRef::setQuest(Quest *quest)
{
	assert( !m_quest );
	m_quest = quest;
	m_questConn = m_quest->addFn(boost::bind(onQuestStateChanged,this,_1));

	if(getState() != qsNotStarted)
	{
		notify(getState());
	}
}

//=====================================================================================//
//                        void QuestRef::onQuestStateChanged()                         //
//=====================================================================================//
void QuestRef::onQuestStateChanged(QuestState qs)
{
	if(qs == qsDestroyed)
	{
		m_quest = 0;
		m_questConn.reset();
	}
	else
	{
		notify(qs);
	}
}

}

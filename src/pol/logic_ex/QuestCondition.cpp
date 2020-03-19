#include "precomp.h"
#include "QuestCondition.h"

namespace PoL
{

//=====================================================================================//
//                          QuestCondition::QuestCondition()                           //
//=====================================================================================//
QuestCondition::QuestCondition(Id_t id, QuestConditionState qcs, HQuestRef quest)
:	Condition(id),
	m_state(qcs),
	m_quest(quest)
{
	m_questConn = m_quest->addFn(boost::bind(onQuestStateChanged,this,_1));
	updateState(m_quest->getState(),false);
}

//=====================================================================================//
//                          QuestCondition::~QuestCondition()                          //
//=====================================================================================//
QuestCondition::~QuestCondition()
{
}

//=====================================================================================//
//                            void QuestCondition::reset()                             //
//=====================================================================================//
void QuestCondition::reset()
{
	Condition::reset();
	updateState(m_quest->getState(),false);
}

//=====================================================================================//
//                     void QuestCondition::onQuestStateChanged()                      //
//=====================================================================================//
void QuestCondition::onQuestStateChanged(QuestState qs)
{
	updateState(qs);
}

//=====================================================================================//
//                         void QuestCondition::updateState()                          //
//=====================================================================================//
void QuestCondition::updateState(QuestState qs, bool ntf)
{
	switch(m_state)
	{
	case qcsCompleted:
		setState(qs == qsCompleted,ntf);
		break;
	case qcsNotStarted:
		setState(!m_quest->isStarted(),ntf);
		break;
	case qcsStarted:
		setState(m_quest->isStarted(),ntf);
		break;
	case qcsPassed:
		setState(m_quest->isPassed(),ntf);
		break;
	case qcsFailed:
		setState(m_quest->isFailed(),ntf);
		break;
	}
}

}
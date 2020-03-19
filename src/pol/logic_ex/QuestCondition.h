#pragma once

#include "QuestRef.h"
#include "Condition.h"
#include "QuestConditionState.h"

namespace PoL
{

//=====================================================================================//
//                       class QuestCondition : public Condition                       //
//=====================================================================================//
class QuestCondition : public Condition
{
	QuestConditionState m_state;
	HQuestRef m_quest;
	QuestRef::Connection m_questConn;

public:
	QuestCondition(Id_t id, QuestConditionState, HQuestRef);
	~QuestCondition();

	/// ���������� ��� �������
	virtual SizerTypes getType() const { return stQuestCond; }

	virtual void reset();

private:
	void updateState(QuestState st, bool ntf = true);
	/// �������� ��������� �������
	void onQuestStateChanged(QuestState st);
};

}
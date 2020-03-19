#pragma once

#include "Quest.h"
#include "QuestState.h"

namespace PoL
{

//=====================================================================================//
//                                   class QuestRef                                    //
//=====================================================================================//
class QuestRef : public MlRefCounted, private NotifyFn< boost::function<void (QuestState)> >
{
	typedef NotifyFn< boost::function<void (QuestState)> > NFn_t;

public:
	using NFn_t::Fn_t;
	using NFn_t::addFn;
	using NFn_t::Connection;

private:
	Quest *m_quest;
	Quest::Connection m_questConn;

public:
	QuestRef();

	/// Установить квест, на который ссылается объект
	void setQuest(Quest *);

	/// Возвращает состояние подконтрольного квеста
	QuestState getState() const
	{
		return m_quest ? m_quest->getState() : qsNotStarted;
	}

	bool isStarted() const
	{
		return m_quest ? m_quest->isStarted() : false;
	}

	bool isPassed() const
	{
		return m_quest ? m_quest->isPassed() : false;
	}

	bool isFailed() const
	{
		return m_quest ? m_quest->isFailed() : false;
	}

private:
	void onQuestStateChanged(QuestState);
};

}

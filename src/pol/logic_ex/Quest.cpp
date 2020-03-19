#include "precomp.h"

#include "Act.h"
#include "Task.h"
#include "Quest.h"
#include "Utils.h"
#include "ActMgr.h"
#include "QuestParams.h"
#include "ActTemplate.h"
#include "TaskDatabase.h"
#include "QuestReportAct.h"
#include "QuestContextAct.h"

/// Включает автоматическое "говорение" квестовых фраз
//#define POL_QUEST_AUTO_SAY

namespace PoL
{

namespace QuestDetail
{

std::string g_stateNames[] = { "NotStarted", "Started", "ReadyToPass", "Passed", "Failed", "TaskDone",
							   "Completed", "ReadyToFail", "ReadyToPassBad", "Destroyed" };

#ifdef POL_QUEST_AUTO_SAY
//=====================================================================================//
//                              class QuestPhrasesReader                               //
//=====================================================================================//
class QuestPhrasesReader
{
	typedef std::hash_map<std::string,QuestPhrases> Phrases_t;
	Phrases_t m_phrases;

	static mll::utils::table::col_type getCol(const mll::utils::table &tbl, 
											  mll::utils::table::row_type r,
											  const std::string &n)
	{
		return tbl.col(std::find(r.begin(),r.end(),n));
	}

public:
	QuestPhrasesReader()
	{
		using mll::utils::table;

		VFileWpr file("scripts/quests/locale.txt");
		VFileBuf buf(file.get());
		std::istream stream(&buf);

		table tbl;
		stream >> tbl;

		table::row_type hdr					= tbl.row(0);
		table::col_type sysname				= getCol(tbl,hdr,"sysname");
		table::col_type locname				= getCol(tbl,hdr,"name");
		table::col_type startMsg			= getCol(tbl,hdr,"started");
		table::col_type completeMsg			= getCol(tbl,hdr,"completed");
		table::col_type complonsMsg			= getCol(tbl,hdr,"completed_on_start");
		table::col_type failonsMsg			= getCol(tbl,hdr,"failed_on_start");
		table::col_type inProgrMsg			= getCol(tbl,hdr,"inprogress");
		table::col_type failedMsg			= getCol(tbl,hdr,"failed");
		table::col_type startMsgSound		= getCol(tbl,hdr,"started_sound");
		table::col_type completeMsgSound	= getCol(tbl,hdr,"completed_sound");
		table::col_type complonsMsgSound	= getCol(tbl,hdr,"completed_on_start_sound");
		table::col_type failonsMsgSound		= getCol(tbl,hdr,"failed_on_start_sound");
		table::col_type inProgrMsgSound		= getCol(tbl,hdr,"inprogress_sound");
		table::col_type failedMsgSound		= getCol(tbl,hdr,"failed_sound");
		table::col_type journalMsg			= getCol(tbl,hdr,"journal");

		for(int i = 1; i < tbl.height(); ++i)
		{
			std::string quest = sysname[i];
			std::transform(quest.begin(),quest.end(),quest.begin(),std::tolower);
			QuestPhrases phrases;

			phrases.m_locname = locname[i];
			phrases.m_journal = journalMsg[i];
			phrases.m_onStart = phrase_s(startMsg[i],startMsgSound[i]);
			phrases.m_onFailed = phrase_s(failedMsg[i],failedMsgSound[i]);
			phrases.m_onCompleted = phrase_s(completeMsg[i],completeMsgSound[i]);
			phrases.m_onRemind = phrase_s(inProgrMsg[i],inProgrMsgSound[i]);
			phrases.m_onCompletedOnStart = phrase_s(complonsMsg[i],complonsMsgSound[i]);
			phrases.m_onFailedOnStart = phrase_s(failonsMsg[i],failonsMsgSound[i]);

			m_phrases.insert(std::make_pair(quest,phrases));
		}
	}

	QuestPhrases getQuestPhrases(const std::string &name) const
	{
		Phrases_t::const_iterator i = m_phrases.find(name);
		if(i == m_phrases.end()) return QuestPhrases();
		return i->second;
	}
};
#endif

}

using namespace QuestDetail;

Quest::StateChange Quest::m_stateChanges[MaxStateChanges] =
{
	{qsNotStarted,qsStarted,onStartQuest},
	{qsTaskDone,qsCompleted,onQuestCompleted},
	{qsReadyToPass,qsTaskDone,onCompletedOnStart},
	{qsReadyToFail,qsFailed,onQuestFailed},
	{qsReadyToPassBad,qsPassed,onFailedOnStart},
	{qsNotStarted,qsNotStarted,0}
};

//=====================================================================================//
//                                   Quest::Quest()                                    //
//=====================================================================================//
Quest::Quest(Id_t id, const QuestParams &qp)
:	Persistent(id),
	m_enabled(qp.m_enabled),
	m_state(qsNotStarted),
	m_initiallyEnabled(qp.m_enabled),
	m_onStartAct(qp.m_onStart),
	m_onFailAct(qp.m_onFail),
	m_onSuccessAct(qp.m_onSucceed),
	m_onRemindAct(qp.m_onRemind),
	m_onComplonsAct(qp.m_onCompletedOnStart),
	m_onFailonsAct(qp.m_onFailedOnStart),
	m_onPreSucceedAct(qp.m_onPreSucceed),
	m_holder(qp.m_holder),
	m_remind(qp.m_remind),
	m_dependency(qp.m_dependency),
	m_complonsPlayed(false),
	m_cannotRemind(-1)
{
#ifdef POL_QUEST_AUTO_SAY
	static QuestPhrasesReader qpr;
	m_phrases = qpr.getQuestPhrases(getName());
#endif

	m_tasks[ttStart] = qp.m_start;
	m_tasks[ttQuest] = qp.m_quest;
	m_tasks[ttEnd] = qp.m_end;

	if(m_tasks[ttStart])
	{
		m_startConn = m_tasks[ttStart]->addFn(boost::bind(onStartTaskChanged,this,_1));
		m_tasks[ttStart]->setPriority(4);
	}

	if(m_tasks[ttEnd])
	{
		m_endConn = m_tasks[ttEnd]->addFn(boost::bind(onEndTaskChanged,this,_1));
		m_tasks[ttEnd]->setPriority(0);
	}

	if(m_tasks[ttQuest])
	{
		m_questConn = m_tasks[ttQuest]->addFn(boost::bind(onQuestTaskChanged,this,_1));
		m_tasks[ttQuest]->setPriority(2);

		if(m_tasks[ttQuest]->getPreCondition())
		{
			m_preCondConn = m_tasks[ttQuest]->getPreCondition()->addFn(boost::bind(onPreCondition,this,_1));
			m_tasks[ttQuest]->getPreCondition()->setPriority(3);
		}
	}

	if(m_remind)
	{
		m_remindConn = m_remind->addFn(boost::bind(onRemindChanged,this,_1));
		m_remind->setPriority(1);
	}

	if(m_dependency)
	{
		m_dependsConn = m_dependency->addFn(boost::bind(onDependencyChanged,this,_1));
		m_dependency->setPriority(5);
		enable(m_dependency->getKnownState());
	}

	resetState();
}

//=====================================================================================//
//                                   Quest::~Quest()                                   //
//=====================================================================================//
Quest::~Quest()
{
}

//=====================================================================================//
//                              void Quest::resetState()                               //
//=====================================================================================//
void Quest::resetState()
{
	m_state = qsNotStarted;

	for(int i = 0; i < ttCount; ++i)
	{
		m_taskStates[i] = m_tasks[i] ? m_tasks[i]->getKnownState() : tsCompleted;
	}

	adjustTasks();
	promoteState();
}

//=====================================================================================//
//                              void Quest::sendMessage()                              //
//=====================================================================================//
void Quest::sendMessage(QuestMessage qm)
{
	TaskType task;
	TaskState target;
	bool enableEnd = false;

	switch(qm)
	{
	case qmTaskCompleted:
		target = tsCompleted;
		task = ttQuest;
		if(m_tasks[ttEnd]) m_tasks[ttEnd]->enable(true);
		break;
	case qmTaskFailed:
		target = tsFailed;
		task = ttQuest;
		if(m_tasks[ttEnd]) m_tasks[ttEnd]->enable(true);
		break;
	case qmEndTaskCompleted:
		target = tsCompleted;
		task = ttEnd;
		break;
	case qmEndTaskFailed:
		target = tsFailed;
		task = ttEnd;
		break;
	case qmStartTaskCompleted:
		target = tsCompleted;
		task = ttStart;
		if(m_remind) m_remind->enable(true);
		break;
	case qmStartTaskFailed:
		target = tsFailed;
		task = ttStart;
		break;
	}

	if(target != tsCompleted || m_taskStates[task] != tsFailed)
	{
		m_taskStates[task] = target;
		promoteState();
	}
}

//=====================================================================================//
//                             void Quest::promoteState()                              //
//=====================================================================================//
void Quest::promoteState()
{
	QuestState state = m_state;

	if(isEnabled())
	{
		// Конечный автомат зашит здесь. Подробности смотри в quests.vsd
		switch(m_state)
		{
		case qsNotStarted:
			if(failed(ttQuest)) changeState(qsReadyToPassBad);
			else if(failed(ttStart) || failed(ttEnd)) changeState(qsPassed);
			else if(completed(ttStart)) changeState(qsStarted);
			else if(completed(ttQuest)) changeState(qsReadyToPass);
			break;
		case qsStarted:
			if(failed(ttQuest)) changeState(qsReadyToFail);
			else if(failed(ttEnd)) changeState(qsFailed);
			//else if(failed(ttEnd)) changeState(qsUncompletable);
			else if(completed(ttQuest)) changeState(qsTaskDone);
			break;
		case qsReadyToPass:
			if(failed(ttEnd) || failed(ttStart)) changeState(qsPassed);
			else if(completed(ttStart)) changeState(qsTaskDone);
			break;
		case qsPassed:
			// Конечное состояние
			break;
		case qsFailed:
			// Конечное состояние
			break;
		case qsTaskDone:
			if(failed(ttEnd)) changeState(qsFailed);
			else if(completed(ttEnd)) changeState(qsCompleted);
			break;
		case qsReadyToFail:
			if(failed(ttEnd)) changeState(qsPassed);
			else if(completed(ttEnd)) changeState(qsFailed);
			break;
		case qsReadyToPassBad:
			if(completed(ttEnd)) changeState(qsPassed);
			break;
		case qsCompleted:
			// Конечное состояние
			break;
		}

		if(m_state != state) // Состояние квеста изменилось
		{
			promoteState(); // Возможно, нужно продвигать состояние дальше
		}
	}
}

//=====================================================================================//
//                           bool Quest::validState() const                            //
//=====================================================================================//
bool Quest::validState(QuestState s) const
{
	switch(m_state)
	{
	case qsStarted:
		return completed(ttStart);
	case qsReadyToPass:
		return completed(ttQuest);
	case qsFailed:
		return completed(ttStart) && (failed(ttQuest) || failed(ttEnd));
	case qsTaskDone:
		return completed(ttStart) && completed(ttQuest);
	case qsCompleted:
		return completed(ttStart) && completed(ttQuest) && completed(ttEnd);
	case qsPassed:
		return failed(ttStart) || failed(ttEnd) || failed(ttQuest);
	}

	return true;
}

//=====================================================================================//
//                        Quest::Fn_t Quest::getStateChangeFn()                        //
//=====================================================================================//
Quest::StateFn_t Quest::getStateChangeFn(QuestState start, QuestState end)
{
	for(int i = 0; i < MaxStateChanges && m_stateChanges[i].m_fn; ++i)
	{
		if(m_stateChanges[i].m_from == start && m_stateChanges[i].m_to == end)
		{
			return m_stateChanges[i].m_fn;
		}
	}

	return 0;
}

//=====================================================================================//
//                              void Quest::changeState()                              //
//=====================================================================================//
void Quest::changeState(QuestState newState)
{
	assert( m_state != newState );
	assert( validState(newState) );
	StateFn_t fn = getStateChangeFn(m_state,newState);

	ACT_LOG("INFO: quest [" << getName() << "] changed state from "
		<< g_stateNames[m_state] << " to " << g_stateNames[newState] << "\n");

	m_state = newState;

	if(fn) (this->*fn)();
	NFn_t::notify(m_state);
}

//=====================================================================================//
//                       BaseEntity *Quest::getHolderPtr() const                       //
//=====================================================================================//
BaseEntity *Quest::getHolderPtr() const
{
	return getEntityByName(m_holder);
}

//=====================================================================================//
//                              HAct Quest::makeContext()                              //
//=====================================================================================//
HAct Quest::makeContext(HAct act)
{
	if(TaskDatabase::instance()->isProcessingMessage())
	{
		act = new QuestContextAct(act,m_holder,TaskDatabase::instance()->getMessageInProcess().m_object);
	}

	return act = new QuestReportAct(act);
}

//=====================================================================================//
//                             void Quest::onStartQuest()                              //
//=====================================================================================//
void Quest::onStartQuest()
{
	ACT_LOG("INFO: quest [" << getName() << "] notified about StartQuest\n");

	if(!m_phrases.m_onStart.m_text.empty())
	{
		::Forms::GetInst()->ShowTalkDialog(getHolderPtr(),m_phrases.m_onStart);
	}

	if(m_onStartAct)
	{
		ActMgr::start(makeContext(m_onStartAct->createAct()));
	}

	if(TaskDatabase::instance()->isProcessingMessage())
	{
		m_cannotRemind = TaskDatabase::instance()->getMessageCount();
	}
}

//=====================================================================================//
//                           void Quest::onQuestCompleted()                            //
//=====================================================================================//
void Quest::onQuestCompleted()
{
	ACT_LOG("INFO: quest [" << getName() << "] notified about QuestCompleted\n");

	if(!m_complonsPlayed)
	{
#ifdef POL_QUEST_AUTO_SAY
		if(!m_phrases.m_onCompleted.m_text.empty())
		{
			::Forms::GetInst()->ShowTalkDialog(getHolderPtr(),m_phrases.m_onCompleted);
		}
#endif

		if(m_onSuccessAct)
		{
			ActMgr::start(makeContext(m_onSuccessAct->createAct()));
		}
	}
}

//=====================================================================================//
//                          void Quest::onCompletedOnStart()                           //
//=====================================================================================//
void Quest::onCompletedOnStart()
{
	ACT_LOG("INFO: quest [" << getName() << "] notified about QuestCompletedOnStart\n");

#ifdef POL_QUEST_AUTO_SAY
	if(!m_phrases.m_onCompletedOnStart.m_text.empty())
	{
		::Forms::GetInst()->ShowTalkDialog(getHolderPtr(),m_phrases.m_onCompletedOnStart);
	}
#endif

	if(m_onComplonsAct)
	{
		ActMgr::start(makeContext(m_onComplonsAct->createAct()));
	}

	m_complonsPlayed = true;
}

//=====================================================================================//
//                            void Quest::onFailedOnStart()                            //
//=====================================================================================//
void Quest::onFailedOnStart()
{
	ACT_LOG("INFO: quest [" << getName() << "] notified about QuestFailedOnStart\n");

#ifdef POL_QUEST_AUTO_SAY
	if(!m_phrases.m_onFailedOnStart.m_text.empty())
	{
		::Forms::GetInst()->ShowTalkDialog(getHolderPtr(),m_phrases.m_onFailedOnStart);
	}
#endif

	if(m_onFailonsAct)
	{
		ActMgr::start(makeContext(m_onFailonsAct->createAct()));
	}

	m_complonsPlayed = true;
}

//=====================================================================================//
//                             void Quest::onRemindQuest()                             //
//=====================================================================================//
void Quest::onRemindQuest()
{
	ACT_LOG("INFO: quest [" << getName() << "] notified about QuestRemind\n");
 
	if(   !TaskDatabase::instance()->isProcessingMessage()
	   ||  m_cannotRemind != TaskDatabase::instance()->getMessageCount())
	{
#ifdef POL_QUEST_AUTO_SAY
		if(!m_phrases.m_onRemind.m_text.empty())
		{
			::Forms::GetInst()->ShowTalkDialog(getHolderPtr(),m_phrases.m_onRemind);
		}
#endif

		if(m_onRemindAct)
		{
			ActMgr::start(makeContext(m_onRemindAct->createAct()));
		}
	}
}

//=====================================================================================//
//                             void Quest::onQuestFailed()                             //
//=====================================================================================//
void Quest::onQuestFailed()
{
	ACT_LOG("INFO: quest [" << getName() << "] notified about QuestFailed\n");

#ifdef POL_QUEST_AUTO_SAY
	if(!m_phrases.m_onFailed.m_text.empty())
	{
		::Forms::GetInst()->ShowTalkDialog(getHolderPtr(),m_phrases.m_onFailed);
	}
#endif

	if(m_onFailAct)
	{
		ActMgr::start(makeContext(m_onFailAct->createAct()));
	}
}

//=====================================================================================//
//                             void Quest::onPreSuccess()                              //
//=====================================================================================//
void Quest::onPreSuccess()
{
	ACT_LOG("INFO: quest [" << getName() << "] notified about PreSuccess\n");

	if(TaskDatabase::instance()->isProcessingMessage())
	{
		m_cannotRemind = TaskDatabase::instance()->getMessageCount();
	}

	if(m_onPreSucceedAct)
	{
		HAct pscd = makeContext(m_onPreSucceedAct->createAct());

		ActMgr::start(pscd);
		while(!pscd->isEnded())
		{
			ActMgr::play();
		}
	}
}

//=====================================================================================//
//                           void Quest::onEndTaskChanged()                            //
//=====================================================================================//
void Quest::onEndTaskChanged(TaskState ts)
{
	switch(ts)
	{
	case tsFailed: sendMessage(qmEndTaskFailed); break;
	case tsCompleted: sendMessage(qmEndTaskCompleted); break;
	}
}

//=====================================================================================//
//                          void Quest::onStartTaskChanged()                           //
//=====================================================================================//
void Quest::onStartTaskChanged(TaskState ts)
{
	switch(ts)
	{
	case tsFailed: sendMessage(qmStartTaskFailed); break;
	case tsCompleted: sendMessage(qmStartTaskCompleted); break;
	}
}

//=====================================================================================//
//                          void Quest::onQuestTaskChanged()                           //
//=====================================================================================//
void Quest::onQuestTaskChanged(TaskState ts)
{
	switch(ts)
	{
	case tsFailed: sendMessage(qmTaskFailed); break;
	case tsCompleted: sendMessage(qmTaskCompleted); break;
	}
}

//=====================================================================================//
//                            void Quest::onRemindChanged()                            //
//=====================================================================================//
void Quest::onRemindChanged(bool val)
{
	if(val && completed(ttStart) && notCompleted(ttQuest))
	{
		onRemindQuest();
	}
}

//=====================================================================================//
//                            void Quest::onPreCondition()                             //
//=====================================================================================//
void Quest::onPreCondition(bool val)
{
	if(val && completed(ttStart) && notCompleted(ttQuest))
	{
		onPreSuccess();
	}
}

//=====================================================================================//
//                          void Quest::onDependencyChanged()                          //
//=====================================================================================//
void Quest::onDependencyChanged(bool val)
{
	if(!isStarted() || val)
	{
		enable(val);
	}
}

//=====================================================================================//
//                              void Quest::store() const                              //
//=====================================================================================//
void Quest::store(mll::io::obinstream &out) const
{
	out << asInt(m_enabled) << asInt(m_state);
	for(int i = 0; i < ttCount; ++i)
	{
		out << asInt(m_taskStates[i]);
	}
	out << asInt(m_complonsPlayed);
}

//=====================================================================================//
//                                void Quest::restore()                                //
//=====================================================================================//
void Quest::restore(mll::io::ibinstream &in)
{
	in >> asInt(m_enabled) >> asInt(m_state);
	for(int i = 0; i < ttCount; ++i)
	{
		in >> asInt(m_taskStates[i]);
	}
	in >> asInt(m_complonsPlayed);
}

//=====================================================================================//
//                              void Quest::resetTasks()                               //
//=====================================================================================//
void Quest::resetTasks()
{
	for(int i = 0; i != ttCount; ++i)
	{
		if(m_tasks[i]) m_tasks[i]->taskReset();
	}
}

//=====================================================================================//
//                                 void Quest::reset()                                 //
//=====================================================================================//
void Quest::reset()
{
	if(m_dependency) m_enabled = m_dependency->getState();
	m_enabled = m_enabled && m_initiallyEnabled;
	m_complonsPlayed = false;
	resetState();
}

//=====================================================================================//
//                              void Quest::adjustTasks()                              //
//=====================================================================================//
void Quest::adjustTasks()
{
	if(m_enabled)
	{
		if(m_tasks[ttStart]) m_tasks[ttStart]->enable(true);

		if(m_remind) m_remind->enable(completed(ttStart) && notCompleted(ttQuest));

		if(m_tasks[ttQuest])
		{
			m_tasks[ttQuest]->enable(true);
			if(m_tasks[ttEnd])
			{
				m_tasks[ttEnd]->enable(!notCompleted(ttQuest));
			}
		}
		else
		{
			if(m_tasks[ttEnd]) m_tasks[ttEnd]->enable(true);
		}
	}
	else
	{
		if(m_tasks[ttStart]) m_tasks[ttStart]->enable(false);
		if(m_tasks[ttEnd]) m_tasks[ttEnd]->enable(false);
		if(m_tasks[ttQuest]) m_tasks[ttQuest]->enable(false);
		if(m_remind) m_remind->enable(false);
	}
}

//=====================================================================================//
//                                void Quest::enable()                                 //
//=====================================================================================//
void Quest::enable(bool v)
{
	if(m_enabled != v)
	{
		m_enabled = v;
		adjustTasks();
		promoteState();
	}
}

//=====================================================================================//
//                            void Quest::dumpState() const                            //
//=====================================================================================//
void Quest::dumpState(std::ostream &out) const
{
	out << "quest [" << getName() << "]\n"
		<< "\tenabled[" << (isEnabled()?"true":"false")
		<< "] state[" << g_stateNames[getState()] << "]\n";
}

}
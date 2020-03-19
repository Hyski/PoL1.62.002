#include "precomp.h"
#include "TalkTask.h"

namespace PoL
{

//=====================================================================================//
//                 TalkTask::TalkTask(Id_t id, const std::string &subj                 //
//=====================================================================================//
TalkTask::TalkTask(Id_t id, const std::string &subj)
:	Task(id),
	m_subj(subj),
	m_noFail(false)
{
	m_talkConn = TaskDatabase::instance()->attach(boost::bind(onSubjectTalk,this),m_subj,TaskDatabase::mtPersonUsed);
	m_deadConn = TaskDatabase::instance()->attach(boost::bind(onSubjectDead,this),m_subj,TaskDatabase::mtDead);
}

//=====================================================================================//
//                                TalkTask::TalkTask()                                 //
//=====================================================================================//
TalkTask::TalkTask(Id_t id, const std::string &subj, bool noFail)
:	Task(id),
	m_subj(subj),
	m_noFail(noFail)
{
	m_talkConn = TaskDatabase::instance()->attach(boost::bind(onSubjectTalk,this),m_subj,TaskDatabase::mtPersonUsed);
	if(!m_noFail)
	{
		m_deadConn = TaskDatabase::instance()->attach(boost::bind(onSubjectDead,this),m_subj,TaskDatabase::mtDead);
	}
}

//=====================================================================================//
//                              void TalkTask::doEnable()                              //
//=====================================================================================//
void TalkTask::doEnable(bool v)
{
	m_talkConn.enable(v);
}

//=====================================================================================//
//                            void TalkTask::setPriority()                             //
//=====================================================================================//
void TalkTask::setPriority(int p)
{
	m_talkConn.setPriority(p);
}

//=====================================================================================//
//                           void TalkTask::onSubjectDead()                            //
//=====================================================================================//
void TalkTask::onSubjectDead()
{
	if(getState() == tsNotCompleted) setState(tsFailed);
}

//=====================================================================================//
//                           void TalkTask::onSubjectTalk()                            //
//=====================================================================================//
void TalkTask::onSubjectTalk()
{
	if(isEnabled())
	{
		setState(tsCompleted);
	}
}

}
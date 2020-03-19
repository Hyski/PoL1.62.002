#include "precomp.h"
#include "KillTask.h"

namespace PoL
{

//=====================================================================================//
//                                KillTask::KillTask()                                 //
//=====================================================================================//
KillTask::KillTask(Id_t id, const std::string &subj, bool dead)
:	Task(id),
	m_dead(dead),
	m_subj(subj)
{
	if(!m_dead) m_killConn = TaskDatabase::instance()->attach(boost::bind(onSubjectKilled,this),m_subj,TaskDatabase::mtKilled);
	m_deadConn = TaskDatabase::instance()->attach(boost::bind(onSubjectDead,this),m_subj,TaskDatabase::mtDead);
}

//=====================================================================================//
//                          void KillTask::onSubjectKilled()                           //
//=====================================================================================//
void KillTask::onSubjectKilled()
{
	setState(tsCompleted);
}

//=====================================================================================//
//                            void KillTask::setPriority()                             //
//=====================================================================================//
void KillTask::setPriority(int p)
{
	m_killConn.setPriority(p);
	m_deadConn.setPriority(p);
}

//=====================================================================================//
//                           void KillTask::onSubjectDead()                            //
//=====================================================================================//
void KillTask::onSubjectDead()
{
	if(getState() != tsCompleted)
	{
		if(m_dead)
		{
			setState(tsCompleted);
		}
		else
		{
			setState(tsFailed);
		}
	}
}

}
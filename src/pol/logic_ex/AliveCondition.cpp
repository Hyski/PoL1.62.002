#include "precomp.h"
#include "AliveCondition.h"

namespace PoL
{

//=====================================================================================//
//                           TaskCondition::TaskCondition()                            //
//=====================================================================================//
AliveCondition::AliveCondition(Id_t id, const std::string &who, bool alive)
:	Condition(id),
	m_who(who),
	m_alive(alive)
{
	m_deadConn = TaskDatabase::instance()->attach(boost::bind(onDeadSubject,this),m_who,TaskDatabase::mtDead);
	setState(m_alive,false);
}

//=====================================================================================//
//                          void TaskCondition::setPriority()                          //
//=====================================================================================//
void AliveCondition::setPriority(int p)
{
	m_deadConn.setPriority(p);
}

//=====================================================================================//
//                             void TaskCondition::reset()                             //
//=====================================================================================//
void AliveCondition::reset()
{
	setState(m_alive,false);
}

//=====================================================================================//
//                         void TaskCondition::onTaskChanged()                         //
//=====================================================================================//
void AliveCondition::onDeadSubject()
{
	setState(!m_alive);
}

}

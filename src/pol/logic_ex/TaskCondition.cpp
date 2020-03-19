#include "precomp.h"
#include "TaskCondition.h"

namespace PoL
{

//=====================================================================================//
//                           TaskCondition::TaskCondition()                            //
//=====================================================================================//
TaskCondition::TaskCondition(Id_t id, HTask task, TaskState state)
:	Condition(id),
	m_task(task),
	m_reqState(state)
{
	m_taskConn = m_task->addFn(boost::bind(onTaskChanged,this,_1));
	updateState(false);
}

//=====================================================================================//
//                          void TaskCondition::setPriority()                          //
//=====================================================================================//
void TaskCondition::setPriority(int p)
{
	m_task->setPriority(p);
}

//=====================================================================================//
//                             void TaskCondition::reset()                             //
//=====================================================================================//
void TaskCondition::reset()
{
	updateState(false);
}

//=====================================================================================//
//                          void TaskCondition::updateState()                          //
//=====================================================================================//
void TaskCondition::updateState(bool ntf)
{
	updateState(m_task->getKnownState(),ntf);
}

//=====================================================================================//
//                          void TaskCondition::updateState()                          //
//=====================================================================================//
void TaskCondition::updateState(TaskState ts, bool ntf)
{
	setState(ts == m_reqState,ntf);
}

//=====================================================================================//
//                         void TaskCondition::onTaskChanged()                         //
//=====================================================================================//
void TaskCondition::onTaskChanged(TaskState ts)
{
	updateState(ts);
}

}

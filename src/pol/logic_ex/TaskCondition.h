#pragma once

#include "Task.h"
#include "Condition.h"

namespace PoL
{

//=====================================================================================//
//                       class TaskCondition : public Condition                        //
//=====================================================================================//
class TaskCondition : public Condition
{
	TaskState m_reqState;
	HTask m_task;
	Task::Connection m_taskConn;

public:
	TaskCondition(Id_t, HTask, TaskState);

	/// Возвращает тип объекта
	virtual SizerTypes getType() const { return stTaskCond; }

	virtual void setPriority(int);
	virtual void reset();

private:
	void updateState(bool ntf = true);
	void updateState(TaskState ts, bool ntf = true);
	void onTaskChanged(TaskState);
};

}

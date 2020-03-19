#pragma once

#include "Task.h"

namespace PoL
{

//=====================================================================================//
//                            class TaskList : public Task                             //
//=====================================================================================//
class TaskList : public Task
{
	typedef std::vector<HTask> Tasks_t;
	typedef std::vector<TaskState> TaskStates_t;
	typedef std::vector< boost::shared_ptr<Task::Connection> > TaskConns_t;

	Tasks_t m_tasks;
	TaskConns_t m_taskConns;
	TaskStates_t m_taskStates;
	unsigned int m_counts[tsCount];

public:
	template<typename T> TaskList(Id_t id, T b, T e);
	~TaskList();

	/// Возвращает тип объекта
	virtual SizerTypes getType() const { return stTaskList; }
	/// Возвращает количество подзадач
	virtual int getSubTaskCount() const { return m_tasks.size(); }

	/// Установить приоритет задачи
	virtual void setPriority(int);

	/// Сохранить состояние объекта
	virtual void store(mll::io::obinstream &) const;
	/// Восстановить состояние объекта
	virtual void restore(mll::io::ibinstream &);

	virtual HCondition getPreCondition() const;

	/// Сбросить состояние на начальное
	virtual void reset();
	virtual void taskReset();

private:
	/// Включить/выключить задачи из списка
	virtual void doEnable(bool);

	/// Установить состояние задачи в соответствии с m_counts
	void applyState(bool notify=true);
	/// Инициализировать состояние задачи
	void initState();
	/// Вызовется при изменении состояния задачи
	void onTaskStateChanged(int i, TaskState ts);
};

//=====================================================================================//
//                                TaskList::TaskList()                                 //
//=====================================================================================//
template<typename T>
TaskList::TaskList(Id_t id, T b, T e)
:	Task(id),
	m_tasks(b,e)
{
	initState();
	applyState();
}

}
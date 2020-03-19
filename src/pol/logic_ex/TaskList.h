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

	/// ���������� ��� �������
	virtual SizerTypes getType() const { return stTaskList; }
	/// ���������� ���������� ��������
	virtual int getSubTaskCount() const { return m_tasks.size(); }

	/// ���������� ��������� ������
	virtual void setPriority(int);

	/// ��������� ��������� �������
	virtual void store(mll::io::obinstream &) const;
	/// ������������ ��������� �������
	virtual void restore(mll::io::ibinstream &);

	virtual HCondition getPreCondition() const;

	/// �������� ��������� �� ���������
	virtual void reset();
	virtual void taskReset();

private:
	/// ��������/��������� ������ �� ������
	virtual void doEnable(bool);

	/// ���������� ��������� ������ � ������������ � m_counts
	void applyState(bool notify=true);
	/// ���������������� ��������� ������
	void initState();
	/// ��������� ��� ��������� ��������� ������
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
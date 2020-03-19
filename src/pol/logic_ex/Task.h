#pragma once

#include "NotifyFn.h"
#include "TaskState.h"
#include "Condition.h"
#include "Persistent.h"

namespace PoL
{

//=====================================================================================//
//                          class Task : public MlRefCounted                           //
//=====================================================================================//
class Task : public Persistent, private NotifyFn< boost::function<void (TaskState)> >
{
	typedef NotifyFn< boost::function<void (TaskState)> > NFn_t;

public:
	using NFn_t::Fn_t;
	using NFn_t::addFn;
	using NFn_t::Connection;

private:
	TaskState m_state;
	TaskState m_knownState;
	bool m_enabled;

public:
	Task(Id_t id) : Persistent(id), m_state(tsNotCompleted), m_knownState(tsNotCompleted), m_enabled(true) {}
	Task(Id_t id, bool e) : Persistent(id), m_state(tsNotCompleted), m_knownState(tsNotCompleted), m_enabled(e) {}
	virtual ~Task() {}

	/// Возвращает состояние задачи
	TaskState getState() const { return m_state; }	

	/// Возвращает состояние задачи "для клиента"
	TaskState getKnownState() const { return m_knownState; }

	virtual HCondition getPreCondition() const { return 0; }

	/// Установить приоритет задачи
	virtual void setPriority(int) {}

	/// Сохранить состояние объекта
	virtual void store(mll::io::obinstream &) const;
	/// Восстановить состояние объекта
	virtual void restore(mll::io::ibinstream &);

	/// Сбросить состояние на начальное
	virtual void reset();
	virtual void taskReset();

	virtual void dumpState(std::ostream &out) const
	{
		const char *statedescs[] = {"not completed", "completed", "failed"};

		out << "task [" << getName() << "]\n"
			<< "\tstate[" << statedescs[getKnownState()]
			<< "] internal-state[" << statedescs[getState()]
			<< "] enabled[" << (isEnabled()?"true":"false") << "]\n";
	}

	/// Включить/выключить задачу
	void enable(bool v);

	bool isEnabled() const { return m_enabled; }

protected:
	/// Установить состояние задачи
	void setState(TaskState ts, bool ntf = true)
	{
		if(ts != m_state)
		{
			if(isEnabled() && ntf && m_knownState != ts)
			{
				m_knownState = ts;
				notify();
			}
			m_state = ts;
		}
	}

private:
	virtual void doEnable(bool v) {}

	void notify()
	{
		NFn_t::notify(m_knownState);
	}
};

}
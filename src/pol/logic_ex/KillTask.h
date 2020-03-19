#pragma once

#include "Task.h"
#include "TaskDatabase.h"

namespace PoL
{

//=====================================================================================//
//                            class KillTask : public Task                             //
//=====================================================================================//
class KillTask : public Task
{
	bool m_dead;
	std::string m_subj;
	TaskDatabase::Connection m_killConn, m_deadConn;

public:
	KillTask(Id_t, const std::string &subj, bool dead);

	/// Возвращает тип объекта
	virtual SizerTypes getType() const { return stKillTask; }

	/// Установить приоритет задачи
	virtual void setPriority(int);

	virtual void store(mll::io::obinstream &out) const { Task::store(out); }
	virtual void restore(mll::io::ibinstream &in) { Task::restore(in); }
	virtual void reset() { Task::reset(); }

private:
	void onSubjectKilled();
	void onSubjectDead();
};

}

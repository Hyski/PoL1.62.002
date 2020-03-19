#pragma once

#include "Task.h"
#include "TaskDatabase.h"

namespace PoL
{

//=====================================================================================//
//                            class TalkTask : public Task                             //
//=====================================================================================//
class TalkTask : public Task
{
	std::string m_subj;
	TaskDatabase::Connection m_talkConn, m_deadConn;
	bool m_noFail;

public:
	TalkTask(Id_t, const std::string &subj);
	TalkTask(Id_t, const std::string &subj, bool noFail);

	/// Возвращает тип объекта
	virtual SizerTypes getType() const { return stTalkTask; }

	/// Установить приоритет задачи
	virtual void setPriority(int);

	virtual void store(mll::io::obinstream &out) const { Task::store(out); }
	virtual void restore(mll::io::ibinstream &in) { Task::restore(in); }
	virtual void reset() { Task::reset(); }

private:
	virtual void doEnable(bool v);

	void onSubjectTalk();
	void onSubjectDead();
};

}
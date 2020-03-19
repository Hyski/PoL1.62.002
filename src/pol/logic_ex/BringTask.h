#pragma once

#include "Task.h"
#include "Signal.h"
#include "TaskDatabase.h"

namespace PoL
{

//=====================================================================================//
//                            class BringTask : public Task                            //
//=====================================================================================//
class BringTask : public Task
{
	HCondition m_signal;
	std::string m_itset;
	std::string m_person;
	TaskDatabase::Connection m_talkConn, m_deadConn;
	bool m_noFail;

public:
	BringTask(Id_t id, const std::string &itset, const std::string &person, HCondition);
	BringTask(Id_t id, const std::string &itset, const std::string &person, HCondition, bool noFail);

	/// Возвращает тип объекта
	virtual SizerTypes getType() const { return stBringTask; }

	/// Установить приоритет задачи
	virtual void setPriority(int);

	virtual void store(mll::io::obinstream &out) const;
	virtual void restore(mll::io::ibinstream &in);
	virtual void reset();

	virtual HCondition getPreCondition() const { return m_signal; }

private:
	virtual void doEnable(bool v);

	void onSubjectTalk();
	void onSubjectDead();
};

//=====================================================================================//
//                          class BringSignal : public Signal                          //
//=====================================================================================//
class BringSignal : public Signal
{
	std::string m_itset, m_person;
	TaskDatabase::Connection m_talkConn;

public:
	BringSignal(Id_t id, const std::string &itset, const std::string &person);

	/// Возвращает тип объекта
	virtual SizerTypes getType() const { return stBringSignal; }

	/// Установить приоритет
	virtual void setPriority(int);

	/// Сохранить состояние объекта
	virtual void store(mll::io::obinstream &out) const { Signal::store(out); }
	/// Восстановить состояние объекта
	virtual void restore(mll::io::ibinstream &in) { Signal::restore(in); }

private:
	virtual void doEnable(bool);
	/// Вызывается при возникновении события
	void onSignal();
};

}
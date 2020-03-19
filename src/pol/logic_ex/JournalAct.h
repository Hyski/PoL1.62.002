#pragma once

#include "Act.h"

namespace PoL
{

//=====================================================================================//
//                            class JournalAct : public Act                            //
//=====================================================================================//
class JournalAct : public Act
{
	std::string m_jid, m_hdr, m_strid;
	bool m_add;

	bool m_started;
	bool m_ended;

public:
	JournalAct(const std::string &jid, const std::string &hdr, const std::string &strid, bool);

	/// Закончить акт, применив все его побочные эффекты
	virtual void skip() {}
	/// Начался ли акт
	virtual bool isStarted() const { return m_started; }
	/// Начать акт
	virtual void start();
	/// Дать управление акту, чтобы он изменил игровую ситуацию
	virtual void update(float tau);
	/// Возвращает true, если акт закончен
	virtual bool isEnded() const { return m_ended; }
	/// Может ли действие закончиться
	virtual bool isEndable() const { return true; }

private:
	void addRecord();
	void removeRecord();
};

}

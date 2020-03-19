#pragma once

#include "Signal.h"
#include "TaskDatabase.h"

namespace PoL
{

//=====================================================================================//
//                       class LeaveLevelSignal : public Signal                        //
//=====================================================================================//
class LeaveLevelSignal : public Signal
{
	std::string m_from, m_to;
	TaskDatabase::Connection m_sigConn;

public:
	LeaveLevelSignal(Id_t id, const std::string &from);
	LeaveLevelSignal(Id_t id, const std::string &from, const std::string &to);

	/// Возвращает тип объекта
	virtual SizerTypes getType() const { return stLeaveLevelSignal; }

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

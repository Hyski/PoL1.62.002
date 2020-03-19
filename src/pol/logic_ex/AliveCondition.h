#pragma once

#include "Condition.h"
#include "TaskDatabase.h"

namespace PoL
{

//=====================================================================================//
//                       class AliveCondition : public Condition                       //
//=====================================================================================//
class AliveCondition : public Condition
{
	std::string m_who;
	TaskDatabase::Connection m_deadConn;
	bool m_alive;

public:
	AliveCondition(Id_t, const std::string &, bool = true);

	/// Возвращает тип объекта
	virtual SizerTypes getType() const { return stAliveCond; }

	virtual void setPriority(int);
	virtual void reset();

private:
	void onDeadSubject();
};

}

#pragma once

#include "Condition.h"

namespace PoL
{

//=====================================================================================//
//                        class AndCondition : public Condition                        //
//=====================================================================================//
class AndCondition : public Condition
{
	HCondition m_left, m_right;
	Condition::Connection m_leftConn, m_rightConn;

public:
	AndCondition(Id_t, HCondition left, HCondition right);

	/// Возвращает тип объекта
	virtual SizerTypes getType() const { return stAndCond; }

	virtual void reset();

private:
	void updateState(bool ntf = true);
	void onConditionChanged();
};

}

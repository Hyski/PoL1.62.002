#pragma once

#include "Condition.h"

namespace PoL
{

//=====================================================================================//
//                        class OrCondition : public Condition                         //
//=====================================================================================//
class OrCondition : public Condition
{
	HCondition m_left, m_right;
	Condition::Connection m_leftConn, m_rightConn;

public:
	OrCondition(Id_t, HCondition left, HCondition right);

	/// Возвращает тип объекта
	virtual SizerTypes getType() const { return stOrCond; }

	virtual void reset();

private:
	void updateState(bool ntf = true);
	void onConditionChanged();
};

}

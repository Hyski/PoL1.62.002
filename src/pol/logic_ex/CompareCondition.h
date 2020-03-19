#pragma once

#include "Counter.h"
#include "Condition.h"
#include "LogicalCondition.h"

namespace PoL
{

//=====================================================================================//
//                      class CompareCondition : public Condition                      //
//=====================================================================================//
class CompareCondition : public Condition
{
	LogicalCondition m_lc;
	HCounter m_left, m_right;
	Counter::Connection m_leftConn, m_rightConn;

public:
	CompareCondition(Id_t, LogicalCondition, HCounter, HCounter);
	~CompareCondition();

	/// Возвращает тип объекта
	virtual SizerTypes getType() const { return stCompareCond; }

	/// Сбросить состояние на начальное
	virtual void reset();

private:
	/// Обновить состояние условия
	void updateState(bool notify=true);
	/// Вызовется при изменении одного из счетчиков
	void onCounterChanged();
};

}

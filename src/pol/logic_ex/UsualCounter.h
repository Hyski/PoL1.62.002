#pragma once

#include "Counter.h"

namespace PoL
{

//=====================================================================================//
//                         class UsualCounter : public Counter                         //
//=====================================================================================//
class UsualCounter : public Counter
{
	int m_counter;
	int m_initialCounter;
	HCondition m_incSig, m_decSig;

	Condition::Connection m_incConn, m_decConn;

public:
	UsualCounter(Id_t id, int n, HCondition inc, HCondition dec);
	virtual ~UsualCounter();

	/// Возвращает тип объекта
	virtual SizerTypes getType() const { return stUsualCounter; }

	/// Увеличить счетчик
	void increment();
	/// Уменьшить счетчик
	void decrement();

	/// Возвращает значение счетчика
	int getCounter() const { return m_counter; }
	/// Установить значение счетчика явно
	void setCounter(int c);

	/// Возвращает начальное значение
	virtual int getInitialValue() const { return m_initialCounter; }

	/// Сохранить состояние объекта
	virtual void store(mll::io::obinstream &) const;
	/// Восстановить состояние объекта
	virtual void restore(mll::io::ibinstream &);

	/// Сбросить состояние на начальное
	virtual void reset();
};

}
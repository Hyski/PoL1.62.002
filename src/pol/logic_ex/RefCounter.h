#pragma once

#include "Counter.h"

namespace PoL
{

//=====================================================================================//
//                          class RefCounter : public Counter                          //
//=====================================================================================//
class RefCounter : public Counter
{
	HCounter m_ctr;
	Counter::Connection m_conn;

public:
	RefCounter(Id_t id, HCounter);
	~RefCounter();

	/// Возвращает тип объекта
	virtual SizerTypes getType() const { return stRefCounter; }

	/// Увеличить счетчик
	virtual void increment() { if(m_ctr) m_ctr->increment(); }
	/// Уменьшить счетчик
	virtual void decrement() { if(m_ctr) m_ctr->decrement(); }

	/// Возвращает значение счетчика
	virtual int getCounter() const { return m_ctr ? m_ctr->getCounter() : 0; }
	/// Установить значение счетчика явно
	virtual void setCounter(int c) { if(m_ctr) m_ctr->setCounter(c); }

	/// Возвращает начальное значение
	virtual int getInitialValue() const { return m_ctr ? m_ctr->getInitialValue() : 0; }

	/// Сохранить состояние объекта
	virtual void store(mll::io::obinstream &) const;
	/// Восстановить состояние объекта
	virtual void restore(mll::io::ibinstream &);

	/// Сбросить состояние на начальное
	virtual void reset();

private:
	/// Вызывается при изменении счетчика
	void onCounterChanged();
};

}
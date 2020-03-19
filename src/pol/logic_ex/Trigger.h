#pragma once

#include "Condition.h"
#include "Persistent.h"

namespace PoL
{

//=====================================================================================//
//                          class Trigger : public Persistent                          //
//=====================================================================================//
class Trigger : public Persistent
{
	HCondition m_cond;
	HActTemplate m_act;
	Condition::Connection m_conn;
	bool m_onetime;
	bool m_triggered;

public:
	Trigger(Id_t, HCondition, HActTemplate, bool onetime);
	~Trigger();

	/// Возвращает тип объекта
	virtual SizerTypes getType() const { return stTrigger; }

	/// Сохранить состояние объекта
	virtual void store(mll::io::obinstream &) const;
	/// Восстановить состояние объекта
	virtual void restore(mll::io::ibinstream &);

	/// Сбросить состояние на начальное
	virtual void reset();

	/// Срабатывал ли этот триггер
	bool isSignaled() const { return m_triggered; }

	virtual void dumpState(std::ostream &out) const
	{
		if(m_onetime)
		{
			out << "strigger [" << getName() << "]\n"
				<< "\ttriggered[" << (m_triggered?"true":"false") << "\n";
		}
	}

private:
	void onConditionChanged(bool);
};

}
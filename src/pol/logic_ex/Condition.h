#pragma once

#include "NotifyFn.h"
#include "Persistent.h"

namespace PoL
{

//=====================================================================================//
//                                   class Condition                                   //
//=====================================================================================//
class Condition : public Persistent, private NotifyFn< boost::function<void (bool)> >
{
	typedef NotifyFn< boost::function<void (bool)> > NFn_t;

public:
	using NFn_t::Connection;
	using NFn_t::addFn;


private:
	bool m_initialState;	///< Начальное состояние
	bool m_state;			///< Состояние условия
	bool m_knownState;		///< Состояние условия
	bool m_enabled;

protected:
	Condition(Id_t id);
	Condition(bool v, Id_t id);
	Condition(bool v, Id_t id, bool enabled);

public:
	virtual ~Condition() {}

	/// Возвращает состояние условия
	bool getState() const { return m_state; }
	/// Возвращает состояние условия
	bool getKnownState() const { return m_knownState; }

	/// Установить состояние
	void setState(bool s, bool notify=true)
	{
		if(m_state != s)
		{
			m_state = s;
			if(isEnabled())
			{
				m_knownState = m_state;
				if(notify) NFn_t::notify(m_state);
			}
		}
	}

	void enable(bool v);
	bool isEnabled() const { return m_enabled; }

	/// Установить приоритет
	virtual void setPriority(int) {}

	/// Сохранить состояние объекта
	virtual void store(mll::io::obinstream &) const;
	/// Восстановить состояние объекта
	virtual void restore(mll::io::ibinstream &);

	virtual void dumpState(std::ostream &out) const
	{
		out << "condition [" << getName() << "]\n"
			<< "\tstate[" << (getKnownState()?"true":"false")
			<< "] internal-state[" << (getState()?"true":"false")
			<< "] " << (isEnabled()?"enabled":"disabled") << "\n";
	}

	/// Сбросить состояние на начальное
	virtual void reset();

private:
	virtual void doEnable(bool) {}
};

}
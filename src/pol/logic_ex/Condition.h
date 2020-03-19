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
	bool m_initialState;	///< ��������� ���������
	bool m_state;			///< ��������� �������
	bool m_knownState;		///< ��������� �������
	bool m_enabled;

protected:
	Condition(Id_t id);
	Condition(bool v, Id_t id);
	Condition(bool v, Id_t id, bool enabled);

public:
	virtual ~Condition() {}

	/// ���������� ��������� �������
	bool getState() const { return m_state; }
	/// ���������� ��������� �������
	bool getKnownState() const { return m_knownState; }

	/// ���������� ���������
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

	/// ���������� ���������
	virtual void setPriority(int) {}

	/// ��������� ��������� �������
	virtual void store(mll::io::obinstream &) const;
	/// ������������ ��������� �������
	virtual void restore(mll::io::ibinstream &);

	virtual void dumpState(std::ostream &out) const
	{
		out << "condition [" << getName() << "]\n"
			<< "\tstate[" << (getKnownState()?"true":"false")
			<< "] internal-state[" << (getState()?"true":"false")
			<< "] " << (isEnabled()?"enabled":"disabled") << "\n";
	}

	/// �������� ��������� �� ���������
	virtual void reset();

private:
	virtual void doEnable(bool) {}
};

}
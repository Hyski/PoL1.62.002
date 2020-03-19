#include "precomp.h"
#include "Condition.h"

namespace PoL
{

static const int g_conditionVersion = 0;

//=====================================================================================//
//                               Condition::Condition()                                //
//=====================================================================================//
Condition::Condition(Id_t id)
:	Persistent(id),
	m_state(false),
	m_knownState(false),
	m_initialState(false),
	m_enabled(true)
{
}

//=====================================================================================//
//                               Condition::Condition()                                //
//=====================================================================================//
Condition::Condition(bool ini, Id_t id)
:	Persistent(id),
	m_state(ini),
	m_knownState(ini),
	m_initialState(ini),
	m_enabled(true)
{
}

//=====================================================================================//
//                               Condition::Condition()                                //
//=====================================================================================//
Condition::Condition(bool ini, Id_t id, bool ena)
:	Persistent(id),
	m_state(ini),
	m_knownState(ini),
	m_initialState(ini),
	m_enabled(ena)
{
}

//=====================================================================================//
//                              void Condition::enable()                               //
//=====================================================================================//
void Condition::enable(bool v)
{
	if(m_enabled != v)
	{
		m_enabled = v;
		doEnable(m_enabled);
		if(m_enabled && m_knownState != m_state)
		{
			m_knownState = m_state;
			NFn_t::notify(m_state);
		}
	}
}

//=====================================================================================//
//                            void Condition::store() const                            //
//=====================================================================================//
void Condition::store(mll::io::obinstream &out) const
{
	out << asInt(g_conditionVersion);
	out << asInt(m_state);
	out << asInt(m_enabled);
	out << asInt(m_knownState);
}

//=====================================================================================//
//                              void Condition::restore()                              //
//=====================================================================================//
void Condition::restore(mll::io::ibinstream &in)
{
	int temp;
	int vers;
	bool newEnabled;

	in >> asInt(vers);
	in >> asInt(m_state);
	in >> asInt(newEnabled);
	in >> asInt(m_knownState);

	enable(newEnabled);
}

//=====================================================================================//
//                               void Condition::reset()                               //
//=====================================================================================//
void Condition::reset()
{
	m_state = m_initialState;
	m_knownState = m_initialState;
}

}

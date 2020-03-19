#include "precomp.h"
#include "Task.h"

namespace PoL
{

//=====================================================================================//
//                              void Task::store() const                               //
//=====================================================================================//
void Task::store(mll::io::obinstream &out) const
{
	out << asInt(m_state);
	out << asInt(m_knownState);
	out << asInt(m_enabled);
}

//=====================================================================================//
//                                void Task::restore()                                 //
//=====================================================================================//
void Task::restore(mll::io::ibinstream &in)
{
	bool newEnabled;

	in >> asInt(m_state);
	in >> asInt(m_knownState);
	in >> asInt(newEnabled);

	enable(newEnabled);
}

//=====================================================================================//
//                                 void Task::reset()                                  //
//=====================================================================================//
void Task::reset()
{
	m_state = tsNotCompleted;
	m_knownState = tsNotCompleted;
}

//=====================================================================================//
//                               void Task::taskReset()                                //
//=====================================================================================//
void Task::taskReset()
{
	reset();
}

//=====================================================================================//
//                                 void Task::enable()                                 //
//=====================================================================================//
void Task::enable(bool v)
{
	if(m_enabled != v)
	{
		ACT_LOG("INFO: task [" << getName() << "] is " << (v?"enabled":"disabled") << "\n");
		m_enabled = v;
		doEnable(m_enabled);
		if(m_enabled && m_knownState != m_state)
		{
			m_knownState = m_state;
			notify();
		}
	}
}


}
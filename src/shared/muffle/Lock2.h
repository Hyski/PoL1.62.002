#pragma once

#include "CriticalSection.h"

namespace Muffle2
{

//=====================================================================================//
//                                     class Lock2                                     //
//=====================================================================================//
class Lock
{
	CriticalSection *m_cs;

public:
	Lock() : m_cs(0) {}
	Lock(CriticalSection *cs) : m_cs(cs) { m_cs->enter(); }
	Lock(Lock &l) : m_cs(l.m_cs) { l.m_cs = 0; }
	~Lock() { if(m_cs) m_cs->leave(); }

	Lock &operator=(Lock &l)
	{
		if(m_cs) m_cs->leave();
		m_cs = l.m_cs;
		l.m_cs = 0;
		return *this;
	}

	void reset()
	{
		m_cs->leave();
		m_cs = 0;
	}
};

}


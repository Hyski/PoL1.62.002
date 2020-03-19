#include "precomp.h"

#include "UsualCounter.h"

namespace PoL
{

//=====================================================================================//
//                            UsualCounter::UsualCounter()                             //
//=====================================================================================//
UsualCounter::UsualCounter(Id_t id, int n, HCondition inc, HCondition dec)
:	Counter(id),
	m_counter(n),
	m_incSig(inc),
	m_decSig(dec),
	m_initialCounter(n)
{
	if(m_incSig) m_incConn = m_incSig->addFn(boost::bind(increment,this));
	if(m_decSig) m_decConn = m_decSig->addFn(boost::bind(decrement,this));
}

//=====================================================================================//
//                            UsualCounter::~UsualCounter()                            //
//=====================================================================================//
UsualCounter::~UsualCounter()
{
}

//=====================================================================================//
//                           void UsualCounter::increment()                            //
//=====================================================================================//
void UsualCounter::increment()
{
	if(m_incSig->getKnownState())
	{
		setCounter(m_counter+1);
	}
}

//=====================================================================================//
//                           void UsualCounter::decrement()                            //
//=====================================================================================//
void UsualCounter::decrement()
{
	if(m_decSig->getKnownState())
	{
		setCounter(m_counter-1);
	}
}

//=====================================================================================//
//                           void UsualCounter::setCounter()                           //
//=====================================================================================//
void UsualCounter::setCounter(int c)
{
	if(c != m_counter)
	{
		ACT_LOG("INFO: counter [" << getName() << std::dec
			<< "] changed from " << m_counter << " to " << c << "\n");
		m_counter = c;
		notify();
	}
}

//=====================================================================================//
//                          void UsualCounter::store() const                           //
//=====================================================================================//
void UsualCounter::store(mll::io::obinstream &out) const
{
	Counter::store(out);
	out << m_counter;
}

//=====================================================================================//
//                            void UsualCounter::restore()                             //
//=====================================================================================//
void UsualCounter::restore(mll::io::ibinstream &in)
{
	Counter::restore(in);
	in >> m_counter;
}

//=====================================================================================//
//                             void UsualCounter::reset()                              //
//=====================================================================================//
void UsualCounter::reset()
{
	m_counter = m_initialCounter;
}

}

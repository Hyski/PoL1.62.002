#pragma once

#include "Act.h"

namespace PoL
{

//=====================================================================================//
//                            class ComplexAct : public Act                            //
//=====================================================================================//
class ComplexAct : public Act
{
public:
	/// Содержит ли акт выигрыщ или проигрыш
	virtual bool hasEndGame() const
	{
		return std::count_if(actsBegin(),actsEnd(),boost::bind(Act::hasEndGame,_1));
	}

protected:
	typedef std::vector<HAct> Acts_t;

	Acts_t::iterator actsBegin() { return m_acts.begin(); }
	Acts_t::iterator actsEnd() { return m_acts.end(); }
	Acts_t::const_iterator actsBegin() const { return m_acts.begin(); }
	Acts_t::const_iterator actsEnd() const { return m_acts.end(); }
	size_t actsCount() const { return m_acts.size(); }

private:
	Acts_t m_acts;

public:
	template<typename It> ComplexAct(It begin, It end);
};

//=====================================================================================//
//                           inline ComplexAct::ComplexAct()                           //
//=====================================================================================//
template<typename It>
inline ComplexAct::ComplexAct(It begin, It end)
:	m_acts(begin,end)
{
}

}
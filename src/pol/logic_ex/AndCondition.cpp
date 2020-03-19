#include "precomp.h"
#include "AndCondition.h"

namespace PoL
{

//=====================================================================================//
//                            AndCondition::AndCondition()                             //
//=====================================================================================//
AndCondition::AndCondition(Id_t id, HCondition left, HCondition right)
:	Condition(id),
	m_left(left),
	m_right(right)
{
	if(m_left) m_leftConn = m_left->addFn(boost::bind(onConditionChanged,this));
	if(m_right) m_rightConn = m_right->addFn(boost::bind(onConditionChanged,this));

	onConditionChanged();
}

//=====================================================================================//
//                       void AndCondition::onConditionChanged()                       //
//=====================================================================================//
void AndCondition::onConditionChanged()
{
	updateState();
}

//=====================================================================================//
//                          void AndCondition::updateState()                           //
//=====================================================================================//
void AndCondition::updateState(bool ntf)
{
	setState((m_left?m_left->getState():true) && (m_right?m_right->getState():true),ntf);
}

//=====================================================================================//
//                             void AndCondition::reset()                              //
//=====================================================================================//
void AndCondition::reset()
{
	Condition::reset();
	updateState(false);
}

}

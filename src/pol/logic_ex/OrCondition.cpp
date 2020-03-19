#include "precomp.h"
#include "OrCondition.h"

namespace PoL
{

//=====================================================================================//
//                             OrCondition::OrCondition()                              //
//=====================================================================================//
OrCondition::OrCondition(Id_t id, HCondition left, HCondition right)
:	Condition(id),
	m_left(left),
	m_right(right)
{
	if(m_left) m_leftConn = m_left->addFn(boost::bind(onConditionChanged,this));
	if(m_right) m_rightConn = m_right->addFn(boost::bind(onConditionChanged,this));

	updateState();
}

//=====================================================================================//
//                       void OrCondition::onConditionChanged()                        //
//=====================================================================================//
void OrCondition::onConditionChanged()
{
	updateState();
}

//=====================================================================================//
//                           void OrCondition::updateState()                           //
//=====================================================================================//
void OrCondition::updateState(bool ntf)
{
	setState((m_left?m_left->getState():false) || (m_right?m_right->getState():false),ntf);
}

//=====================================================================================//
//                              void OrCondition::reset()                              //
//=====================================================================================//
void OrCondition::reset()
{
	Condition::reset();
	updateState(false);
}

}

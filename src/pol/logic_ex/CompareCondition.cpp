#include "precomp.h"

#include "CompareCondition.h"

namespace PoL
{

//=====================================================================================//
//                        CompareCondition::CompareCondition()                         //
//=====================================================================================//
CompareCondition::CompareCondition(Id_t id, LogicalCondition lc, HCounter l, HCounter r)
:	Condition(id),
	m_lc(lc),
	m_left(l),
	m_right(r)
{
	if(m_left) m_leftConn = m_left->addFn(boost::bind(onCounterChanged,this));
	if(m_right) m_rightConn = m_right->addFn(boost::bind(onCounterChanged,this));

	onCounterChanged();
}

//=====================================================================================//
//                        CompareCondition::~CompareCondition()                        //
//=====================================================================================//
CompareCondition::~CompareCondition()
{
}

//=====================================================================================//
//                      void CompareCondition::onCounterChanged()                      //
//=====================================================================================//
void CompareCondition::onCounterChanged()
{
	updateState();
}

//=====================================================================================//
//                           void CompareCondition::reset()                            //
//=====================================================================================//
void CompareCondition::reset()
{
	updateState(false);
}

//=====================================================================================//
//                        void CompareCondition::updateState()                         //
//=====================================================================================//
void CompareCondition::updateState(bool notify)
{
	int l = m_left ? m_left->getCounter() : 0;
	int r = m_right ? m_right->getCounter() : 0;
	bool result;

	ACT_LOG("INFO: condition [" << getName() << std::dec
		<< "] notified about l&r change: l==" << l << "; r==" << r << "\n");

	switch(m_lc)
	{
	case lcGreater:
		result = (l > r);
		break;
	case lcLess:
		result = (l < r);
		break;
	case lcGreaterEqual:
		result = (l >= r);
		break;
	case lcLessEqual:
		result = (l <= r);
		break;
	case lcEqual:
		result = (l == r);
		break;
	case lcNotEqual:
		result = (l != r);
		break;
	}

	setState(result,notify);
}

}
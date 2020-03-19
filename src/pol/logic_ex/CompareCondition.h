#pragma once

#include "Counter.h"
#include "Condition.h"
#include "LogicalCondition.h"

namespace PoL
{

//=====================================================================================//
//                      class CompareCondition : public Condition                      //
//=====================================================================================//
class CompareCondition : public Condition
{
	LogicalCondition m_lc;
	HCounter m_left, m_right;
	Counter::Connection m_leftConn, m_rightConn;

public:
	CompareCondition(Id_t, LogicalCondition, HCounter, HCounter);
	~CompareCondition();

	/// ���������� ��� �������
	virtual SizerTypes getType() const { return stCompareCond; }

	/// �������� ��������� �� ���������
	virtual void reset();

private:
	/// �������� ��������� �������
	void updateState(bool notify=true);
	/// ��������� ��� ��������� ������ �� ���������
	void onCounterChanged();
};

}

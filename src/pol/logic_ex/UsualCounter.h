#pragma once

#include "Counter.h"

namespace PoL
{

//=====================================================================================//
//                         class UsualCounter : public Counter                         //
//=====================================================================================//
class UsualCounter : public Counter
{
	int m_counter;
	int m_initialCounter;
	HCondition m_incSig, m_decSig;

	Condition::Connection m_incConn, m_decConn;

public:
	UsualCounter(Id_t id, int n, HCondition inc, HCondition dec);
	virtual ~UsualCounter();

	/// ���������� ��� �������
	virtual SizerTypes getType() const { return stUsualCounter; }

	/// ��������� �������
	void increment();
	/// ��������� �������
	void decrement();

	/// ���������� �������� ��������
	int getCounter() const { return m_counter; }
	/// ���������� �������� �������� ����
	void setCounter(int c);

	/// ���������� ��������� ��������
	virtual int getInitialValue() const { return m_initialCounter; }

	/// ��������� ��������� �������
	virtual void store(mll::io::obinstream &) const;
	/// ������������ ��������� �������
	virtual void restore(mll::io::ibinstream &);

	/// �������� ��������� �� ���������
	virtual void reset();
};

}
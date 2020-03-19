#pragma once

#include "Counter.h"

namespace PoL
{

//=====================================================================================//
//                          class RefCounter : public Counter                          //
//=====================================================================================//
class RefCounter : public Counter
{
	HCounter m_ctr;
	Counter::Connection m_conn;

public:
	RefCounter(Id_t id, HCounter);
	~RefCounter();

	/// ���������� ��� �������
	virtual SizerTypes getType() const { return stRefCounter; }

	/// ��������� �������
	virtual void increment() { if(m_ctr) m_ctr->increment(); }
	/// ��������� �������
	virtual void decrement() { if(m_ctr) m_ctr->decrement(); }

	/// ���������� �������� ��������
	virtual int getCounter() const { return m_ctr ? m_ctr->getCounter() : 0; }
	/// ���������� �������� �������� ����
	virtual void setCounter(int c) { if(m_ctr) m_ctr->setCounter(c); }

	/// ���������� ��������� ��������
	virtual int getInitialValue() const { return m_ctr ? m_ctr->getInitialValue() : 0; }

	/// ��������� ��������� �������
	virtual void store(mll::io::obinstream &) const;
	/// ������������ ��������� �������
	virtual void restore(mll::io::ibinstream &);

	/// �������� ��������� �� ���������
	virtual void reset();

private:
	/// ���������� ��� ��������� ��������
	void onCounterChanged();
};

}
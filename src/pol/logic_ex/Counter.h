#pragma once

#include "Signal.h"
#include "Persistent.h"

namespace PoL
{

//=====================================================================================//
//                          class Counter : public Persistent                          //
//=====================================================================================//
class Counter : public Persistent, private NotifyFn< boost::function<void (int)> >
{
	typedef NotifyFn< boost::function<void (int)> > NFn_t;

public:
	using NFn_t::Fn_t;
	using NFn_t::addFn;
	using NFn_t::Connection;

public:
	Counter(Id_t id);
	virtual ~Counter();

	/// ��������� �������
	virtual void increment() = 0;
	/// ��������� �������
	virtual void decrement() = 0;

	/// ���������� �������� ��������
	virtual int getCounter() const = 0;
	/// ���������� �������� �������� ����
	virtual void setCounter(int c) = 0;

	/// ���������� ��������� ��������
	virtual int getInitialValue() const = 0;

	virtual void dumpState(std::ostream &out) const
	{
		out << "counter [" << getName() << "]\n"
			<< "\tvalue[" << getCounter() << "]\n";
	}

	virtual void store(mll::io::obinstream &out) const;
	virtual void restore(mll::io::ibinstream &in);

protected:
	/// ��������� �����������
	void notify()
	{
		NFn_t::notify(getCounter());
	}
};

}
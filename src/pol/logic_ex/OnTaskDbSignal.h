#pragma once

#include "Signal.h"
#include "TaskDatabase.h"

namespace PoL
{
	
//=====================================================================================//
//                        class OnTaskDbSignal : public Signal                         //
//=====================================================================================//
class OnTaskDbSignal : public Signal
{
	std::string m_subj;
	TaskDatabase::MessageType m_mt;
	TaskDatabase::Connection m_sigConn;

public:
	OnTaskDbSignal(Id_t id, TaskDatabase::MessageType mt);
	OnTaskDbSignal(Id_t id, TaskDatabase::MessageType mt, const std::string &subj);

	/// ���������� ��� �������
	virtual SizerTypes getType() const { return stTaskDbSignal; }

	/// ���������� ���������
	virtual void setPriority(int);

	/// ��������� ��������� �������
	virtual void store(mll::io::obinstream &out) const { Signal::store(out); }
	/// ������������ ��������� �������
	virtual void restore(mll::io::ibinstream &in) { Signal::restore(in); }

private:
	virtual void doEnable(bool);
	/// ���������� ��� ������������� �������
	void onSignal();
};

}
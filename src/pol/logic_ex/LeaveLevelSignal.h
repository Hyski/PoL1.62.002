#pragma once

#include "Signal.h"
#include "TaskDatabase.h"

namespace PoL
{

//=====================================================================================//
//                       class LeaveLevelSignal : public Signal                        //
//=====================================================================================//
class LeaveLevelSignal : public Signal
{
	std::string m_from, m_to;
	TaskDatabase::Connection m_sigConn;

public:
	LeaveLevelSignal(Id_t id, const std::string &from);
	LeaveLevelSignal(Id_t id, const std::string &from, const std::string &to);

	/// ���������� ��� �������
	virtual SizerTypes getType() const { return stLeaveLevelSignal; }

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

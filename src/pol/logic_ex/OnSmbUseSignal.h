#pragma once

#include "Signal.h"
#include "TaskDatabase.h"

namespace PoL
{

//=====================================================================================//
//                        class OnSmbUseSignal : public Signal                         //
//=====================================================================================//
class OnSmbUseSignal : public Signal
{
	std::string m_obj;
	std::string m_subj;
	TaskDatabase::Connection m_sigConn;

public:
	OnSmbUseSignal(Id_t id, const std::string &obj, const std::string &subj);

	/// ���������� ��� �������
	virtual SizerTypes getType() const { return stSmbUseSignal; }

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

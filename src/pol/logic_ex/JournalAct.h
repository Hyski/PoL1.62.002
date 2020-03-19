#pragma once

#include "Act.h"

namespace PoL
{

//=====================================================================================//
//                            class JournalAct : public Act                            //
//=====================================================================================//
class JournalAct : public Act
{
	std::string m_jid, m_hdr, m_strid;
	bool m_add;

	bool m_started;
	bool m_ended;

public:
	JournalAct(const std::string &jid, const std::string &hdr, const std::string &strid, bool);

	/// ��������� ���, �������� ��� ��� �������� �������
	virtual void skip() {}
	/// ������� �� ���
	virtual bool isStarted() const { return m_started; }
	/// ������ ���
	virtual void start();
	/// ���� ���������� ����, ����� �� ������� ������� ��������
	virtual void update(float tau);
	/// ���������� true, ���� ��� ��������
	virtual bool isEnded() const { return m_ended; }
	/// ����� �� �������� �����������
	virtual bool isEndable() const { return true; }

private:
	void addRecord();
	void removeRecord();
};

}

#pragma once

#include "Act.h"

namespace PoL
{

//=====================================================================================//
//                           class AlignmentAct : public Act                           //
//=====================================================================================//
class AlignmentAct : public Act
{
	bool m_started;
	bool m_ended;

	::RelationType m_requiredRelation;
	std::string m_team1, m_team2;

public:
	AlignmentAct(const std::string &, const std::string &, const std::string &);

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
};

}

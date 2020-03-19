#pragma once

#include "Condition.h"
#include "TaskDatabase.h"

namespace PoL
{

//=====================================================================================//
//                      class ContextCondition : public Condition                      //
//=====================================================================================//
class ContextCondition : public Condition
{
	std::string m_level;
	int m_phase;
	bool m_inLevel;

	TaskDatabase::Connection m_phaseChangedConn, m_enterLevelConn, m_leaveLevelConn;

public:
	ContextCondition(Id_t id, const std::string &, int phase);

	/// ���������� ��� �������
	virtual SizerTypes getType() const { return stContextCond; }

	/// ���������� ���������
	virtual void setPriority(int);

	/// �������� ��������� �� ���������
	virtual void reset();

	/// ������������ ��������� �������
	virtual void restore(mll::io::ibinstream &);

private:
	void onPhaseChanged();
	void onEnterLevel();
	void onLeaveLevel();

	void update(bool ntf = true);
};

}

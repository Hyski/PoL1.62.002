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

	/// Закончить акт, применив все его побочные эффекты
	virtual void skip() {}
	/// Начался ли акт
	virtual bool isStarted() const { return m_started; }
	/// Начать акт
	virtual void start();
	/// Дать управление акту, чтобы он изменил игровую ситуацию
	virtual void update(float tau);
	/// Возвращает true, если акт закончен
	virtual bool isEnded() const { return m_ended; }
	/// Может ли действие закончиться
	virtual bool isEndable() const { return true; }
};

}

#pragma once

#include "Act.h"

namespace PoL
{

//=====================================================================================//
//                          class ChangeLevelAct : public Act                          //
//=====================================================================================//
class ChangeLevelAct : public Act
{
	std::string m_level;
	bool m_started;
	bool m_ended;

public:
	ChangeLevelAct(const std::string &);

	/// Закончить акт, применив все его побочные эффекты
	virtual void skip() {}
	/// Начался ли акт
	virtual bool isStarted() const { return m_started; }
	/// Начать акт
	virtual void start();
	/// Дать управление акту, чтобы он изменил игровую ситуацию
	virtual void update(float tau);
	/// Возвращает true, если акт закончен
	virtual bool isEnded() const;
	/// Может ли действие закончиться
	virtual bool isEndable() const { return true; }
};

}
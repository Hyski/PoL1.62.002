#pragma once

#include "Act.h"

namespace PoL
{

//=====================================================================================//
//                              class RefAct : public Act                              //
//=====================================================================================//
class RefAct : public Act
{
	HAct m_act;

public:
	RefAct(const std::string &act);

	/// Закончить акт, применив все его побочные эффекты
	virtual void skip() { m_act->skip(); }
	/// Начался ли акт
	virtual bool isStarted() const { return m_act->isStarted(); }
	/// Начать акт
	virtual void start() { m_act->start(); }
	/// Дать управление акту, чтобы он изменил игровую ситуацию
	virtual void update(float tau) { m_act->update(tau); }
	/// Возвращает true, если акт закончен
	virtual bool isEnded() const { return m_act->isEnded(); }
	/// Может ли действие закончиться
	virtual bool isEndable() const { return m_act->isEndable(); }
};

}

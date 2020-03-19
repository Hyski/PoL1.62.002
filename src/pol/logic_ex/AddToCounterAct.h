#pragma once

#include "Act.h"
#include "Counter.h"

namespace PoL
{

//=====================================================================================//
//                         class AddToCounterAct : public Act                          //
//=====================================================================================//
class AddToCounterAct : public Act
{
	bool m_started;
	bool m_ended;
	HCounter m_ctr;
	int m_delta;

public:
	AddToCounterAct(const std::string &, int delta);

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
#pragma once

#include "Act.h"

namespace PoL
{

//=====================================================================================//
//                            class WinGameAct : public Act                            //
//=====================================================================================//
class WinGameAct : public Act
{
	bool m_started;
	bool m_ended;
	int m_phase;

public:
	WinGameAct();

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
	/// Содержит ли акт выигрыщ или проигрыш
	virtual bool hasEndGame() const { return true; }
};

}

#pragma once

#include "Act.h"

namespace PoL
{

//=====================================================================================//
//                             class DoorAct : public Act                              //
//=====================================================================================//
class DoorAct : public Act
{
	std::string m_doorId;
	bool m_open;

	bool m_started;
	bool m_ended;

	float m_duration;

public:
	DoorAct(const std::string &doorId, bool open);

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

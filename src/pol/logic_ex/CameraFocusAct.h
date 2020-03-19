#pragma once

#include "Act.h"

namespace PoL
{

//=====================================================================================//
//                          class CameraFocusAct : public Act                          //
//=====================================================================================//
class CameraFocusAct : public Act
{
	std::string m_subj;
	::point3 m_target;
	bool m_canFocus;
	bool m_started;
	bool m_ended;
	float m_startTime;

public:
	CameraFocusAct(const std::string &);

	/// Закончить акт, применив все его побочные эффекты
	virtual void skip() { m_ended = true; }
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

#pragma once

#include "Act.h"

class KeyAnimation;

namespace PoL
{

//=====================================================================================//
//                           class CamSplineAct : public Act                           //
//=====================================================================================//
class CamSplineAct : public Act
{
	std::string m_camera;
	float m_length;

	bool m_started;
	bool m_ended;
	float m_startTime;
	bool m_roofsVisible;

public:
	CamSplineAct(const std::string &camname, float dur = 0.0f);

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

#pragma once

#include "Act.h"

namespace PoL
{

//=====================================================================================//
//                           class CamFollowAct : public Act                           //
//=====================================================================================//
class CamFollowAct : public Act, private ::EntityObserver
{
	std::string m_who;
	BaseEntity *m_entity;
	bool m_started;

public:
	CamFollowAct(const std::string &who);
	~CamFollowAct();

	/// Закончить акт, применив все его побочные эффекты
	virtual void skip() {}
	/// Начался ли акт
	virtual bool isStarted() const { return m_started; }
	/// Начать акт
	virtual void start();
	/// Дать управление акту, чтобы он изменил игровую ситуацию
	virtual void update(float tau);
	/// Возвращает true, если акт закончен
	virtual bool isEnded() const { return false; }
	/// Может ли действие закончиться
	virtual bool isEndable() const { return false; }

private:
	void Update(BaseEntity *, event_t, info_t);
};

}

#pragma once

#include "Act.h"

namespace PoL
{

//=====================================================================================//
//                          class MovePersonAct : public Act                           //
//=====================================================================================//
class MovePersonAct : public Act, private ::EntityObserver
{
public:
	enum MoveType
	{
		mtWalk,
		mtRun,
		mtCrawl
	};

private:
	std::string m_name;
	ipnt2_t m_where;
	MoveType m_moveType;
	std::string m_zone;

	bool m_started;
	bool m_ended;

	bool m_useZone;

	::GraphHuman::move_type m_oldMoveType;
	::GraphHuman::stance_type m_oldStance;
	::GraphHuman::pose_type m_oldPose;

	// Свойства, инициализируемые при старте
	std::auto_ptr< ::Activity > m_activity;
	HumanEntity *m_entity;
	VehicleEntity *m_vehentity;

public:
	MovePersonAct(const std::string &name, const ipnt2_t &, MoveType);
	MovePersonAct(const std::string &name, const std::string &zone, MoveType);

	/// Начался ли акт
	virtual bool isStarted() const { return m_started; }
	/// Начать акт
	virtual void start();
	/// Закончить акт, применив все его побочные эффекты
	virtual void skip() {}
	/// Дать управление акту, чтобы он изменил игровую ситуацию
	virtual void update(float tau);
	/// Возвращает true, если акт закончен
	virtual bool isEnded() const { return m_ended; }

private:
	void abnormalEnd();
	bool getZoneCoord(::ipnt2_t *);

private:
	void Update(BaseEntity *, event_t, info_t);
};

}
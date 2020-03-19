#pragma once

//#include "VisUtils3.h"
//#include "VisMap.h"
#include "DecoratorDrv.h"
#include "SpectatorObserver.h"

//=====================================================================================//
//                                class OpponentViewDrv                                //
//=====================================================================================//
// класс - останавливает действие если замечен враг
class OpponentViewDrv
:	public DecoratorDrv, 
	private EntityObserver,
	private SpectatorObserver
{
	typedef std::set<eid_t> eid_set_t;

	eid_set_t m_eids;
	unsigned  m_flags;

	BaseEntity *m_entity;

public:
	enum flag_type
	{
		F_PRINT_MSGS = 1 << 1, //выводить сообщения в консоль 
	};

	OpponentViewDrv(BaseEntity* entity, unsigned flags, Driver* drv = 0);
	~OpponentViewDrv();

	activity_command CalcStep(Activity* activity, activity_command user_cmd);

private:
	void Update(Spectator* spectator, SpectatorObserver::event_t event, SpectatorObserver::info_t info);
	void Update(BaseEntity* entity, EntityObserver::event_t event, EntityObserver::info_t info);

	void DetachAllObservers();
};

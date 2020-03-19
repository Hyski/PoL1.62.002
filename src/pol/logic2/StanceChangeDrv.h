#pragma once

#include "VisMap.h"
//#include "VisUtils3.h"
#include "DecoratorDrv.h"
#include "MarkerObserver.h"
#include "SpectatorObserver.h"

//=====================================================================================//
//                                class StanceChangeDrv                                //
//=====================================================================================//
// декоратор для смены стойки
class StanceChangeDrv
:	public DecoratorDrv, 
	private EntityObserver,
	private MarkerObserver,
	private SpectatorObserver
{
    BaseEntity* m_entity;

public:
    StanceChangeDrv(BaseEntity* entity, Driver* drv = 0);
	~StanceChangeDrv();

private:
    void Update(Spectator* spectator, SpectatorObserver::event_t event, SpectatorObserver::info_t info);
    void Update(Marker* marker, MarkerObserver::event_t event, MarkerObserver::info_t info);
    void Update(BaseEntity* entity, EntityObserver::event_t event, EntityObserver::info_t info);

    void DetachAllObservers();
};

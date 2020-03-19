#pragma once

#include "VisMap.h"
#include "EntityAux.h"
#include "DecoratorDrv.h"
#include "MarkerObserver.h"

//=====================================================================================//
//                                  class ReactionDrv                                  //
//=====================================================================================//
// класс - менеджер реакций
class ReactionDrv
:	public DecoratorDrv,
	private EntityObserver,
	private MarkerObserver
{
    typedef std::set<eid_t> eid_set_t;

	bool m_fplay;

    eid_set_t m_can_react;
    eid_set_t m_can_not_react;

    Activity*   m_activity;
    BaseEntity* m_entity;

public:
    ReactionDrv(BaseEntity* entity, Driver* drv = 0);
    ~ReactionDrv();

    activity_command CalcStep(Activity* activity, activity_command user_cmd);    
    bool Run(Activity* activity, bool result);

private:
    bool PlayReaction(Activity* notifier);
    void Update(Marker* marker, MarkerObserver::event_t event, MarkerObserver::info_t info);
    void Update(BaseEntity* entity, EntityObserver::event_t event, EntityObserver::info_t info);

    void DetachAllObservers();
};

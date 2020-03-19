#include "logicdefs.h"

#include "Entity.h"
#include "Marker.h"
#include "AIUtils.h"
#include "Spectator.h"
#include "StanceChangeDrv.h"

//=====================================================================================//
//                         StanceChangeDrv::StanceChangeDrv()                          //
//=====================================================================================//
StanceChangeDrv::StanceChangeDrv(BaseEntity* entity, Driver* drv)
:	DecoratorDrv(drv),
	m_entity(entity)
{
    m_entity->Attach(this, EV_DESTROY);
    m_entity->Attach(this, EV_DEATH_PLAYED);

    EntityContext* context = m_entity->GetEntityContext();

    if(m_entity->Cast2Human())
	{
        //будeм отслеживать всех кто попдает к нам в поле видимости
        context->GetSpectator()->AttachSpectatorObserver(this, EV_VISIBLE_MARKER);
        context->GetSpectator()->AttachSpectatorObserver(this, EV_INVISIBLE_MARKER);
    }

    //будем отслеживать всех кто нас теряет из FOS и замечает
    context->GetMarker()->AttachMarkerObserver(this, EV_VISIBLE_FOR_SPECTATOR);
    context->GetMarker()->AttachMarkerObserver(this, EV_INVISIBLE_FOR_SPECTATOR);
}

//=====================================================================================//
//                         StanceChangeDrv::~StanceChangeDrv()                         //
//=====================================================================================//
StanceChangeDrv::~StanceChangeDrv()
{
    if(m_entity) DetachAllObservers();
}

//=====================================================================================//
//                           void StanceChangeDrv::Update()                            //
//=====================================================================================//
void StanceChangeDrv::Update(Spectator* spectator, SpectatorObserver::event_t event,
							 SpectatorObserver::info_t info)
{
    AIUtils::ChangeHumanStance(m_entity->Cast2Human());        
}

//=====================================================================================//
//                           void StanceChangeDrv::Update()                            //
//=====================================================================================//
void StanceChangeDrv::Update(Marker* marker, MarkerObserver::event_t event,
							 MarkerObserver::info_t info)
{
    Spectator* spectator = static_cast<spectator_info*>(info)->m_spectator;

	if(spectator->GetEntity() && spectator->GetEntity()->Cast2Human())
	{
		HumanEntity* human = spectator->GetEntity()->Cast2Human();
        AIUtils::ChangeHumanStance(human);
	}
}

//=====================================================================================//
//                           void StanceChangeDrv::Update()                            //
//=====================================================================================//
void StanceChangeDrv::Update(BaseEntity* entity, EntityObserver::event_t event,
							 EntityObserver::info_t info)
{
    DetachAllObservers();
}

//=====================================================================================//
//                     void StanceChangeDrv::DetachAllObservers()                      //
//=====================================================================================//
void StanceChangeDrv::DetachAllObservers()
{
    m_entity->Detach(this);
    m_entity->GetEntityContext()->GetMarker()->DetachMarkerObserver(this);
    m_entity->GetEntityContext()->GetSpectator()->DetachSpectatorObserver(this);

    m_entity = 0;
}

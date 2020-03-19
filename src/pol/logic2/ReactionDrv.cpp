#include "logicdefs.h"

#include "Thing.h"
#include "Entity.h"
#include "Marker.h"
#include "AIUtils.h"
#include "Graphic.h"
#include "Activity.h"
#include "Cameraman.h"
#include "Spectator.h"
#include "ReactionDrv.h"

//=====================================================================================//
//                             ReactionDrv::ReactionDrv()                              //
//=====================================================================================//
ReactionDrv::ReactionDrv(BaseEntity* entity, Driver* drv)
:	DecoratorDrv(drv),
	m_entity(entity),
	m_fplay(false),
	m_activity(0)
{
	m_entity->Attach(this, EV_DESTROY);
	m_entity->Attach(this, EV_DEATH_PLAYED);
	m_entity->Attach(this, EV_PREPARE_DEATH);

	Marker* marker = m_entity->GetEntityContext()->GetMarker();

	marker->AttachMarkerObserver(this, EV_VISIBLE_FOR_SPECTATOR);

	//соберем список тех, кто уже на нас реагировал
	VisMap::SpecIterator_t itor = VisMap::GetInst()->SpecBegin(marker, VisParams::vpVisible);

	while(itor != VisMap::GetInst()->SpecEnd())
	{
		if(BaseEntity* entity = itor->GetEntity())
		{
			m_can_not_react.insert(entity->GetEID());
		}

		++itor;
	}
}

//=====================================================================================//
//                             ReactionDrv::~ReactionDrv()                             //
//=====================================================================================//
ReactionDrv::~ReactionDrv()
{ 
	if(m_entity) DetachAllObservers();
	delete m_activity;
}

//=====================================================================================//
//                      activity_command ReactionDrv::CalcStep()                       //
//=====================================================================================//
activity_command ReactionDrv::CalcStep(Activity* activity, activity_command user_cmd)
{
	if(m_can_react.empty())
	{
		return DecoratorDrv::CalcStep(activity, user_cmd);
	}
	else
	{
		return DecoratorDrv::CalcStep(activity, AC_STOP);
	}
}

//=====================================================================================//
//                               bool ReactionDrv::Run()                               //
//=====================================================================================//
bool ReactionDrv::Run(Activity* activity, bool result)
{
	if(!result) m_fplay = true;

	if(m_fplay = m_fplay && PlayReaction(activity))
		return true;

	return DecoratorDrv::Run(activity, result);
}

//=====================================================================================//
//                          bool ReactionDrv::PlayReaction()                           //
//=====================================================================================//
bool ReactionDrv::PlayReaction(Activity* notifier)
{  
	if(m_activity)
	{
		if(!m_activity->Run(AC_TICK))
		{
			delete m_activity;
			m_activity = 0;
		}

		return true;
	}

	//если нас кто-нибудь заметил или перца уже убили
	while(m_can_react.size() && m_entity)
	{
		eid_t eid = *m_can_react.begin();
		m_can_react.erase(m_can_react.begin());

		if(BaseEntity* killer = EntityPool::GetInst()->Get(eid))
		{
			ActivityObserver::enemy_reacted_s info(killer);
			notifier->Notify(ActivityObserver::EV_ENEMY_REACTED, &info);

			int    accuracy = 0;
			point3 pos = m_entity->GetGraph()->GetShotPoint();

			if(HumanEntity* human = killer->Cast2Human())
			{
				WeaponThing* weapon = static_cast<WeaponThing*>(human->GetEntityContext()->GetThingInHands(TT_WEAPON));
				if(weapon)
				{
					accuracy = AIUtils::CalcShootAccuracy(human, weapon, pos);
				}
			}
			else if(VehicleEntity* vehicle = killer->Cast2Vehicle())
			{
				accuracy = vehicle->GetInfo()->GetAccuracy();
			}

			if(killer->Cast2Human() && killer->Cast2Human()->GetEntityContext()->GetThingInHands(TT_WEAPON))
			{
				m_activity = ActivityFactory::CreateShoot(
					killer,
					/*pos, */
					AIUtils::NormalizeAccuracy(killer, accuracy),
					ActivityFactory::CT_REACTION_SHOOT,
					ActivityFactory::shoot_info_s(m_entity->GetEID())); 
			}
			return true;
		}
	}

	return false;
}

//=====================================================================================//
//                             void ReactionDrv::Update()                              //
//=====================================================================================//
void ReactionDrv::Update(Marker* marker, MarkerObserver::event_t event,
						 MarkerObserver::info_t info)
{
	if(BaseEntity* entity = static_cast<spectator_info*>(info)->m_spectator->GetEntity())
	{
		//если перец уже есть в списке 
		if(m_can_not_react.count(entity->GetEID()))
			return;

		if(AIUtils::CanReact(m_entity, entity))
		{
			m_can_react.insert(entity->GetEID());
		}
		else
		{
			m_can_not_react.insert(entity->GetEID());
		}
	}
}

//=====================================================================================//
//                             void ReactionDrv::Update()                              //
//=====================================================================================//
void ReactionDrv::Update(BaseEntity* entity, EntityObserver::event_t event,
						 EntityObserver::info_t info)
{
	if(event == EV_PREPARE_DEATH && m_entity->GetPlayer() == PT_PLAYER)
	{
		Cameraman::GetInst()->FocusEntity(m_entity);
	}

	DetachAllObservers();
}

//=====================================================================================//
//                       void ReactionDrv::DetachAllObservers()                        //
//=====================================================================================//
void ReactionDrv::DetachAllObservers()
{
	m_entity->Detach(this);
	m_entity->GetEntityContext()->GetMarker()->DetachMarkerObserver(this);

	m_entity = 0;
}

#include "logicdefs.h"

#include "Marker.h"
#include "Entity.h"
#include "AIUtils.h"
#include "Activity.h"
#include "Spectator.h"
#include "DirtyLinks.h"
#include "EnemyDetection.h"
#include "OpponentViewDrv.h"

//=====================================================================================//
//                         OpponentViewDrv::OpponentViewDrv()                          //
//=====================================================================================//
OpponentViewDrv::OpponentViewDrv(BaseEntity* entity, unsigned flags, Driver* drv)
:	DecoratorDrv(drv),
	m_entity(entity),
	m_flags(flags)
{
	m_entity->Attach(this, EV_PREPARE_DEATH);

	if(m_entity->GetPlayer() == PT_PLAYER)
	{
		PlayerSpectator::GetInst()->GetSpectator()->AttachSpectatorObserver(this, EV_VISIBLE_MARKER);
	}
	else
	{
		m_entity->GetEntityContext()->GetSpectator()->AttachSpectatorObserver(this, EV_VISIBLE_MARKER);
	}
}

//=====================================================================================//
//                         OpponentViewDrv::~OpponentViewDrv()                         //
//=====================================================================================//
OpponentViewDrv::~OpponentViewDrv()
{
	if(m_entity) DetachAllObservers();
}

//=====================================================================================//
//                    activity_command OpponentViewDrv::CalcStep()                     //
//=====================================================================================//
activity_command OpponentViewDrv::CalcStep(Activity* activity, activity_command user_cmd)
{
	if(m_eids.size())
	{
		user_cmd = AC_STOP;

		eid_set_t::iterator itor = m_eids.begin();

		while(itor != m_eids.end())
		{
			ActivityObserver::enemy_spotted_s info(EntityPool::GetInst()->Get(*itor));
			activity->Notify(ActivityObserver::EV_ENEMY_SPOTTED, &info); 
			++itor;
		}
	}

	return DecoratorDrv::CalcStep(activity, user_cmd);
}

//=====================================================================================//
//                           void OpponentViewDrv::Update()                            //
//=====================================================================================//
void OpponentViewDrv::Update(Spectator* spectator, SpectatorObserver::event_t event,
							 SpectatorObserver::info_t info)
{
	BaseEntity* entity = static_cast<marker_info*>(info)->m_marker->GetEntity();

	if(entity && EnemyDetector::getInst()->isHeEnemy4Me(m_entity, entity))
	{
		m_eids.insert(entity->GetEID());
		if(m_flags & F_PRINT_MSGS)
		{
			DirtyLinks::Print( mlprintf( DirtyLinks::GetStrRes("enemy_spotted").c_str(), m_entity->GetInfo()->GetName().c_str()));
		}
	}
}       

//=====================================================================================//
//                           void OpponentViewDrv::Update()                            //
//=====================================================================================//
void OpponentViewDrv::Update(BaseEntity* entity, EntityObserver::event_t event,
							 EntityObserver::info_t info)
{
	DetachAllObservers();
}

//=====================================================================================//
//                     void OpponentViewDrv::DetachAllObservers()                      //
//=====================================================================================//
void OpponentViewDrv::DetachAllObservers()
{
	m_entity->Detach(this);
	m_entity->GetEntityContext()->GetSpectator()->DetachSpectatorObserver(this);

	PlayerSpectator::GetInst()->GetSpectator()->DetachSpectatorObserver(this);

	m_entity = 0;
}

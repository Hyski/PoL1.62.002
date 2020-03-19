#pragma warning(disable:4786)

#include "logicdefs.h"

#include "Thing.h"
#include "Entity.h"
#include "AIUtils.h"
#include "Damager.h"
#include "DirtyLinks.h"

#include "TracingLOFCheck.h"

#include "ShootManager.h"
#include "ShootNotifier.h"

#include "UseActivity.h"
#include "MoveActivity.h"
#include "ShootActivity.h"
#include "ThrowActivity.h"
#include "RotateActivity.h"
#include "LandingActivity.h"
#include "ShipmentActivity.h"
#include "MultiMoveActivity.h"

#include "TargetManager.h"

#include "GrenadeThrowManager.h"
#include "CameraShieldThrowManager.h"

#include "PassDrv.h"
#include "MPLimitDrv.h"
#include "UpdateMPDrv.h"
#include "ReactionDrv.h"
#include "PathShowDrv.h"
#include "StepLimitDrv.h"
#include "DecAccuracyDrv.h"
//#include "FirstPhraseDrv.h"
#include "StanceChangeDrv.h"
#include "UpdateVisMapDrv.h"
#include "OpponentViewDrv.h"

#include "PathUtils.h"

//=====================================================================================//
//                    ActivityFactory::shoot_info_s::shoot_info_s()                    //
//=====================================================================================//
ActivityFactory::shoot_info_s::shoot_info_s(eid_t entity)
:	m_entity(EntityPool::GetInst()->Get(entity)),
	m_shotAim(0.0f,0.0f,0.0f),
	m_approxPos(0.0f,0.0f,0.0f),
	m_type(tEntity)
{
	GetShotAim();
	GetApproxPos();
}

//=====================================================================================//
//            BaseEntity *ActivityFactory::shoot_info_s::GetEntity() const             //
//=====================================================================================//
BaseEntity *ActivityFactory::shoot_info_s::GetEntity() const
{
	return m_type == tEntity ? m_entity.get() : 0;
}

//=====================================================================================//
//             point3 ActivityFactory::shoot_info_s::GetApproxPos() const              //
//=====================================================================================//
point3 ActivityFactory::shoot_info_s::GetApproxPos() const
{
	if(IsEntity())
	{
		if(GetEntity())
		{
			return m_approxPos = GetEntity()->GetGraph()->GetPos3();
		}
		else
		{
			return m_approxPos;
		}
	}
	else if(IsObject())
	{
		return DirtyLinks::GetObjCenter(GetObject());
	}
	else
	{
		return GetPoint();
	}
}

//=====================================================================================//
//              point3 ActivityFactory::shoot_info_s::GetShotAim() const               //
//=====================================================================================//
point3 ActivityFactory::shoot_info_s::GetShotAim() const
{
	if(IsEntity())
	{
		if(GetEntity())
		{
			return m_shotAim = GetEntity()->GetGraph()->GetShotPoint();
		}
		else
		{
			return m_shotAim;
		}
	}
	else if(IsObject())
	{
		return DirtyLinks::GetObjCenter(GetObject());
	}
	else
	{
		return GetPoint();
	}
}
//=====================================================================================//
//                       Activity* ActivityFactory::CreateMove()                       //
//=====================================================================================//
Activity* ActivityFactory::CreateMove(BaseEntity* entity, const pnt_vec_t& path, unsigned flags)
{
	//assert( !path.empty() );
    Driver* drv = 0;

    if(flags & CT_STAY_CHANGE) drv = new StanceChangeDrv(entity, drv);
    if(flags & CT_PLAY_REACTION) drv = new ReactionDrv(entity, drv);    
    //if(flags & CT_PLAY_FIRST_PHRASE) drv = new FirstPhraseDrv(entity,drv);

    if(flags & CT_ENEMY_STOP)
	{
        unsigned drv_flags = 0;
       
        if(flags & CT_PRINT_MSGS) drv_flags |= OpponentViewDrv::F_PRINT_MSGS;

        drv = new OpponentViewDrv(entity, drv_flags, drv);
    }
 
    unsigned pass_flags = 0;

    if(flags & CT_BUS_PASS)   pass_flags |= PassDrv::F_STOP_BEFORE_HUMAN;
    if(flags & CT_USUAL_PASS) pass_flags |= PassDrv::F_STOP_BEFORE_ALLY|PassDrv::F_CALC_DANGER_HEXES;

    drv = new PassDrv(entity, path, pass_flags, drv);

    if(flags & CT_PATH_SHOW)  drv = new PathShowDrv(path,drv);
    
    if(flags & CT_UPDATE_MPS)
	{
        int hexcost = 0;
        
        if(HumanEntity* human = entity->Cast2Human())
            hexcost = human->GetEntityContext()->GetHexCost();
        else if(VehicleEntity* vehicle = entity->Cast2Vehicle())
            hexcost = vehicle->GetEntityContext()->GetHexCost();
        
        drv = new UpdateMPDrv(entity, hexcost, drv);
    }

    if(flags & CT_UPDATE_VIS)
	{
        unsigned flags = 0;
        
        flags |= UpdateVisMapDrv::F_UPDATE_ALWAYS;
        flags |= UpdateVisMapDrv::F_UPDATE_MARKER;
        flags |= UpdateVisMapDrv::F_UPDATE_SPECTATOR;
        
        drv = new UpdateVisMapDrv(entity, flags, drv);
    }

    if(flags & CT_MPS_LIMIT)  drv = new MPLimitDrv(entity, drv);
    
    if(flags & CT_STEP_LIMIT)
	{
        unsigned drv_flags = 0;
        
        if(flags & CT_PRINT_MSGS) drv_flags |= StepLimitDrv::F_PRINT_MSGS;

        drv = new StepLimitDrv(entity, drv_flags, drv);
    }

    if(HumanEntity* human = entity->Cast2Human())
	{
        if(flags & CT_ACCURACY_DECREASE && human->GetGraph()->IsRunMove())
            drv = new DecAccuracyDrv(human, drv);
    }

	bool noMoveInterrupt = false;
	if(flags & CT_NO_MOVE_INTERRUPT) noMoveInterrupt = true;

    return new MoveActivity(entity, path, drv, Cameraman::GetInst()->FilmMove(entity),noMoveInterrupt);
}

//=====================================================================================//
//                    Activity *ActivityFactory::CreateMultiMove()                     //
//=====================================================================================//
Activity *ActivityFactory::CreateMultiMove(const std::vector<BaseEntity *> &ents,
										   const std::vector<pnt_vec_t> &paths, unsigned flags)
{
	assert( paths.size() == ents.size() );

	std::vector<PoL::TrackInfo> tis;
	tis.resize(ents.size());

	for(int i = 0; i != ents.size(); ++i)
	{
		tis[i].m_path = paths[i];

		Driver* drv = 0;

		if(flags & CT_STAY_CHANGE) drv = new StanceChangeDrv(ents[i], drv);
		if(flags & CT_PLAY_REACTION) drv = new ReactionDrv(ents[i], drv);    

		if(flags & CT_ENEMY_STOP)
		{
			unsigned drv_flags = 0;
	       
			if(flags & CT_PRINT_MSGS) drv_flags |= OpponentViewDrv::F_PRINT_MSGS;

			drv = new OpponentViewDrv(ents[i], drv_flags, drv);
		}
	 
		unsigned int pass_flags = 0;

		if(flags & CT_BUS_PASS)   pass_flags |= PassDrv::F_STOP_BEFORE_HUMAN;
		if(flags & CT_USUAL_PASS) pass_flags |= PassDrv::F_STOP_BEFORE_ALLY|PassDrv::F_CALC_DANGER_HEXES;

		drv = new PassDrv(ents[i], tis[i].m_path, pass_flags, drv);

		//if(flags & CT_PATH_SHOW)
		//{
		//	drv = new PathShowDrv(tis[i].m_path, drv);
		//}
	    
		if(flags & CT_UPDATE_MPS)
		{
			int hexcost = 0;
	        
			if(HumanEntity *human = ents[i]->Cast2Human())
			{
				hexcost = human->GetEntityContext()->GetHexCost();
			}
			else if(VehicleEntity *vehicle = ents[i]->Cast2Vehicle())
			{
				hexcost = vehicle->GetEntityContext()->GetHexCost();
			}
	        
			drv = new UpdateMPDrv(ents[i], hexcost, drv);
		}

		if(flags & CT_UPDATE_VIS)
		{
			unsigned flags = 0;
	        
			flags |= UpdateVisMapDrv::F_UPDATE_ALWAYS;
			flags |= UpdateVisMapDrv::F_UPDATE_MARKER;
			flags |= UpdateVisMapDrv::F_UPDATE_SPECTATOR;
	        
			drv = new UpdateVisMapDrv(ents[i], flags, drv);
		}

		if(flags & CT_MPS_LIMIT)
		{
			drv = new MPLimitDrv(ents[i], drv);
		}
	    
		if(flags & CT_STEP_LIMIT)
		{
			unsigned drv_flags = 0;
	        
			if(flags & CT_PRINT_MSGS) drv_flags |= StepLimitDrv::F_PRINT_MSGS;

			drv = new StepLimitDrv(ents[i], drv_flags, drv);
		}

		if(HumanEntity* human = ents[i]->Cast2Human())
		{
			if(flags & CT_ACCURACY_DECREASE && human->GetGraph()->IsRunMove())
			{
				drv = new DecAccuracyDrv(human, drv);
			}
		}

		tis[i].m_entity = ents[i];
		tis[i].m_driver = drv;
	}

	return new PoL::MultiMoveActivity(tis);
}

//=====================================================================================//
//                      Activity* ActivityFactory::CreateRotate()                      //
//=====================================================================================//
Activity* ActivityFactory::CreateRotate(BaseEntity* entity, float angle, unsigned flags)
{
	return CreateRotate(entity,angle,AIUtils::CalcMps4EntityRotate(entity,angle),flags);
}

//=====================================================================================//
//                      Activity* ActivityFactory::CreateRotate()                      //
//=====================================================================================//
Activity* ActivityFactory::CreateRotate(BaseEntity* entity, float angle, int maxmovepnts, unsigned flags)
{
    Driver* drv = 0;

    if(flags & CT_ENEMY_STOP)
	{
        unsigned drv_flags = 0;
        if(flags & CT_PRINT_MSGS) drv_flags |= OpponentViewDrv::F_PRINT_MSGS;
        drv = new OpponentViewDrv(entity, drv_flags, drv);
    }

    if(flags & CT_UPDATE_VIS)
	{
        unsigned flags = 0;

        flags |= UpdateVisMapDrv::F_UPDATE_ALWAYS;
        flags |= UpdateVisMapDrv::F_UPDATE_SPECTATOR;

        drv = new UpdateVisMapDrv(entity, flags, drv);
    }

    if(flags & CT_MPS_LIMIT)   drv = new MPLimitDrv(entity, drv);
    if(flags & CT_UPDATE_MPS)  drv = new UpdateMPDrv(entity, 1, drv);
    if(flags & CT_STAY_CHANGE) drv = new StanceChangeDrv(entity, drv);
    
    return new RotateActivity(entity, angle, maxmovepnts, drv, Cameraman::GetInst()->FilmRotate(entity), (flags & CT_UPDATE_MPS) == 0);
}

//=====================================================================================//
//                      Activity* ActivityFactory::CreateShoot()                       //
//=====================================================================================//
Activity* ActivityFactory::CreateShoot(BaseEntity* entity, float accuracy,
									   unsigned flags, const shoot_info_s& info)
{
    Driver* drv = 0;

    if(flags & CT_UPDATE_MPS)
	{
        if(HumanEntity* human = entity->Cast2Human())
		{
            WeaponThing* weapon = static_cast<WeaponThing*>(human->GetEntityContext()->GetThingInHands(TT_WEAPON));
            drv = new UpdateMPDrv(entity, weapon->GetMovepnts(human->GetEntityContext()->GetShotType()), drv);
        }

        if(VehicleEntity* vehicle = entity->Cast2Vehicle())
		{
            drv = new UpdateMPDrv(vehicle, vehicle->GetInfo()->GetMp4Shot(), drv);
        }
    }
    
    if(flags & CT_UPDATE_VIS)
	{
        unsigned flags = 0;

        flags |= UpdateVisMapDrv::F_UPDATE_ONCE;
        flags |= UpdateVisMapDrv::F_UPDATE_SPECTATOR;

        drv = new UpdateVisMapDrv(entity, flags, drv);
    }

    if(flags & CT_STAY_CHANGE) drv = new StanceChangeDrv(entity, drv);

    unsigned entity_damager = 0;

    entity_damager |= EntityDamager::FT_MAKE_EFFECTS;
    entity_damager |= EntityDamager::FT_SEND_GAME_EVENTS;
    entity_damager |= EntityDamager::FT_PLAY_ENTITY_HURT;

    if(flags & CT_HURT_SHOOT)
	{
        entity_damager |= EntityDamager::FT_MAKE_DAMAGES;
        entity_damager |= EntityDamager::FT_PLAY_ENTITY_DEATH;
    }

    if(flags & CT_PRINT_DMGS) entity_damager |= EntityDamager::FT_PRINT_MESSAGES;
    
    TracingLOFCheck* lof = 0;

    if(flags & CT_TRACE_LOF) lof = new TracingLOFCheck(info,false);///*info.m_entity, info.m_object, */(flags & CT_PRINT_MSGS) != 0);

	BaseEntity* victim = info.GetEntity(); //(info.m_type == shoot_info_s::tEntity ? EntityPool::GetInst()->Get(info.m_entity) : 0);
    Camerawork* cam = Cameraman::GetInst()->FilmShoot(entity, victim);

    TargetManager* target_mgr = new TargetManager(accuracy, info, lof/*to, info.m_object, info.m_entity*/);
    ShootNotifier* notifier   = flags & CT_SEND_EVENTS ? new ShootNotifier(info /*victim, info.m_object*/) : 0;
	ShootManager*  shoot_mgr  = new ShootManager(target_mgr, entity_damager, notifier, lof, flags&CT_NO_AMMO_DEC?true:false);

    return new ShootActivity(entity, shoot_mgr, info, drv, cam);
}

//=====================================================================================//
//                     Activity* ActivityFactory::CreateShipment()                     //
//=====================================================================================//
Activity* ActivityFactory::CreateShipment(HumanEntity* human, VehicleEntity* vehicle, unsigned flags)
{
    Driver* drv = 0;

    if(flags & CT_STAY_CHANGE)
	{
        drv = new StanceChangeDrv(human, drv);
        drv = new StanceChangeDrv(vehicle, drv);
    }

	if(flags & CT_UPDATE_MPS) drv = new UpdateMPDrv(human, PoL::Inv::MovepointsForShipment, drv);

    return new ShipmentActivity(human, vehicle, drv, Cameraman::GetInst()->FilmShipment(human, vehicle));
}

//=====================================================================================//
//                     Activity* ActivityFactory::CreateLanding()                      //
//=====================================================================================//
Activity* ActivityFactory::CreateLanding(HumanEntity* human,const ipnt2_t& to, unsigned flags)
{
    Driver* drv = 0;

	if(flags & CT_UPDATE_MPS) drv = new UpdateMPDrv(human, PoL::Inv::MovepointsForLanding, drv);

    if(flags & CT_STAY_CHANGE)
	{
        drv = new StanceChangeDrv(human, drv);
        drv = new StanceChangeDrv(human->GetEntityContext()->GetCrew(), drv);
    }

    return new LandingActivity(human, to, drv, Cameraman::GetInst()->FilmLanding(human));
}

//=====================================================================================//
//                       Activity* ActivityFactory::CreateUse()                        //
//=====================================================================================//
Activity* ActivityFactory::CreateUse(BaseEntity* entity, const rid_t& rid, unsigned flags)
{
    Driver* drv = 0;

	if(flags & CT_UPDATE_MPS) drv = new UpdateMPDrv(entity, PoL::Inv::MovepointsForUse, drv);

	if(flags & CT_UPDATE_VIS)
	{
        unsigned int flags = 0;

        flags |= UpdateVisMapDrv::F_UPDATE_ALWAYS;        
        flags |= UpdateVisMapDrv::F_UPDATE_SPECTATOR;

        drv = new UpdateVisMapDrv(entity, flags, drv);
    }

    if(flags & CT_STAY_CHANGE) drv = new StanceChangeDrv(entity, drv);

    unsigned int use_flags = 0;

    if(flags & CT_SWITCH_OBJECT) use_flags |= UseActivity::F_SWITCH_OBJ;
    if(!DirtyLinks::IsElevatorObj(rid)) use_flags |= UseActivity::F_PLAY_USE;
	if(flags & CT_SEND_EVENTS) use_flags |= UseActivity::F_SEND_NOTIFY;

    return new UseActivity(entity->Cast2Human(), rid, use_flags, drv, Cameraman::GetInst()->FilmUse(entity, rid));
}

//=====================================================================================//
//                      Activity* ActivityFactory::CreateThrow()                       //
//=====================================================================================//
Activity* ActivityFactory::CreateThrow(HumanEntity* human, unsigned flags,
									   const shoot_info_s& info, float accuracy)
{
    Driver*    drv = 0;
    BaseThing* thing = human->GetEntityContext()->GetThingInHands(TT_THROWABLE);

    if(flags & CT_UPDATE_MPS) drv = new UpdateMPDrv(human, AIUtils::CalcMps2Act(thing), drv);
       
    if(flags & CT_UPDATE_VIS)
	{
        unsigned flags = 0;

        flags |= UpdateVisMapDrv::F_UPDATE_ONCE;
        flags |= UpdateVisMapDrv::F_UPDATE_SPECTATOR;

        drv = new UpdateVisMapDrv(human, flags, drv);
    }

    if(flags & CT_STAY_CHANGE) drv = new StanceChangeDrv(human, drv);

    ThrowManager* throw_mgr = 0;

    if(thing->GetInfo()->IsGrenade())
	{
        unsigned entity_damager = 0;
        
        entity_damager |= EntityDamager::FT_MAKE_EFFECTS;
        entity_damager |= EntityDamager::FT_SEND_GAME_EVENTS;
        entity_damager |= EntityDamager::FT_PLAY_ENTITY_HURT;
        
        if(flags & CT_HURT_SHOOT)
		{
            entity_damager |= EntityDamager::FT_MAKE_DAMAGES;
            entity_damager |= EntityDamager::FT_PLAY_ENTITY_DEATH;
        }

        if(flags & CT_PRINT_DMGS) entity_damager |= EntityDamager::FT_PRINT_MESSAGES;

        ShootNotifier* notifier = 0;

        if(flags & CT_SEND_EVENTS)
		{
            notifier = new ShootNotifier(info);
        }

        throw_mgr = new GrenadeThrowManager(entity_damager, notifier);
    }
	else if(thing->GetInfo()->GetType() & (TT_CAMERA|TT_SHIELD))
	{
        throw_mgr = new CameraShieldThrowManager();
    }

    Camerawork* cam = Cameraman::GetInst()->FilmThrow(human, info.GetEntity());

    return new ThrowActivity(human, info.GetApproxPos(), accuracy, throw_mgr, drv, cam);
}

//===========================================================================

void Activity::Detach(ActivityObserver* observer)
{ 
    m_observers.Detach(observer);
}

void Activity::Attach(ActivityObserver* observer, ActivityObserver::event_t type)
{ 
    m_observers.Attach(observer, type);
}

void Activity::Notify(ActivityObserver::event_t event, ActivityObserver::info_t info)
{ 
    m_observers.Notify(this, event, info);
}
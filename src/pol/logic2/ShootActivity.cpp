#include "logicdefs.h"

#include "Entity.h"
#include "DirtyLinks.h"
#include "ShootManager.h"
#include "ShootActivity.h"

namespace Game { bool RenderEnabled(); }

//=====================================================================================//
//                           ShootActivity::ShootActivity()                            //
//=====================================================================================//
ShootActivity::ShootActivity(BaseEntity* entity, ShootManager* shoot,
							 const ActivityFactory::shoot_info_s &info, Driver* drv, Camerawork* cam)
:	m_shoot(shoot),
	m_info(info),
	m_dir(info.GetShotAim() - entity->GetGraph()->GetPos3()),
	m_action(GraphEntity::AT_TURN),
	m_entity(entity),
    //m_dir(to - entity->GetGraph()->GetPos3()),
	m_activity_sender(entity),
    BaseActivity(drv, cam),
	m_end_time(0),
	m_flags(0),
	m_pos(entity->GetGraph()->GetPos2())
{
    m_shoot->Init(this, m_entity.get());
    m_entity->Attach(this, EntityObserver::EV_PREPARE_DEATH);


#ifdef _HOME_VERSION
	Timer::Update();
	m_startTime = Timer::GetSeconds();
	TIMING_LOG("Shoot activity\t" << m_startTime << "\t");
#endif
}

//=====================================================================================//
//                           ShootActivity::~ShootActivity()                           //
//=====================================================================================//
ShootActivity::~ShootActivity()
{
#ifdef _HOME_VERSION
	Timer::Update();
	TIMING_LOG(Timer::GetSeconds()-m_startTime << "\n");
#endif

	delete m_shoot;
    DetachObservers();
}

//=====================================================================================//
//                              bool ShootActivity::Run()                              //
//=====================================================================================//
bool ShootActivity::Run(activity_command cmd)
{
	if(m_entity.valid()) m_entity->Accept(*this);        
    
    if(m_flags & F_CALC_STEP)
	{
        cmd = DrvCalcStep(cmd);
        m_flags &= ~F_CALC_STEP;
    }
    
    if(m_flags & F_WAS_SHOOTING && m_flags & F_CALC_MOVEPNTS)
	{
        cmd = DrvCalcMovepnts(cmd);
        m_flags &= ~F_CALC_MOVEPNTS;
    }
    
    if(cmd == AC_STOP) m_shoot->Stop(this, m_entity.get());
    
    return DrvRun(m_shoot->Run(this, m_entity) || (m_entity.valid() && m_action != GraphEntity::AT_STAY));
}

//=====================================================================================//
//                             void ShootActivity::Visit()                             //
//=====================================================================================//
void ShootActivity::Visit(HumanEntity* human)
{
    //еще не время?
    if(m_end_time > Timer::GetSeconds())
		return;

	//while(m_end_time > Timer::GetSeconds())
	//{
	//	Timer::Update();
	//}
    
    //проиграем последовательность анимаций
    switch(m_action)
	{
    case GraphEntity::AT_TURN:
        {
            if(human->GetGraph()->NeedRotate(m_dir))
			{
				m_end_time = human->GetGraph()->SetAngle(Dir2Angle(m_dir));
			}
            else
			{
				m_action = GraphEntity::AT_SHOOT;                
			}
        }
        break;

    case GraphEntity::AT_SHOOT:
        {
            GraphEntity::action_type action =       human->GetEntityContext()->GetShotType() == SHT_AIMSHOT 
                                                ?   GraphEntity::AT_AIMSHOOT
                                                :   GraphEntity::AT_SHOOT;

            m_end_time = human->GetGraph()->ChangeAction(action);
            m_action   = GraphEntity::AT_RECOIL;
        }
        break;
        
    case GraphEntity::AT_RECOIL:
        {
            m_flags |= F_CALC_STEP;

            if(m_shoot->IsDone(this, human))
			{
                m_flags |= F_CALC_MOVEPNTS;
                m_action = GraphEntity::AT_STAY;
                
                GraphEntity::action_type recoil = human->GetEntityContext()->GetShotType() == SHT_AIMSHOT 
                                                    ?   GraphEntity::AT_AIMRECOIL
                                                    :   GraphEntity::AT_RECOIL;
                
                if(m_flags & F_WAS_SHOOTING) m_end_time = human->GetGraph()->ChangeAction(recoil);                                
                return;
            }

			if(m_shoot->Shoot(this, human))
			{
				m_flags |= F_WAS_SHOOTING;
			}
        }
        break;

    case GraphEntity::AT_STAY:
        return;
    }
}

//=====================================================================================//
//                             void ShootActivity::Visit()                             //
//=====================================================================================//
void ShootActivity::Visit(VehicleEntity* vehicle)
{
    //еще не время?
    if(m_end_time > Timer::GetSeconds())
	{
        return;
	}

	//while(m_end_time > Timer::GetSeconds())
	//{
	//	Timer::Update();
	//}
    
    switch(m_action)
	{
    case GraphEntity::AT_TURN:
        {
            if(vehicle->GetGraph()->NeedRotate(m_dir))
                m_end_time = vehicle->GetGraph()->SetAngle(Dir2Angle(m_dir));
            else
                m_action = GraphEntity::AT_SHOOT;                
        }
        break;
        
    case GraphEntity::AT_SHOOT:
        {
            m_end_time = vehicle->GetGraph()->ChangeAction(GraphEntity::AT_SHOOT);
            m_action   = GraphEntity::AT_RECOIL;
        }
        break;

    case GraphEntity::AT_RECOIL:
        {
            m_flags |= F_CALC_STEP;

            //если нет патронов или неверная линия огня
            if(m_shoot->IsDone(this, vehicle))
			{
                m_action = GraphEntity::AT_MOVE;
                vehicle->GetGraph()->ChangeAction(GraphEntity::AT_MOVE);
                vehicle->GetGraph()->MoveTo(m_pos);
                m_flags |= F_CALC_MOVEPNTS;
                return;
            }

            if(m_shoot->Shoot(this, vehicle))
			{
				m_flags |= F_WAS_SHOOTING;
				m_end_time = vehicle->GetGraph()->ChangeAction(GraphEntity::AT_RECOIL);
			}
			//else
			//{
			//	m_action = GraphEntity::AT_MOVE;
			//	vehicle->GetGraph()->ChangeAction(GraphEntity::AT_MOVE);
			//	vehicle->GetGraph()->MoveTo(m_pos);
			//	m_flags |= F_CALC_MOVEPNTS;
			//	return;
			//}
        }
        break;
        
    case GraphEntity::AT_MOVE:
        if(GraphUtils::IsInVicinity(vehicle->GetGraph()->GetPos3(), m_pos, 0.2))
		{
            m_end_time = vehicle->GetGraph()->ChangeAction(GraphEntity::AT_STAY);
            m_action   = GraphEntity::AT_STAY;
        }
        return;
        
    case GraphEntity::AT_STAY:
        return;
    }
}

//=====================================================================================//
//                             void ShootActivity::Visit()                             //
//=====================================================================================//
void ShootActivity::Visit(TraderEntity* trader)
{
    throw CASUS("ShootActivity: торговцы не умеют стрелять!");
}

//=====================================================================================//
//                            void ShootActivity::Update()                             //
//=====================================================================================//
void ShootActivity::Update(BaseEntity* entity, event_t event, info_t info)
{
    DetachObservers();
}

//=====================================================================================//
//                        void ShootActivity::DetachObservers()                        //
//=====================================================================================//
void ShootActivity::DetachObservers()
{
    if(m_entity.valid())
	{
        m_entity->Detach(this);
        m_entity.reset();
    }
}

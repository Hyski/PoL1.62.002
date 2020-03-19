#include "logicdefs.h"

#include "Entity.h"
#include "HexGrid.h"
#include "Graphic.h"
#include "Damager.h"
#include "PassDrv.h"
#include "Activity.h"
#include "PathUtils.h"
#include "DirtyLinks.h"
#include "EnemyDetection.h"

//=====================================================================================//
//                                 PassDrv::PassDrv()                                  //
//=====================================================================================//
PassDrv::PassDrv(BaseEntity* entity, const pnt_vec_t& path, unsigned flags, Driver* drv)
:	m_entity(entity),
	DecoratorDrv(drv),
	m_path(path),
	m_activity(0),
	m_flags(flags)
{
	if(m_flags & F_CALC_DANGER_HEXES)
	{
		ipnt2_t pos = entity->GetGraph()->GetPos2();

		if(HexGrid::GetInst()->Get(pos).GetRisk())
		{
			m_flags &= ~F_STOP_ON_DANGER_HEXES;
		}
		else
		{
			m_flags |= F_STOP_ON_DANGER_HEXES;
		}
	}
}

//=====================================================================================//
//                        activity_command PassDrv::CalcStep()                         //
//=====================================================================================//
activity_command PassDrv::CalcStep(Activity* activity, activity_command user_cmd)
{
	m_activity = activity;
	m_command = user_cmd;

	if(m_path.size()) m_entity->Accept(*this);

	return DecoratorDrv::CalcStep(activity, m_command);
}

//=====================================================================================//
//                                void PassDrv::Visit()                                //
//=====================================================================================//
void PassDrv::Visit(HumanEntity* human)
{
	pnt_vec_t track;

	PathUtils::CalcTrackField(m_entity, m_path.back(), &track);

	AreaManager area(track);
	AreaManager::iterator itor = area.begin();

	if(itor != area.end())
	{
		m_command = AC_STOP;
		ActivityObserver::meet_entity_s info(&*itor);
		m_activity->Notify(ActivityObserver::EV_MEET_ENTITY, &info);
	}

	RiskSite* risk = HexGrid::GetInst()->Get(human->GetGraph()->GetPos2()).GetRisk();

	if(risk && m_flags & F_CALC_DANGER_HEXES)
	{
		human->GetEntityContext()->SetFlameSteps(human->GetEntityContext()->GetFlameSteps() + 1);

		if((human->GetEntityContext()->GetFlameSteps() % GROUND_FIRE_PATH_LENGTH) == 0)
		{
			if(m_flags & F_STOP_ON_DANGER_HEXES)
			{
				m_command = AC_STOP;
				DirtyLinks::Print( mlprintf( DirtyLinks::GetStrRes("act_danger_zone").c_str(), m_entity->GetInfo()->GetName().c_str()));
			}

			//~~~~~~~~~~~~ временное решение ~~~~~~~~~~~~~~

			unsigned dmg_flags = EntityDamager::DEFAULT_FLAGS;
			dmg_flags &= ~EntityDamager::FT_PLAY_ENTITY_HURT;

			//~~~~~~~~~~~~ временное решение ~~~~~~~~~~~~~~

			EntityDamager::GetInst()->Damage(human, risk, dmg_flags);
			m_activity->Notify(ActivityObserver::EV_DANGER_ZONE);
		}
	}

	if(risk == 0) m_flags |= F_STOP_ON_DANGER_HEXES;

	m_path.pop_back();
}

//=====================================================================================//
//                                void PassDrv::Visit()                                //
//=====================================================================================//
void PassDrv::Visit(VehicleEntity* vehicle)
{
	pnt_vec_t track;

	PathUtils::CalcTrackField(m_entity, m_path.back(), &track);

	AreaManager area(track);
	AreaManager::iterator itor = area.begin();

	while(itor != area.end())
	{
		BaseEntity* entity = &*itor;

		ActivityObserver::meet_entity_s info(entity);
		m_activity->Notify(ActivityObserver::EV_MEET_ENTITY, &info);

		if(     entity->Cast2Vehicle()
			||  vehicle->GetInfo()->IsRobot()
			|| (m_flags & F_STOP_BEFORE_HUMAN && entity->Cast2Human())
			|| (m_flags & F_STOP_BEFORE_ALLY && entity->GetPlayer() == vehicle->GetPlayer())
			|| (vehicle->GetPlayer() == PT_PLAYER && !EnemyDetector::getInst()->isHeEnemy(entity))
			)
			m_command = AC_STOP;
		else
			entity->GetEntityContext()->PlayDeath(vehicle);

		++itor;
	}

	m_path.pop_back();
}

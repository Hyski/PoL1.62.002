#include "logicdefs.h"

#include "Entity.h"
#include "AIUtils.h"
#include "Graphic.h"
#include "RotateActivity.h"

//=====================================================================================//
//                              static int GetMovepnts()                               //
//=====================================================================================//
static int GetMovepnts(BaseEntity *e)
{
	if(e->Cast2Human()) return e->Cast2Human()->GetEntityContext()->GetTraits()->GetMovepnts();
	if(e->Cast2Vehicle()) return e->Cast2Vehicle()->GetEntityContext()->GetMovepnts();
	return 0;
}

//=====================================================================================//
//                             static int RemoveMovepnts()                             //
//=====================================================================================//
static void RemoveMovepnts(BaseEntity *e, int p)
{
	if(e->Cast2Human()) e->Cast2Human()->GetEntityContext()->GetTraits()->PoLAddMovepnts(p);
	if(e->Cast2Vehicle())
	{
		VehicleContext *vc = e->Cast2Vehicle()->GetEntityContext();
		vc->PoLSetMovepnts(vc->GetMovepnts()+p);
	}
}

//=====================================================================================//
//                          RotateActivity::RotateActivity()                           //
//=====================================================================================//
RotateActivity::RotateActivity(BaseEntity* entity, float angle, int maxMovePnts,
							   Driver* drv, Camerawork* cam, bool keepMovepnts)
:	BaseActivity(drv, cam),
	m_entity(entity),
	m_step_delta(PI/18),
	m_angle(angle),
	m_step_last(0),
	m_mps_delta(PoL::Inv::TurnAngle4OneMp),
	m_mps_last(0),
	m_end_time(-1),
	m_activity_sender(entity),
	m_maxMovePnts(maxMovePnts),
	m_keepMovepnts(keepMovepnts)
{
	assert( m_maxMovePnts <= AIUtils::CalcMps4EntityRotate(PIm2) );
#ifdef _HOME_VERSION
	Timer::Update();
	m_startTime = Timer::GetSeconds();
	TIMING_LOG("Rotate activity\t" << m_startTime << "\t");
#endif
}

//=====================================================================================//
//                          RotateActivity::~RotateActivity()                          //
//=====================================================================================//
RotateActivity::~RotateActivity()
{
#ifdef _HOME_VERSION
	Timer::Update();
	TIMING_LOG(Timer::GetSeconds() - m_startTime << "\n");
#endif
}

//=====================================================================================//
//                             bool RotateActivity::Run()                              //
//=====================================================================================//
bool RotateActivity::Run(activity_command user_cmd)
{
#ifdef _HOME_VERSION
	Timer::Update();
	if(Timer::GetSeconds() - m_startTime > 0.3f && isHidMovScreen())
	{
		int a = 0;
	}
#endif

	//вызываемся в первый раз?
	if(m_end_time < 0)
	{
		if(!UnlimitedMoves() && (!m_keepMovepnts && GetMovepnts(m_entity) <= 0))
		{
			m_end_time = 0;
			Notify(ActivityObserver::EV_MOVEPNTS_EXPIRIED,0);
			return DrvRun(m_end_time > Timer::GetSeconds());
		}

		//if(DrvCalcMovepnts(AC_TICK) == AC_STOP)
		//{
		//	m_end_time = 0;
		//	return DrvRun(m_end_time > Timer::GetSeconds());
		//}

		m_mps_last = m_step_last = m_entity->GetGraph()->GetAngle();
		m_end_time = m_entity->GetGraph()->SetAngle(m_angle);                 
	}

	float current = m_entity->GetGraph()->GetAngle();

	if(fabsf(current - m_step_last) > m_step_delta)
	{
		m_step_last = current;
		user_cmd = DrvCalcStep(user_cmd); 
		if(user_cmd == AC_STOP) Stop();
	}

	if((fabsf(current - m_mps_last) > m_mps_delta) && m_maxMovePnts > 0)
	{
		--m_maxMovePnts;
		m_mps_last = current;
		user_cmd = DrvCalcMovepnts(user_cmd);
		if(user_cmd == AC_STOP) Stop();
	}

	if(!DrvRun(m_end_time > Timer::GetSeconds()))
	{  
		if(m_maxMovePnts > 0 && m_entity && !m_keepMovepnts)
		{
			RemoveMovepnts(m_entity,-m_maxMovePnts);
			m_maxMovePnts = 0;
		}

		m_entity->GetGraph()->SetLoc(m_entity->GetGraph()->GetPos3(), m_angle);
		DrvCalcStep(AC_TICK);
		return false;
	}

	return true;
}

//=====================================================================================//
//                             void RotateActivity::Stop()                             //
//=====================================================================================//
void RotateActivity::Stop()
{
	m_end_time = 0;
	if(m_maxMovePnts > 0 && m_entity)
	{
		RemoveMovepnts(m_entity,-m_maxMovePnts);
		m_maxMovePnts = 0;
	}

	m_entity->GetGraph()->ChangeAction(GraphEntity::AT_STAY);
}

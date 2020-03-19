#include "logicdefs.h"

#include "Entity.h"
#include "Activity.h"
#include "MPLimitDrv.h"

//=====================================================================================//
//                              MPLimitDrv::MPLimitDrv()                               //
//=====================================================================================//
MPLimitDrv::MPLimitDrv(BaseEntity* entity, Driver* drv)
:	DecoratorDrv(drv),
	m_entity(entity),
	m_cmd(AC_TICK)
{
}

//=====================================================================================//
//                     activity_command MPLimitDrv::CalcMovepnts()                     //
//=====================================================================================//
activity_command MPLimitDrv::CalcMovepnts(Activity* activity, activity_command user_cmd)
{
	if(!UnlimitedMoves())
	{
		m_entity->Accept(*this);
	}

	if(m_cmd == AC_STOP)
	{
		activity->Notify(ActivityObserver::EV_MOVEPNTS_EXPIRIED, 0);
		user_cmd = m_cmd;
	}

	return DecoratorDrv::CalcMovepnts(activity, user_cmd);
}

//=====================================================================================//
//                       activity_command MPLimitDrv::CalcStep()                       //
//=====================================================================================//
activity_command MPLimitDrv::CalcStep(Activity *activity, activity_command user_cmd)
{
	if(!UnlimitedMoves())
	{
		m_entity->Accept(*this);
	}

	if(m_cmd == AC_STOP)
	{
		activity->Notify(ActivityObserver::EV_MOVEPNTS_EXPIRIED, 0);
		user_cmd = m_cmd;
	}

	return DecoratorDrv::CalcStep(activity,user_cmd);
}

//=====================================================================================//
//                              void MPLimitDrv::Visit()                               //
//=====================================================================================//
void MPLimitDrv::Visit(HumanEntity* human)
{
	const int mps = human->GetEntityContext()->GetTraits()->GetMovepnts();
	const int mps4step = human->GetEntityContext()->GetHexCost();
	m_cmd = mps >= mps4step ? AC_TICK : AC_STOP;
}

//=====================================================================================//
//                              void MPLimitDrv::Visit()                               //
//=====================================================================================//
void MPLimitDrv::Visit(VehicleEntity* vehicle)
{
	const int mps = vehicle->GetEntityContext()->GetMovepnts();
	const int mps4step = vehicle->GetEntityContext()->GetHexCost();
	m_cmd = mps >= mps4step ? AC_TICK : AC_STOP;
}

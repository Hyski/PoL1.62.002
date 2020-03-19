#include "logicdefs.h"

#include "Entity.h"
#include "Activity.h"
#include "DirtyLinks.h"
#include "StepLimitDrv.h"

//=====================================================================================//
//                            StepLimitDrv::StepLimitDrv()                             //
//=====================================================================================//
StepLimitDrv::StepLimitDrv(BaseEntity* entity, unsigned flags, Driver* drv)
:	DecoratorDrv(drv),
m_entity(entity),
m_cmd(AC_TICK),
m_flags(flags)
{
}

//=====================================================================================//
//                      activity_command StepLimitDrv::CalcStep()                      //
//=====================================================================================//
activity_command StepLimitDrv::CalcStep(Activity* activity, activity_command user_cmd)
{
	m_entity->Accept(*this);

	if(m_cmd == AC_STOP)
	{
		user_cmd = AC_STOP;
		activity->Notify(ActivityObserver::EV_STEPS_EXPIRIED, 0);
	}

	return DecoratorDrv::CalcStep(activity, user_cmd);
}

//=====================================================================================//
//                             void StepLimitDrv::Visit()                              //
//=====================================================================================//
void StepLimitDrv::Visit(HumanEntity* human)
{
	HumanContext* context = human->GetEntityContext();

	if(context->GetStepsCount())
	{
		return;
	}

	m_cmd = AC_STOP;

	if(m_flags & F_PRINT_MSGS && context->GetTraits()->GetWeight() > context->GetLimits()->GetWeight())
	{
		DirtyLinks::Print( mlprintf( DirtyLinks::GetStrRes("weight_overload").c_str(), human->GetInfo()->GetName().c_str()));        
	}
}

//=====================================================================================//
//                             void StepLimitDrv::Visit()                              //
//=====================================================================================//
void StepLimitDrv::Visit(VehicleEntity* vehicle)
{
	if(vehicle->GetEntityContext()->GetStepsCount())
	{
		return;
	}

	m_cmd = AC_STOP;
}

#include "logicdefs.h"

#include "Entity.h"
#include "DecAccuracyDrv.h"

//=====================================================================================//
//                          DecAccuracyDrv::DecAccuracyDrv()                           //
//=====================================================================================//
DecAccuracyDrv::DecAccuracyDrv(HumanEntity *human, Driver *drv)
:	m_human(human),
	DecoratorDrv(drv),
	m_hex_counter(0)
{
}

//=====================================================================================//
//                          DecAccuracyDrv::~DecAccuracyDrv()                          //
//=====================================================================================//
DecAccuracyDrv::~DecAccuracyDrv() 
{
}

//=====================================================================================//
//                     activity_command DecAccuracyDrv::CalcStep()                     //
//=====================================================================================//
activity_command DecAccuracyDrv::CalcStep(Activity* activity, activity_command user_cmd)
{
	++m_hex_counter;
	return DecoratorDrv::CalcStep(activity, user_cmd);
}

//=====================================================================================//
//                             bool DecAccuracyDrv::Run()                              //
//=====================================================================================//
bool DecAccuracyDrv::Run(Activity* activity, bool result)
{
	if(m_human.valid() && !result)
	{
		HumanContext::Traits *traits = m_human->GetEntityContext()->GetTraits();
		const int accuracy = traits->GetAccuracy();
		const int accuracy_fall = static_cast<int>(PoL::Inv::AccuracyFallFactor * static_cast<float>(m_hex_counter-1));
		const int accuracy_delta = std::min(accuracy,accuracy_fall);
		traits->PoLAddAccuracy(-accuracy_delta);
		m_human->GetEntityContext()->AddAccuracyRunAffect(accuracy_delta);

		m_human.reset();
	}

	return DecoratorDrv::Run(activity, result);
}

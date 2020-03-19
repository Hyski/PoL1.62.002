#include "logicdefs.h"

#include "Entity.h"
#include "Graphic.h"
#include "PathUtils.h"
#include "LandingActivity.h"

//=====================================================================================//
//                         LandingActivity::LandingActivity()                          //
//=====================================================================================//
LandingActivity::LandingActivity(HumanEntity* human, const ipnt2_t& to, Driver* drv, Camerawork* cam)
:	m_activity_sender1(human),
	BaseActivity(drv, cam),
	m_end_time(0),
    m_human(human),
	m_activity_sender2(human->GetEntityContext()->GetCrew())
{
    m_human->GetGraph()->SetLoc(to, m_human->GetGraph()->GetAngle());
        
    float   time1 = m_human->GetGraph()->ChangeAction(GraphEntity::AT_LANDING),
            time2 = m_human->GetEntityContext()->GetCrew()->GetGraph()->ChangeAction(GraphEntity::AT_LANDING);
        
    m_end_time = std::max(time1, time2);
}

//=====================================================================================//
//                             bool LandingActivity::Run()                             //
//=====================================================================================//
bool LandingActivity::Run(activity_command cmd)
{
    DrvCalcStep(cmd);

    if(m_end_time < Timer::GetSeconds() && m_human->GetEntityContext()->GetCrew())
	{
        DrvCalcMovepnts(cmd);

        VehicleEntity* vehicle = m_human->GetEntityContext()->GetCrew();

        vehicle->GetEntityContext()->DecCrew(m_human);
        PathUtils::LinkEntity(m_human);
    }
    
    return DrvRun(m_end_time > Timer::GetSeconds());
}

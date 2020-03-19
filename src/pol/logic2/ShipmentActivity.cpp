#include "logicdefs.h"

#include "Entity.h"
#include "Graphic.h"
#include "PathUtils.h"
#include "ShipmentActivity.h"

//=====================================================================================//
//                        ShipmentActivity::ShipmentActivity()                         //
//=====================================================================================//
ShipmentActivity::ShipmentActivity(HumanEntity* human, VehicleEntity* vehicle, Driver* drv, Camerawork* cam)
:	m_human(human),
	m_vehicle(vehicle),
	BaseActivity(drv, cam),
    m_end_time(0),
	m_activity_sender1(human),
	m_activity_sender2(vehicle)
{
    const float time1 = human->GetGraph()->ChangeAction(GraphEntity::AT_SHIPMENT);
    const float time2 = vehicle->GetGraph()->ChangeAction(GraphEntity::AT_SHIPMENT);

    m_end_time = std::max(time1, time2);
}

//=====================================================================================//
//                            bool ShipmentActivity::Run()                             //
//=====================================================================================//
bool ShipmentActivity::Run(activity_command cmd)
{
    DrvCalcStep(cmd);
    
    if(m_end_time < Timer::GetSeconds() && !m_human->GetEntityContext()->GetCrew())
	{
        DrvCalcMovepnts(cmd);
        PathUtils::UnlinkEntity(m_human);
        m_vehicle->GetEntityContext()->IncCrew(m_human);
    }
    
    return DrvRun(m_end_time > Timer::GetSeconds());
}

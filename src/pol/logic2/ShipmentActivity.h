#pragma once

#include "BaseActivity.h"
#include "ActivityStatusSender.h"

//=====================================================================================//
//                    class ShipmentActivity : public BaseActivity                     //
//=====================================================================================//
// посадка человека в технику
class ShipmentActivity : public BaseActivity
{
    float m_end_time;  
   
    HumanEntity*   m_human;
    VehicleEntity* m_vehicle;

    ActivityStatusSender m_activity_sender1;
    ActivityStatusSender m_activity_sender2;

public:
    ShipmentActivity(HumanEntity* human, VehicleEntity* vehicle, Driver* drv = 0, Camerawork* cam = 0);

	bool Run(activity_command cmd);
};

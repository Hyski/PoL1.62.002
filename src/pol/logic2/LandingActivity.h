#pragma once

#include "BaseActivity.h"
#include "ActivityStatusSender.h"

//=====================================================================================//
//                     class LandingActivity : public BaseActivity                     //
//=====================================================================================//
// Высадка человека из техники
class LandingActivity : public BaseActivity
{
    float        m_end_time;
    HumanEntity* m_human;

    ActivityStatusSender m_activity_sender1;
    ActivityStatusSender m_activity_sender2;

public:
    LandingActivity(HumanEntity* human, const ipnt2_t& to, Driver* drv = 0, Camerawork* cam = 0);

    bool Run(activity_command cmd);
};

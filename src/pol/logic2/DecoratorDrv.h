#pragma once

#include "Driver.h"

//=====================================================================================//
//                         class DecoratorDrv : public Driver                          //
//=====================================================================================//
// драйвер - декоратор
class DecoratorDrv : public Driver
{
public:
    DecoratorDrv(Driver* drv = 0) : m_drv(drv) {}
    ~DecoratorDrv() { delete m_drv; }

    activity_command CalcStep(Activity* activity, activity_command user_cmd)
    {
        return m_drv ? m_drv->CalcStep(activity, user_cmd) : user_cmd;
    }

    activity_command CalcMovepnts(Activity* activity, activity_command user_cmd)
    {
        return m_drv ? m_drv->CalcMovepnts(activity, user_cmd) : user_cmd;
    }
    
    bool Run(Activity* activity, bool result)
    {
        return m_drv ? m_drv->Run(activity, result) : result;
    }

private:
    Driver* m_drv;
};

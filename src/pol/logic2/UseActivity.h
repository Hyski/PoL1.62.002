#pragma once

#include "BaseActivity.h"
#include "GameObserver.h"

//=====================================================================================//
//                       class UseActivity : public BaseActivity                       //
//=====================================================================================//
// use на объекте
class UseActivity : public BaseActivity
{
    unsigned m_flags; 
    float m_end_time;
	bool m_firstTime;

    GameObserver::use_info m_info;

public:
    enum flag_type
	{
        F_PLAY_USE    = 1 << 0, 
        F_SWITCH_OBJ  = 1 << 1,
        F_SEND_NOTIFY = 1 << 2,
    };

    UseActivity(HumanEntity* human, const rid_t& rid, unsigned flags, Driver* drv = 0, Camerawork* cam = 0);

    bool Run(activity_command cmd);
};

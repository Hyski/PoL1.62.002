#pragma once

#include "Graphic.h"
#include "BaseActivity.h"
#include "ActivityStatusSender.h"

class ThrowManager;

//=====================================================================================//
//                      class ThrowActivity : public BaseActivity                      //
//=====================================================================================//
// бросок гранаты
class ThrowActivity : public BaseActivity
{
    unsigned m_flags;

    point3 m_to;

    float m_accuracy;
    float m_end_time;

    HumanEntity*  m_human;
    ThrowManager* m_throw;

    GraphEntity::action_type m_action;
    ActivityStatusSender m_activity_sender;

#ifdef _HOME_VERSION
	float m_startTime;
#endif

public:
    enum
	{
        F_CALC_STEP     = 1 << 0 ,
        F_CALC_MOVEPNTS = 1 << 1,
    };

	ThrowActivity(HumanEntity* human, const point3& to, float accuracy, ThrowManager* mgr, Driver* drv = 0, Camerawork* cam = 0);
    ~ThrowActivity();

    bool Run(activity_command cmd);

private:
    //пориграть бросок гранаты
    void PlayThrow();
};

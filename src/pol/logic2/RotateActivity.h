#pragma once

#include "BaseActivity.h"
#include "ActivityStatusSender.h"

//=====================================================================================//
//                     class RotateActivity : public BaseActivity                      //
//=====================================================================================//
// поворот сущесва
class RotateActivity : public BaseActivity
{
    float m_angle;
    float m_end_time;
    float m_mps_last;
    float m_step_last;

    const float m_mps_delta;
    const float m_step_delta;
	int m_maxMovePnts;

	bool m_keepMovepnts;

    BaseEntity* m_entity;

    ActivityStatusSender m_activity_sender;

#ifdef _HOME_VERSION
	float m_startTime;
#endif

public:
    RotateActivity(BaseEntity* entity, float angle, int maxmovepnts, Driver* drv = 0, Camerawork* cam = 0, bool keepMovepnts = false);
	~RotateActivity();
    bool Run(activity_command user_cmd);

private:
    void Stop();
};

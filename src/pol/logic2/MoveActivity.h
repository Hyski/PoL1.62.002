#pragma once

#include "Graphic.h"
#include "BaseActivity.h"
#include "ActivityStatusSender.h"
#include <undercover/skin/RealEntityMover.h>

//=====================================================================================//
//                                 class MoveActivity                                  //
//=====================================================================================//
// перемещение существом
class MoveActivity
:	public BaseActivity,
	private PoL::MovementController,
	private EntityObserver
{
    bool m_hasEnded;
    bool m_first_time;

    pnt_vec_t m_path;
	PoL::BaseEntityRef m_entity;

    activity_command m_usr_cmd;

    ActivityStatusSender m_activity_sender;
	bool m_noInterrupt;

	bool setEnded(bool val);
	bool hasEnded() const { return m_hasEnded; }

public:
    MoveActivity(BaseEntity *, const pnt_vec_t &, Driver * = 0, Camerawork * = 0, bool noInterrupt = false);
    ~MoveActivity();

	bool Run(activity_command cmd);
	Command onStep();
	void onEnd();

	bool needSpeedup() { return m_usr_cmd == AC_SPEED_UP; }

private:
	virtual void Update(subject_t, event_t, info_t);
};

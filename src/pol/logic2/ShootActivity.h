#pragma once

#include "Graphic.h"
#include "EntityRef.h"
#include "EntityAux.h"
#include "BaseActivity.h"
#include "ActivityFactory.h"
#include "ActivityStatusSender.h"

class ShootManager;

//=====================================================================================//
//                                 class ShootActivity                                 //
//=====================================================================================//
// стрельба для существа
class ShootActivity
:	public BaseActivity,
	private EntityObserver,
	private EntityVisitor
{
    ipnt2_t  m_pos;
	ActivityFactory::shoot_info_s m_info;
	point3   m_dir;
    
    unsigned int m_flags;
    float    m_end_time;

	float m_fall_off;

	PoL::BaseEntityRef m_entity;   
    ShootManager* m_shoot;

    GraphEntity::action_type m_action;

    ActivityStatusSender m_activity_sender;

#ifdef _HOME_VERSION
	float m_startTime;
#endif

public:
    enum
	{
        F_CALC_STEP     = 1 << 0,
        F_CALC_MOVEPNTS = 1 << 1,
        F_WAS_SHOOTING  = 1 << 2,
    };

	ShootActivity(BaseEntity* entity, ShootManager* shoot, const ActivityFactory::shoot_info_s &info, Driver* drv = 0, Camerawork* cam = 0);
    ~ShootActivity();

    bool Run(activity_command cmd);

protected:
    virtual bool DrvRun(bool result)
    {
        result = m_driver ? m_driver->Run(this, result) : result;
        return m_camerawork && (m_flags&F_WAS_SHOOTING) ? m_camerawork->Film(result) : result;
    }

private:
    void Visit(HumanEntity* human);
    void Visit(VehicleEntity* vehicle);
    void Visit(TraderEntity* trader);
    void Update(BaseEntity* entity, event_t event, info_t info);
    void DetachObservers();
};

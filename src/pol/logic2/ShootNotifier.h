#pragma once

#include "EntityAux.h"
#include "ActivityFactory.h"

//=====================================================================================//
//                    class ShootNotifier : private EntityObserver                     //
//=====================================================================================//
// реализация уведомлений о стрельбе
class ShootNotifier : private EntityObserver
{
	ActivityFactory::shoot_info_s m_info;

public:
	ShootNotifier(const ActivityFactory::shoot_info_s &info);
    ~ShootNotifier();

    void SendShoot(BaseEntity* actor, const point3& pos);
    void SendHit(BaseEntity* actor, const point3& pos, BaseEntity* victim, const rid_t& effect);

private:
    void Update(subject_t subject, event_t event, info_t info);
};

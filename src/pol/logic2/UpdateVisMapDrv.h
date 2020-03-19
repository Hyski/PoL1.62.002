#pragma once

#include "EntityAux.h"
#include "DecoratorDrv.h"

//=====================================================================================//
//         class UpdateVisMapDrv : public DecoratorDrv, private EntityObserver         //
//=====================================================================================//
// Update карты видимости
class UpdateVisMapDrv : public DecoratorDrv, private EntityObserver
{
    unsigned int m_flags;
    BaseEntity *m_entity;

public:
    enum
	{
        F_UPDATE_MARKER    = 1 << 0,
        F_UPDATE_SPECTATOR = 1 << 1,

        F_UPDATE_ALWAYS = 1 << 2,
        F_UPDATE_ONCE   = 1 << 3,   
    };

    UpdateVisMapDrv(BaseEntity* entity, unsigned flags = 0, Driver* drv = 0);
    ~UpdateVisMapDrv();

    activity_command CalcStep(Activity* activity, activity_command user_cmd);

private:
    void UpdateSpectatorAndMarker();
    void Update(subject_t subj, event_t event, info_t info);
};

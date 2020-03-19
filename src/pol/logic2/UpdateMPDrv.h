#pragma once

#include "EntityAux.h"
#include "DecoratorDrv.h"

//=====================================================================================//
//           class UpdateMPDrv : public DecoratorDrv, private EntityVisitor            //
//=====================================================================================//
// драйвер обновления movepnts
class UpdateMPDrv : public DecoratorDrv, private EntityVisitor
{
    int m_moves;
    BaseEntity *m_entity;

public:
    UpdateMPDrv(BaseEntity* entity, int moves, Driver* drv = 0);

    activity_command CalcMovepnts(Activity* activity, activity_command user_cmd);

private:
    void Visit(HumanEntity* human);
    void Visit(VehicleEntity* vehicle);
};

#pragma once

#include "EntityAux.h"
#include "DecoratorDrv.h"

//=====================================================================================//
//                                 class StepLimitDrv                                  //
//=====================================================================================//
// ограничитель действия по кол-ву шагов
class StepLimitDrv
:	public DecoratorDrv,
	private EntityVisitor
{
    unsigned int m_flags;
    BaseEntity *m_entity;
    activity_command m_cmd;

public:
    enum
	{
        F_PRINT_MSGS = 1 << 0,  //вывод сообщений в лог
    };

    StepLimitDrv(BaseEntity* entity, unsigned flags, Driver* drv);

    activity_command CalcStep(Activity* activity, activity_command user_cmd);

private:
    void Visit(HumanEntity* human);
    void Visit(VehicleEntity* vehicle);
};

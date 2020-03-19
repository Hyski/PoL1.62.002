#pragma once

#include "EntityAux.h"
#include "DecoratorDrv.h"

//=====================================================================================//
//                                  class MPLimitDrv                                   //
//=====================================================================================//
// ограничитель действия по кол-ву ходов
class MPLimitDrv
:	public DecoratorDrv,
	private EntityVisitor
{
	activity_command m_cmd;
	BaseEntity *m_entity;

public:
	MPLimitDrv(BaseEntity* entity, Driver* drv = 0);

	activity_command CalcStep(Activity* activity, activity_command user_cmd);
	activity_command CalcMovepnts(Activity* activity, activity_command user_cmd);

private:
	void Visit(HumanEntity* human);
	void Visit(VehicleEntity* vehicle);
};

#pragma once

#include "EntityAux.h"
#include "DecoratorDrv.h"

//=====================================================================================//
//                                    class PassDrv                                    //
//=====================================================================================//
// декоратор для проверки проходмости
class PassDrv
:	public DecoratorDrv,
	private EntityVisitor
{
	activity_command m_command;

	pnt_vec_t m_path;
	Activity *m_activity;
	BaseEntity *m_entity;

	unsigned int m_flags;

public:
	enum flags_type
	{
		F_STOP_BEFORE_HUMAN    = 1 << 0,
		F_STOP_BEFORE_ALLY     = 1 << 1,
		F_CALC_DANGER_HEXES    = 1 << 2,
		F_STOP_ON_DANGER_HEXES = 1 << 3
	};

	PassDrv(BaseEntity* entity, const pnt_vec_t& path, unsigned flags, Driver* drv = 0);

	activity_command CalcStep(Activity* activity, activity_command user_cmd);

private:
	void Visit(HumanEntity* human);
	void Visit(VehicleEntity* vehicle);
};

#pragma once

#include "EntityAux.h"
#include "EntityRef.h"
#include "DecoratorDrv.h"

//=====================================================================================//
//                                class DecAccuracyDrv                                 //
//=====================================================================================//
// уменьшение точноcти при беге
class DecAccuracyDrv : public DecoratorDrv
{
    int m_hex_counter;
	PoL::HumanEntityRef m_human;

public:
	DecAccuracyDrv(HumanEntity *human, Driver *drv);
	~DecAccuracyDrv();

    activity_command CalcStep(Activity *activity, activity_command user_cmd);
    bool Run(Activity* activity, bool result);
};

#pragma once

#include "ActivityCommand.h"

class Activity;

//=====================================================================================//
//                                    class Driver                                     //
//=====================================================================================//
// драйвер - класс управляющий действием
class Driver
{
public:
    virtual ~Driver() {}

    //перехват выполнения действия
    virtual bool Run(Activity* activity, bool activity_result) = 0;

    //расчет одного элементарного шага действия
    virtual activity_command CalcStep(Activity* activity, activity_command user_cmd) = 0;
    //расчет ходов во время действия
    virtual activity_command CalcMovepnts(Activity* activity, activity_command user_cmd) = 0;
};

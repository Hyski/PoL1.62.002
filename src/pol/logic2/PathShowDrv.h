#pragma once

#include "DecoratorDrv.h"

//=====================================================================================//
//                       class PathShowDrv : public DecoratorDrv                       //
//=====================================================================================//
// декоратор показывающий путь
class PathShowDrv : public DecoratorDrv
{
    pnt_vec_t m_path;
	bool m_firstTime;

public:
    PathShowDrv(const pnt_vec_t& path, Driver* drv);
    ~PathShowDrv();

    activity_command CalcStep(Activity* activity, activity_command user_cmd);
    bool Run(Activity* activity, bool result);

private:
    void ShowPath();
};

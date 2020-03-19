#pragma once

#include "Driver.h"
#include "Activity.h"
#include "Cameraman.h"

#ifdef _HOME_VERSION
#define TIMING_LOG(L)	MLL_MAKE_LOG("poltime.log",L)
#else
#define TIMING_LOG(L)
#endif

//=====================================================================================//
//                        class BaseActivity : public Activity                         //
//=====================================================================================//
// действие обладающее общей функциональностью
class BaseActivity : public Activity
{
public:
    BaseActivity(Driver* driver = 0, Camerawork* camera = 0)
	:	m_driver(driver),
		m_camerawork(camera)
	{
	}

    ~BaseActivity()
    { 
        delete m_driver;
        delete m_camerawork;
    }

protected:
    virtual bool DrvRun(bool result)
    {
        result = m_driver ? m_driver->Run(this, result) : result;
        return m_camerawork ? m_camerawork->Film(result) : result;
    }

    activity_command DrvCalcStep(activity_command user_cmd)
    {
        return m_driver ? m_driver->CalcStep(this, user_cmd) : user_cmd;
    }

    activity_command DrvCalcMovepnts(activity_command user_cmd)
    {
        return m_driver ? m_driver->CalcMovepnts(this, user_cmd) : user_cmd;
    }

protected:
    Driver*  m_driver;
    Camerawork* m_camerawork;
};

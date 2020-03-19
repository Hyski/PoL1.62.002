#include "logicdefs.h"

#include "Entity.h"
#include "Graphic.h"
#include "DirtyLinks.h"
#include "UseActivity.h"

//=====================================================================================//
//                             UseActivity::UseActivity()                              //
//=====================================================================================//
UseActivity::UseActivity(HumanEntity* human, const rid_t& rid, unsigned flags, Driver* drv, Camerawork* cam)
:	m_end_time(0),
	BaseActivity(drv, cam),
	m_info(human, rid),
	m_flags(flags),
	m_firstTime(true)
{
}

//=====================================================================================//
//                               bool UseActivity::Run()                               //
//=====================================================================================//
bool UseActivity::Run(activity_command cmd)
{
    DrvCalcStep(cmd);
    
    while(m_end_time < Timer::GetSeconds())
	{
        //сначала играем анимацию USE
        if(m_flags & F_PLAY_USE)
		{
            m_flags &= ~F_PLAY_USE;
            m_end_time = m_info.m_actor->GetGraph()->ChangeAction(GraphHuman::AT_USE);
            break;
        }

        if(m_flags & F_SWITCH_OBJ)
		{
            //сбросить флаг
            m_flags &= ~F_SWITCH_OBJ;

            //уведомить о USE'е после проигрывания анимации
            m_flags |= F_SEND_NOTIFY;

            m_end_time = DirtyLinks::SwitchObjState(m_info.m_obj4use);                
            break;
        }
        
        if(m_flags & F_SEND_NOTIFY)
		{
			GameEvMessenger::GetInst()->Notify(GameObserver::EV_USE, &m_info);
		}

        break;
    }

	if(m_firstTime)
	{
		m_firstTime = false;
		DrvCalcMovepnts(cmd);
	}

    return DrvRun(m_end_time > Timer::GetSeconds());
}

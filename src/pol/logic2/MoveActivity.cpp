#include "logicdefs.h"

#include "Entity.h"
#include "PathUtils.h"
#include "MoveActivity.h"

//=====================================================================================//
//                            MoveActivity::MoveActivity()                             //
//=====================================================================================//
MoveActivity::MoveActivity(BaseEntity *entity, const pnt_vec_t& path, Driver* drv,
						   Camerawork* cam, bool noInterrupt)
:	m_entity(entity),
	m_first_time(true),
	m_activity_sender(entity),
    m_hasEnded(true),
	BaseActivity(drv, cam),
	m_usr_cmd(AC_TICK),
	m_path(path),
	m_noInterrupt(noInterrupt)
{
	if(cam)
	{
		cam->Prepare();
	}

	if(m_entity.valid())
	{
		m_entity->Attach(this,EV_PREPARE_DEATH);
	}
}

//=====================================================================================//
//                            MoveActivity::~MoveActivity()                            //
//=====================================================================================//
MoveActivity::~MoveActivity()
{  
}

//=====================================================================================//
//                            bool MoveActivity::setEnded()                            //
//=====================================================================================//
bool MoveActivity::setEnded(bool val)
{
	assert( !m_hasEnded || val );

	if(val != m_hasEnded && !m_hasEnded)
	{
		m_hasEnded = val;
	}

	return m_hasEnded;
}

//=====================================================================================//
//                              bool MoveActivity::Run()                               //
//=====================================================================================//
bool MoveActivity::Run(activity_command cmd)
{
	if(m_first_time)
	{
        m_first_time = !m_first_time;
        
        PathUtils::UnlinkEntity(m_entity.get());

        if(DrvCalcStep(AC_TICK) == AC_STOP)
		{
            if(m_entity.valid())
			{
				PathUtils::LinkEntity(m_entity.get());
			}

            return DrvRun(!hasEnded());
        }

		m_hasEnded = m_entity.valid() ? false : true;//grom
		if(m_entity.valid())
		{
			if(m_noInterrupt)
			{
				m_entity->GetGraph()->PoLMove(m_path, this);
			}
			else
			{
				m_entity->GetGraph()->Move(m_path, this);
			}
		}
	}

	// stop и speedup нельзя отменить
	if(m_usr_cmd != AC_STOP && cmd != AC_TICK && m_usr_cmd != AC_SPEED_UP)
	{
		m_usr_cmd = cmd;
	}

    return DrvRun(!hasEnded() && m_entity.valid());
}

//=====================================================================================//
//                           cmd_type MoveActivity::OnStep()                           //
//=====================================================================================//
MoveActivity::Command MoveActivity::onStep(/*MoveEvents::event_type event*/)
{
	if(DrvCalcMovepnts(m_usr_cmd) == AC_STOP)
	{
        return cmdStop;
	}

	if(DrvCalcStep(m_usr_cmd) == AC_STOP)
	{
        return cmdStop;
	}

	return cmdContinue;
}

//=====================================================================================//
//                             void MoveActivity::onEnd()                              //
//=====================================================================================//
void MoveActivity::onEnd()
{
	if(m_entity.valid())
	{
		PathUtils::LinkEntity(m_entity.get());
	}

	setEnded(true);
}

//=====================================================================================//
//                             void MoveActivity::Update()                             //
//=====================================================================================//
void MoveActivity::Update(subject_t subj, event_t ev, info_t info)
{
	m_entity->Detach(this);
	m_entity.reset();
}

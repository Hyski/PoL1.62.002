#include "logicdefs.h"

#include "Entity.h"
#include "HexGrid.h"
#include "PathUtils.h"
#include "ParMoveActivity.h"
#include "MultiMoveActivity.h"

#define PARMOVE_LOG(L)		MLL_MAKE_DEBUG_LOG("parmove.log",L)

namespace PoL
{

ParMoveActivity::OnRun_t ParMoveActivity::m_onRun[] =
{
	runInit,
	runMoving,
	runStopping,
	runEnded
};

ParMoveActivity::OnStep_t ParMoveActivity::m_onStep[] =
{
	onStepInit,
	onStepMoving,
	onStepStopping,
	onStepEnded,
};

//=====================================================================================//
//                         ParMoveActivity::ParMoveActivity()                          //
//=====================================================================================//
ParMoveActivity::ParMoveActivity(MultiMoveActivity *mmact, const TrackInfo &info)
:	BaseActivity(info.m_driver,0),
	m_mmact(mmact),
	m_entity(info.m_entity),
	m_path(info.m_path),
	m_state(sInit),
	m_unlinked(false),
	m_curHex(0),
	m_initPos(m_entity->GetGraph()->GetPos2()),
	m_paused(false),
	m_blocker(0)
{
	PARMOVE_LOG(m_entity->GetInfo()->GetRID() << " started parmove\n");
	unlink();
}

//=====================================================================================//
//                         ParMoveActivity::~ParMoveActivity()                         //
//=====================================================================================//
ParMoveActivity::~ParMoveActivity()
{
	PARMOVE_LOG(m_entity->GetInfo()->GetRID() << " ended parmove\n");
	link();
}

//=====================================================================================//
//                const ipnt2_t &ParMoveActivity::getCurHexPos() const                 //
//=====================================================================================//
const ipnt2_t &ParMoveActivity::getCurHexPos() const
{
	if(m_curHex == 0)
	{
		return m_initPos;
	}
	else if(m_curHex+1 > m_path.size())
	{
		return m_path.front();
	}
	else
	{
		return *(m_path.rbegin() + (m_curHex-1));
		//return m_path[m_path.size()-m_curHex-1];
	}
}

//=====================================================================================//
//                const ipnt2_t &ParMoveActivity::getNextHexPos() const                //
//=====================================================================================//
const ipnt2_t &ParMoveActivity::getNextHexPos() const
{
		return *(m_path.rbegin() + (m_curHex-1) + 1);
}

//=====================================================================================//
//                            void ParMoveActivity::link()                             //
//=====================================================================================//
void ParMoveActivity::link()
{
	if(m_unlinked && m_entity.valid())
	{
		assert( m_entity->GetGraph()->GetPos2() == getCurHexPos() );
		PathUtils::LinkEntity(m_entity.get(),getCurHexPos());
		m_unlinked = false;
	}
}

//=====================================================================================//
//                           void ParMoveActivity::unlink()                            //
//=====================================================================================//
void ParMoveActivity::unlink()
{
	if(!m_unlinked && m_entity.valid())
	{
		PathUtils::UnlinkEntity(m_entity.get());
		m_unlinked = true;
	}
}

//=====================================================================================//
//                             bool ParMoveActivity::Run()                             //
//=====================================================================================//
bool ParMoveActivity::Run(activity_command cmd)
{
	if(!m_entity.valid())
	{
		m_state = sEnded;
		return false;
	}

	bool ret = true;
	
	do
	{
		ret = (this->*m_onRun[m_state])(cmd);
	}
	while(!ret);

	return m_result;
}

//=====================================================================================//
//                 ParMoveActivity::cmd_type ParMoveActivity::OnStep()                 //
//=====================================================================================//
ParMoveActivity::Command ParMoveActivity::onStep()
{
	return (this->*m_onStep[m_state])();
}

//=====================================================================================//
//                            void ParMoveActivity::onEnd()                            //
//=====================================================================================//
void ParMoveActivity::onEnd()
{
	m_state = sEnded;
}

//=====================================================================================//
//                           bool ParMoveActivity::runInit()                           //
//=====================================================================================//
bool ParMoveActivity::runInit(activity_command cmd)
{
	bool ret = true;

	if(DrvCalcStep(AC_TICK) == AC_STOP)
	{
		m_state = sEnded;
		ret = false;
	}
	else
	{
		if(!m_entity.valid())
		{
			m_state = sEnded;
		}
		else
		{
			m_state = sMoving;
			m_entity->GetGraph()->Move(m_path,this);
		}
	}

	m_result = DrvRun(m_entity.valid());

	return ret;
}

//=====================================================================================//
//                          bool ParMoveActivity::runMoving()                          //
//=====================================================================================//
bool ParMoveActivity::runMoving(activity_command cmd)
{
	if(cmd == AC_SPEED_UP) cmd = AC_TICK;

	// stop и speedup нельз€ отменить
	if(cmd != AC_TICK || !m_entity.valid())
	{
		m_state = sStopping;
	}

	m_result = DrvRun(m_entity.valid());
	return true;
}

//=====================================================================================//
//                         bool ParMoveActivity::runStopping()                         //
//=====================================================================================//
bool ParMoveActivity::runStopping(activity_command cmd)
{
	m_result = true;
	return true;
}

//=====================================================================================//
//                          bool ParMoveActivity::runEnded()                           //
//=====================================================================================//
bool ParMoveActivity::runEnded(activity_command cmd)
{
	m_result = false;
	return true;
}

//=====================================================================================//
//               ParMoveActivity::cmd_type ParMoveActivity::onStepInit()               //
//=====================================================================================//
ParMoveActivity::Command ParMoveActivity::onStepInit()
{
	assert( !"ParMoveActivity::onStepInit. —юда мы не должны заходить!!!" );
	return cmdContinue;
}

//=====================================================================================//
//                    bool ParMoveActivity::isCircularDependency()                     //
//=====================================================================================//
bool ParMoveActivity::isCircularDependency()
{
	typedef std::vector<ParMoveActivity*> Acts_t;
	Acts_t acts;

	ParMoveActivity *cur = this;

	for(; cur; cur = cur->m_blocker)
	{
		if(std::find(acts.begin(),acts.end(),cur) != acts.end())
		{
			return true;
		}
		acts.push_back(cur);
	}

	return false;
}

//=====================================================================================//
//         ParMoveActivity::MotionCommand ParMoveActivity::canMoveToNextHex()          //
//=====================================================================================//
ParMoveActivity::MotionCommand ParMoveActivity::canMoveToNextHex()
{
	MultiMoveActivity::ObsInfo_t ot = m_mmact->getObstructorType(this);
	m_blocker = ot.second;

	switch(ot.first)
	{
		case MultiMoveActivity::otMoving:
			if(isCircularDependency())
			{
				if(m_paused)
				{
					m_paused = false;
					return mcUnpause;
				}
				else
				{
					return mcStop;
				}
			}
			else
			{
				m_paused = true;
				return mcPause;
			}
		case MultiMoveActivity::otNothing:
			if(m_paused)
			{
				m_paused = false;
				return mcUnpause;
			}
			else
			{
				return mcNothing;
			}
		case MultiMoveActivity::otStopped:
			if(m_state == sMoving)
			{
				m_state = sStopping;
			}

			if(m_paused)
			{
				m_paused = false;
				return mcUnpause;
			}
			else
			{
				return mcStop;
			}
	}

	assert( !"Can't go here!" );
	return mcNothing;
}

//=====================================================================================//
//              ParMoveActivity::cmd_type ParMoveActivity::onStepMoving()              //
//=====================================================================================//
ParMoveActivity::Command ParMoveActivity::onStepMoving()
{
	PARMOVE_LOG(m_entity->GetInfo()->GetRID() << " incremented m_curHex to " << m_curHex+1 << "\n");
	++m_curHex;

	if(DrvCalcMovepnts(AC_TICK) == AC_STOP || DrvCalcStep(AC_TICK) == AC_STOP)
	{
		m_state = sStopping;
        return cmdStop;
	}

    return cmdContinue;
}

//=====================================================================================//
//             ParMoveActivity::cmd_type ParMoveActivity::onStepStopping()             //
//=====================================================================================//
ParMoveActivity::Command ParMoveActivity::onStepStopping()
{
	PARMOVE_LOG(m_entity->GetInfo()->GetRID() << " incremented m_curHex to " << m_curHex+1 << "\n");
	++m_curHex;

	if(DrvCalcMovepnts(AC_STOP) == AC_STOP || DrvCalcStep(AC_STOP) == AC_STOP)
	{
        return cmdStop;
	}


    return cmdContinue;
}

//=====================================================================================//
//              ParMoveActivity::cmd_type ParMoveActivity::onStepEnded()               //
//=====================================================================================//
ParMoveActivity::Command ParMoveActivity::onStepEnded()
{
	assert( !"ParMoveActivity::onStepEnded. —юда мы не должны заходить!!!" );
	return cmdContinue;
}

}


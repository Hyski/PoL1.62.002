#include "precomp.h"

#include <common/Graphpipe/Camera.h>
#include <undercover/GameLevel/GameLevel.h>
#include "CameraFocusAct.h"
#include <undercover/GameLevel/DynObjectPool.h>

namespace PoL
{

static const float FocusTime = 0.5f;

//=====================================================================================//
//                          CameraFocusAct::CameraFocusAct()                           //
//=====================================================================================//
CameraFocusAct::CameraFocusAct(const std::string &subj)
:	m_started(false),
	m_ended(false),
	m_canFocus(false),
	m_subj(subj)
{
}

//=====================================================================================//
//                            void CameraFocusAct::start()                             //
//=====================================================================================//
void CameraFocusAct::start()
{
	assert( !isStarted() );
	assert( !isEnded() );

	BaseEntity *entity = getEntityByName(m_subj);

	if(entity)
	{
		m_target = entity->GetGraph()->GetPos3();
		m_canFocus = true;
	}
	else if(getDeadEntityPosByName(m_subj,&m_target))
	{
		m_canFocus = true;
	}
	else if(DynObjectSet *dset = ::IWorld::Get()->GetLevel()->GetLevelObjects()->GetObjectSet(m_subj))
	{
		m_target = dset->GetCenter();
		m_canFocus = true;
	}
	else
	{
		m_canFocus = false;
	}

	if(m_canFocus)
	{
		m_startTime = getCurrentTime();
		IWorld::Get()->GetCamera()->FocusOn(m_target,FocusTime);
	}
	else
	{
		m_startTime = getCurrentTime() - FocusTime;
	}

	m_started = true;
}

//=====================================================================================//
//                            void CameraFocusAct::update()                            //
//=====================================================================================//
void CameraFocusAct::update(float tau)
{
	assert( isStarted() );
	assert( !isEnded() );

	if(getCurrentTime() - m_startTime >= FocusTime)
	{
		m_ended = true;
	}
}

}
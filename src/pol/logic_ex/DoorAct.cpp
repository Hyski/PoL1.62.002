#include "precomp.h"
#include "DoorAct.h"
#include <undercover/GameLevel/GameLevel.h>
#include <undercover/GameLevel/DynObjectSet.h>
#include <undercover/GameLevel/DynObjectPool.h>

namespace PoL
{

//=====================================================================================//
//                                 DoorAct::DoorAct()                                  //
//=====================================================================================//
DoorAct::DoorAct(const std::string &doorId, bool open)
:	m_doorId(doorId),
	m_open(open),
	m_started(false),
	m_ended(false),
	m_duration(0.0f)
{
}

//=====================================================================================//
//                                void DoorAct::start()                                //
//=====================================================================================//
void DoorAct::start()
{
	m_started = true;

	if(!m_doorId.empty())
	{
		::DynObjectSet *dset = ::IWorld::Get()->GetLevel()->GetLevelObjects()->GetObjectSet(m_doorId);
		if(dset && dset->IsDoor())
		{
			float curtime = ::Timer::GetSeconds();
			m_duration = dset->ChangeState(m_open?1.0f:0.0f,curtime)-curtime;
		}
	}
}

//=====================================================================================//
//                               void DoorAct::update()                                //
//=====================================================================================//
void DoorAct::update(float tau)
{
	assert( m_started );
	assert( !m_ended );

	m_duration -= tau;
	if(m_duration <= 0.0f)
	{
		m_ended = true;
	}
}



}


#include "precomp.h"
#include "CamFollowAct.h"
#include <Common/Graphpipe/Camera.h>

namespace PoL
{

//=====================================================================================//
//                            CamFollowAct::CamFollowAct()                             //
//=====================================================================================//
CamFollowAct::CamFollowAct(const std::string &who)
:	m_who(who),
	m_entity(0),
	m_started(false)
{
}

//=====================================================================================//
//                            CamFollowAct::~CamFollowAct()                            //
//=====================================================================================//
CamFollowAct::~CamFollowAct()
{
	if(m_entity) m_entity->Detach(this);
}

//=====================================================================================//
//                             void CamFollowAct::start()                              //
//=====================================================================================//
void CamFollowAct::start()
{
	if(m_entity = getEntityByName(m_who))
	{
		m_entity->Attach(this,EV_DESTROY);
	}
	else
	{
		ACT_LOG("WARNING: camfollow(" << m_who << "): target not found\n");
	}
	m_started = true;
}

//=====================================================================================//
//                             void CamFollowAct::update()                             //
//=====================================================================================//
void CamFollowAct::update(float tau)
{
	assert( m_started );

	if(m_entity)
	{
		::IWorld::Get()->GetCamera()->FocusOn(m_entity->GetGraph()->GetPos3(),0.2f);
	}
}

//=====================================================================================//
//                             void CamFollowAct::Update()                             //
//=====================================================================================//
void CamFollowAct::Update(BaseEntity *, event_t, info_t)
{
	if(m_entity) m_entity->Detach(this);
	m_entity = 0;
}

}

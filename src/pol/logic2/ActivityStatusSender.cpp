#include "logicdefs.h"

#include "Entity.h"
#include "ActivityStatusSender.h"

//=====================================================================================//
//                    ActivityStatusSender::ActivityStatusSender()                     //
//=====================================================================================//
ActivityStatusSender::ActivityStatusSender(BaseEntity *actor)
:	m_actor(actor)
{
	if(m_actor.valid())
	{
		m_actor->Attach(this, EV_PREPARE_DEATH);

		activity_info_s info(activity_info_s::AE_BEG);
		m_actor->Notify(EV_ACTIVITY_INFO, &info);
	}
}

//=====================================================================================//
//                    ActivityStatusSender::~ActivityStatusSender()                    //
//=====================================================================================//
ActivityStatusSender::~ActivityStatusSender()
{
	if(m_actor.valid())
	{
		m_actor->Detach(this);

		activity_info_s info(activity_info_s::AE_END);
		m_actor->Notify(EV_ACTIVITY_INFO, &info);
	}
}

//=====================================================================================//
//                         void ActivityStatusSender::Update()                         //
//=====================================================================================//
void ActivityStatusSender::Update(subject_t, event_t, info_t)
{
	m_actor->Detach(this);
	m_actor.reset();
}

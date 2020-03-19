#include "logicdefs.h"

#include "Entity.h"
#include "GameObserver.h"
#include "ShootNotifier.h"

//=====================================================================================//
//                           ShootNotifier::ShootNotifier()                            //
//=====================================================================================//
ShootNotifier::ShootNotifier(const ActivityFactory::shoot_info_s &info)
:	m_info(info)
{
    if(m_info.GetEntity()) m_info.GetEntity()->Attach(this, EV_PREPARE_DEATH);
    if(m_info.GetEntity()) m_info.GetEntity()->Attach(this, EV_DEATH_PLAYED);
}

//=====================================================================================//
//                           ShootNotifier::~ShootNotifier()                           //
//=====================================================================================//
ShootNotifier::~ShootNotifier()
{
    if(m_info.GetEntity()) m_info.GetEntity()->Detach(this);
}

//=====================================================================================//
//                           void ShootNotifier::SendShoot()                           //
//=====================================================================================//
void ShootNotifier::SendShoot(BaseEntity* actor, const point3& pos)
{
    GameObserver::shoot_info info(pos, actor, m_info.GetEntity(), m_info.GetObject());
    GameEvMessenger::GetInst()->Notify(GameObserver::EV_SHOOT, &info);
}

//=====================================================================================//
//                            void ShootNotifier::SendHit()                            //
//=====================================================================================//
void ShootNotifier::SendHit(BaseEntity* actor, const point3& pos, BaseEntity* victim, const rid_t& effect)
{
    SendShoot(actor, pos);

    GameObserver::hit_info info(actor, victim, pos, effect);
    GameEvMessenger::GetInst()->Notify(GameObserver::EV_HIT, &info);
}

//=====================================================================================//
//                            void ShootNotifier::Update()                             //
//=====================================================================================//
void ShootNotifier::Update(subject_t subject, event_t event, info_t info)
{
	if(m_info.GetEntity())
	{
		m_info.GetEntity()->Detach(this);
		m_info.ClearEntity();
	}
}

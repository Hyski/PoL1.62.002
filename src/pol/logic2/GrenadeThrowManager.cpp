#include "logicdefs.h"

#include "Thing.h"
#include "Entity.h"
#include "Graphic.h"
#include "AIUtils.h"
#include "Damager.h"
#include "ShootNotifier.h"
#include "GrenadeThrowManager.h"
#include "Form.h"

//=====================================================================================//
//                     GrenadeThrowManager::GrenadeThrowManager()                      //
//=====================================================================================//
GrenadeThrowManager::GrenadeThrowManager(unsigned entity_damager, ShootNotifier* notifier)
:	m_notifier(notifier),
	m_damager(entity_damager),
    m_grenade(0),
	m_end_time(0),
	m_entity(0),
	m_entityId(0)
{
}

//=====================================================================================//
//                     GrenadeThrowManager::~GrenadeThrowManager()                     //
//=====================================================================================//
GrenadeThrowManager::~GrenadeThrowManager()
{ 
    delete m_grenade;
    delete m_notifier;
}

//=====================================================================================//
//                          void GrenadeThrowManager::Init()                           //
//=====================================================================================//
void GrenadeThrowManager::Init(HumanEntity* human, const point3& to)
{
    m_to = to;
    m_entity = human;
	m_entityId = m_entity->GetEID();

    m_grenade = static_cast<GrenadeThing*>(human->GetEntityContext()->GetThingInHands(TT_GRENADE));
    human->GetEntityContext()->RemoveThing(m_grenade);

    if(m_grenade->GetInfo()->IsPlasmaGrenade()) human->Notify(EntityObserver::EV_USE_PLASMA_GRENADE);
    
    human->GetGraph()->GetBarrel(NULLVEC, &m_from);
            
    point3 dir = m_to - m_from;
    
    // Ѕросить гранату
	const float range = AIUtils::CalcGrenadeRange(human,(human->GetGraph()->GetPos3()-to).Length());
    _GrenadeTracer tracer(human->GetGraph()->GetGID(), m_from, Normalize(dir), dir.Length(), 0, 10.0f, range);
	if(Forms::GetInst()->IsHMVisible())
	{
		m_end_time = Timer::GetSeconds();
	}
	else
	{
		m_end_time = EffectMgr::GetInst()->MakeGrenadeFlight(tracer.GetTrace(), m_grenade->GetInfo()->GetItemModel(), &m_to);
	}
    
    m_material = tracer.GetMaterial();
    m_eid      = tracer.GetEntity();
    m_obj      = tracer.GetObject();
    
    //разослать уведомлени€ о броске гранаты
    if(m_notifier) m_notifier->SendShoot(m_entity, m_from);
}

//=====================================================================================//
//                           bool GrenadeThrowManager::Run()                           //
//=====================================================================================//
bool GrenadeThrowManager::Run()
{
    if(m_grenade && m_end_time < Timer::GetSeconds())
	{
        damage_s damages[3];
        
        damages[0] = m_grenade->GetInfo()->GetBDmg();
        damages[1] = m_grenade->GetInfo()->GetADmg();
        
        //проиграть взрыв
        EffectMgr::GetInst()->MakeHit(m_grenade->GetInfo()->GetHitEffect(), m_from, m_to);

        hit_t hit(m_from, m_to, m_grenade->GetInfo()->GetDmgRadius(), damages);
		hit.m_fall_off = 0.0f;

        switch(m_material)
		{
        case ShotTracer::MT_AIR:
        case ShotTracer::MT_NONE:
            Damager::GetInst()->HandleAirHit(m_entity, hit, m_damager);
            break;

        case ShotTracer::MT_SHIELD:
            Damager::GetInst()->HandleShieldHit(m_entity, hit, m_damager);
            break;

        case ShotTracer::MT_WALL:
            Damager::GetInst()->HandleLevelHit(m_entity, hit, m_damager);
            break;
            
        case ShotTracer::MT_OBJECT:
            Damager::GetInst()->HandleObjectHit(m_entity, m_obj, hit, m_damager);
            break;
            
        case ShotTracer::MT_ENTITY:
            Damager::GetInst()->HandleEntityHit(m_entity, EntityPool::GetInst()->Get(m_eid), hit, m_damager);
            break;
        }        

		// Ќа тот случай, если убийца убилс€ сам
		BaseEntity *killer = EntityPool::GetInst()->Get(m_entityId);

        //разослать уведомлени€ о взрыве
        if(m_material == ShotTracer::MT_ENTITY)
		{
            BaseEntity* victim = EntityPool::GetInst()->Get(m_eid);
            if(m_notifier) m_notifier->SendHit(killer, m_to, victim, m_grenade->GetInfo()->GetHitEffect());
        }
		else
		{
            if(m_notifier) m_notifier->SendHit(killer, m_to, 0, m_grenade->GetInfo()->GetHitEffect());
        }
        
        delete m_grenade;
        m_grenade = 0;
    }

    return m_grenade != 0;
}

#include "logicdefs.h"

#include "Thing.h"
#include "Entity.h"
#include "Graphic.h"
#include "AIUtils.h"
#include "Activity.h"
#include "SndUtils.h"
#include "DirtyLinks.h"
#include "ShootManager.h"
#include "ShootNotifier.h"
#include "TargetManager.h"
#include "TracingLOFCheck.h"
#include "GameObjectsMgr.h"

#include <undercover/Game.h>

//=====================================================================================//
//                                 class EffectPlayer                                  //
//=====================================================================================//
// абстракция проигрывателя эффектов
class EffectPlayer
{
public:
    
    virtual ~EffectPlayer() {}

    //узнать эффекп при попадании
    virtual const rid_t GetHitEffect(ShotTracer::material_type material) const = 0;

    //поиграть эффект выстрела
    virtual void PlayFlash(BaseEntity* entity, const point3& to) = 0;
    //проиграть полет пули
    virtual float PlayTrace(const point3& from, const point3& to) = 0;
    //проиграть попадание
    virtual void PlayHit(ShotTracer::material_type material, const point3& from, const point3& to) = 0;
};

namespace ShootManagerDetails
{
    //
    // обычный проигрыватель эффектов
    //
    class UsualEffectPlayer : public EffectPlayer
	{
    public:

        UsualEffectPlayer(const rid_t& hit, const rid_t& trace) : 
          m_hit(hit), m_trace(trace) {}

        float PlayTrace(const point3& from, const point3& to)
        {
            return EffectMgr::GetInst()->MakeTracer(m_trace, from, to);
        }

        void PlayFlash(BaseEntity* entity, const point3& to)
        {
            if(VehicleEntity* vehicle = entity->Cast2Vehicle())
			{ 
                const VehicleInfo* info = vehicle->GetInfo();

                PlayEntityFlashEffect(vehicle, NULLVEC, to, info->GetShotSound(), info->GetFlashEffect());
                if(info->HaveShellOutlet()) PlayEntityShellsEffect(vehicle, info->GetShellOutlet(), to, info->GetShellsEffect());

            }
			else if(HumanEntity* human = entity->Cast2Human())
			{
                HumanContext* context = human->GetEntityContext();  
                WeaponThing*  weapon  = static_cast<WeaponThing*>(context->GetThingInHands(TT_WEAPON));
                
                PlayEntityFlashEffect(human, weapon->GetInfo()->GetBarrel(), to, weapon->GetInfo()->GetShotSound(context->GetShotType()), weapon->GetInfo()->GetFlashEffect());
                if(weapon->GetInfo()->HaveShellOutlet()) PlayEntityShellsEffect(human, weapon->GetInfo()->GetShellOutlet(), to, weapon->GetAmmo()->GetInfo()->GetShellsEffect());
            }
        }

        void PlayHit(ShotTracer::material_type material, const point3& from, const point3& to)
        {
            EffectMgr::GetInst()->MakeHit(GetHitEffect(material), from, to);
        }

        const rid_t GetHitEffect(ShotTracer::material_type material) const
        {
            switch(material){
            case ShotTracer::MT_AIR:
            case ShotTracer::MT_NONE:
                return "";
                
            case ShotTracer::MT_WALL:
            case ShotTracer::MT_ENTITY:
            case ShotTracer::MT_OBJECT:
                return m_hit;
                
            case ShotTracer::MT_SHIELD:
                return "hit_shield";
            }

            return "";
        }

    private:

        void PlayEntityFlashEffect( BaseEntity* entity, const point3& barrel,
            const point3& to, const rid_t& shot_sound, const rid_t& flash)
        {
            point3 from;

            //узнать расположение ствола
            entity->GetGraph()->GetBarrel(barrel, &from);
            
            //проиграть звук выстрела
            PlaySound(from, shot_sound);
            //проиграть вспышку выстрела
            EffectMgr::GetInst()->MakeFlash(flash, from, to);
        }

        void PlayEntityShellsEffect(BaseEntity* entity, const point3& outlet,
                                    const point3& to, const std::string& shells)
        {
				if(shells.empty() || shells=="-") return;//grom
            point3 from;            
            entity->GetGraph()->GetBarrel(outlet, &from);
            EffectMgr::GetInst()->MakeShellOutlet(entity, shells, from, to);
        }

    protected:

        virtual void PlaySound(const point3& from, const rid_t& snd)
        {
            SndPlayer::GetInst()->Play(from, snd);
        }

    private:

        rid_t  m_hit;
        rid_t  m_trace;
    };

    //
    // проигрывает единичный звук выстрела
    //
    class SingleShotEffectPlayer : public UsualEffectPlayer{
    public:

        SingleShotEffectPlayer(const rid_t& hit, const rid_t& trace) :
            UsualEffectPlayer(hit, trace), m_counter(0) {}

    protected:

        void PlaySound(const point3& from, const rid_t& rid)
        {
            if(m_counter++ == 0) UsualEffectPlayer::PlaySound(from, rid);
        }

    private:

        int m_counter;
    };

    //
    // инициализация в зависимости от типа существа
    //
    class Initializer : public EntityVisitor
	{
    public:
        Initializer() : m_shots(0), m_barrel(NULLVEC) {}

        void Visit(HumanEntity* human)
        {
            WeaponThing* weapon = static_cast<WeaponThing*>(human->GetEntityContext()->GetThingInHands(TT_WEAPON));
            
            shot_type sht_type = human->GetEntityContext()->GetShotType();
            m_shots = weapon->GetShotCount(sht_type);
            
            //запомнить повреждения
            m_hit.m_dmg[hit_t::BW_DMG].m_type = weapon->GetAmmo()->GetInfo()->GetBDmg().m_type;
            m_hit.m_dmg[hit_t::BW_DMG].m_val  = weapon->GetInfo()->GetBDmg().m_val + weapon->GetAmmo()->GetInfo()->GetBDmg().m_val;                
            
            //запомнить weapon additional damage
            m_hit.m_dmg[hit_t::AW_DMG] = weapon->GetInfo()->GetADmg();                
            
            //запомнить ammo aditional damage
            m_hit.m_dmg[hit_t::AA_DMG] = weapon->GetAmmo()->GetInfo()->GetADmg();
            
            //запомнить радиус повреждения
            m_hit.m_radius = weapon->GetAmmo()->GetInfo()->GetDmgRadius();
            
            rid_t hit   = weapon->GetAmmo()->GetInfo()->GetHitEffect(),
                  trace = weapon->GetAmmo()->GetInfo()->GetTraceEffect();

            if(sht_type == SHT_AUTOSHOT)
                m_player = new SingleShotEffectPlayer(hit, trace);
            else
                m_player = new UsualEffectPlayer(hit, trace);

            //узнать коорд стволов оружия
            m_barrel = weapon->GetInfo()->GetBarrel();
        }

        void Visit(VehicleEntity* vehicle)
        {
            m_shots = vehicle->GetInfo()->GetQuality();

            if(m_shots > vehicle->GetEntityContext()->GetAmmoCount())
                m_shots = vehicle->GetEntityContext()->GetAmmoCount();
            
            //запомнить basic damage
            m_hit.m_dmg[hit_t::BW_DMG] = vehicle->GetInfo()->GetBDmg();
            
            //запомнить weapon additional damage
            m_hit.m_dmg[hit_t::AW_DMG] = vehicle->GetInfo()->GetADmg();
            
            //запомнить ammo aditional damage
            m_hit.m_dmg[hit_t::AA_DMG].m_val   = 0;
            m_hit.m_dmg[hit_t::AA_DMG].m_type = DT_NONE;

            //запомнить радиус повреждения
            m_hit.m_radius = vehicle->GetInfo()->GetDmgRadius();
            
            //запомнить эффект попадания
            m_player = new UsualEffectPlayer( vehicle->GetInfo()->GetHitEffect(),
                                              vehicle->GetInfo()->GetTraceEffect());
        }

        int GetShots() const { return m_shots; }

        const hit_t& GetHit() const { return m_hit; }

        const point3& GetBarrel() const { return m_barrel; }

        EffectPlayer* GetEffectPlayer() const { return m_player; }
   
    private:

        hit_t  m_hit;
        int    m_shots; 
        point3 m_barrel;

        EffectPlayer* m_player;
    }; 

    //
    // уменьшить кол-во патронов в оружии существа
    //
    class AmmoDec : public EntityVisitor
	{
    public:

        AmmoDec(int count = 1) : m_count(count) {}

        void Visit(HumanEntity* human)
        {
            WeaponThing* weapon = static_cast<WeaponThing*>(human->GetEntityContext()->GetThingInHands(TT_WEAPON));

            weapon->GetAmmo()->SetBulletCount(weapon->GetAmmo()->GetBulletCount() - m_count);
            if(weapon->GetAmmo()->GetBulletCount() == 0) delete weapon->Load(0);
        }

        void Visit(VehicleEntity* vehicle)
        {
            vehicle->GetEntityContext()->SetAmmoCount(vehicle->GetEntityContext()->GetAmmoCount() - m_count);
        }

    private:

        int m_count;
    };
}

using namespace ShootManagerDetails;

//=====================================================================================//
//                            ShootManager::ShootManager()                             //
//=====================================================================================//
ShootManager::ShootManager(TargetManager* gunner, unsigned entity_dmgr,
						   ShootNotifier* notifier, TracingLOFCheck* lof,
						   bool noAmmoDec)
:	m_gunner(gunner),
	m_lof_check(lof),
	m_notifier(notifier),
    m_entity_damager(entity_dmgr),
	m_noAmmoDec(noAmmoDec),
	m_shot_failed(false),
	m_notified(false)
{
}

//=====================================================================================//
//                            ShootManager::~ShootManager()                            //
//=====================================================================================//
ShootManager::~ShootManager()
{
    delete m_gunner;
    delete m_notifier;
    delete m_lof_check;
    delete m_effect_player;
}

//=====================================================================================//
//                              void ShootManager::Init()                              //
//=====================================================================================//
void ShootManager::Init(Activity* activity, const PoL::BaseEntityRef &entity)
{
    Initializer initer;

    entity->Accept(initer);

    m_hit = initer.GetHit();
    m_barrel = initer.GetBarrel();
    m_shots_count = initer.GetShots();
    m_effect_player = initer.GetEffectPlayer();

	m_fall_off = 0.0f;
	if(entity->Cast2Human())
	{
		HumanContext *context = entity->Cast2Human()->GetEntityContext();
		WeaponThing *weapon = static_cast<WeaponThing*>(context->GetThingInHands(TT_WEAPON));
		m_fall_off = weapon->GetAmmo() ? weapon->GetAmmo()->GetInfo()->GetDamageFalloff() : 0.0f;
	}
	else if(entity->Cast2Vehicle())
	{
		m_fall_off = entity->Cast2Vehicle()->GetInfo()->GetFallOff();
	}
}

//=====================================================================================//
//                             void ShootManager::Shoot()                              //
//=====================================================================================//
bool ShootManager::Shoot(Activity* activity, const PoL::BaseEntityRef &entity)
{
    if(m_shots_count == 0) return false;
    
    entity->GetGraph()->GetBarrel(m_barrel, &m_hit.m_from);

    m_shots_count--;                

    if(!MakeShot(entity.get(), m_hit.m_from, shot_s(m_hit)))
	{
		return false;
	}

	if(!m_noAmmoDec)
	{
		entity->Accept(AmmoDec());
	}

	return true;
}

//=====================================================================================//
//                              void ShootManager::Stop()                              //
//=====================================================================================//
void ShootManager::Stop(Activity* activity, const PoL::BaseEntityRef &entity)
{
    m_shots_count = 0;
}

//=====================================================================================//
//                             bool ShootManager::IsDone()                             //
//=====================================================================================//
bool ShootManager::IsDone(Activity* activity, const PoL::BaseEntityRef &entity)
{
    //если можем проверить линию стрельбы - проверим
	if(m_shot_failed)
	{
		m_shots_count = 0;
		if(!m_notified)
		{
			activity->Notify(ActivityObserver::EV_NO_LOF, 0);
			m_notified = true;
		}
	}

	return m_shots_count == 0;

	//if(m_lof_check)
	//{
	//	point3 from;

	//	entity->GetGraph()->GetBarrel(m_barrel, &from);

	//	//если нет линии стрельбы - не стреляем
	//	if(!m_gunner->TargetReachable())
	//		//if(!m_lof_check->IsExist(entity.get(), from, m_gunner->GetTarget()))
	//	{
	//		m_shots_count = 0;
	//		activity->Notify(ActivityObserver::EV_NO_LOF, 0);
	//	}

	//	//проверка происходит однажды
	//	delete m_lof_check;
	//	m_lof_check = 0;
	//}

	//return m_shots_count == 0;
}

//=====================================================================================//
//                              bool ShootManager::Run()                               //
//=====================================================================================//
bool ShootManager::Run(Activity* activity, const PoL::BaseEntityRef &entity_)
{
	if(m_shot_failed || (m_shots.empty() && m_shots_count == 0)) return false;

	BaseEntity *entity = entity_.get();
    shot_lst_t::iterator itor = m_shots.begin();

    while(itor != m_shots.end())
	{
        if(itor->m_end_time < Timer::GetSeconds())
		{
            //выстрел уже закончился
			if( !DirtyLinks::IsGoodEntity(entity) ) //Grom
			{
				entity = 0;
			}

            if(IsTargetDestroyed(*itor))
			{
				if(entity)
				{
					MakeShot(entity, itor->m_from, *itor, itor->m_to - itor->m_from);
				}
			}
            else                    
            {
				if(entity)
				{
					MakeHit(entity, *itor);
				}
			}

			if( !DirtyLinks::IsGoodEntity(entity) ) //Grom
			{
				entity = 0;
			}

			itor = m_shots.erase(itor);
        }
		else
		{
			++itor;
		}
    }
    
    return !m_shots.empty();
}

//=====================================================================================//
//                            void ShootManager::MakeShot()                            //
//=====================================================================================//
bool ShootManager::MakeShot(BaseEntity* entity, point3& from, shot_s& shot, point3 dir)
{
	if(m_shot_failed) return false;

    //посчитаем трассировки
    shot.m_count ++;

    TargetManager::trace_info trace_info;

    //отmрассируем выстрел
    if(shot.m_count == 1)
	{
		m_shot_failed = !m_gunner->MakeFirstTrace(entity, from, &trace_info);
		if(m_shot_failed) return false;
	}
    else
	{
		m_gunner->MakeNextTrace(entity, from, dir, &trace_info);
	}
    
    //проиграем эффект полета пули
    shot.m_from     = from;  
    shot.m_to       = trace_info.m_end; 
    shot.m_entity   = trace_info.m_entity;
    shot.m_object   = trace_info.m_object;
    shot.m_material = trace_info.m_material;        
    
    //разослать сообщение о выстреле
    if(m_notifier) m_notifier->SendShoot(entity, shot.m_from);
    
    shot.m_end_time = 0.0f;

	if(Game::RenderEnabled())
	{
		//проиграем эффекты полета пули
		shot.m_end_time = m_effect_player->PlayTrace(shot.m_from, shot.m_to);
	    
		//проиграем звук выстрела и эффект вылета пули
		if(shot.m_count == 1) m_effect_player->PlayFlash(entity, shot.m_to);
	}
	else
	{
		shot.m_end_time = Timer::GetSeconds();
	}

	//поставим выстрел в список
	shot.m_fall_off = m_fall_off;
	m_shots.push_back(shot);

	return true;
}

//=====================================================================================//
//                            void ShootManager::MakeHit()                             //
//=====================================================================================//
void ShootManager::MakeHit(BaseEntity* entity, const shot_s& shot)
{
	if(Game::RenderEnabled())
	{
		//проиграем эффект попадания
		m_effect_player->PlayHit(shot.m_material, shot.m_from, shot.m_to);
	}

    //запустим механизм объемного взрыва
    switch(shot.m_material){
    case ShotTracer::MT_WALL:
        Damager::GetInst()->HandleLevelHit(entity, shot, m_entity_damager);
        break;

    case ShotTracer::MT_SHIELD:
        Damager::GetInst()->HandleShieldHit(entity,  shot, m_entity_damager);
        break;
    
    case ShotTracer::MT_OBJECT:
        Damager::GetInst()->HandleObjectHit(entity, shot.m_object, shot, m_entity_damager);
        break;

    case ShotTracer::MT_ENTITY:
        {
            BaseEntity* victim = EntityPool::GetInst()->Get(shot.m_entity);
            Damager::GetInst()->HandleEntityHit(entity, victim, shot, m_entity_damager);
        }
        break;

    case ShotTracer::MT_AIR:
    case ShotTracer::MT_NONE:
        Damager::GetInst()->HandleAirHit(entity,  shot, m_entity_damager);
        break;
    }

	if(!DirtyLinks::IsGoodEntity(entity))
		entity = NULL;//Grom

    //разослать уведомления о взрыве
    if(shot.m_material == ShotTracer::MT_ENTITY)
	{
        BaseEntity* victim = EntityPool::GetInst()->Get(shot.m_entity);
        if(m_notifier) m_notifier->SendHit(entity, shot.m_to, victim, m_effect_player->GetHitEffect(shot.m_material));
    }
	else
	{
        if(m_notifier) m_notifier->SendHit(entity, shot.m_to, 0, m_effect_player->GetHitEffect(shot.m_material));
    }
}

//=====================================================================================//
//                    bool ShootManager::IsTargetDestroyed() const                     //
//=====================================================================================//
bool ShootManager::IsTargetDestroyed(const shot_s& shot) const
{
    return      (shot.m_material == ShotTracer::MT_ENTITY  && !EntityPool::GetInst()->Get(shot.m_entity))
            ||   (shot.m_material == ShotTracer::MT_OBJECT  && GameObjectsMgr::GetInst()->IsDestroyed(shot.m_object));
}

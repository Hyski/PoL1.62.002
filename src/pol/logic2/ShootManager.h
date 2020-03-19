#pragma once

#include "Damager.h"
#include "EntityRef.h"
#include "TraceUtils.h"

class TracingLOFCheck;
class TargetManager;
class ShootNotifier;
class EffectPlayer;
class Activity;

//=====================================================================================//
//                                 class ShootManager                                  //
//=====================================================================================//
// реалиазация механизма стрельбы
class ShootManager
{
    hit_t  m_hit;
    point3 m_barrel;
    int    m_shots_count; //кол-во выстрелов
	float m_fall_off;
	bool m_shot_failed;
	bool m_notified;
 
    TracingLOFCheck *m_lof_check;
    TargetManager *m_gunner;
    ShootNotifier *m_notifier;

    unsigned m_entity_damager;

    //информация о выстреле
    struct shot_s : public hit_t
	{
        int   m_count;    //количество трассировок выстрела

        eid_t m_entity;   //существо на кот закончилась трассировка    
        rid_t m_object;   //объект на кот закончилась трассировка
        float m_end_time; //время когда закончиться проигрывание эффекта стрельбы        

        ShotTracer::material_type m_material; //материал на кот окнчился эффект

        shot_s(const hit_t& hit) : m_count(0) { static_cast<hit_t&>(*this) = hit; }
    };

    typedef std::list<shot_s> shot_lst_t;
    shot_lst_t m_shots;

    EffectPlayer* m_effect_player;
	bool m_noAmmoDec;

public:
    ShootManager(TargetManager* gunner, unsigned entity_dmgr, ShootNotifier* notifier = 0, TracingLOFCheck* lof = 0, bool noAmmoDec = false);
    ~ShootManager();

	void Init(Activity* activity, const PoL::BaseEntityRef &entity);
    bool Shoot(Activity* activity, const PoL::BaseEntityRef &entity);
    void Stop(Activity* activity, const PoL::BaseEntityRef &entity);

    bool IsDone(Activity* activity, const PoL::BaseEntityRef &entity);
    bool Run(Activity* activity, const PoL::BaseEntityRef &entity);

private:
    bool MakeShot(BaseEntity* entity, point3& from, shot_s& shot, point3 dir = NULLVEC);
    void MakeHit(BaseEntity* entity, const shot_s& shot);
    bool IsTargetDestroyed(const shot_s& shot) const;
};

#pragma once

#include "TraceUtils.h"
#include "ThrowManager.h"

class BaseEntity;
class GrenadeThing;
class ShootNotifier;

//=====================================================================================//
//                   class GrenadeThrowManager : public ThrowManager                   //
//=====================================================================================//
// менеджер для броска гранаты
class GrenadeThrowManager : public ThrowManager
{
    point3 m_to;
    point3 m_from;

    rid_t  m_obj;
    eid_t  m_eid;
	eid_t  m_entityId;

    BaseEntity* m_entity;

    float    m_end_time;
    unsigned m_damager;

    GrenadeThing*  m_grenade;
    ShootNotifier* m_notifier;

    ShotTracer::material_type m_material;

public:
    GrenadeThrowManager(unsigned entity_damager, ShootNotifier* notifier = 0);
    ~GrenadeThrowManager();

    void Init(HumanEntity* human, const point3& to);
    bool Run();
};

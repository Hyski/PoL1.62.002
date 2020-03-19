#pragma once

#include "Activity.h"
#include "EntityAux.h"
#include "EntityRef.h"
#include "TraceUtils.h"

class TracingLOFCheck;

//=====================================================================================//
//                                 class TargetManager                                 //
//=====================================================================================//
class TargetManager
{
    const float m_near_dist;

	ActivityFactory::shoot_info_s m_info;
	PoL::BaseEntityRef m_victim;
	point3 m_target;
	TracingLOFCheck *m_lof;

    float m_accuracy;

public:
	TargetManager(float accuracy, const ActivityFactory::shoot_info_s &info, TracingLOFCheck *m_lof);
    ~TargetManager();

    //информация о трассировке
    struct trace_info
	{
        point3 m_end;

        eid_t  m_entity;
        rid_t  m_object;
        
        ShotTracer::material_type m_material;
    };
    
    point3 GetTarget();
    bool MakeFirstTrace(BaseEntity* entity, const point3& from, trace_info* info);
    void MakeNextTrace(BaseEntity* entity, const point3& from, const point3& dir, trace_info* info);

private:
    //void Update(subject_t subj, event_t event, info_t info);
    bool IsVictimExists(BaseEntity* actor) const;
    bool IsObjectExists() const;

	bool FindTarget(BaseEntity *actor, const point3 &origin);
    point3 MakeScatter(BaseEntity *actor, const point3 &origin, const point3 &dir);
};

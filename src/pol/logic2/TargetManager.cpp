#include "logicdefs.h"

#include "Thing.h"
#include "Entity.h"
#include "Graphic.h"
#include "AIUtils.h"
#include "DirtyLinks.h"
#include "TargetManager.h"
#include "EnemyDetection.h"
#include "TracingLOFCheck.h"
#include "GameObjectsMgr.h"

//=====================================================================================//
//                           TargetManager::TargetManager()                            //
//=====================================================================================//
TargetManager::TargetManager(float accuracy, const ActivityFactory::shoot_info_s &info,
							 TracingLOFCheck *lof)
:	m_info(info),
	m_accuracy(accuracy),
	m_near_dist(2.0f),
	m_victim(m_info.GetEntity()),
	m_lof(lof)
{
}

//=====================================================================================//
//                           TargetManager::~TargetManager()                           //
//=====================================================================================//
TargetManager::~TargetManager()
{
}

//=====================================================================================//
//                          point3 TargetManager::GetTarget()                          //
//=====================================================================================//
point3 TargetManager::GetTarget()
{
	return m_target;
}

//=====================================================================================//
//                        void TargetManager::MakeFirstTrace()                         //
//=====================================================================================//
bool TargetManager::MakeFirstTrace(BaseEntity* entity, const point3& from, trace_info* info)
{
	if(!FindTarget(entity,from))
	{
		if(m_lof && entity->GetPlayer() == PT_PLAYER) m_lof->PrintMessage(entity);
		return false;
	}

	point3 ent_pos = entity->GetGraph()->GetPos3();
	float  ent_ang = entity->GetGraph()->GetAngle() - PId2;

	//вектор направления сущ-ва
	point3 entity_dir(cosf(ent_ang), sinf(ent_ang), 0);

	//вектор направления оружия сущ-ва 
	point3 from_entpos_2_barrel(from - ent_pos);
	from_entpos_2_barrel.z = 0;
	from_entpos_2_barrel = Normalize(from_entpos_2_barrel);

	//направление трассировки
	point3 trace_dir = MakeScatter(entity, from, GetTarget() - from);

	//вектора направелния от оружия до цели
	point3 from_barrel_2_target = trace_dir;
	from_barrel_2_target.z = 0;
	from_barrel_2_target = Normalize(from_barrel_2_target);

	//если можем убить себя то корректируем направление трассировки
	if(     entity_dir.Dot(from_entpos_2_barrel) >= 0
		&&  from_barrel_2_target.Dot(from_entpos_2_barrel) < 0)
		trace_dir = entity_dir;

	//трассируем выстрел
	MakeNextTrace(entity, from, trace_dir, info);

	return true;
}

//=====================================================================================//
//                         point3 TargetManager::FindTarget()                          //
//=====================================================================================//
bool TargetManager::FindTarget(BaseEntity *actor, const point3 &origin)
{
	if(!m_lof)
	{
		m_target = m_info.GetShotAim();
		return true;
	}

	if(m_info.IsEntity())
	{
		if(BaseEntity *entity = m_info.GetEntity())
		{
			GraphEntity *gent = entity->GetGraph();

			if(gent->GetShotPointCount() == 0)
			{
				if(m_lof->IsExist(actor,origin,gent->GetShotPoint()))
				{
					m_target = gent->GetShotPoint();
					return true;
				}
				else
				{
					return false;
				}
			}

			for(int i = 0; i != gent->GetShotPointCount(); ++i)
			{
				const point3 target = gent->GetShotPointNumber(i);
				if(m_lof->IsExist(actor,origin,target))
				{
					m_target = target;
					return true;
				}
			}
		}
	}
	else if(m_info.IsObject())
	{
		const point3 target = DirtyLinks::GetObjCenter(m_info.GetObject());
		if(m_lof->IsExist(actor,origin,target))
		{
			m_target = target;
			return true;
		}
	}
	else if(m_info.IsPoint())
	{
		if(m_lof->IsExist(actor,origin,m_info.GetPoint()))
		{
			m_target = m_info.GetPoint();
			return true;
		}
	}

	return false;
}

//=====================================================================================//
//                         void TargetManager::MakeNextTrace()                         //
//=====================================================================================//
void TargetManager::MakeNextTrace(BaseEntity* entity, const point3& from,
									 const point3& dir, trace_info* info)
{
	//трассируем выстрел
	ShotTracer trace(entity ? entity->GetEID() : 0, from, dir, 0, ShotTracer::F_SKIP_GHOST);

	//вернем результаты трассировки        
	info->m_end      = trace.GetEnd(); 
	info->m_entity   = trace.GetEntity();
	info->m_object   = trace.GetObject();
	info->m_material = trace.GetMaterial();        
}

//=====================================================================================//
//                            void TargetManager::Update()                             //
//=====================================================================================//
//void TargetManager::Update(subject_t subj, event_t event, info_t info)
//{
//	m_victim->Detach(this);
//	m_victim = 0;
//}

//=====================================================================================//
//                     bool TargetManager::IsVictimExists() const                      //
//=====================================================================================//
bool TargetManager::IsVictimExists(BaseEntity* actor) const
{
	return m_victim.valid() && m_victim->GetEID() != actor->GetEID();
}

//=====================================================================================//
//                     bool TargetManager::IsObjectExists() const                      //
//=====================================================================================//
bool TargetManager::IsObjectExists() const
{
	return m_info.IsObject() && !GameObjectsMgr::GetInst()->IsDestroyed(m_info.GetObject());
}

//=====================================================================================//
//                           static point3 RandUnitVector()                            //
//=====================================================================================//
static point3 RandUnitVector(const point3 &dir)
{
	Quaternion q;
	q.FromAngleAxis(aiNormRand() * PIm2, Normalize(dir));
	return q * Normalize(dir.Cross(point3(1.0f,0.0f,0.0f)).Cross(dir));
}

//=====================================================================================//
//                         point3 TargetManager::MakeScatter()                         //
//=====================================================================================//
//сделать разбос по направлению
point3 TargetManager::MakeScatter(BaseEntity *entity, const point3 &origin, const point3 &dir)
{
	float scatter = 10.0f;

	if(HumanEntity *he = entity->Cast2Human())
	{
		if(WeaponThing *wt = static_cast<WeaponThing*>(he->GetEntityContext()->GetThingInHands(TT_WEAPON)))
		{
			scatter = wt->GetInfo()->GetMaxScatter();
		}
	}

	const bool needhit = m_accuracy == 1.0f || m_accuracy >= aiNormRand();
	if(!IsVictimExists(entity) && needhit)
	{
		return dir;
	}

	const float max_angle = scatter / 180.0f * PI; // максимальный угол разброса 5 градусов
	const float dist_on_meter = tanf(max_angle);
	const int max_iterations = 8;
	int indices[max_iterations];
	const float step = 1.0f / static_cast<float>(max_iterations);

	for(int i = 0; i < max_iterations; ++i)
	{
		indices[i] = i;
	}

	std::random_shuffle(indices,indices+max_iterations);

	const float factor = 0.25f;
	const float max_z  = 1.5f;

	const point3 devia = RandUnitVector(dir) * dist_on_meter * dir.Length();
	assert( devia.Dot(dir) < 1e-4f );
	point3 targdevia = aiNormRand() * devia;

	if(IsVictimExists(entity))
	{
		bool found = false;

		for(int idx = 0; idx < max_iterations && !found; ++idx)
		{
			const int i = indices[idx];
			const float factor = i*step;
			const point3 curdir = Normalize(dir + factor*devia);

			point3 pos;
			if(eid_t hit = EntityPool::GetInst()->TraceEntities(origin,curdir,entity->GetEID(),&pos))
			{
				if(needhit)
				{
					if(hit == m_victim->GetEID())
					{
						targdevia = factor*devia;
						found = true;
					}
				}
				else
				{
					if(hit != m_victim->GetEID())
					{
						if(entity->GetPlayer() != PT_PLAYER
							&& EnemyDetector::getInst()->isHeEnemy4Me(entity,EntityPool::GetInst()->Get(hit)))
						{
							targdevia = factor*devia;
							found = true;
						}
					}
				}
			}
			else
			{
				if(!needhit)
				{
					targdevia = factor*devia;
					found = true;
				}
			}
		}

		if(needhit && !found)
		{
			targdevia = point3(0.0f,0.0f,0.0f);
		}
	}

	return dir + targdevia;
}

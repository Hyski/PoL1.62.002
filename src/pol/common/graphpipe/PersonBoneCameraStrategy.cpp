#include "precomp.h"
#include "PersonBoneCameraStrategy.h"
#include <logic2/logictypes.h>
#include <logic2/graphic.h>
#include <logic2/entity.h>
#include <undercover/skin/person.h>
#include "Camera.h"

RealEntity *PersonBoneCameraStrategy::m_realEntity = 0;

//=====================================================================================//
//                         class PersonBoneCameraStrategy::Obs                         //
//=====================================================================================//
class PersonBoneCameraStrategy::Obs : private EntityObserver
{
public:
	eid_t m_eid;
	std::string m_boneId;
	BaseEntity *m_entity;

	Obs(eid_t e) : m_eid(e), m_entity(0)
	{
		if(m_eid)
		{
			EntityPool *pool = EntityPool::GetInst();
			m_entity = pool->Get(m_eid);
			ActiveContext *ac = static_cast<ActiveContext*>(m_entity->GetEntityContext());
			RealEntity *e = static_cast<RealEntity*>(IWorld::Get()->GetCharacterPool()->Get(m_entity->GetGraph()->GetGID()));
			m_boneId = m_entity->GetInfo()->GetViewBone();

			if(!m_entity->Cast2Human() && !m_entity->Cast2Vehicle() || !e->GetBone(m_boneId))
			{
				m_entity = 0;
				m_eid = 0;
				return;
			}

			m_realEntity = e;
			m_entity->Attach(this,EV_DESTROY);
		}
	}

	~Obs()
	{
		if(m_entity)
		{
			m_entity->Detach(this);
			m_realEntity = 0;
		}
	}

	void Update(BaseEntity* entity, event_t event, info_t info)
	{
		assert( entity == m_entity );
		assert( event == EV_DESTROY );
		m_entity->Detach(this);
		m_eid = 0;
		m_entity = 0;
		m_realEntity = 0;
	}
};

namespace PersonBoneCameraStrategyDetail
{

}

using namespace PersonBoneCameraStrategyDetail;

//=====================================================================================//
//                PersonBoneCameraStrategy::PersonBoneCameraStrategy()                 //
//=====================================================================================//
PersonBoneCameraStrategy::PersonBoneCameraStrategy(Camera *cam)
:	m_cam(cam),
	m_angleX(0.0f),
	m_angleZ(0.0f)
{
}

//=====================================================================================//
//                PersonBoneCameraStrategy::~PersonBoneCameraStrategy()                //
//=====================================================================================//
PersonBoneCameraStrategy::~PersonBoneCameraStrategy()
{
}

//=====================================================================================//
//                       void PersonBoneCameraStrategy::Update()                       //
//=====================================================================================//
void PersonBoneCameraStrategy::Update(float tau)
{
	if(!m_eobs.get() || !m_eobs->m_eid
		|| (m_eobs->m_entity->Cast2Human() && m_eobs->m_entity->Cast2Human()->GetEntityContext()->GetCrew())
		)
	{
		if(m_eobs.get() && m_eobs->m_entity && m_eobs->m_entity->Cast2Human() && m_eobs->m_entity->Cast2Human()->GetEntityContext()->GetCrew())
		{
			m_cam->FocusOn(m_eobs->m_entity->Cast2Human()->GetEntityContext()->GetCrew()->GetGraph()->GetPos3());
		}
		m_cam->RemovePersonBoneCam();
		m_eobs.reset();
		return;
	}

	using namespace mll::algebra;
	using mll::algebra::matrix3;
	using mll::algebra::point3;
	using mll::algebra::quaternion;

	RealEntity *e = static_cast<RealEntity*>(IWorld::Get()->GetCharacterPool()->Get(m_eobs->m_entity->GetGraph()->GetGID()));
	const SkState::BoneState *bone = e->GetBoneEx(m_eobs->m_boneId);
	
	matrix3 correct( 1.0f, 0.0f, 0.0f, 0.0f,
						0.0f, 0.0f, 1.0f, 0.0f,
						0.0f, 1.0f, 0.0f, 0.0f,
						0.0f, 0.0f, 0.0f, 1.0f);

	const matrix3 rot = quaternion(vector3(1.0f,0.0f,0.0f),m_angleX).as_matrix()
		* quaternion(vector3(0.0f,0.0f,-1.0f),m_angleZ).as_matrix();
		
	matrix3 world = make_matrix(*e->GetWorld());
	point3 bonePos(bone->Trans.x,bone->Trans.y,bone->Trans.z);
	//quaternion orient(bone->Orient.x,bone->Orient.y,bone->Orient.z,bone->Orient.w);

	matrix3 view = (/*orient.as_matrix() * */rot * translate(bonePos) * world).invert() * correct;
	matrix3 invview = matrix3(view).invert();
	m_params.m_dir   = -vector3(0.0f,0.0f,1.0f)*invview;
	m_params.m_up    =  vector3(0.0f,1.0f,0.0f)*invview;
	m_params.m_right = -vector3(1.0f,0.0f,0.0f)*invview;
	m_params.m_pos   =   point3(0.0f,0.0f,0.0f)*invview;

	ConstructView();
}

//=====================================================================================//
//                      void PersonBoneCameraStrategy::RotateX()                       //
//=====================================================================================//
void PersonBoneCameraStrategy::RotateX(float phi)
{
	m_angleX += phi;
	if(m_angleX < -(PId2)*0.9f) m_angleX = -(PId2)*0.9f;
	if(m_angleX >  (PId2)*0.9f) m_angleX =  (PId2)*0.9f;
}

//=====================================================================================//
//                      void PersonBoneCameraStrategy::RotateZ()                       //
//=====================================================================================//
void PersonBoneCameraStrategy::RotateZ(float phi)
{
	m_angleZ += phi;
	m_angleZ = fmodf(m_angleZ,PIm2);
}

//=====================================================================================//
//                       void PersonBoneCameraStrategy::Reset()                        //
//=====================================================================================//
void PersonBoneCameraStrategy::Reset(eid_t e)
{
	m_eobs.reset();
	m_eobs.reset(new Obs(e));
	if(!m_eobs->m_eid)
	{
		m_eobs.reset();
		m_cam->RemovePersonBoneCam();
	}
	else
	{
		m_angleX = 0.0f;
		m_angleZ = 0.0f;
		RealEntity *e = static_cast<RealEntity*>(IWorld::Get()->GetCharacterPool()->Get(m_eobs->m_entity->GetGraph()->GetGID()));

		const SkState::BoneState *bone = e->GetBoneEx(m_eobs->m_boneId);
		using namespace mll::algebra;
		using mll::algebra::matrix3;
		using mll::algebra::point3;
		using mll::algebra::quaternion;
		
		matrix3 correct( 1.0f, 0.0f, 0.0f, 0.0f,
						 0.0f, 0.0f, 1.0f, 0.0f,
						 0.0f, 1.0f, 0.0f, 0.0f,
						 0.0f, 0.0f, 0.0f, 1.0f );

		matrix3 world = make_matrix(*e->GetWorld());
		point3 bonePos(bone->Trans.x,bone->Trans.y,bone->Trans.z);

		quaternion orient(bone->Orient.x,bone->Orient.y,bone->Orient.z,bone->Orient.w);
		matrix3 view = (/*orient.as_matrix() * */translate(bonePos) * world).invert() * correct;
		matrix3 inv_view = matrix3(view).invert();
		vector3 newdir   =  vector3(0.0f,0.0f,1.0f)*inv_view;
		vector3 newup    =  vector3(0.0f,1.0f,0.0f)*inv_view;
		vector3 newright = -vector3(1.0f,0.0f,0.0f)*inv_view;
		point3 newpos    =   point3(0.0f,0.0f,0.0f)*inv_view;

		KeyAnimation ka;
		Quaternion q;

		float r[3][3];
		r[0][0] = newright.x; r[1][0] = newright.y; r[2][0] = newright.z;
		r[0][1] = newup.x;    r[1][1] = newup.y;    r[2][1] = newup.z;
		r[0][2] = newdir.x;   r[1][2] = newdir.y;    r[2][2] = newdir.z;
		q.FromRotationMatrix(r);
		q.w = -q.w;
		ka.SetKey(1.0f, q, ::point3(newpos));

		m_cam->LinkToSpline(ka,Timer::GetSeconds());
	}
}
// camera.cpp: implementation of the Camera class.
//
//////////////////////////////////////////////////////////////////////
#include "precomp.h"
#include "camera.h"
#include <undercover/gamelevel/grid.h>
#include <undercover/gamelevel/gamelevel.h>
#include <undercover/gamelevel/dynobjectpool.h>
#include <logic2/hexutils.h>

#include "FreeCameraStrategy.h"
#include "SplineCameraStrategy.h"
#include "PersonBoneCameraStrategy.h"

/**************************************************************                 

Virtuality                                         

(c) by MiST Land 2000                                    
---------------------------------------------------                     
Description:Класс камеры. Отвечает за перемещение камеры,
создание матриц видовой и проекционной.
Также предоставляет дополнительные методы
Например, для picking: (x,y) -> луч.

Author: Grom 
Creation: 12 апреля 2000
***************************************************************/  
#include "../precomp.h"   


#if 0&&defined(_HOME_VERSION)
CLog camera_log;
#define camlog	camera_log["camera.log"]
#else
#define camlog	/##/
#endif

//=====================================================================================//
//                                  Camera::Camera()                                   //
//=====================================================================================//
Camera::Camera()
:	m_viewMod(true),
	m_projMod(true),
	m_lastUpdateTime(0.0f)
{
	FreeCameraStrategy *freecam = 0;

	m_strategies[cmtFree].reset(freecam = new FreeCameraStrategy);
	m_strategies[cmtSpline].reset(new SplineCameraStrategy(this,freecam));
	m_strategies[cmtPersonBone].reset(new PersonBoneCameraStrategy(this));
	m_camStack.push_back(cmtFree);
}

//void Camera::SetProjection(float _Fov, float _Near, float _Far, float _Aspect)
//{
//	Aspect=_Aspect;
//	Fov=_Fov;
//	Far=_Far;
//	Near=_Near;
//	D3DUtil_SetProjectionMatrix( Proj,Fov,Aspect,Near,Far);
//	ProjMod=true;
//}

//=====================================================================================//
//                                void Camera::Apply()                                 //
//=====================================================================================//
void Camera::Apply(IDirect3DDevice7 *D3DDev)
{
	D3DDev->SetTransform(D3DTRANSFORMSTATE_VIEW, &mll::algebra::D3DMATRIX(GetView()));
	D3DDev->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &mll::algebra::D3DMATRIX(GetProj()));
	m_projMod = m_viewMod = false;
}

//=====================================================================================//
//                             point3 Camera::Pick() const                             //
//=====================================================================================//
point3 Camera::Pick(float x,float y) const
{
	using mll::algebra::vector3;
	using mll::algebra::matrix3;

	// Compute the vector of the pick ray in screen space
	vector3 result(0.0f,0.0f,1.0f);
	float localx = x/D3DKernel::ResX();
	float localy = y/D3DKernel::ResY();
	float xx =  (2.0f*localx-1.0f) / m_params.m_proj._00;
	float yy = -(2.0f*localy-1.0f) / m_params.m_proj._11;
	//v.z =  1.0f;

	result.x  = xx*m_params.m_view._00 + yy*m_params.m_view._01 + m_params.m_view._02;
	result.y  = xx*m_params.m_view._10 + yy*m_params.m_view._11 + m_params.m_view._12;
	result.z  = xx*m_params.m_view._20 + yy*m_params.m_view._21 + m_params.m_view._22;

	return ::point3(result.normalize());
}

//=====================================================================================//
//                                void Camera::Rotate()                                //
//=====================================================================================//
void Camera::Rotate(float phi, unsigned int axis)
{
	if(GetMoveType() == cmtFree)
	{
		FreeCameraStrategy *cam = static_cast<FreeCameraStrategy *>(m_strategies[cmtFree].get());
		switch(axis)
		{
			case Z|SELF: cam->RotateZAroundSelf(phi); break;
			case Z|AIMSPOT: cam->RotateZ(phi); break;
			case RIGHT|SELF: cam->RotateXAroundSelf(phi); break;
			case RIGHT|AIMSPOT: cam->RotateX(phi); break;
		}
	}
	else if(GetMoveType() == cmtPersonBone)
	{
		PersonBoneCameraStrategy *cam = static_cast<PersonBoneCameraStrategy *>(m_strategies[cmtPersonBone].get());
		switch(axis)
		{
			case Z|SELF: cam->RotateZ(phi); break;
			case RIGHT|SELF: cam->RotateX(phi); break;
		}
	}
}       

//=====================================================================================//
//                             void Camera::SetLocation()                              //
//=====================================================================================//
void Camera::SetLocation(const point3 &p, const point3 &dir)
{
	using mll::algebra::point3;
	using mll::algebra::vector3;

	FreeCameraStrategy *cam = static_cast<FreeCameraStrategy *>(m_strategies[cmtFree].get());
	cam->SetLocation(point3(p.x,p.y,p.z),vector3(dir.x,dir.y,dir.z));

	if(m_camStack.size() > 1)
	{
		m_camStack.erase(m_camStack.begin()+1,m_camStack.end());
	}
}

//=====================================================================================//
//                            void Camera::UpdateFrustum()                             //
//=====================================================================================//
void Camera::UpdateFrustum()
{
	typedef mll::algebra::quaternion quat;

	//far,right,left,bottom,up,near;
	Cone.Planes[0] = Plane(point3(-m_params.m_dir), point3(m_params.m_pos + m_params.m_dir*m_params.m_far));//Far
	Cone.Planes[5] = Plane(point3( m_params.m_dir), point3(m_params.m_pos + m_params.m_dir*m_params.m_near));//Near

	float FovW = atan2f(1.f, m_params.m_proj._00)*2.0f;
	float FovH = atan2f(1.f, m_params.m_proj._11)*2.0f;

	quat q;

	q = quat(m_params.m_up,PId2-FovW*0.5f);
	Cone.Planes[1] = Plane(m_params.m_dir*q.as_matrix(), m_params.m_pos); //Left

	q = quat(m_params.m_up,-PId2+FovW*0.5f);
	Cone.Planes[2] = Plane(m_params.m_dir*q.as_matrix(), m_params.m_pos); //Right

	q = quat(m_params.m_right,PId2-FovH*0.5f);
	Cone.Planes[3] = Plane(m_params.m_dir*q.as_matrix(), m_params.m_pos); //up

	q = quat(m_params.m_right,-PId2+FovH*0.5f);
	Cone.Planes[4] = Plane(m_params.m_dir*q.as_matrix(), m_params.m_pos); //bottom

	const float tanfov = tanf(m_params.m_fov*0.5f);
	Cone.Points[0] = m_params.m_pos; //вершина пирамиды
	Cone.Points[1] = m_params.m_pos + m_params.m_far*(m_params.m_dir - tanfov*(m_params.m_right + m_params.m_up));
	Cone.Points[2] = m_params.m_pos + m_params.m_far*(m_params.m_dir + tanfov*(m_params.m_right + m_params.m_up));
	Cone.Points[3] = m_params.m_pos + m_params.m_far*(m_params.m_dir - tanfov*(m_params.m_right - m_params.m_up));
	Cone.Points[4] = m_params.m_pos + m_params.m_far*(m_params.m_dir + tanfov*(m_params.m_right - m_params.m_up));
}

//=====================================================================================//
//                                 void Camera::Move()                                 //
//=====================================================================================//
void Camera::Move(const point3 &delta)
{
	if(m_camStack.back() == cmtFree)
	{
		FreeCameraStrategy *cam = static_cast<FreeCameraStrategy*>(m_strategies[cmtFree].get());
		cam->Move(mll::algebra::vector3(delta.x,delta.y,delta.z));
	}
}

//=====================================================================================//
//                             void Camera::LinkToSpline()                             //
//=====================================================================================//
void Camera::LinkToSpline(const KeyAnimation &spl, float tau)
{
	const float focustime = 0.1f;
	m_camStack.erase(std::remove(m_camStack.begin(),m_camStack.end(),cmtSpline),m_camStack.end());
	m_camStack.push_back(cmtSpline);

	SplineCameraStrategy *cam = static_cast<SplineCameraStrategy*>(m_strategies[cmtSpline].get());
	cam->Reset(spl,tau);
}

//=====================================================================================//
//                           void Camera::RemoveSplineCam()                            //
//=====================================================================================//
void Camera::RemoveSplineCam()
{
	m_camStack.erase(std::remove(m_camStack.begin(),m_camStack.end(),cmtSpline),m_camStack.end());
}

//=====================================================================================//
//                         void Camera::RemovePersonBoneCam()                          //
//=====================================================================================//
void Camera::RemovePersonBoneCam()
{
	m_camStack.erase(std::remove(m_camStack.begin(),m_camStack.end(),cmtPersonBone),m_camStack.end());
}

//=====================================================================================//
//                                void Camera::Update()                                //
//=====================================================================================//
void Camera::Update(float tau)
{
	using mll::algebra::matrix3;
	const float delta = tau - m_lastUpdateTime;
	m_lastUpdateTime = tau;

	matrix3 oldview = m_params.m_view;
	matrix3 oldproj = m_params.m_proj;

	CameraStrategy *cam = m_strategies[m_camStack.back()].get();
	cam->Update(delta);
	m_params = cam->GetParams();

	m_viewMod = (oldview != m_params.m_view);
	m_projMod = (oldproj != m_params.m_proj);

	if(m_viewMod || m_projMod)
	{
		UpdateFrustum();
	}
}

//=====================================================================================//
//                             static point3 CalcBestDir()                             //
//=====================================================================================//
static mll::algebra::vector3 CalcBestDir(const mll::algebra::point3 &pos, const mll::algebra::vector3 &dflt)
{
	using mll::algebra::point3;
	using mll::algebra::vector3;
	using mll::algebra::quaternion;

	GameLevel *lev = IWorld::Get()->GetLevel();
	DynObjectPool *levobj = lev->GetLevelObjects();

	vector3 bestvec = dflt;
	float bestdist = 0.0f;

	const vector3 compar = -vector3(dflt).normalize();

	if(lev)
	{
		const int iterations = 16;
		float distances[iterations] = {0.0f};
		const float delta_phi = (PIm2 / static_cast<float>(iterations));
		const float rho = PId2*0.5f;
		const float start_phi = atan2f(dflt.y,dflt.x);

		const vector3 start(cosf(rho),0.0f,sinf(rho));

		for(int i = 0; i < iterations; ++i)
		{
			const quaternion q(vector3(0.0f,0.0f,1.0f),static_cast<float>(i)*delta_phi + start_phi);
			const vector3 cur = start * q.as_matrix();

			ray r;

			r.Origin = ::point3(pos);
			r.Direction = ::point3(cur);

			r.Update();

			float dist = -1.0f;

			GameLevel::TraceResult tr;
			const bool result = lev->TraceRay3(r,mdRoof,&tr);

			if(result) dist = (tr.m_pos - ::point3(pos)).Length();

			bool result2 = false;
			if(levobj)
			{
				point3 cpos = pos;
				::point3 norm, pnt;
				std::string name;
				while(result2 = levobj->TraceRay(::point3(cpos),::point3(cur), &pnt, &norm, &name, mdRoof))
				{
					const float cdist = (pnt - ::point3(pos)).Length();
					if(dist == -1.0f)
					{
						dist = cdist;
					}
					else
					{
						dist = std::min(cdist,dist);
					}

					cpos = cur*0.01f + pnt.as_mll_point();

					if(cdist >= 0.75f) break;
				}
			}

			float factor = std::max(compar.dot(cur),0.5f);
			if(factor >= 0.9f) factor *= 1.5f;
			if(dist == -1.0f) dist = 150.0f;
			distances[i] = dist * factor;

			//if(dist == -1.0f || dist > bestdist)
			//{
			//	bestvec = cur;
			//	if(result) bestdist = dist;
			//}
		}

		int best = 0;

		float distances2[iterations];

		for(int i = 0; i < iterations; ++i)
		{
			distances2[i] = (distances[i]
						  +  distances[(i + iterations - 1) % iterations] * 0.33f
						  +  distances[(i + iterations + 1) % iterations] * 0.33f
						  +  distances[(i + iterations - 2) % iterations] * 0.15f
						  +  distances[(i + iterations + 2) % iterations] * 0.15f)
						  / ( 1.0f + 0.33f*2.0f + 0.15f*2.0f );
		}

		for(int i = 1; i < iterations; ++i)
		{
			if(distances2[best] < distances2[i])
			{
				best = i;
			}
		}

		bestvec = (start * quaternion(vector3(0.0f,0.0f,1.0f),delta_phi*(float)best + start_phi)) * -1.0f;
	}

	return bestvec;
}

//=====================================================================================//
//                               void Camera::FocusOn()                                //
//=====================================================================================//
void Camera::FocusOn(const point3 &spot, float tau)
{
	const float epsilon = 1e-4f;

	KeyAnimation ka;
	Quaternion q;
	point3 NewDir,NewPos,NewUp,NewRight;
	NewDir = point3(CalcBestDir(spot.as_mll_point(),(spot - point3(m_params.m_pos)).as_mll_vector()));

	while(NewDir.Length() < epsilon)
	{
		NewDir.x = 2.0f*frand() - 1.0f;
		NewDir.y = 2.0f*frand() - 1.0f;
		NewDir.z = 2.0f*frand() - 1.0f;
	}

	NewDir = Normalize(NewDir);

	NewDir.z = -sqrtf(NewDir.x*NewDir.x + NewDir.y*NewDir.y);
	NewDir = Normalize(NewDir);
	NewRight = Normalize(NewDir.Cross(AXISZ));
	NewUp = Normalize(NewRight.Cross(NewDir));
	NewPos = spot - 11*NewDir;

	float r[3][3];
	r[0][0] = NewRight.x; r[1][0] = NewRight.y; r[2][0] = NewRight.z;
	r[0][1] = NewUp.x;    r[1][1] = NewUp.y;    r[2][1] = NewUp.z;
	r[0][2] = -NewDir.x;  r[1][2] = -NewDir.y;  r[2][2] = -NewDir.z;
	q.FromRotationMatrix(r);
	q.w = -q.w;
	ka.SetKey(tau, q, NewPos);  
	LinkToSpline(ka,Timer::GetSeconds());
}

////=====================================================================================//
////                              void Camera::FocusOnEx()                               //
////=====================================================================================//
//void Camera::FocusOnEx(const point3 &spot, const point3 &dir, float tau)
//{
//	const float epsilon = 1e-4f;
//
//	KeyAnimation ka;
//	Quaternion q;
//	point3 NewDir,NewPos,NewUp,NewRight;
//	NewDir = dir; //spot - point3(m_params.m_pos);
//
//	while(NewDir.Length() < epsilon)
//	{
//		NewDir.x = 2.0f*frand() - 1.0f;
//		NewDir.y = 2.0f*frand() - 1.0f;
//		NewDir.z = 2.0f*frand() - 1.0f;
//	}
//
//	NewDir = Normalize(NewDir);
//
//	NewDir.z = -sqrtf(NewDir.x*NewDir.x+NewDir.y*NewDir.y);
//	NewDir = Normalize(NewDir);
//	NewRight = Normalize(NewDir.Cross(AXISZ));
//	NewUp = Normalize(NewRight.Cross(NewDir));
//	NewPos = spot - 11*NewDir;
//
//	float r[3][3];
//	r[0][0] = NewRight.x; r[1][0] = NewRight.y; r[2][0] = NewRight.z;
//	r[0][1] = NewUp.x;    r[1][1] = NewUp.y;    r[2][1] = NewUp.z;
//	r[0][2] = -NewDir.x;  r[1][2] = -NewDir.y;  r[2][2] = -NewDir.z;
//	q.FromRotationMatrix(r);
//	q.w = -q.w;
//	ka.SetKey(tau, q, NewPos);  
//	LinkToSpline(ka,Timer::GetSeconds());
//}

////=====================================================================================//
////                               void Camera::SetDest()                                //
////=====================================================================================//
//void Camera::SetDest(const point3 &, const point3 &dest)
//{
//	FocusOn(dest);
//}

//=====================================================================================//
//                            void Camera::AttachToEntity()                            //
//=====================================================================================//
void Camera::AttachToEntity(unsigned int eid)
{
	m_camStack.push_back(cmtPersonBone);
	PersonBoneCameraStrategy *cam = static_cast<PersonBoneCameraStrategy *>(m_strategies[cmtPersonBone].get());
	cam->Reset(eid);
}

//=====================================================================================//
//                           void Camera::DetachFromEntity()                           //
//=====================================================================================//
void Camera::DetachFromEntity()
{
	RemovePersonBoneCam();
	FocusOn(::point3(m_params.m_pos),1.0f);
}

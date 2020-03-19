#include "precomp.h"

#include "Camera.h"
#include "FreeCameraStrategy.h"
#include "SplineCameraStrategy.h"

//=====================================================================================//
//                    SplineCameraStrategy::SplineCameraStrategy()                     //
//=====================================================================================//
SplineCameraStrategy::SplineCameraStrategy(Camera *c, FreeCameraStrategy *s)
:	m_lastUpdate(0.0f),
	m_cam(c),
	m_normalCam(s)
{
}

//=====================================================================================//
//                         void SplineCameraStrategy::Update()                         //
//=====================================================================================//
void SplineCameraStrategy::Update(float tau)
{
	const float animlen = m_path.GetLastTime();
	if(m_lastUpdate < animlen)
	{
		float time = std::min(m_lastUpdate + tau, animlen);

		::point3 pos;
		m_path.GetTrans(&pos, time);
		m_params.m_pos = mll::algebra::point3(pos.x,pos.y,pos.z);

		Quaternion q;
		m_path.GetAngle(&q, time);
		q.w = -q.w;

		::point3 dir = Normalize(q*(-AXISZ));
		::point3 up = Normalize(q*(AXISY));
		::point3 right = Normalize(q*AXISX);

		m_params.m_dir = mll::algebra::vector3(dir.x,dir.y,dir.z);
		m_params.m_up = mll::algebra::vector3(up.x,up.y,up.z);
		m_params.m_right = mll::algebra::vector3(right.x,right.y,right.z);

		ConstructView();

		m_lastUpdate += tau;
	}

	if(m_lastUpdate >= animlen)
	{
		m_cam->RemoveSplineCam();
		m_normalCam->AcceptParams(m_params);
	}
}

//=====================================================================================//
//                         void SplineCameraStrategy::Reset()                          //
//=====================================================================================//
void SplineCameraStrategy::Reset(const KeyAnimation &ka, float tau)
{
	const float focustime = 0.1f;
	m_params = m_cam->GetParams();
	m_path = ka;

	//в начале  добавим текущее положение
	float r[3][3];
	r[0][0] = m_params.m_right.x;
	r[1][0] = m_params.m_right.y;
	r[2][0] = m_params.m_right.z;
	r[0][1] = m_params.m_up.x;
	r[1][1] = m_params.m_up.y;
	r[2][1] = m_params.m_up.z;
	r[0][2] = -m_params.m_dir.x;
	r[1][2] = -m_params.m_dir.y;
	r[2][2] = -m_params.m_dir.z;

	Quaternion q;
	q.FromRotationMatrix(r);
	q.w = -q.w;
	m_path.SetKey(-focustime, q, ::point3(m_params.m_pos));

	//в конце выправим матрицу по вертикали
	point3 NRight,NUp,NDir,NPos;
	m_path.GetTrans(&NPos,m_path.GetLastTime()+1);
	m_path.GetAngle(&q,m_path.GetLastTime()+1);
	q.w = -q.w;
	NDir = Normalize(q*(-AXISZ));
	NRight = Normalize(NDir.Cross(AXISZ));
	NUp = Normalize(NRight.Cross(NDir));
	r[0][0]=NRight.x; r[1][0]=NRight.y; r[2][0]=NRight.z;
	r[0][1]=NUp.x;    r[1][1]=NUp.y;    r[2][1]=NUp.z;
	r[0][2]=-NDir.x;  r[1][2]=-NDir.y;  r[2][2]=-NDir.z;
	q.FromRotationMatrix(r);
	q.w=-q.w;
	m_path.SetKey(m_path.GetLastTime()+focustime*0.5f,q,NPos); 

	m_lastUpdate = -focustime;
}

//=====================================================================================//
//                          void SplineCameraStrategy::Stop()                          //
//=====================================================================================//
void SplineCameraStrategy::Stop(bool toEndPoint)
{
}

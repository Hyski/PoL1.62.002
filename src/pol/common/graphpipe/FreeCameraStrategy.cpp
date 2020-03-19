#include "precomp.h"

#include "FreeCameraStrategy.h"
#include <undercover/gamelevel/grid.h>
#include <logic2/hexutils.h>

static const float _UpAngle=(TORAD(35));
static const float _DownAngle=(TORAD(89.9));
static const float _NearDist=7;
static const float _FarDist=40;//15;

//=====================================================================================//
//                      FreeCameraStrategy::FreeCameraStrategy()                       //
//=====================================================================================//
FreeCameraStrategy::FreeCameraStrategy()
:	m_aimSpot(0.0f,0.0f,0.0f)
{
	m_params.SetProjParams(CameraParams::getNearFreePlane(),m_params.m_far,m_params.m_aspect);
	//using mll::algebra::point3;
	//using mll::algebra::vector3;

	//m_params.m_dir = vector3(0.0f,1.0f,-1.0f).normalize();
	//m_params.m_right = -vector3(0.0f,0.0f,1.0f).cross(m_params.m_dir).normalize();
	//m_params.m_up = m_params.m_dir.cross(m_params.m_right).normalize();
	//m_params.m_pos = point3(0.0f,0.0f,0.0f) - m_params.m_dir * 7.0f;
	//CalcAimSpot();
	//ConstructView();
}

//=====================================================================================//
//                          void FreeCameraStrategy::Update()                          //
//=====================================================================================//
void FreeCameraStrategy::Update(float tau)
{
	float len = (m_params.m_pos - m_aimSpot).length();
	float val = 0.0f;

	if(len < _NearDist*0.9f) val = -20.0f*tau;
	if(len > _FarDist*1.1f) val = 20.0f*tau;

	if(val > 5.0f) val = 5.0f;

	//if(m_params.m_dir.z*val > _FarDist)
	//{
	//	val = (_FarDist-m_params.m_pos.z)/m_params.m_dir.z;
	//}

	//if(m_params.m_pos.z+m_params.m_dir.z*val < _NearDist)
	//{
	//	val = (_NearDist-m_params.m_pos.z)/m_params.m_dir.z;
	//}

	if(val != 0.0f)
	{
		//m_params.m_pos += m_params.m_dir*val;
		//CameraStrategy::ConstructView();
	}

	MoveBack(tau);
}

//=====================================================================================//
//                         void FreeCameraStrategy::MoveBack()                         //
//=====================================================================================//
void FreeCameraStrategy::MoveBack(float time)
{
	using mll::algebra::point3;
	using mll::algebra::vector3;
	using mll::algebra::matrix3;
	using mll::algebra::quaternion;

	vector3 DirY = m_params.m_dir;
	vector3 DirX = m_params.m_right;

	HexGrid *g = HexGrid::GetInst();

	if(g)
	{
		float deltax = 0.0f, deltay = 0.0f;
		float MaxX = g->GetSizeX(), MaxY = g->GetSizeY();
		float cosa = sinf(TORAD(60));

		const float bound = 0.0f;

		float x = m_params.m_pos.x + m_params.m_dir.x*15.0f;
		float y = m_params.m_pos.y + m_params.m_dir.y*15.0f;

		if(x > MaxX - bound)
		{
			deltax = MaxX - bound - x;
		}
		else if(x < bound)
		{
			deltax = bound - x;
		}

		if(y > MaxY*cosa - bound)
		{
			deltay = MaxY*cosa - bound - y;
		}
		else if(y < bound)
		{
			deltay = bound - y;
		}

		float c = std::min(time * powf(2.0f, sqrtf(deltax*deltax + deltay*deltay)*0.2f ),1.0f);

		deltax *= c;
		deltay *= c;

		if(deltax || deltay)
		{
			vector3 D(deltax,deltay,0.0f);
			vector3 MoveVec(DirX.dot(D)/DirX.sqr_length(),DirY.dot(D)/DirY.sqr_length(),0.0f);
			Move(MoveVec);
		}
	}
}


//=====================================================================================//
//                           void FreeCameraStrategy::Move()                           //
//=====================================================================================//
void FreeCameraStrategy::Move(const mll::algebra::vector3 &delta)
{
	using namespace mll::algebra;
	using mll::algebra::point3;
	using mll::algebra::vector3;

	vector3 dir_y = vector3(m_params.m_dir.x,m_params.m_dir.y,0.0f).normalize();//Normalize(point3(Dir.x,Dir.y,0.0f));
	vector3 dir_x = vector3(m_params.m_right.x,m_params.m_right.y,0.0f).normalize();//Normalize(point3(Right.x,Right.y,0.0f));
	m_params.m_pos += delta.x * dir_x + delta.y * dir_y;

	if(delta.x != 0.0f || delta.y != 0.0f)
	{
		CalcAimSpot();
		m_dist = (m_params.m_pos - m_aimSpot).length();
	}
	Zoom(delta.z);

	ConstructView();
}

//=====================================================================================//
//                         void FreeCameraStrategy::RotateX()                          //
//=====================================================================================//
void FreeCameraStrategy::RotateX(float phi)
{
	using mll::algebra::point3;
	using mll::algebra::vector3;
	using mll::algebra::matrix3;
	using mll::algebra::quaternion;

	phi *= -1.0f;

	const vector3 dirp(m_params.m_dir.x,m_params.m_dir.y,0.0f);
	const float curphi = -atan2f(m_params.m_dir.z,dirp.length());
	if(curphi - phi > _DownAngle)
	{
		phi = curphi - _DownAngle;
	}
	if(curphi - phi < _UpAngle)
	{
		phi = curphi - _UpAngle;
	}

	matrix3 mtr = quaternion(m_params.m_right,phi).as_matrix();

	const float dist = (m_params.m_pos - m_aimSpot).dot(m_params.m_dir);

	m_params.m_dir = m_params.m_dir * mtr;
	m_params.m_up = m_params.m_up * mtr;

	m_params.m_pos = m_aimSpot + dist*m_params.m_dir;

	ConstructView();
}

//=====================================================================================//
//                         void FreeCameraStrategy::RotateZ()                          //
//=====================================================================================//
void FreeCameraStrategy::RotateZ(float phi)
{
	using mll::algebra::point3;
	using mll::algebra::vector3;
	using mll::algebra::matrix3;
	using mll::algebra::quaternion;

	matrix3 mtr = quaternion(vector3(0.0f,0.0f,1.0f),-phi).as_matrix();

	const float dist = (m_params.m_pos - m_aimSpot).dot(m_params.m_dir);

	m_params.m_dir = m_params.m_dir * mtr;
	m_params.m_right = m_params.m_right * mtr;
	m_params.m_up = m_params.m_up * mtr;

	m_params.m_pos = m_aimSpot + dist*m_params.m_dir;

	ConstructView();
}

//=====================================================================================//
//                    void FreeCameraStrategy::RotateXAroundSelf()                     //
//=====================================================================================//
void FreeCameraStrategy::RotateXAroundSelf(float phi)
{
	using mll::algebra::point3;
	using mll::algebra::vector3;
	using mll::algebra::matrix3;
	using mll::algebra::quaternion;

	phi *= -1.0f;

	const vector3 dirp(m_params.m_dir.x,m_params.m_dir.y,0.0f);
	const float curphi = acosf(m_params.m_dir.dot(dirp)/(m_params.m_dir.length()*dirp.length()));
	if(curphi + phi > _DownAngle) phi = _DownAngle - curphi;
	if(curphi + phi < _UpAngle) phi = _UpAngle - curphi;
	matrix3 mtr = quaternion(m_params.m_right,phi).as_matrix();

	m_params.m_dir = m_params.m_dir * mtr;
	m_params.m_up = m_params.m_up * mtr;

	CalcAimSpot();
	ConstructView();
}

//=====================================================================================//
//                    void FreeCameraStrategy::RotateZAroundSelf()                     //
//=====================================================================================//
void FreeCameraStrategy::RotateZAroundSelf(float phi)
{
	using mll::algebra::point3;
	using mll::algebra::vector3;
	using mll::algebra::matrix3;
	using mll::algebra::quaternion;

	matrix3 mtr = quaternion(vector3(0.0f,0.0f,1.0f),-phi).as_matrix();

	m_params.m_dir = m_params.m_dir * mtr;
	m_params.m_right = m_params.m_right * mtr;
	m_params.m_up = m_params.m_up * mtr;

	CalcAimSpot();
	ConstructView();
}

//=====================================================================================//
//                           void FreeCameraStrategy::Zoom()                           //
//=====================================================================================//
void FreeCameraStrategy::Zoom(float dist)
{
	const mll::algebra::vector3 dir = mll::algebra::vector3(m_params.m_dir).normalize();

	if(m_dist - dist > _FarDist)
	{
		m_dist = _FarDist;
	}
	else if(m_dist - dist < _NearDist)
	{
		m_dist = _NearDist;
	}
	else
	{
		m_dist -= dist;
	}

	m_params.m_pos = m_aimSpot - dir*m_dist;
	ConstructView();
}

//=====================================================================================//
//                       void FreeCameraStrategy::CalcAimSpot()                        //
//=====================================================================================//
void FreeCameraStrategy::CalcAimSpot()
{
	using mll::algebra::point3;
	using mll::algebra::vector3;
	using mll::algebra::matrix3;
	using mll::algebra::quaternion;

	vector3 dir = m_params.m_dir;

	float step = 0.0f;
	point3 p;
	Grid *g = (Grid*)HexGrid::GetInst();

	float MaxX = 0.0f, MaxY = 0.0f;

	if(g)
	{
		MaxX = g->GetSizeX();
		MaxY = g->GetSizeY();
	}

	float h = 0.0f;

	if(g)
	{
		for(step = 0.0f; step < 40.0f; step += 0.3f)
		{
			p = m_params.m_pos + step * dir;
			const bool outoffield = g->IsOutOfRange(HexUtils::scr2hex(::point3(p)));

			if(!outoffield)
			{
				h = g->Height(::point3(p.x,p.y,p.z));
			}

			if(h > p.z)
			{
				p += dir * ((h - p.z) / dir.z);
				break;
			}
		}
	}
	else
	{
		p = m_params.m_pos - (m_params.m_pos.z / dir.z) * dir;
	}

	m_aimSpot = p;
}

//=====================================================================================//
//                      void FreeCameraStrategy::ConstructView()                       //
//=====================================================================================//
void FreeCameraStrategy::ConstructView()
{
	//using mll::algebra::point3;
	//using mll::algebra::vector3;
	//using mll::algebra::matrix3;
	//using mll::algebra::quaternion;

	//m_params.m_right = vector3( 1.0f, 0.0f, 0.0f);
	//m_params.m_up    = vector3( 0.0f, 1.0f, 0.0f);
	//m_params.m_dir   = vector3( 0.0f, 0.0f,-1.0f);

	//m_params.m_dir = m_params.m_dir * quaternion(vector3(1.0f,0.0f,0.0f),m_rotx).as_matrix();
	//m_params.m_dir = m_params.m_dir * quaternion(vector3(0.0f,0.0f,1.0f),m_rotz).as_matrix();
	//m_params.m_up = m_params.m_up * quaternion(vector3(1.0f,0.0f,0.0f),m_rotx).as_matrix();
	//m_params.m_up = m_params.m_up * quaternion(vector3(0.0f,0.0f,1.0f),m_rotz).as_matrix();
	//m_params.m_right = m_params.m_right * quaternion(vector3(1.0f,0.0f,0.0f),m_rotx).as_matrix();
	//m_params.m_right = m_params.m_right * quaternion(vector3(0.0f,0.0f,1.0f),m_rotz).as_matrix();

	//m_params.m_pos = m_aimSpot - m_params.m_dir*m_dist;

	CameraStrategy::ConstructView();
}

//=====================================================================================//
//                       void FreeCameraStrategy::SetLocation()                        //
//=====================================================================================//
void FreeCameraStrategy::SetLocation(const mll::algebra::point3 &pos, const mll::algebra::vector3 &direction)
{
	using mll::algebra::point3;
	using mll::algebra::vector3;
	using mll::algebra::matrix3;
	using mll::algebra::quaternion;

	const vector3 dir = direction;

	m_params.m_pos = pos;
	m_params.m_dir = dir;

	vector3 tup(0.0f,0.0f,1.0f);
	vector3 right = tup.cross(dir).normalize();
	vector3 up = dir.cross(right).normalize();

	m_params.m_up = up;
	m_params.m_right = right;

	CameraStrategy::ConstructView();
}

//=====================================================================================//
//                       void FreeCameraStrategy::AcceptParams()                       //
//=====================================================================================//
void FreeCameraStrategy::AcceptParams(const CameraParams &p)
{
	const CameraParams oldparams = m_params;
	m_params = p;
	m_params.m_near = oldparams.m_near;
	m_params.m_far = oldparams.m_far;
	m_params.m_fov = oldparams.m_fov;
	m_params.m_proj = oldparams.m_proj;
	m_params.m_aspect = oldparams.m_aspect;

	CalcAimSpot();
	m_dist = (m_params.m_pos - m_aimSpot).length();
}
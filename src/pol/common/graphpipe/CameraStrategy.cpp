#include "precomp.h"
#include "CameraStrategy.h"
#include "../Utils/VFileWpr.h"

namespace CameraStrategyDetails
{

//=====================================================================================//
//                              class CameraScriptReader                               //
//=====================================================================================//
struct CameraScriptReader
{
	float m_nearPerson;
	float m_nearFree;
	float m_far;

	CameraScriptReader()
	:	m_nearPerson(0.1f),
		m_nearFree(1.0f),
		m_far(420.0f)
	{
		PoL::VFileWpr vfile("scripts/camera.txt");

		if(vfile.get() && vfile.get()->Size())
		{
			using mll::utils::table;

			PoL::VFileBuf buf(vfile.get());
			std::istream in(&buf);

			table tbl;
			in >> tbl;

			for(int i = 0; i < tbl.height(); ++i)
			{
				if(tbl.col(0)[i] == "near-clip-plane-person")
				{
					m_nearPerson = boost::lexical_cast<float>(tbl.col(1)[i]);
				}
				else if(tbl.col(0)[i] == "near-clip-plane-free")
				{
					m_nearFree = boost::lexical_cast<float>(tbl.col(1)[i]);
				}
				else if(tbl.col(0)[i] == "far-clip-plane")
				{
					m_far = boost::lexical_cast<float>(tbl.col(1)[i]);
				}
			}
		}
	}
};

static const CameraScriptReader &getCameraParams()
{
	static CameraScriptReader rdr;
	return rdr;
}

}

using namespace CameraStrategyDetails;

//=====================================================================================//
//                          float CameraParams::getFarPlane()                          //
//=====================================================================================//
float CameraParams::getFarPlane()
{
	return getCameraParams().m_far;
}

//=====================================================================================//
//                      float CameraParams::getNearPersonPlane()                       //
//=====================================================================================//
float CameraParams::getNearPersonPlane()
{
	return getCameraParams().m_nearPerson;
}

//=====================================================================================//
//                       float CameraParams::getNearFreePlane()                        //
//=====================================================================================//
float CameraParams::getNearFreePlane()
{
	return getCameraParams().m_nearFree;
}

//=====================================================================================//
//                            CameraParams::CameraParams()                             //
//=====================================================================================//
CameraParams::CameraParams()
:	m_pos(0.0f,0.0f,0.0f),
	m_near(getNearPersonPlane()),
	m_far(getFarPlane()),
	m_fov(TORAD(60)),
	m_aspect(768.0f/1024.0f),
	m_view(mll::algebra::matrix3::identity()),
	m_invview(mll::algebra::matrix3::identity()),
	m_proj(mll::algebra::matrix3::identity())
{
	using mll::algebra::vector3;

	m_dir = vector3(1.0f,0.0f,-1.0f).normalize();
	m_right = vector3(0.0f,0.0f,1.0f).cross(m_dir).normalize();
	m_up = -m_dir.cross(m_right).normalize();

	SetProjParams(m_near,m_far,m_aspect);
}

//=====================================================================================//
//                         void CameraParams::SetProjParams()                          //
//=====================================================================================//
void CameraParams::SetProjParams(float dnear, float dfar, float aspect)
{
	m_near = dnear;
	m_far = dfar;
	m_aspect = aspect;

	m_proj = mll::algebra::matrix3::zero();

    float w = m_aspect * ( cosf(m_fov*0.5f)/sinf(m_fov*0.5f) );
    float h =    1.0f  * ( cosf(m_fov*0.5f)/sinf(m_fov*0.5f) );
    float Q = m_far / ( m_far - m_near );

    m_proj._00 = w;
    m_proj._11 = h;
    m_proj._22 = Q;
    m_proj._23 = 1.0f;
    m_proj._32 = -Q*m_near;
}

//=====================================================================================//
//                          CameraStrategy::CameraStrategy()                           //
//=====================================================================================//
CameraStrategy::CameraStrategy()
{
}
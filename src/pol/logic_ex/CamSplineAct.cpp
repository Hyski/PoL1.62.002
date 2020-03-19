#include "precomp.h"
#include "CamSplineAct.h"
#include <undercover/GameLevel/GameLevel.h>
#include <common/GraphPipe/Camera.h>

namespace PoL
{

//=====================================================================================//
//                            CamSplineAct::CamSplineAct()                             //
//=====================================================================================//
CamSplineAct::CamSplineAct(const std::string &camname, float dur)
:	m_camera(camname),
	m_started(false),
	m_ended(false),
	m_length(dur)
{
}

//=====================================================================================//
//                             void CamSplineAct::start()                              //
//=====================================================================================//
void CamSplineAct::start()
{
	assert( !isStarted() );
	assert( !isEnded() );

	::IWorld *world = ::IWorld::Get();
	const ::KeyAnimation *anim = world->GetLevel()->GetCameraPath(m_camera);
	m_roofsVisible = IWorld::Get()->GetLevel()->IsRoofVisible();

	if(anim)
	{
		::KeyAnimation ranim;
		if(m_length != 0.0f)
		{
			::Quaternion quat;
			::point3 pos;

			anim->GetTrans(&pos,0.0f);
			anim->GetAngle(&quat,0.0f);

			ranim.SetKey(0.0f,quat,pos);
			ranim.SetKey(m_length,quat,pos);
		}
		else
		{
			ranim = *anim;
		}

		world->GetCamera()->LinkToSpline(ranim,0.0f);
		m_startTime = getCurrentTime() + ranim.GetLastTime();
	}

	IWorld::Get()->GetLevel()->SetRoofVisible(true);
	m_started = true;
}

//=====================================================================================//
//                             void CamSplineAct::update()                             //
//=====================================================================================//
void CamSplineAct::update(float tau)
{
	assert( isStarted() );
	assert( !isEnded() );

	if(getCurrentTime() - m_startTime >= 0.0f)
	{
		m_ended = true;
		IWorld::Get()->GetLevel()->SetRoofVisible(m_roofsVisible);
	}
}

}

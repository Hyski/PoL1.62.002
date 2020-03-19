#include "logicdefs.h"

#include "Entity.h"
#include "AIUtils.h"
#include "ThrowManager.h"
#include "ThrowActivity.h"

namespace ThrowActivityDetail
{

	//при превышении этого damage'a проигрывается эффект крови
	const float throw_scatter_radius = 2.0f;
	//const float near_shoot_distance  = 1.0f;

	//произвести разброс в окружности
	point3 MakeCircleScatter(const point3& center, float radius)
	{
		point3 dir = AXISX;
		dir = dir * radius * aiNormRand();

		Quaternion qt;
		qt.FromAngleAxis(aiNormRand()*PIm2, AXISZ);

		dir = qt * dir;
		return center + dir; 
	}
}

using namespace ThrowActivityDetail;

//=====================================================================================//
//                           ThrowActivity::ThrowActivity()                            //
//=====================================================================================//
ThrowActivity::ThrowActivity(HumanEntity* human, const point3& to, float accuracy, ThrowManager* mgr, Driver* drv, Camerawork* cam)
:	BaseActivity(drv, cam),
	m_end_time(0),
	m_human(human),
	m_accuracy(accuracy),
	m_flags(0),
	m_throw(mgr),
	m_to(to),
	m_action(GraphEntity::AT_TURN),
	m_activity_sender(human)
{
#ifdef _HOME_VERSION
	Timer::Update();
	m_startTime = Timer::GetSeconds();
	TIMING_LOG("Throw activity\nstarted\t" << m_startTime << "\n");
#endif
}

//=====================================================================================//
//                           ThrowActivity::~ThrowActivity()                           //
//=====================================================================================//
ThrowActivity::~ThrowActivity()
{
#ifdef _HOME_VERSION
	Timer::Update();
	TIMING_LOG("Throw activity\nended\t" << Timer::GetSeconds() << "\n");
#endif
	delete m_throw;
}

//=====================================================================================//
//                              bool ThrowActivity::Run()                              //
//=====================================================================================//
bool ThrowActivity::Run(activity_command cmd)
{
#ifdef _HOME_VERSION
	Timer::Update();
	if(Timer::GetSeconds() - m_startTime > 0.3f && isHidMovScreen())
	{
		int a = 0;
	}
#endif

	if(m_flags & F_CALC_STEP)
	{
		cmd = DrvCalcStep(cmd);
		m_flags &= ~F_CALC_STEP;
	}

	if(m_flags & F_CALC_MOVEPNTS)
	{
		cmd = DrvCalcMovepnts(cmd);
		m_flags &= ~F_CALC_MOVEPNTS;
	}

	PlayThrow();

	return DrvRun(m_throw->Run() || (m_action != GraphEntity::AT_STAY));
}

//=====================================================================================//
//                           void ThrowActivity::PlayThrow()                           //
//=====================================================================================//
//пориграть бросок гранаты
void ThrowActivity::PlayThrow()
{
	//while(m_end_time > Timer::GetSeconds())
	//{
	//	Timer::Update();
	//}

	if(m_end_time > Timer::GetSeconds())       
		return;

	switch(m_action)
	{
	case GraphEntity::AT_TURN:
		{
			//развернуть человека в сторону броска
			point3 dir = m_to - m_human->GetGraph()->GetPos3();
			m_end_time = m_human->GetGraph()->SetAngle(Dir2Angle(dir));
			m_action   = GraphEntity::AT_SHOOT;

			m_flags |= F_CALC_STEP;
		}
		break;

	case GraphEntity::AT_SHOOT:
		{
			m_end_time = m_human->GetGraph()->ChangeAction(GraphEntity::AT_SHOOT);
			m_action   = GraphEntity::AT_RECOIL;

			m_flags |= F_CALC_STEP;
		}
		break;

	case GraphEntity::AT_RECOIL:
		{
			m_flags |= F_CALC_STEP;
			m_flags |= F_CALC_MOVEPNTS;

			const float dist = (m_human->GetGraph()->GetPos3() - m_to).Length();
			const point3 to = MakeCircleScatter(m_to, AIUtils::CalcGrenadeRange(m_human,dist) * (1 - m_accuracy));
			m_throw->Init(m_human, to);

			m_end_time = m_human->GetGraph()->ChangeAction(GraphEntity::AT_RECOIL);
			m_action   = GraphEntity::AT_STAY;
		}
		break;

	case GraphEntity::AT_STAY:
		return;
	}
}

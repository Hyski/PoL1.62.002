#include "precomp.h"

#include "Person.h"
#include "../Game.h"
#include "RealEntityMover.h"
#include <logic2/HexGrid.h>
#include <logic2/HexUtils.h>

#define PARMOVE_LOG(L)		MLL_MAKE_DEBUG_LOG("parmove.log",L)

namespace PoL
{

//=====================================================================================//
//                         RealEntityMover::RealEntityMover()                          //
//=====================================================================================//
RealEntityMover::RealEntityMover(ANIMATION type, RealEntity *ent, const pnt_vec_t &path,
								 MovementController *ctrl)
:	m_ent(ent),
	m_path(path),
	m_ctrl(ctrl),
	m_lastHex(0),
	m_targetHex(path.size()),
	m_speed(1.0f),
	m_startTime(Timer::GetSeconds()),
	m_lastUpdate(m_startTime),
	m_initAngle(m_ent->GetAngle()),
	m_initPos(m_ent->GetLocation()),
	m_needSpeedUp(false),
	m_splineReady(false),
	m_paused(false),
	m_lifeCtrl(ctrl->getLifeTokenCtrl())
{
	std::reverse(m_path.begin(),m_path.end());

	if(!m_path.empty())
	{
		m_ent->PoLSetAction(type,m_startTime);
		m_speed = m_ent->GetAnimVelocity();
		assert( m_speed >= 1e-4f );
	}
}

//=====================================================================================//
//                         RealEntityMover::~RealEntityMover()                         //
//=====================================================================================//
RealEntityMover::~RealEntityMover()
{
	if(m_lifeCtrl.lock())
	{
		m_ctrl->onEnd();
	}

	if(m_ent->GetCurState() != DEAD)
	{
		m_ent->SetAction(STAY,Timer::GetSeconds());
	}
}

//=====================================================================================//
//                        void RealEntityMover::prepareSpline()                        //
//=====================================================================================//
void RealEntityMover::prepareSpline()
{
	assert( !m_path.empty() );
	assert( m_splineReady == false );

	m_spline.addPoint(m_initPos);

	for(size_t i = 0; i < m_path.size(); ++i)
	{
		const point3 p1 = i ? HexGrid::GetInst()->Get(m_path[i-1]) : m_initPos;
		const point3 p2 = HexGrid::GetInst()->Get(m_path[i]);
		m_spline.addPoint((p1+p2)*0.5f);
	}

	m_spline.addPoint(HexGrid::GetInst()->Get(m_path.back()));

	m_spline.bake(!canMoveFast());
	m_splineReady = true;
}

//=====================================================================================//
//            const RealEntityMover::Spline_t &RealEntityMover::getSpline()            //
//=====================================================================================//
const RealEntityMover::Spline_t &RealEntityMover::getSpline()
{
	if(!m_splineReady)
	{
		prepareSpline();
	}

	return m_spline;
}

//=====================================================================================//
//              MovementController::Command RealEntityMover::doVisitHex()              //
//=====================================================================================//
MovementController::Command RealEntityMover::doVisitHex()
{
	MovementController::Command result = m_ctrl->onStep();
	//if(result == MovementController::cmdContinue)
	{
		PARMOVE_LOG(m_ent->GetSysName() << ": increased m_lastHex to " << m_lastHex+1 << "\n");
		++m_lastHex;
	}

	return result;
}

//=====================================================================================//
//                         bool RealEntityMover::canMoveFast()                         //
//=====================================================================================//
bool RealEntityMover::canMoveFast() const
{
	return m_needSpeedUp || !m_ent->Enabled || !Game::RenderEnabled();
}

//=====================================================================================//
//                          bool RealEntityMover::moveFast()                           //
//=====================================================================================//
bool RealEntityMover::moveFast()
{
	HexGrid * const grid = HexGrid::GetInst();

	for(; !isEnded() && canMoveFast();)
	{
		setLoc(m_lastHex);

		if(doVisitHex() == MovementController::cmdStop)
		{
			m_targetHex = m_lastHex;
		}
	}

	return !isEnded() || m_ent->GetCurState() == DEAD;
}

//=====================================================================================//
//                           void RealEntityMover::setLoc()                            //
//=====================================================================================//
void RealEntityMover::setLoc(size_t i)
{
	HexGrid *grid = HexGrid::GetInst();

	point3 pos = grid->Get(m_path[i]);
	float angle = m_initAngle;

	if(i)
	{
		angle = angleFromVector(grid->Get(m_path[i]) - grid->Get(m_path[i-1]));
	}
	else
	{
		angle = angleFromVector(grid->Get(m_path[i]) - m_initPos);
	}

	m_ent->SetLocation(pos,angle);
}

//=====================================================================================//
//                      float RealEntityMover::angleFromVector()                       //
//=====================================================================================//
float RealEntityMover::angleFromVector(const point3 &vec)
{
	return atan2f(vec.x,-vec.y);
}

//=====================================================================================//
//                         float RealEntityMover::calcAngle()                          //
//=====================================================================================//
float RealEntityMover::calcAngle(float dist)
{
	if(dist == 0.0f) return m_initAngle;

	const float step = 0.5f;
	const float pdist = std::max(0.0f,dist-step);

	const Spline_t &spline = getSpline();
	return angleFromVector(spline.getPoint(dist) - spline.getPoint(pdist));
}

//=====================================================================================//
//                        void RealEntityMover::placeOnSpline()                        //
//=====================================================================================//
void RealEntityMover::placeOnSpline(float dist)
{
	float angle = calcAngle(dist);
	m_ent->SetLocation(getSpline().getPoint(dist),angle);
}

//=====================================================================================//
//                        bool RealEntityMover::isEnded() const                        //
//=====================================================================================//
bool RealEntityMover::isEnded() const
{
	return m_lastHex >= m_targetHex;
}

//=====================================================================================//
//                              static float findBorder()                              //
//=====================================================================================//
template<typename S>
static float findBorder(const S &spline, float a, float b)
{
	const int iterations = 8;

	float c = (a+b)*0.5f;
	ipnt2_t pa = HexUtils::scr2hex(spline.getPoint(a));
	ipnt2_t pb = HexUtils::scr2hex(spline.getPoint(b));
	ipnt2_t pc = HexUtils::scr2hex(spline.getPoint(c));

	for(int i = 0; i < iterations; ++i)
	{
		if(pa == pc)
		{
			pa = pc;
			a = c;
		}
		else
		{
			pb = pc;
			b = c;
		}

		pc = HexUtils::scr2hex(spline.getPoint(c));
	}

	return c;
}

//=====================================================================================//
//                        float RealEntityMover::getHexStart()                         //
//=====================================================================================//
float RealEntityMover::getHexStart(size_t pos)
{
	assert( m_path.size() > pos );

	while(m_hexStarts.size() <= pos)
	{
		const size_t oldsize = m_hexStarts.size();
		const float step = 0.01f;
		const size_t curhex = m_hexStarts.size();
		const float start = m_hexStarts.empty() ? 0.0f : m_hexStarts.back();
		const Spline_t &spline = getSpline();

		for(float cur = start; !spline.isFinished(cur); cur += step)
		{
			if(HexUtils::scr2hex(spline.getPoint(cur)) == m_path[curhex])
			{
				m_hexStarts.push_back(findBorder(spline,cur-step,cur));
				break;
			}
		}

		// Если сплайн не заходит в нужный нам хекс
		if(m_hexStarts.size() == oldsize)
		{
			if(curhex+1 == m_path.size())
			{
				m_hexStarts.push_back(spline.getDistance());
			}
			else
			{
				for(float cur = start; !spline.isFinished(cur); cur += step)
				{
					if(HexUtils::scr2hex(spline.getPoint(cur)) == m_path[curhex+1])
					{
						m_hexStarts.push_back((cur + start) * 0.5f);
						break;
					}
				}
			}
		}
	}

	return m_hexStarts[pos];
}

//=====================================================================================//
//                        float RealEntityMover::getHexCenter()                        //
//=====================================================================================//
float RealEntityMover::getHexCenter(size_t pos)
{
	assert( m_path.size() > pos );

	while(m_hexCentres.size() <= pos)
	{
		if(pos == m_path.size()-1)
		{
			m_hexCentres.push_back(getSpline().getDistance());
		}
		else
		{
			const float step = 0.01f;
			const size_t curhex = m_hexCentres.size();
			const float start = getHexStart(curhex);
			const float end = getHexStart(curhex+1);
			const point3 pos = HexGrid::GetInst()->Get(m_path[curhex]);
			float mindist = 1e10f;
			float minpos = 1e10f;
			const Spline_t &spline = getSpline();

			for(float cur = start; cur < end; cur += step)
			{
				float dist = (spline.getPoint(cur) - pos).SqrLength();
				if(dist < mindist)
				{
					mindist = dist;
					minpos = cur;
				}
			}

			m_hexCentres.push_back(minpos);
		}
	}

	return m_hexCentres[pos];
	//return static_cast<float>(pos+1);
}

//=====================================================================================//
//                      size_t RealEntityMover::getHexFromDist()                       //
//=====================================================================================//
size_t RealEntityMover::getHexFromDist(float dist)
{
	point3 cpos = getSpline().getPoint(dist);
	ipnt2_t hpos = HexUtils::scr2hex(cpos);
	pnt_vec_t::iterator it = std::find(m_path.begin(),m_path.end(),hpos);
	return it != m_path.end() ? it - m_path.begin() : WrongIndex;
}

//=====================================================================================//
//                         bool RealEntityMover::moveNormal()                          //
//=====================================================================================//
bool RealEntityMover::moveNormal(float tau)
{
	if(m_paused)
	{
		MovementController::MotionCommand mc = m_ctrl->canMoveToNextHex();

		switch(mc)
		{
			case MovementController::mcStop:
				m_targetHex = m_lastHex;
				m_paused = false;
				break;
			case MovementController::mcNothing:
				break;
			case MovementController::mcUnpause:
				m_paused = false;
				break;
		}
	}

	if(m_paused)
	{
		m_startTime += tau - m_lastUpdate;
		m_lastUpdate = tau;
		return true;
	}

	const float lastUpdate = m_lastUpdate;
	m_lastUpdate = tau;

	// Позиция на сплайне, где чувак сейчас находится
	float dist = m_speed*(tau - m_startTime);
	float prevdist = m_speed*(lastUpdate - m_startTime);
	assert( dist >= 0.0f );
	assert( prevdist >= 0.0f );

	if(!isEnded())
	{
		size_t maxhex = m_lastHex;
		int count = 0;

		const float step = 0.1f;
		HexGrid *grid = HexGrid::GetInst();

		for(float cur = prevdist; cur < dist+step; cur += step)
		{
			float rcur = std::min(cur,dist);
			size_t idx = getHexFromDist(rcur);
			if(idx != WrongIndex && idx >= maxhex && idx <= m_targetHex)
			{
				maxhex = idx;
				count = std::max((size_t)count,idx-m_lastHex+1);
			}
		}

		while(count-- && !isEnded())
		{
			MovementController::MotionCommand mc = m_ctrl->canMoveToNextHex();

			switch(mc)
			{
				case MovementController::mcStop:
					m_targetHex = m_lastHex;
					m_paused = false;
					break;
				case MovementController::mcNothing:
					assert( !m_paused );
					setLoc(m_lastHex);
					if(doVisitHex() == MovementController::cmdStop)
					{
						m_targetHex = m_lastHex;
					}
					break;
				case MovementController::mcPause:
					m_paused = true;
					return true;
				case MovementController::mcUnpause:
					assert( !"Must not reach this place!" );
					break;
			}
		}
	}

	if(isEnded())
	{
		assert( !m_paused );
		if(!m_targetHex)
		{
			m_ent->SetLocation(m_initPos,m_initAngle);
			return false;
		}
		else if(dist >= getHexCenter(m_targetHex-1))
		{
			setLoc(m_targetHex-1);
			return false;
		}
	}

	//4. установить человека.
	placeOnSpline(dist);

	return true;
}

//=====================================================================================//
//                           void RealEntityMover::update()                            //
//=====================================================================================//
bool RealEntityMover::update(float tau)
{
	if(!m_lifeCtrl.lock() || m_path.empty()) return false;
	m_needSpeedUp = m_needSpeedUp || m_ctrl->needSpeedup();

	return canMoveFast() ? moveFast() : moveNormal(tau);
}

}

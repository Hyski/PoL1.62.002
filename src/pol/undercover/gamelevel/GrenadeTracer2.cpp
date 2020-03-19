#include "precomp.h"
#include "GameLevel.h"
#include "DynObjectPool.h"
#include "GrenadeTracer2.h"
#include <logic2/logicdefs.h>
#include <logic2/AIUtils.h>
#include <Common/GraphPipe/culling.h>

namespace GrenadeTracerDetail
{

//=====================================================================================//
//                                 struct TraceResult                                  //
//=====================================================================================//
struct TraceResult
{
	enum HitSurfaceType
	{
		hstNoHit,
		hstLevel,
		hstDynObject,
		hstCharacter,
		hstShield
	};

	HitSurfaceType m_surfaceType;
	point3 m_pos;
	point3 m_normal;
	float m_coord;
};

//=====================================================================================//
//                               class TraceGrenadeUtils                               //
//=====================================================================================//
class TraceGrenadeUtils
{
	GameLevel *m_lev;
	DynObjectPool *m_objpool;
	CharacterPool *m_cpool;
	ShieldPool *m_spool;
	CharacterPool::Handle m_entityToSkip;

public:
	TraceGrenadeUtils(CharacterPool::Handle e)
	:	m_lev(IWorld::Get()->GetLevel()),
		m_objpool(m_lev->GetLevelObjects()),
		m_cpool(IWorld::Get()->GetCharacterPool()),
		m_spool(ShieldPool::GetInst()),
		m_entityToSkip(e)
	{
	}

	bool TraceShields(const point3 &origin, const point3 &dir, point3 *pos, point3 *norm) const
	{
		const float bignum = 1e+7f;
		float result = bignum;
		const float shieldHeight = 3.0f;

		ShieldPool::iterator s1 = m_spool->begin();
		ShieldPool::iterator se = m_spool->end();

		point3 rpos, rnorm;
		for(; s1 != se; ++s1)
		{
			Cylinder c(s1->GetPos(), shieldHeight, s1->GetRadius());

			if(c.TraceSegment(origin,dir,&rpos,&rnorm)
				&& result > dir.Dot(rpos-origin))
			{
				*pos = rpos;
				*norm = rnorm;
				result = dir.Dot(rpos-origin);
			}
		}
		
		return result != bignum;
	}

	TraceResult TraceSegmentForGrenade(const ray &r) const
	{
		TraceResult result;
		result.m_surfaceType = TraceResult::hstNoHit;
		result.m_coord = 1e+7f;

		const float rlen = r.Direction.Length();
		const point3 dir = Normalize(r.Direction);
		point3 pos, norm;

		GameLevel::TraceResult tr;

		// ѕроверка на пересечение с геометрией уровн€
		if(m_lev->TraceSegment3(r, mdGhost, &tr))
		{
			result.m_pos = r.Origin + tr.m_dist*r.Direction;
			result.m_normal = tr.m_norm;
			result.m_coord = tr.m_dist*rlen;
			result.m_surfaceType = TraceResult::hstLevel;
		}

		// ѕроверка на пересечение с динамическими объектами (DynObject)
		std::string objname;
		if(m_objpool->TraceSegment(r.Origin,r.Direction,&pos,&norm,&objname)
			&& result.m_coord > dir.Dot(pos-r.Origin))
		{
			result.m_pos = pos;
			result.m_normal = norm;
			result.m_coord = dir.Dot(pos - r.Origin);
			result.m_surfaceType = TraceResult::hstDynObject;
		}

		// ѕроверка на пересечение с персонажами
		CharacterPool::Handle who;
		if(m_cpool->TraceSegment(r.Origin,r.Direction,&pos,&norm,&who) 
			&& who != m_entityToSkip
			&& result.m_coord > dir.Dot(pos-r.Origin))
		{
			result.m_pos = pos;
			result.m_normal = norm;
			result.m_coord = dir.Dot(pos - r.Origin);
			result.m_surfaceType = TraceResult::hstCharacter;
		}

		if(TraceShields(r.Origin,r.Direction,&pos,&norm)
			&& result.m_coord > dir.Dot(pos-r.Origin))
		{
			result.m_pos = pos;
			result.m_normal = norm;
			result.m_coord = dir.Dot(pos - r.Origin);
			result.m_surfaceType = TraceResult::hstShield;
		}

		result.m_coord /= rlen;
		return result;
	}
};

struct GrenadeParams
{
	float m_wantedSegmentLength;
	float m_dampingFactor;
	float m_minVelocity;
	float m_flytimeFactor;
	float m_reflectDistance;

	GrenadeParams()
	:	m_wantedSegmentLength(0.25f),
		m_dampingFactor(0.6f),
		m_minVelocity(2.1f),
		m_flytimeFactor(0.125f),
		m_reflectDistance(0.01f)
	{
		VFile *f = DataMgr::Load("scripts/grenade.txt");
		if(f && f->Size())
		{
			std::string data;
			data.resize(f->Size());
			std::copy_n(f->Data(),f->Size(),data.begin());
			std::istringstream sstr(data);
			std::string name;

			sstr >> name >> m_wantedSegmentLength;
			assert( name == "segment-length" );
			sstr >> name >> m_dampingFactor;
			assert( name == "damping-factor" );
			sstr >> name >> m_minVelocity;
			assert( name == "minimal-velocity" );
			sstr >> name >> m_flytimeFactor;
			assert( name == "flytime-factor" );
			sstr >> name >> m_reflectDistance;
			assert( name == "reflect-distance" );
		}
	}
};

}

//using namespace GrenadeTracerDetail;

//=====================================================================================//
//                               void PolTraceGrenade()                                //
//=====================================================================================//
void PolTraceGrenade(const point3 &origin, const point3 &target, KeyAnimation *anim,
					 CharacterPool::Handle skip, float maxTime)
{
	using GrenadeTracerDetail::TraceResult;
	using GrenadeTracerDetail::GrenadeParams;
	using GrenadeTracerDetail::TraceGrenadeUtils;

	static const GrenadeParams gp;
	const float wantedTau = (target-origin).Length()*gp.m_flytimeFactor;
	const float epsilon = 1e-4f;

	const point3 g(0.0f,0.0f,-9.81f);
	const point3 vel0 = (target-origin)/wantedTau - wantedTau*g*0.5f;

	point3 pos = origin;
	point3 vel = vel0;
	float curTime = 0.0f;
	TraceGrenadeUtils utils(skip);
	anim->SetKey(curTime,pos);
	int bounceCount = 0;

	for(; curTime < maxTime && vel.Length() > gp.m_minVelocity && bounceCount <= PoL::Inv::GrenadeMaxBounce; )
	{
		float tau = gp.m_wantedSegmentLength/vel.Length();
		assert( tau < wantedTau * 1.5f );
		const point3 velDelta = tau*g;
		const point3 newvel = vel+velDelta;
		const float seglength = newvel.Length();
		const point3 normvel = newvel/seglength;
		const ray r(pos,tau*seglength*normvel);
		TraceResult res = utils.TraceSegmentForGrenade(r);

		if(res.m_surfaceType) // есть пересечение
		{
			assert( res.m_coord - 1.0f <= epsilon );
			assert( res.m_coord >= 0.0f );
			tau *= res.m_coord;
		}

		pos += tau*newvel;
		curTime += tau;
		vel = newvel;

		if(res.m_surfaceType) // если требуетс€, то отразим гранату от поверхности
		{
			if(res.m_normal.Dot(vel) > 0.0f) res.m_normal *= -1.0f;
			res.m_normal = Normalize(res.m_normal);
			vel -= 2.0f*vel.Dot(res.m_normal)*res.m_normal;
			vel *= gp.m_dampingFactor;
			pos += res.m_normal*gp.m_reflectDistance;
			++bounceCount;
		}

		anim->SetKey(curTime,pos);
	}
}

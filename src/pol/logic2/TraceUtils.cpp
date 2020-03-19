#include "logicdefs.h"

#include <common/graphpipe/graphpipe.h>
#include <undercover/gamelevel/gamelevel.h>

#include "Entity.h"
#include "Graphic.h"
#include "TraceUtils.h"
#include "DirtyLinks.h"
#include "aiutils.h"
#include <undercover/GameLevel/DynObjectPool.h>
#include <common/utils/profiler.h>
#include <undercover/GameLevel/GrenadeTracer2.h>

//=====================================================================================//
//                               void ShotTracer::Calc()                               //
//=====================================================================================//
void ShotTracer::Calc()
{
	//CodeProfiler CP("SHOTtracer");
	const float BOGUS=100000.f;
	float BestDist=BOGUS;
	material_type BestMat=MT_AIR;
	Touch.TouchPoint=Ray.Origin+Ray.Direction*50.f;

	unsigned int passThru = 0;

	if((m_flags & F_VISROOFS) && !IWorld::Get()->GetLevel()->IsRoofVisible()) passThru |= mdRoof;
	if(m_flags & F_SIGHT) passThru |= mdTransparent;
	if(m_flags & F_CURSOR) passThru |= mdCursor;
	if(m_flags & F_SKIP_GHOST) passThru |= mdGhost;

	point3 tp;

	GameLevel::TraceResult gltr;

	//проверим Bsp
	float Dist; point3 Norm;
	bool gltres;

	if(Radius == 0.0f)
	{
		gltres = IWorld::Get()->GetLevel()->TraceRay3(Ray,passThru,&gltr);
		gltr.m_dist /= Ray.Direction.Length();
	}
	else
	{
		ray SegRay(Ray);
		const float factor = Radius / SegRay.Direction.Length();
		SegRay.Direction *= factor;
		if(gltres = IWorld::Get()->GetLevel()->TraceSegment3(SegRay,passThru,&gltr))
		{
			gltr.m_dist *= factor;
			gltres = gltr.m_dist < Radius;
		}
	}

	if(gltres)
	{
		Touch.TouchPoint = gltr.m_pos;
		Touch.TouchNorm = gltr.m_norm;
		BestDist = gltr.m_dist;
		BestMat = MT_WALL;
	}
	//проверим Gamelevel
	//...

	GameLevel *g = DirtyLinks::GetGameLevel();
	DynObjectPool *dopool = g->GetLevelObjects();
	std::string Name;
	point3 p;
	if(dopool && dopool->TraceRay(Ray.Origin,Ray.Direction,&p,&Norm,&Name, passThru))
	{
		float Dist=(p-Ray.Origin).Length();
		if(!(Dist>=BestDist||(Radius!=0.f&&Dist>Radius)))
		{
			Touch.TouchPoint=Ray.Origin+Ray.Direction*Dist;
			Touch.TouchNorm=Norm;
			Touch.ObjName=Name;
			BestDist=Dist;
			BestMat=MT_OBJECT;
		}
	}

	//проход по массиву существ
	EntityPool::iterator itor     = EntityPool::GetInst()->begin(ET_ALL_ENTS, PT_ALL_PLAYERS, EA_NOT_INCREW);
	EntityPool::iterator end_itor = EntityPool::GetInst()->end();

	for(;itor != end_itor; ++itor)
	{
		//id существа это
		unsigned CurId = itor->GetEID();

		if(id==CurId || itor->IsRaised(EA_INCREW) || (m_fskip_invisible_ents && !itor->GetGraph()->IsVisible()))
			continue;

		if(m_skip_entities_with_camera && itor->GetGraph()->IsCameraAttached()) continue;

		//получить доступ к граф. оболочке
		GraphEntity *gi= itor->GetGraph(); //указат. на Person или Vehicle
		if(!gi->TraceRay(Ray,&Dist,&Norm,m_ent_as_box)) continue;
		if(Dist>=BestDist)  continue;
		if(Radius==0.f||Dist<Radius)
		{
			Touch.TouchPoint=Ray.Origin+Ray.Direction*Dist;
			Touch.TouchNorm=Norm;
			BestDist=Dist;
			BestMat=MT_ENTITY;
			Touch.Ent=CurId;
		}
	}
	if(!m_skip_shields)
	{
		ShieldPool::iterator s1=ShieldPool::GetInst()->begin(),se=ShieldPool::GetInst()->end();
		point3 N;     float D;
		for(;s1!=se;s1++)
		{
			Cylinder c(s1->GetPos(),3, s1->GetRadius());
			if(c.TraceRay(Ray,&D,&N))
			{
				Touch.TouchPoint=Ray.Origin+Ray.Direction*D;
				Touch.TouchNorm=N;
				BestDist=D;
				BestMat=MT_SHIELD;
			}
		}

	}
	Touch.Mat=BestMat;
}

//=====================================================================================//
//                             void _GrenadeTracer::Calc()                             //
//=====================================================================================//
void _GrenadeTracer::Calc()
{
	const float epsilon = 1e-4f;

	if(Ray.Direction.Length() < epsilon)
	{
		Ray.Direction = point3(0.0f,0.0f,0.1f);
	}

	const point3 from = Ray.Origin;
	point3 to = from + Normalize(Ray.Direction)*Radius;

	PolTraceGrenade(from,to,&ka,SkipEnt,m_Time);

	ka.GetTrans(&Touch.TouchPoint,ka.GetLastTime());
	Touch.TouchNorm=AXISZ;
	Touch.Ent=0;
	Touch.Mat=MT_AIR;
}

//=====================================================================================//
//                                    class LOSdraw                                    //
//=====================================================================================//
class LOSdraw
{
public:
	enum{LINES=50};

private:
	struct Init
	{
		Init()
		{
			LOSdraw::primi.Verts.reserve(LINES*2);
			LOSdraw::primi.Prim = Primi::LINES;
			LOSdraw::primi.Contents = Primi::NEEDTRANSFORM;
		}
	};
	friend struct Init;

	static Primi primi;
	static Init primi_init;

public:
	static void AddLine(point3 &from, point3 &to, unsigned colfrom=0xffffff,unsigned colto=0xff2f2f)
	{
		if(primi.Verts.size()==LINES*2) primi.Verts.erase(primi.Verts.begin(),primi.Verts.begin()+2);
		Primi::FVFVertex vertex1, vertex2;
		vertex1.pos = from;
		vertex2.pos = to;
		vertex1.diffuse = colfrom;
		vertex2.diffuse = colto;
		primi.Verts.push_back(vertex1);
		primi.Verts.push_back(vertex2);
	}
	static void Draw()
	{
		if(primi.Verts.empty()) return;
#ifdef _HOME_VERSION
		DirtyLinks::GetGraphPipe()->Chop("vertexwhite",&primi,"Line of sight");
#else
		DirtyLinks::GetGraphPipe()->Chop("vertexwhite",&primi);
#endif
	}
};
Primi LOSdraw::primi;
LOSdraw::Init LOSdraw::primi_init;
//point3 LOSdraw::Lines[LOSdraw::LINES*2];
//unsigned LOSdraw::col[LOSdraw::LINES*2];
//int LOSdraw::num=0;

void DrawIt()
{
	LOSdraw::Draw();
}


//=====================================================================================//
//                              bool LOSTracer::CalcLOS()                              //
//=====================================================================================//
bool LOSTracer::CalcLOS()
{
	//#define _DRAW_LOF_

	SightClear=true;
	point3 Norm,dot;
	std::string Name;

	//ray r(Source,Destination-Source);
	const point3 dir = Destination-Source;
	point3 right = 0.5f*Normalize(dir.Cross(AXISZ));
	point3 up = 1.0f*Normalize(dir.Cross(right));
	GameLevel *g = DirtyLinks::GetGameLevel();


	bool LOSforEnt=did?true:false;
	bool LOSforLev=!LOSforEnt;
	bool LOSforObj=m_ObjName.size()?true:false;

	int counter=1;
	const int NumTests=1;

#ifdef _DRAW_LOF_
	LOSdraw::AddLine(Destination,Destination+0.2*AXISZ);LOSdraw::AddLine(Destination,Destination-0.2*AXISZ);
	LOSdraw::AddLine(Destination,Destination+0.2*AXISX);LOSdraw::AddLine(Destination,Destination-0.2*AXISX);
	LOSdraw::AddLine(Destination,Destination+0.2*AXISY);LOSdraw::AddLine(Destination,Destination-0.2*AXISY);
#endif

	float RayLen = dir.Length();
	for(int i=0;i<NumTests;i++)
	{
		point3 direct = Destination-Source;
		float factor = 0.99f;//(direct.Length() > 1.0f ? (1.0f - 0.5f/direct.Length()) : 0.5f);
		//ray Ray(Source,Destination-Source+right*(frand() - 0.5f)+ up*(frand() - 0.5f));	//fixme: необходимо задать отклонение
		ray Ray(Source,direct*factor);	//fixme: необходимо задать отклонение
		bool LevelHit=false,ObjHit=false;

		GameLevel::TraceResult tr;
		LevelHit = g && g->TraceSegment3(Ray,m_skip,&tr);

		if(!LevelHit && g && g->GetLevelObjects()->TraceSegment2(Ray.Origin,Ray.Direction,&dot,&Norm,&Name,m_skip))
		{
			if((dot-Ray.Origin).Length()-0.5<RayLen&&m_ObjName!=Name) ObjHit=true;
		}

#ifdef _DRAW_LOF_
		if(LevelHit)     LOSdraw::AddLine(Source,Ray.Origin+Ray.Direction*Dist,0x20ff20,0xff2020);
		else if(ObjHit)  LOSdraw::AddLine(Source,dot,0x2020ff,0xff2020);
		else             LOSdraw::AddLine(Source,Ray.Origin+Ray.Direction,0x7f7f7f,0xffffff);
#endif      
		if(LevelHit||ObjHit) counter--;
		else 
		{
			//fixme:
			counter=1;
			break;
			//^^^^^^^^^^^^
			counter++;
		}
	}
	SightClear=counter>0?true:false;

	return SightClear;
}



/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: �������� ��������
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                
#include "precomp.h"

#include "../GraphPipe/GraphPipe.h"
#include "EffectManager.h"
#include "SmokeEffect.h"
#include "FireEffect.h"
#include "ExplosionEffect.h"
#include "BombEffect.h"
#include "SparkleEffect.h"
#include "DirectExplosion.h"
#include "LineEffect.h"
#include "SpotEffect.h"
#include "SpangleEffect.h"
#include "TailEffect.h"
#include "LensFlareEffect.h"
#include "HaloEffect.h"
#include "SelectionEffect.h"
#include "DLightEffect.h"
#include "ShootSmokeEffect.h"
#include "GrenadeTracer.h"
#include "ShieldEffect.h"
#include "ModelDestructEffect.h"
#include "MeatEffect.h"
#include <undercover/iworld.h>
#include <logic2/TraceUtils.h>
#include "ModelManager.h"

#include <undercover/GameLevel/GameLevel.h>
#include <undercover/GameLevel/DynObjectSet.h>


//---------- ��� ���� ------------
//#define _DEBUG_EFFECTS
#ifdef _DEBUG_EFFECTS
CLog effects_log;
#define log	effects_log["3deffects.log"]
#else
#define log	/##/
#endif

namespace
{
	point3 getBBoxCenter(const BBox* LocalBBox, const D3DMATRIX* Matrix)
	{
		point3 p;
		PointMatrixMultiply(*(reinterpret_cast<D3DVECTOR *>(&p)),
			*(reinterpret_cast<D3DVECTOR *>(&LocalBBox->GetCenter())),
			*Matrix);
		return p;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
//
// ����� ��������� ��������
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// �������� ��� �������
void EffectManager::clear()
{
	next_id = 1;
	Effects_Map_Iterator i;
	if(!effects.empty())
	{
		for(i = effects.begin(); i != effects.end(); i++)
		{
			if( (*i).second ) delete (*i).second;
		}
	}
	effects.clear();
	m_marks.clear();
}

// ����������� �� ���������
EffectManager::EffectManager(GraphPipe *pipe)
:	next_id(1),
	pGraphPipe(pipe)
{
	SetEffectsQuality(1.0);
}

// ����������
EffectManager::~EffectManager()
{
	Effects_Map_Iterator i;
	if(!effects.empty())
	{
		for(i = effects.begin(); i != effects.end(); i++)
		{
			if( (*i).second ) delete (*i).second;
		}
	}
	effects.clear();
}

// ��������� ��� ������� ��� ���� ��������
void EffectManager::NextTick(float time)
{
	//DebugInfo::Add(100, 200, "number of effects = %d", effects.size());
	m_destruction_manager.NextTick(time);
	m_model_manager.NextTick(time);
	// ������� ��������������� NextTick ��� ���� ��������
	cur_time = time;
	Effects_Map_Iterator i = effects.begin();
	Effects_Map_Iterator end = effects.end();
	while(i != end)
	{
		if(!(*i).second)
		{
			i++;
			continue;
		}
		(*i).second->NextTick(time);
		i++;
	}

	//  ������� �� �������, ������� ��������� ���� ������
	i = effects.begin();
	while(i != end)
	{
		// ������ ��� ������������ ������
		if(  (!(*i).second)  ||  (*i).second->IsFinished()  )
		{
			Effects_Map_Iterator j = i;
			i++;
			delete (*j).second;
			effects.erase(j);
			continue;
		}
		i++;
	}
}
// ���������� �������� ������� � ����������� ������
// ��� ���� ��������
void EffectManager::Update()
{
	if(effects.empty()) return;
	BBox b;
	Effects_Map_Iterator i = effects.begin();
	Effects_Map_Iterator end = effects.end();
	while(i != end)
	{
		if(!(*i).second)
		{
			i++;
			continue;
		}
		// TEMP
		BaseEffect* be = (*i).second;
		TailCircleEffect* te = static_cast<TailCircleEffect*>(be);
		// end TEMP
		// �������� ������ �� ���������
		b = (*i).second->GetBBox(cur_time);
		Frustum * fru = &pGraphPipe->GetCam()->Cone;
		if( !(fru->TestBBox(b) == Frustum::NOTVISIBLE) )
			(*i).second->Update(); // ������ �����
		i++;
	}
}
// ��������� ���� ��������
void EffectManager::DrawEffects()
{
	m_destruction_manager.Draw(pGraphPipe);
	m_model_manager.Draw();
	if(effects.empty()) return;
	BBox b;
	Effects_Map_Iterator i, end = effects.end();
	for(i = effects.begin(); i != end; i++)
	{
		if(!(*i).second) continue;

		// �������� ������ �� ���������
		b = (*i).second->GetBBox(cur_time);
		// ��������� BBox'a
		//  pGraphPipe->DrawBBox(b, 0xffffffff);
		Frustum * fru = &pGraphPipe->GetCam()->Cone;
		if( fru->TestBBox(b)  == Frustum::NOTVISIBLE)
		{
			continue; // ��� ���� ���������
		}

		Primi &p = *(*i).second->GetReadyPrim();
		//p.Pos = (*i).second->GetReadyVector();
		//p.Diffuse = (*i).second->GetColorsVector();
		//p.UVs[0] = (*i).second->GetTexCoordsVector();
		//p.IdxNum = 0;
		if((*i).second->GetType() == BaseEffect::EFFECT_TYPE::_2D)
		{
			p.Prim = Primi::TRIANGLE;
			//p.Prim = Primi::LINESSTRIP;
			p.Contents = Primi::PrimContents::NEEDTRANSFORM; //0;
		}
		if((*i).second->GetType() == BaseEffect::EFFECT_TYPE::_3D)
		{
			p.Prim = Primi::TRIANGLEFAN;
			//p.Prim = Primi::LINESSTRIP;
			p.Contents = Primi::PrimContents::NEEDTRANSFORM;
		}
		if((*i).second->GetType() == BaseEffect::EFFECT_TYPE::_3D_TRIANGLE)
		{
			p.Prim = Primi::TRIANGLE;
			//p.Prim = Primi::TRIANGLEFAN;
			//p.Prim = Primi::LINESSTRIP;
			p.Contents = Primi::PrimContents::NEEDTRANSFORM;
		}
		if((*i).second->GetType() == BaseEffect::EFFECT_TYPE::_3D_LINESSTRIP)
		{
			p.Prim = Primi::LINESSTRIP;
			p.Contents = Primi::PrimContents::NEEDTRANSFORM;
		}
		if((*i).second->GetType() == BaseEffect::EFFECT_TYPE::_3D_POINTS)
		{
			p.Prim = Primi::POINTS;
			p.Contents = Primi::PrimContents::NEEDTRANSFORM;
		}
		//p.VertNum = (*i).second->GetReadyNum();
#ifdef _HOME_VERSION
		std::ostringstream sstr;
		sstr << "EffectWithShader[" << i->second->GetShader() << "]";
		pGraphPipe->Chop((*i).second->GetShader(),&p,i->second->GetReadyNum(),sstr.str().c_str());
#else
		pGraphPipe->Chop((*i).second->GetShader(),&p,i->second->GetReadyNum());
#endif
	}
}


// ���������� ������
void EffectManager::DestroyEffect(long id)
{
	if(!id)
	{
		log("EffectManager::DestroyEffect: Effect ID is zero!\n");
		return;
		//throw CASUS("EffectManager::DestroyEffect: Effect ID is zero!!!");
	}

	typedef std::vector<EffectsMap::iterator> Itors_t;
	Itors_t itors;

	EffectsMap::iterator i = effects.find(id);

	for(; i != effects.end() && i->second->m_masterEffect == id; ++i)
	{
		itors.push_back(i);
	}

	for(Itors_t::iterator j = itors.begin(); j != itors.end(); ++j)
	{
		delete (*j)->second;
		effects.erase(*j);
	}
}

// ��������� ��������� � ������ ������ �����
void EffectManager::NewTurn()
{
	if(effects.empty()) return;
	Effects_Map_Iterator i, end = effects.end();
	for(i = effects.begin(); i != end; i++)
	{
		(*i).second->NewTurn();
	}
}


// ������ ������� �������
unsigned int EffectManager::CreateAmbientEffect(const std::string& strSysName, const point3& Target, const point3& Color)
{
	// ������ ������ � ����� � ����������� �� ��������
	if(m_effect_info.m_effect_info_map.find(strSysName) == m_effect_info.m_effect_info_map.end())
	{
		log("EffectManager::CreateAmbientEffect: Effect \"%s\" is Not Found!\n", strSysName.c_str());
		return 0;
	}
	// ���������� �� ������� �������
	EffectInfo::EFFECT_INFO_LIST eil = m_effect_info.m_effect_info_map[strSysName];
	if(eil.empty())
	{
		log("EffectManager::CreateAmbientEffect: \"%s\" Params Error!\n", strSysName.c_str());
		return 0;
		//throw CASUS("EffectManager::CreateAmbientEffect: Params Error!");
	}
	unsigned int retn = next_id;

	EffectInfo::EFFECT_INFO_LIST::iterator i = eil.begin();
	while(i != eil.end())
	{
		PBaseEffect &cureffect = effects[next_id];

		switch( i->type )
		{
		case EffectInfo::ET::ET_BLINKING_HALO :
			{
				if(m_effect_info.blinking_halo_map.find((*i).id) == m_effect_info.blinking_halo_map.end())
				{
					log("EffectManager::CreateMovableEffect: \"%s\" ET_BLINKING_HALO Params Error!\n", strSysName.c_str());
					return 0;
				}

				BLINKING_HALO_EFFECT_INFO bhei = m_effect_info.blinking_halo_map[(*i).id];

				// �������� ������
				cureffect = new BlinkingHaloEffect(Target,point3(1.0f,1.0f,1.0f),bhei);
				break;
			}
		case EffectInfo::ET::ET_FIRE :
			{
				if(m_effect_info.fire_effect_map.find((*i).id) == m_effect_info.fire_effect_map.end())
				{
					log("EffectManager::CreateAmbientEffect: \"%s\" Params Error!\n", strSysName.c_str());
					return 0;
					//throw CASUS("EffectManager::CreateAmbientEffect: Params Error!");
				}
				FIRE_EFFECT_INFO fei = m_effect_info.fire_effect_map[(*i).id];

				// �������� ������
				cureffect = new FireEffect(quality,Target,fei);
				break;
			}
		case EffectInfo::ET::ET_SMOKE :
			{
				if(m_effect_info.smoke_effect_map.find((*i).id) == m_effect_info.smoke_effect_map.end())
				{
					log("EffectManager::CreateAmbientEffect: \"%s\" Params Error!\n", strSysName.c_str());
					return 0;
					//throw CASUS("EffectManager::CreateAmbientEffect: Params Error!");
				}
				SMOKE_EFFECT_INFO sei = m_effect_info.smoke_effect_map[(*i).id];

				// �������� ������
				cureffect = new SmokeEffect(quality,Target,sei);
				break;
			}
		case EffectInfo::ET::ET_HSMOKE :
			{
				if(m_effect_info.hsmoke_effect_map.find((*i).id) == m_effect_info.hsmoke_effect_map.end())
				{
					log("EffectManager::CreateAmbientEffect: \"%s\" Params Error!\n", strSysName.c_str());
					return 0;
					//throw CASUS("EffectManager::CreateAmbientEffect: Params Error!");
				}
				HSMOKE_EFFECT_INFO sei = m_effect_info.hsmoke_effect_map[(*i).id];

				// �������� ������
				cureffect = new HeavySmokeEffect(
					quality,           // ��������
					Target,            // �������� �����
					sei.BirthRange,    // ������� ������ ��������� �����
					sei.Vel,           // ��������� ��������
					sei.Accel,         // ���������
					sei.SizeBegin,     // ��������� ������
					sei.SizeRnd,       // ��������� ����� ���������� �������
					sei.SizeRange,     // ������� �������
					sei.LifeTime,      // ������������ ����� �����
					sei.MaxPartNum,    // ������������ ���������� ������
					sei.Shader,        // ��� �������
					sei.Sound,         // ��� ��������� �������
					sei.SndMov         // ��������� �� ����
					);
				break;
			}
		case EffectInfo::ET::ET_HALO :
			{
				if(m_effect_info.halo_effect_map.find((*i).id) == m_effect_info.halo_effect_map.end())
				{
					log("EffectManager::CreateAmbientEffect: \"%s\" Params Error!\n", strSysName.c_str());
					return 0;
					//throw CASUS("EffectManager::CreateAmbientEffect: Params Error!");
				}

				HALO_EFFECT_INFO hei = m_effect_info.halo_effect_map[(*i).id];

				// �������� ������
				cureffect = new HaloEffect(Target,Color,hei);
				break;
			}
		case EffectInfo::ET::ET_SIMPLE_HALO :
			{
				if(m_effect_info.simple_halo_effect_map.find((*i).id) == m_effect_info.simple_halo_effect_map.end())
				{
					log("EffectManager::CreateAmbientEffect: \"%s\" Params Error!\n", strSysName.c_str());
					return 0;
					//throw CASUS("EffectManager::CreateAmbientEffect: Params Error!");
				}

				SIMPLE_HALO_EFFECT_INFO shei = m_effect_info.simple_halo_effect_map[(*i).id];

				// �������� ������
				cureffect = new SimpleHaloEffect(Target,Color,shei);
				break;
			}
		case EffectInfo::ET::ET_FLY :
			{
				if(m_effect_info.fly_effect_map.find((*i).id) == m_effect_info.fly_effect_map.end())
				{
					log("EffectManager::CreateAmbientEffect: \"%s\" Params Error!\n", strSysName.c_str());
					return 0;
					//throw CASUS("EffectManager::CreateAmbientEffect: FlyEffect Params Error!");
				}
				FLY_EFFECT_INFO fei = m_effect_info.fly_effect_map[(*i).id];

				// �������� ������
				cureffect = new FlyEffect(quality,Target,fei);
				break;
			}
		default : 
			{
				log("EffectManager::CreateAmbientEffect: \"%s\" Params Error!\n", strSysName.c_str());
				return 0;
				//throw CASUS("EffectManager::CreateAmbientEffect: Params Error!");
			}
		}
		cureffect->Init(pGraphPipe);
		cureffect->m_masterEffect = retn;
		next_id++;
		i++;
	}
	return retn;// (next_id - 1);
}

// ������� ��� �������� �� ������
void EffectManager::CreateFlashEffect(const std::string& strSysName, const point3& Source, const point3& Target)
{
	// ������ ������ � ����� � ����������� �� ��������
	if(m_effect_info.m_effect_info_map.find(strSysName) == m_effect_info.m_effect_info_map.end())
	{
		log("EffectManager::CreateFlashEffect: Effect \"%s\" is Not Found!\n", strSysName.c_str());
		//throw CASUS("EffectManager::CreateFlashEffect: Effect SysName is Not Found!");
		return;
	}
	// ���������� �� ������� �������
	EffectInfo::EFFECT_INFO_LIST eil = m_effect_info.m_effect_info_map[strSysName];
	if(eil.empty())
	{
		log("EffectManager::CreateFlashEffect: \"%s\" Effect Info List Is Empty!\n", strSysName.c_str());
		//throw CASUS("EffectManager::CreateFlashEffect: Effect Info List Is Empty!");
		return;
	}
	EffectInfo::EFFECT_INFO_LIST::iterator i = eil.begin();
	while(i != eil.end())
	{
		switch( (*i).type )
		{
		case EffectInfo::ET::ET_ONE_UPSIZING_SPOT :
			{
				if(m_effect_info.one_upsizing_spot_map.find((*i).id) == m_effect_info.one_upsizing_spot_map.end())
				{
					log("EffectManager::CreateFlashEffect: \"%s\" ET_ONE_UPSIZING_SPOT Params Error!\n", strSysName.c_str());
					//throw CASUS("EffectManager::CreateFlashEffect: ET_ONE_UPSIZING_SPOT Params Error!");
					return;
				}
				UPSIZING_SPOT_INFO usi = m_effect_info.one_upsizing_spot_map[(*i).id];

				// �������� ������
				effects[next_id] = new OneUpsizingSpot(
					Source,			   // ��������� �����
					Target,            // �������� �����
					usi
					);

				effects[next_id]->Init(pGraphPipe);
				next_id++;
				break;
			}
		case EffectInfo::ET::ET_TWO_UPSIZING_SPOT :
			{
				if(m_effect_info.two_upsizing_spot_map.find((*i).id) == m_effect_info.two_upsizing_spot_map.end())
				{
					log("EffectManager::CreateFlashEffect: \"%s\" ET_TWO_UPSIZING_SPOT Params Error!\n", strSysName.c_str());
					//throw CASUS("EffectManager::CreateFlashEffect: ET_ONE_UPSIZING_SPOT Params Error!");
					return;
				}
				UPSIZING_SPOT_INFO usi = m_effect_info.two_upsizing_spot_map[(*i).id];

				// �������� ������
				effects[next_id] = new TwoUpsizingSpot(
					Source,			   // ��������� �����
					Target,            // �������� �����
					usi
					);

				effects[next_id]->Init(pGraphPipe);
				next_id++;
				break;
			}
		case EffectInfo::ET::ET_DIRECT_SPARKLES :
			{
				if(m_effect_info.direct_sparkles_map.find((*i).id) == m_effect_info.direct_sparkles_map.end())
				{
					log("EffectManager::CreateFlashEffect: \"%s\" ET_DIRECT_SPARKLES Params Error!\n", strSysName.c_str());
					//throw CASUS("EffectManager::CreateFlashEffect: ET_ONE_UPSIZING_SPOT Params Error!");
					return;
				}
				DIRECT_SPARKLES_INFO dsi = m_effect_info.direct_sparkles_map[(*i).id];

				// �������� ������
				effects[next_id] = new DirectSparkles(
					Source,                       // �������� �����
					Normalize(Target - Source),   // ����������� ��������
					dsi
					);

				effects[next_id]->Init(pGraphPipe);
				next_id++;
				break;
			}
		case EffectInfo::ET::ET_WAVE_SPOT :
			{
				if(m_effect_info.wave_spot_map.find((*i).id) == m_effect_info.wave_spot_map.end())
				{
					log("EffectManager::CreateFlashEffect: \"%s\" ET_WAVE_SPOT Params Error!\n", strSysName.c_str());
					//throw CASUS("EffectManager::CreateFlashEffect: ET_ONE_UPSIZING_SPOT Params Error!");
					return;
				}
				WAVE_SPOT_INFO wsi = m_effect_info.wave_spot_map[(*i).id];

				// �������� ������
				effects[next_id] = new WaveSpot(
					Source,                       // �������� �����
					wsi
					);

				effects[next_id]->Init(pGraphPipe);
				next_id++;
				break;
			}
		case EffectInfo::ET::ET_SHOOT_SMOKE :
			{
				if(m_effect_info.shoot_smoke_effect_map.find((*i).id) == m_effect_info.shoot_smoke_effect_map.end())
				{
					log("EffectManager::CreateFlashEffect: \"%s\" ET_SHOOT_SMOKE Params Error!\n", strSysName.c_str());
					//throw CASUS("EffectManager::CreateFlashEffect: ET_ONE_UPSIZING_SPOT Params Error!");
					return;
				}
				SHOOT_SMOKE_EFFECT_INFO ssei = m_effect_info.shoot_smoke_effect_map[(*i).id];

				// �������� ������
				effects[next_id] = new ShootSmokeEffect(
					Source,			   // ��������� �����
					Target,            // �������� �����
					ssei
					);

				effects[next_id]->Init(pGraphPipe);
				next_id++;
				break;
			}
		case EffectInfo::ET::ET_TAIL_CIRCLE :
			{
				if(m_effect_info.tail_circle_effect_map.find((*i).id) == m_effect_info.tail_circle_effect_map.end())
				{
					log("EffectManager::CreateFlashEffect: \"%s\" ET_TAIL_CIRCLE Params Error!\n", strSysName.c_str());
					//throw CASUS("EffectManager::CreateFlashEffect: ET_ONE_UPSIZING_SPOT Params Error!");
					return;
				}
				TAIL_EFFECT_INFO tei = m_effect_info.tail_circle_effect_map[(*i).id];

				// �������� ������
				effects[next_id] = new TailCircleEffect(
					Source,              // ��������� �����
					Source + (Normalize(Target - Source)),              // �������� �����
					tei
					);
				effects[next_id]->Init(pGraphPipe);
				next_id++;

				// �������� ������
				effects[next_id] = new TailCircleEffect(
					Source + (Normalize(Target - Source)),              // ��������� �����
					Source,              // �������� �����
					tei
					);
				effects[next_id]->Init(pGraphPipe);
				next_id++;
				break;
			}
		case EffectInfo::ET::ET_MARK :
			{
				if(m_effect_info.mark_effect_map.find((*i).id) == m_effect_info.mark_effect_map.end())
				{
					log("EffectManager::CreateFlashEffect: \"%s\" ET_MARK Params Error!\n", strSysName.c_str());
					//throw CASUS("EffectManager::CreateFlashEffect: ET_ONE_UPSIZING_SPOT Params Error!");
					return;
				}
				MARK_EFFECT_INFO mei = m_effect_info.mark_effect_map[(*i).id];

				// �������� ������
				if(mei.Type == "constmark")
				{
					IWorld::Get()->GetLevel()->DoMark(Source, mei.Radius,
						GetRandomWord(mei.Shaders));
					MarkInfo mi;
					mi.m_info = mei;
					mi.m_birthtime = Timer::GetSeconds();
					mi.m_target = Source;
					m_marks.push_back(mi);
					break;
				}

				if(mei.Type == "splash")
				{
					// �������� ������� �����
					boost::shared_ptr<Splash> spl(new Splash(Source, mei.Radius,
						GetRandomWord(mei.Shaders),
						mei.StartColor, Timer::GetSeconds(), mei.LifeTime));
					
					IWorld::Get()->GetLevel()->AddMark(spl);
					break;
				}

				if(mei.Type == "dmark")
				{
					// �������� ������� �����
					boost::shared_ptr<DMark> dmk(new DMark(Source, mei.Radius,
						GetRandomWord(mei.Shaders),
						mei.StartColor, Timer::GetSeconds(), mei.LifeTime,
						mei.SwitchTime, 0xFFFFFFFF));
					
					IWorld::Get()->GetLevel()->AddMark(dmk);
					MarkInfo mi;
					mi.m_info = mei;
					mi.m_birthtime = Timer::GetSeconds();
					mi.m_target = Source;
					m_marks.push_back(mi);
					break;
				}

				log("EffectManager::CreateFlashEffect: \"%s\" ET_MARK illegal type!\n", strSysName.c_str());
				return;
				//throw CASUS("EffectManager::CreateFlashEffect: ET_MARK illegal type!");
			}
		case EffectInfo::ET::ET_WAVE_SPOT2 :
			{
				if(m_effect_info.wave_spot2_map.find((*i).id) == m_effect_info.wave_spot2_map.end())
				{
					log("EffectManager::CreateFlashEffect: \"%s\" ET_WAVE_SPOT2 Params Error!\n", strSysName.c_str());
					return;
				}
				WAVE_SPOT_INFO wsi = m_effect_info.wave_spot2_map[(*i).id];

				// �������� ������
				effects[next_id] = new WaveSpot2(
					Source,                       
					Target,
					wsi
					);

				effects[next_id]->Init(pGraphPipe);
				next_id++;
				break;
			}
		default : 
			{
				log("EffectManager::CreateFlashEffect: \"%s\" Unknown Effect In Effect Info List!\n", strSysName.c_str());
				return;
				//throw CASUS("EffectManager::CreateFlashEffect: Unknown Effect In Effect Info List!");
			}
		}
		i++;
	}
}

// ��������
float EffectManager::CreateTracerEffect(const std::string& strSysName, const point3& Source, const point3& Target)
{
	float t = 0;
	// ������ ������ � ����� � ����������� �� ��������
	if(m_effect_info.m_effect_info_map.find(strSysName) == m_effect_info.m_effect_info_map.end())
	{
		log("EffectManager::CreateTracerEffect: Effect \"%s\" is Not Found!\n", strSysName.c_str());
		return 0;
	}
	// ���������� �� ������� �������
	EffectInfo::EFFECT_INFO_LIST eil = m_effect_info.m_effect_info_map[strSysName];
	if(eil.empty())
	{
		log("EffectManager::CreateTracerEffect: \"%s\" Effects Info List Is Empty!\n", strSysName.c_str());
		return 0;
	}
	EffectInfo::EFFECT_INFO_LIST::iterator i = eil.begin();
	while(i != eil.end())
	{
		switch( (*i).type )
		{
		case EffectInfo::ET::ET_LINE :
			{
				if(m_effect_info.m_line_effect_map.find((*i).id) == m_effect_info.m_line_effect_map.end())
				{
					log("EffectManager::CreateTracerEffect: \"%s\" ET_LINE Params Error!\n", strSysName.c_str());
					return 0;
					//throw CASUS("EffectManager::CreateTracerEffect: ET_LINE Params Error!");
				}
				LINE_EFFECT_INFO lei = m_effect_info.m_line_effect_map[(*i).id];

				// �������� ������
				effects[next_id] = new LineEffect(
					Source,              // ��������� �����
					Target,              // �������� �����
					lei
					);
				effects[next_id]->Init(pGraphPipe);
				next_id++;

				t = Timer::GetSeconds() + (Target - Source).Length() / lei.VelFactor;
				break;
			}
		case EffectInfo::ET::ET_SPANGLE :
			{
				if(m_effect_info.m_spangle_effect_map.find((*i).id) == m_effect_info.m_spangle_effect_map.end())
				{
					log("EffectManager::CreateTracerEffect: \"%s\" ET_SPANGLE Params Error!\n", strSysName.c_str());
					return 0;
					//throw CASUS("EffectManager::CreateTracerEffect: ET_LINE Params Error!");
				}
				SPANGLE_EFFECT_INFO sei = m_effect_info.m_spangle_effect_map[(*i).id];

				// �������� ������
				effects[next_id] = new SpangleEffect(
					quality,           // ��������
					Source,            // ��������� �����
					Target,            // �������� �����
					sei					
					);
				effects[next_id]->Init(pGraphPipe);
				next_id++;

				t = Timer::GetSeconds() + (Target - Source).Length() / sei.VelFactor;
				break;
			}
		case EffectInfo::ET::ET_LINE_BALL :
			{
				if(m_effect_info.line_ball_effect_map.find((*i).id) == m_effect_info.line_ball_effect_map.end())
				{
					log("EffectManager::CreateTracerEffect: \"%s\" ET_LINE_BALL Params Error!\n", strSysName.c_str());
					return 0;
					//throw CASUS("EffectManager::CreateTracerEffect: ET_LINE Params Error!");
				}
				LINE_BALL_EFFECT_INFO lbei = m_effect_info.line_ball_effect_map[(*i).id];

				// �������� ������
				effects[next_id] = new LineBallEffect(
					quality,
					Source,              // ��������� �����
					Target,              // �������� �����
					lbei
					);
				effects[next_id]->Init(pGraphPipe);
				next_id++;

				t = Timer::GetSeconds() + (Target - Source).Length() / lbei.VelFactor;
				break;
			}
		case EffectInfo::ET::ET_ROTATE_SPOT :
			{
				if(m_effect_info.m_rotate_spot_effect_map.find((*i).id) == m_effect_info.m_rotate_spot_effect_map.end())
				{
					log("EffectManager::CreateTracerEffect: \"%s\" ET_LINE_BALL Params Error!\n", strSysName.c_str());
					return 0;
					//throw CASUS("EffectManager::CreateTracerEffect: ET_LINE Params Error!");
				}
				ROTATE_SPOT_EFFECT_INFO rsei = m_effect_info.m_rotate_spot_effect_map[(*i).id];

				// �������� ������
				effects[next_id] = new RotateSpotEffect(
					Source,              // ��������� �����
					Target,              // �������� �����
					rsei
					);
				effects[next_id]->Init(pGraphPipe);
				next_id++;

				t = Timer::GetSeconds() + (Target - Source).Length() / rsei.VelFactor;
				break;
			}
		case EffectInfo::ET::ET_TAIL :
			{
				if(m_effect_info.m_tail_effect_map.find((*i).id) == m_effect_info.m_tail_effect_map.end())
				{
					log("EffectManager::CreateTracerEffect: \"%s\" ET_TAIL Params Error!\n", strSysName.c_str());
					return 0;
				}
				TAIL_EFFECT_INFO tei = m_effect_info.m_tail_effect_map[(*i).id];

				// �������� ������
				effects[next_id] = new TailEffect(
					Source,              // ��������� �����
					Target,              // �������� �����
					tei
					);
				effects[next_id]->Init(pGraphPipe);

				next_id++;
				t = Timer::GetSeconds() + (Target - Source).Length() / tei.VelFactor;
				break;
			}
		case EffectInfo::ET::ET_TAIL_PARABOLA :
			{
				if(m_effect_info.tail_parabola_effect_map.find((*i).id) == m_effect_info.tail_parabola_effect_map.end())
				{
					log("EffectManager::CreateTracerEffect: \"%s\" ET_TAIL_PARABOLA Params Error!\n", strSysName.c_str());
					return 0;
					//throw CASUS("EffectManager::CreateTracerEffect: ET_LINE Params Error!");
				}
				TAIL_PARABOLA_EFFECT_INFO tpei = m_effect_info.tail_parabola_effect_map[(*i).id];
				TAIL_EFFECT_INFO tei;
				tei.Frequency = tpei.Frequency;
				tei.LifeTime = tpei.LifeTime;
				tei.Shader = tpei.Shader;
				tei.Size = tpei.Size;
				tei.SizeEnd = tpei.SizeEnd;
				tei.SndMov = tpei.SndMov;
				tei.Sound = tpei.Sound;
				tei.VelFactor = tpei.VelFactor;

				// �������� ������
				effects[next_id] = new TailParabolaEffect(
					Source,              // ��������� �����
					Target,              // �������� �����
					tei,
					tpei.Gravitation     // ����������
					);
				effects[next_id]->Init(pGraphPipe);

				next_id++;
				t = Timer::GetSeconds() + (Target - Source).Length() / tpei.VelFactor;
				break;
			}
		case EffectInfo::ET::ET_TAIL_CIRCLE :
			{
				if(m_effect_info.tail_circle_effect_map.find((*i).id) == m_effect_info.tail_circle_effect_map.end())
				{
					log("EffectManager::CreateTracerEffect: \"%s\" ET_TAIL_CIRCLE Params Error!\n", strSysName.c_str());
					return 0;
					//throw CASUS("EffectManager::CreateTracerEffect: ET_LINE Params Error!");
				}
				TAIL_EFFECT_INFO tei = m_effect_info.tail_circle_effect_map[(*i).id];

				// �������� ������
				effects[next_id] = new TailCircleEffect(
					Source,              // ��������� �����
					Target,              // �������� �����
					tei
					);
				effects[next_id]->Init(pGraphPipe);

				next_id++;
				t = Timer::GetSeconds() + (Target - Source).Length() / tei.VelFactor;
				break;
			}
		case EffectInfo::ET::ET_SHINE_SPOT :
			{
				if(m_effect_info.shine_spot_effect_map.find((*i).id) == m_effect_info.shine_spot_effect_map.end())
				{
					log("EffectManager::CreateTracerEffect: \"%s\" ET_SHINE_SPOT Params Error!\n", strSysName.c_str());
					return 0;
					//throw CASUS("EffectManager::CreateTracerEffect: ET_LINE Params Error!");
				}
				SPOT_EFFECT_INFO sei = m_effect_info.shine_spot_effect_map[(*i).id];

				// �������� ������
				effects[next_id] = new ShineSpotEffect(
					Source,              // ��������� �����
					Target,              // �������� �����
					sei
					);
				effects[next_id]->Init(pGraphPipe);
				next_id++;

				t = Timer::GetSeconds() + (Target - Source).Length() / sei.VelFactor;
				break;
			}
		case EffectInfo::ET::ET_FLARE :
			{
				if(m_effect_info.lens_flare_effect_map.find((*i).id) == m_effect_info.lens_flare_effect_map.end())
				{
					log("EffectManager::CreateTracerEffect: \"%s\" ET_FLARE Params Error!\n", strSysName.c_str());
					return 0;
					//throw CASUS("EffectManager::CreateTracerEffect: ET_LINE Params Error!");
				}
				LENS_FLARE_EFFECT_INFO lfei = m_effect_info.lens_flare_effect_map[(*i).id];

				// �������� ������
				effects[next_id] = new LensEffect(
					Source,              // ��������� �����
					Target,              // �������� �����
					lfei
					);
				effects[next_id]->Init(pGraphPipe);
				next_id++;

				t = Timer::GetSeconds() + (Target - Source).Length() / lfei.VelFactor;
				break;
			}
		case EffectInfo::ET::ET_LINE_DLIGHT :
			{
				if(m_effect_info.line_dlight_effect_map.find((*i).id) == m_effect_info.line_dlight_effect_map.end())
				{
					log("EffectManager::CreateTracerEffect: \"%s\" ET_LINE_DLIGHT Params Error!\n", strSysName.c_str());
					return 0;
					//throw CASUS("EffectManager::CreateTracerEffect: ET_LINE Params Error!");
				}
				LINE_DLIGHT_EFFECT_INFO ldei = m_effect_info.line_dlight_effect_map[(*i).id];

				// �������� ������
				effects[next_id] = new LineDLightEffect(
					Source,              // ��������� �����
					Target,              // �������� �����
					ldei
					);
				effects[next_id]->Init(pGraphPipe);
				next_id++;

				t = Timer::GetSeconds() + (Target - Source).Length() / ldei.VelFactor;
				break;
			}
		case EffectInfo::ET::ET_PLASMA_BEAM :
			{
				if(m_effect_info.plasma_beam_map.find((*i).id) == m_effect_info.plasma_beam_map.end())
				{
					log("EffectManager::CreateTracerEffect: \"%s\" ET_PLASMA_BEAM Params Error!\n", strSysName.c_str());
					return 0;
					//throw CASUS("EffectManager::CreateTracerEffect: ET_LINE Params Error!");
				}
				PLASMA_BEAM_INFO pbi = m_effect_info.plasma_beam_map[(*i).id];

				// �������� ������
				effects[next_id] = new PlasmaBeamEffect(
					Source,              // ��������� �����
					Target,              // �������� �����
					pbi
					);
				effects[next_id]->Init(pGraphPipe);
				next_id++;

				t = 0;
				break;
			}
		case EffectInfo::ET::ET_LIGHTNING :
			{
				if(m_effect_info.lightning_map.find((*i).id) == m_effect_info.lightning_map.end())
				{
					log("EffectManager::CreateTracerEffect: \"%s\" ET_LIGHTNING Params Error!\n", strSysName.c_str());
					return 0;
					//throw CASUS("EffectManager::CreateTracerEffect: ET_LINE Params Error!");
				}
				LIGHTNING_INFO li = m_effect_info.lightning_map[(*i).id];

				// �������� ������
				effects[next_id] = new LightningEffect(
					Source,              // ��������� �����
					Target,              // �������� �����
					li
					);
				effects[next_id]->Init(pGraphPipe);
				next_id++;

				t = 0;
				break;
			}
		case EffectInfo::ET::ET_TRACER_LINE_MODEL :
			{
				if(m_effect_info.tracer_line_model_map.find((*i).id) == m_effect_info.tracer_line_model_map.end())
				{
					log("EffectManager::CreateTracerEffect: \"%s\" ET_TRACER_LINE_MODEL Params Error!\n", strSysName.c_str());
					return 0;
					//throw CASUS("EffectManager::CreateTracerEffect: ET_LINE Params Error!");
				}
				TRACER_LINE_MODEL_INFO tlmi = m_effect_info.tracer_line_model_map[(*i).id];

				// �������� ������
				BaseEffect* be = new TracerLineModel(
					Source,              // ��������� �����
					Target,              // �������� �����
					tlmi
					);
				be->Init(pGraphPipe);
				m_model_manager.AddEffect(be);
				t = Timer::GetSeconds() + (Target - Source).Length() / tlmi.VelFactor;
				break;
			}
		case EffectInfo::ET::ET_TRACER_PARABOLA_MODEL :
			{
				if(m_effect_info.tracer_parabola_model_map.find((*i).id) == m_effect_info.tracer_parabola_model_map.end())
				{
					log("EffectManager::CreateTracerEffect: \"%s\" ET_TRACER_PARABOLA_MODEL Params Error!\n", strSysName.c_str());
					return 0;
					//throw CASUS("EffectManager::CreateTracerEffect: ET_LINE Params Error!");
				}
				TRACER_PARABOLA_MODEL_INFO tpmi = m_effect_info.tracer_parabola_model_map[(*i).id];

				// �������� ������
				BaseEffect* be = new TracerParabolaModel(
					Source,              // ��������� �����
					Target,              // �������� �����
					tpmi
					);
				be->Init(pGraphPipe);
				m_model_manager.AddEffect(be);
				t = Timer::GetSeconds() + (Target - Source).Length() / tpmi.VelFactor;
				break;
			}
		default : 
				{
					log("EffectManager::CreateTracerEffect: \"%s\" Params Error!\n", strSysName.c_str());
					return 0;
					//throw CASUS("EffectManager::CreateTracerEffect: ET_LINE Params Error!");
				}
		}
		i++;
	}
	return t;
}

// ������ ���������
void EffectManager::CreateHitEffect(const std::string& strSysName, const point3& Source, const point3& Target, const point3& Ground, const float Growth)
{
	// ������ ������ � ����� � ����������� �� ��������
	if(m_effect_info.m_effect_info_map.find(strSysName) == m_effect_info.m_effect_info_map.end())
	{
		log("EffectManager::CreateHitEffect: Effect \"%s\" is Not Found!\n", strSysName.c_str());
		return;
		//throw CASUS("EffectManager::CreateHitEffect: Effect SysName is Not Found!");
	}
	// ���������� �� ������� �������
	EffectInfo::EFFECT_INFO_LIST eil = m_effect_info.m_effect_info_map[strSysName];
	if(eil.empty())
	{
		log("EffectManager::CreateHitEffect: \"%s\" Params List Is Empty!\n", strSysName.c_str());
		return;
		//throw CASUS("EffectManager::CreateHitEffect: Params List Is Empty!");
	}
	EffectInfo::EFFECT_INFO_LIST::iterator i = eil.begin();
	while(i != eil.end())
	{
		switch( (*i).type )
		{
		case EffectInfo::ET::ET_SPARKLE :
			{
				if(m_effect_info.m_sparkle_effect_map.find((*i).id) == m_effect_info.m_sparkle_effect_map.end())
				{
					log("EffectManager::CreateHitEffect: \"%s\" ET_SPARKLE Params Error!\n", strSysName.c_str());
					return;
				}
				SPARKLE_EFFECT_INFO sei = m_effect_info.m_sparkle_effect_map[(*i).id];

				// �������� ������
			
				effects[next_id] = new SparkleEffect(
					Target,            // �������� �����
					sei
					);

				effects[next_id]->Init(pGraphPipe);
				next_id++;
				break;
			}
		case EffectInfo::ET::ET_EXPLOSION_SMOKE :
			{
				if(m_effect_info.m_explosion_smoke_map.find((*i).id) == m_effect_info.m_explosion_smoke_map.end())
				{
					log("EffectManager::CreateHitEffect: \"%s\" ET_EXPLOSION_SMOKE Params Error!\n", strSysName.c_str());
					return;
				}
				EXPLOSION_SMOKE_INFO esi = m_effect_info.m_explosion_smoke_map[(*i).id];

				// �������� ������
			
				effects[next_id] = new ExplosionSmokeEffect(
					quality,           // ��������
					Target + point3(0, 0, 0.8),            // �������� �����
					esi
					);

				effects[next_id]->Init(pGraphPipe);
				next_id++;
				break;
			}
		case EffectInfo::ET::ET_SMOKE_RING :
			{
				if(m_effect_info.m_smoke_ring_map.find((*i).id) == m_effect_info.m_smoke_ring_map.end())
				{
					log("EffectManager::CreateHitEffect: \"%s\" ET_SMOKE_RING Params Error!\n", strSysName.c_str());
					return;
				}
				SMOKE_RING_INFO sri = m_effect_info.m_smoke_ring_map[(*i).id];

				// �������� ������
			
				effects[next_id] = new ExplosionSmokeRing(
					Target,            // �������� �����
					sri
					);

				effects[next_id]->Init(pGraphPipe);
				next_id++;
				break;
			}
		case EffectInfo::ET::ET_GRENADE :
			{
				if(m_effect_info.m_grenade_effect_map.find((*i).id) == m_effect_info.m_grenade_effect_map.end())
				{
					log("EffectManager::CreateHitEffect: \"%s\" ET_GRENADE Params Error!\n", strSysName.c_str());
					return;
				}
				GRENADE_EFFECT_INFO gei = m_effect_info.m_grenade_effect_map[(*i).id];

				// �������� ������
				effects[next_id] = new GrenadeEffect(
					quality,           // ��������
					Target + point3(0, 0, 0.2),            // �������� �����
					gei
					);

				effects[next_id]->Init(pGraphPipe);
				next_id++;
				break;
			}
		case EffectInfo::ET::ET_BOMBSPARKLES :
			{
				if(m_effect_info.m_bomb_sparkles_map.find((*i).id) == m_effect_info.m_bomb_sparkles_map.end())
				{
					log("EffectManager::CreateHitEffect: \"%s\" ET_BOMBSPARKLES Params Error!\n", strSysName.c_str());
					return;
				}
				BOMB_SPARKLES_INFO bsi = m_effect_info.m_bomb_sparkles_map[(*i).id];

				// �������� ������
				effects[next_id] = new BombSparkles(
					Target,            // �������� �����
					bsi
					);

				effects[next_id]->Init(pGraphPipe);
				next_id++;
				break;
			}
		case EffectInfo::ET::ET_LINE_BALL :
			{
				if(m_effect_info.line_ball_effect_map.find((*i).id) == m_effect_info.line_ball_effect_map.end())
				{
					log("EffectManager::CreateHitEffect: \"%s\" ET_LINE_BALL Params Error!\n", strSysName.c_str());
					return;
				}
				LINE_BALL_EFFECT_INFO lbei = m_effect_info.line_ball_effect_map[(*i).id];

				// �������� ������
				effects[next_id] = new LineBallEffect(
					quality,
					Ground,              // ��������� �����
					Ground + point3(0, 0, Growth),              // �������� �����
					lbei
					);
				effects[next_id]->Init(pGraphPipe);
				next_id++;

				// � ��� ����
				effects[next_id] = new LineBallEffect(
					quality,
					Ground + point3(0, 0, Growth),              // ��������� �����
					Ground,              // �������� �����
					lbei
					);
				effects[next_id]->Init(pGraphPipe);
				next_id++;

				break;
			}
		case EffectInfo::ET::ET_DIRECT_SPARKLES :
			{
				if(m_effect_info.direct_sparkles_map.find((*i).id) == m_effect_info.direct_sparkles_map.end())
				{
					log("EffectManager::CreateHitEffect: \"%s\" ET_DIRECT_SPARKLES Params Error!\n", strSysName.c_str());
					return;
				}
				DIRECT_SPARKLES_INFO dsi = m_effect_info.direct_sparkles_map[(*i).id];

				// �������� ������
				effects[next_id] = new DirectSparkles(
					Target,                       // �������� �����
					Normalize(Source - Target),   // ����������� ��������
					dsi
					);

				effects[next_id]->Init(pGraphPipe);
				next_id++;
				break;
			}
		case EffectInfo::ET::ET_FLASH :
			{
				if(m_effect_info.flash_effect_map.find((*i).id) == m_effect_info.flash_effect_map.end())
				{
					log("EffectManager::CreateHitEffect: \"%s\" ET_FLASH Params Error!\n", strSysName.c_str());
					return;
				}
				FLASH_EFFECT_INFO fei = m_effect_info.flash_effect_map[(*i).id];

				// �������� ������
				effects[next_id] = new FlashEffect(
					Target,                       // �������� �����
					fei
					);

				effects[next_id]->Init(pGraphPipe);
				next_id++;
				break;
			}
		case EffectInfo::ET::ET_WAVE_SPOT :
			{
				if(m_effect_info.wave_spot_map.find((*i).id) == m_effect_info.wave_spot_map.end())
				{
					log("EffectManager::CreateHitEffect: \"%s\" ET_WAVE_SPOT Params Error!\n", strSysName.c_str());
					return;
				}
				WAVE_SPOT_INFO wsi = m_effect_info.wave_spot_map[(*i).id];

				// �������� ������
				effects[next_id] = new WaveSpot(
					Target,                       // �������� �����
					wsi
					);

				effects[next_id]->Init(pGraphPipe);
				next_id++;
				break;
			}
		case EffectInfo::ET::ET_SHOOT_SMOKE :
			{
				if(m_effect_info.shoot_smoke_effect_map.find((*i).id) == m_effect_info.shoot_smoke_effect_map.end())
				{
					log("EffectManager::CreateHitEffect: \"%s\" ET_SHOOT_SMOKE Params Error!\n", strSysName.c_str());
					return;
				}
				SHOOT_SMOKE_EFFECT_INFO ssei = m_effect_info.shoot_smoke_effect_map[(*i).id];

				// �������� ������
				effects[next_id] = new ShootSmokeEffect(
					Target,			   // ��������� �����
					Source,            // �������� �����
					ssei
					);

				effects[next_id]->Init(pGraphPipe);
				next_id++;
				break;
			}
		case EffectInfo::ET::ET_MARK :
			{
				if(m_effect_info.mark_effect_map.find((*i).id) == m_effect_info.mark_effect_map.end())
				{
					log("EffectManager::CreateHitEffect: \"%s\" ET_MARK Params Error!\n", strSysName.c_str());
					return;
				}
				MARK_EFFECT_INFO mei = m_effect_info.mark_effect_map[(*i).id];

				// �������� ������
				if(mei.Type == "constmark")
				{
					IWorld::Get()->GetLevel()->DoMark(Target, mei.Radius,
						GetRandomWord(mei.Shaders));
					MarkInfo mi;
					mi.m_info = mei;
					mi.m_birthtime = Timer::GetSeconds();
					mi.m_target = Target;
					m_marks.push_back(mi);
					break;
				}

				if(mei.Type == "splash")
				{
					// �������� ������� �����
					boost::shared_ptr<Splash> spl(new Splash(Target, mei.Radius,
						GetRandomWord(mei.Shaders),
						mei.StartColor, Timer::GetSeconds(), mei.LifeTime));
					
					IWorld::Get()->GetLevel()->AddMark(spl);
					break;
				}

				if(mei.Type == "dmark")
				{
					// �������� ������� �����
					boost::shared_ptr<DMark> dmk(new DMark(Target, mei.Radius,
						GetRandomWord(mei.Shaders),
						mei.StartColor, Timer::GetSeconds(), mei.LifeTime,
						mei.SwitchTime, 0xFFFFFFFF));
					
					IWorld::Get()->GetLevel()->AddMark(dmk);
					MarkInfo mi;
					mi.m_info = mei;
					mi.m_birthtime = Timer::GetSeconds();
					mi.m_target = Target;
					m_marks.push_back(mi);
					break;
				}
				if(mei.Type == "blood")
				{
					// �������� ������� �����
					boost::shared_ptr<DMark> dmk(new DMark(Ground + point3(0, 0, 0.1), mei.Radius,
						GetRandomWord(mei.Shaders),
						mei.StartColor, Timer::GetSeconds(), mei.LifeTime,
						mei.SwitchTime, 0xFFFFFFFF));
					
					IWorld::Get()->GetLevel()->AddMark(dmk);
					MarkInfo mi;
					mi.m_info = mei;
					mi.m_birthtime = Timer::GetSeconds();
					mi.m_target = Target;
					m_marks.push_back(mi);
					break;
				}

				log("EffectManager::CreateHitEffect: \"%s\" ET_MARK illegal type!\n", strSysName.c_str());
				return;
			}
		case EffectInfo::ET::ET_MULTI_FLASH :
			{
				if(m_effect_info.multi_flash_effect_map.find((*i).id) == m_effect_info.multi_flash_effect_map.end())
				{
					log("EffectManager::CreateHitEffect: \"%s\" ET_MULTI_FLASH Params Error!\n", strSysName.c_str());
					return;
				}
				FLASH_EFFECT_INFO fei = m_effect_info.multi_flash_effect_map[(*i).id];

				// �������� ������
				effects[next_id] = new MultiTexFlashEffect(
					Target,                       // �������� �����
					fei
					);

				effects[next_id]->Init(pGraphPipe);
				next_id++;
				break;
			}
		case EffectInfo::ET::ET_FIRE :
			{
				if(m_effect_info.fire_effect_map.find((*i).id) == m_effect_info.fire_effect_map.end())
				{
					log("EffectManager::CreateHitEffect: \"%s\" ET_FIRE Params Error!\n", strSysName.c_str());
					return;
				}
				FIRE_EFFECT_INFO fei = m_effect_info.fire_effect_map[(*i).id];

				// �������� ������
				effects[next_id] = new FireEffect(
					quality,           // ��������
					Target,			   // ��������� �����
					fei
					);

				effects[next_id]->Init(pGraphPipe);
				next_id++;
				break;
			}
		case EffectInfo::ET::ET_SMOKE :
			{
				if(m_effect_info.smoke_effect_map.find((*i).id) == m_effect_info.smoke_effect_map.end())
				{
					log("EffectManager::CreateHitEffect: \"%s\" ET_SMOKE Params Error!\n", strSysName.c_str());
					return;
				}
				SMOKE_EFFECT_INFO sei = m_effect_info.smoke_effect_map[(*i).id];

				// �������� ������
				effects[next_id] = new SmokeEffect(
					quality,           // ��������
					Target,            // �������� �����
					sei
					);

				effects[next_id]->Init(pGraphPipe);
				next_id++;

				break;
			}
		case EffectInfo::ET::ET_ROTATE_SPOT :
			{
				if(m_effect_info.m_rotate_spot_effect_map.find((*i).id) == m_effect_info.m_rotate_spot_effect_map.end())
				{
					log("EffectManager::CreateHitEffect: \"%s\" ET_ROTATE_SPOT Params Error!\n", strSysName.c_str());
					return;
				}
				ROTATE_SPOT_EFFECT_INFO rsei = m_effect_info.m_rotate_spot_effect_map[(*i).id];

				// �������� ������
				point3 pnt = Ground;
				pnt.z += Growth;
				effects[next_id] = new RotateSpotEffect(
					Ground,              // ��������� �����
					pnt,                 // �������� �����
					rsei
					);
				effects[next_id]->Init(pGraphPipe);
				next_id++;
				break;
			}
		case EffectInfo::ET::ET_ROTATE_SPOT2 :
			{
				if(m_effect_info.rotate_spot_effect2_map.find((*i).id) == m_effect_info.rotate_spot_effect2_map.end())
				{
					log("EffectManager::CreateHitEffect: \"%s\" ET_ROTATE_SPOT2 Params Error!\n", strSysName.c_str());
					return;
				}
				ROTATE_SPOT_EFFECT2_INFO rsei = m_effect_info.rotate_spot_effect2_map[(*i).id];

				// �������� ������
				effects[next_id] = new RotateSpotEffect2(
					Ground + point3(0, 0, Growth),              // ����������� �����
					rsei
					);
				effects[next_id]->Init(pGraphPipe);
				next_id++;
				break;
			}
		case EffectInfo::ET::ET_DIRECT_EXPLOSION :
			{
				if(m_effect_info.direct_explosion_map.find((*i).id) == m_effect_info.direct_explosion_map.end())
				{
					log("EffectManager::CreateHitEffect: \"%s\" ET_DIRECT_EXPLOSION Params Error!\n", strSysName.c_str());
					return;
				}
				DIRECT_EXPLOSION_INFO dei = m_effect_info.direct_explosion_map[(*i).id];

				// �������� ������-������� � ������ �������
				ShotTracer tr(0, Source,
					Normalize(Target - Source),
					(Target - Source).Length() + 0.1,
					ShotTracer::F_SKIP_SHIELDS|ShotTracer::F_SKIP_INVISIBLE_ENTS|ShotTracer::F_SIGHT);
				// �������� ������
				effects[next_id] = new DirectExplosion(
					Target,              // ����� ���������
					tr.GetEndNorm(),    // ����������� ������
					dei
					);
				effects[next_id]->Init(pGraphPipe);
				next_id++;
				break;
			}
		case EffectInfo::ET::ET_SHIELD :
			{
				if(m_effect_info.shield_map.find((*i).id) == m_effect_info.shield_map.end())
				{
					log("EffectManager::CreateHitEffect: \"%s\" ET_SHIELD Params Error!\n", strSysName.c_str());
					return;
				}
				SHIELD_INFO si = m_effect_info.shield_map[(*i).id];

				// �������� ������
				effects[next_id] = new ShieldEffect(
					Source,
					Target,              // ����� ���������
					si
					);
				effects[next_id]->Init(pGraphPipe);
				next_id++;
				break;
			}
		case EffectInfo::ET::ET_TAIL_CIRCLE :
			{
				if(m_effect_info.tail_circle_effect_map.find((*i).id) == m_effect_info.tail_circle_effect_map.end())
				{
					log("EffectManager::CreateHitEffect: \"%s\" ET_TAIL_CIRCLE Params Error!\n", strSysName.c_str());
					return;
				}
				TAIL_EFFECT_INFO tei = m_effect_info.tail_circle_effect_map[(*i).id];

				// �������� ������
				effects[next_id] = new TailCircleEffect(
					Target + point3(0,0,0.5),              // ��������� �����
					Target - point3(0,0,0.5),              // �������� �����
					tei
					);
				effects[next_id]->Init(pGraphPipe);
				next_id++;

				// �������� ������
				effects[next_id] = new TailCircleEffect(
					Target - point3(0,0,0.5),              // ��������� �����
					Target + point3(0,0,0.5),              // �������� �����
					tei
					);
				effects[next_id]->Init(pGraphPipe);
				next_id++;
				break;
			}
		default : 
				{
					log("EffectManager::CreateHitEffect: \"%s\" Params Error!\n", strSysName.c_str());
					return;
				}
		}
		i++;
	}
}

// ������� ������ ���������� ������
void EffectManager::DestroyGlass(const std::string& SysName, const DynObject& DynObj, const point3& ExpPos)
{
	// ������ ������ � ����� � ����������� �� ��������
	if(m_effect_info.m_effect_info_map.find(SysName) == m_effect_info.m_effect_info_map.end())
	{
		log("EffectManager::DestroyGlass: Effect \"%s\" is Not Found!\n", SysName.c_str());
		return;
	}
	// ���������� �� ������� �������
	EffectInfo::EFFECT_INFO_LIST eil = m_effect_info.m_effect_info_map[SysName];
	if(eil.empty())
	{
		log("EffectManager::DestroyGlass: \"%s\" Params List Is Empty!\n", SysName.c_str());
		return;
	}
	EffectInfo::EFFECT_INFO_LIST::iterator i = eil.begin();
	while(i != eil.end())
	{
		switch( (*i).type )
		{
		case EffectInfo::ET::ET_GLASS_DESTRUCT :
			{
				if(m_effect_info.glass_destruct_effect_map.find((*i).id) == m_effect_info.glass_destruct_effect_map.end())
				{
					log("EffectManager::DestroyGlass: ET_GLASS_DESTRUCT Params Error!\n");
					return;
				}
				GLASS_DESTRUCT_EFFECT_INFO gdei = m_effect_info.glass_destruct_effect_map[(*i).id];

				// �������� ������
				m_destruction_manager.DestroyGlass(DynObj, ExpPos, gdei);			

				break;
			}
		default : 
				  {
					  log("EffectManager::DestroyGlass: Params Error!\n");
					  return;
				  }
		}
		i++;
	}
}

// ������� ������ ����������� ������� �������
void EffectManager::DestroyObjectByExplosion(const point3& ExpPos, const DynObjectSet &dset)
{
	for(DynObjectSet::ConstIterator_t it = dset.Begin(); it != dset.End(); ++it)
	{
		// ��������� �� ��� �������
		DestructibleInfo::PrimaryEffect pe = (*it)->GetDestructibleInfo().m_primaryEffect;
		DestructibleInfo::SecondaryEffect se = (*it)->GetDestructibleInfo().m_secondaryEffect;
		DestructibleInfo::TertiaryEffect te = (*it)->GetDestructibleInfo().m_tertiaryEffect;

		HDynObject obj = *it;

		if(pe == DestructibleInfo::peGlass)
		{
			// ������
			EffectInfo::EFFECT_INFO_LIST eil = m_effect_info.m_effect_info_map["wood_destruct"];
			if(eil.empty())
			{
				log("EffectManager::DestroyObjectByExplosion: Params List Is Empty!\n");
				return;
			}
			EffectInfo::EFFECT_INFO_LIST::iterator i = eil.begin();
			for(;i != eil.end();i++)
			{
				switch( i->type )
				{
					case EffectInfo::ET::ET_GLASS_DESTRUCT :
					{
						if(m_effect_info.glass_destruct_effect_map.find(i->id) == m_effect_info.glass_destruct_effect_map.end())
						{
							log("EffectManager::DestroyObjectByExplosion: ET_GLASS_DESTRUCT Params Error!\n");
							return;
						}

						GLASS_DESTRUCT_EFFECT_INFO gdei = m_effect_info.glass_destruct_effect_map[(*i).id];
						
						// �������� ������
						m_destruction_manager.DestroyGlass(*obj, ExpPos, gdei);
						
						break;
					}
					default: 
					{
						log("EffectManager::DestroyObjectByExplosion: Params Error!\n");
						return;
					}
				}
			}
		}
		else if(pe == DestructibleInfo::peMetal) // �� ������
		{
			// ������
			DestroyObjectByModel(ExpPos, obj.get(), OT_METAL, OET_BLACKSMOKE);
		}
		else if(pe == DestructibleInfo::peWood) // �� ������, � �� ������
		{
			// ������
			DestroyObjectByModel(ExpPos, obj.get(), OT_WOOD, OET_BLACKSMOKE);
		}
		else if(pe == DestructibleInfo::peStone) // �� ������, � �� ������, � �� ������
		{
			// ������
			DestroyObjectByModel(ExpPos, obj.get(), OT_STONE, OET_BLACKSMOKE);
		}

		// ���� ����� �������� ������ ������
		if(te == DestructibleInfo::teBlast)
		{
			point3 center = getBBoxCenter(&obj->GetLocalBound(),
				&obj->GetWorldMatrix());
			CreateHitEffect("hit_medium_g_explosion",
				ExpPos,
				center,
				center,
				1.0f);
		}
	}
}

// ������ ���������
unsigned int EffectManager::CreateSelectionEffect(const std::string& strSysName, const point3& Target)
{
	// ������ ������ � ����� � ����������� �� ��������
	if(m_effect_info.m_effect_info_map.find(strSysName) == m_effect_info.m_effect_info_map.end())
	{
		log("EffectManager::CreateSelectionEffect: Effect \"%s\" is Not Found!\n", strSysName.c_str());
		return 0;
	}
	// ���������� �� ������� �������
	EffectInfo::EFFECT_INFO_LIST eil = m_effect_info.m_effect_info_map[strSysName];
	if(eil.empty())
	{
		log("EffectManager::CreateSelectionEffect: \"%s\" Effect Info List Is Empty!\n", strSysName.c_str());
		return 0;
	}
	unsigned int retn = next_id;

	EffectInfo::EFFECT_INFO_LIST::iterator i = eil.begin();
	while(i != eil.end())
	{
		switch( i->type )
		{
		case EffectInfo::ET::ET_SELECTION :
			{
				if(m_effect_info.selection_effect_map.find((*i).id) == m_effect_info.selection_effect_map.end())
				{
					log("EffectManager::CreateSelectionEffect: \"%s\" ET_SELECTION Params Error!\n", strSysName.c_str());
					return 0;
				}
				SELECTION_EFFECT_INFO sei = m_effect_info.selection_effect_map[(*i).id];

				// �������� ������
				effects[next_id] = new SelectionEffect(
					Target,  // ����������� �����
					sei
					);

				break;
			}
		default : 
				  {
					  log("EffectManager::CreateSelectionEffect: \"%s\" Unknown Effect In Effect Info List!", strSysName.c_str());
					  return 0;
				  }
		}

		effects[next_id]->Init(pGraphPipe);
		effects[next_id]->m_masterEffect = retn;
		next_id++;
		i++;
	}

	return retn; //(next_id - 1);
}

// ������, ������� ����� ����������� � �������� �����
unsigned int EffectManager::CreateMovableEffect(const std::string& strSysName, const point3& Target, float Radius/* = 0.0f*/)
{
	// ������ ������ � ����� � ����������� �� ��������
	if(m_effect_info.m_effect_info_map.find(strSysName) == m_effect_info.m_effect_info_map.end())
	{
		log("EffectManager::CreateMovableEffect: Effect \"%s\" is Not Found!\n", strSysName.c_str());
		return 0;
	}
	// ���������� �� ������� �������
	EffectInfo::EFFECT_INFO_LIST eil = m_effect_info.m_effect_info_map[strSysName];
	if(eil.empty())
	{
		log("EffectManager::CreateMovableEffect: \"%s\" Effect Info List Is Empty!\n", strSysName.c_str());
		return 0;
	}
	unsigned int retn = next_id;

	EffectInfo::EFFECT_INFO_LIST::iterator i = eil.begin();
	while(i != eil.end())
	{
		EffectInfo::ET et = i->type;
		PBaseEffect &cureffect = effects[next_id];

		switch( et )
		{
		case EffectInfo::ET::ET_SELECTION :
			{
				if(m_effect_info.selection_effect_map.find(i->id) == m_effect_info.selection_effect_map.end())
				{
					log("EffectManager::CreateMovableEffect: \"%s\" ET_SELECTION Params Error!\n", strSysName.c_str());
					return 0;
				}
				SELECTION_EFFECT_INFO sei = m_effect_info.selection_effect_map[i->id];

				// ��������� ��������: ������ ������� �� �� ����� � �� 
				// ���������� ���������� �������
				if(Radius != 0.0f)
				{
					sei.Radius = Radius;
				}

				// �������� ������
				cureffect = new SelectionEffect(Target, sei);
				break;
			}
		case EffectInfo::ET::ET_FIRE :
			{
				if(m_effect_info.fire_effect_map.find((*i).id) == m_effect_info.fire_effect_map.end())
				{
					log("EffectManager::CreateMovableEffect: \"%s\" ET_FIRE Params Error!\n", strSysName.c_str());
					return 0;
				}

				FIRE_EFFECT_INFO fei = m_effect_info.fire_effect_map[(*i).id];

				// �������� ������
				cureffect = new FireEffect(quality,Target,fei);
				break;
			}
		case EffectInfo::ET::ET_ROTATE_SPOT2 :
			{
				if(m_effect_info.rotate_spot_effect2_map.find((*i).id) == m_effect_info.rotate_spot_effect2_map.end())
				{
					log("EffectManager::CreateMovableEffect: \"%s\" ET_ROTATE_SPOT2 Params Error!\n", strSysName.c_str());
					return 0;
				}

				ROTATE_SPOT_EFFECT2_INFO rsei = m_effect_info.rotate_spot_effect2_map[(*i).id];

				// �������� ������
				cureffect = new RotateSpotEffect2(Target,rsei);
				break;
			}
		case EffectInfo::ET::ET_BLINKING_HALO :
			{
				if(m_effect_info.blinking_halo_map.find((*i).id) == m_effect_info.blinking_halo_map.end())
				{
					log("EffectManager::CreateMovableEffect: \"%s\" ET_BLINKING_HALO Params Error!\n", strSysName.c_str());
					return 0;
				}

				BLINKING_HALO_EFFECT_INFO bhei = m_effect_info.blinking_halo_map[(*i).id];

				// �������� ������
				cureffect = new BlinkingHaloEffect(Target,bhei.Color,bhei);
				break;
			}
		case EffectInfo::ET::ET_FLY_CYLINDER :
			{
				if(m_effect_info.m_fly_cylinder_map.find((*i).id) == m_effect_info.m_fly_cylinder_map.end())
				{
					log("EffectManager::CreateMovableEffect: \"%s\" ET_FLY_CYLINDER Params Error!\n", strSysName.c_str());
					return 0;
				}

				FLY_CYLINDER_EFFECT_INFO ei = m_effect_info.m_fly_cylinder_map[(*i).id];

				// �������� ������
				cureffect = new FlyCylinderEffect(quality,Target,ei);
				break;
			}
		default : 
				{
					log("EffectManager::CreateMovableEffect: \"%s\" Unknown Effect In Effect Info List!\n", strSysName.c_str());
					return 0;
				}
		}

		cureffect->Init(pGraphPipe);
		cureffect->m_masterEffect = retn;
		next_id++;

		i++;
	}

	return retn;//(next_id - 1);
}

// �������� ������� �������
void EffectManager::SetPosition(unsigned int id, const point3& Pos)
{
	if(!id)
	{
		log("EffectManager::SetPosition: effect id == 0!\n");
		return;
	}
	if(effects.empty()) return;
	Effects_Map_Iterator i = effects.find(id);
	if(i == effects.end()) return;

	for(; i != effects.end() && i->second->m_masterEffect == id; ++i)
	{
		i->second->SetPosition(Pos);
	}
}

// �������� ������������� �������
void EffectManager::SetActivity(unsigned int id, const float Activity)
{
	if(!id)
	{
		log("EffectManager::SetActivity: effect id == 0!\n");
		return;
	}
	if(effects.empty()) return;
	Effects_Map_Iterator i = effects.find(id);
	if(i == effects.end()) return;

	for(; i != effects.end() && i->second->m_masterEffect == id; ++i)
	{
		i->second->SetActivity(Activity);
	}
}

// ������ �������� � ���������� ������
void EffectManager::FinishEffect(unsigned int id)
{
	if(!id)
	{
		log("EffectManager::SetActivity: effect id == 0!\n");
		return;
	}
	if(effects.empty()) return;
	Effects_Map_Iterator i = effects.find(id);
	if(i == effects.end()) return;

	for(; i != effects.end() && i->second->m_masterEffect == id; ++i)
	{
		i->second->Finish();
	}
}

// �������� ��������� ����� �� ������� ����
std::string& EffectManager::GetRandomWord(std::vector<std::string>& vec)
{
	int n = vec.size();
	int i = (float)rand()/32768.0 * n;
	return vec[i];
}

// ������� ������ ����������� ������� ����� ������
void EffectManager::DestroyObjectByModel(const point3& ExpPos, const DynObject *obj, ObjType obj_type, ObjEffectType effect_type)
{
	std::string names[5];
	if(obj_type == OT_WOOD)
	{
		names[0] = "Animations/skins/items/wood_1.skin";
		names[1] = "Animations/skins/items/wood_2.skin";
		names[2] = "Animations/skins/items/wood_3.skin";
		names[3] = "Animations/skins/items/wood_4.skin";
		names[4] = "Animations/skins/items/wood_5.skin";
	}
	if(obj_type == OT_METAL)
	{
		names[0] = "Animations/skins/items/metal_1.skin";
		names[1] = "Animations/skins/items/metal_2.skin";
		names[2] = "Animations/skins/items/metal_3.skin";
		names[3] = "Animations/skins/items/metal_4.skin";
		names[4] = "Animations/skins/items/metal_5.skin";
	}
	if(obj_type == OT_STONE)
	{
		names[0] = "Animations/skins/items/stone_1.skin";
		names[1] = "Animations/skins/items/stone_2.skin";
		names[2] = "Animations/skins/items/stone_3.skin";
		names[3] = "Animations/skins/items/stone_4.skin";
		names[4] = "Animations/skins/items/stone_5.skin";
	}

	const point3 pos = obj->GetBound().GetCenter();

	EffectInfo::EFFECT_INFO_LIST eil = m_effect_info.m_effect_info_map["model_destruct"];
	if(eil.empty())
	{
		log("EffectManager::DestroyObjectByModel: Params List Is Empty!\n");
		return;
	}

	EffectInfo::EFFECT_INFO_LIST::iterator i = eil.begin();
	for(;i != eil.end();i++)
	{
		switch( i->type )
		{
			case EffectInfo::ET::ET_MODEL_DESTRUCT:
			{
				if(m_effect_info.model_destruct_map.find(i->id) == m_effect_info.model_destruct_map.end())
				{
					log("EffectManager::DestroyObjectByModel: ET_GLASS_DESTRUCT Params Error!\n");
					return;
				}
				MODEL_DESTRUCT_INFO mdi = m_effect_info.model_destruct_map[i->id];
				
				// �������� ������
				BaseEffect* be = new ModelDestructEffect(
					names,
					5,
					&obj->GetLocalBound(),
					&obj->GetWorldMatrix(),
					ExpPos,
					mdi
					);
				be->Init(pGraphPipe);
				m_model_manager.AddEffect(be);
				next_id++;
				break;
			}
			case EffectInfo::ET::ET_SPARKLE :
				{
					if(m_effect_info.m_sparkle_effect_map.find((*i).id) == m_effect_info.m_sparkle_effect_map.end())
					{
						log("EffectManager::CreateHitEffect: \"%s\" ET_SPARKLE Params Error!\n", strSysName.c_str());
						return;
					}
					SPARKLE_EFFECT_INFO sei = m_effect_info.m_sparkle_effect_map[(*i).id];

					// �������� ������
				
					effects[next_id] = new SparkleEffect(
						pos,            // �������� �����
						sei
						);

					effects[next_id]->Init(pGraphPipe);
					next_id++;
					break;
				}
			case EffectInfo::ET::ET_EXPLOSION_SMOKE :
				{
					if(m_effect_info.m_explosion_smoke_map.find((*i).id) == m_effect_info.m_explosion_smoke_map.end())
					{
						log("EffectManager::CreateHitEffect: \"%s\" ET_EXPLOSION_SMOKE Params Error!\n", strSysName.c_str());
						return;
					}
					EXPLOSION_SMOKE_INFO esi = m_effect_info.m_explosion_smoke_map[(*i).id];

					// �������� ������
				
					effects[next_id] = new ExplosionSmokeEffect(
						quality,           // ��������
						pos + point3(0, 0, 0.8),            // �������� �����
						esi
						);

					effects[next_id]->Init(pGraphPipe);
					next_id++;
					break;
				}
			case EffectInfo::ET::ET_SMOKE_RING :
				{
					if(m_effect_info.m_smoke_ring_map.find((*i).id) == m_effect_info.m_smoke_ring_map.end())
					{
						log("EffectManager::CreateHitEffect: \"%s\" ET_SMOKE_RING Params Error!\n", strSysName.c_str());
						return;
					}
					SMOKE_RING_INFO sri = m_effect_info.m_smoke_ring_map[(*i).id];

					// �������� ������
				
					effects[next_id] = new ExplosionSmokeRing(
						pos,            // �������� �����
						sri
						);

					effects[next_id]->Init(pGraphPipe);
					next_id++;
					break;
				}
			case EffectInfo::ET::ET_GRENADE :
				{
					if(m_effect_info.m_grenade_effect_map.find((*i).id) == m_effect_info.m_grenade_effect_map.end())
					{
						log("EffectManager::CreateHitEffect: \"%s\" ET_GRENADE Params Error!\n", strSysName.c_str());
						return;
					}
					GRENADE_EFFECT_INFO gei = m_effect_info.m_grenade_effect_map[(*i).id];

					// �������� ������
					effects[next_id] = new GrenadeEffect(
						quality,           // ��������
						pos + point3(0, 0, 0.2),            // �������� �����
						gei
						);

					effects[next_id]->Init(pGraphPipe);
					next_id++;
					break;
				}
			case EffectInfo::ET::ET_BOMBSPARKLES :
				{
					if(m_effect_info.m_bomb_sparkles_map.find((*i).id) == m_effect_info.m_bomb_sparkles_map.end())
					{
						log("EffectManager::CreateHitEffect: \"%s\" ET_BOMBSPARKLES Params Error!\n", strSysName.c_str());
						return;
					}
					BOMB_SPARKLES_INFO bsi = m_effect_info.m_bomb_sparkles_map[(*i).id];

					// �������� ������
					effects[next_id] = new BombSparkles(
						pos,            // �������� �����
						bsi
						);

					effects[next_id]->Init(pGraphPipe);
					next_id++;
					break;
				}
			case EffectInfo::ET::ET_DIRECT_SPARKLES :
				{
					if(m_effect_info.direct_sparkles_map.find((*i).id) == m_effect_info.direct_sparkles_map.end())
					{
						log("EffectManager::CreateHitEffect: \"%s\" ET_DIRECT_SPARKLES Params Error!\n", strSysName.c_str());
						return;
					}
					DIRECT_SPARKLES_INFO dsi = m_effect_info.direct_sparkles_map[(*i).id];

					// �������� ������
					effects[next_id] = new DirectSparkles(
						pos,                       // �������� �����
						Normalize(pos - ExpPos),   // ����������� ��������
						dsi
						);

					effects[next_id]->Init(pGraphPipe);
					next_id++;
					break;
				}
			case EffectInfo::ET::ET_FLASH :
				{
					if(m_effect_info.flash_effect_map.find((*i).id) == m_effect_info.flash_effect_map.end())
					{
						log("EffectManager::CreateHitEffect: \"%s\" ET_FLASH Params Error!\n", strSysName.c_str());
						return;
					}
					FLASH_EFFECT_INFO fei = m_effect_info.flash_effect_map[(*i).id];

					// �������� ������
					effects[next_id] = new FlashEffect(
						pos,                       // �������� �����
						fei
						);

					effects[next_id]->Init(pGraphPipe);
					next_id++;
					break;
				}
			case EffectInfo::ET::ET_WAVE_SPOT :
				{
					if(m_effect_info.wave_spot_map.find((*i).id) == m_effect_info.wave_spot_map.end())
					{
						log("EffectManager::CreateHitEffect: \"%s\" ET_WAVE_SPOT Params Error!\n", strSysName.c_str());
						return;
					}
					WAVE_SPOT_INFO wsi = m_effect_info.wave_spot_map[(*i).id];

					// �������� ������
					effects[next_id] = new WaveSpot(
						pos,                       // �������� �����
						wsi
						);

					effects[next_id]->Init(pGraphPipe);
					next_id++;
					break;
				}
			case EffectInfo::ET::ET_MARK :
				{
					if(m_effect_info.mark_effect_map.find((*i).id) == m_effect_info.mark_effect_map.end())
					{
						log("EffectManager::CreateHitEffect: \"%s\" ET_MARK Params Error!\n", strSysName.c_str());
						return;
					}
					MARK_EFFECT_INFO mei = m_effect_info.mark_effect_map[(*i).id];

					// �������� ������
					if(mei.Type == "constmark")
					{
						IWorld::Get()->GetLevel()->DoMark(pos, mei.Radius,
							GetRandomWord(mei.Shaders));
						MarkInfo mi;
						mi.m_info = mei;
						mi.m_birthtime = Timer::GetSeconds();
						mi.m_target = pos;
						m_marks.push_back(mi);
						break;
					}

					if(mei.Type == "splash")
					{
						// �������� ������� �����
						boost::shared_ptr<Splash> spl(new Splash(pos, mei.Radius,
							GetRandomWord(mei.Shaders),
							mei.StartColor, Timer::GetSeconds(), mei.LifeTime));
						
						IWorld::Get()->GetLevel()->AddMark(spl);
						break;
					}

					if(mei.Type == "dmark")
					{
						// �������� ������� �����
						boost::shared_ptr<DMark> dmk(new DMark(pos, mei.Radius,
							GetRandomWord(mei.Shaders),
							mei.StartColor, Timer::GetSeconds(), mei.LifeTime,
							mei.SwitchTime, 0xFFFFFFFF));
						
						IWorld::Get()->GetLevel()->AddMark(dmk);
						MarkInfo mi;
						mi.m_info = mei;
						mi.m_birthtime = Timer::GetSeconds();
						mi.m_target = pos;
						m_marks.push_back(mi);
						break;
					}

					log("EffectManager::CreateHitEffect: \"%s\" ET_MARK illegal type!\n", strSysName.c_str());
					return;
				}
			case EffectInfo::ET::ET_MULTI_FLASH :
				{
					if(m_effect_info.multi_flash_effect_map.find((*i).id) == m_effect_info.multi_flash_effect_map.end())
					{
						log("EffectManager::CreateHitEffect: \"%s\" ET_MULTI_FLASH Params Error!\n", strSysName.c_str());
						return;
					}
					FLASH_EFFECT_INFO fei = m_effect_info.multi_flash_effect_map[(*i).id];

					// �������� ������
					effects[next_id] = new MultiTexFlashEffect(
						pos,                       // �������� �����
						fei
						);

					effects[next_id]->Init(pGraphPipe);
					next_id++;
					break;
				}
			case EffectInfo::ET::ET_FIRE :
				{
					if(m_effect_info.fire_effect_map.find((*i).id) == m_effect_info.fire_effect_map.end())
					{
						log("EffectManager::CreateHitEffect: \"%s\" ET_FIRE Params Error!\n", strSysName.c_str());
						return;
					}
					FIRE_EFFECT_INFO fei = m_effect_info.fire_effect_map[(*i).id];

					// �������� ������
					effects[next_id] = new FireEffect(
						quality,           // ��������
						pos,			   // ��������� �����
						fei
						);

					effects[next_id]->Init(pGraphPipe);
					next_id++;
					break;
				}
			case EffectInfo::ET::ET_SMOKE :
				{
					if(m_effect_info.smoke_effect_map.find((*i).id) == m_effect_info.smoke_effect_map.end())
					{
						log("EffectManager::CreateHitEffect: \"%s\" ET_SMOKE Params Error!\n", strSysName.c_str());
						return;
					}
					SMOKE_EFFECT_INFO sei = m_effect_info.smoke_effect_map[(*i).id];

					// �������� ������
					effects[next_id] = new SmokeEffect(
						quality,           // ��������
						pos,            // �������� �����
						sei
						);

					effects[next_id]->Init(pGraphPipe);
					next_id++;

					break;
				}
			case EffectInfo::ET::ET_DIRECT_EXPLOSION :
				{
					if(m_effect_info.direct_explosion_map.find((*i).id) == m_effect_info.direct_explosion_map.end())
					{
						log("EffectManager::CreateHitEffect: \"%s\" ET_DIRECT_EXPLOSION Params Error!\n", strSysName.c_str());
						return;
					}
					DIRECT_EXPLOSION_INFO dei = m_effect_info.direct_explosion_map[(*i).id];

					// �������� ������-������� � ������ �������
					ShotTracer tr(0, ExpPos,
						Normalize(pos - ExpPos),
						(pos - ExpPos).Length() + 0.1,
						ShotTracer::F_SKIP_SHIELDS|ShotTracer::F_SKIP_INVISIBLE_ENTS|ShotTracer::F_SIGHT);
					// �������� ������
					effects[next_id] = new DirectExplosion(
						pos,              // ����� ���������
						tr.GetEndNorm(),    // ����������� ������
						dei
						);
					effects[next_id]->Init(pGraphPipe);
					next_id++;
					break;
				}
			case EffectInfo::ET::ET_SHIELD :
				{
					if(m_effect_info.shield_map.find((*i).id) == m_effect_info.shield_map.end())
					{
						log("EffectManager::CreateHitEffect: \"%s\" ET_SHIELD Params Error!\n", strSysName.c_str());
						return;
					}
					SHIELD_INFO si = m_effect_info.shield_map[(*i).id];

					// �������� ������
					effects[next_id] = new ShieldEffect(
						ExpPos,
						pos,              // ����� ���������
						si
						);
					effects[next_id]->Init(pGraphPipe);
					next_id++;
					break;
				}
			case EffectInfo::ET::ET_TAIL_CIRCLE :
				{
					if(m_effect_info.tail_circle_effect_map.find((*i).id) == m_effect_info.tail_circle_effect_map.end())
					{
						log("EffectManager::CreateHitEffect: \"%s\" ET_TAIL_CIRCLE Params Error!\n", strSysName.c_str());
						return;
					}
					TAIL_EFFECT_INFO tei = m_effect_info.tail_circle_effect_map[(*i).id];

					// �������� ������
					effects[next_id] = new TailCircleEffect(
						pos + point3(0,0,0.5),              // ��������� �����
						pos - point3(0,0,0.5),              // �������� �����
						tei
						);
					effects[next_id]->Init(pGraphPipe);
					next_id++;

					// �������� ������
					effects[next_id] = new TailCircleEffect(
						pos - point3(0,0,0.5),              // ��������� �����
						pos + point3(0,0,0.5),              // �������� �����
						tei
						);
					effects[next_id]->Init(pGraphPipe);
					next_id++;
					break;
				}
			default: 
			{
				log("EffectManager::DestroyObjectByModel: Params Error!\n");
				return;
			}
		}
	}
}

// ������ ������ �������
void EffectManager::CreateGrenadeEffect(const KeyAnimation& ka, const std::string& skin)
{
	BaseEffect* be = new GrenadeTracer(ka, skin, 10.0);
	be->Init(pGraphPipe);
	m_model_manager.AddEffect(be);
}

// ������ ������ ������
void EffectManager::CreateShellEffect(unsigned int eid, const std::string& skin, const point3& Source, const point3& Target)
{
	BaseEffect* be = new ShellTracer(eid, Source, Target, skin, 10.0f);
	be->Init(pGraphPipe);
	m_model_manager.AddEffect(be);
}

// ������ ������ �������� ����
void EffectManager::CreateMeatEffect(unsigned int eid, const point3& Explosion, const point3& Human)
{
	// ������ ������ � ����� � ����������� �� ��������
	if(m_effect_info.m_effect_info_map.find("fresh_meat") == m_effect_info.m_effect_info_map.end())
	{
		log("EffectManager::CreateMeatEffect: Effect \"fresh_meat\" is Not Found!\n");
		return;
	}
	
	// ���������� �� ������� �������
	EffectInfo::EFFECT_INFO_LIST eil = m_effect_info.m_effect_info_map["fresh_meat"];
	if(eil.empty())
	{
		log("EffectManager::CreateMeatEffect: \"fresh_meat\" Effects Info List Is Empty!\n");
		return;
	}
	EffectInfo::EFFECT_INFO_LIST::iterator i = eil.begin();
	if((*i).type != EffectInfo::ET::ET_MEAT)
	{
		log("EffectManager::CreateMeatEffect: \"fresh_meat\" Bad effect type!\n");
		return;
	}
	if(m_effect_info.meat_effect_map.find((*i).id) == m_effect_info.meat_effect_map.end())
	{
		log("EffectManager::CreateMeatEffect: \"fresh_meat\" ET_MEAT Params Error!\n");
		return;
	}
	MEAT_EFFECT_INFO mei = m_effect_info.meat_effect_map[(*i).id];

	BaseEffect* be = new MeatEffect(eid, Explosion, Human, mei, 10.0f, 6.0f);
	be->Init(pGraphPipe);
	m_model_manager.AddEffect(be);
	
	CreateHitEffect("hit_large_blood", Explosion + (Human - Explosion)*2.0f, Human, point3(0,0,0), 1.0);
}

//=====================================================================================//
//                           void EffectManager::MakeSave()                            //
//=====================================================================================//
// ������ �������� (������� �� ������)
void EffectManager::MakeSave(SavSlot &slot)
{
	assert( slot.IsSaving() );

	// �����������

	// ����������� ��������
	updateMarks();
	slot << m_marks.size();

	Marks_t::iterator i = m_marks.begin();
	Marks_t::iterator end = m_marks.end();
	while(i != end)
	{
		slot << (*i).m_target;
		slot << (*i).m_info.Radius;
		slot << (*i).m_info.Type;
		slot << (*i).m_info.StartColor;
		slot << (*i).m_info.LifeTime;
		slot << (*i).m_info.SwitchTime;
		slot << (*i).m_info.Shaders.size();
		std::vector<std::string>::const_iterator I = (*i).m_info.Shaders.begin();
		std::vector<std::string>::const_iterator E = (*i).m_info.Shaders.end();
		while(I != E)
		{
			slot << (*I);
			++I;
		}
		++i;
	}
}

//=====================================================================================//
//                           void EffectManager::MakeLoad()                            //
//=====================================================================================//
// ������ �������� (������� �� ������)
void EffectManager::MakeLoad(SavSlot& slot, GameLevel *level)
{
	// ��������

	// ������ ��� ������ ��������
	m_marks.clear();
	int n;
	slot >> n;
	for(int i = 0; i < n; i++)
	{
		MarkInfo mi;
		slot >> mi.m_target;
		slot >> mi.m_info.Radius;
		slot >> mi.m_info.Type;
		slot >> mi.m_info.StartColor;
		slot >> mi.m_info.LifeTime;
		slot >> mi.m_info.SwitchTime;
		int N;
		slot >> N;
		for(int j = 0; j < N; j++)
		{
			std::string str;
			slot >> str;
			mi.m_info.Shaders.push_back(str);
		}

		m_marks.push_back(mi);
	}

	// ����������� ��� ��������
	restoreMarks(level);
}

// ������������ ��� ������� ������� �� ������
void EffectManager::updateMarks()
{
	if(m_marks.empty()) return;
	Marks_t::iterator i = m_marks.begin();
	Marks_t::iterator end = m_marks.end();
	float current_time = Timer::GetSeconds();
	while(i != end)
	{
		MARK_EFFECT_INFO mei = (*i).m_info;
		if( (mei.Type == "dmark") ||
			(mei.Type == "blood")
			)
		{
			// �������� ���� �� ��������
			if(current_time > ((*i).m_birthtime + mei.LifeTime) )
			{
				// ������� ���� ����� �� ������
				Marks_t::iterator j = i;
				++i;
				m_marks.erase(j);
				continue;
			}
			else
			{
				// ������� ����� ������������ ����� �����
				(*i).m_info.LifeTime = mei.LifeTime - (current_time - (*i).m_birthtime);
				(*i).m_birthtime = current_time;
			}
		}
		++i;
	}
}

//=====================================================================================//
//                         void EffectManager::restoreMarks()                          //
//=====================================================================================//
// ������������ ��� ������� ������� �� ������
void EffectManager::restoreMarks(GameLevel *level)
{
	if(m_marks.empty()) return;
	Marks_t::iterator i = m_marks.begin();
	Marks_t::iterator end = m_marks.end();
	float current_time = Timer::GetSeconds();
	for(; i != end; ++i)
	{
		MARK_EFFECT_INFO mei = (*i).m_info;
		if(mei.Type == "constmark")
		{
			level->DoMark((*i).m_target, mei.Radius, GetRandomWord(mei.Shaders));
			(*i).m_birthtime = current_time;
		}
		if( (mei.Type == "dmark") ||
			(mei.Type == "blood")
			)
		{
			// �������� ������� �����
			boost::shared_ptr<DMark> dmk(new DMark((*i).m_target, mei.Radius,
				GetRandomWord(mei.Shaders),
				mei.StartColor, current_time, mei.LifeTime,
				mei.SwitchTime, 0xFFFFFFFF));
			(*i).m_birthtime = current_time;
			level->AddMark(dmk);
		}
	}
}
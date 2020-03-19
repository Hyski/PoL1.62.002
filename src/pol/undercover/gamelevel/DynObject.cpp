#include "precomp.h"

#include "Grid.h"
#include "SndAuto.h"
#include "../IWorld.h"
#include "DynObject.h"
#include "GameLevel.h"
#include "DynObjectSet.h"
#include "LevelToLogic.h"
#include "DynObjectOldData.h"
#include <Common/GraphPipe/GraphPipe.h>

// ������ ��������� ������� ��� �������
//{"name", std::string}			// ������������� ������� (case insensitive)
//{"shadow", bool ("yes"|*)}		// ����������� �� ������ ���� (������������)
//{"use", std::string)			// ���� �� �������������
//{"usekey", std::string)			// ���� ��������� ��� �������������
//{"usewisdom", int)				// ��������� ��������� ��� �������������
//{"animated", bool ("auto"|*)}	// ������ ��������� ����������
//{"ghost", bool ("yes"|*)}		// ������ ���������
//{"transparent", bool ("yes"|*)}	// ������ ���������
//{"station", station_info}
//{"sounds", sounds_info}
//{"crushtype"||"crashtype", destruction_info}
//{"hitpoints", int}				// �����������, ����������� ��� �����������
//{"storage", storage_info}		// �����-�� ��������� ����-��
//{"database", database_info}		// ���� ������
//{"curstate", float}				// ������� ���������

static MlVersion g_dynObjectVersion(0,0);

namespace DynObjectDetail
{
}

using namespace DynObjectDetail;

//=====================================================================================//
//                               DynObject::DynObject()                                //
//=====================================================================================//
DynObject::DynObject(const DynObjectOldData &info)
:	DynObjectInfo(info),
	m_inMotion(info.IsAnimated()),
	m_animState(info.GetAnimState()),
	m_targetState(1.0f),
	m_startTime(0.0f),
	m_set(0),
	m_sound(info.GetSoundInfo().m_sounds),
	m_isRoof(info.IsRoof()),
	m_isTransparentShader(false)
{
}

//=====================================================================================//
//                               DynObject::~DynObject()                               //
//=====================================================================================//
DynObject::~DynObject()
{
	//if(Sounder)delete Sounder;
}

//=====================================================================================//
//                            void DynObject::StartSounds()                            //
//=====================================================================================//
void DynObject::StartSound()
{
	m_sound.Start();
}

void invmat(D3DMATRIX &a, const D3DMATRIX &b);

//=====================================================================================//
//                           void DynObject::UpdateOnTime()                            //
//=====================================================================================//
void DynObject::UpdateOnTime(float tau)
{
	point3 pos;
	Quaternion orient;

	m_anim.GetTrans(&pos,tau);
	m_anim.GetAngle(&orient,tau);

	D3DUtil_SetIdentityMatrix(m_world);
	orient.ToRotationMatrix(m_world.m);

	m_world._41 = pos.x;
	m_world._42 = pos.y;
	m_world._43 = pos.z;

	invmat(m_invWorld,m_world);
	m_sound.SetPos(pos);
}

//=====================================================================================//
//                            void DynObject::ChangeState()                            //
//=====================================================================================//
float DynObject::ChangeState(float tau)
{
	return ChangeState(1.0f-floorf(m_animState+0.5f),tau);
}

//=====================================================================================//
//                           float DynObject::ChangeState()                            //
//=====================================================================================//
float DynObject::ChangeState(float target, float tau)
{
	if(m_anim.GetLastTime() < 1e-3f)
	{
		m_animState = 0.0f;
	}
	else
	{
		if(m_inMotion)
		{
			POL_LOG("DynObject::ChangeState(): ��������: ����� � ����� ����, ������� �� ������ �����������\n");
			POL_LOG("\tname: [" << GetName() << "]\n");
			POL_LOG("\tlevel: [" << IWorld::Get()->GetLevel()->GetLevelName() << "]\n");
		}

		if(tau != 0.0f) // ��������� �������� ������
		{
			if(fabsf(target - m_animState) > 1e-3f)
			{
				m_sound.SetState(m_animState == 0.0f ? SndAuto::CLOSING : SndAuto::OPENING);
				m_inMotion = true;
				m_targetState = target;
				m_startTime = tau;
			}
		}
		else
		{
			m_animState = target;
		}
	}

	return fabsf(m_targetState-m_animState) * m_anim.GetLastTime();
}

//=====================================================================================//
//                              void DynObject::Update()                               //
//=====================================================================================//
void DynObject::Update(float tau)
{
	assert( m_inMotion );

	float stdiff = m_targetState - m_animState;
	float remState = fabsf(stdiff) * m_anim.GetLastTime();	// ���������� �� ����� ������������ �������� �����
	float deltaState = (tau - m_startTime) / remState;

	if(deltaState >= 1.0f)
	{
		if(IsAnimated())
		{
			deltaState = fmodf(deltaState,1.0f);
			m_startTime = tau - deltaState*m_anim.GetLastTime();
		}
		else
		{
			stdiff = 0.0f;
			m_inMotion = false;
			m_animState = m_targetState;
			m_sound.SetState(m_animState == 0.0f ? SndAuto::CLOSE : SndAuto::OPEN);
		}
	}

	float curTau = (m_animState + stdiff*deltaState) * m_anim.GetLastTime();
	UpdateOnTime(curTau);
	UpdateBound();
}

//=====================================================================================//
//                            void DynObject::UpdateBound()                            //
//=====================================================================================//
void DynObject::UpdateBound()
{
	point3 np;
	Bound.Degenerate();
	for(int i=0;i<8;i++)
	{
		point3 p = LocalBound[i];
		PointMatrixMultiply(TODXVECTOR(np),TODXVECTOR(p),m_world);
		Bound.Enlarge(np);
	}
}

//=====================================================================================//
//                             bool DynObject::TraceRay()                              //
//=====================================================================================//
bool DynObject::TraceRay(const ray &r, float *Pos, point3 *Norm)
{
	if(GetSet() && GetSet()->Destroyed()) return false;

	bool inter = false;
	if(Bound.IntersectRay(r,Norm))
	{
		ray nr;
		PointMatrixMultiply(*(D3DVECTOR*)&nr.Origin,
			*(D3DVECTOR*)&r.Origin,
			m_invWorld);
		VectorMatrixMultiply(*(D3DVECTOR*)&nr.Direction,
			*(D3DVECTOR*)&r.Direction,
			m_invWorld);

		inter = m_texObject.TraceRay(nr,Pos,Norm);
		if(inter)
		{
			point3 norm;
			VectorMatrixMultiply(*(D3DVECTOR*)&norm,*(D3DVECTOR*)Norm,m_world);
			*Norm=norm;
		}
	}
	return inter;
}

//=====================================================================================//
//                           bool DynObject::TraceSegment()                            //
//=====================================================================================//
bool DynObject::TraceSegment(const ray &r, float *Pos, point3 *Norm)
{
	if(GetSet() && GetSet()->Destroyed()) return false;

	bool inter = false;
	point3 pointOfInt;
	if(Bound.IntersectRay(r,&pointOfInt))
	{
		if((pointOfInt-r.Origin).Length() > r.Direction.Length()) return false;

		ray nr;
		PointMatrixMultiply(*(D3DVECTOR*)&nr.Origin,
			*(D3DVECTOR*)&r.Origin,
			m_invWorld);
		VectorMatrixMultiply(*(D3DVECTOR*)&nr.Direction,
			*(D3DVECTOR*)&r.Direction,
			m_invWorld);

		inter = m_texObject.TraceRay(nr,Pos,Norm);
		if(inter)
		{
			point3 norm;
			VectorMatrixMultiply(*(D3DVECTOR*)&norm,*(D3DVECTOR*)Norm,m_world);
			*Norm=norm;
		}
	}
	return inter;
}

//=====================================================================================//
//                           bool DynObject::MakeSaveLoad()                            //
//=====================================================================================//
bool DynObject::MakeSave(SavSlot &slot)
{
	g_dynObjectVersion.Store(slot);

	slot << m_targetState;
	slot << (m_inMotion?m_targetState:m_animState);
	slot << m_isRoof;

	//Write(slot);
	return true;
	//OptSlot out;
	//Props.Rewind();

	//Props >> q;
	//std::map<std::string,std::string> keys;
	//while(q--)
	//{
	//	std::string Key,Val;
	//	Props>>Key>>Val;
	//	keys[Key]=Val;
	//}
	//char t[100];
	//sprintf(t,"%f",m_animationProgress > 0.5f? 1.0f : 0.0f);
	//keys["curstate"]=t;
	//out<<keys.size();
	//std::map<std::string,std::string>::iterator it=keys.begin(),it1=keys.end();
	//for(;it!=it1;it++)
	//{
	//	out<<it->first<<it->second;
	//}
	//Props=out;
	//Props.Save(slot);
}

//=====================================================================================//
//                             bool DynObject::MakeLoad()                              //
//=====================================================================================//
bool DynObject::MakeLoad(SavSlot &slot)
{
	assert( !slot.IsSaving() );
	if(slot.GetStore()->GetVersion() > 4)
	{
		Load5(slot);
	}
	else
	{
		Load4(slot);
	}

	return true;
}

//=====================================================================================//
//                          void DynObject::UpdateAfterLoad()                          //
//=====================================================================================//
void DynObject::UpdateAfterLoad()
{
	LocalBound=m_texObject.GetBBox();
 	UpdateOnTime(m_animState*m_anim.GetLastTime());

	point3 pos;
	m_anim.GetTrans(&pos,m_animState*m_anim.GetLastTime());
	m_sound.SetPos(pos);

	UpdateBound();
}

//=====================================================================================//
//                               void DynObject::Load4()                               //
//=====================================================================================//
void DynObject::Load4(SavSlot &slot)
{
	m_texObject.MakeSaveLoad(slot);
	m_anim.MakeSaveLoad(slot);

	m_isTransparentShader = false;
	for(int i = 0; i < m_texObject.PartNum && !m_isTransparentShader; ++i)
	{
		VShader *shader = IWorld::Get()->GetPipe()->FindShader(m_texObject.Parts[i]->MaterialName);
		if(shader && shader->Transparent) m_isTransparentShader = true;
	}

	UpdateAfterLoad();
}

//=====================================================================================//
//                          void DynObject::SetShapeHandle()                           //
//=====================================================================================//
void DynObject::SetShapeHandle(ShapePool::Handle h)
{
	m_MyShape = h;
}

//=====================================================================================//
//                           void DynObject::SetTexObject()                            //
//=====================================================================================//
void DynObject::SetTexObject(const TexObject &texo)
{
	m_texObject = texo;

	m_isTransparentShader = false;
	for(int i = 0; i < m_texObject.PartNum && !m_isTransparentShader; ++i)
	{
		VShader *shader = IWorld::Get()->GetPipe()->FindShader(m_texObject.Parts[i]->MaterialName);
		if(shader && shader->Transparent) m_isTransparentShader = true;
	}
}

//=====================================================================================//
//                               void DynObject::Load5()                               //
//=====================================================================================//
void DynObject::Load5(SavSlot &slot)
{
	MlVersion ver;
	ver.Restore(slot);

	if(!ver.IsCompatible(g_dynObjectVersion))
	{
		throw CASUS("Wrong save version");
	}

	slot >> m_targetState;
	slot >> m_animState;
	slot >> m_isRoof;

	m_inMotion = false;
	m_startTime = 0.0f;
	
	if(IsAnimated())
	{
		m_animState = 0.0f;
		m_inMotion = true;
	}

	UpdateOnTime(m_animState*m_anim.GetLastTime());
	point3 pos;
	m_anim.GetTrans(&pos,m_animState*m_anim.GetLastTime());
	m_sound.SetPos(pos);

	UpdateBound();
}

//=====================================================================================//
//                             void DynObject::StopSound()                             //
//=====================================================================================//
//void DynObject::StopSound()
//{
//	Sounder->Stop();
//}
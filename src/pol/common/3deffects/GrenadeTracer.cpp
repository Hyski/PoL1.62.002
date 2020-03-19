/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ������ �������� �������
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#include "precomp.h"

#include "../GraphPipe/GraphPipe.h"
#include "EffectInfo.h"
#include "GrenadeTracer.h"
#include <undercover/skin/animalibrary.h>
#include <logic2/TraceUtils.h>

//using namespace EffectUtility;
using EffectUtility::get0F;
using EffectUtility::get_FF;

static const std::string ITEMSDIR="Animations/skins/items/";
static const std::string ITEMSSKEL="Animations/anims/items/Fiery_Granade.skel";

//
// ������ �������� �������
//

// ����������� � �����������
GrenadeTracer::GrenadeTracer(
							 const KeyAnimation& ka,
							 const std::string& skin_name,
							 float angle_vel
							 )
{
	ready_vector_size = ready_num = 0;

	key_animation = ka;
	lifetime = ka.GetLastTime();
	m_angle_vel = point3(get0F(angle_vel), get0F(angle_vel), get0F(angle_vel));

	// �������� ������
	AnimaLibrary *lib=AnimaLibrary::GetInst();
	m_skeleton = lib->GetSkAnimation(ITEMSSKEL);
	m_animadata = m_skeleton->Start(0);


	// �������� ����
	m_skin = lib->GetSkSkin(ITEMSDIR + skin_name + ".skin");
	if(!m_skin)
	{
		// ������� ����������
		std::string str = "illegal skin: \"";
		str += ITEMSDIR + skin_name + ".skin\"";
		throw CASUS(str);
	}
	// ��������� ��� � �������
	m_skin->ValidateLinks(m_skeleton);
	
	// ����, ������� ���������� ������ �� ��� ����� NextTick
	first_time = true;
	// ����, ����������� �� ������������� ���������� �������
	finished = false;

	// ����� ���
	sound_object = new EmptySound();
}

// ����������
GrenadeTracer::~GrenadeTracer()
{
	// ����� ����
	delete sound_object;

	// ���������� ����
	delete m_skin;
}

// ��������� ��� �������
// (������ ������ ���������)
bool GrenadeTracer::NextTick(const float current_time)
{
	if(first_time) { first_time = false; start_time = current_time; }

	float dt; // ������� �� ������� ����� ������� �������� � �������� �������� �������
	dt = current_time - start_time;

	if (dt >= lifetime)
	{
		finished = true;
		return true;
	}

	// ����� ������
	key_animation.GetTrans(&current_point, dt);

	m_angles.x = fmod(m_angle_vel.x * dt, PIm2);
	m_angles.y = fmod(m_angle_vel.y * dt, PIm2);
	m_angles.z = fmod(m_angle_vel.z * dt, PIm2);

	return true;
}

// ���������� ������� ������� ����� � ������� ������ �����
bool GrenadeTracer::Update()
{
	// ������
	D3DMATRIX mat(
		FastCos(m_angles.y)*FastCos(m_angles.z),
		FastCos(m_angles.y)*FastSin(m_angles.z),
		-FastSin(m_angles.y),
		0,

		FastSin(m_angles.x)*FastSin(m_angles.y)*FastCos(m_angles.z) - FastCos(m_angles.x)*FastSin(m_angles.z),
		FastSin(m_angles.x)*FastSin(m_angles.y)*FastSin(m_angles.z) + FastCos(m_angles.x)*FastCos(m_angles.z),
		FastSin(m_angles.x)*FastCos(m_angles.y),
		0,

		FastCos(m_angles.x)*FastSin(m_angles.y)*FastCos(m_angles.z) + FastSin(m_angles.x)*FastSin(m_angles.z),
		FastCos(m_angles.x)*FastSin(m_angles.y)*FastSin(m_angles.z) - FastSin(m_angles.x)*FastCos(m_angles.z),
		FastCos(m_angles.x)*FastCos(m_angles.y),
		0,

		current_point.x,
		current_point.y,
		current_point.z,
		1);

	m_animadata.LastState.GetBoneEx(0).World = mat;
	m_skin->Update(&m_animadata.LastState);
#ifdef _HOME_VERSION
	pGraphPipe->Chop(m_skin->GetMesh(),"Grenade");
#else
	pGraphPipe->Chop(m_skin->GetMesh());
#endif
	return true;
}

// ���������� bounding box �������
BBox GrenadeTracer::GetBBox(const float )
{
	bbox.Degenerate();
	bbox.Enlarge(current_point + point3(5.0, 5.0, 5.0));
	bbox.Enlarge(current_point - point3(5.0, 5.0, 5.0));
	return bbox;
}

//////////////////////////////////////////////////////////////////////////////
//
// ������ �������� ������
//
//////////////////////////////////////////////////////////////////////////////

// �������� ��� ������� (� ����� ������ �����������)

// ����������� � �����������
ShellTracer::ShellTracer(
						 const unsigned int entity_id,
						 const point3& source,
						 const point3& target,
						 const std::string& skin_name,
						 float angle_vel,
						 float vel
						 )
{
	ready_vector_size = ready_num = 0;

	// �������� ������ ������
	point3 AXIS_dir = Normalize(target - source);
	point3 AXIS_y = Normalize(AXIS_dir.Cross(point3(0.0f, 0.0f, 1.0f)));
	point3 AXIS_x = Normalize(AXIS_dir.Cross(AXIS_y));
	float xx = get0F();
	float yy = get0F();
	float zz = get0F(0.2f);

	point3 dir = Normalize(xx*AXIS_x + yy*AXIS_y + zz*AXIS_dir);

	_GrenadeTracer tracer(entity_id, source, dir, vel, 0);
	key_animation = tracer.GetTrace();
	
	
	lifetime = key_animation.GetLastTime();
	m_angle_vel = point3(get0F(angle_vel), get0F(angle_vel), get0F(angle_vel));

	// �������� ������
	AnimaLibrary *lib=AnimaLibrary::GetInst();
	m_skeleton = lib->GetSkAnimation(ITEMSSKEL);
	m_animadata = m_skeleton->Start(0);


	// �������� ����
	m_skin = lib->GetSkSkin(ITEMSDIR + skin_name + ".skin");
	// ��������� ��� � �������
	m_skin->ValidateLinks(m_skeleton);
	if(!m_skin)
	{
		// ������� ����������
		std::string str = "illegal skin: \"";
		str += ITEMSDIR + skin_name + ".skin\"";
		throw CASUS(str);
	}
	
	// ����, ������� ���������� ������ �� ��� ����� NextTick
	first_time = true;
	// ����, ����������� �� ������������� ���������� �������
	finished = false;

	// ����� ���
	sound_object = new EmptySound();
}

// ����������
ShellTracer::~ShellTracer()
{
	// ����� ����
	delete sound_object;

	// ���������� ����
	delete m_skin;
}

// ��������� ��� �������
// (������ ������ ���������)
bool ShellTracer::NextTick(const float current_time)
{
	if(first_time) { first_time = false; start_time = current_time; }

	float dt; // ������� �� ������� ����� ������� �������� � �������� �������� �������
	dt = current_time - start_time;

	if (dt >= lifetime)
	{
		finished = true;
		return true;
	}

	// ����� ������
	key_animation.GetTrans(&current_point, dt);

	m_angles.x = fmod(m_angle_vel.x * dt, PIm2);
	m_angles.y = fmod(m_angle_vel.y * dt, PIm2);
	m_angles.z = fmod(m_angle_vel.z * dt, PIm2);

	return true;
}

// ���������� ������� ������� ����� � ������� ������ �����
bool ShellTracer::Update()
{
	// ������
	D3DMATRIX mat(
		FastCos(m_angles.y)*FastCos(m_angles.z),
		FastCos(m_angles.y)*FastSin(m_angles.z),
		-FastSin(m_angles.y),
		0,

		FastSin(m_angles.x)*FastSin(m_angles.y)*FastCos(m_angles.z) - FastCos(m_angles.x)*FastSin(m_angles.z),
		FastSin(m_angles.x)*FastSin(m_angles.y)*FastSin(m_angles.z) + FastCos(m_angles.x)*FastCos(m_angles.z),
		FastSin(m_angles.x)*FastCos(m_angles.y),
		0,

		FastCos(m_angles.x)*FastSin(m_angles.y)*FastCos(m_angles.z) + FastSin(m_angles.x)*FastSin(m_angles.z),
		FastCos(m_angles.x)*FastSin(m_angles.y)*FastSin(m_angles.z) - FastSin(m_angles.x)*FastCos(m_angles.z),
		FastCos(m_angles.x)*FastCos(m_angles.y),
		0,

		current_point.x,
		current_point.y,
		current_point.z,
		1);

	m_animadata.LastState.GetBoneEx(0).World = mat;
	m_skin->Update(&m_animadata.LastState);
#ifdef _HOME_VERSION
	pGraphPipe->Chop(m_skin->GetMesh(),"Shell");
#else
	pGraphPipe->Chop(m_skin->GetMesh());
#endif
	return true;
}

// ���������� bounding box �������
BBox ShellTracer::GetBBox(const float )
{
	bbox.Degenerate();
	bbox.Enlarge(current_point + point3(5.0, 5.0, 5.0));
	bbox.Enlarge(current_point - point3(5.0, 5.0, 5.0));
	return bbox;
}


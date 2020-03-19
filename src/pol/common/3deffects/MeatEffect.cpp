/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ����� ��� �������� ����
				
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#include "precomp.h"

#include "../GraphPipe/GraphPipe.h"
#include <undercover/skin/animalibrary.h>
#include <logic2/TraceUtils.h>

#include "EffectInfo.h"
#include "MeatEffect.h"

using namespace EffectUtility;

static const std::string SKELETON="Animations/anims/items/Fiery_Granade.skel";
static const std::string SKINDIR="Animations/skins/items/";
static const int models_num = 15;

std::string skin_name[models_num] = {
	"head", // 1
		"hand", // 2
		"hand", // 3
		"meat01", // 4
		"meat02", // 5
		"meat03", // 6
		"meat04", // 7
		"meat05", // 8
		"meat06", // 9
		"meat07", // 10
		"meat08", // 11
		"meat09", // 12
		"meat10", // 13
		"meat11", // 14
		"tors", // 15
};


//
// ��������� ��� �������� ���������� �� ����� ��������
//
struct MeatEffectParticle
{
	SkSkin* m_skin;
	KeyAnimation m_key_animation;
	point3 m_angle_vel;
	point3 m_angles;
	bool m_dead;
	float m_lifetime;
};


//////////////////////////////////////////////////////////////////////////////
//
// ������ �������� ����
//
//////////////////////////////////////////////////////////////////////////////

// ����������� � �����������
MeatEffect::MeatEffect(
					   const unsigned int entity_id,
					   const point3& explosion,
					   const point3& human,
					   const MEAT_EFFECT_INFO& info,
					   float angle_vel,
					   float vel
					   )
{
	start_size = info.Size*0.5;
	end_size = info.SizeEnd*0.5;
	particles_lifetime = info.LifeTime;
	frequency = info.Frequency;

	// ������
	SetShader(info.Shader);
	
	bbox.Degenerate();
	bbox.Enlarge(human + point3(10.0, 10.0, 10.0));
	bbox.Enlarge(human - point3(10.0, 10.0, 10.0));

	m_models = new MeatEffectParticle[models_num];
	lifetime = 0.0f;

	// �������� ������
	AnimaLibrary *lib=AnimaLibrary::GetInst();
	SkAnim* skeleton = lib->GetSkAnimation(SKELETON);
	m_animadata = skeleton->Start(0);

	// �������� ������ ������
	point3 dir = Normalize(human - explosion);

	for(int i = 0; i < models_num; i++)
	{
		float xx = get0F();
		float yy = get0F();
		float zz = get0F();
		point3 dir_i;
		dir_i.x = dir.x * xx;
		dir_i.y = dir.y * yy;
		dir_i.z = dir.z * zz;
		dir_i = Normalize(dir_i);

		_GrenadeTracer tracer(entity_id, human, dir_i, vel*(get0F()+0.5f), 0, 2.0f);
		m_models[i].m_key_animation = tracer.GetTrace();
		m_models[i].m_lifetime = m_models[i].m_key_animation.GetLastTime();
		if(m_models[i].m_lifetime > lifetime) lifetime = m_models[i].m_lifetime;
		m_models[i].m_angle_vel = point3(get0F(angle_vel), get0F(angle_vel), get0F(angle_vel));

		// �������� ����
		m_models[i].m_skin = lib->GetSkSkin(SKINDIR + skin_name[i] + ".skin");
		if(!m_models[i].m_skin)
		{
			// ������� ����������
			std::string str = "illegal skin: \"";
			str += SKINDIR + skin_name[i] + ".skin\"";
			for(int k = 0; k < i; k++)
			{
				delete m_models[k].m_skin;
			}
			delete []m_models;
			throw CASUS(str);
		}
		// ��������� ��� � �������
		m_models[i].m_skin->ValidateLinks(skeleton);
		m_models[i].m_dead = false;
	}

	// ������� �������
	points_num = ceil(frequency * lifetime * models_num);
	ready_vector_size = points_num*PEAK_NUM;
	points_vector = new point3 [points_num];
	size_vector = new float [points_num];
	points2d_vector = new point3 [points_num];
	prim.Verts.resize(ready_vector_size);
	//color_vector = new unsigned int [ready_vector_size];
	//texcoord_vector = new texcoord [ready_vector_size];
	//ready_vector = new point3 [ready_vector_size];

	particles_num = 0;


	// ����, ������� ���������� ������ �� ��� ����� NextTick
	first_time = true;
	// ����, ����������� �� ������������� ���������� �������
	finished = false;

	// ����� ���
	sound_object = new EmptySound();
}

// ����������
MeatEffect::~MeatEffect()
{
	delete [] points_vector;
	delete [] size_vector;
	delete [] points2d_vector;
	//delete [] color_vector;
	//delete [] texcoord_vector;
	//delete [] ready_vector;

	for(int i = 0; i < models_num; i++)
	{
		delete m_models[i].m_skin;
	}
	delete[] m_models;
	// ����� ����
	delete sound_object;
}

// ��������� ��� �������
// (������ ������ ���������)
bool MeatEffect::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }
	m_dt = current_time - start_time;

	if(m_dt < lifetime)
	{
		// ��� ��� ����-�� ����� - ������ ������������

		// ���������, ������� ������� ������ � ����������� �������� �,
		// ���� ����, �������� ����� �������

		// ���������� ������, ������� ����� �������
		int n = floor(frequency * (current_time - previous_birth_time));
		// ���� �� ���������
		for(int i = 0; i < models_num; i++)
		{
			if(m_models[i].m_dead) continue; // �������� ��� ��������
			if(m_models[i].m_lifetime < m_dt)
			{
				// �������� �������� ������ ���
				m_models[i].m_dead = true;
				continue;
			}
			// ������ ��������, ������� ��� �����
			point3 current_point;
			m_models[i].m_key_animation.GetTrans(&current_point, m_dt);
			// ������� ������, ���� �����
			if(n > 0)
			{
				// ���� ������� ���� ��� ����� ������
				float delta_t = (current_time - previous_birth_time) / n;
				for(int j = 1; j <= n; j++)
				{
					BloodParticle bp;
					bp.birthtime = previous_birth_time + j*delta_t;
					bp.start_point = current_point;// - velocity*(n - j)*delta_t; // FIXME
					bp.coords = bp.start_point;
					bp.lifetime = particles_lifetime;
					bp.velocity = point3(0, 0, -1.5f);
					bp.texcoord_type = (float)rand()/32768.0 * 4;
					
					particles.push_back(bp);
				} // for
			} // if(n > 0)
		}// for �� ���������
		if(n>0)
		{
			// �������� ����� ����� ��������
			previous_birth_time = current_time;
		}
	}

	// ������ ������� ������� � ���������� ��� ��� �����
	BloodParticlesListIterator I, J, E; // ��������, �������������� � �������� ���������
	I = particles.begin();
	E = particles.end();
	while(I != E)
	{
		float dtt = (current_time - (*I).birthtime);
		if( dtt >= (*I).lifetime )
		{
			// ������ �������
			J = I;
			I++;
			particles.erase(J);
			continue;
		}
		// ��� ����� ������� - ����������� ��� ��� ���
		(*I).size = start_size + (end_size - start_size)*dtt/(*I).lifetime;
		(*I).coords = (*I).start_point + (*I).velocity * dtt;
		unsigned int c = 255*(1 - dtt/(*I).lifetime);
		(*I).color = RGBA_MAKE(c, c, c, c);
		I++;
	}
	
	// ���� ������ ���� ��� ����� ����� - �������� ������
	if(particles.empty() && (m_dt > lifetime))
	{
		finished = true;
		return true;
	}
	return true;
}

// ���������� ������� ������� ����� � ������� ������ �����
bool MeatEffect::Update()
{
	// ������ ������
	float x1 = 0.0f, x2 = 1.0f, y1 = 0.0f, y2 = 1.0f;
	particles_num = 0;
	BloodParticlesListIterator I, E; // �������� � �������� ���������
	E = particles.end();
	for(I = particles.begin(); I != E; I++)
	{
		points_vector[particles_num] = (*I).coords;
		size_vector[particles_num] = (*I).size;

		prim.Verts[particles_num*PEAK_NUM].diffuse = (*I).color;
		prim.Verts[particles_num*PEAK_NUM + 1].diffuse = (*I).color;
		prim.Verts[particles_num*PEAK_NUM + 2].diffuse = (*I).color;
		prim.Verts[particles_num*PEAK_NUM + 3].diffuse = (*I).color;
		prim.Verts[particles_num*PEAK_NUM + 4].diffuse = (*I).color;
		prim.Verts[particles_num*PEAK_NUM + 5].diffuse = (*I).color;

		switch((*I).texcoord_type)
		{
		case 0: x1 = 0; y1 = 0; x2 = 0.5; y2 = 0.5; break;
		case 1: x1 = 0.5; y1 = 0; x2 = 1; y2 = 0.5; break;
		case 2: x1 = 0; y1 = 0.5; x2 = 0.5; y2 = 1; break;
		case 3: x1 = 0.5; y1 = 0.5; x2 = 1; y2 = 1; break;
		}
		// ����������� ���������� ����������
		prim.Verts[particles_num*PEAK_NUM].uv = texcoord(x1, y1);
		prim.Verts[particles_num*PEAK_NUM + 1].uv = texcoord(x2, y1);
		prim.Verts[particles_num*PEAK_NUM + 2].uv = texcoord(x2, y2);
		prim.Verts[particles_num*PEAK_NUM + 3].uv = texcoord(x1, y1);
		prim.Verts[particles_num*PEAK_NUM + 4].uv = texcoord(x2, y2);
		prim.Verts[particles_num*PEAK_NUM + 5].uv = texcoord(x1, y2);

		particles_num++;
	}
	// ����� ������� ������� �������������
	pGraphPipe->TransformPoints(particles_num, points_vector, points2d_vector);
	// ���������� �����
	Multiply();

	// ��������
	for(int i = 0; i < models_num; i++)
	{
		if(m_models[i].m_dead) continue;
		point3 current_point;
		m_models[i].m_key_animation.GetTrans(&current_point, m_dt);
		
		m_models[i].m_angles.x = fmod(m_models[i].m_angle_vel.x * m_dt, PIm2);
		m_models[i].m_angles.y = fmod(m_models[i].m_angle_vel.y * m_dt, PIm2);
		m_models[i].m_angles.z = fmod(m_models[i].m_angle_vel.z * m_dt, PIm2);
		
		D3DMATRIX mat(
			FastCos(m_models[i].m_angles.y)*FastCos(m_models[i].m_angles.z),
			FastCos(m_models[i].m_angles.y)*FastSin(m_models[i].m_angles.z),
			-FastSin(m_models[i].m_angles.y),
			0,
			
			FastSin(m_models[i].m_angles.x)*FastSin(m_models[i].m_angles.y)*FastCos(m_models[i].m_angles.z) - FastCos(m_models[i].m_angles.x)*FastSin(m_models[i].m_angles.z),
			FastSin(m_models[i].m_angles.x)*FastSin(m_models[i].m_angles.y)*FastSin(m_models[i].m_angles.z) + FastCos(m_models[i].m_angles.x)*FastCos(m_models[i].m_angles.z),
			FastSin(m_models[i].m_angles.x)*FastCos(m_models[i].m_angles.y),
			0,
			
			FastCos(m_models[i].m_angles.x)*FastSin(m_models[i].m_angles.y)*FastCos(m_models[i].m_angles.z) + FastSin(m_models[i].m_angles.x)*FastSin(m_models[i].m_angles.z),
			FastCos(m_models[i].m_angles.x)*FastSin(m_models[i].m_angles.y)*FastSin(m_models[i].m_angles.z) - FastSin(m_models[i].m_angles.x)*FastCos(m_models[i].m_angles.z),
			FastCos(m_models[i].m_angles.x)*FastCos(m_models[i].m_angles.y),
			0,
			
			current_point.x,
			current_point.y,
			current_point.z,
			1);
		
		m_animadata.LastState.GetBoneEx(0).World = mat;
		m_models[i].m_skin->Update(&m_animadata.LastState);
#ifdef _HOME_VERSION
		pGraphPipe->Chop(m_models[i].m_skin->GetMesh(),"MeatPiece");
#else
		pGraphPipe->Chop(m_models[i].m_skin->GetMesh());
#endif
	}

	return true;
}

bool MeatEffect::Start(const float start_time)
{
	// ��������� ����� ������ ���������� �������
	this->start_time = start_time;
	// ����
	color = 0xffffffff;
	previous_birth_time = start_time;
	return true;
}

// ������� ����������� �����
void MeatEffect::Multiply()
{
	float f, n, betta;
	float xfactor;
	float yfactor;
	int xres, yres;
	pGraphPipe->GetProjOptions(&f, &n, &betta);
	pGraphPipe->GetResolution(&xres, &yres);
	point3 right = pGraphPipe->GetCam()->GetRight();
	point3 up = pGraphPipe->GetCam()->GetUp();

	xfactor = PoLFXFactor;
	yfactor = PoLFXFactor;

	ready_num = 0;
	int ii = 0;
	for(int i = 0; i < particles_num; i++)
	{
		point3 xdelta = right * size_vector[i] * xfactor;
		point3 ydelta = up * size_vector[i] * yfactor;

		prim.Verts[ii*PEAK_NUM + 0].pos = points2d_vector[i] - xdelta + ydelta;
		prim.Verts[ii*PEAK_NUM + 1].pos = points2d_vector[i] + xdelta + ydelta;
		prim.Verts[ii*PEAK_NUM + 2].pos = points2d_vector[i] + xdelta - ydelta;
		prim.Verts[ii*PEAK_NUM + 3].pos = points2d_vector[i] - xdelta + ydelta;
		prim.Verts[ii*PEAK_NUM + 4].pos = points2d_vector[i] + xdelta - ydelta;
		prim.Verts[ii*PEAK_NUM + 5].pos = points2d_vector[i] - xdelta - ydelta;

		// �������� ���������� �������� �����
		ready_num += PEAK_NUM;
	}
}

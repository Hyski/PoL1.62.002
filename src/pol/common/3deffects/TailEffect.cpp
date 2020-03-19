/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ����� ������ �� ������
				����� �������� �������� �� ��������
				
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#include "precomp.h"

#include "../GraphPipe/GraphPipe.h"
#include <undercover/skin/animalibrary.h>

#include "EffectInfo.h"
#include "TailEffect.h"

//Grom
class PerlinNoise
{
public:
	point3 getValue(const point3& pnt);
	PerlinNoise();
private:
	std::vector<point3> m_Vectors; 
	float m_GridStep;

	float frand() const {return ((float)rand())/32767.f-0.5f;}
	point3 getBase(int x, int y, int z)
	{
		unsigned n=(17065*x)+(y*77789)+(z*998678);
		return m_Vectors[n%m_Vectors.size()];
	}
};
PerlinNoise::PerlinNoise()
:m_GridStep(0.3f)
{
	const int VecNum = 512;
	m_Vectors.resize(VecNum);

	for(int i=0; i<VecNum; ++i)
	{
		m_Vectors[i] = point3(frand(), frand(), frand());
		//m_Vectors[i].normalize(); 
	}
}
point3 PerlinNoise::getValue(const point3& pnt)
{

	int nx = pnt.x/m_GridStep;
	int ny = pnt.y/m_GridStep;
	int nz = pnt.z/m_GridStep;
	point3 v1 = getBase(0+nx, 0+ny, 0+nz); 
	point3 v2 = getBase(1+nx, 0+ny, 0+nz); 
	point3 v3 = getBase(1+nx, 0+ny, 1+nz); 
	point3 v4 = getBase(0+nx, 0+ny, 1+nz); 
	point3 v5 = getBase(0+nx, 1+ny, 0+nz); 
	point3 v6 = getBase(1+nx, 1+ny, 0+nz); 
	point3 v7 = getBase(1+nx, 1+ny, 1+nz); 
	point3 v8 = getBase(0+nx, 1+ny, 1+nz); 

	point3 delta = point3 (fmod(pnt.x,m_GridStep), fmod(pnt.y,m_GridStep), fmod(pnt.z,m_GridStep));
	delta = delta/m_GridStep;


	point3 a1 = v5*delta.y + v1*(1.f-delta.y);
	point3 a2 = v6*delta.y + v2*(1.f-delta.y);
	point3 a3 = v7*delta.y + v3*(1.f-delta.y);
	point3 a4 = v8*delta.y + v4*(1.f-delta.y);


	point3 a5 = a2*delta.x + a1*(1.f-delta.x);
	point3 a6 = a3*delta.x + a4*(1.f-delta.x);

	point3 a  = a6*delta.z + a5*(1.f-delta.z);

	return Normalize(a);

}
PerlinNoise Noise;
//Grom
////////////////////////////////////////////////////////////////////////////////////////////////
//
// ����� ������ �� ������
//
/////////////////////////////////////////////////////////////////////////////////////////////////

// ����������� � �����������
TailEffect::TailEffect(
					   const point3 StartPoint,     // ��������� �����
					   const point3 EndPoint,	    // �������� �����
					   const TAIL_EFFECT_INFO& info
					   )
{
	start_point = StartPoint;                       // ��������� �����
	end_point = EndPoint;                           // �������� �����
	start_size = info.Size*0.5;
	end_size = info.SizeEnd*0.5;
	lifetime = (end_point - start_point).Length() / info.VelFactor;
	velocity = (end_point - start_point)/lifetime;  // ��������
	direction = Normalize(end_point - start_point);
	particles_lifetime = info.LifeTime;
	frequency = info.Frequency;


	// ������� �������
	points_num = ceil(frequency * lifetime);
	ready_vector_size = points_num*PEAK_NUM;
	points_vector = new point3 [points_num];
	size_vector = new float [points_num];
	points2d_vector = new point3 [points_num];
	//color_vector = new unsigned int [ready_vector_size];
	//texcoord_vector = new texcoord [ready_vector_size];
	//ready_vector = new point3 [ready_vector_size];
	prim.Verts.resize(ready_vector_size);

	particles_num = 0;

	// ����, ������� ���������� ������ �� ��� ����� NextTick
	first_time = true;
	// ����, ����������� �� ������������� ���������� �������
	finished = false;

	// ������
	SetShader(info.Shader);
	// ����
	if(info.Sound == "")
	{
		// ����� ���
		sound_object = new EmptySound();
	}
	else
	{
		if(info.SndMov)
		{
			sound_object = new ActiveSound(info.Sound);
		}
		else
		{
			sound_object = new FixedSound(info.Sound, StartPoint);
		}
	}
}

// ����������
TailEffect::~TailEffect()
{
	delete [] points_vector;
	delete [] size_vector;
	delete [] points2d_vector;
	//delete [] color_vector;
	//delete [] texcoord_vector;
	//delete [] ready_vector;
	// ����
	delete sound_object;
}

// ��������� ��� �������
// (������ ������ ���������)
bool TailEffect::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); last_tick=current_time;}
	
	float delta_time = current_time-last_tick;
	last_tick = current_time;


	float dt; // ������� �� ������� ����� ������� �������� � �������� �������� �������
	float dtt; // �����, ������� ������� ������� � ������� ������ ��������
	unsigned int c;
	dt = current_time - start_time;

	// ���������� ������� �������
	current_point = start_point + velocity*dt;

	if (dt < lifetime)
	{
		// ���������, ������� ������� ������ � ����������� �������� �, ���� ����, �������� ����� �������
		int n = floor(frequency * (current_time - previous_birth_time));
		if(n > 0)
		{
			// ���� ������� ���� ��� ����� ������
			float delta_t = (current_time - previous_birth_time) / n;
			for(int i = 1; i <= n; i++)
			{
				TailParticle tp;
				tp.birthtime = previous_birth_time + i*delta_t;
				tp.start_point = current_point - velocity*(n - i)*delta_t;
				tp.coords = tp.start_point;
				tp.lifetime = particles_lifetime;
				tp.velocity = point3(0, 0, 0);
				tp.texcoord_type = (float)rand()/32768.0 * 4;
				
				particles.push_back(tp);
			}

			// �������� ����� ����� ��������
			previous_birth_time = current_time;
		}
	}

	// ������ ������� ������� � ���������� ��� ��� �����
	TailParticlesListIterator I, J, E; // ��������, �������������� � �������� ���������
	I = particles.begin();
	E = particles.end();
	while(I != E)
	{
		dtt = (current_time - (*I).birthtime);
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
		I->start_point += Noise.getValue(I->start_point)*delta_time*3*end_size/particles_lifetime;
		I->coords = I->start_point;
		c = 255*(1 - dtt/(*I).lifetime);
		(*I).color = RGBA_MAKE(c, c, c, c);
		I++;
	}

	// ���� ������ ���� ��� ����� ����� - �������� ������
	if(particles.empty() && (dt > lifetime))
	{
		finished = true;
		return true;
	}
	return true;
}

// ���������� ������� ������� ����� � ������� ������ �����
bool TailEffect::Update()
{
	// ����
	sound_object->Update(current_point, velocity);

	float x1 = 0, x2 = 1, y1 = 0, y2 = 1;
	particles_num = 0;
	TailParticlesListIterator I, E; // �������� � �������� ���������
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

	return true;
}

// ���������� bounding box �������
BBox TailEffect::GetBBox(const float )
{
	BBox b;
	b.Degenerate();
	b.Enlarge(start_point - point3(end_size, end_size, end_size));
	b.Enlarge(end_point + point3(end_size, end_size, end_size));
	return b;
}

// � ������ ����� ������ ���������� ������
// ������� ��� ������� �������
bool TailEffect::Start(const float start_time)
{
	// ��������� ����� ������ ���������� �������
	this->start_time = start_time;

	// ����
	color = 0xffffffff;

	// ���������� ������� �����
	current_point = start_point;
	previous_birth_time = start_time;

	return true;
}



// ������� ����������� �����
void TailEffect::Multiply()
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
		++ii;
		ready_num += PEAK_NUM;
	}
}


//
// ����� ������ �� ������
//

// ���������� ������� ������� ����� � ������� ������ �����
bool TailCircleEffect::Update()
{
	// ����
	sound_object->Update(current_point, velocity);

	particles_num = 0;
	TailParticlesListIterator I, E; // �������� � �������� ���������
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

		particles_num++;
	}
	// ���������� �����
	Multiply();

	return true;
}

// � ������ ����� ������ ���������� ������
// ������� ��� ������� �������
bool TailCircleEffect::Start(const float start_time)
{
	// ��������� ����� ������ ���������� �������
	this->start_time = start_time;

	// ����
	color = 0xffffffff;

	// ���������� ������� �����
	current_point = start_point;
	previous_birth_time = start_time;

	// �������� ���������� ����������
	for(int i = 0; i < points_num; i++)
	{
		prim.Verts[i*PEAK_NUM].uv = texcoord(0, 1);
		prim.Verts[i*PEAK_NUM + 1].uv = texcoord(1, 1);
		prim.Verts[i*PEAK_NUM + 2].uv = texcoord(1, 0);
		prim.Verts[i*PEAK_NUM + 3].uv = texcoord(0, 1);
		prim.Verts[i*PEAK_NUM + 4].uv = texcoord(1, 0);
		prim.Verts[i*PEAK_NUM + 5].uv = texcoord(0, 0);
	}

	return true;
}

// ������� ����������� �����
void TailCircleEffect::Multiply()
{
	point3 p = direction.Cross(point3(1.0, 0.0, 0.0));
	p = Normalize(p);
	point3 up;
	Quaternion qtpi4, qtpi2, qtpi;
	qtpi4.FromAngleAxis(PId4, direction);
	qtpi2.FromAngleAxis(PId2, direction);
	qtpi.FromAngleAxis(PI, direction);
	ready_num = 0;
	for(int i = 0; i < particles_num; i++)
	{
		up = p*size_vector[i];
		// ��������� ����������
		prim.Verts[i*PEAK_NUM].pos = points_vector[i] + qtpi4*(qtpi*up);
		prim.Verts[i*PEAK_NUM + 1].pos = points_vector[i] + qtpi4*(qtpi2*up);
		prim.Verts[i*PEAK_NUM + 2].pos = points_vector[i] + qtpi4*up;
		prim.Verts[i*PEAK_NUM + 3].pos = points_vector[i] + qtpi4*(qtpi*up);
		prim.Verts[i*PEAK_NUM + 4].pos = points_vector[i] + qtpi4*up;
		prim.Verts[i*PEAK_NUM + 5].pos = points_vector[i] + qtpi4*(qtpi2*(qtpi*up));

		// �������� ���������� �������� �����
		ready_num += PEAK_NUM;
	}
}


//
// ����� ������ �� ������, �������� �� ��������
//

// ����������� � �����������
TailParabolaEffect::TailParabolaEffect(
									   const point3 StartPoint,                          // ��������� �����
									   const point3 EndPoint,	                         // �������� �����
									   const TAIL_EFFECT_INFO& info,
									   const float Gravitation
									   ) : TailEffect(StartPoint, EndPoint, info)
{ 
	gravitation = Gravitation;
	vz = (EndPoint.z - StartPoint.z)/lifetime - gravitation*lifetime*0.5;
}

// ��������� ��� �������
// (������ ������ ���������)
bool TailParabolaEffect::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }
	float dt; // ������� �� ������� ����� ������� �������� � �������� �������� �������
	float dtt; // �����, ������� ������� ������� � ������� ������ ��������
	unsigned int c;
	dt = current_time - start_time;

	// ���������� ������� �������
	current_point = start_point + velocity*dt;
	current_point.z = start_point.z + vz*dt + gravitation*dt*dt*0.5;

	if (dt < lifetime)
	{
		// ���������, ������� ������� ������ � ����������� �������� �, ���� ����, �������� ����� �������
		int n = floor(frequency * (current_time - previous_birth_time));
		if(n > 0)
		{
			// ���� ������� ���� ��� ����� ������
		float delta_t = (current_time - previous_birth_time) / n;
			for(int i = 1; i <= n; i++)
			{
				TailParticle tp;
				tp.birthtime = previous_birth_time + i*delta_t;
				float dttt = tp.birthtime - start_time;

				tp.start_point = start_point + velocity*dttt;
				tp.start_point.z = start_point.z + vz*dttt + gravitation*dttt*dttt*0.5;
				tp.coords = tp.start_point;
				tp.lifetime = particles_lifetime;
				tp.velocity = point3(0, 0, 0);
				tp.texcoord_type = (float)rand()/32768.0 * 4;
				
				particles.push_back(tp);
			}

			// �������� ����� ����� ��������
			previous_birth_time = current_time;
		}
	}

	// ������ ������� ������� � ���������� ��� ��� �����
	TailParticlesListIterator I, J, E; // ��������, �������������� � �������� ���������
	I = particles.begin();
	E = particles.end();
	while(I != E)
	{
		dtt = (current_time - (*I).birthtime);
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
		c = 255*(1 - dtt/(*I).lifetime);
		(*I).color = RGBA_MAKE(c, c, c, c);
		I++;
	}

	// ���� ������ ���� ��� ����� ����� - �������� ������
	if(particles.empty() && (dt > lifetime))
	{
		finished = true;
		return true;
	}
	return true;
}

// ���������� bounding box �������
BBox TailParabolaEffect::GetBBox(const float )
{
	BBox b;
	b.Degenerate();
	b.Enlarge(start_point - point3(end_size, end_size, end_size));
	b.Enlarge(end_point + point3(end_size, end_size, end_size));
	point3 tmp = start_point + velocity*lifetime*0.5;
	tmp.z = start_point.z + vz*0.5*lifetime + gravitation*0.5*0.5*0.5*lifetime*lifetime;
	return b;
}

//
// ����� �������� �������� �� ��������
//

// ����������� � �����������
TracerParabolaModel::TracerParabolaModel(
										 const point3 StartPoint,  // ��������� �����
										 const point3 EndPoint,	   // �������� �����
										 const TRACER_PARABOLA_MODEL_INFO& info
										 )
{
	// �������� �������� �����, ������ � ���������� ���������
	points_num = 0;
	ready_vector_size = ready_num = 0;

	// ����, ������� ���������� ������ �� ��� ����� NextTick
	first_time = true;

	// 
	start_point = StartPoint;                       // ��������� �����
	end_point = EndPoint;                           // �������� �����

	// ����� ����� �������
	lifetime = (EndPoint - StartPoint).Length()/info.VelFactor;

	direction = Normalize(EndPoint - StartPoint);
	vel = direction * info.VelFactor;  // ��������
	gravitation = info.Gravitation;
	vz = (EndPoint.z - StartPoint.z)/lifetime - gravitation*lifetime*0.5;
	bbox.Degenerate();
	bbox.Enlarge(start_point);
	bbox.Enlarge(end_point);
	
	// ����, ����������� �� ������������� ���������� �������
	finished = false;

	// ����� ���
	sound_object = new EmptySound();

	// �������� ��������
	AnimaLibrary *lib=AnimaLibrary::GetInst();
	m_skeleton = lib->GetSkAnimation(std::string("Animations/anims/items/") + info.Shader);
	m_animadata = m_skeleton->Start(0);

	// �������� ����
	m_skin = lib->GetSkSkin(std::string("Animations/skins/items/") + info.Sound);
	if(!m_skin)
	{
		// ������� ����������
		std::string str = "illegal skin: \"";
		str += std::string("Animations/skins/items/") + info.Sound + "\"";
		delete sound_object;
		throw CASUS(str);
	}
	// ��������� ��� � �������
	m_skin->ValidateLinks(m_skeleton);


	m_mat._11 = 1;
	m_mat._12 = 0;
	m_mat._13 = 0;
	m_mat._14 = 0;

	m_mat._21 = 0;
	m_mat._22 = 1;
	m_mat._23 = 0;
	m_mat._24 = 0;

	m_mat._31 = 0;
	m_mat._32 = 0;
	m_mat._33 = 1;
	m_mat._34 = 0;

	m_mat._41 = 0;
	m_mat._42 = 0;
	m_mat._43 = 0;
	m_mat._44 = 1;

	m_animadata.LastState.GetBoneEx(0).World = m_mat;
	m_skin->Update(&m_animadata.LastState);
}

// ����������
TracerParabolaModel::~TracerParabolaModel()
{
	// �����
	delete sound_object;
	// ��������
	delete m_skin;
}

// ��������� ��� �������
// (������ ������ ���������)
bool TracerParabolaModel::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }

	float dt; // ������� �� ������� ����� ������� �������� � �������� �������� �������
	dt = current_time - start_time;

	if (dt >= lifetime)
	{
		finished = true;
		return true;
	}
	current_point = start_point + vel*dt;
	current_point.z = start_point.z + vz*dt + gravitation*dt*dt*0.5;
	return true;
}

// ���������� ������� ������� ����� � ������� ������ �����
bool TracerParabolaModel::Update()
{
	// ����
	sound_object->Update(start_point, vel);

	// ��������
	m_mat._41 = current_point.x;
	m_mat._42 = current_point.y;
	m_mat._43 = current_point.z;

	m_animadata.LastState.GetBoneEx(0).World = m_mat;
	m_skin->Update(&m_animadata.LastState);
#ifdef _HOME_VERSION
	pGraphPipe->Chop(m_skin->GetMesh(),"TracerParabolaModel");
#else
	pGraphPipe->Chop(m_skin->GetMesh());
#endif
	return true;
}

// ���������� bounding box �������
BBox TracerParabolaModel::GetBBox(const float )
{
	return bbox;
}

// � ������ ����� ������ ���������� ������
// ������� ��� ������� �������
bool TracerParabolaModel::Start(const float start_time)
{
	// ��������� ����� ������ ���������� �������
	this->start_time = start_time;
	return true;
}

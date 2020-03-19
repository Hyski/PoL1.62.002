/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ������������ �����
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                
#include "precomp.h"

#include "..\GraphPipe\GraphPipe.h"
#include "EffectInfo.h"
#include "DirectExplosion.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ����� ������������������ ���������� ������
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// �����������
DirectSparkleSequence::DirectSparkleSequence(const unsigned int SeqLen)
{
	seq_len = SeqLen;
	sequence = new BaseParticle[seq_len];
}

DirectSparkleSequence::~DirectSparkleSequence()
{
	delete [] sequence;
}

// ��������� ��� �������

void DirectSparkleSequence::NextTick(const float dt)
{
	float t;
	int c;
	
	for(int i = 0; i != seq_len; i++)
	{
		// ����������
		t = (dt - dt/seq_len*0.3*i);
		if(t<0) t = 0;
		sequence[seq_len - i - 1].coords = start_point + velocity*t + accel*t*t*0.5;
		// ������
		sequence[seq_len - i - 1].size = (lifetime-t*0.8)*size;
		// ����
		c = (lifetime - dt)*(255.0/lifetime);
		if(i == 0) sequence[seq_len - i - 1].color = RGBA_MAKE(0xff, 0xff, 0xff, 0xff);
		else sequence[seq_len - i - 1].color = RGBA_MAKE(c, c, c, c);
	}
}


// ������������������� ������������������
void DirectSparkleSequence::Rebirth(
		const point3 st_point,         // ��������� �����
		const point3 vecVelocity,    // ������ ����������� ��������
		const point3 DV,                // ������� ��������
		const point3 Accel,       // ���������
		const float stSize,	              // ��������� ������
		const float stRange,            // ��������� ����� ���������� �������
		const float Lifetime             // ����� �����
		)
{
	// ��� ��������� ���������
	float dx, dy, dz; //, a, b, r;
	point3 dv;

	// ���������� ������� �������
	coords = st_point;
	// ����� �������� ������� �������
	start_point = st_point;
	// ��������� ������� �������
	accel = Accel;
	// ����� �����
	lifetime = Lifetime;

	dv =DV;
	dx = (1 - 2.0*(float)rand()/32768.0);
	dy = (1 - 2.0*(float)rand()/32768.0);
	dz = (1 - 2.0*(float)rand()/32768.0);
	dv.x *= dx;
	dv.y *= dy;
	dv.z *= dz;
	velocity = vecVelocity + dv;
	// ������, ���� � ��������� ���������� ������
	sequence[0].size = stSize + (float)rand()/32768.0*stRange;
	// �������������� ������
	size = sequence[0].size;
	sequence[0].color = 0xffa0a0a0;
	sequence[0].coords = st_point;
	for(int i = 1; i != seq_len; i++)
	{
		sequence[i].size = sequence[0].size + 0.01;
		sequence[i].color = sequence[0].color + 0x00100000;
		sequence[i].coords = st_point;
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ����� ������������ ����
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// ��������� ��� �������
// (������ ������ ���������)
bool DirectSparkles::NextTick(const float current_time)
{
	
	if(first_time) { first_time = false; Start(current_time); }

	float dt; // ������� �� ������� ����� ������� �������� � �������� �������� �������
	dt = current_time - start_time;
	if (dt >= lifetime)
	{
		finished = true;
		return true;
	}

	for(int i = 0; i != sequences_num; i++) sequences[i]->NextTick(dt);
	return true;
}

// ����������
DirectSparkles::~DirectSparkles()
{
	for(unsigned int i = 0; i < sequences_num; i++) delete sequences[i];
	delete[] sequences;
	//delete[] color_vector;
	delete[] points2d_vector;
	delete[] points_vector;
	//delete[] texcoord_vector;
	//delete[] ready_vector;
	// ����
	delete sound_object;
}

// ����������� � �����������
DirectSparkles::DirectSparkles(
							   const point3 rp,            // �������� �����
							   const point3 vecVelocity,   // ������ ����������� ��������
							   const DIRECT_SPARKLES_INFO &info							   )
{
	// ������� ������ �������������������
	seq_len = info.PartNum;
	sequences_num = info.SeqNum;
	sequences = new PDirectSparkleSequence[sequences_num];
	for(unsigned int i = 0; i < sequences_num; i++)
	{
		sequences[i] = new DirectSparkleSequence(info.PartNum);
	}

	// �������� �������� �����, ������ � ���������� ���������
	points_num = sequences_num * info.PartNum;
	ready_vector_size = ready_num = points_num*PEAK_NUM;

	// ������� �������
	//color_vector = new unsigned int [ready_vector_size];
	points2d_vector = new point3 [points_num];
	points_vector = new point3 [points_num];
	//texcoord_vector = new texcoord [ready_vector_size];
	//ready_vector = new point3 [ready_vector_size];
	prim.Verts.resize(ready_vector_size);

	// ����, ������� ���������� ������ �� ��� ����� NextTick
	first_time = true;

	// �������� �����
	root_point = rp;

	vec_velocity = vecVelocity;        // ������ ����������� ��������
	vec_velocity *= info.VelFactor;
	dv = info.VelDisp;                                  // ������� ��������
	accel = info.Accel;                           // ���������
	start_size = info.SizeBegin;                    // ��������� ������
	start_size_range = info.SizeRnd;       // ��������� ����� ���������� �������

	lifetime = info.LifeTime;         // ������������ ����� �����
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
			sound_object = new FixedSound(info.Sound, rp);
		}
	}
}

// � ������ ����� ������ ���������� ������
// ������� ��� ������� �������
bool DirectSparkles::Start(const float start_time)
{
	// ��������� ����� ������ ���������� �������
	this->start_time = this->prev_time = start_time;

	// ������������������� ������������������
	for(int i = 0; i != sequences_num; i++)
		sequences[i]->Rebirth(root_point, vec_velocity, dv, accel, start_size, start_size_range, lifetime);

	// ����������� ���������� ����������
	for(int i = 0; i != sequences_num*seq_len; i++)
	{	
		prim.Verts[i*PEAK_NUM].uv = texcoord(0, 0);
		prim.Verts[i*PEAK_NUM + 1].uv = texcoord(1, 0);
		prim.Verts[i*PEAK_NUM + 2].uv = texcoord(1, 1);
		prim.Verts[i*PEAK_NUM + 3].uv = texcoord(0, 0);
		prim.Verts[i*PEAK_NUM + 4].uv = texcoord(1, 1);
		prim.Verts[i*PEAK_NUM + 5].uv = texcoord(0, 1);
	}
	return true;
}

// ���������� ������� ������� ����� � ������� ������ �����
bool DirectSparkles::Update()
{
	// ����
	sound_object->Update(root_point, vec_velocity);
	// ����������� ������ ��� ������������� (points_vector)
	for(int i = 0; i != sequences_num; i++)
	{
		for(int j = 0; j != seq_len; j++)
		{
			points_vector[i*seq_len + j] = sequences[i]->sequence[j].coords;
		}
	}
	// ����� ������� ������� �������������
	pGraphPipe->TransformPoints(points_num, points_vector, points2d_vector);
	// ���������� �����
	Multiply();
	return true;
}

// ������� ����������� �����
void DirectSparkles::Multiply()
{
	float f, n, betta;
	float xfactor;
	float yfactor;
	int xres, yres;
	int k, l, i, j;
	pGraphPipe->GetProjOptions(&f, &n, &betta);
	pGraphPipe->GetResolution(&xres, &yres);
	point3 right = pGraphPipe->GetCam()->GetRight();
	point3 up = pGraphPipe->GetCam()->GetUp();

	xfactor = PoLFXFactor;
	yfactor = PoLFXFactor;

	// ������� �������, ��� ��� ����� �����, � ���������� ������� ������ 
	// ����� ������� �������, � ������� ��� �����
	ready_num = ready_vector_size;
	// ������� ���������� �����, ������� ���������� � ������� �������
	k = 0;
	// ������� ����� ����� � ������� ����� �������������
	l = 0; 
	for(i = 0; i != sequences_num; i++) // �� �������������������
	{
		for(j = 0; j != seq_len; j++) // �� ������ � ������������������
		{
			point3 xdelta = right * sequences[i]->sequence[j].size * xfactor;
			point3 ydelta = up * sequences[i]->sequence[j].size * yfactor;

			prim.Verts[k*PEAK_NUM + 0].pos = points2d_vector[l] - xdelta + ydelta;
			prim.Verts[k*PEAK_NUM + 1].pos = points2d_vector[l] + xdelta + ydelta;
			prim.Verts[k*PEAK_NUM + 2].pos = points2d_vector[l] + xdelta - ydelta;
			prim.Verts[k*PEAK_NUM + 3].pos = points2d_vector[l] - xdelta + ydelta;
			prim.Verts[k*PEAK_NUM + 4].pos = points2d_vector[l] + xdelta - ydelta;
			prim.Verts[k*PEAK_NUM + 5].pos = points2d_vector[l] - xdelta - ydelta;

			// ��������� ����
			prim.Verts[k*PEAK_NUM].diffuse = sequences[i]->sequence[j].color;
			prim.Verts[k*PEAK_NUM + 1].diffuse = sequences[i]->sequence[j].color;
			prim.Verts[k*PEAK_NUM + 2].diffuse = sequences[i]->sequence[j].color;
			prim.Verts[k*PEAK_NUM + 3].diffuse = sequences[i]->sequence[j].color;
			prim.Verts[k*PEAK_NUM + 4].diffuse = sequences[i]->sequence[j].color;
			prim.Verts[k*PEAK_NUM + 5].diffuse = sequences[i]->sequence[j].color;
		
			// �������� ���������� ������������ �����
			k++;
			// �������� ����� � ������� ����� ����� �������������
			l++;
		}
	}
}

// ���������� bounding box �������
BBox DirectSparkles::GetBBox(const float current_time)
{
	BBox b;
	float dx, dy, dz, dt;
	b.Degenerate();
	dt = current_time - start_time;
	dx = (vec_velocity.x + 1.0)*dt + accel.x*dt*dt*0.5;
	dy = (vec_velocity.y + 1.0)*dt + accel.y*dt*dt*0.5;
	dz = (vec_velocity.z + 1.0)*dt + accel.z*dt*dt*0.5;
	b.Enlarge(root_point);
	b.Enlarge(root_point + point3(dx, dy, dz));
	b.Enlarge(root_point - point3(dx, dy, dz));
	return b;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ����� ������������� ����
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// ��������� ��� �������
// (������ ������ ���������)
bool DirectExplosion::NextTick(const float )
{
	return true;
}

// ���������� bounding box �������
BBox DirectExplosion::GetBBox(const float )
{
	BBox b;
	b.Degenerate();
	return b;
}

// ���������� ������� ������� ����� � ������� ������ �����
bool DirectExplosion::Update()
{
	return true;
}

// ����������
DirectExplosion::~DirectExplosion()
{
}

// ����������� � �����������
DirectExplosion::DirectExplosion(
								 const point3 ,            // �������� �����
								 const point3 ,   // ������ ����������� ��������
								 const DIRECT_EXPLOSION_INFO& 
								 )
{
}

// � ������ ����� ������ ���������� ������
// ������� ��� ������� �������
bool DirectExplosion::Start(const float )
{
	return true;
}

// ������� ����������� �����
void DirectExplosion::Multiply()
{
}

// ������� ����������� ������� ��� �� ��������
void DirectExplosion::Rebirth(DirectParticle & , float )
{
}

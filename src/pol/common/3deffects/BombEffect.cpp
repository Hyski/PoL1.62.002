/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ����� �����
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#include "precomp.h"

#include "..\GraphPipe\GraphPipe.h"
#include "EffectInfo.h"
#include "BombEffect.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ����� ������������������ ���������� ������
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// ��������� ��� �������
void ParticleSequence::NextTick(const float dt)
{
	float t;
	int c;
	
	for(int i = 0; i < SEQUENCE_LEN; i++)
	{
		// ����������
		t = (dt - dt/SEQUENCE_LEN*0.3*i);
		if(t<0) t = 0;
		sequence[SEQUENCE_LEN - i - 1].coords = start_point + velocity*t + accel*t*t*0.5;
		// ������
		sequence[SEQUENCE_LEN - i - 1].size = (lifetime-t*0.8)*size;
		// ����
		c = (lifetime - dt)*(255.0/lifetime);
		if(i == 0) sequence[SEQUENCE_LEN - i - 1].color = RGBA_MAKE(0xff, 0xff, 0xff, 0xff);
		else sequence[SEQUENCE_LEN - i - 1].color = RGBA_MAKE(c, c, c, c);
	}
}

// ������������������� ������������������
void ParticleSequence::Rebirth(
		const point3 st_point,         // ��������� �����
		const point3 stVelocity,       // ��������� ��������
		const point3 Accel,       // ���������
		const float stSize,	              // ��������� ������
		const float stRange,            // ��������� ����� ���������� �������
		const float Lifetime,             // ����� �����
		const bool bFull                  // ���� true, �� ������ �����, ����� - ��������
		)
{
	// ��� ��������� ���������
	float dx, dy, dz, a, b, r;

	// ���������� ������� �������
	coords = st_point;
	// ����� �������� ������� �������
	start_point = st_point;
	// ��������� ������� �������
	accel = Accel;
	// ����� �����
	lifetime = Lifetime;
	// �������� ������� �������
	a = ((float)rand())/32768.0 * PIm2; // ��������� ���� �����
	// ��������� ���� �����
	if(bFull) b = ((float)rand())/32768.0 * PIm2;
	else b = ((float)rand())/32768.0 * PI;
	r = (0.8 + 0.2*(float)rand()/32768.0); // ��������� ������
	dx = r*cos(b)*cos(a) * stVelocity.x;
	dz = r*sin(b)  * stVelocity.z;
	dy = r*cos(b)*sin(a)  * stVelocity.y;
	velocity = point3(dx, dy, dz);

	// ������, ���� � ��������� ���������� ������
	sequence[0].size = stSize + (float)rand()/32768.0*stRange;
	// �������������� ������
	size = sequence[0].size;
	sequence[0].color = 0xffa0a0a0;
	sequence[0].coords = st_point;
	for(int i = 1; i < SEQUENCE_LEN; i++)
	{
		sequence[i].size = sequence[0].size + 0.01;
		sequence[i].color = sequence[0].color + 0x00100000;
		sequence[i].coords = st_point;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ����� ������������� ������������������� ������
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// ����������� � �����������
BombSparkles::BombSparkles(
						   const point3 rp,            // �������� �����
						   const BOMB_SPARKLES_INFO& info
						   )
{
	vel = point3(0, 0, 0);
	// ������� ������ �������������������
	sequences_num = info.PartNum;
	sequences = new ParticleSequence[sequences_num];

	// �������� �������� �����, ������ � ���������� ���������
	points_num = sequences_num * SEQUENCE_LEN;
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

	start_velocity = info.Vel;          // ��������� ��������
	accel = info.Accel;                           // ���������
	start_size = info.Size0;                    // ��������� ������
	start_size_range = info.SizeRnd;       // ��������� ����� ���������� �������
	size_factor = info.SizeRange;            // ��������� �������
	full = info.Full;                  // ���� true, �� ������ �����, ����� - ��������

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

// ����������
BombSparkles::~BombSparkles()
{
	delete[] sequences;
	//delete[] color_vector;
	delete[] points2d_vector;
	delete[] points_vector;
	//delete[] texcoord_vector;
	//delete[] ready_vector;
	// ����
	delete sound_object;
}

// ��������� ��� �������
// (������ ������ ���������)
bool BombSparkles::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }

	float dt; // ������� �� ������� ����� ������� �������� � �������� �������� �������
	dt = current_time - start_time;
	if (dt >= lifetime)
	{
		finished = true;
		return true;
	}

	for(int i = 0; i < sequences_num; i++) sequences[i].NextTick(dt);
	return true;
}

// � ������ ����� ������ ���������� ������
// ������� ��� ������� �������
bool BombSparkles::Start(const float start_time)
{
	// ��������� ����� ������ ���������� �������
	this->start_time = this->prev_time = start_time;

	// ������������������� ������������������
	for(int i = 0; i < sequences_num; i++)
		sequences[i].Rebirth(root_point, start_velocity, accel, start_size, start_size_range, lifetime, full);

	// ����������� ���������� ����������
	for(int i = 0; i < sequences_num*SEQUENCE_LEN; i++)
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
bool BombSparkles::Update()
{
	// ����
	sound_object->Update(root_point, vel);
	// ����������� ������ ��� ������������� (points_vector)
	for(int i = 0; i < sequences_num; i++)
	{
		for(int j = 0; j < SEQUENCE_LEN; j++)
		{
			points_vector[i*SEQUENCE_LEN + j] = sequences[i].sequence[j].coords;
		}
	}
	// ����� ������� ������� �������������
	pGraphPipe->TransformPoints(points_num, points_vector, points2d_vector);
	// ���������� �����
	Multiply();
	return true;
}

// ������� ����������� �����
void BombSparkles::Multiply()
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
	for(i = 0; i < sequences_num; i++) // �� �������������������
	{
		for(j = 0; j < SEQUENCE_LEN; j++) // �� ������ � ������������������
		{
			point3 xdelta = right * sequences[i].sequence[j].size * xfactor;
			point3 ydelta = up * sequences[i].sequence[j].size * yfactor;

			// ��������� ����������
			prim.Verts[k*PEAK_NUM + 0].pos = points2d_vector[l] - xdelta + ydelta;
			prim.Verts[k*PEAK_NUM + 1].pos = points2d_vector[l] + xdelta + ydelta;
			prim.Verts[k*PEAK_NUM + 2].pos = points2d_vector[l] + xdelta - ydelta;
			prim.Verts[k*PEAK_NUM + 3].pos = points2d_vector[l] - xdelta + ydelta;
			prim.Verts[k*PEAK_NUM + 4].pos = points2d_vector[l] + xdelta - ydelta;
			prim.Verts[k*PEAK_NUM + 5].pos = points2d_vector[l] - xdelta - ydelta;

		
			// ��������� ����
			prim.Verts[k*PEAK_NUM].diffuse = sequences[i].sequence[j].color;
			prim.Verts[k*PEAK_NUM + 1].diffuse = sequences[i].sequence[j].color;
			prim.Verts[k*PEAK_NUM + 2].diffuse = sequences[i].sequence[j].color;
			prim.Verts[k*PEAK_NUM + 3].diffuse = sequences[i].sequence[j].color;
			prim.Verts[k*PEAK_NUM + 4].diffuse = sequences[i].sequence[j].color;
			prim.Verts[k*PEAK_NUM + 5].diffuse = sequences[i].sequence[j].color;
		
			// �������� ���������� ������������ �����
			k++;
			// �������� ����� � ������� ����� ����� �������������
			l++;
		}
	}
}

// ���������� bounding box �������
BBox BombSparkles::GetBBox(const float current_time)
{
	BBox b;
	float dx, dy, dz, dt;
	dt = current_time - start_time;
	dx = start_velocity.x*dt + accel.x*dt*dt*0.5;
	dy = start_velocity.y*dt + accel.y*dt*dt*0.5;
	dz = start_velocity.z*dt + accel.z*dt*dt*0.5;
	b.Degenerate();
	b.Enlarge(root_point + point3(dx, dy, dz));
	b.Enlarge(root_point - point3(dx, dy, dz));
	return b;
}

/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ����� ���� �� ��������
				
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#include "precomp.h"

#include "../GraphPipe/GraphPipe.h"
#include "EffectInfo.h"
#include "ShootSmokeEffect.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// ����� ���� �� ��������
//
/////////////////////////////////////////////////////////////////////////////////////////////////

// ����������� � �����������
ShootSmokeEffect::ShootSmokeEffect(
								   const point3 StartPoint,   // ��������� �����
								   const point3 EndPoint,	  // �������� �����
								   const SHOOT_SMOKE_EFFECT_INFO& info
								   )
{
	points_num = info.PartNum;

	// ������� �������
	ready_vector_size = points_num*PEAK_NUM;
	ready_num = ready_vector_size;
	points_vector = new point3 [points_num];
	points2d_vector = new point3 [points_num];
	//color_vector = new unsigned int [ready_vector_size];
	//texcoord_vector = new texcoord [ready_vector_size];
	//ready_vector = new point3 [ready_vector_size];
	prim.Verts.resize(ready_vector_size);
	
	particles = new ShootSmokeParticle[points_num];

	root_point = StartPoint;                       // ��������� �����
	direction = Normalize(EndPoint - StartPoint);

	/*
	accel.x = info.Accel.x * direction.x;
	accel.y = info.Accel.y * direction.y;
	*/
	point3 tmp;
	tmp.x = direction.x;
	tmp.y = direction.y;
	tmp.z = 0;
	tmp = Normalize(tmp);
	accel = info.Accel.x*tmp;
	accel.z = info.Accel.z;

	for(int i = 0; i < points_num; i++)
	{
		/*
		particles[i].velocity.x = (info.SlowVel.x + (info.FastVel.x - info.SlowVel.x)*(float)rand()/32768.0)*direction.x;
		particles[i].velocity.y = (info.SlowVel.y + (info.FastVel.y - info.SlowVel.y)*(float)rand()/32768.0)*direction.y;
		*/
		particles[i].velocity = (info.SlowVel.x + (info.FastVel.x - info.SlowVel.x)*(float)rand()/32768.0)*tmp;

		particles[i].velocity.z = (info.SlowVel.z + (info.FastVel.z - info.SlowVel.z)*(float)rand()/32768.0);

		particles[i].size_vel = info.SlowSizeVel + (info.FastSizeVel - info.SlowSizeVel)*(float)rand()/32768.0;
	}
	
	lifetime = info.LifeTime;
	start_size = info.StartSize;

	tmp = root_point + info.FastVel.x*direction*lifetime + 0.5*accel;
	bbox.Degenerate();
	bbox.Enlarge(root_point);
	bbox.Enlarge(tmp);

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
ShootSmokeEffect::~ShootSmokeEffect()
{
	delete [] points_vector;
	delete [] points2d_vector;
	//delete [] color_vector;
	//delete [] texcoord_vector;
	//delete [] ready_vector;
	delete [] particles;
	// ����
	delete sound_object;
}

// ��������� ��� �������
// (������ ������ ���������)
bool ShootSmokeEffect::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }
	float dt; // ������� �� ������� ����� ������� �������� � �������� �������� �������
	dt = current_time - start_time;

	// ���� ����� ����� - �������� ������
	if(dt > lifetime)
	{
		finished = true;
		return true;
	}

	for(int i = 0; i < points_num; i++)
	{
		// ����������
		particles[i].coords = root_point + dt * particles[i].velocity + 0.5*dt*dt*accel;
		// ����
		int c = 255*(1 - dt/lifetime);
		color = RGBA_MAKE(c, c, c, c);
		// ������
		particles[i].size = start_size + dt*particles[i].size_vel;
	}
	return true;
}

// ���������� ������� ������� ����� � ������� ������ �����
bool ShootSmokeEffect::Update()
{
	// ����
	sound_object->Update(root_point, velocity);

	for(int i = 0; i < points_num; i++)
		points_vector[i] = particles[i].coords;

	// ����� ������� ������� �������������
	pGraphPipe->TransformPoints(points_num, points_vector, points2d_vector);
	// ���������� �����
	Multiply();

	return true;
}

// � ������ ����� ������ ���������� ������
// ������� ��� ������� �������
bool ShootSmokeEffect::Start(const float start_time)
{
	// ��������� ����� ������ ���������� �������
	this->start_time = start_time;

	// ����
	color = 0xffffffff;

	for(int i = 0; i < points_num; i++)
	{
		// ���������� ����������
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
void ShootSmokeEffect::Multiply()
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

	int ii = 0;
	ready_num = 0;
	for(int i = 0; i < points_num; i++)
	{
		point3 xdelta = right * particles[i].size * xfactor;
		point3 ydelta = up * particles[i].size * yfactor;

		prim.Verts[ii*PEAK_NUM + 0].pos = points2d_vector[i] - xdelta + ydelta;
		prim.Verts[ii*PEAK_NUM + 1].pos = points2d_vector[i] + xdelta + ydelta;
		prim.Verts[ii*PEAK_NUM + 2].pos = points2d_vector[i] + xdelta - ydelta;
		prim.Verts[ii*PEAK_NUM + 3].pos = points2d_vector[i] - xdelta + ydelta;
		prim.Verts[ii*PEAK_NUM + 4].pos = points2d_vector[i] + xdelta - ydelta;
		prim.Verts[ii*PEAK_NUM + 5].pos = points2d_vector[i] - xdelta - ydelta;

		prim.Verts[ii*PEAK_NUM].diffuse = color;
		prim.Verts[ii*PEAK_NUM + 1].diffuse = color;
		prim.Verts[ii*PEAK_NUM + 2].diffuse = color;
		prim.Verts[ii*PEAK_NUM + 3].diffuse = color;
		prim.Verts[ii*PEAK_NUM + 4].diffuse = color;
		prim.Verts[ii*PEAK_NUM + 5].diffuse = color;

		++ii;
		ready_num += PEAK_NUM;
	}
}

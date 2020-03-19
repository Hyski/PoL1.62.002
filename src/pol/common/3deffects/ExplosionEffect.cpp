/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ����� (������)
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#include "precomp.h"

#include "..\GraphPipe\GraphPipe.h"
#include "EffectInfo.h"
#include "ExplosionEffect.h"



//
// ����� ������
//


// ���������� ������� ������� ����� � ������� ������ �����
bool ExplosionEffect::Update()
{
	// ����
	sound_object->Update(root_point, start_velocity);
	// ����������� ������ ��� ������������� (points_vector)
	for(int i = 0; i < particles_num; i++)
	{
		points_vector[i] = particles[i].coords;
	}
	// ����� ������� ������� �������������
	pGraphPipe->TransformPoints(points_num, points_vector, points2d_vector);
	// ���������� �����
	Multiply();
	return true;
}



// ������� ����������� �����
void ExplosionEffect::Multiply()
{
	float f, n, betta;
	float xfactor;
	float yfactor;
	int xres, yres;
	int k;
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
	for(int i = 0; i<points_num; i++)
	{
		point3 xdelta = right * particles[i].size * xfactor;
		point3 ydelta = up * particles[i].size * yfactor;

		prim.Verts[k*PEAK_NUM + 0].pos = points2d_vector[i] - xdelta + ydelta;
		prim.Verts[k*PEAK_NUM + 1].pos = points2d_vector[i] + xdelta + ydelta;
		prim.Verts[k*PEAK_NUM + 2].pos = points2d_vector[i] + xdelta - ydelta;
		prim.Verts[k*PEAK_NUM + 3].pos = points2d_vector[i] - xdelta + ydelta;
		prim.Verts[k*PEAK_NUM + 4].pos = points2d_vector[i] + xdelta - ydelta;
		prim.Verts[k*PEAK_NUM + 5].pos = points2d_vector[i] - xdelta - ydelta;

		// ��������� ����
		prim.Verts[k*PEAK_NUM].diffuse = particles[i].color;
		prim.Verts[k*PEAK_NUM + 1].diffuse = particles[i].color;
		prim.Verts[k*PEAK_NUM + 2].diffuse = particles[i].color;
		prim.Verts[k*PEAK_NUM + 3].diffuse = particles[i].color;
		prim.Verts[k*PEAK_NUM + 4].diffuse = particles[i].color;
		prim.Verts[k*PEAK_NUM + 5].diffuse = particles[i].color;

		// �������� ���������� ������������ �����
		k++;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// ����� ������ �������
//////////////////////////////////////////////////////////////////////////////////////////////

// ����������� � �����������
GrenadeEffect::GrenadeEffect(const float quality,        // ��������
							 const point3 rp,            // �������� �����
							 const GRENADE_EFFECT_INFO& info
							 )
{
	// ������� ������ ������
	particles_num =  1 + 8*quality;
	particles = new ExplosionParticle[particles_num];

	// �������� �������� �����, ������ � ���������� ���������
	points_num = particles_num;
	ready_vector_size = ready_num = particles_num*PEAK_NUM;

	// ������� �������
	//color_vector = new unsigned int [ready_vector_size];
	points2d_vector = new point3 [points_num];
	points_vector = new point3 [points_num];
	//texcoord_vector = new texcoord [ready_vector_size];
	//ready_vector = new point3 [ready_vector_size];
	prim.Verts.resize(ready_vector_size);

	// ����, ������� ���������� ������ �� ��� ����� NextTick
	first_time = true;

	// �����, �� ������� ���� ���
	root_point = rp;

	start_velocity = info.Vel;          // ��������� ��������
	accel = info.Accel;                           // ���������
	start_size = info.Size0;                    // ��������� ������
	start_size_range = info.SizeRnd;       // ��������� ����� ���������� �������
	size_factor = info.SizeRange;            // ��������� �������

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
GrenadeEffect::~GrenadeEffect()
{
	delete[] particles;
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
bool GrenadeEffect::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }
	int c; // ����
	float s; // ������
	float dt; // ������� �� ������� ����� ������� �������� � �������� �������� �������
	dt = current_time - start_time;
	if (dt >= lifetime)
	{
		finished = true;
		return true;
	}

	c = (lifetime - dt)*(255.0/lifetime);
	c = RGBA_MAKE(c, (int)(c*0.7), (int)(c*0.7), (int)(c*0.7));

	s = start_size + sqrt(dt)*size_factor;

	// ����������� ����������, ���� � ������
	for(int i = 0; i < particles_num; i++)
	{
		// ����
		particles[i].color = c;
		
		// ������
		particles[i].size = s;

		// ���������� 
		particles[i].coords =  particles[i].start_point + particles[i].velocity*dt + accel*dt*dt/2.0;
	}
	return true;
}

// � ������ ����� ������ ���������� ������
// ������� ��� ������� �������
bool GrenadeEffect::Start(const float start_time)
{
	// ��������� ����� ������ ���������� �������
	this->start_time = start_time;

	// ������������������� �������
	for(int i = 0; i < particles_num; i++)	Rebirth(particles[i], start_time);

	// ����������� ���������� ����������
	for(int i = 0; i < particles_num; i++)
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

// ������� ����������� ������� ��� �� ��������
void GrenadeEffect::Rebirth(ExplosionParticle & ep, float )
{
	// ��� ��������� ���������
	float dx, dy, dz, a, b, r;
	// ��������� ����� ��������
	dx = (0.5 - (float)rand()/32768.0) * 0.9;
	dy = (0.5 - (float)rand()/32768.0) * 0.9;
	dz = (0.5 - (float)rand()/32768.0) * 0.5;
	ep.coords = root_point + point3(dx, dy, dz);
	ep.start_point = root_point + point3(dx, dy, dz);
	a = ((float)rand())/32768.0 * PIm2; // ��������� ���� �����
	b = ((float)rand())/32768.0 * PId2; // ��������� ���� �����

	ep.size = start_size + (float)rand()/32768.0*start_size_range;
	r = (0.8 + 0.2*(float)rand()/32768.0); // ��������� ������
	dx = r*cos(b)*cos(a) * start_velocity.x;
	dz = r*sin(b)  * start_velocity.z;
	dy = r*cos(b)*sin(a)  * start_velocity.y;
	// ����
	ep.color = 0xffffa0a0;
	ep.velocity = point3(dx, dy, dz);
}

// ���������� bounding box �������
BBox GrenadeEffect::GetBBox(const float current_time)
{
	BBox b;
	float dx, dy, dz, dt, s;
	dt = current_time - start_time;
	dx = start_velocity.x*dt + accel.x*dt*dt*0.5;
	dy = start_velocity.y*dt + accel.y*dt*dt*0.5;
	dz = start_velocity.z*dt + accel.z*dt*dt*0.5;
	s = start_size + sqrt(dt)*size_factor;
	b.maxx = root_point.x + dx + s;
	b.maxy = root_point.y + dy + s;
	b.maxz = root_point.z + dz + s;
	b.minx = root_point.x - dx - s;
	b.miny = root_point.y - dy - s;
	b.minz = root_point.z - dz - s;
	return b;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// ����� ������������� ���� ��� ��������
//////////////////////////////////////////////////////////////////////////////////////////////

// ����������� � �����������
SparkleEffect::SparkleEffect(
							 const point3 rp,            // �������� �����
							 const SPARKLE_EFFECT_INFO& info
	)
{
	// ������� ������ ������
	particles_num = info.PartNum;
	particles = new ExplosionParticle[particles_num];

	// �������� �������� �����, ������ � ���������� ���������
	points_num = particles_num;
	ready_vector_size = ready_num = particles_num*PEAK_NUM;

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

	lifetime = info.LifeTime;         // ������������ ����� �����
	full = info.Full;                    // ���� true, �� ������ �����, ����� - ��������
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
SparkleEffect::~SparkleEffect()
{
	delete[] particles;
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
bool SparkleEffect::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }
	int c; // ����
	float s; // ������
	float dt; // ������� �� ������� ����� ������� �������� � �������� �������� �������
	dt = current_time - start_time;

	if (dt >= lifetime)
	{
		finished = true;
		return true;
	}

	c = (lifetime - dt)*(255.0/lifetime);
	c = RGBA_MAKE(0xff, c, c, c);
	s = start_size + dt*size_factor;

	// ����������� ����������, ���� � ������
	for(int i = 0; i < particles_num; i++)
	{
		// ����
		particles[i].color = c;
		
		// ������
		particles[i].size = s;

		// ���������� 
		particles[i].coords = root_point + particles[i].velocity*dt + accel*dt*dt/2.0;
	}

	// ����� ���������� �����
	prev_time = current_time;
	return true;
}

// � ������ ����� ������ ���������� ������
// ������� ��� ������� �������
bool SparkleEffect::Start(const float start_time)
{
	// ��������� ����� ������ ���������� �������
	this->start_time = this->prev_time = start_time;

	// ������������������� �������
	for(int i = 0; i < particles_num; i++)	Rebirth(particles[i], start_time);

	// ����������� ���������� ����������
	for(int i = 0; i < particles_num; i++)
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

// ������� ����������� ������� ��� �� ��������
void SparkleEffect::Rebirth(ExplosionParticle & ep, float )
{
	// ��� ��������� ���������
	float dx, dy, dz, a, b, r;
	ep.coords = root_point;
	a = ((float)rand())/32768.0 * PIm2; // ��������� ���� �����
	// ��������� ���� �����
	if(full) b = ((float)rand())/32768.0 * PIm2;
	else b = ((float)rand())/32768.0 * PI;

	ep.size = start_size + (float)rand()/32768.0*start_size_range;
	r = (0.8 + 0.2*(float)rand()/32768.0); // ��������� ������
	dx = r*cos(b)*cos(a) * start_velocity.x;
	dz = r*sin(b)  * start_velocity.z;
	dy = r*cos(b)*sin(a)  * start_velocity.y;
	// ����
	ep.color = 0xffffffff;
	ep.velocity = point3(dx, dy, dz);
}

// ���������� bounding box �������
BBox SparkleEffect::GetBBox(const float current_time)
{
	BBox b;

	float dx, dy, dz, dt;
	dt = current_time - start_time;
	dx = start_velocity.x*dt + accel.x*dt*dt*0.5;
	dy = start_velocity.y*dt + accel.y*dt*dt*0.5;
	dz = start_velocity.z*dt + accel.z*dt*dt*0.5;
	b.maxx = root_point.x + dx;
	b.maxy = root_point.y + dy;
	b.maxz = root_point.z + dz;
	b.minx = root_point.x - dx;
	b.miny = root_point.y - dy;
	b.minz = root_point.z - dz;

	return b;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// ����� ���� ��� �������
/////////////////////////////////////////////////////////////////////////////////////////////////

// ����������� � �����������
ExplosionSmokeEffect::ExplosionSmokeEffect(const float quality,        // ��������
										   const point3 rp,            // �������� �����
										   const EXPLOSION_SMOKE_INFO& info
	)
{
	// ������� ������ ������
	particles_num = 1 + 4*quality;
	particles = new ExplosionParticle[particles_num];

	// �������� �������� �����, ������ � ���������� ���������
	points_num = particles_num;
	ready_vector_size = ready_num = particles_num*PEAK_NUM;

	// ������� �������
	//color_vector = new unsigned int [ready_vector_size];
	points2d_vector = new point3 [points_num];
	points_vector = new point3 [points_num];
	//texcoord_vector = new texcoord [ready_vector_size];
	//ready_vector = new point3 [ready_vector_size];
	prim.Verts.resize(ready_vector_size);

	// ����, ������� ���������� ������ �� ��� ����� NextTick
	first_time = true;

	// �����, �� ������� ���� ���
	root_point = rp;

	start_velocity = info.Vel;          // ��������� ��������
	accel = info.Accel;                           // ���������
	start_size = info.Size0;                    // ��������� ������
	start_size_range = info.SizeRnd;       // ��������� ����� ���������� �������
	size_factor = info.SizeRange;            // ��������� �������

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
ExplosionSmokeEffect::~ExplosionSmokeEffect()
{
	delete[] particles;
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
bool ExplosionSmokeEffect::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }
	int c; // ����
	float s; // ������
	float dt; // ������� �� ������� ����� ������� �������� � �������� �������� �������
	dt = current_time - this->start_time;
	if (dt >= this->lifetime)
	{
		finished = true;
		return true;
	}

	c = (lifetime - dt)*(255.0/lifetime);
	c = RGBA_MAKE(c, (int)(c*0.7), (int)(c*0.7), c);
	s = start_size + sqrt(dt)*size_factor;

	// ����������� ����������, ���� � ������
	for(int i = 0; i < particles_num; i++)
	{
		// ����
		particles[i].color = c;
		
		// ������
		particles[i].size = s;

		// ���������� 
		particles[i].coords =  particles[i].start_point + particles[i].velocity*dt + accel*dt*dt/2.0;
	}

	// ����� ���������� �����
	prev_time = current_time;
	return true;
}

// � ������ ����� ������ ���������� ������
// ������� ��� ������� �������
bool ExplosionSmokeEffect::Start(const float start_time)
{
	// ��������� ����� ������ ���������� �������
	this->start_time = this->prev_time = start_time;

	// ������������������� �������
	for(int i = 0; i < particles_num; i++)	Rebirth(particles[i], start_time);

	// ����������� ���������� ����������
	for(int i = 0; i < particles_num; i++)
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

// ������� ����������� ������� ��� �� ��������
void ExplosionSmokeEffect::Rebirth(ExplosionParticle & ep, float )
{
	// ��� ��������� ���������
	float dx, dy, dz, a, b, r;
	// ��������� ����� ��������
	dx = (0.5 - (float)rand()/32768.0) * 0.9;
	dy = (0.5 - (float)rand()/32768.0) * 0.9;
	dz = (0.5 - (float)rand()/32768.0) * 0.5;
	//ep.coords = root_point;// + point3(dx, dy, dz);
	ep.start_point = root_point;// + point3(dx, dy, dz);
	a = ((float)rand())/32768.0 * PIm2; // ��������� ���� �����
	b = ((float)rand())/32768.0 * PId2; // ��������� ���� �����

	ep.size = start_size + (float)rand()/32768.0*start_size_range;
	r = (0.8 + 0.2*(float)rand()/32768.0); // ��������� ������
	dx = r*cos(b)*cos(a) * start_velocity.x;
	dz = r*sin(b)  * start_velocity.z;
	dy = r*cos(b)*sin(a)  * start_velocity.y;
	// ����
	ep.color = 0xffffa0a0;
	ep.velocity = point3(dx, dy, dz);
}

// ���������� bounding box �������
BBox ExplosionSmokeEffect::GetBBox(const float current_time)
{
	BBox b;
	float dx, dy, dz, dt, s;
	dt = current_time - start_time;
	dx = start_velocity.x*dt + accel.x*dt*dt*0.5;
	dy = start_velocity.y*dt + accel.y*dt*dt*0.5;
	dz = start_velocity.z*dt + accel.z*dt*dt*0.5;
	s = start_size + sqrt(dt)*size_factor;
	b.maxx = root_point.x + dx + s;
	b.maxy = root_point.y + dy + s;
	b.maxz = root_point.z + dz + s;
	b.minx = root_point.x - dx - s;
	b.miny = root_point.y - dy - s;
	b.minz = root_point.z - dz - s;
	return b;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// ����� ������ ���� ��� �������
/////////////////////////////////////////////////////////////////////////////////////////////////

bool ExplosionSmokeRing::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }
	int c; // ����
	float dt; // ������� �� ������� ����� ������� �������� � �������� �������� �������
	dt = current_time - start_time;

	if (dt >= lifetime)
	{
		finished = true;
		return true;
	}

	// ����
	c = (lifetime - dt)*(255.0/lifetime);
	color = RGBA_MAKE(c, (int)(c*0.7), (int)(c*0.7), c);
	for(int i = 0; i < points_num; i++) prim.Verts[i].diffuse = color;
		
	// ������
	size = start_size + sqrt(dt)*size_factor;
	
	// ���������� ������
	prim.Verts[0].pos = root_point + point3(size, 0, 0);
	prim.Verts[1].pos = root_point + point3(0, size, 0);
	prim.Verts[2].pos = root_point + point3(-size, 0, 0);
	prim.Verts[3].pos = root_point + point3(0, -size, 0);
	return true;
}

// ���������� ������� ������� ����� � ������� ������ �����
bool ExplosionSmokeRing::Update()
{
	// ����
	sound_object->Update(root_point, start_velocity);
	return true;
}

// ����������� � �����������
ExplosionSmokeRing::ExplosionSmokeRing(
									   const point3 rp,            // �������� �����
									   const SMOKE_RING_INFO &info
									   )
{
	start_velocity = point3(0, 0, 0);
	// ������� ������ ������
	particles_num = 4;
	coords = new point3[particles_num];
	// �������� �������� �����, ������ � ���������� ���������
	points_num = particles_num;
	ready_vector_size = ready_num = particles_num;

	// ������� �������
	//color_vector = new unsigned int [ready_vector_size];
	points2d_vector = new point3 [points_num];
	points_vector = new point3 [points_num];
	//texcoord_vector = new texcoord [ready_vector_size];
	//ready_vector = new point3 [ready_vector_size];
	prim.Verts.resize(ready_vector_size);

	// ����, ������� ���������� ������ �� ��� ����� NextTick
	first_time = true;

	// ����� � ������� ��������� ������
	root_point = rp + point3(0, 0, 0.2);

	start_size = info.Size0;                    // ��������� ������
	size_factor = info.SizeRange;            // ��������� �������
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
ExplosionSmokeRing::~ExplosionSmokeRing()
{
	//delete[] color_vector;
	delete[] points2d_vector;
	delete[] points_vector;
	//delete[] texcoord_vector;
	//delete[] ready_vector;
	delete[] coords;
	// ����
	delete sound_object;
}

// � ������ ����� ������ ���������� ������
// ������� ��� ������� �������
bool ExplosionSmokeRing::Start(const float start_time)
{
	// ��������� ����� ������ ���������� �������
	this->start_time = start_time;

	color = 0xffffffff;

	// �������� ���������� ������
	for(int i = 0; i < particles_num; i++)
	{
		coords[i] = root_point;
	}

	// ����������� ���������� ����������
	prim.Verts[0].uv = texcoord(0, 0);
	prim.Verts[1].uv = texcoord(1, 0);
	prim.Verts[2].uv = texcoord(1, 1);
	prim.Verts[3].uv = texcoord(0, 1);
	return true;
}

// ������� ����������� �����
void ExplosionSmokeRing::Multiply()
{
}

// ���������� bounding box �������
BBox ExplosionSmokeRing::GetBBox(const float current_time)
{
	BBox b;

	float dt; // ������� �� ������� ����� ������� �������� � �������� �������� �������
	dt = current_time - this->start_time;
	// ������
	size = start_size + sqrt(dt)*size_factor;

	b.maxx = root_point.x + size;
	b.maxy = root_point.y + size;
	b.maxz = root_point.z + 0.01;
	b.minx = root_point.x - size;
	b.miny = root_point.y - size;
	b.minz = root_point.z - 0.01;

	return b;
}


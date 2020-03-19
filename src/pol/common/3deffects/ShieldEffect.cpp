/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ������� �������
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#include "precomp.h"

#include "..\GraphPipe\GraphPipe.h"
#include "EffectInfo.h"
#include "ShieldEffect.h"

// ������� ��� ���������� ������� ��������� ��������
namespace
{
	void setUnitVectors(point3* vectors, unsigned int num)
	{
		float alpha = 0;
		for(int i = 0; i < num; i++)
		{
			vectors[i] = point3(FastCos(alpha), FastSin(alpha), 1.0f);
			alpha += PIm2/num;
		}
	}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ����� ������� �������
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// ����������� � �����������
ShieldEffect::ShieldEffect(
						   const point3& Source,
						   const point3& Target,
						   const SHIELD_INFO& info
						   )
{

	// �������� �����
	root_point = Target;
	vel = point3(0, 0, 0);
	
	spot_num = info.PointsNum;        // ���������� ����� � ����������
	circles_num = info.CirclesNum;    // ���������� �����������
	dx = info.DX;                     // ���������� �� ��
	a = info.ParabolaFactor;          // ���������� � ��������
	lifetime = info.LifeTime;         // ����� ����� �������
	size = 0.02;

	// �������� �������� �����, ������ � ���������� ���������
	points_num = circles_num * spot_num;
	ready_vector_size = ready_num = points_num * PEAK_NUM;

	// ������� �������
	unit_vectors = new point3[spot_num];
	//color_vector = new unsigned int [ready_vector_size];
	points2d_vector = new point3 [points_num];
	points_vector = new point3 [points_num];
	//texcoord_vector = new texcoord [ready_vector_size];
	//ready_vector = new point3 [ready_vector_size];
	prim.Verts.resize(ready_vector_size);

	// ����, ������� ���������� ������ �� ��� ����� NextTick
	first_time = true;
	// ����, ����������� �� ������������� ���������� �������
	finished = false;

	direction = Normalize(Source - Target);

	bbox.Degenerate();
	bbox.Enlarge(root_point - point3(dx*circles_num, dx*circles_num, dx*circles_num));
	bbox.Enlarge(root_point + point3(dx*circles_num, dx*circles_num, dx*circles_num));

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
			sound_object = new FixedSound(info.Sound, Target);
		}
	}
}

// ����������
ShieldEffect::~ShieldEffect()
{
	delete [] unit_vectors;
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
bool ShieldEffect::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }

	float dt; // ������� �� ������� ����� ������� �������� � �������� �������� �������
	dt = current_time - start_time;
	if (dt >= lifetime)
	{
		finished = true;
		return true;
	}

	// �������� ����
	/*
	float factor = 1.0;
	if(dt < 0.1*lifetime) factor = 10.0f*dt/lifetime;
	if(dt > 0.2*lifetime) factor = 1.25f*(lifetime - dt)/lifetime;*/
	for(int i = 0; i < circles_num; i++) // ���� �� �����������
	{
		for(int j = 0; j < spot_num; j++) // ���� �� ������ � ����������
		{
			// �������� ����
			int c = (255 - 240.0f*i/circles_num);
			if(dt < 0.2*lifetime)
				c *= 5.0f*dt/lifetime;
			if(dt > 0.2*lifetime)
			{
				c = (255*i/circles_num);
				c *= 1.25f*(lifetime - dt)/lifetime;
			}
			
			unsigned int color = RGB_MAKE(c, c, c);
			prim.Verts[(i*spot_num + j)*PEAK_NUM].diffuse = color;
			prim.Verts[(i*spot_num + j)*PEAK_NUM + 1].diffuse = color;
			prim.Verts[(i*spot_num + j)*PEAK_NUM + 2].diffuse = color;
			prim.Verts[(i*spot_num + j)*PEAK_NUM + 3].diffuse = color;
			prim.Verts[(i*spot_num + j)*PEAK_NUM + 4].diffuse = color;
			prim.Verts[(i*spot_num + j)*PEAK_NUM + 5].diffuse = color;

		}
	}

	return true;
}

// ���������� bounding box �������
BBox ShieldEffect::GetBBox(const float )
{
	return bbox;
}

// ���������� ������� ������� ����� � ������� ������ �����
bool ShieldEffect::Update()
{
	// ����
	sound_object->Update(root_point, vel);
	// ����� ������� ������� �������������
	pGraphPipe->TransformPoints(points_num, points_vector, points2d_vector);
	// ���������� �����
	Multiply();
	return true;
}

// � ������ ����� ������ ���������� ������
// ������� ��� ������� �������
bool ShieldEffect::Start(const float start_time)
{
	// ��������� ����� ������ ���������� �������
	this->start_time = start_time;


	// �������� ������ ��������� ��������
	setUnitVectors(unit_vectors, spot_num);

	// �������� ������� ���������, ��������� � ���������
	point3 OZ = direction;
	point3 OY = Normalize(direction.Cross(point3(0.0f, 0.0f, 1.0f)));
	point3 OX = Normalize(OY.Cross(OZ));

	float xy_shift = dx;

	// ��������� ��� �����
	for(int i = 0; i < circles_num; i++) // ���� �� �����������
	{
		for(int j = 0; j < spot_num; j++) // ���� �� ������ � ����������
		{
			// ����� � ��������� ������� ���������
			point3 p = unit_vectors[j]*xy_shift;
			p.z *= xy_shift*a; 

			// ��������� � �������
			float x = p.x*OX.x + p.y*OY.x + p.z*OZ.x;
			float y = p.x*OX.y + p.y*OY.y + p.z*OZ.y;
			float z = p.x*OX.z + p.y*OY.z + p.z*OZ.z;

			points_vector[i*spot_num + j] = root_point + point3(x, y, z);

			// �������� ����
			int c = 255 - 240.0f*i/circles_num;
			unsigned int color = RGB_MAKE(c, c, c);
			prim.Verts[(i*spot_num + j)*PEAK_NUM].diffuse = color;
			prim.Verts[(i*spot_num + j)*PEAK_NUM + 1].diffuse = color;
			prim.Verts[(i*spot_num + j)*PEAK_NUM + 2].diffuse = color;
			prim.Verts[(i*spot_num + j)*PEAK_NUM + 3].diffuse = color;
			prim.Verts[(i*spot_num + j)*PEAK_NUM + 4].diffuse = color;
			prim.Verts[(i*spot_num + j)*PEAK_NUM + 5].diffuse = color;

		}
		xy_shift += dx;
	}


	// ����������� ���������� ����������
	for(int i = 0; i < points_num; i++)
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

// ������� ����������� �����
void ShieldEffect::Multiply()
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
	for(int i = 0; i < points_num; i++)
	{
		point3 xdelta = right * size * xfactor;
		point3 ydelta = up * size * yfactor;

		prim.Verts[k*PEAK_NUM + 0].pos = points2d_vector[i] - xdelta + ydelta;
		prim.Verts[k*PEAK_NUM + 1].pos = points2d_vector[i] + xdelta + ydelta;
		prim.Verts[k*PEAK_NUM + 2].pos = points2d_vector[i] + xdelta - ydelta;
		prim.Verts[k*PEAK_NUM + 3].pos = points2d_vector[i] - xdelta + ydelta;
		prim.Verts[k*PEAK_NUM + 4].pos = points2d_vector[i] + xdelta - ydelta;
		prim.Verts[k*PEAK_NUM + 5].pos = points2d_vector[i] - xdelta - ydelta;

		// �������� ���������� ������������ �����
		k++;
	}
}

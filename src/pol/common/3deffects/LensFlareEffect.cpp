/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: эффект lens flare
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                


#include "precomp.h"

#include "..\GraphPipe\GraphPipe.h"
#include "EffectInfo.h"
#include "LensFlareEffect.h"

//
// класс lens flare
//

// конструктор с параметрами
LensEffect::LensEffect(
					   const point3 StartPoint,    // начальная точка
					   const point3 EndPoint,	   // конечная точка
					   const LENS_FLARE_EFFECT_INFO& info
					   )
{
	// создание массивов точек, цветов и текстурных координат
	ready_num = PEAK_NUM;
	// создать массивы
	//color_vector = new unsigned int [PEAK_NUM];
	points2d_vector = new point3 [1];
	points_vector = new point3 [1];
	//texcoord_vector = new texcoord [PEAK_NUM];
	//ready_vector = new point3 [PEAK_NUM];
	prim.Verts.resize(PEAK_NUM);

	// флаг, который показывает первый ли это вызов NextTick
	first_time = true;

	start_point = StartPoint;                       // начальная точка
	end_point = EndPoint;                           // конечная точка

	// время жизни эффекта
	lifetime = (end_point - start_point).Length() / info.VelFactor;

	size = info.Size;                                    // размер

	distance = info.Distance;                            // расстояние до центра

	velocity = (end_point - start_point)/lifetime;  // скорость
	
	// флаг, указывающий на необходимость завершения эффекта
	finished = false;

	// шейдер
	SetShader(info.Shader);
	// звук
	if(info.Sound == "")
	{
		// звука нет
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

// деструктор
LensEffect::~LensEffect()
{
	//delete[] color_vector;
	delete[] points2d_vector;
	delete[] points_vector;
	//delete[] texcoord_vector;
	//delete[] ready_vector;
	// звук
	delete sound_object;
}

// следующий тик времени
// (только расчет координат)
bool LensEffect::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }

	float dt; // разница во времени между текущим временем и временем рождения эффекта
	dt = current_time - start_time;

	if (dt >= lifetime)
	{
		finished = true;
		return true;
	}

	// координаты частиц
	current_point = start_point + velocity*dt;
	return true;
}

// подготовка массива готовых точек и массива цветов точек
bool LensEffect::Update()
{
	// звук
	sound_object->Update(current_point, velocity);
	// подготовить массив для проецирования (points_vector)
	points_vector[0] = current_point;
	// вызов внешней функции проецирования
	pGraphPipe->TransformPoints(1, points_vector, points2d_vector);
	// размножаем точки
	Multiply();
	return true;
}

// возвращает bounding box эффекта
BBox LensEffect::GetBBox(const float)
{
	BBox b;
	float s = 1.0;
	b.Degenerate();
	b.Enlarge(current_point + point3(s, s, s));
	b.Enlarge(current_point - point3(s, s, s));
	return b;
}

// с вызова этого метода начинается отсчет
// времени для расчета эффекта
bool LensEffect::Start(const float start_time)
{
	// запомнить время начала проработки эффекта
	this->start_time = start_time;
	// прописываем текстурные координаты
	prim.Verts[0].uv = texcoord(0, 0);
	prim.Verts[1].uv = texcoord(1, 0);
	prim.Verts[2].uv = texcoord(1, 1);
	prim.Verts[3].uv = texcoord(0, 0);
	prim.Verts[4].uv = texcoord(1, 1);
	prim.Verts[5].uv = texcoord(0, 1);

	// z - координата
	prim.Verts[0].pos.z = 0;
	prim.Verts[1].pos.z = 0;
	prim.Verts[2].pos.z = 0;
	prim.Verts[3].pos.z = 0;
	prim.Verts[4].pos.z = 0;
	prim.Verts[5].pos.z = 0;

	// цвет
	color = 0xffffffff;
	prim.Verts[0].diffuse = color;
	prim.Verts[1].diffuse = color;
	prim.Verts[2].diffuse = color;
	prim.Verts[3].diffuse = color;
	prim.Verts[4].diffuse = color;
	prim.Verts[5].diffuse = color;
	// координаты частиц
	current_point = start_point;
	return true;
}

// функция размножения точек
void LensEffect::Multiply()
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

	point3 target = point3(points2d_vector[0].x, points2d_vector[0].y, 0.0);
	point3 center = point3(xres*0.5, yres*0.5, 0.0);
	point3 flare = center + (center - target)*distance;

	point3 xdelta = right * size * xfactor;
	point3 ydelta = up * size * yfactor;

	// размножим координаты
	prim.Verts[0].pos = flare - xdelta + ydelta;
	prim.Verts[1].pos = flare + xdelta + ydelta;
	prim.Verts[2].pos = flare + xdelta - ydelta;
	prim.Verts[3].pos = flare - xdelta + ydelta;
	prim.Verts[4].pos = flare + xdelta - ydelta;
	prim.Verts[5].pos = flare - xdelta - ydelta;

	ready_num = 6;
}

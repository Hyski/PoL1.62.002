/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: эффект динамического освещения
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                


#include "precomp.h"

#include "../GraphPipe/GraphPipe.h"
#include <undercover/gamelevel/GameLevel.h>
#include <undercover/iworld.h>
#include "EffectInfo.h"
#include "DLightEffect.h"
#include <logic2/TraceUtils.h>


//
// базовый класс эффекта динамического освещения
//

// конструктор
DLightEffect::DLightEffect()
{
	// флаг, который показывает первый ли это вызов NextTick
	first_time = true;
	vel = point3(0, 0, 0);  // скорость
	ready_num = 0;
	sound_object = new EmptySound();
}

// деструктор
DLightEffect::~DLightEffect()
{
	// звук
	delete sound_object;

	// динамическое освещение
	if(boost::shared_ptr<Mark> sp = d_light.lock())
	{
		sp->Dead = true;
	}
}

// подготовка массива готовых точек и массива цветов точек
bool DLightEffect::Update()
{
	// звук
	sound_object->Update(current_point, vel);

	if(boost::shared_ptr<Mark> sp = d_light.lock())
	{
		sp->Pos = current_point;
		sp->FindTris = true;
	}
	return true;
}

// возвращает bounding box эффекта
BBox DLightEffect::GetBBox(const float )
{
	BBox b;
	b.Degenerate();
	b.Enlarge(current_point - point3(100.1, 100.1, 100.1));
	b.Enlarge(current_point + point3(100.1, 100.1, 100.1));
	return b;
}


//
// класс динамического освещения, летящего по прямой
//

// конструктор с параметрами
LineDLightEffect::LineDLightEffect(
								   const point3& StartPoint,            // начальная скорость
								   const point3& EndPoint,              // конечная точка
								   const LINE_DLIGHT_EFFECT_INFO& info
								   )
{
	// флаг, который показывает первый ли это вызов NextTick
	first_time = true;
	// флаг, указывающий на необходимость завершения эффекта
	finished = false;

	start_point = StartPoint;
	end_point = EndPoint;
	color = info.Color;
	radius = info.Radius;
	current_point = start_point;

	// время жизни эффекта
	lifetime = (end_point - start_point).Length() / info.VelFactor;

	velocity = (end_point - start_point)/lifetime;  // скорость

	direction = Normalize(end_point - start_point);
}

// следующий тик времени
// (только расчет координат)
bool LineDLightEffect::NextTick(const float current_time)
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

// с вызова этого метода начинается отсчет
// времени для расчета эффекта
bool LineDLightEffect::Start(const float start_time)
{
	// запомнить время начала проработки эффекта
	this->start_time = start_time;

	// создание динамического источника света
	boost::shared_ptr<Mark> sp(new Mark(current_point, radius, "dynlight"));
	d_light = sp;
	sp->dynamic = true;
	sp->Col = color;

	IWorld::Get()->GetLevel()->AddMark(sp);

	return true;
}


//
// класс эффекта вспышки
//

// конструктор с параметрами
FlashEffect::FlashEffect(
						 const point3& Target,            // точка попадания
						 const FLASH_EFFECT_INFO& info
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

	// 
	root_point = Target;                       // начальная точка

	// время жизни эффекта
	lifetime = info.LifeTime;
	switch_time = info.SwitchTime;
	size_begin = info.SizeBegin;
	size_end = info.SizeEnd;

	// флаг, указывающий на необходимость завершения эффекта
	finished = false;

	bbox.Degenerate();
	bbox.Enlarge(point3(1000, 1000, 1000));
	bbox.Enlarge(point3(-1000, -1000, -1000));

	vel = point3(0, 0, 0);

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
			sound_object = new FixedSound(info.Sound, Target);
		}
	}
}

// деструктор
FlashEffect::~FlashEffect()
{
	// звук
	delete sound_object;
	//delete[] color_vector;
	//delete[] texcoord_vector;
	//delete[] ready_vector;
	delete[] points2d_vector;
	delete[] points_vector;
}

// следующий тик времени
// (только расчет координат)
bool FlashEffect::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }
	if(finished) return true;

	float dt; // разница во времени между текущим временем и временем рождения эффекта
	dt = current_time - start_time;

	if (dt >= lifetime)
	{
		finished = true;
		return true;
	}
	
	if(dt < switch_time)
	{
		// координаты частиц
		size = size_begin + (size_end - size_begin)*dt/switch_time;
	}
	else
	{
		// переключаем
		size = size_end;
		unsigned int c = 255*(1.0 - (dt - switch_time)/(lifetime - switch_time));
		color = RGB_MAKE(c, c, c);
	}

	return true;
}

// подготовка массива готовых точек и массива цветов точек
bool FlashEffect::Update()
{
	// звук
	sound_object->Update(root_point, vel);
	// подготовить массив для проецирования (points_vector)
	points_vector[0] = root_point;
	// вызов внешней функции проецирования
	pGraphPipe->TransformPoints(1, points_vector, points2d_vector);
	// размножаем точки
	Multiply();

	for(int i = 0; i < MAX_STAGES; ++i)
	{
		pGraphPipe->FindShader(GetShader())->ZFunc[i] = D3DCMP_ALWAYS;
	}

	return true;
}

// с вызова этого метода начинается отсчет
// времени для расчета эффекта
bool FlashEffect::Start(const float start_time)
{
	// проверим trace ray, если не видать, то эффект сразу убьем
	point3 camera = pGraphPipe->GetCam()->GetPos();
	// оттрассируем луч
	ShotTracer tr(0, camera,
		(root_point - camera),
    (root_point - camera).Length()-0.1,
	ShotTracer::F_SKIP_SHIELDS|ShotTracer::F_SKIP_INVISIBLE_ENTS|ShotTracer::F_SIGHT);
				
	if(tr.GetMaterial() != ShotTracer::MT_AIR)
	{
		// луч попал в какой-то объект => убиваем эффект
		finished = true;
		return true;
	}

	// запомнить время начала проработки эффекта
	this->start_time = start_time;

	// цвет
	for(int i = 0; i < ready_num; i++) prim.Verts[i].diffuse = 0xffffffff;

	// прописываем текстурные координаты
	prim.Verts[0].uv = texcoord(0, 1);
	prim.Verts[1].uv = texcoord(1, 1);
	prim.Verts[2].uv = texcoord(1, 0);
	prim.Verts[3].uv = texcoord(0, 1);
	prim.Verts[4].uv = texcoord(1, 0);
	prim.Verts[5].uv = texcoord(0, 0);

	prim.Verts[0].pos.z = 0;
	prim.Verts[1].pos.z = 0;
	prim.Verts[2].pos.z = 0;
	prim.Verts[3].pos.z = 0;
	prim.Verts[4].pos.z = 0;
	prim.Verts[5].pos.z = 0;

	color = 0xffffffff;
	
	return true;
}
	
// функция размножения точек
void FlashEffect::Multiply()
{
	float f, n, betta;
	float xfactor;
	float yfactor;
	int xres, yres;
	pGraphPipe->GetProjOptions(&f, &n, &betta);
	pGraphPipe->GetResolution(&xres, &yres);
	point3 right = pGraphPipe->GetCam()->GetRight();
	point3 up = pGraphPipe->GetCam()->GetUp();
	point3 dir = pGraphPipe->GetCam()->GetFront();
	point3 campos = pGraphPipe->GetCam()->GetPos();

	xfactor = PoLFXFactor;
	yfactor = PoLFXFactor;

	point3 xdelta = right * size * xfactor;
	point3 ydelta = up * size * yfactor;

	prim.Verts[0].pos = points2d_vector[0] - xdelta + ydelta;
	prim.Verts[1].pos = points2d_vector[0] + xdelta + ydelta;
	prim.Verts[2].pos = points2d_vector[0] + xdelta - ydelta;
	prim.Verts[3].pos = points2d_vector[0] - xdelta + ydelta;
	prim.Verts[4].pos = points2d_vector[0] + xdelta - ydelta;
	prim.Verts[5].pos = points2d_vector[0] - xdelta - ydelta;

	//for(int i = 0; i < 6; ++i)
	//{
	//	prim.Verts[i].pos -= dir * dir.Dot(prim.Verts[i].pos-campos);
	//	prim.Verts[i].pos += dir * n * 1.1f;
	//}

	prim.Verts[0].diffuse = color;
	prim.Verts[1].diffuse = color;
	prim.Verts[2].diffuse = color;
	prim.Verts[3].diffuse = color;
	prim.Verts[4].diffuse = color;
	prim.Verts[5].diffuse = color;

	ready_num = 6;
}

//
// класс эффекта многотекстурной вспышки
//

// функция размножения точек
void MultiTexFlashEffect::Multiply()
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

	// размножим координаты
	point3 xdelta = right * size * xfactor;
	point3 ydelta = up * size * yfactor;

	prim.Verts[0].pos = points2d_vector[0] - xdelta + ydelta;
	prim.Verts[1].pos = points2d_vector[0] + xdelta + ydelta;
	prim.Verts[2].pos = points2d_vector[0] + xdelta - ydelta;
	prim.Verts[3].pos = points2d_vector[0] - xdelta + ydelta;
	prim.Verts[4].pos = points2d_vector[0] + xdelta - ydelta;
	prim.Verts[5].pos = points2d_vector[0] - xdelta - ydelta;

	prim.Verts[0].diffuse = color;
	prim.Verts[1].diffuse = color;
	prim.Verts[2].diffuse = color;
	prim.Verts[3].diffuse = color;
	prim.Verts[4].diffuse = color;
	prim.Verts[5].diffuse = color;

	switch(phase)
	{
	case 0 : 
		{
			u1 = 0;
			v1 = 0;
			u2 = 0.5;
			v2 = 0.5;
			break;
		}
	case 1 : 
		{
			u1 = 0.5;
			v1 = 0;
			u2 = 1;
			v2 = 0.5;
			break;
		}
	case 2 :
		{
			u1 = 0.5;
			v1 = 0.5;
			u2 = 1;
			v2 = 1;
			break;
		}
	case 3 : 
		{
			u1 = 0;
			v1 = 0.5;
			u2 = 0.5;
			v2 = 1;
			phase = 0;
			break;
		}
	}

	phase++;
	// прописываем текстурные координаты
	prim.Verts[0].uv = texcoord(u1, v2);
	prim.Verts[1].uv = texcoord(u2, v2);
	prim.Verts[2].uv = texcoord(u2, v1);
	prim.Verts[3].uv = texcoord(u1, v2);
	prim.Verts[4].uv = texcoord(u2, v1);
	prim.Verts[5].uv = texcoord(u1, v1);

	ready_num = 6;
}

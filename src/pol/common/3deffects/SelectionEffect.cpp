/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: эффект выделения
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

//#define SPOT_EFFECT_DEBUG 1 // enable for log file

#include "precomp.h"

#include "..\GraphPipe\GraphPipe.h"
#include "EffectInfo.h"
#include "SelectionEffect.h"

//
// класс выделения
//

// конструктор с параметрами
SelectionEffect::SelectionEffect(
								 const point3 Target,        // центральная точка
								 const SELECTION_EFFECT_INFO &info
								 )
{
	visible = true;
	// корневая точка
	root_point = Target + point3(0.0f, 0.0f, 0.5f);

	// параметры
	radius = info.Radius;
	small_size = info.SmallSize;
	large_size = info.LargeSize;
	angle_velocity = info.AngVel;
	angle_delta = info.AngDelta;

	// размерности массивов
	points_num = 2 * info.PartNum;
	ready_vector_size = points_num * PEAK_NUM;
	ready_num = ready_vector_size;

	// создать массивы
	points_vector = new point3 [points_num];
	points2d_vector = new point3 [points_num];
	//color_vector = new unsigned int [ready_num];
	//texcoord_vector = new texcoord [ready_num];
	//ready_vector = new point3 [ready_num];
	prim.Verts.resize(ready_num);

	// флаг, который показывает первый ли это вызов NextTick
	first_time = true;
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
			sound_object = new FixedSound(info.Sound, Target);
		}
	}
}

// деструктор
SelectionEffect::~SelectionEffect()
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
bool SelectionEffect::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }
	if(!visible)
	{
		ready_num = 0;
		return true;
	}
	bbox.Degenerate();
	bbox.Enlarge(root_point - point3(radius+large_size, radius+large_size, 0.1));
	bbox.Enlarge(root_point + point3(radius+large_size, radius+large_size, 0.1));

	float dt; // разница во времени между текущим временем и временем рождения эффекта
	float dtt;


	dt = current_time - start_time;

	dtt=dt+0.37*sin(dt*PIm2);

	Quaternion qt;
	qt.FromAngleAxis(fmod(angle_velocity*dtt, PIm2), axis);

	current_vector = (qt * start_vector) * radius;

	// цвет
	unsigned int c = 200.0 + 55.0*sin(dt*PIm2);
	color = RGB_MAKE(c, c, c);

	return true;
}

// подготовка массива готовых точек и массива цветов точек
bool SelectionEffect::Update()
{
	if(!visible) return true;
	// звук
	sound_object->Update(root_point, velocity);
	// подготовить массив для проецирования (points_vector)
	for(int i = 0; i < points_num; i++)
	{
		Quaternion qt;
		qt.FromAngleAxis(fmodf((PIm2*i*2.0)/points_num, PIm2), axis);
		if(i % 2)
		{
			Quaternion qt1;
			qt1.FromAngleAxis(angle_delta, axis);
			points_vector[i] = root_point + (qt1 * (qt * current_vector));
		}
		else
		{
			points_vector[i] = root_point + (qt * current_vector);
		}
	}
	// вызов внешней функции проецирования
	pGraphPipe->TransformPoints(points_num, points_vector, points2d_vector);
	// размножаем точки
	Multiply();
	return true;
}

// с вызова этого метода начинается отсчет
// времени для расчета эффекта
bool SelectionEffect::Start(const float start_time)
{
	// запомнить время начала проработки эффекта
	this->start_time = start_time;
	start_vector = Normalize(point3(1, 1, 0));
	current_vector = start_vector;
	axis = point3(0, 0, 1.0);
	
	// цвет
	color = 0xffffffff;

	// прописываем текстурные координаты
	for(int i = 0; i < points_num; i++)
	{
		prim.Verts[i*PEAK_NUM].uv = texcoord(0, 1);
		prim.Verts[i*PEAK_NUM + 1].uv = texcoord(1, 1);
		prim.Verts[i*PEAK_NUM + 2].uv = texcoord(1, 0);
		prim.Verts[i*PEAK_NUM + 3].uv = texcoord(0, 1);
		prim.Verts[i*PEAK_NUM + 4].uv = texcoord(1, 0);
		prim.Verts[i*PEAK_NUM + 5].uv = texcoord(0, 0);

		// цвет
		prim.Verts[i*PEAK_NUM].diffuse = color;
		prim.Verts[i*PEAK_NUM + 1].diffuse = color;
		prim.Verts[i*PEAK_NUM + 2].diffuse = color;
		prim.Verts[i*PEAK_NUM + 3].diffuse = color;
		prim.Verts[i*PEAK_NUM + 4].diffuse = color;
		prim.Verts[i*PEAK_NUM + 5].diffuse = color;
	}

	return true;
}

// функция размножения точек
void SelectionEffect::Multiply()
{
	if(!visible) return;
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

	float smallfactor = small_size * xfactor;

	// сначала считаем, что все точки видны, и количество готовых вершин 
	// равно размеру массива, в котором они лежат
	ready_num = ready_vector_size;
	// текущее количество точек, которые размножены в готовые вершины
	k = 0;  
	for(int i = 0; i < points_num; i++)
	{
		point3 xdelta, ydelta;
		
		if(i%2)
		{
			xdelta = right * smallfactor;
			ydelta = up * smallfactor;
		}
		else
		{
			xdelta = right * smallfactor;
			ydelta = up * smallfactor;
		}

		prim.Verts[k*PEAK_NUM + 0].pos = points2d_vector[i] - xdelta + ydelta;
		prim.Verts[k*PEAK_NUM + 1].pos = points2d_vector[i] + xdelta + ydelta;
		prim.Verts[k*PEAK_NUM + 2].pos = points2d_vector[i] + xdelta - ydelta;
		prim.Verts[k*PEAK_NUM + 3].pos = points2d_vector[i] - xdelta + ydelta;
		prim.Verts[k*PEAK_NUM + 4].pos = points2d_vector[i] + xdelta - ydelta;
		prim.Verts[k*PEAK_NUM + 5].pos = points2d_vector[i] - xdelta - ydelta;
	
		// цвет
		prim.Verts[k*PEAK_NUM].diffuse = color;
		prim.Verts[k*PEAK_NUM + 1].diffuse = color;
		prim.Verts[k*PEAK_NUM + 2].diffuse = color;
		prim.Verts[k*PEAK_NUM + 3].diffuse = color;
		prim.Verts[k*PEAK_NUM + 4].diffuse = color;
		prim.Verts[k*PEAK_NUM + 5].diffuse = color;

		// увеличим количество размноженных точек
		k++;
	}
}


/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: эффект разрушения объектов
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#include "precomp.h"

#include "..\GraphPipe\GraphPipe.h"
#include "EffectInfo.h"
#include "Effect.h"
#include "DestructEffect.h"
#include <undercover/gamelevel/DynObject.h>
/*
#if defined (GLASS_DEBUG) //for log
CLog LOG_GLASS;
#endif // for log
*/

// вычисление площади треугольника по трем вершинам
static float TriangleSquare(const DestructObject::TriangleInf &tr)
{
	return ((tr.Points[1] - tr.Points[0]).Cross(tr.Points[2] - tr.Points[0])).Length()*0.5;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// класс, использующийся для отображения (унаследован от TexObject)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// конструктор
DestructObject::DestructObject(const float MaxSquare)
{
	PartNum = 0;
	max_square = MaxSquare;
}

// деструктор
DestructObject::~DestructObject()
{
}

// оператор присваивания для инициализации объекта
DestructObject& DestructObject::operator = (const DynObject &a)
{
	// уничтожим в нашем объекте все лишнее
	Release();

	// скопируем из предоставленного объекта части, которые необходимо копировать вручную
	PartNum = a.GetTexObject()->PartNum;
	for(int i = 0; i < PartNum; i++)
	{
		// 1. скопируем из предоставленного объекта в два списка
		TriangleList ready_triangles;
		TriangleList temp_triangles;
		CopyToList(a.GetTexObject()->Parts[i], (D3DMATRIX&)a.GetWorldMatrix(), ready_triangles, temp_triangles);
		// 2. преобразуем список разбив большие треугольники
		ReformList(ready_triangles, temp_triangles);
		// 3. скопируем из списка в новый объект
		// создаем новый SimpleTexturedObject (STObject)
		Parts[i] = new STObject;
		Parts[i]->MaterialName = a.GetTexObject()->Parts[i]->MaterialName;
		CopyFromList(Parts[i], ready_triangles);
	}
	return *this;
}

// конструктор копирования
DestructObject::DestructObject(const DestructObject &a)
{
	Release();
	TexObject::operator=(a);
}

// для копирования из предоставленного объекта в список
void DestructObject::CopyToList(STObject * stobj, D3DMATRIX& matr, TriangleList& ready_tr, TriangleList& temp_tr)
{
	TriangleInf tr;
	for(int i = 0; i < stobj->prim.Idxs.size();)
	{
		VectorMatrixMultiply( *(D3DVECTOR*)&tr.Normals[0],
			*(D3DVECTOR*)&stobj->prim.Verts[stobj->GetIndexesRO()[i]].norm,
			matr);
		VectorMatrixMultiply( *(D3DVECTOR*)&tr.Normals[1],
			*(D3DVECTOR*)&stobj->prim.Verts[stobj->GetIndexesRO()[i+1]].norm,
			matr);
		VectorMatrixMultiply( *(D3DVECTOR*)&tr.Normals[2],
			*(D3DVECTOR*)&stobj->prim.Verts[stobj->GetIndexesRO()[i+2]].norm,
			matr);

		PointMatrixMultiply( *(D3DVECTOR*)&tr.Points[0],
			*(D3DVECTOR*)&stobj->prim.Verts[stobj->GetIndexesRO()[i]].pos,
			matr);
		PointMatrixMultiply( *(D3DVECTOR*)&tr.Points[1],
			*(D3DVECTOR*)&stobj->prim.Verts[stobj->GetIndexesRO()[i+1]].pos,
			matr);
		PointMatrixMultiply( *(D3DVECTOR*)&tr.Points[2],
			*(D3DVECTOR*)&stobj->prim.Verts[stobj->GetIndexesRO()[i+2]].pos,
			matr);

		tr.uv[0] = stobj->prim.Verts[stobj->GetIndexesRO()[i]].uv;
		tr.uv[1] = stobj->prim.Verts[stobj->GetIndexesRO()[i+1]].uv;
		tr.uv[2] = stobj->prim.Verts[stobj->GetIndexesRO()[i+2]].uv;

		tr.square = TriangleSquare(tr);

		// если площадь большая - кладем во временный список, иначе - в конечный
		if(tr.square > max_square)
		{
			temp_tr.push_back(tr);
		}
		else
		{
			ready_tr.push_back(tr);
		}
		i += 3;
	}
}

// для копирования из списка в новый объект
void DestructObject::CopyFromList(STObject * stobj, const TriangleList& tlist)
{
	unsigned int pnum = 3 * tlist.size();
	unsigned int inum = pnum;
	//stobj->PntNum = 3 * tlist.size();
	//stobj->IdxNum = stobj->PntNum;

	stobj->prim.Verts.resize(pnum);
	stobj->prim.Idxs.resize(inum);

	int index = 0;
	TriangleList::const_iterator i, end;
	end = tlist.end();
	for(i = tlist.begin(); i != end; i++)
	{
		stobj->GetIndexesFull()[index] = index;
		stobj->prim.Verts[index].pos = (*i).Points[0];
		stobj->prim.Verts[index].norm = (*i).Normals[0];
		stobj->prim.Verts[index].uv = (*i).uv[0];
		index++;

		stobj->GetIndexesFull()[index] = index;
		stobj->prim.Verts[index].pos = (*i).Points[1];
		stobj->prim.Verts[index].norm = (*i).Normals[1];
		stobj->prim.Verts[index].uv = (*i).uv[1];
		index++;

		stobj->GetIndexesFull()[index] = index;
		stobj->prim.Verts[index].pos = (*i).Points[2];
		stobj->prim.Verts[index].norm = (*i).Normals[2];
		stobj->prim.Verts[index].uv = (*i).uv[2];
		index++;

	}
}

//=====================================================================================//
//                          static point3 GetPointAtFactor()                           //
//=====================================================================================//
static point3 GetPointAtFactor(const point3 &pt1, const point3 &pt2, float factor)
{
	return factor*pt1 + (1.0f - factor)*pt2;
}

//=====================================================================================//
//                           static float GetFloatAtFactor()                           //
//=====================================================================================//
static float GetFloatAtFactor(float f1, float f2, float factor)
{
	return f1*factor + (1.0f - factor) * f2;
}

// преобразование списока с разбиением больших треугольников
void DestructObject::ReformList(TriangleList& ready_tr, TriangleList& temp_tr)
{
/*	LOGG("DestructObject::ReformList: begin\n");
	LOGG("  ready_tr.size = %d, temp_tr.size = %d, max_square = %f\n", ready_tr.size(), temp_tr.size(), max_square);
	int n = 0;*/

	int s1 = temp_tr.size();

	while(!temp_tr.empty())
	{
		//LOGG("\t While %d\n", n);
		TriangleList::iterator i = temp_tr.begin();
		TriangleList::iterator end = temp_tr.end();
		while(i != temp_tr.end())
		{
			if(i->square < max_square)
			{
				// переносим треугольник из временного в конечный список
				//LOGG("\t Moving triangle: square = %f\n", (*i).square);

				ready_tr.push_front(*i);
				i = temp_tr.erase(i);

				//LOGG("\t ready_tr.size = %d, temp_tr.size = %d\n", ready_tr.size(), temp_tr.size());
			}	
			else
			{
				// разбиваем
				//LOGG("\t Breaking triangle: square = %f\n", (*i).square);

				float d = 0.0f;
				int l1 = 0, l2 = 0;

				for(int k = 0; k != 3; ++k)
				{
					const float nd = (i->Points[k] - i->Points[(k+1)%3]).SqrLength();
					if(nd > d)
					{
						d = nd;
						l1 = k;
					}
				}

				d = 0.0f;

				for(int k = 0; k != 3; ++k)
				{
					if(k == l1) continue;
					const float nd = (i->Points[k] - i->Points[(k+1)%3]).SqrLength();
					if(nd > d)
					{
						d = nd;
						l2 = k;
					}
				}

				float factor1 = 0.5f + ((float)rand() / (float)RAND_MAX) * 0.25f;
				float factor2 = 0.5f + ((float)rand() / (float)RAND_MAX) * 0.25f;

				point3 pnt1 = GetPointAtFactor(i->Points[l1], i->Points[(l1+1)%3],factor1);
				point3 pnt2 = GetPointAtFactor(i->Points[l2], i->Points[(l2+1)%3],factor2);
				point3 nrm1 = GetPointAtFactor(i->Normals[l1], i->Normals[(l1+1)%3],factor1);
				point3 nrm2 = GetPointAtFactor(i->Normals[l2], i->Normals[(l2+1)%3],factor2);
				float u1 = GetFloatAtFactor(i->uv[l1].u, i->uv[(l1+1)%3].u, factor1);
				float u2 = GetFloatAtFactor(i->uv[l2].u, i->uv[(l2+1)%3].u, factor2);
				float v1 = GetFloatAtFactor(i->uv[l1].v, i->uv[(l1+1)%3].v, factor1);
				float v2 = GetFloatAtFactor(i->uv[l2].v, i->uv[(l2+1)%3].v, factor2);
				texcoord uv1(u1,v1);
				texcoord uv2(u2,v2);

				TriangleInf tr1;

				tr1.Points[0] = pnt1;
				tr1.Normals[0] = nrm1;
				tr1.uv[0] = uv1;

				tr1.Points[1] = i->Points[(l1+2)%3];
				tr1.Normals[1] = i->Normals[(l1+2)%3];
				tr1.uv[1] = i->uv[(l1+2)%3];

				int idxv;

				if(l1 == (l2+1)%3)
				{
					idxv = (l1+1)%3;
				}
				else
				{
					idxv = l1;
				}

				tr1.Points[2] = i->Points[idxv];
				tr1.Normals[2] = i->Normals[idxv];
				tr1.uv[2] = i->uv[idxv];

				tr1.square = TriangleSquare(tr1);
				temp_tr.push_front(tr1);

				TriangleInf tr2;

				tr2.Points[0] = tr1.Points[1];
				tr2.Normals[0] = tr1.Normals[1];
				tr2.uv[0] = tr1.uv[1];

				tr2.Points[1] = pnt1;
				tr2.Normals[1] = nrm1;
				tr2.uv[1] = uv1;

				tr2.Points[2] = pnt2;
				tr2.Normals[2] = nrm2;
				tr2.uv[2] = uv2;

				tr2.square = TriangleSquare(tr2);
				temp_tr.push_front(tr2);

				TriangleInf tr3;

				tr3.Points[0] = pnt1;
				tr3.Normals[0] = nrm1;
				tr3.uv[0] = uv1;

				tr3.Points[1] = pnt2;
				tr3.Normals[1] = nrm2;
				tr3.uv[1] = uv2;

				int idx;

				if((l1+1)%3 == l2)
				{
					idx = l2;
				}
				else
				{
					idx = l1;
				}

				tr3.Points[2] = i->Points[idx];
				tr3.Normals[2] = i->Normals[idx];
				tr3.uv[2] = i->uv[idx];

				tr3.square = TriangleSquare(tr3);
				temp_tr.push_front(tr3);

				float sumsq = tr1.square + tr2.square + tr3.square;
				float oldsq = i->square;
				assert( fabsf(sumsq - oldsq) < 1e-4f );

				i = temp_tr.erase(i);

				//LOGG("\t ready_tr.size = %d, temp_tr.size = %d\n", ready_tr.size(), temp_tr.size());
			}
		}
	}
	//LOGG("DestructObject::ReformList: end\n");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// класс эффекта разбивающегося стекла
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// конструктор
GlassDestruct::GlassDestruct(const DynObject& DynObj,
							 const point3& ExpPos,
							 const GLASS_DESTRUCT_EFFECT_INFO& info) : BaseDestructEffect(info.MaxSquare)
{
	triangles = 0;
	sound_object = 0;
	destruct_object = DynObj;
	//save_object = DynObj;
	if(!destruct_object.PartNum) { finished = true; return;}
	direction = Normalize(destruct_object.GetBBox().GetCenter() - ExpPos);
	first_time = true;
	finished = false;
	accel = info.Accel;
	friction_factor = info.FrictionFactor;
	lifetime = info.LifeTime;

	// посчитаем количество всех треугольников в объекте
	triangles_num = 0;
	for(int i = 0; i < destruct_object.PartNum; i++)
	{
		triangles_num += destruct_object.Parts[i]->prim.Idxs.size() / 3;
	}

	//LOGG("GlassDestruct::GlassDestruct: triangles_num = %d\n", triangles_num);

	// создадим массив треугольников
	triangles = new TriangleDesc[triangles_num];
	// заполним его
	unsigned int index = 0;
	for(int i = 0; i < destruct_object.PartNum; i++)
	{
		for(int j = 0; j < destruct_object.Parts[i]->prim.Idxs.size();)
		{
			// из трех идущих подряд точек (вершин одного тр-ка) строим центр масс
			point3 tmp = destruct_object.Parts[i]->prim.Verts[j].pos;
			tmp += destruct_object.Parts[i]->prim.Verts[j + 1].pos;
			tmp += destruct_object.Parts[i]->prim.Verts[j + 2].pos;
			tmp = tmp / 3.0;
			triangles[index].center = tmp;
			triangles[index].nodes[0] = destruct_object.Parts[i]->prim.Verts[j].pos - tmp;
			triangles[index].nodes[1] = destruct_object.Parts[i]->prim.Verts[j + 1].pos - tmp;
			triangles[index].nodes[2] = destruct_object.Parts[i]->prim.Verts[j + 2].pos - tmp;

			tmp.x = info.VelDisp.x*(1.0f - (float)rand()/16384.0f);
			tmp.y = info.VelDisp.y*(1.0f - (float)rand()/16384.0f);
			tmp.z = info.VelDisp.z*(1.0f - (float)rand()/16384.0f);
			tmp = Normalize(direction + tmp);
			triangles[index].velocity = tmp * (info.MinVelFactor + (info.MaxVelFactor - info.MinVelFactor)*(float)rand()/32768.0);

			triangles[index].angle_velocity.x = info.MaxAngleVel*(1 - (float)rand()/16384.0f);
			triangles[index].angle_velocity.y = info.MaxAngleVel*(1 - (float)rand()/16384.0f);
			triangles[index].angle_velocity.z = info.MaxAngleVel*(1 - (float)rand()/16384.0f);

			index++;
			j += 3;
		}
	}

	// звук
	if(info.Sound == "")
	{
		// звука нет
		sound_object = new EmptySound();
	}
	else
	{
		sound_object = new FixedSound(info.Sound, triangles[0].center);
	}
}

// деструктор
GlassDestruct::~GlassDestruct()
{
	delete[] triangles;
	delete sound_object;
}

// следующий тик времени
void GlassDestruct::NextTick(const float time)
{
	if(finished) return;
	if(first_time)
	{
		start_time = time;
		prev_time = time;
		first_time = false;
	}

	// время, прошедшее с момента запуска эффекта
	float dt = time - start_time;
	if(dt > lifetime)
	{
		finished = true;
		return;
	}
	
	// время, прошедшее с момента предыдущего расчета координат
	float dtt = time - prev_time;
	
	for(unsigned int i = 0; i < triangles_num; i++)
	{
		// пересчитаем скорость
		triangles[i].velocity += accel*dtt;
		point3 friction = -triangles[i].velocity * (friction_factor*dtt);
		triangles[i].velocity += friction;
		//friction.z = 0;
		// переместим центры масс
		triangles[i].center += triangles[i].velocity*dtt;
		// пересчитаем текущие вектора от центра масс к вершинам
		Quaternion qt;
		for(int j = 0; j < 3; j++)
		{
			// поворот вокруг OX
			qt.FromAngleAxis(fmod((triangles[i].angle_velocity.x*dt), PIm2), point3(1.0, 0, 0));
			triangles[i].next_nodes[j] = qt * triangles[i].nodes[j];

			// поворот вокруг OY
			qt.FromAngleAxis(fmod((triangles[i].angle_velocity.y*dt), PIm2), point3(0, 1.0, 0));
			triangles[i].next_nodes[j] = qt * triangles[i].next_nodes[j];

			// поворот вокруг OZ
			qt.FromAngleAxis(fmod((triangles[i].angle_velocity.z*dt), PIm2), point3(0, 0, 1.0));
			triangles[i].next_nodes[j] = qt * triangles[i].next_nodes[j];
		}
	}

	// проапдейтим все вершины
	unsigned int index = 0;
	for(int i = 0; i < destruct_object.PartNum; i++)
	{
		for(int j = 0; j < destruct_object.Parts[i]->prim.Idxs.size();)
		{
			// заполняем по три точки, идущих подряд
			destruct_object.Parts[i]->prim.Verts[j].pos = triangles[index].center + triangles[index].next_nodes[0];
			destruct_object.Parts[i]->prim.Verts[j + 1].pos = triangles[index].center + triangles[index].next_nodes[1];
			destruct_object.Parts[i]->prim.Verts[j + 2].pos = triangles[index].center + triangles[index].next_nodes[2];

			index++;
			j += 3;
		}
	}

	// новое предыдущее время
	prev_time = time;
}

// отрисовать эффект
void GlassDestruct::Draw(GraphPipe * const graph_pipe)
{
#ifdef _HOME_VERSION
	graph_pipe->Chop(&destruct_object,"GlassPiece");
#else
	graph_pipe->Chop(&destruct_object);
#endif
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// класс менеджера эффектов разрушающихся объектов
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// конструктор
DestructEffectManager::DestructEffectManager()
{
}

// деструктор
DestructEffectManager::~DestructEffectManager()
{
	if(effects.empty()) return;

	EffectsList::iterator i;
	for(i = effects.begin(); i != effects.end(); i++)
	{
		delete (*i);
	}
	effects.clear();
}

// следующий тик времени
void DestructEffectManager::NextTick(const float time)
{
	if(effects.empty()) return;

	// вызовем тики у всех эффектов
	EffectsList::iterator i;
	EffectsList::const_iterator end = effects.end();
	for(i = effects.begin(); i != end; i++)
	{
		(*i)->NextTick(time);
	}

	// уничтожим отработавшие эффекты
	bool not_clear = true;
	while(not_clear)
	{
		// если список пуст - закончим очистку:
		if(effects.empty()) break;

		end = effects.end();
		for(i = effects.begin(); i != end; i++)
		{
			// если эффект отработал - удалим его
			if((*i)->IsFinished())
			{
				delete (*i);
				effects.erase(i);
				not_clear = true;
				break;
			}

			// если попался работающий эффект
			not_clear = false;
		}
	}
}

// отрисовка всех эффектов
void DestructEffectManager::Draw(GraphPipe * const graph_pipe)
{
	if(effects.empty()) return;

	// вызовем отрисовку у всех эффектов
	EffectsList::iterator i;
	EffectsList::const_iterator end = effects.end();
	for(i = effects.begin(); i != end; i++)
	{
		(*i)->Draw(graph_pipe);
	}
}

// временная функция для разбивания стекла
void DestructEffectManager::DestroyGlass(const DynObject& DynObj,
										 const point3& ExpPos,
										 const GLASS_DESTRUCT_EFFECT_INFO& info)
{
	BaseDestructEffect * bde = new GlassDestruct(DynObj, ExpPos, info);
	effects.push_back(bde);
}


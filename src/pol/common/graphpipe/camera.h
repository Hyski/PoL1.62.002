#if !defined(AFX_CAMERA_H__A2C89401_104C_11D4_81EA_0060520A50A9__INCLUDED_)
#define AFX_CAMERA_H__A2C89401_104C_11D4_81EA_0060520A50A9__INCLUDED_

/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description:Класс камеры. Отвечает за перемещение камеры,
               создание матриц видовой и проекционной.
               Также предоставляет дополнительные методы
               Например, для picking: (x,y) -> луч.

   Author: Grom 
   Creation: 12 апреля 2000
***************************************************************/                
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "culling.h"
#include <undercover/skin/keyanimation.h>

#include "CameraStrategy.h"

class GeneralGrid;
class KeyAnimation;

//=====================================================================================//
//                                    class Camera                                     //
//=====================================================================================//
class Camera
{
	typedef std::auto_ptr<CameraStrategy> HStrategy;
	typedef std::vector<CameraMoveType> CamStack_t;

	HStrategy m_strategies[cmtMoveTypeCount];
	CamStack_t m_camStack;

	CameraParams m_params;
	//point3 Pos,Dir,Right,Up; //ориентация камеры
	//Quaternion  Orient;      // ориентация камеры
	//D3DMATRIX View,Proj;     //матрицы отгруженные в графическую систему

	//float Near,Far,Fov,Aspect;

	float m_lastUpdateTime;
	bool m_viewMod, m_projMod;

	//point3 AimSpot;          //точка на сетке, куда смотрит камера
	//bool AimSpotDirty;
	//MOVE_TYPE Moving;

public:
	Frustum Cone;

private:
	void ConstructView();
	void CalcAimSpot();
	float GetOrientAngle();
	void UpdateFrustum();//обновить Frustum
	void Zoom(float val);   //(+)приближение/(-)удаление камеры
	void MoveBack(float time);

public:
	enum RotateAxis{SELF=0x10,AIMSPOT=0x20,RIGHT=0x01,Z=0x02};

	Camera();

	//void SetProjection(float Fov, float Near, float Far, float Aspect);

	/// Применить матрицы камеры к устройству
	void Apply(IDirect3DDevice7 *D3DDev);    

	/// Перевод точки на экране в направление
	point3 Pick(float x,float y) const;

	/// Поворот вокруг указанной оси
	void Rotate(float Angle, unsigned int Axis); 

	/// Смещение на указанное расстояние (z - управление приближением)
	void Move(const point3 &Delta); 

	/// Камера встает на рельсы сплайна
	void LinkToSpline(const KeyAnimation &spl, float Time);

	/// При необходимости обновляется положение камеры
	void Update(float tau); 

	/// Установить фокусировку на точке
	void SetDest(const point3 &dir, const point3 &dest);

	/// Установить позицию камеры
	void SetLocation(const point3 &p, const point3 &dir);

	/// Привязана ли камера к хексам
	bool IsLinked() const { return m_camStack.back() != cmtFree; }

	/// Смена фокуса камеры на указанную точку
	void FocusOn(const point3 &spot, float FocusTime = 0.2f);
	/// Смена фокуса камеры на указанную точку с указанным направлением
	void FocusOnEx(const point3 &spot, const point3 &dir, float FocusTime = 0.2f);

	/// Зацепка для сплайновой камеры. Как только она отработала вызывается этот метод
	void RemoveSplineCam();
	void RemovePersonBoneCam();

	/// Прицепить к сущности
	void AttachToEntity(unsigned int);
	/// Отцепить от сущности
	void DetachFromEntity();

	/// Возвращает текущий тип камеры
	CameraMoveType GetMoveType() const { return m_camStack.back(); }

	/// Получить положение камеры
	point3 GetPos() const { return point3(m_params.m_pos); }
	point3 GetFront() const { return point3(m_params.m_dir); }
	point3 GetUp() const { return point3(m_params.m_up); }
	point3 GetRight() const { return point3(m_params.m_right); }

	/// Изменилась ли позиция камеры с прошлого кадра
	bool IsViewChanged() const { return m_viewMod; }
	/// Изменилась ли проекция камеры с прошлого кадра
	bool IsProjChanged() const { return m_projMod; }

	/// Получить видовую матрицу камеры
	const mll::algebra::matrix3 &GetView() const { return m_params.m_view; }
	/// Получить матрицу проекции камеры
	const mll::algebra::matrix3 &GetProj() const {return m_params.m_proj;}

	const CameraParams &GetParams() const { return m_params; }
};

#endif // !defined(AFX_CAMERA_H__A2C89401_104C_11D4_81EA_0060520A50A9__INCLUDED_)

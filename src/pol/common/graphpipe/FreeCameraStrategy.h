#pragma once

#include "CameraStrategy.h"

//=====================================================================================//
//                              class FreeCameraStrategy                               //
//=====================================================================================//
class FreeCameraStrategy : public CameraStrategy
{
	mll::algebra::point3 m_aimSpot;			///< точка на сетке, куда смотрит камера
	//bool m_aimSpotDirty;
	//float m_rotz, m_rotx;		///< Поворот вокруг осей
	float m_dist;				///< Расстояние до целевой точки

public:
	FreeCameraStrategy();

	/// Обновить состояние камеры
	virtual void Update(float tau);
	/// Возвращает тип камеры
	virtual CameraMoveType GetType() const { return cmtFree; }

	/// Управление камерой
	void Move(const mll::algebra::vector3 &);
	void RotateX(float);
	void RotateZ(float);
	void RotateXAroundSelf(float);
	void RotateZAroundSelf(float);
	void Zoom(float);

	/// Установить позицию камеры
	void SetLocation(const mll::algebra::point3 &pos, const mll::algebra::vector3 &dir);

	void AcceptParams(const CameraParams &);

private:
	void CalcAimSpot();
	void ConstructView();
	void MoveBack(float tau);
};

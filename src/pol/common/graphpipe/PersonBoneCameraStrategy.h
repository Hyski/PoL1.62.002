#pragma once

#include "CameraStrategy.h"

class Camera;
class RealEntity;

//=====================================================================================//
//               class PersonBoneCameraStrategy : public CameraStrategy                //
//=====================================================================================//
class PersonBoneCameraStrategy : public CameraStrategy
{
	class Obs;
	friend class Obs;

	Camera *m_cam;
	std::auto_ptr<Obs> m_eobs;

    float m_angleX, m_angleZ;

public:
	PersonBoneCameraStrategy(Camera *);
	~PersonBoneCameraStrategy();

	/// Обновить состояние камеры
	virtual void Update(float tau);
	/// Возвращает тип камеры
	virtual CameraMoveType GetType() const { return cmtPersonBone; }

	/// Повернуть по оси Z
	void RotateZ(float phi);
	/// Повернуть по оси X
	void RotateX(float phi);

	/// Установить камеру на сущность
	void Reset(unsigned int);

	static RealEntity *m_realEntity;
};

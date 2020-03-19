#pragma once

#include "CameraStrategy.h"
#include <undercover/skin/keyanimation.h>

class Camera;
class FreeCameraStrategy;

//=====================================================================================//
//                             class SplineCameraStrategy                              //
//=====================================================================================//
class SplineCameraStrategy : public CameraStrategy
{
	//float m_starttime;//
	float m_lastUpdate;					///< Время последнего обновления
	KeyAnimation m_path;				///< Сплайн, по которому летит камера

	Camera *m_cam;						///< Сама камера
	FreeCameraStrategy *m_normalCam;	///< Обычная стратегия камеры
	
public:
	SplineCameraStrategy(Camera *, FreeCameraStrategy *);

	/// Обновить состояние камеры
	virtual void Update(float tau);
	/// Возвращает тип камеры
	virtual CameraMoveType GetType() const { return cmtSpline; }

	/// Установить новую кривую
	void Reset(const KeyAnimation &, float tau);

	/// Закончить путь камеры
	void Stop(bool toEndPoint);
};

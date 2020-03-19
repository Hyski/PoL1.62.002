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
	float m_lastUpdate;					///< ����� ���������� ����������
	KeyAnimation m_path;				///< ������, �� �������� ����� ������

	Camera *m_cam;						///< ���� ������
	FreeCameraStrategy *m_normalCam;	///< ������� ��������� ������
	
public:
	SplineCameraStrategy(Camera *, FreeCameraStrategy *);

	/// �������� ��������� ������
	virtual void Update(float tau);
	/// ���������� ��� ������
	virtual CameraMoveType GetType() const { return cmtSpline; }

	/// ���������� ����� ������
	void Reset(const KeyAnimation &, float tau);

	/// ��������� ���� ������
	void Stop(bool toEndPoint);
};

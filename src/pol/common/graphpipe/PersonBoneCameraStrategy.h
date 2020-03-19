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

	/// �������� ��������� ������
	virtual void Update(float tau);
	/// ���������� ��� ������
	virtual CameraMoveType GetType() const { return cmtPersonBone; }

	/// ��������� �� ��� Z
	void RotateZ(float phi);
	/// ��������� �� ��� X
	void RotateX(float phi);

	/// ���������� ������ �� ��������
	void Reset(unsigned int);

	static RealEntity *m_realEntity;
};

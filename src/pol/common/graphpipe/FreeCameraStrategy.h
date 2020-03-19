#pragma once

#include "CameraStrategy.h"

//=====================================================================================//
//                              class FreeCameraStrategy                               //
//=====================================================================================//
class FreeCameraStrategy : public CameraStrategy
{
	mll::algebra::point3 m_aimSpot;			///< ����� �� �����, ���� ������� ������
	//bool m_aimSpotDirty;
	//float m_rotz, m_rotx;		///< ������� ������ ����
	float m_dist;				///< ���������� �� ������� �����

public:
	FreeCameraStrategy();

	/// �������� ��������� ������
	virtual void Update(float tau);
	/// ���������� ��� ������
	virtual CameraMoveType GetType() const { return cmtFree; }

	/// ���������� �������
	void Move(const mll::algebra::vector3 &);
	void RotateX(float);
	void RotateZ(float);
	void RotateXAroundSelf(float);
	void RotateZAroundSelf(float);
	void Zoom(float);

	/// ���������� ������� ������
	void SetLocation(const mll::algebra::point3 &pos, const mll::algebra::vector3 &dir);

	void AcceptParams(const CameraParams &);

private:
	void CalcAimSpot();
	void ConstructView();
	void MoveBack(float tau);
};

#if !defined(AFX_CAMERA_H__A2C89401_104C_11D4_81EA_0060520A50A9__INCLUDED_)
#define AFX_CAMERA_H__A2C89401_104C_11D4_81EA_0060520A50A9__INCLUDED_

/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description:����� ������. �������� �� ����������� ������,
               �������� ������ ������� � ������������.
               ����� ������������� �������������� ������
               ��������, ��� picking: (x,y) -> ���.

   Author: Grom 
   Creation: 12 ������ 2000
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
	//point3 Pos,Dir,Right,Up; //���������� ������
	//Quaternion  Orient;      // ���������� ������
	//D3DMATRIX View,Proj;     //������� ����������� � ����������� �������

	//float Near,Far,Fov,Aspect;

	float m_lastUpdateTime;
	bool m_viewMod, m_projMod;

	//point3 AimSpot;          //����� �� �����, ���� ������� ������
	//bool AimSpotDirty;
	//MOVE_TYPE Moving;

public:
	Frustum Cone;

private:
	void ConstructView();
	void CalcAimSpot();
	float GetOrientAngle();
	void UpdateFrustum();//�������� Frustum
	void Zoom(float val);   //(+)�����������/(-)�������� ������
	void MoveBack(float time);

public:
	enum RotateAxis{SELF=0x10,AIMSPOT=0x20,RIGHT=0x01,Z=0x02};

	Camera();

	//void SetProjection(float Fov, float Near, float Far, float Aspect);

	/// ��������� ������� ������ � ����������
	void Apply(IDirect3DDevice7 *D3DDev);    

	/// ������� ����� �� ������ � �����������
	point3 Pick(float x,float y) const;

	/// ������� ������ ��������� ���
	void Rotate(float Angle, unsigned int Axis); 

	/// �������� �� ��������� ���������� (z - ���������� ������������)
	void Move(const point3 &Delta); 

	/// ������ ������ �� ������ �������
	void LinkToSpline(const KeyAnimation &spl, float Time);

	/// ��� ������������� ����������� ��������� ������
	void Update(float tau); 

	/// ���������� ����������� �� �����
	void SetDest(const point3 &dir, const point3 &dest);

	/// ���������� ������� ������
	void SetLocation(const point3 &p, const point3 &dir);

	/// ��������� �� ������ � ������
	bool IsLinked() const { return m_camStack.back() != cmtFree; }

	/// ����� ������ ������ �� ��������� �����
	void FocusOn(const point3 &spot, float FocusTime = 0.2f);
	/// ����� ������ ������ �� ��������� ����� � ��������� ������������
	void FocusOnEx(const point3 &spot, const point3 &dir, float FocusTime = 0.2f);

	/// ������� ��� ���������� ������. ��� ������ ��� ���������� ���������� ���� �����
	void RemoveSplineCam();
	void RemovePersonBoneCam();

	/// ��������� � ��������
	void AttachToEntity(unsigned int);
	/// �������� �� ��������
	void DetachFromEntity();

	/// ���������� ������� ��� ������
	CameraMoveType GetMoveType() const { return m_camStack.back(); }

	/// �������� ��������� ������
	point3 GetPos() const { return point3(m_params.m_pos); }
	point3 GetFront() const { return point3(m_params.m_dir); }
	point3 GetUp() const { return point3(m_params.m_up); }
	point3 GetRight() const { return point3(m_params.m_right); }

	/// ���������� �� ������� ������ � �������� �����
	bool IsViewChanged() const { return m_viewMod; }
	/// ���������� �� �������� ������ � �������� �����
	bool IsProjChanged() const { return m_projMod; }

	/// �������� ������� ������� ������
	const mll::algebra::matrix3 &GetView() const { return m_params.m_view; }
	/// �������� ������� �������� ������
	const mll::algebra::matrix3 &GetProj() const {return m_params.m_proj;}

	const CameraParams &GetParams() const { return m_params; }
};

#endif // !defined(AFX_CAMERA_H__A2C89401_104C_11D4_81EA_0060520A50A9__INCLUDED_)

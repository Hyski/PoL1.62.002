#pragma once

//=====================================================================================//
//                                 struct CameraParams                                 //
//=====================================================================================//
struct CameraParams
{
	mll::algebra::point3 m_pos;
	mll::algebra::vector3 m_dir, m_right, m_up;
	float m_near, m_far, m_fov, m_aspect;

	mll::algebra::matrix3 m_view, m_invview;
	mll::algebra::matrix3 m_proj;

	CameraParams();

	void SetProjParams(float dnear, float dfar, float aspect);

	static float getNearPersonPlane();
	static float getNearFreePlane();
	static float getFarPlane();
};

//=====================================================================================//
//                                 enum CameraMoveType                                 //
//=====================================================================================//
enum CameraMoveType
{
	cmtFree,
	cmtSpline,
	cmtPersonBone,
	cmtPerson,
	cmtMoveTypeCount // не тип стратегии камеры
};

//=====================================================================================//
//                                class CameraStrategy                                 //
//=====================================================================================//
class CameraStrategy
{
protected:
	CameraParams m_params;

public:
	CameraStrategy();
	virtual ~CameraStrategy() {}

	/// Обновить состояние камеры
	virtual void Update(float tau) = 0;
	/// Возвращает тип камеры
	virtual CameraMoveType GetType() const = 0;
	/// Получить параметры камеры
	const CameraParams &GetParams() const { return m_params; }

protected:
	void ConstructView()
	{
		m_params.m_view = mll::algebra::matrix3::identity();

		m_params.m_view._00 = m_params.m_right.x;
		m_params.m_view._10 = m_params.m_right.y;
		m_params.m_view._20 = m_params.m_right.z;

		m_params.m_view._01 = m_params.m_up.x;
		m_params.m_view._11 = m_params.m_up.y;
		m_params.m_view._21 = m_params.m_up.z;

		m_params.m_view._02 = m_params.m_dir.x;
		m_params.m_view._12 = m_params.m_dir.y;
		m_params.m_view._22 = m_params.m_dir.z;

		m_params.m_view._30 = -m_params.m_pos.dot(m_params.m_right);
		m_params.m_view._31 = -m_params.m_pos.dot(m_params.m_up);
		m_params.m_view._32 = -m_params.m_pos.dot(m_params.m_dir);

		m_params.m_invview = m_params.m_view;
		m_params.m_invview.invert();
	}
};

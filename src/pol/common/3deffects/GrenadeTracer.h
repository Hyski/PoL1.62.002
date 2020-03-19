/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ������ �������� �������
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined(__GRENADE_TRACER_H__)
#define __GRENADE_TRACER_H__

#include "Effect.h"
#include "Particle.h"

#include <undercover/skin/skanim.h>

//
// ������ �������� �������
//

class GrenadeTracer : public BaseEffect
{
public:

	// �������� ��� ������� (� ����� ������ �����������)
	virtual EFFECT_TYPE GetType() const { return EFFECT_TYPE::_2D; }

	// ����������� � �����������
	GrenadeTracer(
		const KeyAnimation& ka,
		const std::string& skin_name,
		float angle_vel
		);

	// ����������
	virtual ~GrenadeTracer();

	// ��������� ��� �������
	// (������ ������ ���������)
	virtual bool NextTick(const float current_time);

	// ���������� ������� ������� ����� � ������� ������ �����
	virtual bool Update();

	// ������ �������� ���� ������?
	virtual bool IsFinished() const { return finished; }

	// ���������� bounding box �������
	virtual BBox GetBBox(const float current_time);
protected:
	virtual bool Start(const float ) { return true; };

	//
	// ������� ������ �������
	//

	BBox bbox;
	float start_time;                         // ����� ������ �������
	float lifetime;                           // ����� ����� �������

	point3 current_point;
	KeyAnimation key_animation;
	SkAnim* m_skeleton;    // ������
	SkSkin* m_skin;           // ����
	AnimaData m_animadata;    // ���������� � �����������
	point3 m_angle_vel;
	point3 m_angles;

	// ����, ����������� �� ������������� ���������� �������
	bool finished;
};

//
// ������ �������� ������
//

class ShellTracer : public BaseEffect
{
public:

	// �������� ��� ������� (� ����� ������ �����������)
	virtual EFFECT_TYPE GetType() const { return EFFECT_TYPE::_2D; }

	// ����������� � �����������
	ShellTracer(
		const unsigned int entity_id,
		const point3& source,
		const point3& target,
		const std::string& skin_name,
		float angle_vel,
		float vel = 2.0f
		);

	// ����������
	virtual ~ShellTracer();

	// ��������� ��� �������
	// (������ ������ ���������)
	virtual bool NextTick(const float current_time);

	// ���������� ������� ������� ����� � ������� ������ �����
	virtual bool Update();

	// ������ �������� ���� ������?
	virtual bool IsFinished() const { return finished; }

	// ���������� bounding box �������
	virtual BBox GetBBox(const float current_time);
protected:
	virtual bool Start(const float ) { return true; };

	//
	// ������� ������ �������
	//

	BBox bbox;
	float start_time;                         // ����� ������ �������
	float lifetime;                           // ����� ����� �������

	point3 current_point;
	KeyAnimation key_animation;
	SkAnim* m_skeleton;    // ������
	SkSkin* m_skin;           // ����
	AnimaData m_animadata;    // ���������� � �����������
	point3 m_angle_vel;
	point3 m_angles;

	// ����, ����������� �� ������������� ���������� �������
	bool finished;
};

#endif // !defined(__GRENADE_TRACER_H__)

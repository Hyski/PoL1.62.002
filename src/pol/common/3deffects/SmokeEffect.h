/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ��� (������)
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined(__SMOKEEFFECT_H__)
#define __SMOKEEFFECT_H__

#include "Effect.h"
#include "Particle.h"

//
// ����� �������, ������������ ��� ����
//

class SmokeParticle : public BaseParticle
{
public:
	// �������� �������
	point3 velocity;
	// ����� �������� �������
	float birth_time;
	// ���������� ���������� �������
	texcoord texcoords1;
	texcoord texcoords2;

	// ����� ����� �������
	float life_time;
};

//
// ����� ����
//

class SmokeEffect : public BaseEffect  
{
public:

	// ��������� ��� �������
	// (������ ������ ���������)
	virtual bool NextTick(const float current_time);

	// ���������� ������� ������� ����� � ������� ������ �����
	virtual bool Update();

	// ���������� bounding box �������
	virtual BBox GetBBox(const float current_time);

	// ����������� � �����������
	SmokeEffect(
		const float quality,            // ��������
		const point3 rp,                // �������� �����
		const SMOKE_EFFECT_INFO& info
		);

	// ����������
	virtual ~SmokeEffect();

	// ������ �������� ���� ������?
	// (�� ��������� ������ �������� ����������)
	virtual bool IsFinished() const { return finished; }

	// ��������� ������� � ������ ������ ����� ������
	virtual void NewTurn() { turns_left--; }
protected:
	// � ������ ����� ������ ���������� ������
	// ������� ��� ������� �������
	virtual bool Start(const float start_time);

	//
	// ������� ������ �������
	//

	// ���������� ���������� ����� �� ���������� �������
	int turns_left;
	// ���� ������ ������� �������
	bool reduce;
	// ����� ������ �������
	float start_reduce_time;
	// ����� �������
	float reduce_time;

	// ������ ������
	SmokeParticle * particles;
	// ���������� ������ � �������
	unsigned int particles_num;

	// �����, �� ������� ���� ���
	point3 root_point;
	// ��������� ��������� ��������� ����� - ����� �������� ������
	float start_point_factor;
	// ��������
	point3 start_velocity;
	// ���������
	point3 accel;
	// ����������� ��������� ������
	float min_start_size;
	// ������ ��������� ���������o �������
	float size_range;
	// ����������� ��������� ������� ������� � ����������� �� �������
	float size_factor;
	// ������������ ����� ����� �������
	float max_lifetime;
	// ���� ��������� ������
	bool finished;
	// ���� ��������� ������� �� ����������
	bool m_exp;

	// ������� ����������� �����
	void Multiply();
	// ������� ����������� ������� ��� �� ��������
	void Rebirth(SmokeParticle & sp, float curtime, float disp);
};

//
// ����� �������� ����
//

class HeavySmokeEffect : public BaseEffect  
{
public:

	// ��������� ��� �������
	// (������ ������ ���������)
	virtual bool NextTick(const float current_time);

	// ���������� ������� ������� ����� � ������� ������ �����
	virtual bool Update();

	// ���������� bounding box �������
	virtual BBox GetBBox(const float current_time);

	// ����������� �� ���������
	HeavySmokeEffect();

	// ����������� � �����������
	HeavySmokeEffect(
		const float quality,            // ��������
		const point3 rp,                // �������� �����
		const float st_point_factor,    // ������� ������ ��������� �����
		const point3 stVelocity,        // ��������� ��������
		const point3 Accel,             // ���������
		const float stSIZE,             // ��������� ������
		const float stRANGE,            // ��������� ����� ���������� �������
		const float sizefactor,         // ������� �������
		const float maxlifetime,        // ������������ ����� �����
		const int MaxPartNum,           // ������������ ���������� ������
		const std::string& Shader,      // ��� �������
		const std::string& Sound,       // ��� ��������� �������
		const bool SoundMove            // ��������� �� ����
		);

	// ����������
	virtual ~HeavySmokeEffect();
protected:
	// � ������ ����� ������ ���������� ������
	// ������� ��� ������� �������
	virtual bool Start(const float start_time);

	//
	// ������� ������ �������
	//

	// ������ ������
	SmokeParticle * particles;
	// ���������� ������ � �������
	unsigned int particles_num;

	// �����, �� ������� ���� ���
	point3 root_point;
	// ��������� ��������� ��������� ����� - ����� �������� ������
	float start_point_factor;
	// ����� ������ ���������� �������
	float prev_time;
	// ��������
	point3 start_velocity;
	// ���������
	point3 accel;
	// ����������� ��������� ������
	float min_start_size;
	// ������ ��������� ���������o �������
	float size_range;
	// ����������� ��������� ������� ������� � ����������� �� �������
	float size_factor;
	// ������������ ����� ����� �������
	float max_lifetime;

	// ������� ����������� �����
	void Multiply();
	// ������� ����������� ������� ��� �� ��������
	void Rebirth(SmokeParticle & sp, float curtime, float disp);
};


//
// ����� �������, ������������ ��� ���
//

class FlyParticle : public BaseParticle
{
public:
	// ����������� ����������:
	// ������
	float r;
	// ���� �����
	float alpha;
	// ���� �����
	float betta;


	// ���� ����� (��� ��������� ������� �� ������)
	float gamma;
	// ������� �������� ��� ���������
	point3 vel;
	// ������� ������
	float radius;
	// ������� ��������� ���������
	void setCoords(const point3& root)
	{
		coords.x = r * cos(betta);
		coords.y = r * sin(betta);
		coords.z = r * sin(alpha);
		coords += root;
	}
};


//
// ����� ���
//

class FlyEffect : public BaseEffect  
{
public:

	// ��������� ��� �������
	// (������ ������ ���������)
	virtual bool NextTick(const float current_time);

	// ���������� ������� ������� ����� � ������� ������ �����
	virtual bool Update();

	// ���������� bounding box �������
	virtual BBox GetBBox(const float current_time);

	// ����������� � �����������
	FlyEffect(
		const float quality,            // ��������
		const point3 rp,                // �������� �����
		const FLY_EFFECT_INFO& info
		);

	// ����������
	virtual ~FlyEffect();

protected:
	// � ������ ����� ������ ���������� ������
	// ������� ��� ������� �������
	virtual bool Start(const float start_time);

	//
	// ������� ������ �������
	//


	// ������ ������
	FlyParticle * particles;
	// ���������� ������ � �������
	unsigned int particles_num;
	// ����� ����������� ����
	float previous_time;

	// ����������� �����
	point3 root_point;
	// ����������� ��������� ������
	float min_start_size;
	// ������ ��������� ���������o �������
	float size_range;

	float min_radius, radius_range;
	point3 min_vel;

	point3 vel;
	BBox bb;

	// ������� ����������� �����
	void Multiply();
};

//
// ����� �������, ������������ ��� ���
//

class FlyCylinderParticle : public BaseParticle
{
public:
	// �������������� ���������� (������ ����������):
	// ������
	float h;
	// ���� �����
	float alpha;
	// �������� �� �������
	float vh;
	// �������� �� ����
	float valpha;
	// �������� � �������� �������
	float delay;
	// ������� ��������� ���������
	void setCoords(const point3& root, float r)
	{
		coords.x = r * cos(alpha);
		coords.y = r * sin(alpha);
		coords.z = h;
		coords += root;
	}
};

//
// ����� ��� � ��������, �������� �� ��������
//

class FlyCylinderEffect : public BaseEffect  
{
public:
	// ����������� � �����������
	FlyCylinderEffect(
		const float quality,            // ��������
		const point3 rp,                // �������� �����
		const FLY_CYLINDER_EFFECT_INFO& info
		);

	// ����������
	virtual ~FlyCylinderEffect();

	// ��������� ��� �������
	// (������ ������ ���������)
	virtual bool NextTick(const float current_time);

	// ���������� ������� ������� ����� � ������� ������ �����
	virtual bool Update();

	// ���������� bounding box �������
	virtual BBox GetBBox(const float current_time) { return m_bbox; }

protected:
	// � ������ ����� ������ ���������� ������
	// ������� ��� ������� �������
	virtual bool Start(const float start_time);

	//
	// ������� ������ �������
	//
	BBox m_bbox;
	// ������ �������
	float m_height;
	// ���������� �������� ������
	unsigned int m_particles_num;
	// �������� �� ������� ����� ���������� ������
	float m_deltat;
	// ������ �������� ������
	float m_part_size;
	// ������ �������
	float m_radius;
	// ���������� ������ � ������
	unsigned int m_tail_part_num;
	// �������� ������
	float m_velh;
	float m_velalpha;
	// �������� �����
	point3 m_root;
	// ������ ������
	typedef std::vector<FlyCylinderParticle> ParticlesVector_t;
	ParticlesVector_t m_particles;
	ParticlesVector_t m_not_delayed_particles;
	// ����� ����������� ����
	float m_previous_time;
	// �������� ��� �����
	point3 m_vel;
	// ���� �������� ��� ��������
	point3 m_color;
	// ������� �������� ������� �� �������� ����� ��������
	float m_mark_time;

	// ������� ����������� �����
	void Multiply();
};

#endif // !defined(__SMOKEEFFECT_H__)

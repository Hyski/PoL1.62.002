/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ������
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined(__EXPLOSIONEFFECT_H__)
#define __EXPLOSIONEFFECT_H__

#include "Effect.h"
#include "Particle.h"

//
// ����� �������, ������������ ��� ������
//
class ExplosionParticle : public BaseParticle
{
public:
	// �������� �������
	point3 velocity;
	// ����� �������� �������
	point3 start_point;
};


//
// ����� ������
//

class ExplosionEffect : public BaseEffect  
{
public:

	// ���������� ������� ������� ����� � ������� ������ �����
	virtual bool Update();

	// ����������
	virtual ~ExplosionEffect() {}

	// ������ �������� ���� ������?
	virtual bool IsFinished() const { return finished; }

protected:
	// ������� ����������� �����
	void Multiply();
	// ����������� �� ���������
	ExplosionEffect() {};

	//
	// ������� ������ �������
	//

	// ������ ������
	ExplosionParticle * particles;
	// ���������� ������ � �������
	unsigned int particles_num;
	// ����, ����������� �� ������������� ���������� �������
	bool finished;
	point3 root_point;                       // �������� �����
	point3 start_velocity;                // ��������� ��������
};

//
// ����� ������ �������
//

class GrenadeEffect : public ExplosionEffect  
{
public:

	// ��������� ��� �������
	// (������ ������ ���������)
	virtual bool NextTick(const float current_time);

	// ���������� bounding box �������
	virtual BBox GetBBox(const float current_time);

	// ����������
	virtual ~GrenadeEffect();
	// ����������� � �����������
	GrenadeEffect(const float quality,  // ��������
		const point3 rp,                // �������� �����
		const GRENADE_EFFECT_INFO& info
		);

protected:

	// � ������ ����� ������ ���������� ������
	// ������� ��� ������� �������
	virtual bool Start(const float start_time);

	//
	// ������� ������ �������
	//

	float start_time;                         // ����� ������ �������

	point3 accel;                           // ���������
	float start_size;                        // ��������� ������
	float start_size_range;              // ��������� ����� ���������� �������
	float size_factor;                      // ��������� �������
	float lifetime;               // ����� ����� �������

	// ������� ����������� �����
	void Multiply();
	// ������� ����������� ������� ��� �� ��������
	void Rebirth(ExplosionParticle & ep, float curtime);
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ����� ������������� ���� ��� ��������
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class SparkleEffect : public ExplosionEffect  
{
public:

	// ��������� ��� �������
	// (������ ������ ���������)
	virtual bool NextTick(const float current_time);

	// ���������� bounding box �������
	virtual BBox GetBBox(const float current_time);

	// ����������
	virtual ~SparkleEffect();
	// ����������� � �����������
	SparkleEffect(
		const point3 rp,                // �������� �����
		const SPARKLE_EFFECT_INFO& info
		);

protected:

	// � ������ ����� ������ ���������� ������
	// ������� ��� ������� �������
	virtual bool Start(const float start_time);

	//
	// ������� ������ �������
	//

	
	float prev_time;                         // ���������� ������� �����
	float start_time;                         // ����� ������ �������

	point3 accel;                           // ���������
	float start_size;                        // ��������� ������
	float start_size_range;              // ��������� ����� ���������� �������
	float size_factor;                      // ��������� �������
	float lifetime;               // ����� ����� �������
	bool full;                  // ���� true, �� ������ �����, ����� - ��������


	// ������� ����������� �����
	void Multiply();
	// ������� ����������� ������� ��� �� ��������
	void Rebirth(ExplosionParticle & ep, float curtime);
};



//
// ����� ���� ��� �������
//

class ExplosionSmokeEffect : public ExplosionEffect  
{
public:

	// ��������� ��� �������
	// (������ ������ ���������)
	virtual bool NextTick(const float current_time);

	// ���������� bounding box �������
	virtual BBox GetBBox(const float current_time);

	// ����������
	virtual ~ExplosionSmokeEffect();
	// ����������� � �����������
	ExplosionSmokeEffect(const float quality,  // ��������
		const point3 rp,                       // �������� �����
		const EXPLOSION_SMOKE_INFO& info
		);

protected:

	// � ������ ����� ������ ���������� ������
	// ������� ��� ������� �������
	virtual bool Start(const float start_time);

	//
	// ������� ������ �������
	//

	float prev_time;                         // ���������� ������� �����
	float start_time;                         // ����� ������ �������

	point3 accel;                           // ���������
	float start_size;                        // ��������� ������
	float start_size_range;              // ��������� ����� ���������� �������
	float size_factor;                      // ��������� �������
	float lifetime;               // ����� ����� �������

	// ������� ����������� ������� ��� �� ��������
	void Rebirth(ExplosionParticle & ep, float curtime);
};

//
// ����� �������� ������ ��� �������
//

class ExplosionSmokeRing : public BaseEffect  
{
public:

	// �������� ��� ������� (� ����� ������ �����������)
	virtual EFFECT_TYPE GetType() const { return EFFECT_TYPE::_3D; }

	// ��������� ��� �������
	// (������ ������ ���������)
	virtual bool NextTick(const float current_time);

	// ���������� ������� ������� ����� � ������� ������ �����
	virtual bool Update();

	// ������ �������� ���� ������?
	virtual bool IsFinished() const { return finished; }

	// ���������� bounding box �������
	virtual BBox GetBBox(const float current_time);

	// ����������
	virtual ~ExplosionSmokeRing();
	// ����������� � �����������
	ExplosionSmokeRing(
		const point3 rp,                     // �������� �����
		const SMOKE_RING_INFO &info
		);

protected:

	// � ������ ����� ������ ���������� ������
	// ������� ��� ������� �������
	virtual bool Start(const float start_time);

	// ������� ����������� �����
	void Multiply();

	//
	// ������� ������ �������
	//

	
	float start_time;                         // ����� ������ �������
	point3 root_point;                       // �������� �����
	point3 start_velocity; // �� ���� �� ��� �� �����

	float start_size;                        // ��������� ������
	float size_factor;                      // ��������� �������
	float size;                                // ������� ������ �������
	float lifetime;               // ����� ����� �������
	float color;                  // ������� ���� �������
	// ���������� ������ � �������
	unsigned int particles_num;
	// ������ ��������� ��� ������
	point3 * coords;

	// ����, ����������� �� ������������� ���������� �������
	bool finished;
};


#endif // !defined(__EXPLOSIONEFFECT_H__)

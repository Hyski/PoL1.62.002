
/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ������ ������������� ������
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                
#if !defined (__DIRECTEXPLOSION_EFFECT_H__)
#define __DIRECTEXPLOSION_EFFECT_H__

#include "Effect.h"
#include "Particle.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ����� ������������������ ���������� ������
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class DirectSparkleSequence
{
public:
	// ���������� ������� �������
	point3 coords;
	// �������� ������� �������
	point3 velocity;
	// ��������� ������� �������
	point3 accel;
	// �������������� ������
	float size;
	// ����� �������� ������� �������
	point3 start_point;
	// ����� �����
	float lifetime;
	// ������ ������
	BaseParticle *sequence;
	// ������ ������� ������
	unsigned int seq_len;

	// �����������
	DirectSparkleSequence(const unsigned int SeqLen);
	~DirectSparkleSequence();
	
	// ������������������� ������������������
	void Rebirth(
		const point3 st_point,         // ��������� �����
		const point3 vecVelocity,    // ������ ����������� ��������
		const point3 DV,                // ������� ��������
		const point3 Accel,            // ���������
		const float stSize,	              // ��������� ������
		const float stRange,           // ��������� ����� ���������� �������
		const float Lifetime              // ����� �����
		);
	
	// ��������� ��� �������
	void NextTick(const float dt);
	
};

typedef DirectSparkleSequence * PDirectSparkleSequence;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ����� ������������ ���� ������������� ������
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class DirectSparkles : public BaseEffect
{
public:
	// �������� ��� ������� (� ����� ������ �����������)
	virtual EFFECT_TYPE GetType() const { return EFFECT_TYPE::_2D; }
	// ������ �������� ���� ������?
	virtual bool IsFinished() const { return finished; }

	// ��������� ��� �������
	// (������ ������ ���������)
	virtual bool NextTick(const float current_time);

	// ���������� bounding box �������
	virtual BBox GetBBox(const float current_time);

	// ���������� ������� ������� ����� � ������� ������ �����
	virtual bool Update();

	// ����������
	virtual ~DirectSparkles();
	// ����������� � �����������
	DirectSparkles(
		const point3 rp,                 // �������� �����
		const point3 vecVelocity,        // ������ ����������� ��������
		const DIRECT_SPARKLES_INFO &info
		);

protected:

	// � ������ ����� ������ ���������� ������
	// ������� ��� ������� �������
	virtual bool Start(const float start_time);

	//
	// ������� ������ �������
	//

	float prev_time;                       // ���������� ������� �����
	float start_time;                       // ����� ������ �������
	point3 root_point;                    // �������� �����
	point3 vec_velocity;                 // ������ ����������� ��������
	point3 dv;                               // ������� ��������
	point3 accel;                           // ���������
	float start_size;                        // ��������� ������
	float start_size_range;              // ��������� ����� ���������� �������
	float lifetime;               // ����� ����� �������
	unsigned int seq_len;               // ����� ������������������

	// ���������� ������������������� � �������
	unsigned int sequences_num;

	// ������ ���������� �� ������������������
	PDirectSparkleSequence * sequences;

	// ����, ����������� �� ������������� ���������� �������
	bool finished;

	// ������� ����������� �����
	void Multiply();
};


//
// ����� �������, ������������ ��� ������������� ������
//
class DirectParticle : public BaseParticle
{
public:
	// �������� �������
	point3 velocity;
	// ��������� �������
	point3 accel;
	// ����� �������� �������
	point3 start_point;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ����� ������������� ����
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class DirectExplosion : public BaseEffect
{

public:
	// �������� ��� ������� (� ����� ������ �����������)
	virtual EFFECT_TYPE GetType() const { return EFFECT_TYPE::_2D; }
	// ������ �������� ���� ������?
	virtual bool IsFinished() const { return finished; }

	// ��������� ��� �������
	// (������ ������ ���������)
	virtual bool NextTick(const float current_time);

	// ���������� bounding box �������
	virtual BBox GetBBox(const float current_time);

	// ���������� ������� ������� ����� � ������� ������ �����
	virtual bool Update();

	// ����������
	virtual ~DirectExplosion();
	// ����������� � �����������
	DirectExplosion(
		const point3 rp,             // �������� �����
		const point3 vecVelocity,    // ������ ����������� ��������
		const DIRECT_EXPLOSION_INFO& info
		);

protected:

	// � ������ ����� ������ ���������� ������
	// ������� ��� ������� �������
	virtual bool Start(const float start_time);

	//
	// ������� ������ �������
	//

	float prev_time;                       // ���������� ������� �����
	float start_time;                       // ����� ������ �������
	point3 root_point;                    // �������� �����
	point3 vec_velocity;                 // ������ ����������� ��������
	point3 dv;                               // ������� ��������
	point3 accel;                           // ���������
	float start_size;                        // ��������� ������
	float start_size_range;              // ��������� ����� ���������� �������
	float size_factor;                      // ��������� �������
	float lifetime;               // ����� ����� �������

	// ������ ������
	DirectParticle * particles;
	// ���������� ������ � �������
	unsigned int particles_num;

	// ����, ����������� �� ������������� ���������� �������
	bool finished;

	// ������� ����������� �����
	void Multiply();

	// ������� ����������� ������� ��� �� ��������
	void Rebirth(DirectParticle & dp, float curtime);
};


#endif // __DIRECTEXPLOSION_EFFECT_H__                                                                                

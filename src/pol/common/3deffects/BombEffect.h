/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ����� �����
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined(__BOMBEFFECT_H__)
#define __BOMBEFFECT_H__

#include "Effect.h"
#include "Particle.h"

#define SEQUENCE_LEN 10

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ����� ������������������ ���������� ������
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ParticleSequence
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
	BaseParticle sequence[SEQUENCE_LEN];
	// ������������������� ������������������
	void Rebirth(
		const point3 st_point,         // ��������� �����
		const point3 stVelocity,       // ��������� ��������
		const point3 Accel,            // ���������
		const float stSize,	              // ��������� ������
		const float stRange,           // ��������� ����� ���������� �������
		const float Lifetime,              // ����� �����
		const bool bFull                  // ���� true, �� ������ �����, ����� - ��������
		);
	// ��������� ��� �������
	void NextTick(const float dt);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ����� ������������� ������������������� ������
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class BombSparkles : public BaseEffect
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

	// ����������� � �����������
	BombSparkles(
		const point3 rp,               // �������� �����
		const BOMB_SPARKLES_INFO& info
		);
	// ����������
	virtual ~BombSparkles();

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
	point3 start_velocity;                // ��������� ��������
	point3 accel;                           // ���������
	float start_size;                        // ��������� ������
	float start_size_range;              // ��������� ����� ���������� �������
	float size_factor;                      // ��������� �������
	float lifetime;               // ����� ����� �������
	bool full;                  // ���� true, �� ������ �����, ����� - ��������

	// ���������� ������������������� � �������
	unsigned int sequences_num;

	// ��������� �� ������ �������������������
	ParticleSequence * sequences;

	// ����, ����������� �� ������������� ���������� �������
	bool finished;

	// ������� ����������� �����
	void Multiply();

	point3 vel;
};

#endif // !defined(__BOMBEFFECT_H__)

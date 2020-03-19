
/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ������ ������������ ����
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                
#if !defined (__SPARKLE_EFFECT_H__)
#define __SPARKLE_EFFECT_H__

#include "Effect.h"
#include "Particle.h"

#define SEQUENCE_LEN1 6 // ����� ������������������ ������

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ����� ������������������ ���������� ������
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class SparkleSequence
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
	BaseParticle sequence[SEQUENCE_LEN1];
	
	
	// ������������������� ������������������
	void Rebirth(
		const point3 st_point,       // ��������� �����
		const point3 vecVelocity,    // ������ ����������� ��������
		const point3 DV,             // ������� ��������
		const point3 Accel,          // ���������
		const float stSize,	         // ��������� ������
		const float stRange,         // ��������� ����� ���������� �������
		const float Lifetime         // ����� �����
		);
	
	// ��������� ��� �������
	void NextTick(const float dt);
	
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ����� ������������ ����
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class OrientedSparkles : public BaseEffect
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
	virtual ~OrientedSparkles();
	// ����������� � �����������
	OrientedSparkles(
		const float quality,         // ��������
		const point3 rp,             // �������� �����
		const point3 vecVelocity,    // ������ ����������� ��������
		const point3 DV,             // ������� ��������
		const point3 Accel,          // ���������
		const float stSize,	         // ��������� ������
		const float stRange,         // ��������� ����� ���������� �������
		const float sizeFactor,      // ��������� �������
		const float LifeTime,        // ����� ����� �������
		const std::string& Shader,   // ��� �������
		const std::string& Sound,    // ��� ��������� �������
		const bool SoundMove         // ��������� �� ����
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

	// ���������� ������������������� � �������
	unsigned int sequences_num;

	// ��������� �� ������ �������������������
	SparkleSequence * sequences;

	// ����, ����������� �� ������������� ���������� �������
	bool finished;

	// ������� ����������� �����
	void Multiply();
};


#endif // __SPARKLE_EFFECT_H__                                                                                

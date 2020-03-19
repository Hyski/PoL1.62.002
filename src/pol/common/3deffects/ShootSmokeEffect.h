/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ����� ���� �� ��������
				
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined(__SHOOTSMOKEEFFECT_H__)
#define __SHOOTSMOKEEFFECT_H__

#include "Effect.h"
#include "Particle.h"

//
// ����� �������
//
class ShootSmokeParticle : public BaseParticle
{
public:
	point3 velocity;
	float size_vel;
	ShootSmokeParticle(): velocity(0, 0, 0), size_vel(0)
	{}
};

//
// ����� ���� �� ��������
//


class ShootSmokeEffect : public BaseEffect  
{
public:

	// ����������� � �����������
	ShootSmokeEffect(
		const point3 StartPoint,                         // ��������� �����
		const point3 EndPoint,	                         // �������� �����
		const SHOOT_SMOKE_EFFECT_INFO& info
		);

	// ����������
	virtual ~ShootSmokeEffect();


	// �������� ��� ������� (� ����� ������ �����������)
	virtual EFFECT_TYPE GetType() const { return EFFECT_TYPE::_2D; }

	// ��������� ��� �������
	// (������ ������ ���������)
	virtual bool NextTick(const float current_time);

	// ���������� ������� ������� ����� � ������� ������ �����
	virtual bool Update();

	// ������ �������� ���� ������?
	virtual bool IsFinished() const { return finished; }

	// ���������� bounding box �������
	virtual BBox GetBBox(const float) { return bbox; }


protected:

	// � ������ ����� ������ ���������� ������
	// ������� ��� ������� �������
	virtual bool Start(const float start_time);

	// ������� ����������� �����
	virtual void Multiply();

	//
	// ������� ������ �������
	//

	// ������ ������
	ShootSmokeParticle* particles;
	int particles_num;                        // ������� ���������� ������

	float start_time;                         // ����� ������ �������
	point3 root_point;                        // ��������� �����
	point3 direction;                         // ����������� �������� (��������� ������)
	unsigned int color;                       // ����
	
	
	point3 accel;                             // ���������

	float start_size;                         // ��������� ������
	float size_velocity;                      // �������� ��������� �������


	float lifetime;                           // ����� ����� �������

	BBox bbox;

	point3 velocity;                          // ��������
	// ����, ����������� �� ������������� ���������� �������
	bool finished;
};


#endif // !defined(__SHOOTSMOKEEFFECT_H__)

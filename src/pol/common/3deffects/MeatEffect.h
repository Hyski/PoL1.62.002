/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ����� ��� �������� ����
				
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined(__MEATEFFECT_H__)
#define __MEATEFFECT_H__

#include "Effect.h"
#include "Particle.h"

#include <undercover/skin/skanim.h>

// ��������� ��� �������� ���������� �� ����� ��������
struct MeatEffectParticle;
//
// ����� ������� ��� ������� �����
//
class BloodParticle : public BaseParticle
{
public:
	point3 start_point;
	point3 velocity;
	float birthtime;
	float lifetime;
	int texcoord_type;                        // ��� ����������� ���������� ���������
	BloodParticle(): start_point(0, 0, 0), velocity(0, 0, 0), birthtime(0), lifetime(0), texcoord_type(0)
	{}
};

// ��� - ������ ������
typedef std::list<BloodParticle> BloodParticlesList;
// ��� - �������� ��� ������
typedef BloodParticlesList::iterator BloodParticlesListIterator;


//
// ������ �������� ���� (����� �������)
//

class MeatEffect : public BaseEffect
{
public:

	// �������� ��� ������� (� ����� ������ �����������)
	virtual EFFECT_TYPE GetType() const { return EFFECT_TYPE::_2D; }

	// ����������� � �����������
	MeatEffect(
		const unsigned int entity_id,
		const point3& explosion,
		const point3& human,
		const MEAT_EFFECT_INFO& info,
		float angle_vel,
		float vel = 2.0f
		);

	// ����������
	virtual ~MeatEffect();

	// ��������� ��� �������
	// (������ ������ ���������)
	virtual bool NextTick(const float current_time);

	// ���������� ������� ������� ����� � ������� ������ �����
	virtual bool Update();

	// ������ �������� ���� ������?
	virtual bool IsFinished() const { return finished; }

	// ���������� bounding box �������
	virtual BBox GetBBox(const float current_time) { return bbox; }
protected:
	virtual bool Start(const float );

	// ������� ����������� �����
	virtual void Multiply();

	//
	// ������� ������ �������
	//

	// ������ ������
	BloodParticlesList particles;
	int particles_num;                        // ������� ���������� ������
	float * size_vector;                      // ������ � ��������� ������
	unsigned int color;                       // ����
	float start_size;                         // ��������� ������
	float end_size;                           // �������� ������
	float particles_lifetime;                 // ����� ����� ��������
	int frequency;                            // ������� ��������� (���������� ����� ������ � �������)
	float previous_birth_time;                // ����� �������� ���������� ������� (����� ������)

	MeatEffectParticle* m_models;
	BBox bbox;
	float start_time;                         // ����� ������ �������
	float lifetime;                           // ����� ����� �������
	AnimaData m_animadata;    // ���������� � �����������
	// ������� �� ������� ����� ������� �������� � �������� �������� �������
	float m_dt; 

	// ����, ����������� �� ������������� ���������� �������
	bool finished;
};

#endif // !defined(__MEATEFFECT_H__)

/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ����� (������)
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined(__FIREEFFECT_H__)
#define __FIREEFFECT_H__

#include "Effect.h"
#include "Particle.h"

//
// ����� �������, ������������ ��� ����
//

class FireParticle : public BaseParticle
{
public:
	// �������� �������
	point3 velocity;
	// ���������� ���������� �������
	texcoord texcoords1;
	texcoord texcoords2;
	// ����� �������� �������
	float birth_time;
	// ����� ����� �������
	float life_time;
};

//
// ����� ����
//

class FireEffect : public BaseEffect  
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
	FireEffect(
		const float quality,
		const point3 rp,			  // ��������� �����
		const FIRE_EFFECT_INFO& info
		);

	// ����������
	virtual ~FireEffect();

	// ������ �������� ���� ������?
	// (�� ��������� ������ �������� ����������)
	virtual bool IsFinished() const { return finished; }

	// ��������� ������� � ������ ������ ����� ������
	virtual void NewTurn() { if(turns_left) turns_left--; }

	// ��������� ������� �� ��������� �������
	virtual void SetPosition(const point3& Pos) { root_point = Pos; }

	// ��������� ������� �� ��������� �������������
	virtual void SetActivity(const float Activity) { activity = Activity; }

	// ��������� ������� � ������������� ������ ��������� � �������
	virtual void Finish() { turns_left = 0; }

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
	// �������� ������������� ������� (�������)
	float activity;

	// ������ ������
	FireParticle * particles;
	// ���������� ������ � �������
	unsigned int particles_num;

	// �����, �� ������� ����� �����
	point3 root_point;
	// ����� ������ ���������� �������
	float start_time;

	// ������������ ������������ �������� �� �(Y) ��� ������ �������
	float startDX;
	// ������������ ������������ �������� �� Z ��� ������ �������
	float startDZ;
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

	// ������� ����������� �����
	void Multiply();
	// ������� ����������� ������� ��� �� ��������
	void Rebirth(FireParticle & sp, float curtime, float disp);

	point3 vel;
};

#endif // !defined(__FIREEFFECT_H__)

/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ����� ������ �� ������
				����� �������� �������� �� ��������
				
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined(__TAILEFFECT_H__)
#define __TAILEFFECT_H__

#include "Effect.h"
#include "Particle.h"

#include <undercover/skin/skanim.h>

//
// ����� ������� ��� ������
//
class TailParticle : public BaseParticle
{
public:
	point3 start_point;
	point3 velocity;
	float birthtime;
	float lifetime;
	int texcoord_type;                        // ��� ����������� ���������� ���������
	TailParticle(): start_point(0, 0, 0), velocity(0, 0, 0), birthtime(0), lifetime(0), texcoord_type(0)
	{}
};

// ��� - ������ ������
typedef std::list<TailParticle> TailParticlesList;
// ��� - �������� ��� ������
typedef TailParticlesList::iterator TailParticlesListIterator;

//
// ����� ������ �� ������
//


class TailEffect : public BaseEffect  
{
public:

	// ����������� � �����������
	TailEffect(
		const point3 StartPoint,                         // ��������� �����
		const point3 EndPoint,	                         // �������� �����
		const TAIL_EFFECT_INFO& info
		);

	// ����������
	virtual ~TailEffect();


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
	virtual BBox GetBBox(const float current_time);


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
	TailParticlesList particles;
	int particles_num;                        // ������� ���������� ������

	float * size_vector;                      // ������ � ��������� ������
	
	float start_time;                         // ����� ������ �������
	float last_tick;							//����� ���������� �������
	point3 start_point;                       // ��������� �����
	point3 end_point;                         // �������� �����
	point3 direction;                         // ����������� �������� (��������� ������)
	unsigned int color;                       // ����
	float start_size;                         // ��������� ������
	float end_size;                           // �������� ������

	point3 current_point;                     // ������� �����

	float lifetime;                           // ����� ����� �������
	float particles_lifetime;                 // ����� ����� ��������
	int frequency;                            // ������� ��������� (���������� ����� ������ � �������)
	float previous_birth_time;                // ����� �������� ���������� ������� (����� ������)

	point3 velocity;                          // ��������

	// ����, ����������� �� ������������� ���������� �������
	bool finished;
};


//
// ����� ������ �� ������
//

class TailCircleEffect : public TailEffect  
{
public:

	// ����������� � �����������
	TailCircleEffect(
		const point3 StartPoint,                         // ��������� �����
		const point3 EndPoint,	                         // �������� �����
		const TAIL_EFFECT_INFO& info
		) : TailEffect(StartPoint, EndPoint, info) {}

	// �������� ��� ������� (� ����� ������ �����������)
	virtual EFFECT_TYPE GetType() const { return EFFECT_TYPE::_3D_TRIANGLE; }

	// ���������� ������� ������� ����� � ������� ������ �����
	virtual bool Update();

protected:

	// � ������ ����� ������ ���������� ������
	// ������� ��� ������� �������
	virtual bool Start(const float start_time);

	// ������� ����������� �����
	virtual void Multiply();
};

//
// ����� ������ �� ������, �������� �� ��������
//

class TailParabolaEffect : public TailEffect  
{
public:

	// ����������� � �����������
	TailParabolaEffect(
		const point3 StartPoint,                         // ��������� �����
		const point3 EndPoint,	                         // �������� �����
		const TAIL_EFFECT_INFO& info,
		const float Gravitation
		);


	// ��������� ��� �������
	// (������ ������ ���������)
	virtual bool NextTick(const float current_time);

	// ���������� bounding box �������
	virtual BBox GetBBox(const float current_time);

protected:
	float gravitation;
	float vz;
};

//
// ����� �������� �������� �� ��������
//

class TracerParabolaModel : public BaseEffect
{
public:

	// �������� ��� ������� (� ����� ������ �����������)
	virtual EFFECT_TYPE GetType() const { return EFFECT_TYPE::_3D; }

	// ����������� � �����������
	TracerParabolaModel(
		const point3 StartPoint,                         // ��������� �����
		const point3 EndPoint,	                         // �������� �����
		const TRACER_PARABOLA_MODEL_INFO& info
		);

	// ����������
	virtual ~TracerParabolaModel();

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

	// � ������ ����� ������ ���������� ������
	// ������� ��� ������� �������
	virtual bool Start(const float start_time);

	//
	// ������� ������ �������
	//
	BBox bbox;
	float start_time;                         // ����� ������ �������
	point3 start_point;                       // ��������� �����
	point3 end_point;                         // �������� �����
	float gravitation;                        // ����������
	float vz;
	point3 current_point;

	float lifetime;                           // ����� ����� �������
	point3 direction;                         // ����������� ��������

	point3 vel;

	// ����, ����������� �� ������������� ���������� �������
	bool finished;

	// ��� ��������
	SkAnim* m_skeleton;    // ������
	SkSkin* m_skin;           // ����
	AnimaData m_animadata;    // ���������� � �����������
	D3DMATRIX m_mat; // �������
};

#endif // !defined(__TAILEFFECT_H__)

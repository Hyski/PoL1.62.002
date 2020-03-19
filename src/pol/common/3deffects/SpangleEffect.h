/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: �����, ���������� �� ��������
				����� ������� �����
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined(__SPANGLEEFFECT_H__)
#define __SPANGLEEFFECT_H__

#include "Effect.h"
#include "Particle.h"

//
// ����� ����, ���������� �� ��������
//

// ����� ������
class SpangleParticle : public BaseParticle
{
public:
	// �������� �������
	point3 velocity;
	// ����� ����� �������
	float lifetime;
	// ����� �������� �������
	float birthtime;
	// ��������� �����
	point3 start_point;
};

class SpangleEffect : public BaseEffect  
{
public:

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

	// ����������
	virtual ~SpangleEffect();
	// ����������� � �����������
	SpangleEffect(const float quality,  // ��������
		const point3 StartPoint,                         // ��������� �����
		const point3 EndPoint,	                         // �������� �����
		const SPANGLE_EFFECT_INFO& info
		);

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
	SpangleParticle * particles;
	// ���������� ������
	unsigned int particles_num;
	
	float start_time;                         // ����� ������ �������
	point3 start_point;                       // ��������� �����
	point3 end_point;                         // �������� �����
	point3 direction;                         // ����������� �������� (��������� ������)
	float size;                               // ������
	unsigned int color;                       // ����

	point3 current_point;                     // ������� �����

	float lifetime;                           // ����� ����� �������
	float particles_lifetime;                 // ����� ����� ��������

	point3 velocity;                          // ��������

	// ����, ����������� �� ������������� ���������� �������
	bool finished;
};

//
// ����� ������ �����
//


class LineBallEffect : public BaseEffect  
{
public:

	// �������� ��� ������� (� ����� ������ �����������)
	virtual EFFECT_TYPE GetType() const { return EFFECT_TYPE::_3D_LINESSTRIP; }

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
	virtual ~LineBallEffect();
	// ����������� � �����������
	LineBallEffect(const float quality,  // ��������
		const point3 StartPoint,                         // ��������� �����
		const point3 EndPoint,	                         // �������� �����
		const LINE_BALL_EFFECT_INFO& info
		);

protected:

	// � ������ ����� ������ ���������� ������
	// ������� ��� ������� �������
	virtual bool Start(const float start_time);

	// ������� ����������� �����
	virtual void Multiply() {}

	//
	// ������� ������ �������
	//

	// ������ ������
	SpangleParticle * particles;
	// ���������� ������
	unsigned int particles_num;
	
	float start_time;                         // ����� ������ �������
	point3 start_point;                       // ��������� �����
	point3 end_point;                         // �������� �����
	point3 direction;                         // ����������� �������� (��������� ������)
	float length;                             // �����
	float width;                              // ������
	unsigned int color;                       // ����

	point3 first_point;                       // ������ �����
	point3 second_point;                      // ������ �����

	float lifetime;                           // ����� ����� �������
	float particles_lifetime;                 // ����� ����� ��������

	point3 velocity;                          // ��������

	// ����, ����������� �� ������������� ���������� �������
	bool finished;
};

#endif // !defined(__SPANGLEEFFECT_H__)

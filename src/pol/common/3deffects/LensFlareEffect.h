/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ������ lens flare
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined(__LENSFLAREEFFECT_H__)
#define __LENSFLAREEFFECT_H__

#include "Effect.h"
#include "Particle.h"

//
// ����� lens flare
//


class LensEffect : public BaseEffect  
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
	virtual ~LensEffect();
	// ����������� � �����������
	LensEffect(
		const point3 StartPoint,                         // ��������� �����
		const point3 EndPoint,	                         // �������� �����
		const LENS_FLARE_EFFECT_INFO& info
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
	point3 start_point;                       // ��������� �����
	point3 end_point;                         // �������� �����
	float size;                               // ������
	float distance;                           // ���������� �� ������
	unsigned int color;                       // ����

	point3 current_point;                     // ������� �����

	float lifetime;                           // ����� ����� �������

	point3 velocity;                          // ��������

	// ����, ����������� �� ������������� ���������� �������
	bool finished;
};

#endif // !defined(__LENSFLAREEFFECT_H__)

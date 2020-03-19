/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ������� �������
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined(__SHIELDEFFECT_H__)
#define __SHIELDEFFECT_H__

#include "Effect.h"
#include "Particle.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ����� ������� �������
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ShieldEffect : public BaseEffect
{
public:
	// ����������� � �����������
	ShieldEffect(
		const point3& Source,
		const point3& Tartget,
		const SHIELD_INFO& info
		);
	// ����������
	virtual ~ShieldEffect();

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

protected:

	// � ������ ����� ������ ���������� ������
	// ������� ��� ������� �������
	virtual bool Start(const float start_time);

	// ������� ����������� �����
	void Multiply();

	//
	// ������� ������ �������
	//

	unsigned int spot_num;        // ���������� ����� � ����������
	unsigned int circles_num;     // ���������� �����������
	float dx;                     // ���������� �� ��
	float a;                      // ���������� � ��������
	float lifetime;               // ����� ����� �������
	float size;                   // ������ ������
	
	point3* unit_vectors;         // ��������� ������� �� ��������� ����������
	float start_time;             // ����� ������ �������
	point3 root_point;            // �������� �����
	point3 direction;             // �����������
	BBox bbox;

	// ����, ����������� �� ������������� ���������� �������
	bool finished;

	point3 vel;
};

#endif // !defined(__SHIELDEFFECT_H__)

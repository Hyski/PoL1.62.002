/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ������ ���������
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined(__SELECTIONEFFECT_H__)
#define __SELECTIONEFFECT_H__

#include "Effect.h"
#include "Particle.h"

//
// ����� ���������
//

class SelectionEffect : public BaseEffect  
{
public:

	// ����������� � �����������
	SelectionEffect(
		const point3 Target,        // ����������� �����
		const SELECTION_EFFECT_INFO &info
		);

	// ����������
	virtual ~SelectionEffect();

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
	virtual BBox GetBBox(const float ) { return bbox; }

	// ��������� ������� �� ��������� �������
	virtual void SetPosition(const point3& Pos) { root_point = Pos + point3(0.0f, 0.0f, 0.5f); }

	// ��������� ������� �� ��������� �������������
	virtual void SetActivity(const float Activity)
	{
		if(Activity == 0) visible = false;
		else visible = true;
	}

	// ��������� ������� � ������������� ������ ��������� � �������
	virtual void Finish() { finished = true; }

protected:

	// � ������ ����� ������ ���������� ������
	// ������� ��� ������� �������
	virtual bool Start(const float start_time);

	// ������� ����������� �����
	void Multiply();

	//
	// ������� ������ �������
	//

	// ���� ��������� �������
	bool visible;
	float radius;
	float small_size;
	float large_size;
	float angle_velocity;
	float angle_delta;
	// ����
	unsigned int color;
	
	point3 root_point;                     // �������� �����
	float start_time;
	point3 start_vector;                   // ��������� ������
	point3 current_vector;                 // ������� ������
	point3 axis;                           // ��� ��������
	BBox bbox;

	point3 velocity;                          // ��������

	// ����, ����������� �� ������������� ���������� �������
	bool finished;
};

#endif // !defined(__SELECTIONEFFECT_H__)

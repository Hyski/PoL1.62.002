/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ������ ������������� ��������� � ������ �������
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined(__DLIGHTEFFECT_H__)
#define __DLIGHTEFFECT_H__

#include "Effect.h"
#include "Particle.h"
#include <undercover/gamelevel/mark.h>
#include <boost/weak_ptr.hpp>

//
// ������� ����� ������� ������������� ���������
//

class DLightEffect : public BaseEffect  
{
public:

	// �������� ��� ������� (� ����� ������ �����������)
	virtual EFFECT_TYPE GetType() const { return EFFECT_TYPE::_3D; }

	// ���������� ������� ������� ����� � ������� ������ �����
	virtual bool Update();

	// ������ �������� ���� ������?
	virtual bool IsFinished() const { return finished; }

	// ���������� bounding box �������
	virtual BBox GetBBox(const float current_time);

	// �����������
	DLightEffect();
	// ����������
	virtual ~DLightEffect();

protected:
	//
	// ������� ������ �������
	//
	
	// ��������� �� ������ ������������� ���������
	boost::weak_ptr<Mark> d_light;

	float start_time;                         // ����� ������ �������
	float lifetime;                           // ����� ����� �������

	point3 current_point;                     // ������� �����
	float radius;                             // ������
	unsigned int color;                       // ����

	point3 vel;                          // ��������
	// ����, ����������� �� ������������� ���������� �������
	bool finished;
};

//
// ����� ������������� ���������, �������� �� ������
//

class LineDLightEffect : public DLightEffect  
{
public:

	// ����������� � �����������
	LineDLightEffect(
		const point3& StartPoint,            // ��������� ��������
		const point3& EndPoint,              // �������� �����
		const LINE_DLIGHT_EFFECT_INFO& info
		);

	// ����������
	virtual ~LineDLightEffect() {}

	// ��������� ��� �������
	// (������ ������ ���������)
	virtual bool NextTick(const float current_time);


protected:

	// � ������ ����� ������ ���������� ������
	// ������� ��� ������� �������
	virtual bool Start(const float start_time);

	//
	// ������� ������ �������
	//
	point3 start_point;                       // ��������� �����
	point3 end_point;                         // �������� �����

	point3 direction;                         // ����������� ��������
	point3 velocity;                          // ��������
};


//
// ����� ������� �������
//

class FlashEffect : public BaseEffect  
{
public:

	// ����������� � �����������
	FlashEffect(
		const point3& Target,            // ����� ���������
		const FLASH_EFFECT_INFO& info
		);

	// ����������
	virtual ~FlashEffect();

	// ��������� ��� �������
	// (������ ������ ���������)
	virtual bool NextTick(const float current_time);

	// �������� ��� ������� (� ����� ������ �����������)
	virtual EFFECT_TYPE GetType() const { return EFFECT_TYPE::_2D; }

	// ���������� ������� ������� ����� � ������� ������ �����
	virtual bool Update();

	// ������ �������� ���� ������?
	virtual bool IsFinished() const { return finished; }

	// ���������� bounding box �������
	virtual BBox GetBBox(const float ) { return bbox; }

protected:

	// � ������ ����� ������ ���������� ������
	// ������� ��� ������� �������
	virtual bool Start(const float start_time);
	// ������� ����������� �����
	virtual void Multiply();
	
	//
	// ������� ������ �������
	//
	
	float start_time;                         // ����� ������ �������
	float lifetime;                           // ����� ����� �������
	float switch_time;                        // ����� ������������
	float size;                               // ������
	float size_begin;
	float size_end;
	unsigned int color;
	point3 root_point;                     // ������� �����

	BBox bbox;

	point3 vel;                          // ��������
	// ����, ����������� �� ������������� ���������� �������
	bool finished;
};

//
// ����� ������� ��������������� �������
//

class MultiTexFlashEffect : public FlashEffect  
{
public:

	// ����������� � �����������
	MultiTexFlashEffect(
		const point3& Target,            // ����� ���������
		const FLASH_EFFECT_INFO& info
		) : FlashEffect(Target, info) { phase = 0; }

protected:
	// ������� ����������� �����
	virtual void Multiply();
	//
	// ������� ������ �������
	//
	int phase;
	float u1, v1, u2, v2;
};

#endif // !defined(__DLIGHTEFFECT_H__)

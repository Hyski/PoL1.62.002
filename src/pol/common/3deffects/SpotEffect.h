/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: �����
                ���������� �����
                �������, ����������� ������ ������ ������
                �������, ����������� ������ ������ ������2
				��������������� ����� (�����������)
				��������� ����� (��������� ��������������� �����)
				������� ����� (������� ��������������� �����)
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined(__SPOTEFFECT_H__)
#define __SPOTEFFECT_H__

#include "Effect.h"
#include "Particle.h"

//
// ����� �����
//

class SpotEffect : public BaseEffect  
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
	virtual ~SpotEffect();
	// ����������� � �����������
	SpotEffect(
		const point3 StartPoint,                         // ��������� �����
		const point3 EndPoint,	                         // �������� �����
		const SPOT_EFFECT_INFO& info
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
	unsigned int color;                       // ����

	point3 current_point;                     // ������� �����

	float lifetime;                           // ����� ����� �������

	point3 velocity;                          // ��������

	// ����, ����������� �� ������������� ���������� �������
	bool finished;
};


//
// ����� ����������� �����
//

class ShineSpotEffect : public SpotEffect  
{
public:
	// ��������� ��� �������
	// (������ ������ ���������)
	virtual bool NextTick(const float current_time);

	// ����������� � �����������
	ShineSpotEffect(
		const point3 StartPoint,                         // ��������� �����
		const point3 EndPoint,	                         // �������� �����
		const SPOT_EFFECT_INFO& info
		) : SpotEffect(StartPoint, EndPoint, info), phase(0)
	{}

protected:
	unsigned int phase;
};

//
// �������, ����������� ������ ������ ������
//

class RotateSpotEffect : public BaseEffect  
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
	virtual ~RotateSpotEffect();
	// ����������� � �����������
	RotateSpotEffect(
		const point3 StartPoint,                         // ��������� �����
		const point3 EndPoint,	                         // �������� �����
		const ROTATE_SPOT_EFFECT_INFO& info
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
	float size_first;                         // ������ ������ �������
	float size_last;                          // ������ ��������� �������
	float radius_start;                       // ��������� ������
	float radius_end;                         // �������� ������
	unsigned int color;                       // ����
	bool disp;                                // ���� �� ������� ��������

	point3 * current_vectors;                 // ������ ��������� ��������
	point3 * current_points;                  // ������ ������� �����
	float * current_vectors_disp;             // ������ ��� �������� ��������� ��������
	point3 start_vector;                      // ��������� ������
	point3 direction;                         // ����������� ��������

	float lifetime;                           // ����� ����� �������

	point3 velocity;                          // ��������
	float ang_vel;                            // ������� ��������
	float delta;                              // �������� �� ������� ����� ��������������������
	int sequence_number;                      // ���������� �������������������
	int sequence_size;                        // ������ ������������������

	// ����, ����������� �� ������������� ���������� �������
	bool finished;
};

//
// �������, ����������� ������ ������ ������2
//

class RotateSpotEffect2 : public BaseEffect  
{
public:

	// �������� ��� ������� (� ����� ������ �����������)
	virtual EFFECT_TYPE GetType() const { return EFFECT_TYPE::_3D_TRIANGLE; }

	// ��������� ��� �������
	// (������ ������ ���������)
	virtual bool NextTick(const float current_time);

	// ���������� ������� ������� ����� � ������� ������ �����
	virtual bool Update();

	// ������ �������� ���� ������?
	virtual bool IsFinished() const { return finished; }

	// ���������� bounding box �������
	virtual BBox GetBBox(const float ) { return bbox; }

	// ����������
	virtual ~RotateSpotEffect2();
	// ����������� � �����������
	RotateSpotEffect2(
		const point3 Target,                         // ����������� �����
		const ROTATE_SPOT_EFFECT2_INFO& info
		);

protected: 

	// � ������ ����� ������ ���������� ������
	// ������� ��� ������� �������
	virtual bool Start(const float start_time);

	//
	// ������� ������ �������
	//
	
	float start_time;                         // ����� ������ �������
	point3 root_point;                        // ����������� �����
	point3 start_point;                       // ��������� �����
	float size;                               // ������
	float radius_begin;                       // ������
	float radius_end;                         // ������
	unsigned int color;                       // ����

	unsigned int number;                      // ����������

	float lifetime;                           // ����� ����� �������
	float ang_vel;                            // ������� ��������

	// ����, ����������� �� ������������� ���������� �������
	bool finished;

	// BBOX �������
	BBox bbox;

	point3 vel;
};

//
// ����� ���������������� ����� (�������, �����������)
//

class UpsizingSpot : public BaseEffect
{
public:

	// �������� ��� ������� (� ����� ������ �����������)
	virtual EFFECT_TYPE GetType() const { return EFFECT_TYPE::_3D; }

	// ����������� � �����������
	UpsizingSpot(
		const point3 StartPoint,            // ��������� �����
		const point3 EndPoint,	            // �������� �����
		const UPSIZING_SPOT_INFO &info  // ���������� � ����������
		);

	// ����������
	virtual ~UpsizingSpot();

	// ��������� ��� �������
	// (������ ������ ���������)
	virtual bool NextTick(const float current_time);

	// ���������� ������� ������� ����� � ������� ������ �����
	virtual bool Update();

	// ������ �������� ���� ������?
	virtual bool IsFinished() const { return finished; }

protected:

	// � ������ ����� ������ ���������� ������
	// ������� ��� ������� �������
	virtual bool Start(const float start_time);

	// ������� ����������� �����
	virtual void Multiply() = 0;

	//
	// ������� ������ �������
	//
	
	float start_time;                         // ����� ������ �������
	point3 start_point;                       // ��������� �����
	point3 current_point;                     // ������� �����
	point3 direction;                         // ����������� ��������
	unsigned int color;                       // ����

	float lifetime;                           // ����� ����� �������
	float start_vel;                          // ��������� ��������
	float accel;                              // ���������
	float width_first;                        // ������ � ������ �����
	float width_second;                       // ������ � ����� �����
	float width_vel;                          // �������� ��������� ������
	float d_width;                            // ������� ������� � ������

	point3 velocity;                          // ��������

	// ����, ����������� �� ������������� ���������� �������
	bool finished;
};

//
// ����� ���������� ������� (��������� ��������������� �����)
//

class OneUpsizingSpot : public UpsizingSpot  
{
public:

	// ����������� � �����������
	OneUpsizingSpot(
		const point3 StartPoint,            // ��������� �����
		const point3 EndPoint,	            // �������� �����
		const UPSIZING_SPOT_INFO &info      // ���������� � ����������
		) : UpsizingSpot(StartPoint, EndPoint, info) {}

	// ���������� bounding box �������
	virtual BBox GetBBox(const float current_time);

protected:

	// ������� ����������� �����
	void Multiply();
};

//
// ����� �������� ������� (������� ��������������� �����)
//

class TwoUpsizingSpot : public UpsizingSpot  
{
public:

	// ����������� � �����������
	TwoUpsizingSpot(
		const point3 StartPoint,            // ��������� �����
		const point3 EndPoint,	            // �������� �����
		const UPSIZING_SPOT_INFO &info      // ���������� � ����������
		) : UpsizingSpot(StartPoint, EndPoint, info) {}

	// ���������� bounding box �������
	virtual BBox GetBBox(const float current_time);

protected:

	// ������� ����������� �����
	void Multiply();
};

//
// ����� ��������� �����
//

class WaveSpot : public BaseEffect  
{
public:

	// �������� ��� ������� (� ����� ������ �����������)
	virtual EFFECT_TYPE GetType() const { return EFFECT_TYPE::_3D; }

	// ����������� � �����������
	WaveSpot(
		const point3 Target,                // ��������� �����
		const WAVE_SPOT_INFO &info          // ���������� � ����������
		);

	// ����������
	virtual ~WaveSpot();

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
	
	float start_time;                         // ����� ������ �������
	point3 root_point;                        // ��������� �����
	float start_size;                         // ��������� ������
	float size;                               // ������� ������
	float size_velocity;                      // �������� ����������� �������

	unsigned int color;

	float lifetime;                           // ����� ����� �������
	point3 velocity;                          // ��������

	// ����, ����������� �� ������������� ���������� �������
	bool finished;
};

//
// ����� ����������������� ��������� �����
//

class WaveSpot2 : public BaseEffect  
{
public:

	// �������� ��� ������� (� ����� ������ �����������)
	virtual EFFECT_TYPE GetType() const { return EFFECT_TYPE::_3D; }

	// ����������� � �����������
	WaveSpot2(
		const point3 Source,                // ����� ��������
		const point3 Target,                // ����� ���������
		const WAVE_SPOT_INFO &info          // ���������� � ����������
		);

	// ����������
	virtual ~WaveSpot2();

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
	
	float start_time;                         // ����� ������ �������
	point3 root_point;                        // ��������� �����
	point3 direction;                         // �����������
	point3 base;                              // ������� ������
	float start_size;                         // ��������� ������
	float size;                               // ������� ������
	float size_velocity;                      // �������� ����������� �������

	unsigned int color;

	float lifetime;                           // ����� ����� �������
	point3 velocity;                          // ��������

	// ����, ����������� �� ������������� ���������� �������
	bool finished;
};

#endif // !defined(__SPOTEFFECT_H__)

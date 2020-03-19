/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ����� ���������
				������ ����������� ������
				������ ������
				������ �������� �������� �� �����
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined(__LINEEFFECT_H__)
#define __LINEEFFECT_H__

#include "Effect.h"
#include "Particle.h"

#include <undercover/skin/skanim.h>

//
// ����� �����
//

class LineEffect : public BaseEffect
{
public:

	// �������� ��� ������� (� ����� ������ �����������)
	virtual EFFECT_TYPE GetType() const { return EFFECT_TYPE::_3D; }

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
	virtual ~LineEffect();
	// ����������� � �����������
	LineEffect(
		const point3 StartPoint,                         // ��������� �����
		const point3 EndPoint,	                         // �������� �����
		const LINE_EFFECT_INFO& info
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

	point3 first_point;                       // ������ �����
	point3 second_point;                      // ������ �����

	float lifetime;                           // ����� ����� �������
	float length;                             // ����� �����
	float width_first;                        // ������ � ������ �����
	float width_second;                       // ������ � ����� �����
	point3 direction;                         // ����������� ��������

	point3 velocity;                          // ��������

	// ����, ����������� �� ������������� ���������� �������
	bool finished;
};

//
// ����� ����������� ������
//

class PlasmaBeamEffect : public BaseEffect
{
public:

	// �������� ��� ������� (� ����� ������ �����������)
	virtual EFFECT_TYPE GetType() const { return EFFECT_TYPE::_3D; }

	// ����������� � �����������
	PlasmaBeamEffect(
		const point3 StartPoint,                         // ��������� �����
		const point3 EndPoint,	                         // �������� �����
		const PLASMA_BEAM_INFO& info
		);

	// ����������
	virtual ~PlasmaBeamEffect();

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
	void Multiply();

	//
	// ������� ������ �������
	//

	BBox bbox;
	float start_time;                         // ����� ������ �������
	point3 start_point;                       // ��������� �����
	point3 end_point;                         // �������� �����

	float lifetime;                           // ����� ����� �������
	float length;                             // ����� �����
	float width_target;                        // ������ � ������ �����
	float width_source;                       // ������ � ����� �����
	point3 direction;                         // ����������� ��������

	point3 vel;

	// ����, ����������� �� ������������� ���������� �������
	bool finished;
};

//
// ����� ������
//

class LightningEffect : public BaseEffect
{
public:

	// �������� ��� ������� (� ����� ������ �����������)
	virtual EFFECT_TYPE GetType() const { return EFFECT_TYPE::_3D_TRIANGLE; }

	// ����������� � �����������
	LightningEffect(
		const point3 StartPoint,                         // ��������� �����
		const point3 EndPoint,	                         // �������� �����
		const LIGHTNING_INFO& info
		);

	// ����������
	virtual ~LightningEffect();

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
	void Multiply();

	//
	// ������� ������ �������
	//

	point3* nodes;
	point3* start_nodes;
	BBox bbox;
	float start_time;                         // ����� ������ �������
	point3 start_point;                       // ��������� �����
	point3 end_point;                         // �������� �����

	float lifetime;                           // ����� ����� �������
	float length;                             // ����� �����
	float width;                              // ������
	unsigned int brunch_num;                  // ���������� ������ ������
	unsigned int segment_num;                 // ���������� ��������� � �����
	unsigned int color;
	point3 direction;                         // ����������� ��������

	point3 vel;

	// ����, ����������� �� ������������� ���������� �������
	bool finished;
};


//
// �������� �������� �� �����
//

class TracerLineModel : public BaseEffect
{
public:

	// �������� ��� ������� (� ����� ������ �����������)
	virtual EFFECT_TYPE GetType() const { return EFFECT_TYPE::_3D; }

	// ����������� � �����������
	TracerLineModel(
		const point3 StartPoint,                         // ��������� �����
		const point3 EndPoint,	                         // �������� �����
		const TRACER_LINE_MODEL_INFO& info
		);

	// ����������
	virtual ~TracerLineModel();

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

#endif // !defined(__LINEEFFECT_H__)

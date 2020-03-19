/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: Halo
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined(__HALOEFFECT_H__)
#define __HALOEFFECT_H__

#include "Effect.h"
#include "Particle.h"

// ��������� ��� �������� �������� ��������� halo � ����� ������ ���������
struct HaloState
{
	// ��������� ��������� �������
	enum State { hidden, visible, disappearing, appearing };
	State state;

	// ����� ���������� ������������ ��������� ���������
	float prev_time;
	// ����� ���������� ������������� ����
	float prev_trace_time;

	// �����, � ������� �������� ������ ��������� � ���������� ���������
	float appearing_time;
	// �����, � ������� �������� ������ ��������� � ���������� ���������
	float disappearing_time;

	// ������ ����� trace ray'��
	bool trace_ray_visible;
	// ������ ����� bbox'��
	bool bbox_visible;

	// ������ ���� �����������, �� Trace Ray ��� �� ����������!
	bool camera_was_changed;

	HaloState() {   state = hidden;
					prev_time = 0;
					prev_trace_time = 0;
					appearing_time = 0;
					disappearing_time = 0;
					trace_ray_visible = false;
					bbox_visible = false;
					camera_was_changed = false;
				}
};

//
// ������� ����� ��� halo (�����������)
//

class BaseHalo : public BaseEffect  
{
public:

	// ����������� � �����������
	BaseHalo(
		const point3 rp,			  // ��������� �����
		const std::string &Shader,    // ������ �������
		const std::string &Sound,     // ��������� �����
		const bool SoundMove          // ��������� �� ����
		);

	// ����������
	virtual ~BaseHalo();

	// ��������� ��� �������
	// (������ ������ ���������)
	virtual bool NextTick(const float current_time) = 0;

	// ���������� ������� ������� ����� � ������� ������ �����
	virtual bool Update();

	// ���������� bounding box �������
	virtual BBox GetBBox(const float ) { return bbox; }

protected:
	// � ������ ����� ������ ���������� ������
	// ������� ��� ������� �������
	virtual bool Start(const float start_time) = 0;
	// ������� ����������� �����
	virtual void Multiply();

	// ������� ������� ���������
	void UpdateVisibility(const float current_time);

	//
	// ������� ������ �������
	//

	// ��������� ��������� �������
	HaloState state;

	float size;
	// ����
	unsigned int color;
	unsigned int save_color;

	// �������� �����
	point3 root_point;
	BBox bbox;

	point3 vel;
};

//
// ����� simple halo
//

class SimpleHaloEffect : public BaseHalo  
{
public:

	// ����������� � �����������
	SimpleHaloEffect(
		const point3 rp,			  // ��������� �����
        const point3& Color,
		const SIMPLE_HALO_EFFECT_INFO &info
		);

	// ����������
	virtual ~SimpleHaloEffect();

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
};

//
// ����� halo
//

class HaloEffect : public BaseHalo  
{
public:

	// ����������� � �����������
	HaloEffect(
		const point3 rp,			  // ��������� �����
		const point3& Color,
		const HALO_EFFECT_INFO &info
		);

	// ����������
	virtual ~HaloEffect();

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

	float small_size;
	float large_size;
	float near_dist;
	float far_dist;
	float near_color_factor;
	float far_color_factor;

	float dist_range;
	float size_range;
	float color_factor_range;

	point3 color3;
};

//
// ����� ��������� halo
//
class BlinkingHaloEffect : public BaseHalo  
{
public:

	// ����������� � �����������
	BlinkingHaloEffect(
		const point3 rp,			  // ��������� �����
        const point3& Color,
		const BLINKING_HALO_EFFECT_INFO &info
		);

	// ����������
	virtual ~BlinkingHaloEffect();

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
	float sin_a;
	point3 my_save_color;
	float start_time;
};


#endif // !defined(__HALOEFFECT_H__)

/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ���������� �������� � ������� �������
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined(__MODEL_DESTRUCT_EFFECT_H__)
#define __MODEL_DESTRUCT_EFFECT_H__

#include "Effect.h"
#include "Particle.h"

#include <undercover/skin/skanim.h>

//
// ����� ���������� �������� � ������� �������
//

class ModelDestructEffect : public BaseEffect
{
public:

	// �������� ��� ������� (� ����� ������ �����������)
	virtual EFFECT_TYPE GetType() const { return EFFECT_TYPE::_3D; }

	// ����������� � �����������
	ModelDestructEffect(
		const std::string* names,
		unsigned int num,
		const BBox* LocalBBox,
		const D3DMATRIX* Matrix,
		const point3& Pnt,
		const MODEL_DESTRUCT_INFO& info
		);

	// ����������
	virtual ~ModelDestructEffect();

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

	// �������� ����� ��������
	void SetCurrentPoints(point3 * points,
		unsigned int num,
		const BBox* LocalBBox,
		const D3DMATRIX* Matrix);

	// �������� ����� �������
	point3 getCenter(const BBox* LocalBBox, const D3DMATRIX* Matrix);


	//
	// ������� ������ �������
	//
	BBox bbox;
	float start_time;                         // ����� ������ �������
	float previous_time;                      // ����� ����������� ����
	point3 start_point;                       // ��������� �����

	float lifetime;                           // ����� ����� �������
	point3 direction;                         // ����������� ��������

	point3 accel;

	// ����, ����������� �� ������������� ���������� �������
	bool finished;

	// ��� ��������
	SkAnim* m_skeleton;        // ������ ���� �� ����
	SkSkin** m_skins;          // ����� ������ �� ���� �������
	AnimaData m_animadata;    // ���������� � �����������
	point3* current_points;  // ������� ��������� ����� �������� �������
	point3* velocities;      // ��������� ��������
	unsigned int skin_num;   // ���������� ��������
	point3* angle_velocities; // ������� �������� ��������
	point3* current_angles;   // ������� ���� �������� ��������
	float* m_scales;
};

#endif // !defined(__LINEEFFECT_H__)

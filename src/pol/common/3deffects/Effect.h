
/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ������ �������� � ������������ ���� ��� ������ ��������
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                
#if !defined (__EFFECT_H__)
#define __EFFECT_H__

#include <Muffle/ISound.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ����� ���������� �������� ������� ��� ���������� ����� � �������
//
/////////////////////////////////////////////////////////////////////////////////////////////////////

//
// ����������� ����� SoundObject
//
class SoundObject
{
public:
	// ����� ����������� �������, �� ���������� ����������,
	// ����� ���������� ��������� ����� ��� ��������� ���������
	// ������� � ��� ��������
	virtual void Update(const point3& Position, const point3& Velocity) = 0;
	virtual ~SoundObject() {}
};

//
// ����� EmptySound - ��� �������, ������� �� ����������� ����
//
class EmptySound : public SoundObject
{
public:
	// ����� ���, ��� ��� Update ������ �� ������
	virtual void Update(const point3&, const point3&) {}
	
	EmptySound() {}
	virtual ~EmptySound() {}
};

//
// ����� FixedSound - ��� �������, ������� ����������� ����������� ����
//
class FixedSound : public SoundObject
{
public:
	// ���� �����������, ��� ��� Update ������ �� ������
	virtual void Update(const point3&, const point3&) {}

	FixedSound(const std::string& Name, const point3& Target);
	virtual ~FixedSound() {}
};

//
// ����� ActiveSound - ��� �������, ������� ����������� ����������� ����
//
class ActiveSound : public SoundObject
{
	Muffle::HEmitter emitter;

public:
	// ���� ���������, ��� ��� Update ������ �������� ���������� � ��������
	virtual void Update(const point3& Position, const point3& Velocity);

	ActiveSound(const std::string& Name);
	virtual ~ActiveSound();

private:
	// ��������� ����� ����������� �������
	virtual point3& GetPos() { return pos; }
	virtual point3& GetVel() { return vel; }
	point3 pos;
	point3 vel;
};



/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ����� ��������� ������� ����� ��� ���� ��������
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Particle.h"

class GraphPipe;
struct BBox;


// ������� ����� �������
class BaseEffect
{
public:
	BaseEffect() : m_masterEffect(0xFFFFFFFF) {}

	enum { PEAK_NUM = 6 };
	enum EFFECT_TYPE
	{
		_2D,
		_3D,
		_3D_TRIANGLE,
		_3D_LINESSTRIP,
		_3D_POINTS
	};
	virtual ~BaseEffect() {}
	// �������������� �������������
	void Init(GraphPipe * const pGP)
	{ pGraphPipe = pGP; }
////////////////////
//  �������� ������� ��������� �������� ������ � ��������
////////////////////

	// ��������� ��� �������
	// (������ ������ ���������)
	virtual bool NextTick(const float current_time) = 0;

	// ���������� ������� ������� ����� � ������� ������ �����
	virtual bool Update() = 0;

	// ���������� bounding box �������
	virtual BBox GetBBox(const float current_time) = 0;

////////////////////
//  �������������� ������� ��������� �������� ������ � ��������
////////////////////

	// �������� ��� �������
	std::string GetShader() const { return strShader; }
	// ���������� ����� ��� �������
	void SetShader(const std::string& str) { strShader = str; }

	// �������� ���������� ������� �����
	unsigned int GetReadyNum() const { return ready_num; }
	// �������� ����� ������� ������� �����
	Primi *GetReadyPrim() { return &prim; }
	//point3 * GetReadyVector() const
	//{ return ready_vector; }
	//// �������� ����� ������� ������ �����
	//unsigned int * GetColorsVector() const
	//{ return color_vector; }
	//// �������� ����� ������� ���������� ���������
	//texcoord * GetTexCoordsVector() const
	//{ return texcoord_vector; }

	// ������ �������� ���� ������?
	// (�� ��������� ������ �������� ����������)
	virtual bool IsFinished() const { return false; }
	
	// �������� ��� ������� (� ����� ������ �����������)
	virtual EFFECT_TYPE GetType() const { return EFFECT_TYPE::_2D; }

	// ��������� ������� � ������ ������ ����� ������
	virtual void NewTurn() {}

	// ��������� ������� �� ��������� �������
	virtual void SetPosition(const point3&) {}

	// ��������� ������� �� ��������� �������������
	virtual void SetActivity(const float) {}

	// ��������� ������� � ������������� ������ ��������� � �������
	virtual void Finish() {}

	unsigned int m_masterEffect;

////////////////////
//  ������ �������
////////////////////
protected:
	// � ������ ����� ������ ���������� ������
	// ������� ��� ������� �������
	virtual bool Start(const float start_time) = 0;

	// ��������� �� ������ GraphPipe
	GraphPipe * pGraphPipe;

	// ������ �������
	std::string strShader;
	// ������ ������� ��������� �����
	// � ������� ����� ����� �������������.
	int points_num;
	// ������ ������� ��������� ������� � ��������� ������,
	// ������� ������, ������ ���������� ���������
	int ready_vector_size;
	// �������� ���������� ����� ��� ��������� (���������������)
	int ready_num;
	// ��������� �� ������ ��������� ����� ��� �������������
	point3* points_vector;
	// ��������� �� ������ ��������� ����� ����� �������������
	point3* points2d_vector;
	// ��������� �� ������ ��������� ������� � ��������� �����
	Primi prim;
	//point3* ready_vector;
	//// ��������� �� ������ ������ �����
	//unsigned int * color_vector;
	//// ��������� �� ������ ���������� ���������
	//texcoord* texcoord_vector;

	// ����, ������� ���������� ������ �� ��� ����� NextTick
	bool first_time;

	// ������ - ����
	SoundObject * sound_object;
};

typedef BaseEffect * PBaseEffect;


///////////////////////////////////////////////////////////////////////////////
//
// ������������ ���� ��� ������ ��������
//
///////////////////////////////////////////////////////////////////////////////
namespace EffectUtility
{
	// �������� ��������� ����� � ��������� �� 0 �� 1 (float)
	float get0F();
	// �������� ��������� ����� � ��������� �� 0 �� f (float)
	float get0F(float f);
	
	// �������� ��������� ����� � ��������� �� -1 �� 1
	float get_FF();
	// �������� ��������� ����� � ��������� �� -f �� f
	float get_FF(float f);
	// �������� ��������� ����� � ��������� �� -f1 �� f2
	float get_FF(float f1, float f2);
};

#endif // __EFFECT_H__                                                                                

/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ������ ���������� ��������
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined(__DESTRUCTEFFECT_H__)
#define __DESTRUCTEFFECT_H__

#include <undercover/gamelevel/LevelObjects.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// �����, �������������� ��� ����������� (����������� �� TexObject)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class DestructObject : public TexObject
{
public:
	// �����������
	DestructObject(const float MaxSquare = 0.1);
	// ����������� �����������
	DestructObject(const DestructObject &a);
	// ����������
	virtual ~DestructObject();

	// �������� ������������ ��� ������������� �������
	DestructObject &operator =(const DynObject &a);

private: // ������
	float max_square;

public:
	// ��������� � ����������� � ������������, ���������� �� TexObject'a
	struct TriangleInf
	{
		point3 Points[3];
		point3 Normals[3];
		texcoord uv[3];
		float square;
	};

	typedef std::list<TriangleInf> TriangleList;

	// ��� ����������� �� ���������������� ������� � ������
	void CopyToList(STObject * stobj, D3DMATRIX& matr, TriangleList& ready_tr, TriangleList& temp_tr);

	// ��� ����������� �� ������ � ����� ������
	void CopyFromList(STObject * stobj, const TriangleList& tlist);

	// �������������� ������� � ���������� ������� �������������
	void ReformList(TriangleList& ready_tr, TriangleList& temp_tr);
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ������� ����� ������� �������������� �������
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class BaseDestructEffect
{
public:
	BaseDestructEffect(const float MaxSquare) : destruct_object(MaxSquare) {}
	virtual ~BaseDestructEffect() {}
	// �������� ���� ��������� ������
	virtual bool IsFinished() const { return finished; }

	// ��������� ��� �������
	virtual void NextTick(const float time) = 0;

	// ���������� ������
	virtual void Draw(GraphPipe * const graph_pipe) = 0;

protected:
	// ������ �����������
	DestructObject destruct_object;
	//DestructObject save_object;
	// ���� ��������� ������ �������
	bool finished;
	// ���� ������� ����
	bool first_time;


	// ������ - ����
	SoundObject * sound_object;
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ����� ������� �������������� ������
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// ��������� ��� �������� ���������� ������������
struct TriangleDesc
{
	// ������� ����� ����
	point3 center;
	// ��������� ������� �� ������ ���� � ��������
	point3 nodes[3];
	// ������� ������� �� ������ ���� � ��������
	point3 next_nodes[3];
	// ������� �������� �������� ������ ��������������� ����
	point3 angle_velocity;
	// ��������� �������� ������ ����
	point3 velocity;
};

// ��� �����
class GlassDestruct : public BaseDestructEffect
{
public:
	// �����������
	GlassDestruct(const DynObject& DynObj,
		const point3& ExpPos,
		const GLASS_DESTRUCT_EFFECT_INFO& info);
	// ����������
	virtual ~GlassDestruct();

	// ��������� ��� �������
	virtual void NextTick(const float time);

	// ���������� ������
	virtual void Draw(GraphPipe * const graph_pipe);
private:
	BBox debug_bbox;
	// ����������� ��� ���������� - ����� ����� �� ������� ������� ��� ������ ��� �����������
	GlassDestruct();
	// ����� �������� �������
	float start_time;

	// ���������
	point3 accel;
	// ����������� ������ �� ������
	float friction_factor;
	// ����������� ������� ��������
	point3 direction;
	// ����� �����
	float lifetime;
	// ����� ����������� ������� ���������
	float prev_time;

	// ���������� ���� ������������� � �������
	unsigned int triangles_num;

	// ������ �������� �������������
	TriangleDesc * triangles;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ����� ��������� �������� ������������� ��������
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ������ ��������
typedef std::list<BaseDestructEffect *> EffectsList;
// ��� �����
class DestructEffectManager
{
public:
	// �����������
	DestructEffectManager();
	// ����������
	~DestructEffectManager();

	// ��������� ��� �������
	void NextTick(const float time);

	// ��������� ���� ��������
	void Draw(GraphPipe * const graph_pipe);

	// ��������� ������� ��� ���������� ������
	void DestroyGlass(const DynObject& DynObj,
		const point3& ExpPos,
		const GLASS_DESTRUCT_EFFECT_INFO& info);

private:
	// ������ ��������
	EffectsList effects;
};

#endif // !defined(__DESTRUCTEFFECT_H__)

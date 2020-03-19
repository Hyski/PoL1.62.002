#pragma once

#include "SndAuto.h"
#include "GameLevel.h"
#include "objectsNG.h"
#include "DynObjectInfo.h"
#include "../Skin/KeyAnimation.h"
#include <Common/GraphPipe/SimpleTexturedObject.h>

//class SndAuto;
class DynObjectSet;

//=====================================================================================//
//                                   class DynObject                                   //
//=====================================================================================//
class DynObject : public MlRefCounted, DynObjectInfo, boost::noncopyable
{
	TexObject m_texObject;
	KeyAnimation m_anim;

	bool m_inMotion;		///< ����������� �� ������ �� ������ ������
	float m_animState;		///< ������������� ��������� ��������
	float m_targetState;	///< ��������� ��������, � �������� ���� ������������
	float m_startTime;		///< ����� ������ ��������
	bool m_isRoof;			///< ��������� �� ������ � ������

	DynObjectSet *m_set;
	SndAuto m_sound;

	bool m_isTransparentShader;

public:
	DynObject(const DynObjectOldData &);
	~DynObject();

private: //�������� �����
	BBox Bound;     //BBox � ������� �����������
	BBox LocalBound;//BBox � ��������� ����������� �������  (���������)

	D3DMATRIX m_world;    //������� �������
	D3DMATRIX m_invWorld; //�������� � ������� �������
	ShapePool::Handle m_MyShape;

public:    
	bool TraceRay(const ray &r, float *Pos, point3 *Norm);
	bool TraceSegment(const ray &r, float *Pos, point3 *Norm);
	void UpdateBound();
	//void EndTurn();

	/// ��������� �����, ��������� � ��������
	void StartSound();

	/// ��������� �� ������
	bool IsTransparentShader() const { return m_isTransparentShader; }

	/// �������� ���
	TexObject *GetTexObject() { return &m_texObject; }
	const TexObject *GetTexObject() const { return &m_texObject; }

	/// ��������� �� � ������
	bool IsRoof() const { return m_isRoof; }

	/// ����������� �� �� ������ ������ ������
	bool IsInMotion() const { return m_inMotion; }
	/// ���������� ������������ ������� ����� �������� ������� � ������
	float GetAnimLength() const { return m_anim.GetLastTime(); }

	/// �������� ������� �������
	const D3DMATRIX &GetWorldMatrix() const { return m_world; }
	/// �������� ������� �������
	const D3DMATRIX &GetInvWorldMatrix() const { return m_invWorld; }
	/// ���������� aabb ������� � ������� �����������
	const BBox &GetBound() const { return Bound; }
	/// ���������� aabb ������� � ��������� �����������
	const BBox &GetLocalBound() const { return LocalBound; }

	void SetShapeHandle(ShapePool::Handle h);
	ShapePool::Handle GetShapeHandle() const { return m_MyShape; }
	void SetAnimation(const KeyAnimation &anim) { m_anim = anim; }
	void SetWorldMatrix(const D3DMATRIX &world) { m_world = world; }
	void SetInvWorldMatrix(const D3DMATRIX &world) { m_invWorld = world; }
	void SetTexObject(const TexObject &texo);

	/// �������� ������, � ������� ��������� ������
	DynObjectSet *GetSet() { return m_set; }
	const DynObjectSet *GetSet() const { return m_set; }

	/// ���������� ������, � ������� ��������� ������
	void SetSet(DynObjectSet *set) { m_set = set; }

	/// ��������� ������
	bool MakeSave(SavSlot &);
	/// ������������ ������
	bool MakeLoad(SavSlot &);

	/// �������� �� ������ ����������
	bool HasGeometry() const { return m_texObject.PartNum && m_texObject.Parts[0]->prim.Verts.size(); }

	bool operator<(const DynObject &r) const { return GetName() < r.GetName(); }

	struct StringCompare
	{
		bool operator()(const DynObject &l, const std::string &r) const { return l.GetName() < r; }
		bool operator()(const std::string &l, const DynObject &r) const { return l < r.GetName(); }
	};

	void UpdateAfterLoad();

public:
	unsigned int GetTraceParams() const
	{
		unsigned int result = 0;
		if(IsGhost())
		{
			result |= mdGhost;
			result |= mdCursor;
		}
		if(IsTransparent()) result |= mdTransparent|mdDynObjectTransparent;
		return result;
	}

	using DynObjectInfo::GetName;
	using DynObjectInfo::IsGhost;
	using DynObjectInfo::IsTransparent;
	using DynObjectInfo::IsDestructible;

	using DynObjectInfo::IsAnimated;
	using DynObjectInfo::IsSoundable;

	using DynObjectInfo::GetSoundInfo;
	using DynObjectInfo::GetDestructibleInfo;

private: // ��������� ��� DynObjectSet
	float ChangeState(float tau);
	float ChangeState(float target, float tau);
	void Update(float tau);

private:
	friend class DynObjectSet;
	friend class DoorProperty;
	friend class TrainProperty;
	friend class ElevatorProperty;

	void Load5(SavSlot &);
	void Load4(SavSlot &);

	void UpdateOnTime(float tau);
};

typedef MlHandle<DynObject> HDynObject;

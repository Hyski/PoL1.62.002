#pragma once

#include "DynObject.h"
#include "DynObjectSetInfo.h"

class TrainProperty;
class UsableProperty;
class DestructibleProperty;

//=====================================================================================//
//                                 class DynObjectSet                                  //
//=====================================================================================//
class DynObjectSet : public MlRefCounted, DynObjectSetInfo, boost::noncopyable
{
	static const unsigned int MAX_OBJECTS = 32;

	HDynObject m_objects[MAX_OBJECTS];
	unsigned int m_objectCount;

	HDynObject m_updatableObjects[MAX_OBJECTS];
	unsigned int m_updatableObjectCount;

	point3 m_center;
	bool m_isDoor;

	/// �������� �������
	std::auto_ptr<TrainProperty> m_trainProp;
	std::auto_ptr<UsableProperty> m_usableProp;
	std::auto_ptr<DestructibleProperty> m_destrProp;

	bool m_destroyed;
	bool m_visible;

public:
	typedef HDynObject *Iterator_t;
	typedef const HDynObject *ConstIterator_t;

public:
	DynObjectSet(const DynObjectSetInfo &);
	DynObjectSet(const DynObjectSetInfo &, HDynObject *begin, HDynObject *end);
	~DynObjectSet();

	/// ��������� �� ������ � �������
	bool EverNeedUpdate() const;
	/// ��������� �� ������ � �������
	bool EverNeedUpdateUsable() const;

	/// �������� ��������� �������
	void Update(float tau);
	/// �������� ��������� �������
	void UpdateUsable(float tau);
	/// ��������� �� ��������� ����
	void EndTurn();
	/// ��������� ������ � ���, ��� �� ������ ����� ��������� �������
	void Destruct();

	/// ��������� �� ������
	bool Destroyed() const { return m_destroyed; }
	/// ����� �� ������
	bool Visible() const { return m_visible; }

	/// ���������� ��������� ������� (����� ��� �� �����)
	void SetVisibility(bool v);

	void Bake();

	/// �������� ������ � ������� n
	DynObject *GetObject(unsigned int n = 0) { return m_objects[n].get(); }
	const DynObject *GetObject(unsigned int n = 0) const { return m_objects[n].get(); }
	/// �������� ���������� ��������
	unsigned int GetObjectCount() const { return m_objectCount; }

	/// ������������������ ������ � ��������
	Iterator_t Begin() { return m_objects; }
	Iterator_t End() { return m_objects + m_objectCount; }
	ConstIterator_t Begin() const { return m_objects; }
	ConstIterator_t End() const { return m_objects + m_objectCount; }

	/// ���������� ����� �������
	const point3 &GetCenter() const { return m_center; }

	/// �������� ������ � �����
	void AddObject(HDynObject);

	/// ����������� ��������� �������
	float SwitchState(float time);
	/// ���������� ��������� �������
	float ChangeState(float state, float tau);

	/// ����� �� ������������ ������
	bool IsUsable() const;

	/// ������� ��� ������� ������� ��� ��������
	void OpenArea(bool open);
	bool IsOpen() const { return (*Begin())->m_animState > 0.5f; }

	void MakeSave(SavSlot &);
	void MakeLoad(SavSlot &);

public:
	using DynObjectSetInfo::GetName;

	bool IsDoor() const { return m_isDoor; }
	using DynObjectSetInfo::IsTrain;
	using DynObjectSetInfo::IsStorage;
	using DynObjectSetInfo::IsDatabase;
	using DynObjectSetInfo::IsElevator;
	using DynObjectSetInfo::IsDestructible;

	using DynObjectSetInfo::GetUseInfo;
	using DynObjectSetInfo::GetTrainInfo;
	using DynObjectSetInfo::GetStorageInfo;

private:
	DynObjectSet(const DynObjectSet &);
	DynObjectSet &operator=(const DynObjectSet &);
	void Clean();
	void CalcObjCenter();
	void InitProperties();
	StoredObjectInfo GetInfo() const;
};

typedef MlHandle<DynObjectSet> HDynObjectSet;

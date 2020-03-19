#pragma once

#include "DynObjectSet.h"

class DynObject;

//=====================================================================================//
//                                class ObjectProperty                                 //
//=====================================================================================//
/// ������������ ����� �������� �������, �������� "�����������", "����", "����", � �. �.
class ObjectProperty
{
	DynObjectSet *m_objects;

public:
	ObjectProperty(DynObjectSet *);
	virtual ~ObjectProperty();

	/// ���������� ����� ��������, � �������� �������� ��������
	DynObjectSet *GetObjects() { return m_objects; }
	const DynObjectSet *GetObjects() const { return m_objects; }

	/// ��������� ������ � ����
	virtual void MakeSave(SavSlot &slot) = 0;
	virtual void MakeLoad(SavSlot &slot) = 0;
};
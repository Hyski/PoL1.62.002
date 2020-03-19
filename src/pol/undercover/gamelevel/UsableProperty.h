#pragma once

#include "ObjectProperty.h"

//=====================================================================================//
//                    class UsableProperty : public ObjectProperty                     //
//=====================================================================================//
class UsableProperty : public ObjectProperty
{
public:
	UsableProperty(DynObjectSet *set);

	/// ����������� ��������� �������
	/** @param tau ������� ������� �����
		@return ���������� �����, � ������� ���������� ������������ ��������
		        ��������� ��������� */
	virtual float SwitchState(float tau) { return tau; }
	/// ���������� ��������� �������
	/** @param state �������������� ���������
		@param tau ������� ������� �����
		@return ���������� �����, � ������� ���������� ������������ ��������
		        ��������� ��������� */
	virtual float SetState(float state, float tau) { return tau; }
	/// �������� �� ������������ ���������?
	virtual bool CanSwitchState() const { return true; }
	/// �������� ��������� ������� � ����� � ���������� �������� �������
	virtual void Update(float tau) { }

	virtual void OnDestruct() { }

	/// ��������� ������ � ����
	virtual void MakeSave(SavSlot &slot) { }
	virtual void MakeLoad(SavSlot &slot) { }

	static bool CanPass(const ipnt2_t &);
};
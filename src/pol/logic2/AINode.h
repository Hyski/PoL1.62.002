#pragma once

//=====================================================================================//
//                                    class AINode                                     //
//=====================================================================================//
/// ��� AI ������������ � ���� ������ �� AINode
class AINode
{
public:
    virtual ~AINode() {}

    DCTOR_ABS_DEF(AINode)

    //� ���� ������� ���� ������ ��������
    virtual float Think(state_type* st) = 0;
    //����������/������������� AI
    virtual void MakeSaveLoad(SavSlot& st) = 0;
	// ������ ������ ��������� ����
	virtual float getComplexity() const = 0;

	// ������� ������������� ���������� � ����� ���������� ��������
	// ���������� ����, ���� ��� ����� � ���������
	virtual eid_t getEntityEID() const { return 0; }
	// ������ ����������� �� �������� ����� � �������� ID � ��������� ��
	// ������� ���� ����� �� ������ ������
	virtual void deleteChildren(eid_t ID) { }
	// ����� �� ������� ����
	virtual bool need2Delete() const { return false; }
	// ����!
	virtual bool die() { return false; }
};

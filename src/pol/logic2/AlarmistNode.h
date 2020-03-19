#pragma once

#include "AINode.h"

/////////////////////////////////////////////////////////////////////////////
//
// ���� ��� ��������, ������� ��������� ��������
//
/////////////////////////////////////////////////////////////////////////////
class AlarmistNode : public AINode
{
public:
    AlarmistNode(eid_t ent = 0);
    ~AlarmistNode();

    DCTOR_DEF(AlarmistNode)

    float Think(state_type* st);
    void MakeSaveLoad(SavSlot& st);

	// ������ ������ ��������� ����
	virtual float getComplexity() const { return 1.0f; }

	// ������� ������������� ���������� � ����� ���������� ��������
	// ���������� ����, ���� ��� ����� � ���������
	virtual eid_t getEntityEID() const { return m_entity; }

	// ����!
	virtual bool die() { return true; }

private:
    eid_t m_entity;
	bool m_initializing, m_startOfThink;
};

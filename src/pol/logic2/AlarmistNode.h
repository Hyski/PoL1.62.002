#pragma once

#include "AINode.h"

/////////////////////////////////////////////////////////////////////////////
//
// узел для человека, который постоянно паникует
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

	// расчет полной сложности узла
	virtual float getComplexity() const { return 1.0f; }

	// вернуть идентификатор связанного с узлом интеллекта существа
	// возвращает ноль, если нет связи с существом
	virtual eid_t getEntityEID() const { return m_entity; }

	// умри!
	virtual bool die() { return true; }

private:
    eid_t m_entity;
	bool m_initializing, m_startOfThink;
};

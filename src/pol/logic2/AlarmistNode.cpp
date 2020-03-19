#include "logicdefs.h"

#include "AIAPI.h"
#include "Entity.h"
#include "AIUtils.h"
#include "PanicPlayer.h"
#include "AlarmistNode.h"

DCTOR_IMP(AlarmistNode)

//=====================================================================================//
//                            AlarmistNode::AlarmistNode()                             //
//=====================================================================================//
AlarmistNode::AlarmistNode(eid_t ent)
:	m_entity(ent),
	m_initializing(false),
	m_startOfThink(true)
{
}

//=====================================================================================//
//                            AlarmistNode::~AlarmistNode()                            //
//=====================================================================================//
AlarmistNode::~AlarmistNode()
{
}

//=====================================================================================//
//                             float AlarmistNode::Think()                             //
//=====================================================================================//
float AlarmistNode::Think(state_type* st)
{
	if(!st) return 0.0f;

	if(m_entity) AIAPI::getPtr(m_entity)->Cast2Human()->SetAlwaysInPanic(true);

	if(m_startOfThink)
	{
		PanicPlayer::Init(AIAPI::getPtr(m_entity));
		m_initializing = true;
		m_startOfThink = false;
	}

	if(m_initializing)
	{
		if(UnlimitedMoves())
		{
			*st = ST_FINISHED;
			return 1.0f;
		}

		if(PanicPlayer::Execute())
		{
			*st = ST_INCOMPLETE;
			return 0.0f;
		}
		m_initializing = false;
	}

	*st = ST_FINISHED;
	m_startOfThink = true;
	return 1.0f;
}

//=====================================================================================//
//                          void AlarmistNode::MakeSaveLoad()                          //
//=====================================================================================//
void AlarmistNode::MakeSaveLoad(SavSlot& st)
{
	if(st.IsSaving())
	{
		st << m_entity;
	}
	else
	{
		st >> m_entity;
	}
}

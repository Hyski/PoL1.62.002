#pragma once

#include "AIAPI.h"
#include "AIEnemyNode.h"
#include "GameObserver.h"
#include "EnemyActivityObserver.h"

//=====================================================================================//
//                                class CommonEnemyNode                                //
//=====================================================================================//
class CommonEnemyNode
:	public AIEnemyNode,
	protected GameObserver
{
public:
	// ������������ ��������� ����
	enum TurnState
	{
		TS_NONE,
		TS_START,
		TS_INPROGRESS,
		TS_END
	};

	// ������������ ������� ���������
	enum LookroundState
	{
		LS_FIRST,
		LS_SECOND,
		LS_2BASE
	};

public:
	CommonEnemyNode(eid_t id);
	virtual ~CommonEnemyNode();

	// ������ ����������� �������
	virtual point3 getPosition() const;
	// ������� ������������� ���������� � ����� ���������� ��������
	// ���������� ����, ���� ��� ����� � ���������
	virtual eid_t getEntityEID() const { return m_entity; }

	bool ThinkShell(state_type* st);
	void CheckFinished(state_type *st,float *comp);
	bool needAndSeeMedkit(ipnt2_t* target);
	bool SelfCure();
	void SendCureMessage(float dist);
	bool PickUpWeaponAndAmmo(const ipnt2_t &m_target_pnt, WeaponComparator &comparator);
	bool MedkitTake(const ipnt2_t &m_target_pnt, state_type* st);

protected:
	ActivityHolder CreateMoveActivity(const pnt_vec_t& path, unsigned int flags);
	ActivityHolder CreateRotateActivity(float angle, unsigned int flags);
	void InitPanic();

protected:
	// ������� ��������� ����
	TurnState m_turn_state;
	// ������� ����������
	ActivityHolder m_activity;
	// ������� ��������� ���������
	LookroundState m_lookround_state;
	// ������������� ��������
	eid_t m_entity;
	// ������� ����, ��� ���� ��������� � �������� �������������
	bool m_initialising;

	EnemyActivityObserver m_activity_observer;	// ����������� �� ����������� ��������
};

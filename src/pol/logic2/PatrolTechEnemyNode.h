#pragma once

#include "AIEnemyNode.h"
#include "GameObserver.h"
#include "EnemyActivityObserver.h"

class Activity;

//=====================================================================================//
//                              class PatrolTechEnemyNode                              //
//=====================================================================================//
/// ���� ��� ���������� ��������� �������
class PatrolTechEnemyNode
:	public AIEnemyNode,
	private GameObserver
{
public:
	// ����������� - id ��������
	PatrolTechEnemyNode(eid_t id = 0);
	~PatrolTechEnemyNode();

	DCTOR_DEF(PatrolTechEnemyNode)

		float Think(state_type* st);
	void MakeSaveLoad(SavSlot& st);

	// ��������� ��������������� ���������
	virtual void recieveMessage(MessageType type, void * data);

	// ������ ����������� �������
	virtual point3 getPosition() const;

	// ������� ������������� ���������� � ����� ���������� ��������
	// ���������� ����, ���� ��� ����� � ���������
	virtual eid_t getEntityEID() const { return m_entity; }

	// ����!
	virtual bool die();
	// ����� �� ������� ����
	virtual bool need2Delete() const;
private:

	//��������� ������� ���������
	void Update(subject_t subj, event_t event, info_t ptr);

	//��������� ��������� � �����������
	void OnSpawnMsg() {}
	// ��������� ��������� �� �������� ��� ���������
	void OnShootEv(BaseEntity* who, BaseEntity* victim, const point3& where);

	// ��������� ������ Patrol
	bool thinkPatrol(state_type* st);
	// ��������� ������ Attack
	bool thinkAttack(state_type* st);
	// ��������� ������ Pursuit
	bool thinkPursuit(state_type* st);

	// ������������ �������, � ������� ����� ��������� ���������� ��������� �������
	enum PatrolTechEnemyMode
	{
		PTEM_PATROL,
		PTEM_ATTACK,
		PTEM_PURSUIT,
		PTEM_KILLED,
	};

	// ������������ ��������� ����
	enum TurnState
	{
		TS_NONE,
		TS_START,
		TS_INPROGRESS,
		TS_END
	};

	// ������� �����
	PatrolTechEnemyMode m_mode;
	// ������� ��������� ����
	TurnState m_turn_state;

	// ������������� ��������
	eid_t  m_entity;

	// ������� ����������
	Activity* m_activity;

	// ����������� �� ����������� ��������
	EnemyActivityObserver m_activity_observer;

	// ������������� �����, � ������� �������� ����������
	eid_t m_enemy_id;
	// ��������� �����, � ������� �������� ����������
	ipnt2_t m_enemy_pnt;
	// ��������� �����, ������ �� ��� ��������
	ipnt2_t m_shoot_pnt;
	// ����, ������������, ��� � ���� ��������
	bool m_shooted;
	// �������� �����
	ipnt2_t m_target_pnt;
	// �������� ����������� (����)
	float m_target_dir;
	// �����, ��� � ��������� ��� ����� �����
	ipnt2_t m_last_enemy_pnt;
	// ������� ����, ��� ���� ��������� � �������� �������������
	bool m_initialising;
	// ������ ������, ������� ���� ����� ������������ � ���� ����
	// ��-�� ���������� ����� ����
	typedef std::set<eid_t> EntityList_t;
	EntityList_t m_ignored_enemies;
};

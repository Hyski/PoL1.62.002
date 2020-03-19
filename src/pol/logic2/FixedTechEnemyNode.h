#pragma once

#include "AIEnemyNode.h"
#include "EnemyActivityObserver.h"

class Activity;

//=====================================================================================//
//                              class FixedTechEnemyNode                               //
//=====================================================================================//
/// ���� ��� ������������ ��������� �������
class FixedTechEnemyNode : public AIEnemyNode
{
public:
	// ����������� - id ��������
	FixedTechEnemyNode(eid_t id = 0);
	~FixedTechEnemyNode();

	DCTOR_DEF(FixedTechEnemyNode)

		float Think(state_type* st);
	void MakeSaveLoad(SavSlot& st);

	// ��������� ��������������� ���������
	virtual void recieveMessage(MessageType type, void * data) {}

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

	//��������� ��������� � �����������
	void OnSpawnMsg();

	// ��������� ������ Base
	bool thinkBase(state_type* st);
	// ��������� ������ Attack
	bool thinkAttack(state_type* st);
	// ��������� ������ Rotate2Base
	bool thinkRotate2Base(state_type* st);

	// ������������ �������, � ������� ����� ��������� ������������ ��������� �������
	enum FixedTechEnemyMode
	{
		FTEM_BASE,
		FTEM_ATTACK,
		FTEM_ROTATE2BASE,
		FTEM_KILLED,
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
	FixedTechEnemyMode m_mode;
	// ������� ��������� ����
	TurnState m_turn_state;

	// ������������� ��������
	eid_t  m_entity;

	// ������� ����������
	Activity* m_activity;

	// ����������� �� ����������� ��������
	EnemyActivityObserver m_activity_observer;

	// ������� ��������� ��������
	ipnt2_t m_basepnt;
	// ������� ����������� ��������
	float m_basedir;

	// ����������� �� �����, � �������� ����� �����������
	float m_target_dir;
	// ������� ����, ��� ���� ��������� � �������� �������������
	bool m_initialising;
	// ������ ������, ������� ���� ����� ������������ � ���� ����
	// ��-�� ���������� ����� ����
	typedef std::set<eid_t> EntityList_t;
	EntityList_t m_ignored_enemies;
};

#pragma once

#include "CommonEnemyNode.h"

//=====================================================================================//
//                                class PatrolEnemyNode                                //
//=====================================================================================//
/// ���� ��� ����������� �����
class PatrolEnemyNode : public CommonEnemyNode
{
public:
	// ����������� - id ��������
	PatrolEnemyNode(eid_t id = 0);
	~PatrolEnemyNode();

	DCTOR_DEF(PatrolEnemyNode)

	float Think(state_type* st);
	void MakeSaveLoad(SavSlot& st);

	// ��������� ��������������� ���������
	virtual void recieveMessage(MessageType type, void * data);

	virtual bool die();
	virtual bool need2Delete() const;

private:
	//��������� ������� ���������
	void Update(subject_t subj, event_t event, info_t ptr);

	void OnSpawnMsg() {}	//��������� ��������� � �����������
	// ��������� ��������� �� �������� ��� ���������
	void OnShootEv(BaseEntity* who, BaseEntity* victim, const point3& where);
	void OnKillEv(BaseEntity* killer, BaseEntity* victim);//��������� �������� �� ��������

	// ������� � ����� � ������������� �� AEM_ATTACK
	// ���������� true, ���� ������� �����
	bool switchToAttack(eid_t enemy);

	bool thinkPatrol(state_type* st);		// ��������� ������ Patrol
	bool thinkAttack(state_type* st);		// ��������� ������ Attack
	bool thinkWeaponSearch(state_type* st);	// ��������� ������ WeaponSearch
	bool thinkThingTake(state_type* st);	// ��������� ������ ThingTake
	bool thinkWeaponTake(state_type* st);	// ��������� ������ WeaponTake
	bool thinkCure(state_type* st);			// ��������� ������ Cure
	bool thinkPursuit(state_type* st);		// ��������� ������ Pursuit

	// ������������ �������, � ������� ����� ��������� ������������ ����
	enum PatrolEnemyMode
	{
		PEM_PATROL,
		PEM_ATTACK,
		PEM_WEAPONSEARCH,
		PEM_THINGTAKE,
		PEM_WEAPONTAKE,
		PEM_CURE,
		PEM_PURSUIT,
		PEM_KILLED,
	};

	PatrolEnemyMode m_mode;		// ������� �����

	eid_t m_enemy_id;		// ������������� �����, � ������� �������� ����������
	ipnt2_t m_enemy_pnt;	// ��������� �����, � ������� �������� ����������
	eid_t m_cure_id;		// ������������� ����� �� ����� ����������, �������� ����� ������
	ipnt2_t m_cure_pnt;		// ��������� ����� �� ����� ����������, �������� ����� ������
	ipnt2_t m_shoot_pnt;	// ��������� �����, ������ �� ��� ��������
	bool m_shooted;			// ����, ������������, ��� � ���� ��������
	ipnt2_t m_target_pnt;	// �������� �����
	float m_target_dir;		// �������� ����������� (����)
	ipnt2_t m_last_enemy_pnt;	// �����, ��� � ��������� ��� ����� �����

	typedef std::set<eid_t> EntityList_t;
	EntityList_t m_ignored_enemies;		  	// ������ ������, ������� ���� ����� ������������ � ���� ����
	// ��-�� ���������� ����� ����
};

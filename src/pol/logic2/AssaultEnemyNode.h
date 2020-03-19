#pragma once

#include "CommonEnemyNode.h"

//=====================================================================================//
//                               class AssaultEnemyNode                                //
//=====================================================================================//
/// ���� ��� ����������
class AssaultEnemyNode : public CommonEnemyNode
{
public:
	// ����������� - id ��������
	AssaultEnemyNode(eid_t id = 0);
	AssaultEnemyNode(eid_t id, const std::string &ptsid);
	~AssaultEnemyNode();

	DCTOR_DEF(AssaultEnemyNode)

	float Think(state_type* st);
	void MakeSaveLoad(SavSlot& st);

	// ��������� ��������������� ���������
	virtual void recieveMessage(MessageType type, void * data);

	virtual bool die();
	virtual bool need2Delete() const;

private:
	void Update(subject_t subj, event_t event, info_t ptr);	//��������� ������� ���������
	void OnSpawnMsg() {}	// ��������� ��������� � �����������
	void OnShootEv(BaseEntity* who, BaseEntity* victim, const point3& where);	// ��������� ��������� �� �������� ��� ���������
	void OnKillEv(BaseEntity* killer, BaseEntity* victim);//��������� �������� �� ��������

	bool thinkFn(state_type *st) { return (this->*m_modes[m_mode])(st); }

	bool thinkFixed(state_type *st);			// ��������� ������ Fixed
	bool thinkLookround(state_type *st);		// ��������� ������ Lookround
	bool thinkAttack(state_type *st);			// ��������� ������ Attack
	bool thinkWeaponSearch(state_type *st);		// ��������� ������ WeaponSearch
	bool thinkWeaponTake(state_type *st);		// ��������� ������ WeaponTake
	bool thinkPursuit(state_type *st);			// ��������� ������ Pursuit
	bool thinkMedkitTake(state_type *st);		// ��������� ������ MedkitTake
	bool thinkKilled(state_type *st) { return false; }
	bool thinkSeek(state_type *st);

	// ������� � ����� � ������������� �� AEM_ATTACK
	// ���������� true, ���� ������� �����
	bool switchToAttack(eid_t enemy);

	// ������� � ������, ����� ��������� ���
	// ���������� true � ������ ������, � false, ���� ���� ���
	bool comeToWeapon();
	// ���������� �������� �� �����
	// ���������� true, ���� �������� ����
	bool processShootInMe();
	// ���������� ��������� � �����
	// ���������� true, ���� ����� ��������� ����� �����
	bool processEnemySpotted();

	// ������������ �������, � ������� ����� ��������� ������������ ����
	enum AssaultEnemyMode
	{
		AEM_FIXED,
		AEM_LOOKROUND,
		AEM_ATTACK,
		AEM_WEAPONSEARCH,
		AEM_WEAPONTAKE,
		AEM_PURSUIT,
		AEM_KILLED,
		AEM_MEDKITTAKE,
		AEM_SEEK,
		AEM_COUNT
	};

	typedef bool (AssaultEnemyNode::*ThinkFn_t)(state_type *);
	static ThinkFn_t m_modes[AEM_COUNT];

	AssaultEnemyMode m_mode;	// ������� �����

	int m_turn;				// ����� ����� � ���������, ����� ���� �� �����
	eid_t m_enemy_id;		// ������������� �����, � ������� �������� ����������
	ipnt2_t m_enemy_pnt;	// ��������� �����, � ������� �������� ����������
	ipnt2_t m_shoot_pnt;	// ��������� �����, ������ �� ��� ��������
	bool m_shooted;			// ����, ������������, ��� � ���� ��������
	ipnt2_t m_target_pnt;	// �������� �����
	float m_target_dir;		// �������� ����������� (����)
	ipnt2_t m_last_enemy_pnt;	// �����, ��� � ��������� ��� ����� �����
	float m_prev_dir;		// ���������� ����������� ����� ��� ��������� ������ ���������

	typedef std::set<eid_t> EntityList_t;	
	EntityList_t m_ignored_enemies;			// ������ ������, ������� ���� ����� ������������ � ���� ����
	// ��-�� ���������� ����� ����

	std::string m_sysname;
	std::list<ipnt2_t> m_ptlist;

	int getModeInt() const;
	void setModeInt(int);

	bool tryToSeek();

};

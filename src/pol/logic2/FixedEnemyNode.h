#pragma once

#include "CommonEnemyNode.h"

//=====================================================================================//
//                                class FixedEnemyNode                                 //
//=====================================================================================//
/// ���� ��� ������������� �����
class FixedEnemyNode : public CommonEnemyNode
{
public:
	// ����������� - id ��������
	FixedEnemyNode(eid_t id = 0);
	~FixedEnemyNode();

	DCTOR_DEF(FixedEnemyNode)

	float Think(state_type* st);
	void MakeSaveLoad(SavSlot& st);

	// ��������� ��������������� ���������
	virtual void recieveMessage(MessageType type, void * data) {}

	virtual bool die();
	virtual bool need2Delete() const;
private:

	//��������� ������� ���������
	void Update(subject_t subj, event_t event, info_t info);
	void OnSpawnMsg();	//��������� ��������� � �����������

	// ��������� ��������� �� �������� ��� ���������
	void OnShootEv(BaseEntity* who, BaseEntity* victim, const point3& where);

	bool thinkBase(state_type* st);				// ��������� ������ Base
	bool thinkLookRound(state_type* st);		// ��������� ������ LookRound
	bool thinkAttack(state_type* st);			// ��������� ������ Attack
	bool thinkReturn2Base(state_type* st);		// ��������� ������ Return2Base
	bool thinkWeaponSearch(state_type* st);		// ��������� ������ WeaponSearch
	bool thinkWeaponTake(state_type* st);		// ��������� ������ WeaponTake
	bool thinkMedkitTake(state_type* st);		// ��������� ������ MedkitTake

	// ������������ �������, � ������� ����� ��������� ������������ ����
	enum FixedEnemyMode
	{
		FEM_BASE,
		FEM_LOOKROUND,
		FEM_ATTACK,
		FEM_RETURN2BASE,
		FEM_WEAPONSEARCH,
		FEM_WEAPONTAKE,
		FEM_KILLED,
		FEM_MEDKITTAKE,
	};

	// ������� �����
	FixedEnemyMode m_mode;

	ipnt2_t m_basepnt;			// ������� ��������� ��������
	float m_basedir;			// ������� ����������� ��������

	int m_turn;				// ����� ����� � ���������, ����� ���� �� �����
	float m_target_dir;		// ����������� �� �����, � �������� ����� �����������
	ipnt2_t m_target_pnt;	// �����, ���� ����� ����� ��� ���� ����� �����������
	float m_prev_dir;		// ���������� ����������� ����� ��� ��������� ������ ���������
	pnt_vec_t m_base_field;	// ������ ������ �������� ����
	typedef std::set<eid_t> EntityList_t;
	EntityList_t m_ignored_enemies;		  	// ������ ������, ������� ���� ����� ������������ � ���� ����  ��-�� ���������� ����� ����

};

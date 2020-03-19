#pragma once

#include "CommonEnemyNode.h"

//=====================================================================================//
//                                class FixedEnemyNode                                 //
//=====================================================================================//
/// узел для стационарного врага
class FixedEnemyNode : public CommonEnemyNode
{
public:
	// конструктор - id существа
	FixedEnemyNode(eid_t id = 0);
	~FixedEnemyNode();

	DCTOR_DEF(FixedEnemyNode)

	float Think(state_type* st);
	void MakeSaveLoad(SavSlot& st);

	// обработка внутрикомандных сообщений
	virtual void recieveMessage(MessageType type, void * data) {}

	virtual bool die();
	virtual bool need2Delete() const;
private:

	//обработка игровых сообщений
	void Update(subject_t subj, event_t event, info_t info);
	void OnSpawnMsg();	//обработка сообщения о расстановке

	// обработка сообщения об выстреле или попадании
	void OnShootEv(BaseEntity* who, BaseEntity* victim, const point3& where);

	bool thinkBase(state_type* st);				// обработка режима Base
	bool thinkLookRound(state_type* st);		// обработка режима LookRound
	bool thinkAttack(state_type* st);			// обработка режима Attack
	bool thinkReturn2Base(state_type* st);		// обработка режима Return2Base
	bool thinkWeaponSearch(state_type* st);		// обработка режима WeaponSearch
	bool thinkWeaponTake(state_type* st);		// обработка режима WeaponTake
	bool thinkMedkitTake(state_type* st);		// обработка режима MedkitTake

	// перечисление режимов, в которых может находится стационарный враг
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

	// текущий режим
	FixedEnemyMode m_mode;

	ipnt2_t m_basepnt;			// базовое положение существа
	float m_basedir;			// базовое направление существа

	int m_turn;				// номер турна в состоянии, когда враг не виден
	float m_target_dir;		// направление на врага, к которому нужно повернуться
	ipnt2_t m_target_pnt;	// точка, куда нужно дойти или куда нужно повернуться
	float m_prev_dir;		// предыдущее направление юнита при очередном приеме разворота
	pnt_vec_t m_base_field;	// вектор хексов базового поля
	typedef std::set<eid_t> EntityList_t;
	EntityList_t m_ignored_enemies;		  	// список врагов, которых юнит будет игнорировать в этом туре  из-за отсутствия линии огня

};

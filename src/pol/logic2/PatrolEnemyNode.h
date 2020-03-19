#pragma once

#include "CommonEnemyNode.h"

//=====================================================================================//
//                                class PatrolEnemyNode                                //
//=====================================================================================//
/// узел для патрульного врага
class PatrolEnemyNode : public CommonEnemyNode
{
public:
	// конструктор - id существа
	PatrolEnemyNode(eid_t id = 0);
	~PatrolEnemyNode();

	DCTOR_DEF(PatrolEnemyNode)

	float Think(state_type* st);
	void MakeSaveLoad(SavSlot& st);

	// обработка внутрикомандных сообщений
	virtual void recieveMessage(MessageType type, void * data);

	virtual bool die();
	virtual bool need2Delete() const;

private:
	//обработка игровых сообщений
	void Update(subject_t subj, event_t event, info_t ptr);

	void OnSpawnMsg() {}	//обработка сообщения о расстановке
	// обработка сообщения об выстреле или попадании
	void OnShootEv(BaseEntity* who, BaseEntity* victim, const point3& where);
	void OnKillEv(BaseEntity* killer, BaseEntity* victim);//обработка собщения об убийстве

	// Подойти к врагу и переключиться на AEM_ATTACK
	// Возвращает true, если подойти можно
	bool switchToAttack(eid_t enemy);

	bool thinkPatrol(state_type* st);		// обработка режима Patrol
	bool thinkAttack(state_type* st);		// обработка режима Attack
	bool thinkWeaponSearch(state_type* st);	// обработка режима WeaponSearch
	bool thinkThingTake(state_type* st);	// обработка режима ThingTake
	bool thinkWeaponTake(state_type* st);	// обработка режима WeaponTake
	bool thinkCure(state_type* st);			// обработка режима Cure
	bool thinkPursuit(state_type* st);		// обработка режима Pursuit

	// перечисление режимов, в которых может находится стационарный враг
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

	PatrolEnemyMode m_mode;		// текущий режим

	eid_t m_enemy_id;		// идентификатор врага, о котором сообщила подкоманда
	ipnt2_t m_enemy_pnt;	// положение врага, о котором сообщила подкоманда
	eid_t m_cure_id;		// идентификатор юнита из своей подкоманды, которого нужно лечить
	ipnt2_t m_cure_pnt;		// положение юнита из своей подкоманды, которого нужно лечить
	ipnt2_t m_shoot_pnt;	// положение точки, откуда по мне стреляли
	bool m_shooted;			// флаг, показывающий, что в меня стреляля
	ipnt2_t m_target_pnt;	// заданная точка
	float m_target_dir;		// заданное направление (угол)
	ipnt2_t m_last_enemy_pnt;	// точка, где я последний раз видел врага

	typedef std::set<eid_t> EntityList_t;
	EntityList_t m_ignored_enemies;		  	// список врагов, которых юнит будет игнорировать в этом туре
	// из-за отсутствия линии огня
};

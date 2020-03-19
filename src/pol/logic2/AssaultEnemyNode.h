#pragma once

#include "CommonEnemyNode.h"

//=====================================================================================//
//                               class AssaultEnemyNode                                //
//=====================================================================================//
/// узел для штурмовика
class AssaultEnemyNode : public CommonEnemyNode
{
public:
	// конструктор - id существа
	AssaultEnemyNode(eid_t id = 0);
	AssaultEnemyNode(eid_t id, const std::string &ptsid);
	~AssaultEnemyNode();

	DCTOR_DEF(AssaultEnemyNode)

	float Think(state_type* st);
	void MakeSaveLoad(SavSlot& st);

	// обработка внутрикомандных сообщений
	virtual void recieveMessage(MessageType type, void * data);

	virtual bool die();
	virtual bool need2Delete() const;

private:
	void Update(subject_t subj, event_t event, info_t ptr);	//обработка игровых сообщений
	void OnSpawnMsg() {}	// обработка сообщения о расстановке
	void OnShootEv(BaseEntity* who, BaseEntity* victim, const point3& where);	// обработка сообщения об выстреле или попадании
	void OnKillEv(BaseEntity* killer, BaseEntity* victim);//обработка собщения об убийстве

	bool thinkFn(state_type *st) { return (this->*m_modes[m_mode])(st); }

	bool thinkFixed(state_type *st);			// обработка режима Fixed
	bool thinkLookround(state_type *st);		// обработка режима Lookround
	bool thinkAttack(state_type *st);			// обработка режима Attack
	bool thinkWeaponSearch(state_type *st);		// обработка режима WeaponSearch
	bool thinkWeaponTake(state_type *st);		// обработка режима WeaponTake
	bool thinkPursuit(state_type *st);			// обработка режима Pursuit
	bool thinkMedkitTake(state_type *st);		// обработка режима MedkitTake
	bool thinkKilled(state_type *st) { return false; }
	bool thinkSeek(state_type *st);

	// Подойти к врагу и переключиться на AEM_ATTACK
	// Возвращает true, если подойти можно
	bool switchToAttack(eid_t enemy);

	// Подойти к оружию, чтобы подобрать его
	// возвращает true в случае успеха, и false, если пути нет
	bool comeToWeapon();
	// Обработать стрельбу по юниту
	// возвращает true, если стрельба была
	bool processShootInMe();
	// Обработать сообщение о враге
	// возвращает true, если такое сообщение имело место
	bool processEnemySpotted();

	// перечисление режимов, в которых может находится стационарный враг
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

	AssaultEnemyMode m_mode;	// текущий режим

	int m_turn;				// номер турна в состоянии, когда враг не виден
	eid_t m_enemy_id;		// идентификатор врага, о котором сообщила подкоманда
	ipnt2_t m_enemy_pnt;	// положение врага, о котором сообщила подкоманда
	ipnt2_t m_shoot_pnt;	// положение точки, откуда по мне стреляли
	bool m_shooted;			// флаг, показывающий, что в меня стреляля
	ipnt2_t m_target_pnt;	// заданная точка
	float m_target_dir;		// заданное направление (угол)
	ipnt2_t m_last_enemy_pnt;	// точка, где я последний раз видел врага
	float m_prev_dir;		// предыдущее направление юнита при очередном приеме разворота

	typedef std::set<eid_t> EntityList_t;	
	EntityList_t m_ignored_enemies;			// список врагов, которых юнит будет игнорировать в этом туре
	// из-за отсутствия линии огня

	std::string m_sysname;
	std::list<ipnt2_t> m_ptlist;

	int getModeInt() const;
	void setModeInt(int);

	bool tryToSeek();

};

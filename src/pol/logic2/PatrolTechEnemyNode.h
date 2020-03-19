#pragma once

#include "AIEnemyNode.h"
#include "GameObserver.h"
#include "EnemyActivityObserver.h"

class Activity;

//=====================================================================================//
//                              class PatrolTechEnemyNode                              //
//=====================================================================================//
/// узел для патрульной вражеской техники
class PatrolTechEnemyNode
:	public AIEnemyNode,
	private GameObserver
{
public:
	// конструктор - id существа
	PatrolTechEnemyNode(eid_t id = 0);
	~PatrolTechEnemyNode();

	DCTOR_DEF(PatrolTechEnemyNode)

		float Think(state_type* st);
	void MakeSaveLoad(SavSlot& st);

	// обработка внутрикомандных сообщений
	virtual void recieveMessage(MessageType type, void * data);

	// выдать собственную позицию
	virtual point3 getPosition() const;

	// вернуть идентификатор связанного с узлом интеллекта существа
	// возвращает ноль, если нет связи с существом
	virtual eid_t getEntityEID() const { return m_entity; }

	// умри!
	virtual bool die();
	// нужно ли удалять узел
	virtual bool need2Delete() const;
private:

	//обработка игровых сообщений
	void Update(subject_t subj, event_t event, info_t ptr);

	//обработка сообщения о расстановке
	void OnSpawnMsg() {}
	// обработка сообщения об выстреле или попадании
	void OnShootEv(BaseEntity* who, BaseEntity* victim, const point3& where);

	// обработка режима Patrol
	bool thinkPatrol(state_type* st);
	// обработка режима Attack
	bool thinkAttack(state_type* st);
	// обработка режима Pursuit
	bool thinkPursuit(state_type* st);

	// перечисление режимов, в которых может находится патрульная вражеская техника
	enum PatrolTechEnemyMode
	{
		PTEM_PATROL,
		PTEM_ATTACK,
		PTEM_PURSUIT,
		PTEM_KILLED,
	};

	// перечисление состояний тура
	enum TurnState
	{
		TS_NONE,
		TS_START,
		TS_INPROGRESS,
		TS_END
	};

	// текущий режим
	PatrolTechEnemyMode m_mode;
	// текущее состояние тура
	TurnState m_turn_state;

	// идентификатор существа
	eid_t  m_entity;

	// текущая активность
	Activity* m_activity;

	// наблюдатель за активностью существа
	EnemyActivityObserver m_activity_observer;

	// идентификатор врага, о котором сообщила подкоманда
	eid_t m_enemy_id;
	// положение врага, о котором сообщила подкоманда
	ipnt2_t m_enemy_pnt;
	// положение точки, откуда по мне стреляли
	ipnt2_t m_shoot_pnt;
	// флаг, показывающий, что в меня стреляля
	bool m_shooted;
	// заданная точка
	ipnt2_t m_target_pnt;
	// заданное направление (угол)
	float m_target_dir;
	// точка, где я последний раз видел врага
	ipnt2_t m_last_enemy_pnt;
	// признак того, что юнит находится в процессе инициализации
	bool m_initialising;
	// список врагов, которых юнит будет игнорировать в этом туре
	// из-за отсутствия линии огня
	typedef std::set<eid_t> EntityList_t;
	EntityList_t m_ignored_enemies;
};

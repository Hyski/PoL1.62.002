#pragma once

#include "AIEnemyNode.h"
#include "EnemyActivityObserver.h"

class Activity;

//=====================================================================================//
//                              class FixedTechEnemyNode                               //
//=====================================================================================//
/// узел для стационарной вражеской техники
class FixedTechEnemyNode : public AIEnemyNode
{
public:
	// конструктор - id существа
	FixedTechEnemyNode(eid_t id = 0);
	~FixedTechEnemyNode();

	DCTOR_DEF(FixedTechEnemyNode)

		float Think(state_type* st);
	void MakeSaveLoad(SavSlot& st);

	// обработка внутрикомандных сообщений
	virtual void recieveMessage(MessageType type, void * data) {}

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

	//обработка сообщения о расстановке
	void OnSpawnMsg();

	// обработка режима Base
	bool thinkBase(state_type* st);
	// обработка режима Attack
	bool thinkAttack(state_type* st);
	// обработка режима Rotate2Base
	bool thinkRotate2Base(state_type* st);

	// перечисление режимов, в которых может находится стационарная вражеская техника
	enum FixedTechEnemyMode
	{
		FTEM_BASE,
		FTEM_ATTACK,
		FTEM_ROTATE2BASE,
		FTEM_KILLED,
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
	FixedTechEnemyMode m_mode;
	// текущее состояние тура
	TurnState m_turn_state;

	// идентификатор существа
	eid_t  m_entity;

	// текущая активность
	Activity* m_activity;

	// наблюдатель за активностью существа
	EnemyActivityObserver m_activity_observer;

	// базовое положение существа
	ipnt2_t m_basepnt;
	// базовое направление существа
	float m_basedir;

	// направление на врага, к которому нужно повернуться
	float m_target_dir;
	// признак того, что юнит находится в процессе инициализации
	bool m_initialising;
	// список врагов, которых юнит будет игнорировать в этом туре
	// из-за отсутствия линии огня
	typedef std::set<eid_t> EntityList_t;
	EntityList_t m_ignored_enemies;
};

#include "logicdefs.h"

#include "AIAPI.h"
#include "AIUtils.h"
#include "PathUtils.h"
#include "SubteamNode.h"
#include "PanicPlayer.h"
#include "FixedTechEnemyNode.h"

DCTOR_IMP(FixedTechEnemyNode)

//=====================================================================================//
//                      FixedTechEnemyNode::FixedTechEnemyNode()                       //
//=====================================================================================//
// конструктор - id существа
FixedTechEnemyNode::FixedTechEnemyNode(eid_t id)
:	m_mode(FTEM_BASE),
	m_turn_state(TS_NONE),
	m_entity(id),
	m_activity(0),
	m_basepnt(0, 0),
	m_basedir(0),
	m_target_dir(0),
	m_initialising(false)
{
	if(m_entity) OnSpawnMsg();
}

//=====================================================================================//
//                      FixedTechEnemyNode::~FixedTechEnemyNode()                      //
//=====================================================================================//
FixedTechEnemyNode::~FixedTechEnemyNode()
{
	delete m_activity;
}

//=====================================================================================//
//                          float FixedTechEnemyNode::Think()                          //
//=====================================================================================//
float FixedTechEnemyNode::Think(state_type* st)
{
	float complexity = 0.0f;
    //проиграть действие
    if(m_activity){

        if(st) *st = ST_INCOMPLETE;

        if(!m_activity->Run(AC_TICK)){
			m_activity->Detach(&m_activity_observer);
            delete m_activity;
            m_activity = 0;
        }
        return complexity;
    }

	// если не наш ход - на выход
	if(!st)
	{
		m_turn_state = TS_NONE;
		return complexity;
	}

	if(UnlimitedMoves())
	{
		if(st) *st = ST_FINISHED;
		return 1.0f;
	}

	// если m_turn_state == TS_NONE, то это начало нашего хода
	if(m_turn_state == TS_NONE)
	{
		m_turn_state = TS_START;
		// начнем инициализацию
		PanicPlayer::Init(AIAPI::getPtr(m_entity));
		m_initialising = true;
	}

	// проверим флаг инициализации
	if(m_initialising)
	{
		if(PanicPlayer::Execute())
		{
			// нужно продолжать процесс инициализации
			*st = ST_INCOMPLETE;
			return complexity;
		}
		// инициализация закончена
		m_initialising = false;
	}

    // юнит свободен - можно подумать
	bool flag = true;
    while(flag)
	{
        switch(m_mode)
		{
        case FTEM_BASE:    flag = thinkBase(st);         break;
		case FTEM_ATTACK:  flag = thinkAttack(st);       break;
		case FTEM_ROTATE2BASE:  flag = thinkRotate2Base(st);     break;
        }
    }

	if(m_turn_state == TS_END)
	{
		*st = ST_FINISHED;
		m_turn_state = TS_START;
		complexity = 1.0f;
	}
	else *st = ST_INCOMPLETE;
	return complexity;
}

//=====================================================================================//
//                       void FixedTechEnemyNode::MakeSaveLoad()                       //
//=====================================================================================//
void FixedTechEnemyNode::MakeSaveLoad(SavSlot& st)
{
    if(st.IsSaving())
	{
		// сохраняемся
        st << static_cast<int>(m_mode);
        st << static_cast<int>(m_turn_state);
		st << m_entity;
		st << static_cast<int>(m_activity_observer.getLastEvent());

		st << m_basepnt.x << m_basepnt.y << m_basedir << m_target_dir;
    }
	else
	{
		// читаемся
		int tmp;
		st >> tmp; m_mode = static_cast<FixedTechEnemyMode>(tmp);
		st >> tmp; m_turn_state = static_cast<TurnState>(tmp);
		st >> m_entity;
		st >> tmp; m_activity_observer.setLastEvent(static_cast<ActivityObserver::event_type>(tmp));

		st >> m_basepnt.x >> m_basepnt.y >> m_basedir >> m_target_dir;
		
		// то, что восстанавливается без чтения
		m_activity = 0;
		m_initialising = false;
	}
}

//=====================================================================================//
//                   point3 FixedTechEnemyNode::getPosition() const                    //
//=====================================================================================//
// выдать собственную позицию
point3 FixedTechEnemyNode::getPosition() const
{
	return AIAPI::getPos3(m_entity);
}

//=====================================================================================//
//                        void FixedTechEnemyNode::OnSpawnMsg()                        //
//=====================================================================================//
//обработка сообщения о расстановке
void FixedTechEnemyNode::OnSpawnMsg()
{
	m_basepnt = AIAPI::getPos2(m_entity);
	m_basedir = norm_angle(AIAPI::getAngle(m_entity));
}

//=====================================================================================//
//                        bool FixedTechEnemyNode::thinkBase()                         //
//=====================================================================================//
// обработка режима Base
bool FixedTechEnemyNode::thinkBase(state_type* st)
{
	// 1. если я вижу врага - пошлю сообщение и перейду в атаку
	AIAPI::entity_list_t enemies;
	if(AIAPI::getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// я кого-то вижу
		if(eid_t enemy = AIAPI::getMostDangerousEnemy(enemies,
			EnemyComparator(m_entity))	)
		{
			AIAPI::print(m_entity, "|FT|thinkBase: я увидел врага!");
			// выбрал самого опасного врага, пошлю сообщение своей команде
			getSubteam()->sendMessage(this,	AIEnemyNode::MT_ENEMYSPOTTED, &enemy, fixed_tech_enemy_dist);
			// переход в состояние атаки
			m_mode = FTEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			return false;
		}
	}

	// 2. если моя подкоманда засекла врага - перейду в атаку
	enemies.clear();
	if(AIAPI::getEnemiesVisibleBySubteam(getSubteam()->getName(),&enemies) )
	{
		// засекла - выберем самого опасного
		if(eid_t enemy = AIAPI::getMostDangerousEnemy(enemies,
			EnemyComparator(m_entity))	)
		{
			AIAPI::print(m_entity, "|FT|thinkBase: моя команда засекла врага!");
			// выбрал самого опасного врага, переход в состояние атаки
			m_mode = FTEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			return false;
		}
	}

	// 3. никого нет
	AIAPI::print(m_entity, "|FT|thinkBase: никого нет");
	m_turn_state = TS_END;
	return false;
}

//=====================================================================================//
//                       bool FixedTechEnemyNode::thinkAttack()                        //
//=====================================================================================//
// обработка режима Attack
bool FixedTechEnemyNode::thinkAttack(state_type* st)
{
	// 1. выберем врага
	AIAPI::entity_list_t enemies;
	eid_t enemy = 0;
	if(AIAPI::getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// сама видит врага
		// если это начало хода или реакция - сбросим список игнорирования
		if( (m_turn_state == TS_START) ||(m_activity_observer.getLastEvent() == ActivityObserver::EV_ENEMY_REACTED)	)
		{
			m_ignored_enemies.clear();
		}
		// попробуем найти разность списков всех врагов и игнорированных врагов
		EntityList_t diff_list;
		setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);
		if(diff_list.empty())
		{
			// выберем самого опасного врага из всех
			enemy = AIAPI::getMostDangerousEnemy(enemies,EnemyComparator(m_entity));
		}
		else
		{
			// выберем самого опасного врага из непроигнорированных
			enemy = AIAPI::getMostDangerousEnemy(diff_list, EnemyComparator(m_entity));
		}
	}
	
	if(!enemy)
	{
		// самa врага не видит
		enemies.clear();
		if(AIAPI::getEnemiesVisibleBySubteam(getSubteam()->getName(), &enemies) 	)
		{
			// зато видит ее подкоманда
			// если это начало хода или реакция - сбросим список игнорирования
			if( (m_turn_state == TS_START) ||(m_activity_observer.getLastEvent() == ActivityObserver::EV_ENEMY_REACTED) 	)
			{
				m_ignored_enemies.clear();
			}
			// попробуем найти разность списков всех врагов и игнорированных врагов
			EntityList_t diff_list;
			setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);
			if(diff_list.empty())
			{
				// выберем самого опасного врага из всех
				enemy = AIAPI::getMostDangerousEnemy(enemies, EnemyComparator(m_entity));
			}
			else
			{
				// выберем самого опасного врага из непроигнорированных
				enemy = AIAPI::getMostDangerousEnemy(diff_list, EnemyComparator(m_entity));
			}
		}
	}
	
	// 2. если нет врагов - разворот в базовое направление
	if(!enemy)
	{
		// больше нет врагов - разворот в базовое направление
		AIAPI::print(m_entity, "|FT|thinkAttack: врагов нет - на базу");
		m_mode = FTEM_ROTATE2BASE;
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	// враги есть...

	// 3. если технике не хватило мувпоинтов - онa передает ход
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// закончились - закончим ход
		AIAPI::print(m_entity, "|FT|thinkAttack: закончились мувпоинты");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 4. если нет линии огня - добавляем врага в список игнорирования
	// причем, если после этого все враги оказались в списке игнорирования
	// передаем ход
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_NO_LOF)
	{
		// закончились - закончим ход
		AIAPI::print(m_entity, "|FT|thinkAttack: нет линии огня - добавляем врага в список игнорирования");
		m_activity_observer.clear();
		// добавим в список игнорирования
		m_ignored_enemies.insert(enemy);
		EntityList_t diff_list;
		setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);
		if(diff_list.empty())
		{
			// передаем ход
			m_turn_state = TS_END;
			return false;
		}
		// зациклимся для поиска других врагов
		m_turn_state = TS_INPROGRESS;
		return true;
	}


	// 5 Если не повернут к врагу - поворачивается в направлении где был враг
	m_target_dir = norm_angle(Dir2Angle(AIAPI::getPos3(enemy) - getPosition()));
	float cur_angle = norm_angle(AIAPI::getAngle(m_entity));
	if( fabs(m_target_dir - cur_angle) >= angle_eps )
	{
		// нужно поворачиваться к врагу
		m_activity = ActivityFactory::CreateRotate(AIAPI::getPtr(m_entity),m_target_dir, 	ActivityFactory::CT_ENEMY_ROTATE);
		m_activity->Attach(&m_activity_observer,	ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	
	// 6. повернутa к врагу - если сможем будем атаковать
	AIAPI::print(m_entity, "|FT|thinkAttack: будем стрелять, если это возможно!");
	// враг есть - выстрелим по нему если это возможно
	std::string reason;
	if(AIAPI::shootByVehicle(m_entity, enemy, &m_activity, &reason))
	{
		m_turn_state = TS_INPROGRESS;
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_NO_LOF);
	}
	else
	{
		std::string str = "|FT|thinkAttack: стрелять нельзя: ";
		str += reason;
		AIAPI::print(m_entity, str.c_str());
		m_turn_state = TS_END;
	}
	return false;
}

//=====================================================================================//
//                     bool FixedTechEnemyNode::thinkRotate2Base()                     //
//=====================================================================================//
// обработка режима Rotate2Base
bool FixedTechEnemyNode::thinkRotate2Base(state_type* st)
{
	// 1. проверим развернута ли техника в базовом направлении
	float cur_angle = AIAPI::getAngle(m_entity);
	if( fabs(m_basedir - cur_angle) < angle_eps )
	{
		// развернута - перейдем в базовое состояние
		AIAPI::print(m_entity, "|FT|thinkRotate2Base: развернута");
		m_mode = FTEM_BASE;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 2. проверим мувпоинты
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// закончились - закончим ход
		AIAPI::print(m_entity, "|FT|thinkRotate2Base: закончились мувпоинты");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 3. разворот в базовое направление
	AIAPI::print(m_entity, "|FT|thinkRotate2Base: разворачиваемся...");
	m_activity = ActivityFactory::CreateRotate(AIAPI::getPtr(m_entity),m_basedir,	ActivityFactory::CT_ENEMY_ROTATE);
	m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

//=====================================================================================//
//                           bool FixedTechEnemyNode::die()                            //
//=====================================================================================//
// умри!
bool FixedTechEnemyNode::die()
{
	m_mode = FTEM_KILLED;
	return need2Delete();
}

//=====================================================================================//
//                    bool FixedTechEnemyNode::need2Delete() const                     //
//=====================================================================================//
// нужно ли удалять узел
bool FixedTechEnemyNode::need2Delete() const
{
	if( (m_mode == FTEM_KILLED) && !m_activity) return true;
	return false;
}


#include "logicdefs.h"

#include "AIAPI.h"
#include "AIUtils.h"
#include "PathUtils.h"
#include "PanicPlayer.h"
#include "SubteamNode.h"
#include "PatrolTechEnemyNode.h"

DCTOR_IMP(PatrolTechEnemyNode)

//=====================================================================================//
//                     PatrolTechEnemyNode::PatrolTechEnemyNode()                      //
//=====================================================================================//
// конструктор - id существа
PatrolTechEnemyNode::PatrolTechEnemyNode(eid_t id)
:	m_mode(PTEM_PATROL),
	m_turn_state(TS_NONE),
	m_entity(id),
	m_activity(0),
	m_enemy_id(0),
	m_enemy_pnt(0, 0),
	m_shoot_pnt(0, 0),
	m_shooted(false),
	m_target_pnt(0, 0),
	m_target_dir(0),
	m_last_enemy_pnt(0, 0),
	m_initialising(false)
{
    GameEvMessenger::GetInst()->Attach(this, EV_SHOOT);

	if(!m_entity) return;
	OnSpawnMsg();
}

//=====================================================================================//
//                     PatrolTechEnemyNode::~PatrolTechEnemyNode()                     //
//=====================================================================================//
PatrolTechEnemyNode::~PatrolTechEnemyNode()
{
    GameEvMessenger::GetInst()->Detach(this);
	delete m_activity;
}


//=====================================================================================//
//                         float PatrolTechEnemyNode::Think()                          //
//=====================================================================================//
float PatrolTechEnemyNode::Think(state_type* st)
{
	float complexity = 0.0f;
    //проиграть действие
    if(m_activity)
	{
        if(st) *st = ST_INCOMPLETE;

        if(!m_activity->Run(AC_TICK))
		{
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
        case PTEM_PATROL:
            flag = thinkPatrol(st);
            break;
        
		case PTEM_ATTACK:
            flag = thinkAttack(st);
            break;
        
		case PTEM_PURSUIT:
            flag = thinkPursuit(st);
            break;
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
//                      void PatrolTechEnemyNode::MakeSaveLoad()                       //
//=====================================================================================//
void PatrolTechEnemyNode::MakeSaveLoad(SavSlot& st)
{
    if(st.IsSaving())
	{
		// сохраняемся
        st << static_cast<int>(m_mode);
        st << static_cast<int>(m_turn_state);
		st << m_entity;
		st << static_cast<int>(m_activity_observer.getLastEvent());
		st << m_enemy_id;
		st << m_enemy_pnt.x;
		st << m_enemy_pnt.y;
		st << m_shoot_pnt.x;
		st << m_shoot_pnt.y;
		st << m_shooted;
		st << m_target_pnt.x;
		st << m_target_pnt.y;
		st << m_target_dir;
		st << m_last_enemy_pnt.x;
		st << m_last_enemy_pnt.y;
    }
	else
	{
		// читаемся
		int tmp;
		st >> tmp; m_mode = static_cast<PatrolTechEnemyMode>(tmp);
		st >> tmp; m_turn_state = static_cast<TurnState>(tmp);
		st >> m_entity;
		st >> tmp; m_activity_observer.setLastEvent(static_cast<ActivityObserver::event_type>(tmp));
		st >> m_enemy_id;
		st >> m_enemy_pnt.x;
		st >> m_enemy_pnt.y;
		st >> m_shoot_pnt.x;
		st >> m_shoot_pnt.y;
		st >> m_shooted;
		st >> m_target_pnt.x;
		st >> m_target_pnt.y;
		st >> m_target_dir;
		st >> m_last_enemy_pnt.x;
		st >> m_last_enemy_pnt.y;
		
		// то, что восстанавливается без чтения
		m_activity = 0;
		m_initialising = false;
	}
}

//=====================================================================================//
//                     void PatrolTechEnemyNode::recieveMessage()                      //
//=====================================================================================//
// обработка внутрикомандных сообщений
void PatrolTechEnemyNode::recieveMessage(MessageType type, void * data)
{
	eid_t id = *static_cast<eid_t*>(data);
	if(type == MT_ENEMYSPOTTED)
	{
		if(m_enemy_id)
		{
			// уже сообщали о враге
			// проверим кто из врагов ближе
			float old_dist = dist(AIAPI::getPos2(m_entity),
				m_enemy_pnt);
			float new_dist = dist(AIAPI::getPos2(m_entity),
				AIAPI::getPos2(id));
			if(new_dist < old_dist)
			{
				// сменим врага
				m_enemy_id = id;
				m_enemy_pnt = AIAPI::getPos2(m_enemy_id);
			}
			return;
		}
		// это первый враг, о котором сообщили
		m_enemy_id = id;
		m_enemy_pnt = AIAPI::getPos2(m_enemy_id);
		return;
	}
}

//=====================================================================================//
//                   point3 PatrolTechEnemyNode::getPosition() const                   //
//=====================================================================================//
// выдать собственную позицию
point3 PatrolTechEnemyNode::getPosition() const
{
	return AIAPI::getPos3(m_entity);
}

//=====================================================================================//
//                         void PatrolTechEnemyNode::Update()                          //
//=====================================================================================//
void PatrolTechEnemyNode::Update(subject_t subj, event_t event, info_t ptr)
{
    switch(event){
    case EV_SHOOT:
        {
            shoot_info* info = static_cast<shoot_info*>(ptr);
            OnShootEv(info->m_actor, info->m_victim, info->m_point);
        }
        break;
    }
}

//=====================================================================================//
//                        void PatrolTechEnemyNode::OnShootEv()                        //
//=====================================================================================//
// обработка сообщения об выстреле или попадании
void PatrolTechEnemyNode::OnShootEv(BaseEntity* who, BaseEntity* victim, const point3& where)
{
	if(!m_entity) return;

	// не будем реагировать в случае,
	// если техника находится в определенных состояниях
	if(m_mode == PTEM_ATTACK) return;

	if(AIAPI::getPtr(m_entity) == who) return; // это я сам натворил

	float dist = (getPosition() - where).Length();
	if( (dist < alarm_dist) || (AIAPI::getPtr(m_entity) == victim))
	{
		// будем реагировать
		m_shoot_pnt = AIAPI::getPos2(who);
		m_shooted = true;
	}
}

//=====================================================================================//
//                       bool PatrolTechEnemyNode::thinkPatrol()                       //
//=====================================================================================//
// обработка режима Patrol
bool PatrolTechEnemyNode::thinkPatrol(state_type* st)
{
	// 1. если я вижу врага - пошлю сообщение и перейду в атаку
	AIAPI::entity_list_t enemies;
	if(AIAPI::getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// я кого-то вижу
		if(eid_t enemy = AIAPI::getMostDangerousEnemy(enemies,
			EnemyComparator(m_entity))
			)
		{
			AIAPI::print(m_entity, "|PT|thinkPatrol: я увидела врага!");
			// выбралa самого опасного врага
			// пошлю сообщение своей команде
			getSubteam()->sendMessage(this,
				AIEnemyNode::MT_ENEMYSPOTTED,
				&enemy,
				patrol_tech_enemy_dist);
			// переход в состояние атаки
			m_mode = PTEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_last_enemy_pnt = AIAPI::getPos2(enemy);

			// выберем лучший хекс так, чтобы потом хватило мувпоинтов
			// на один выстрел, если таких хексов нет - считает
			// лучшим хекс, в котором стоит.

			// получим общее кол-во оставшихся мувпонитов
			int movpoints = AIAPI::getMovePoints(m_entity);
			// вычтем количество мувпоинтов необходимых для выстрела
			movpoints -= AIAPI::getShootMovePoints(m_entity,
				/*на этот параметр забьет*/SHT_AIMSHOT);
			// вычтем мувпоинты на поворот к врагу
			movpoints -= AIUtils::CalcMps4EntityRotate(PIm2);
			int steps = AIAPI::getSteps(m_entity, movpoints);
			if(!steps)
			{
				// будем считать что стоим где надо, просто переходим в
				// атаку
				return false;
			}
			// рассчитаем поле проходимости
			PathUtils::CalcPassField(AIAPI::getPtr(m_entity),
				0, 0, 0);
			pnt_vec_t field;
			AIAPI::getBaseField(AIAPI::getPos2(m_entity),
				steps, &field);
			// получим вектор доступных хексов
			pnt_vec_t vec;
			AIAPI::getReachableField(field, &vec);
			// получим лучший хекс
			m_target_pnt = AIAPI::getBestPoint(vec,
				WalkingTechHexComparator(m_entity, enemy));
			pnt_vec_t path;
			PathUtils::CalcPath(AIAPI::getPtr(m_entity),
				m_target_pnt, path);
			// можно идти
			m_activity = ActivityFactory::CreateMove(AIAPI::getPtr(m_entity),
				path,
				ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer,
				ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer,
				ActivityObserver::EV_ENEMY_REACTED);
			return false;
		}
	}

	// 2. Если в меня стреляли - будем преследовать
	if(m_shooted)
	{
		AIAPI::print(m_entity, "|PT|thinkPatrol: в меня стреляли - будем преследовать");
		// в меня или рядом со мной стреляли
		m_shooted = false;
		// выберем точку в радиусе 3 хексов от существа, которое стреляло
		// пойдем туда и перейдем при этом в состояние преследования

		// рассчитаем поле проходимости
		PathUtils::CalcPassField(AIAPI::getPtr(m_entity),
			0, 0, 0);
		pnt_vec_t field;
		AIAPI::getBaseField(m_shoot_pnt,
			3, &field);
		// получим вектор доступных хексов
		pnt_vec_t vec;
		AIAPI::getReachableField(field, &vec);
		// получим пока случайный хекс
		m_target_pnt = AIAPI::getRandomPoint(vec);
		pnt_vec_t path;
		PathUtils::CalcPath(AIAPI::getPtr(m_entity),
			m_target_pnt, path);
		// можно идти
		m_activity = ActivityFactory::CreateMove(AIAPI::getPtr(m_entity),
			path,
			ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer,
			ActivityObserver::EV_MOVEPNTS_EXPIRIED);

		m_mode = PTEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 3. если мне сообщили о враге - пойдем к нему
	if(m_enemy_id)
	{
		AIAPI::print(m_entity, "|PT|thinkPatrol: мне сообщили о враге");
		// сообщили
		m_enemy_id = 0;
		// выберем точку в радиусе 1 хекса от врага
		// пойдем туда и перейдем при этом в состояние преследования

		// рассчитаем поле проходимости
		PathUtils::CalcPassField(AIAPI::getPtr(m_entity),
			0, 0, 0);
		pnt_vec_t field;
		AIAPI::getBaseField(m_enemy_pnt,
			1, &field);
		// получим вектор доступных хексов
		pnt_vec_t vec;
		AIAPI::getReachableField(field, &vec);
		// получим пока случайный хекс
		m_target_pnt = AIAPI::getRandomPoint(vec);
		pnt_vec_t path;
		PathUtils::CalcPath(AIAPI::getPtr(m_entity),
			m_target_pnt, path);
		// можно идти
		m_activity = ActivityFactory::CreateMove(AIAPI::getPtr(m_entity),
			path,
			ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer,
			ActivityObserver::EV_MOVEPNTS_EXPIRIED);

		m_mode = PTEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 4. если дошла до заданной точки - выбирает новую
	if(m_target_pnt == AIAPI::getPos2(m_entity))
	{
		AIAPI::print(m_entity, "|PT|thinkPatrol: дошла до заданной точки - выбирает новую");
		// рассчитаем поле проходимости
		pnt_vec_t allhexes;
		PathUtils::CalcPassField(AIAPI::getPtr(m_entity),
			0, &allhexes, 0);
		if(allhexes.empty())
		{
			// передаем ход - идти некуда
			AIAPI::print(m_entity, "|PT|thinkPatrol: мне некуда ходить!!!");
			m_turn_state = TS_END;
			return false;
		}
		// получим случайную точку
		m_target_pnt = AIAPI::getRandomPoint(allhexes);
	}

	// 5. проверим не кончились ли у нас мувпоинты
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// не хватило мувпоинтов
		AIAPI::print(m_entity, "|PT|thinkPatrol: не хватило мувпоинтов");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 6. идем в заданную точку
	AIAPI::print(m_entity, "|PT|thinkPatrol: идем в заданную точку");
	PathUtils::CalcPassField(AIAPI::getPtr(m_entity),
		0, 0, 0);
	pnt_vec_t path;
	PathUtils::CalcPath(AIAPI::getPtr(m_entity),
		m_target_pnt, path);
	if(path.empty())
	{
		// точка оказалась недостижимой - будем считать, что уже пришли
		// куда надо
		m_target_pnt = AIAPI::getPos2(m_entity);
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	// можно идти
	m_activity = ActivityFactory::CreateMove(AIAPI::getPtr(m_entity),
		path,
		ActivityFactory::CT_ENEMY_MOVE);
	m_activity->Attach(&m_activity_observer,
		ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

//=====================================================================================//
//                       bool PatrolTechEnemyNode::thinkAttack()                       //
//=====================================================================================//
// обработка режима Attack
bool PatrolTechEnemyNode::thinkAttack(state_type* st)
{
	// 1. если у техники нет оружия - перейдем к преследованию
	if(!AIAPI::haveTechWeapon(m_entity))
	{
		AIAPI::print(m_entity, "|PT|thinkAttack: нет оружия - будем преследовать");
		m_mode = PTEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		m_activity_observer.clear();
		return false;
	}

	// 2. выберем врага
	AIAPI::entity_list_t enemies;
	eid_t enemy = 0;
	if(AIAPI::getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// сам видит врага

		// если это начало хода или реакция - сбросим список игнорирования
		if(m_turn_state == TS_START)
		{
			m_ignored_enemies.clear();
		}
		// попробуем найти разность списков всех врагов и игнорированных врагов
		EntityList_t diff_list;
		setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);
		if(diff_list.empty())
		{
			// выберем самого опасного врага из всех
			enemy = AIAPI::getMostDangerousEnemy(enemies,
				EnemyComparator(m_entity));
		}
		else
		{
			// выберем самого опасного врага из непроигнорированных
			enemy = AIAPI::getMostDangerousEnemy(diff_list,
				EnemyComparator(m_entity));
		}
		m_last_enemy_pnt = AIAPI::getPos2(enemy);
	}
	
	// 3. если нет врагов - переходим в состояние преследования
	if(!enemy)
	{
		// врагов нет - будем преследовать последнего
		AIAPI::print(m_entity, "|PT|thinkAttack: врагов нет - будем преследовать последнего");
		m_mode = PTEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		m_target_pnt = m_last_enemy_pnt;
		m_activity_observer.clear();
		return false;
	}
	// враги есть...

	// 4. если это начало нового хода или сработала реакция
	if(m_turn_state == TS_START)
	{
		// выбираем лучшую точку и идем в нее
		AIAPI::print(m_entity, "|PT|thinkAttack: выбираем лучшую точку и идем в нее");

		// выберем лучший хекс так, чтобы потом хватило мувпоинтов
		// на один выстрел, если таких хексов нет - считает
		// лучшим хекс, в котором стоит.
		
		m_turn_state = TS_INPROGRESS;
		// получим общее кол-во оставшихся мувпонитов
		int movpoints = AIAPI::getMovePoints(m_entity);
		// вычтем количество мувпоинтов необходимых для выстрела
		movpoints -= AIAPI::getShootMovePoints(m_entity,
			/*забьет на этот параметр*/SHT_AIMSHOT);
		// вычтем мувпоинты на поворот к врагу
		movpoints -= AIUtils::CalcMps4EntityRotate(PIm2);
		int steps = AIAPI::getSteps(m_entity, movpoints);
		if(!steps)
		{
			// будем считать что стоим где надо, просто переходим в
			// атаку
			return false;
		}
		// рассчитаем поле проходимости
		PathUtils::CalcPassField(AIAPI::getPtr(m_entity),
			0, 0, 0);
		pnt_vec_t field;
		AIAPI::getBaseField(AIAPI::getPos2(m_entity),
			steps, &field);
		// получим вектор доступных хексов
		pnt_vec_t vec;
		AIAPI::getReachableField(field, &vec);
		// получим лучший хекс
		m_target_pnt = AIAPI::getBestPoint(vec,
			WalkingTechHexComparator(m_entity, enemy));
		pnt_vec_t path;
		PathUtils::CalcPath(AIAPI::getPtr(m_entity),
			m_target_pnt, path);
		// можно идти
		m_activity = ActivityFactory::CreateMove(AIAPI::getPtr(m_entity),
			path,
			ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer,
			ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_activity->Attach(&m_activity_observer,
			ActivityObserver::EV_ENEMY_REACTED);
		m_activity->Attach(&m_activity_observer,
			ActivityObserver::EV_ENEMY_SPOTTED);
		return false;
	}

	// 5. это продолжение хода

	// 5.1 если не хватило мувпоинтов - передает ход
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// закончились - закончим ход
		AIAPI::print(m_entity, "|PT|thinkAttack: закончились мувпоинты");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 5.2 если сработала реакция или увидел нового врага - имитируем
	//       начало хода
	if( (m_activity_observer.getLastEvent() == ActivityObserver::EV_ENEMY_REACTED) ||
		(m_activity_observer.getLastEvent() == ActivityObserver::EV_ENEMY_SPOTTED)
		)
	{
		AIAPI::print(m_entity, "|PT|thinkAttack: сработала реакция или увидел нового врага");
		m_activity_observer.clear();
		// имитируем начало хода
		m_ignored_enemies.clear();
		m_turn_state = TS_START;
		return true;
	}

	// 5.3 если нет линии огня - добавляем врага в список игнорирования
	// причем, если после этого все враги оказались в списке игнорирования
	// попробуем подойти к одному из них
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_NO_LOF)
	{
		// закончились - закончим ход
		AIAPI::print(m_entity, "|PT|thinkAttack: нет линии огня - добавляем врага в список игнорирования");
		m_activity_observer.clear();
		// добавим в список игнорирования
		m_ignored_enemies.insert(enemy);
		EntityList_t diff_list;
		setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);
		if(diff_list.empty())
		{
			// попробуем подойти поближе к врагу
			AIAPI::print(m_entity, "|PT|thinkAttack: подойдем поближе");
			// рассчитаем поле проходимости
			PathUtils::CalcPassField(AIAPI::getPtr(m_entity),
				0, 0, PathUtils::F_CALC_NEAR_PNTS);
			EntityList_t::iterator i = enemies.begin();
			// найдем существо, к которому можно подойти
			bool found = false;
			while(i != enemies.end())
			{
				// проверим, существуют ли точки рядом с существом
				if(PathUtils::IsNear(AIAPI::getPtr(*i)))
				{
					// к юниту можно подойти
					found = true;
					m_target_pnt = PathUtils::GetNearPnt(AIAPI::getPtr(*i)).m_pnt;
					break;
				}
				++i;
			}
			pnt_vec_t path;
			if(!found)
			{
				// подходящего врага нет - переместимся случайным образом
				pnt_vec_t base, vec;
				AIAPI::getBaseField(AIAPI::getPos2(m_entity),
					patrol_enemy_noLOF_move,
					&base);
				AIAPI::getReachableField(base, &vec, true);
				m_target_pnt = AIAPI::getRandomPoint(vec);
				PathUtils::CalcPath(AIAPI::getPtr(m_entity),
					m_target_pnt, path);
			}
			else
			{
				// есть враг, к которому можно подойти - подойдем на половину пути
				PathUtils::CalcPath(AIAPI::getPtr(m_entity),
					m_target_pnt, path);
				// отрежем половину пути
				path.erase(&path[0], &path[path.size() / 2]);
			}
			if(path.empty())
			{
				// передадим ход
				m_turn_state = TS_END;
				return false;
			}
			// можно идти
			AIAPI::setWalk(m_entity);
			m_activity = ActivityFactory::CreateMove(AIAPI::getPtr(m_entity),
				path,
				ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer,
				ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer,
				ActivityObserver::EV_ENEMY_REACTED);
			m_activity->Attach(&m_activity_observer,
				ActivityObserver::EV_ENEMY_SPOTTED);
			
			m_turn_state = TS_INPROGRESS;
			return false;
		} // if(diff_list.empty())
		// зациклимся для поиска других врагов
		m_turn_state = TS_INPROGRESS;
		return true;
	}

	// 5.4 Если не повернутa к врагу - поворачивается в направлении где был враг
	m_target_dir = norm_angle(Dir2Angle(AIAPI::getPos3(enemy) - getPosition()));
	float cur_angle = norm_angle(AIAPI::getAngle(m_entity));
	if( fabs(m_target_dir - cur_angle) >= angle_eps )
	{
		// нужно поворачиваться к врагу
		m_activity = ActivityFactory::CreateRotate(AIAPI::getPtr(m_entity),
			m_target_dir,
			ActivityFactory::CT_ENEMY_ROTATE);
		m_activity->Attach(&m_activity_observer,
			ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	
	// 5.5 повернутa к врагу - если сможем будем атаковать
	AIAPI::print(m_entity, "|PT|thinkAttack: будем стрелять, если это возможно!");
	// враг есть - выстрелим по нему если это возможно
	std::string reason;
	if(AIAPI::shootByVehicle(m_entity, enemy, &m_activity, &reason))
	{
		m_turn_state = TS_INPROGRESS;
		m_activity->Attach(&m_activity_observer,
			ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_activity->Attach(&m_activity_observer,
			ActivityObserver::EV_NO_LOF);
	}
	else
	{
		std::string str = "|PT|thinkAttack: стрелять нельзя: ";
		str += reason;
		AIAPI::print(m_entity, str.c_str());
		m_turn_state = TS_END;
	}
	return false;
}

//=====================================================================================//
//                      bool PatrolTechEnemyNode::thinkPursuit()                       //
//=====================================================================================//
// обработка режима Pursuit
bool PatrolTechEnemyNode::thinkPursuit(state_type* st)
{
	// 1. если я вижу врага - пошлю сообщение и перейду в атаку
	AIAPI::entity_list_t enemies;
	if(AIAPI::getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// я кого-то вижу
		if(eid_t enemy = AIAPI::getMostDangerousEnemy(enemies,
			EnemyComparator(m_entity))
			)
		{
			AIAPI::print(m_entity, "|PT|thinkPursuit: я увиделa врага!");
			// выбралa самого опасного врага
			// пошлю сообщение своей команде
			getSubteam()->sendMessage(this,
				AIEnemyNode::MT_ENEMYSPOTTED,
				&enemy,
				patrol_tech_enemy_dist);
			// переход в состояние атаки
			m_mode = PTEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_last_enemy_pnt = AIAPI::getPos2(enemy);

			// выберем лучший хекс так, чтобы потом хватило мувпоинтов
			// на один выстрел, если таких хексов нет - считает
			// лучшим хекс, в котором стоит.

			// получим общее кол-во оставшихся мувпонитов
			int movpoints = AIAPI::getMovePoints(m_entity);
			// вычтем количество мувпоинтов необходимых для выстрела
			movpoints -= AIAPI::getShootMovePoints(m_entity,
				/*забьет на этот параметр*/SHT_AIMSHOT);
			// вычтем мувпоинты на поворот к врагу
			movpoints -= AIUtils::CalcMps4EntityRotate(PIm2);
			int steps = AIAPI::getSteps(m_entity, movpoints);
			if(!steps)
			{
				// будем считать что стоим где надо, просто переходим в
				// атаку
				return false;
			}
			// рассчитаем поле проходимости
			PathUtils::CalcPassField(AIAPI::getPtr(m_entity),
				0, 0, 0);
			pnt_vec_t field;
			AIAPI::getBaseField(AIAPI::getPos2(m_entity),
				steps, &field);
			// получим вектор доступных хексов
			pnt_vec_t vec;
			AIAPI::getReachableField(field, &vec);
			// получим лучший хекс
			m_target_pnt = AIAPI::getBestPoint(vec,
				WalkingTechHexComparator(m_entity, enemy));
			pnt_vec_t path;
			PathUtils::CalcPath(AIAPI::getPtr(m_entity),
				m_target_pnt, path);
			// можно идти
			m_activity = ActivityFactory::CreateMove(AIAPI::getPtr(m_entity),
				path,
				ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer,
				ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer,
				ActivityObserver::EV_ENEMY_REACTED);
			return false;
		}
	}

	// 2. проверим не кончились ли у нас мувпоинты
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// не хватило мувпоинтов
		AIAPI::print(m_entity, "|PT|thinkPursuit: не хватило мувпоинтов");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 3. проверим не пришли ли мы в заданную точку
	if(m_target_pnt == AIAPI::getPos2(m_entity))
	{
		// мы пришли
		// 3.1 проверим, повернутa ли в точку, где был враг
		m_target_dir = norm_angle(Dir2Angle(AIAPI::convertPos2ToPos3(m_last_enemy_pnt) - getPosition()));
		float cur_angle = norm_angle(AIAPI::getAngle(m_entity));
		if( fabs(m_target_dir - cur_angle) >= angle_eps )
		{
			// нужно поворачиваться к врагу
			m_activity = ActivityFactory::CreateRotate(AIAPI::getPtr(m_entity),
				m_target_dir,
				ActivityFactory::CT_ENEMY_ROTATE);
			m_activity->Attach(&m_activity_observer,
				ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_turn_state = TS_INPROGRESS;
			return false;
		}
		// 3.2 повернутa - переход в состояние патрулирования
		AIAPI::print(m_entity, "|PT|thinkPursuit: никого не догналa - будем патрулировать");
		// переходим в состояние патрулирования
		m_mode = PTEM_PATROL;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 4. идем в заданную точку
	AIAPI::print(m_entity, "|PT|thinkPursuit: идем в заданную точку");
	PathUtils::CalcPassField(AIAPI::getPtr(m_entity),
		0, 0, 0);
	pnt_vec_t path;
	PathUtils::CalcPath(AIAPI::getPtr(m_entity),
		m_target_pnt, path);
	if(path.empty())
	{
		// точка оказалась недостижимой - будем считать, что уже пришли
		// куда надо
		m_target_pnt = AIAPI::getPos2(m_entity);
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	// можно идти
	m_activity = ActivityFactory::CreateMove(AIAPI::getPtr(m_entity),
		path,
		ActivityFactory::CT_ENEMY_MOVE);
	m_activity->Attach(&m_activity_observer,
		ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

//=====================================================================================//
//                           bool PatrolTechEnemyNode::die()                           //
//=====================================================================================//
// умри!
bool PatrolTechEnemyNode::die()
{
	m_mode = PTEM_KILLED;
	return need2Delete();
}

//=====================================================================================//
//                    bool PatrolTechEnemyNode::need2Delete() const                    //
//=====================================================================================//
// нужно ли удалять узел
bool PatrolTechEnemyNode::need2Delete() const
{
	if( (m_mode == PTEM_KILLED) && !m_activity) return true;
	return false;
}

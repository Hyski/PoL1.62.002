#include "logicdefs.h"
#include "PathUtils.h"
#include "SubteamNode.h"
#include "FixedEnemyNode.h"

/////////////////////////////////////////////////////////////////////////////
//
// узел для стационарного врага
//
/////////////////////////////////////////////////////////////////////////////

DCTOR_IMP(FixedEnemyNode)

//=====================================================================================//
//                          FixedEnemyNode::FixedEnemyNode()                           //
//=====================================================================================//
//. конструктор - id существа и позиция, где он будет стоять
FixedEnemyNode::FixedEnemyNode(eid_t id)
:	CommonEnemyNode(id),
	m_mode(FEM_BASE),
	m_basepnt(0, 0),
	m_basedir(0.0f),
	m_turn(0),
	m_target_dir(0.0f),
	m_target_pnt(0,0),
	m_prev_dir(0.0f)
{
    GameEvMessenger::GetInst()->Attach(this, EV_SHOOT);
	if(!m_entity) return;
	OnSpawnMsg();
}

//=====================================================================================//
//                          FixedEnemyNode::~FixedEnemyNode()                          //
//=====================================================================================//
FixedEnemyNode::~FixedEnemyNode()
{
    GameEvMessenger::GetInst()->Detach(this);
}

//=====================================================================================//
//                            float FixedEnemyNode::Think()                            //
//=====================================================================================//
float FixedEnemyNode::Think(state_type* st)
{
	float complexity = 0.0f;
	if (ThinkShell(st)) return complexity;

	if(UnlimitedMoves())
	{
		if(st) *st = ST_FINISHED;
		return 1.0f;
	}

    // юнит свободен - можно подумать
	bool flag = true;
    while(flag)
	{
        switch(m_mode)
		{
        case FEM_BASE:		  flag = thinkBase(st);        break;
        case FEM_LOOKROUND:   flag = thinkLookRound(st);   break;
        case FEM_ATTACK:      flag = thinkAttack(st);      break;
        case FEM_RETURN2BASE: flag = thinkReturn2Base(st); break;
        case FEM_WEAPONSEARCH:flag = thinkWeaponSearch(st);break;
        case FEM_WEAPONTAKE:  flag = thinkWeaponTake(st);  break;
        case FEM_MEDKITTAKE:  flag = thinkMedkitTake(st);  break;
        }
    }

	CheckFinished(st, &complexity);
	return complexity;
}

//=====================================================================================//
//                         void FixedEnemyNode::MakeSaveLoad()                         //
//=====================================================================================//
void FixedEnemyNode::MakeSaveLoad(SavSlot& st)
{
    if(st.IsSaving())
	{
		// сохраняемся
        st << static_cast<int>(m_mode);
        st << static_cast<int>(m_turn_state);
        st << static_cast<int>(m_lookround_state);
		st << m_entity;
		st << static_cast<int>(m_activity_observer.getLastEvent());

		st << m_basepnt.x << m_basepnt.y << m_basedir << m_turn << m_target_dir
			<< m_target_pnt.x << m_target_pnt.y << m_prev_dir;
    }
	else
	{
		// читаемся
		int tmp;
		st >> tmp; m_mode = static_cast<FixedEnemyMode>(tmp);
		st >> tmp; m_turn_state = static_cast<TurnState>(tmp);
		st >> tmp; m_lookround_state = static_cast<LookroundState>(tmp);
		st >> m_entity;
		st >> tmp; m_activity_observer.setLastEvent(static_cast<ActivityObserver::event_type>(tmp));

		st >> m_basepnt.x >> m_basepnt.y >> m_basedir >> m_turn >> m_target_dir
			>> m_target_pnt.x >> m_target_pnt.y >> m_prev_dir;
		
		// то, что восстанавливается без чтения
		m_activity.Reset();
		AIAPI::getBaseField(m_basepnt, 10, &m_base_field);
		m_initialising = false;
	}
}


//=====================================================================================//
//                          void FixedEnemyNode::OnSpawnMsg()                          //
//=====================================================================================//
//обработка сообщения о расстановке
void FixedEnemyNode::OnSpawnMsg()
{	
	m_basepnt = AIAPI::getPos2(m_entity);
	m_basedir = norm_angle(AIAPI::getAngle(m_entity));
	AIAPI::getBaseField(m_basepnt, 10, &m_base_field);
}

//=====================================================================================//
//                            void FixedEnemyNode::Update()                            //
//=====================================================================================//
void FixedEnemyNode::Update(subject_t subj, event_t event, info_t ptr)
{
    switch(event)
	{
    case EV_SHOOT:
        {
            shoot_info* info = static_cast<shoot_info*>(ptr);
            OnShootEv(info->m_actor, info->m_victim, info->m_point);
        }
        break;
    }
}

//=====================================================================================//
//                          void FixedEnemyNode::OnShootEv()                           //
//=====================================================================================//
// обработка сообщения об выстреле или попадании
void FixedEnemyNode::OnShootEv(BaseEntity* who, BaseEntity* victim, const point3& where)
{
	BaseEntity* be = AIAPI::getPtr(m_entity);
	if( be && (be == who) ) return; // это я сам натворил

	float dist = (getPosition() - where).Length();
	if( (dist < alarm_dist) || (be == victim))
	{	// будем реагировать - оборот на следующий ход
		m_turn = fixed_turns-1;
	}
}

//=====================================================================================//
//                          bool FixedEnemyNode::thinkBase()                           //
//=====================================================================================//
// обработка режима Base
bool FixedEnemyNode::thinkBase(state_type* st)
{
	// 1. проверим свое здоровье, если оно < половины - пошлем сообщение
	SendCureMessage(fixed_cure_dist);

	// 2. если у нас нет оружия - перейдем к его поиску
	if( (!AIAPI::takeBestWeapon(m_entity, FixedWeaponComparator())) &&	(!AIAPI::takeGrenade(m_entity))	)
	{
		AIAPI::print(m_entity, "|F|thinkBase: надо искать пушку!");
		m_mode = FEM_WEAPONSEARCH;
		m_turn_state = TS_INPROGRESS;
		m_turn = 0;
		return false;
	}
	
	// 3. если я вижу врага - пошлю сообщение и перейду в атаку
	AIAPI::entity_list_t enemies;
	if(AIAPI::getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// я кого-то вижу
		if(eid_t enemy = AIAPI::getMostDangerousEnemy(enemies,
			EnemyComparator(m_entity)) 	)
		{
			AIAPI::print(m_entity, "|F|thinkBase: я увидел врага!");
			// выбрал самого опасного врага, пошлю сообщение своей команде
			getSubteam()->sendMessage(this,	AIEnemyNode::MT_ENEMYSPOTTED,	&enemy,	fixed_enemy_dist);

			if(AIAPI::mayShoot(enemy,m_entity))
			{
				m_mode = FEM_ATTACK;
				m_turn_state = TS_INPROGRESS;
				m_turn = 0;
				return false;
			}
			else
			{
				// переход в состояние атаки, выберем лучший хекс в радиусе 10 хексов от базовой точки:
				// рассчитаем поле проходимости
				PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, 0, 0);
				// получим вектор доступных хексов базового поля
				pnt_vec_t vec;
				AIAPI::getReachableField(m_base_field, &vec, true);
				// получим лучший хекс
				m_target_pnt = AIAPI::getBestPoint(vec,	AssaultHexComparator(m_entity, enemy));
				pnt_vec_t path;
				PathUtils::CalcPath(AIAPI::getPtr(m_entity),	m_target_pnt, path);
				// можно идти
				m_activity = CreateMoveActivity(path, ActivityFactory::CT_ENEMY_MOVE);
				m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
				m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_REACTED);
				m_mode = FEM_ATTACK;
				m_turn_state = TS_INPROGRESS;
				m_turn = 0;
				return false;
			}
		}
	}

	// 4. если моя подкоманда засекла врага - перейду в атаку
	enemies.clear();
	if(AIAPI::getEnemiesVisibleBySubteam(getSubteam()->getName(),&enemies)  )
	{
		// засекла - выберем самого опасного
		if(eid_t enemy = AIAPI::getMostDangerousEnemy(enemies,
			EnemyComparator(m_entity)))
		{
			AIAPI::print(m_entity, "|F|thinkBase: моя команда засекла врага!");
			// выбрал самого опасного врага, переход в состояние атаки
			// выберем лучший хекс в радиусе 10 хексов от базовой точки:
			// рассчитаем поле проходимости
			PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, 0, 0);
			// получим вектор доступных хексов базового поля
			pnt_vec_t vec;
			AIAPI::getReachableField(m_base_field, &vec, true);
			// получим лучший хекс
			m_target_pnt = AIAPI::getBestPoint(vec,	FixedHexComparator(m_entity, enemy));
			pnt_vec_t path;
			PathUtils::CalcPath(AIAPI::getPtr(m_entity),m_target_pnt, path);
			// можно идти
			m_activity = CreateMoveActivity(path, ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer,ActivityObserver::EV_ENEMY_REACTED);
			m_mode = FEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_turn = 0;
			return false;
		}
	}

	// 5. врагов нет
	// 5.1 если здоровье меньше максимума - попробуем себя полечить
	SelfCure();

	// 5.2 если вижу аптечку и она мне нужна - побегу за ней
	if(needAndSeeMedkit(&m_target_pnt))
	{
		PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, 0, 0);
		pnt_vec_t path;
		PathUtils::CalcPath(AIAPI::getPtr(m_entity), m_target_pnt, path);
		if(!path.empty())
		{
			// можно идти
			m_activity = CreateMoveActivity(path, ActivityFactory::CT_ENEMY_MOVEIGNORE);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_mode = FEM_MEDKITTAKE;
			m_turn_state = TS_INPROGRESS;
			return false;
		}
	}

	// 5.3 если время оборачиваться - обернусь
	if(++m_turn < fixed_turns)
	{
		AIAPI::print(m_entity, "|F|thinkBase: еще не время обернуться");
		// еще не время оборачиваться
		m_turn_state = TS_END;
		return false;
	}
	// пора обернуться
	AIAPI::print(m_entity, "|F|thinkBase: пора оборачиваться");
	m_turn = 0;
	m_turn_state = TS_INPROGRESS;
	m_mode = FEM_LOOKROUND;
	m_lookround_state = LS_FIRST;
	m_prev_dir = norm_angle(AIAPI::getAngle(m_entity));
	m_target_dir = norm_angle(m_prev_dir + 2*PId3);
	m_activity = CreateRotateActivity(m_target_dir, ActivityFactory::CT_ENEMY_LOOKROUND);
	m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_activity->Attach(&m_activity_observer,ActivityObserver::EV_ENEMY_SPOTTED);
	return false;
}

//=====================================================================================//
//                        bool FixedEnemyNode::thinkLookRound()                        //
//=====================================================================================//
// обработка режима LookRound
bool FixedEnemyNode::thinkLookRound(state_type* st)
{
	// 1. проверим свое здоровье, если оно < половины - пошлем сообщение
	SendCureMessage(fixed_cure_dist);

	// 2. если я вижу врага - пошлю сообщение и перейду в атаку
	AIAPI::entity_list_t enemies;
	if(AIAPI::getEnemiesVisibleByEntity(m_entity, &enemies))
	{	// я кого-то вижу
		if(eid_t enemy = AIAPI::getMostDangerousEnemy(enemies,EnemyComparator(m_entity))	)
		{
			AIAPI::print(m_entity, "|F|thinkLookRound: я увидел врага!");
			// выбрал самого опасного врага
			// пошлю сообщение своей команде
			getSubteam()->sendMessage(this,	AIEnemyNode::MT_ENEMYSPOTTED,&enemy,fixed_enemy_dist);
			// переход в состояние атаки, выберем лучший хекс в радиусе 10 хексов от базовой точки:
			// рассчитаем поле проходимости
			PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, 0, 0);
			// получим вектор доступных хексов базового поля
			pnt_vec_t vec;
			AIAPI::getReachableField(m_base_field, &vec, true);
			// получим лучший хекс
			m_target_pnt = AIAPI::getBestPoint(vec,	FixedHexComparator(m_entity, enemy));
			pnt_vec_t path;
			PathUtils::CalcPath(AIAPI::getPtr(m_entity),	m_target_pnt, path);
			// можно идти
			m_activity = CreateMoveActivity(path,ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer,ActivityObserver::EV_ENEMY_REACTED);
			m_mode = FEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_turn = 0;
			return false;
		}
	}

	// 3. если моя подкоманда засекла врага - перейду в атаку
	enemies.clear();
	if(AIAPI::getEnemiesVisibleBySubteam(getSubteam()->getName(),&enemies))
	{
		// засекла - выберем самого опасного
		if(eid_t enemy = AIAPI::getMostDangerousEnemy(enemies,EnemyComparator(m_entity))	)
		{
			AIAPI::print(m_entity, "|F|thinkLookRound: моя команда засекла врага!");
			// выбрал самого опасного врага, переход в состояние атаки
			// выберем лучший хекс в радиусе 10 хексов от базовой точки:
			// рассчитаем поле проходимости
			PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, 0, 0);
			// получим вектор доступных хексов базового поля
			pnt_vec_t vec;
			AIAPI::getReachableField(m_base_field, &vec, true);
			// получим лучший хекс
			m_target_pnt = AIAPI::getBestPoint(vec,	FixedHexComparator(m_entity, enemy));
			pnt_vec_t path;
			PathUtils::CalcPath(AIAPI::getPtr(m_entity),	m_target_pnt, path);
			// можно идти
			m_activity = CreateMoveActivity(path, ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer,	ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer,	ActivityObserver::EV_ENEMY_REACTED);
			m_mode = FEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_turn = 0;
			return false;
		}
	}

	// 4. получим текущий угол, на который повернуто существо
	float cur_angle = norm_angle(AIAPI::getAngle(m_entity));
	if(fabs(m_target_dir - cur_angle) < angle_eps)
	{
		// закончен очередной прием разворота
		switch(m_lookround_state)
		{
		case LS_FIRST : 
			{
				AIAPI::print(m_entity, "|F|thinkLookRound: закончен первый прием разворота");
				// начинаем второй прием разворота
				m_turn_state = TS_INPROGRESS;
				m_lookround_state = LS_SECOND;
				m_target_dir = norm_angle(m_prev_dir + 4*PId3);
				m_activity = CreateRotateActivity(m_target_dir, ActivityFactory::CT_ENEMY_LOOKROUND);
				m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
				m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_SPOTTED);
				break;
			}
		case LS_SECOND : 
			{
				AIAPI::print(m_entity, "|F|thinkLookRound: закончен второй прием разворота");
				// начинаем заключительный прием разворота
				m_turn_state = TS_INPROGRESS;
				m_lookround_state = LS_2BASE;
				m_target_dir = m_basedir;
				m_activity = CreateRotateActivity(m_target_dir,ActivityFactory::CT_ENEMY_LOOKROUND);
				m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
				m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_SPOTTED);
				break;
			}
		case LS_2BASE : 
			{
				AIAPI::print(m_entity, "|F|thinkLookRound: полностью закончил разворот");
				// переходим в базовое состояние
				m_mode = FEM_BASE;
				m_turn_state = TS_END;
				break;
			}
		default : break;
		}
		return false;
	}

	// 5. если не хватило мувпоинтов - продолжим на следющий ход
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		AIAPI::print(m_entity, "|F|thinkLookRound: не хватило мувпоинтов - продолжим на следющий ход");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// продолжим действие
	m_activity = CreateRotateActivity(m_target_dir,ActivityFactory::CT_ENEMY_LOOKROUND);
	
	m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_activity->Attach(&m_activity_observer,ActivityObserver::EV_ENEMY_SPOTTED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

//=====================================================================================//
//                         bool FixedEnemyNode::thinkAttack()                          //
//=====================================================================================//
// обработка режима Attack
bool FixedEnemyNode::thinkAttack(state_type* st)
{
	// 1. проверим свое здоровье, если оно < половины - пошлем сообщение
	SendCureMessage(fixed_cure_dist);

	// 2. если у нас нет оружия - перейдем к его поиску
	if( (!AIAPI::takeBestWeapon(m_entity, FixedWeaponComparator())) &&
		(!AIAPI::takeGrenade(m_entity))	)
	{
		AIAPI::print(m_entity, "|F|thinkAttack: надо искать пушку!");
		m_mode = FEM_WEAPONSEARCH;
		m_turn_state = TS_INPROGRESS;
		m_activity_observer.clear();
		AIAPI::setStandPose(m_entity);
		return false;
	}

	// 3. выберем врага
	AIAPI::entity_list_t enemies;
	eid_t enemy = 0;
	if(AIAPI::getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// сам видит врага
		// если это начало хода или реакция - сбросим список игнорирования
		if(m_turn_state == TS_START)
		{
			m_ignored_enemies.clear();
			// и заодно присядем если уже не сидим
			AIAPI::setSitPose(m_entity);
		}
		// попробуем найти разность списков всех врагов и игнорированных врагов
		EntityList_t diff_list;
		setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);
		if(diff_list.empty()) // выберем самого опасного врага из всех
			enemy = AIAPI::getMostDangerousEnemy(enemies,EnemyComparator(m_entity));
		else // выберем самого опасного врага из непроигнорированных
			enemy = AIAPI::getMostDangerousEnemy(diff_list,	EnemyComparator(m_entity));
	}
	
	if(!enemy)
	{
		// сам врага не видит
		enemies.clear();
		if(AIAPI::getEnemiesVisibleBySubteam(getSubteam()->getName(),&enemies)	)
		{
			// зато видит его подкоманда
			// если это начало хода или реакция - сбросим список игнорирования
			if(m_turn_state == TS_START)
			{
				m_ignored_enemies.clear();
				// и заодно присядем если уже не сидим
				AIAPI::setSitPose(m_entity);
			}
			// попробуем найти разность списков всех врагов и игнорированных врагов
			EntityList_t diff_list;
			setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);
			if(diff_list.empty())// выберем самого опасного врага из всех
				enemy = AIAPI::getMostDangerousEnemy(enemies,EnemyComparator(m_entity));
			else// выберем самого опасного врага из непроигнорированных
				enemy = AIAPI::getMostDangerousEnemy(diff_list,	EnemyComparator(m_entity));
		}
	}
	
	// 4. если нет врагов - возврат в базовую точку
	if(!enemy)
	{
		// больше нет врагов - переход в состояние возврат в базовую точку
		AIAPI::print(m_entity, "|F|thinkAttack: врагов нет - на базу");
		m_mode = FEM_RETURN2BASE;
		m_turn_state = TS_INPROGRESS;
		m_activity_observer.clear();
		AIAPI::setStandPose(m_entity);
		return false;
	}
	// враги есть...

	// 5. если это начало нового хода
	if(m_turn_state == TS_START)
	{
		// выбираем лучшую точку и идем в нее
		AIAPI::setStandPose(m_entity);
		AIAPI::print(m_entity, "|F|thinkAttack: выбираем лучшую точку и идем в нее");
		// выберем лучший хекс в радиусе 10 хексов от базовой точки:
		// рассчитаем поле проходимости
		PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, 0, 0);
		// получим вектор доступных хексов базового поля
		pnt_vec_t vec;
		AIAPI::getReachableField(m_base_field, &vec, true);
		// получим лучший хекс
		m_target_pnt = AIAPI::getBestPoint(vec,	FixedHexComparator(m_entity, enemy));

		m_turn_state = TS_INPROGRESS;

		if(m_target_pnt != AIAPI::getPos2(m_entity))
		{
			pnt_vec_t path;
			PathUtils::CalcPath(AIAPI::getPtr(m_entity),	m_target_pnt, path);
			// можно идти
			m_activity = CreateMoveActivity(path, ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer,	ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer,	ActivityObserver::EV_ENEMY_REACTED);
			m_activity->Attach(&m_activity_observer,	ActivityObserver::EV_ENEMY_SPOTTED);
			
			return false;
		}
	}

	// 6. это продолжение хода

	// 6.1 если юниту не хватило мувпоинтов - он передает ход
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		AIAPI::print(m_entity, "|F|thinkAttack: закончились мувпоинты");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}
	
	// 6.2 если сработала реакция или увидел нового врага - имитируем
	//       начало хода
	if( (m_activity_observer.getLastEvent() == ActivityObserver::EV_ENEMY_REACTED) ||
		(m_activity_observer.getLastEvent() == ActivityObserver::EV_ENEMY_SPOTTED) 	)
	{
		AIAPI::print(m_entity, "|F|thinkAttack: сработала реакция или увидел нового врага");
		m_activity_observer.clear();
		// имитируем начало хода
		m_ignored_enemies.clear();
		m_turn_state = TS_START;
		return false;
	}

	// 6.3 если нет линии огня - добавляем врага в список игнорирования
	// причем, если после этого все враги оказались в списке игнорирования
	// передаем ход
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_NO_LOF)
	{
		AIAPI::print(m_entity, "|F|thinkAttack: нет линии огня - добавляем врага в список игнорирования");
		m_activity_observer.clear();
		// добавим в список игнорирования
		m_ignored_enemies.insert(enemy);

		EntityList_t diff_list;
		setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);

		if(!diff_list.empty())
		{
			// зациклимся для поиска других врагов
			m_turn_state = TS_INPROGRESS;
			return true;
		}
		
		if(AIAPI::isSitPose(m_entity))
		{
			// встанем
			AIAPI::setStandPose(m_entity);
			// обнулим спискок проигнорированных врагов
			m_ignored_enemies.clear();
			// попробуем поискать врагов еще раз
			m_turn_state = TS_INPROGRESS;
			return false;
		}
		// передаем ход
		AIAPI::setSitPose(m_entity);
		m_turn_state = TS_END;
		return false;
	}

	// 6.4 если видит более одного врага - пытается кинуть шилд, защищаясь от слабейшего
	eid_t weak_enemy = AIAPI::getLeastDangerousEnemy(enemies, EnemyComparator(m_entity));
	if(enemy != weak_enemy)
	{
		// врагов более одного - попробуем кинуть в слабого шилдом
		if(AIAPI::takeShield(m_entity))
		{
			AIAPI::print(m_entity, "|F|thinkAttack: а мы его шилдом!");
			AIAPI::throwShield(m_entity, weak_enemy, &m_activity);
			m_turn_state = TS_INPROGRESS;
			return false;
		}
	}

	// 6.5 Если не повернут к врагу - поворачивается в направлении где был враг
	m_target_dir = norm_angle(Dir2Angle(AIAPI::getPos3(enemy) - getPosition()));
	float cur_angle = norm_angle(AIAPI::getAngle(m_entity));
	if( fabs(m_target_dir - cur_angle) >= angle_eps )
	{
		// нужно поворачиваться к врагу
		AIAPI::setStandPose(m_entity);
		m_activity = CreateRotateActivity(m_target_dir, ActivityFactory::CT_ENEMY_ROTATE);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	// 6.6 повернут к врагу - если сможем будем атаковать, присядем
	AIAPI::setSitPose(m_entity);
	
	AIAPI::print(m_entity, "|F|thinkAttack: будем стрелять, если это возможно!");
	// враг есть - выстрелим по нему если это возможно
	if(!AIAPI::takeBestWeapon(m_entity, FixedWeaponComparator()))
	{
		AIAPI::takeGrenade(m_entity);
	}
	AIAPI::setShootType(m_entity, 80.0f, 15.0f);
	std::string reason;
	if(AIAPI::shoot(m_entity, enemy, &m_activity, &reason))
	{
		m_turn_state = TS_INPROGRESS;
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_NO_LOF);
	}
	else
	{
		std::string str = "|F|thinkAttack: стрелять нельзя: ";
		str += reason;
		AIAPI::print(m_entity, str.c_str());
		AIAPI::setSitPose(m_entity);
		m_turn_state = TS_END;
	}
//	m_turn_state = TS_END;
	return false;
}

//=====================================================================================//
//                       bool FixedEnemyNode::thinkReturn2Base()                       //
//=====================================================================================//
// обработка режима Return2Base
bool FixedEnemyNode::thinkReturn2Base(state_type* st)
{
	// 1. проверим пришли мы уже в базовую точку или нет
	if(m_basepnt == AIAPI::getPos2(m_entity))
	{
		// пришли - переход в базовое состояние
		AIAPI::print(m_entity, "|F|thinkReturn2Base: пришли в базовую точку");
		m_mode = FEM_BASE;
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	// 2. не закончились ли мувпоинты?
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED
		|| AIAPI::getMovePoints(m_entity) == 0)
	{
		// закончились - отдадим ход
		AIAPI::print(m_entity, "|F|thinkReturn2Base: закончились мувпоинты");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// мы находимся не в базовой точке
	// 3. проверим, можно ли вообще дойти до базовой точки
	pnt_vec_t path;
	PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, 0, 0);
	PathUtils::CalcPath(AIAPI::getPtr(m_entity),	m_basepnt, path);
	if(!path.empty())
	{
		// можно - пойдем в нее
		m_target_pnt = m_basepnt;
		m_activity = CreateMoveActivity(path, ActivityFactory::CT_ENEMY_MOVEIGNORE);
		m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// в базовую точку дойти нельзя
	// 4. находимся ли мы на базовом поле?
	if(AIAPI::isEntityInBaseField(m_entity, m_base_field))
	{
		// да - переход в базовое состояние
		AIAPI::print(m_entity, "|F|thinkReturn2Base: пришли в базовое поле");
		m_mode = FEM_BASE;
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	
	// мы вне базового поля
	// 5. можно ли дойти до какой-нибудь точки в базовом поле?
	pnt_vec_t vec;
	AIAPI::getReachableField(m_base_field, &vec, true);
	if(vec.empty())
	{
		// нельзя - переход в базовое состояние
		AIAPI::print(m_entity, "|F|thinkReturn2Base: не могу дойти даже до базового поля!");
		m_mode = FEM_BASE;
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	// можно
	// выберем случайную точку
	m_target_pnt = AIAPI::getRandomPoint(vec);
	PathUtils::CalcPath(AIAPI::getPtr(m_entity),	m_target_pnt, path);
	// пойдем в нее
	m_activity = CreateMoveActivity(path, ActivityFactory::CT_ENEMY_MOVEIGNORE);
	m_activity->Attach(&m_activity_observer,		ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

//=====================================================================================//
//                      bool FixedEnemyNode::thinkWeaponSearch()                       //
//=====================================================================================//
// обработка режима WeaponSearch
bool FixedEnemyNode::thinkWeaponSearch(state_type* st)
{
	// 1. проверим свое здоровье, если оно < половины - пошлем сообщение
	SendCureMessage(fixed_cure_dist);
	// 2. проверим есть ли где-нить рядом оружие
	if(AIAPI::getWeaponLocation(m_entity, &m_target_pnt))
	{
		// недалеко есть оружие - переход к его захвату
		pnt_vec_t base, vec, path;
		AIAPI::getBaseField(m_target_pnt, 1, &base);
		AIAPI::getReachableField(base, &vec, true);
		m_target_pnt = AIAPI::getRandomPoint(vec);

		PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, 0, 0);
		PathUtils::CalcPath(AIAPI::getPtr(m_entity),	m_target_pnt, path);
		if(path.empty())// к оружию нельзя подойти - отдадим ход
		{
			m_turn_state = TS_END;
			return false;
		}
		// можно идти
		m_activity = CreateMoveActivity(path, ActivityFactory::CT_ENEMY_MOVEIGNORE);
		m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		m_mode = FEM_WEAPONTAKE;
		return false;
	}

	// оружия рядом нет - проверим не кончились ли у нас мувпоинты
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED
		|| AIAPI::getMovePoints(m_entity) == 0)
	{
		// не хватило мувпоинтов
		AIAPI::print(m_entity, "|F|thinkWeaponSearch: не хватило мувпоинтов");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}
	
	// если здоровье меньше максимума - попробуем себя полечить
	SelfCure();

	// мувпоинты есть - выберем случайную точку и ломанемся туда
	pnt_vec_t vec, path;
	AIAPI::getReachableField(m_base_field, &vec, true);
	m_target_pnt = AIAPI::getRandomPoint(vec);

	PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, 0, 0);
	PathUtils::CalcPath(AIAPI::getPtr(m_entity),m_target_pnt, path);
	if(path.empty())
	{
		// некуда идти - передадим ход
		m_turn_state = TS_END;
		return false;
	}
	// можно идти
	m_activity = CreateMoveActivity(path, ActivityFactory::CT_ENEMY_MOVEIGNORE);
	m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

//=====================================================================================//
//                       bool FixedEnemyNode::thinkWeaponTake()                        //
//=====================================================================================//
// обработка режима WeaponTake
bool FixedEnemyNode::thinkWeaponTake(state_type* st)
{
	// 1. проверим не подошли ли мы к оружию
	if(PickUpWeaponAndAmmo(m_target_pnt, FixedWeaponComparator()))
	{
		m_mode = FEM_RETURN2BASE;
		return false;
	}

	// 2. проверим не кончились ли у нас мувпоинты
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// не хватило мувпоинтов
		AIAPI::print(m_entity, "|F|thinkWeaponTake: не хватило мувпоинтов");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 3. проверим есть ли где-нить рядом оружие
	if(AIAPI::getWeaponLocation(m_entity, &m_target_pnt))
	{
		// недалеко есть оружие - пойдем к нему
		pnt_vec_t base, vec, path;
		AIAPI::getBaseField(m_target_pnt, 1, &base);
		AIAPI::getReachableField(base, &vec, true);
		m_target_pnt = AIAPI::getRandomPoint(vec);

		PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, 0, 0);
		PathUtils::CalcPath(AIAPI::getPtr(m_entity),m_target_pnt, path);
		if(path.empty())
		{
			// к оружию нельзя подойти - возврат на базу
			m_mode = FEM_RETURN2BASE;
			m_turn_state = TS_INPROGRESS;
			return false;
		}
		// можно идти
		m_activity = CreateMoveActivity(path, ActivityFactory::CT_ENEMY_MOVEIGNORE);
		m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// оружия нет - возврат на базу
	m_mode = FEM_RETURN2BASE;
	m_turn_state = TS_INPROGRESS;
	return false;
}

//=====================================================================================//
//                       bool FixedEnemyNode::thinkMedkitTake()                        //
//=====================================================================================//
// обработка режима MedkitTake
bool FixedEnemyNode::thinkMedkitTake(state_type* st)
{
	if(!MedkitTake(m_target_pnt,st))	m_mode = FEM_RETURN2BASE;
	return false;

}

//=====================================================================================//
//                             bool FixedEnemyNode::die()                              //
//=====================================================================================//
// умри!
bool FixedEnemyNode::die()
{
	m_mode = FEM_KILLED;
	return need2Delete();
}

//=====================================================================================//
//                      bool FixedEnemyNode::need2Delete() const                       //
//=====================================================================================//
// нужно ли удалять узел
bool FixedEnemyNode::need2Delete() const
{
	if( (m_mode == FEM_KILLED) && !m_activity) return true;
	return false;
}

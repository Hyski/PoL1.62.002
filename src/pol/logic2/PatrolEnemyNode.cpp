#include "logicdefs.h"

#include "Entity.h"
#include "AIUtils.h"
#include "PathUtils.h"
#include "SubteamNode.h"
#include "PatrolEnemyNode.h"

DCTOR_IMP(PatrolEnemyNode)

//=====================================================================================//
//                         PatrolEnemyNode::PatrolEnemyNode()                          //
//=====================================================================================//
// конструктор - id существа
PatrolEnemyNode::PatrolEnemyNode(eid_t id)
:	CommonEnemyNode(id),
	m_mode(PEM_PATROL), 
	m_enemy_id(0),
	m_enemy_pnt(0, 0),
	m_cure_id(0),
	m_cure_pnt(0, 0),
	m_shoot_pnt(0, 0),
	m_shooted(false),
	m_target_pnt(0, 0),
	m_target_dir(0),
	m_last_enemy_pnt(0, 0)
{
    GameEvMessenger::GetInst()->Attach(this, EV_SHOOT);
    GameEvMessenger::GetInst()->Attach(this, EV_KILL);

	if(!m_entity) return;

	OnSpawnMsg();
}

//=====================================================================================//
//                         PatrolEnemyNode::~PatrolEnemyNode()                         //
//=====================================================================================//
PatrolEnemyNode::~PatrolEnemyNode()
{
    GameEvMessenger::GetInst()->Detach(this);
}

//=====================================================================================//
//                           float PatrolEnemyNode::Think()                            //
//=====================================================================================//
float PatrolEnemyNode::Think(state_type* st)
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
        case PEM_PATROL:    flag = thinkPatrol(st);       break;
        case PEM_ATTACK:    flag = thinkAttack(st);       break;
        case PEM_WEAPONSEARCH: flag = thinkWeaponSearch(st);     break;
        case PEM_THINGTAKE:  flag = thinkThingTake(st);         break;
        case PEM_WEAPONTAKE:  flag = thinkWeaponTake(st);        break;
        case PEM_CURE:       flag = thinkCure(st);       break;
        case PEM_PURSUIT:    flag = thinkPursuit(st);   break;
        }
    }

	if(HumanEntity *hum = AIAPI::getPtr(m_entity)->Cast2Human())
	{
		hum->GetEntityContext()->GetTraits()->AddMovepnts(-1);
	}
	else if(VehicleEntity *veh = AIAPI::getPtr(m_entity)->Cast2Vehicle())
	{
		hum->GetEntityContext()->GetTraits()->SetMovepnts(hum->GetEntityContext()->GetTraits()->GetMovepnts()-1);
	}

	CheckFinished(st, &complexity);
	return complexity;
}

//=====================================================================================//
//                        void PatrolEnemyNode::MakeSaveLoad()                         //
//=====================================================================================//
void PatrolEnemyNode::MakeSaveLoad(SavSlot& st)
{
    if(st.IsSaving())
	{
		// сохраняемся
        st << static_cast<int>(m_mode);
        st << static_cast<int>(m_turn_state);
		st << m_entity;
		st << static_cast<int>(m_activity_observer.getLastEvent());
		st << m_enemy_id << m_enemy_pnt.x << m_enemy_pnt.y << m_cure_id
			<< m_cure_pnt.x << m_cure_pnt.y << m_shoot_pnt.x << m_shoot_pnt.y
			<< m_shooted << m_target_pnt.x << m_target_pnt.y << m_target_dir
			<< m_last_enemy_pnt.x << m_last_enemy_pnt.y;
    }
	else
	{
		// читаемся
		int tmp;
		st >> tmp; m_mode = static_cast<PatrolEnemyMode>(tmp);
		st >> tmp; m_turn_state = static_cast<TurnState>(tmp);
		st >> m_entity;
		st >> tmp; m_activity_observer.setLastEvent(static_cast<ActivityObserver::event_type>(tmp));

		st >> m_enemy_id >> m_enemy_pnt.x >> m_enemy_pnt.y >> m_cure_id
			>> m_cure_pnt.x >> m_cure_pnt.y >> m_shoot_pnt.x >> m_shoot_pnt.y
			>> m_shooted >> m_target_pnt.x >> m_target_pnt.y >> m_target_dir
			>> m_last_enemy_pnt.x >> m_last_enemy_pnt.y;
		
		// то, что восстанавливается без чтения
		m_activity.Reset();
		m_initialising = false;
	}
}

//=====================================================================================//
//                       void PatrolEnemyNode::recieveMessage()                        //
//=====================================================================================//
// обработка внутрикомандных сообщений
void PatrolEnemyNode::recieveMessage(MessageType type, void * data)
{
	eid_t id = *static_cast<eid_t*>(data);
	switch(type)
	{
	case MT_ENEMYSPOTTED :
		{
			if(m_enemy_id)
			{
				// уже сообщали о враге, проверим кто из врагов ближе
				float old_dist = dist(AIAPI::getPos2(m_entity),	m_enemy_pnt);
				float new_dist = dist(AIAPI::getPos2(m_entity),	AIAPI::getPos2(id));
				if(new_dist < old_dist)
				{  	// сменим врага
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
	case MT_NEEDCURE :
		{
			if(m_cure_id)
			{
				// уже просили о помощи, проверим кто из просящих ближе
				float old_dist = dist(AIAPI::getPos2(m_entity),	m_cure_pnt);
				float new_dist = dist(AIAPI::getPos2(m_entity),	AIAPI::getPos2(id));
				if(new_dist < old_dist)
				{	// сменим просящего о помощи
					m_cure_id = id;
					m_cure_pnt = AIAPI::getPos2(m_cure_id);
				}
				return;
			}
			// это первое сообщение о помощи
			m_cure_id = id;
			m_cure_pnt = AIAPI::getPos2(m_cure_id);
			return;
		}
	default : return;
	}
}

//=====================================================================================//
//                           void PatrolEnemyNode::Update()                            //
//=====================================================================================//
void PatrolEnemyNode::Update(subject_t subj, event_t event, info_t ptr)
{
    switch(event)
	{
    case EV_SHOOT:
        {
            shoot_info* info = static_cast<shoot_info*>(ptr);
            OnShootEv(info->m_actor, info->m_victim, info->m_point);
        }
        break;
    case EV_KILL:
        {
            kill_info* info = static_cast<kill_info*>(ptr);
            OnKillEv(info->m_killer, info->m_victim);
        }
        break;
    }
}

//=====================================================================================//
//                          void PatrolEnemyNode::OnKillEv()                           //
//=====================================================================================//
//обработка собщения об убийстве
void PatrolEnemyNode::OnKillEv(BaseEntity* killer, BaseEntity* victim)
{
	// почистим ссылки на людей
	if(victim == AIAPI::getPtr(m_enemy_id)) m_enemy_id = 0;
	if(victim == AIAPI::getPtr(m_cure_id)) m_cure_id = 0;
}

//=====================================================================================//
//                          void PatrolEnemyNode::OnShootEv()                          //
//=====================================================================================//
// обработка сообщения об выстреле или попадании
void PatrolEnemyNode::OnShootEv(BaseEntity* who, BaseEntity* victim, const point3& where)
{
	if(!m_entity) return;

	// не будем реагировать в случае,
	// если юнит находится в определенных состояниях
	switch(m_mode)
	{
	case PEM_ATTACK: case PEM_WEAPONSEARCH: case PEM_WEAPONTAKE : return;
	default: break;
	}

	if(AIAPI::getPtr(m_entity) == who) return; // это я сам натворил

	float dist = (getPosition() - where).Length();
	if( (dist < alarm_dist) || (AIAPI::getPtr(m_entity) == victim) )
	{
		// будем реагировать
		m_shoot_pnt = AIAPI::getPos2(who);
		m_shooted = true;
	}
}

//=====================================================================================//
//                       bool PatrolEnemyNode::switchToAttack()                        //
//=====================================================================================//
bool PatrolEnemyNode::switchToAttack(eid_t enemy)
{
	int index = 0;
	const shot_type stypes[] = {SHT_AIMSHOT,SHT_AUTOSHOT,SHT_SNAPSHOT};
	const ActivityFactory::controller_type ctrlType = ActivityFactory::CT_ENEMY_MOVE;

	// выберем лучший хекс так, чтобы потом хватило мувпоинтов
	// на один выстрел aim, если таких хексов нет - считает
	// лучшим хекс, в котором стоит.

	// получим общее кол-во оставшихся мувпонитов
	int movpoints = AIAPI::getMovePoints(m_entity);
	int mps4shot = AIAPI::getShootMovePoints(m_entity, SHT_AIMSHOT);
	// вычтем мувпоинты на поворот к врагу
	movpoints -= AIUtils::CalcMps4EntityRotate(PIm2);
	// вычтем мувпоинты на приседание
	movpoints -= PoL::Inv::MpsForPoseChange;

	// вычтем количество мувпоинтов необходимых для выстрела
	movpoints -= mps4shot;

	int steps = 0;
	bool shot = true;

	while(AIAPI::getSteps(m_entity,movpoints) <= 0
		&& AIUtils::CalcEfficiency(AIAPI::getPtr(m_entity),AIAPI::getPos3(enemy)) == 0
		&& index < 2)
	{
		movpoints += mps4shot;
		mps4shot = AIAPI::getShootMovePoints(m_entity,stypes[++index]);
		movpoints -= mps4shot;
	}

	if(index == 2 && AIAPI::getSteps(m_entity,movpoints) <= 0)
	{
		movpoints += mps4shot;
		shot = false;
	}

	if(steps = AIAPI::getSteps(m_entity,movpoints))
	{
		do
		{
			pnt_vec_t field, vec;
			// рассчитаем поле проходимости
			PathUtils::CalcPassField(AIAPI::getPtr(m_entity), 0, 0, 0);
			AIAPI::getBaseField(AIAPI::getPos2(m_entity), steps, &field);
			// получим вектор доступных хексов
			AIAPI::getReachableField(field, &vec, true);
			// получим лучший хекс
			m_target_pnt = AIAPI::getBestPoint(vec,	PatrolHexComparator(m_entity, enemy));

			if(m_target_pnt == AIAPI::getPos2(m_entity) && index < 2)
			{
				movpoints += mps4shot;
				mps4shot = AIAPI::getShootMovePoints(m_entity,stypes[++index]);
				movpoints -= mps4shot;
			}
			else if(m_target_pnt == AIAPI::getPos2(m_entity) && index == 2 && shot)
			{
				movpoints += mps4shot;
				shot = false;
			}
			else
			{
				break;
			}

			steps = AIAPI::getSteps(m_entity,movpoints);
		}
		while(true);

		if(m_target_pnt != AIAPI::getPos2(m_entity))
		{
			pnt_vec_t path;
			PathUtils::CalcPath(AIAPI::getPtr(m_entity), m_target_pnt, path);
			// можно идти
			m_activity = CreateMoveActivity(path, ctrlType);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_REACTED);
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}


//=====================================================================================//
//                         bool PatrolEnemyNode::thinkPatrol()                         //
//=====================================================================================//
// обработка режима Patrol
bool PatrolEnemyNode::thinkPatrol(state_type* st)
{
	// 1. проверим свое здоровье, если оно < половины - пошлем сообщение
	SendCureMessage(patrol_cure_dist);

	// 2. если у нас нет оружия - перейдем к его поиску
	if( (!AIAPI::takeGrenade(m_entity)) &&
		(!AIAPI::takeBestWeapon(m_entity, PatrolWeaponComparator()))	)
	{
		AIAPI::print(m_entity, "|P|thinkPatrol: надо искать пушку!");
		m_mode = PEM_WEAPONSEARCH;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 3. если я вижу врага - пошлю сообщение и перейду в атаку
	AIAPI::entity_list_t enemies;
	if(AIAPI::getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// я кого-то вижу
		if(eid_t enemy = AIAPI::getMostDangerousEnemy(enemies,EnemyComparator(m_entity))	)
		{
			AIAPI::print(m_entity, "|P|thinkPatrol: я увидел врага!");
			// выбрал самого опасного врага
			// пошлю сообщение своей команде
			getSubteam()->sendMessage(this,	AIEnemyNode::MT_ENEMYSPOTTED,	&enemy,	patrol_enemy_dist);
			// переход в состояние атаки
			m_mode = PEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_last_enemy_pnt = AIAPI::getPos2(enemy);
			AIAPI::setRun(m_entity);

			switchToAttack(enemy);
			return false;

			//// выберем лучший хекс так, чтобы потом хватило мувпоинтов
			//// на один выстрел aim, если таких хексов нет - считает
			//// лучшим хекс, в котором стоит.

			//// получим общее кол-во оставшихся мувпонитов
			//int movpoints = AIAPI::getMovePoints(m_entity);
			//// вычтем количество мувпоинтов необходимых для выстрела
			//movpoints -= AIAPI::getShootMovePoints(m_entity,	SHT_AIMSHOT);
			//// вычтем мувпоинты на поворот к врагу
			//movpoints -= AIUtils::CalcMps4EntityRotate(PIm2);
			//int steps = AIAPI::getSteps(m_entity, movpoints);

			//if(!steps) // будем считать что стоим где надо, просто переходим в, атаку
			//	return false;
			//// рассчитаем поле проходимости
			//pnt_vec_t field, vec, path;
			//PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, 0, 0);
			//AIAPI::getBaseField(AIAPI::getPos2(m_entity),	steps, &field);
			//// получим вектор доступных хексов
			//AIAPI::getReachableField(field, &vec, true);
			//// получим лучший хекс
			//m_target_pnt = AIAPI::getBestPoint(vec, PatrolHexComparator(m_entity, enemy));
			//PathUtils::CalcPath(AIAPI::getPtr(m_entity),	m_target_pnt, path);
			//// можно идти
			//m_activity = CreateMoveActivity(path,	ActivityFactory::CT_ENEMY_MOVE);
			//m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			//m_activity->Attach(&m_activity_observer,ActivityObserver::EV_ENEMY_REACTED);
			//return false;
		}
	}

	// 4. Если в меня стреляли - будем преследовать
	if(m_shooted)
	{
		AIAPI::print(m_entity, "|P|thinkPatrol: в меня стреляли - будем преследовать");
		// в меня или рядом со мной стреляли
		m_shooted = false;
		// выберем точку в радиусе 3 хексов от существа, которое стреляло
		// пойдем туда и перейдем при этом в состояние преследования

		// рассчитаем поле проходимости
		pnt_vec_t field, vec, path;
		PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, 0, 0);
		AIAPI::getBaseField(m_shoot_pnt,	3, &field);
		// получим вектор доступных хексов
		AIAPI::getReachableField(field, &vec, true);
		// получим пока случайный хекс
		m_target_pnt = AIAPI::getRandomPoint(vec);
		PathUtils::CalcPath(AIAPI::getPtr(m_entity),	m_target_pnt, path);
		// можно идти
		m_activity = CreateMoveActivity(path,ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer,	ActivityObserver::EV_MOVEPNTS_EXPIRIED);

		m_mode = PEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 5. если мне сообщили о враге - пойдем к нему
	if(m_enemy_id)
	{
		AIAPI::print(m_entity, "|P|thinkPatrol: мне сообщили о враге");
		// сообщили
		m_enemy_id = 0;
		// выберем точку в радиусе 1 хекса от врага
		// пойдем туда и перейдем при этом в состояние преследования

		// рассчитаем поле проходимости
		PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, 0, 0);
		pnt_vec_t field;
		AIAPI::getBaseField(m_enemy_pnt,	1, &field);
		// получим вектор доступных хексов
		pnt_vec_t vec;
		AIAPI::getReachableField(field, &vec, true);
		// получим пока случайный хекс
		m_target_pnt = AIAPI::getRandomPoint(vec);
		pnt_vec_t path;
		PathUtils::CalcPath(AIAPI::getPtr(m_entity),	m_target_pnt, path);
		// можно идти
		m_activity = CreateMoveActivity(path,ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);

		m_mode = PEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 6. если есть кого лечить - переходим в состояние лечения
	if(m_cure_id)
	{
		// проверим здоровье юнита
		if( (AIAPI::getHealthPercent(m_cure_id) < 50.0f) &&
			(AIAPI::haveMedikit(m_entity))	)
		{
			AIAPI::print(m_entity, "|P|thinkPatrol: нужно лечить");
			// его нужно лечить
			m_mode = PEM_CURE;
			m_turn_state = TS_INPROGRESS;
			return false;
		}
		// его не нужно лечить
		m_cure_id = 0;
	}

	// 7. если здоровье меньше максимума - попробуем себя полечить
	SelfCure();

	// 8. если увидели на уровне оружие, патроны или аптечку - 
	// переход в состояние захват вещи
	ipnt2_t pos;
	if(AIAPI::getThingLocation(m_entity,	&pos,TT_WEAPON|TT_AMMO|TT_MEDIKIT)	)
	{
		AIAPI::print(m_entity, "|P|thinkPatrol: увидел на уровне оружие, патроны или аптечку");
		pnt_vec_t base;
		pnt_vec_t vec;
		AIAPI::getBaseField(pos, 1, &base);
		AIAPI::getReachableField(base, &vec, true);
		pos = AIAPI::getRandomPoint(vec);
		PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, 0, 0);
		pnt_vec_t path;
		PathUtils::CalcPath(AIAPI::getPtr(m_entity),	pos, path);
		if(!path.empty())
		{
			// можно идти
			m_target_pnt = pos;
			m_activity = CreateMoveActivity(path,ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_turn_state = TS_INPROGRESS;
			m_mode = PEM_THINGTAKE;
			return false;
		}
	}

	// 9. если дошел до заданной точки - выбирает новую
	if(m_target_pnt == AIAPI::getPos2(m_entity))
	{
		AIAPI::print(m_entity, "|P|thinkPatrol: дошел до заданной точки - выбирает новую");
		// рассчитаем поле проходимости
		pnt_vec_t allhexes;
		PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, &allhexes, 0);
		if(allhexes.empty())
		{
			// передаем ход - идти некуда
			AIAPI::print(m_entity, "|P|thinkPatrol: мне некуда ходить!!!");
			m_turn_state = TS_END;
			return false;
		}
		// получим случайную точку
		m_target_pnt = AIAPI::getRandomPoint(allhexes);
	}

	// 10. проверим не кончились ли у нас мувпоинты
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// не хватило мувпоинтов
		AIAPI::print(m_entity, "|P|thinkPatrol: не хватило мувпоинтов");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 10. идем в заданную точку
	AIAPI::print(m_entity, "|P|thinkPatrol: идем в заданную точку");
	PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, 0, 0);
	pnt_vec_t path;
	PathUtils::CalcPath(AIAPI::getPtr(m_entity),	m_target_pnt, path);
	if(path.empty())
	{
		// точка оказалась недостижимой - будем считать, что уже пришли
		// куда надо
		m_target_pnt = AIAPI::getPos2(m_entity);
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	// можно идти
	m_activity = CreateMoveActivity(path,ActivityFactory::CT_ENEMY_MOVE);
	m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

//=====================================================================================//
//                         bool PatrolEnemyNode::thinkAttack()                         //
//=====================================================================================//
// обработка режима Attack
bool PatrolEnemyNode::thinkAttack(state_type* st)
{
	// 1. проверим свое здоровье, если оно < половины - пошлем сообщение
	SendCureMessage(patrol_cure_dist);
	// 2. если у нас нет оружия - перейдем к его поиску
	if( (!AIAPI::takeGrenade(m_entity)) &&
		(!AIAPI::takeBestWeapon(m_entity, PatrolWeaponComparator()))	)
	{
		AIAPI::print(m_entity, "|P|thinkAttack: надо искать пушку!");
		m_mode = PEM_WEAPONSEARCH;
		m_turn_state = TS_INPROGRESS;
		m_activity_observer.clear();
		AIAPI::setWalk(m_entity);
		return false;
	}

	// 3. выберем врага
	AIAPI::entity_list_t enemies;
	eid_t enemy = 0;
	if(AIAPI::getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// сам видит врага, если это начало хода или реакция - сбросим список игнорирования
		if(m_turn_state == TS_START)	m_ignored_enemies.clear();

		// попробуем найти разность списков всех врагов и игнорированных врагов
		EntityList_t diff_list;
		setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);

		if(diff_list.empty())// выберем самого опасного врага из всех
			enemy = AIAPI::getMostDangerousEnemy(enemies,EnemyComparator(m_entity));
		else// выберем самого опасного врага из непроигнорированных
			enemy = AIAPI::getMostDangerousEnemy(diff_list,	EnemyComparator(m_entity));

		m_last_enemy_pnt = AIAPI::getPos2(enemy);
	}
	
	// 4. если нет врагов - переходим в состояние преследования
	if(!enemy)
	{
		// врагов нет - будем преследовать последнего
		AIAPI::print(m_entity, "|P|thinkAttack: врагов нет - будем преследовать последнего");
		m_mode = PEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		m_target_pnt = m_last_enemy_pnt;
		m_activity_observer.clear();
		AIAPI::setWalk(m_entity);
		return false;
	}
	// враги есть...

	// 5. если это начало нового хода
	if(m_turn_state == TS_START)
	{
		// выбираем лучшую точку и идем в нее
		AIAPI::print(m_entity, "|P|thinkAttack: выбираем лучшую точку и идем в нее");

		// выберем лучший хекс так, чтобы потом хватило мувпоинтов
		// на один выстрел aim, если таких хексов нет - считает
		// лучшим хекс, в котором стоит.
		
		m_turn_state = TS_INPROGRESS;
		AIAPI::setRun(m_entity);

		if(switchToAttack(enemy))
		{
			return false;
		}
	}

	// 6. это продолжение хода
	// 6.1 если юниту не хватило мувпоинтов - он передает ход
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// закончились - закончим ход
		AIAPI::print(m_entity, "|P|thinkAttack: закончились мувпоинты");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 6.2 если сработала реакция или увидел нового врага - имитируем
	//       начало хода
	if( (m_activity_observer.getLastEvent() == ActivityObserver::EV_ENEMY_REACTED) ||
		(m_activity_observer.getLastEvent() == ActivityObserver::EV_ENEMY_SPOTTED)	)
	{
		AIAPI::print(m_entity, "|P|thinkAttack: сработала реакция или увидел нового врага");
		m_activity_observer.clear();
		// имитируем начало хода
		m_ignored_enemies.clear();
		m_turn_state = TS_START;
		return true;
	}

	// 6.3 если нет линии огня - добавляем врага в список игнорирования
	// причем, если после этого все враги оказались в списке игнорирования
	// попробуем подойти к одному из игнорированных врагов
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_NO_LOF)
	{
		// закончились - закончим ход
		AIAPI::print(m_entity, "|P|thinkAttack: нет линии огня - добавляем врага в список игнорирования");
		m_activity_observer.clear();
		// добавим в список игнорирования
		m_ignored_enemies.insert(enemy);
		EntityList_t diff_list;
		setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);
		if(diff_list.empty())
		{
			// попробуем подойти поближе к врагу
			AIAPI::print(m_entity, "|P|thinkAttack: подойдем поближе");
			// рассчитаем поле проходимости
			PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, 0, PathUtils::F_CALC_NEAR_PNTS);
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
				AIAPI::getBaseField(AIAPI::getPos2(m_entity),	patrol_enemy_noLOF_move,&base);
				AIAPI::getReachableField(base, &vec, true);
				m_target_pnt = AIAPI::getRandomPoint(vec);
				PathUtils::CalcPath(AIAPI::getPtr(m_entity),	m_target_pnt, path);
			}
			else
			{
				// есть враг, к которому можно подойти - подойдем на половину пути
				PathUtils::CalcPath(AIAPI::getPtr(m_entity),	m_target_pnt, path);
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
			m_activity = CreateMoveActivity(path,ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_REACTED);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_SPOTTED);
			
			m_turn_state = TS_INPROGRESS;
			return false;
		} 
		// зациклимся для поиска других врагов
		m_turn_state = TS_INPROGRESS;
		return true;
	}

	// 6.4 Если не повернут к врагу - поворачивается в направлении где был враг
	m_target_dir = norm_angle(Dir2Angle(AIAPI::getPos3(enemy) - getPosition()));
	float cur_angle = norm_angle(AIAPI::getAngle(m_entity));
	if( fabs(m_target_dir - cur_angle) >= angle_eps )
	{
		// нужно поворачиваться к врагу
		m_activity = CreateRotateActivity(m_target_dir,ActivityFactory::CT_ENEMY_ROTATE);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	
	// 6.5 повернут к врагу - если сможем будем атаковать
	AIAPI::print(m_entity, "|P|thinkAttack: будем стрелять, если это возможно!");
	// враг есть - выстрелим по нему если это возможно

	// сначала попробуем гранатой (если есть и если получится)
	if(AIAPI::takeGrenade(m_entity))
	{
		// граната в руках - попробуем кинуть ее
		std::string reason;
		if(AIAPI::shoot(m_entity, enemy, &m_activity, &reason))
		{
			// удалось начать бросок гранаты
			m_turn_state = TS_INPROGRESS;
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_NO_LOF);
			return false;
		}
		// начать бросок гранаты не удалось
		std::string str = "|P|thinkAttack: нельзя кинуть гранату: ";
		str += reason;
		AIAPI::print(m_entity, str.c_str());
	}

	// гранаты или нет, или бросить ее не удалось
	// теперь попробуем пострелять из оружия
	if(!AIAPI::takeBestWeapon(m_entity, PatrolWeaponComparator()))
	{
		m_turn_state = TS_END;
		return false;
	}
	// оружие в руках
	AIAPI::setShootType(m_entity, 5.0f, 10.0f);
	std::string reason;
	if(AIAPI::shoot(m_entity, enemy, &m_activity, &reason))
	{
		// удалось начать стрельбу
		m_turn_state = TS_INPROGRESS;
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_NO_LOF);
	}
	else
	{
		if(AIUtils::CalcEfficiency(AIAPI::getPtr(m_entity),AIAPI::getPos3(enemy)) == 0)
		{
			// Нулевая эффективность -- имитируем начало хода, если осталось больше 10 мувпойнтов
			if(AIAPI::getSteps(m_entity,AIAPI::getMovePoints(m_entity)) > 2)
			{
				m_turn_state = TS_START;
				return false;
			}
		}
		std::string str = "|P|thinkAttack: стрелять нельзя: ";
		str += reason;
		AIAPI::print(m_entity, str.c_str());
		m_turn_state = TS_END;
	}
	return false;
}

//=====================================================================================//
//                      bool PatrolEnemyNode::thinkWeaponSearch()                      //
//=====================================================================================//
// обработка режима WeaponSearch
bool PatrolEnemyNode::thinkWeaponSearch(state_type* st)
{
	// 1. проверим свое здоровье, если оно < половины - пошлем сообщение
	SendCureMessage(patrol_cure_dist);

	// 2. проверим есть ли где-нить рядом оружие
	if(AIAPI::getWeaponLocation(m_entity, &m_target_pnt))
	{
		// недалеко есть оружие - переход к его захвату
		AIAPI::print(m_entity, "|P|thinkWeaponSearch: недалеко есть оружие - переход к его захвату!");
		pnt_vec_t base;
		pnt_vec_t vec;
		AIAPI::getBaseField(m_target_pnt, 1, &base);
		AIAPI::getReachableField(base, &vec, true);
		m_target_pnt = AIAPI::getRandomPoint(vec);
		PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, 0, 0);
		pnt_vec_t path;
		PathUtils::CalcPath(AIAPI::getPtr(m_entity),	m_target_pnt, path);
		if(path.empty())
		{
			// к оружию нельзя подойти - отдадим ход
			m_turn_state = TS_END;
			return false;
		}
		// можно идти
		m_activity = CreateMoveActivity(path,ActivityFactory::CT_ENEMY_MOVEIGNORE);
		m_activity->Attach(&m_activity_observer,	ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		m_mode = PEM_WEAPONTAKE;
		return false;
	}

	// 3. если здоровье меньше максимума - попробуем себя полечить
	SelfCure();

	// 4. если дошел до заданной точки - выбирает новую
	if(m_target_pnt == AIAPI::getPos2(m_entity))
	{
		AIAPI::print(m_entity, "|P|thinkWeaponSearch: дошел до заданной точки - выбирает новую");
		// рассчитаем поле проходимости
		pnt_vec_t allhexes;
		PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, &allhexes, 0);
		if(allhexes.empty())
		{
			// передаем ход - идти некуда
			AIAPI::print(m_entity, "|P|thinkWeaponSearch: мне некуда ходить!!!");
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
		AIAPI::print(m_entity, "|P|thinkWeaponSearch: не хватило мувпоинтов");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 6. идем в заданную точку
	AIAPI::print(m_entity, "|P|thinkWeaponSearch: идем в заданную точку");
	PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, 0, 0);
	pnt_vec_t path;
	PathUtils::CalcPath(AIAPI::getPtr(m_entity), m_target_pnt, path);
	if(path.empty())
	{
		// точка оказалась недостижимой - будем считать, что уже пришли
		// куда надо
		m_target_pnt = AIAPI::getPos2(m_entity);
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	// можно идти
	m_activity = CreateMoveActivity(path,ActivityFactory::CT_ENEMY_MOVE);
	m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

//=====================================================================================//
//                       bool PatrolEnemyNode::thinkThingTake()                        //
//=====================================================================================//
// обработка режима ThingTake
bool PatrolEnemyNode::thinkThingTake(state_type* st)
{
	// 1. проверим свое здоровье, если оно < половины - пошлем сообщение
	SendCureMessage(patrol_cure_dist);

	// 2. если я вижу врага - пошлю сообщение и перейду в атаку
	AIAPI::entity_list_t enemies;
	if(AIAPI::getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// я кого-то вижу
		if(eid_t enemy = AIAPI::getMostDangerousEnemy(enemies, EnemyComparator(m_entity)) )
		{
			AIAPI::print(m_entity, "|P|thinkThingTake: я увидел врага!");
			// выбрал самого опасного врага
			// пошлю сообщение своей команде
			getSubteam()->sendMessage(this, AIEnemyNode::MT_ENEMYSPOTTED, &enemy, patrol_enemy_dist);
			// переход в состояние атаки
			m_mode = PEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_last_enemy_pnt = AIAPI::getPos2(enemy);
			AIAPI::setRun(m_entity);

			switchToAttack(enemy);
			return false;
		}
	}

	// 3. проверим не подошли ли мы к вещи
	if(m_target_pnt == AIAPI::getPos2(m_entity))
	{
		// мы подошли к валяющимся вещам - возьмем что сможем
		AIAPI::print(m_entity, "|P|thinkThingTake: мы подошли к валяющимся вещам - возьмем что сможем");
		AIAPI::pickupAllNearAmmo(m_entity);
		AIAPI::pickupAllNearMedikit(m_entity);
		AIAPI::pickupAllNearWeapon(m_entity);
		AIAPI::dropUselessWeapon(m_entity, PatrolWeaponComparator());
		// переходим в состояние патрулирования
		m_mode = PEM_PATROL;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 4. Если в меня стреляли - будем преследовать
	if(m_shooted)
	{
		AIAPI::print(m_entity, "|P|thinkThingTake: в меня стреляли - будем преследовать");
		// в меня или рядом со мной стреляли
		m_shooted = false;
		// выберем точку в радиусе 3 хексов от существа, которое стреляло
		// пойдем туда и перейдем при этом в состояние преследования

		// рассчитаем поле проходимости
		PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, 0, 0);
		pnt_vec_t field;
		AIAPI::getBaseField(m_shoot_pnt,	3, &field);
		// получим вектор доступных хексов
		pnt_vec_t vec;
		AIAPI::getReachableField(field, &vec, true);
		// получим пока случайный хекс
		m_target_pnt = AIAPI::getRandomPoint(vec);
		pnt_vec_t path;
		PathUtils::CalcPath(AIAPI::getPtr(m_entity),	m_target_pnt, path);
		// можно идти
		m_activity = CreateMoveActivity(path,ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);

		m_mode = PEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 5. если мне сообщили о враге - пойдем к нему
	if(m_enemy_id)
	{
		AIAPI::print(m_entity, "|P|thinkThingTake: мне сообщили о враге");
		// сообщили
		m_enemy_id = 0;
		// выберем точку в радиусе 1 хекса от врага
		// пойдем туда и перейдем при этом в состояние преследования

		// рассчитаем поле проходимости
		PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, 0, 0);
		pnt_vec_t field;
		AIAPI::getBaseField(m_enemy_pnt,	1, &field);
		// получим вектор доступных хексов
		pnt_vec_t vec;
		AIAPI::getReachableField(field, &vec, true);
		// получим пока случайный хекс
		m_target_pnt = AIAPI::getRandomPoint(vec);
		pnt_vec_t path;
		PathUtils::CalcPath(AIAPI::getPtr(m_entity),	m_target_pnt, path);
		// можно идти
		m_activity = CreateMoveActivity(path,ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);

		m_mode = PEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 6. если есть кого лечить - переходим в состояние лечения
	if(m_cure_id)
	{
		// проверим здоровье юнита
		if( (AIAPI::getHealthPercent(m_cure_id) < 50.0f) &&	(AIAPI::haveMedikit(m_entity)))
		{
			AIAPI::print(m_entity, "|P|thinkThingTake: нужно лечить");
			// его нужно лечить
			m_mode = PEM_CURE;
			m_turn_state = TS_INPROGRESS;
			return false;
		}
		// его не нужно лечить
		m_cure_id = 0;
	}
	
	// 7. если здоровье меньше максимума - попробуем себя полечить
	SelfCure();

	// 8. проверим не кончились ли у нас мувпоинты
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// не хватило мувпоинтов
		AIAPI::print(m_entity, "|P|thinkThingTake: не хватило мувпоинтов");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 9. проверим есть ли где-нить рядом оружие
	if(AIAPI::getThingLocation(m_entity, &m_target_pnt,	TT_AMMO|TT_WEAPON|TT_MEDIKIT)	)
	{
		// недалеко есть оружие - пойдем к нему
		pnt_vec_t base;
		pnt_vec_t vec;
		AIAPI::getBaseField(m_target_pnt, 1, &base);
		AIAPI::getReachableField(base, &vec, true);
		m_target_pnt = AIAPI::getRandomPoint(vec);
		PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, 0, 0);
		pnt_vec_t path;
		PathUtils::CalcPath(AIAPI::getPtr(m_entity),	m_target_pnt, path);
		if(path.empty())
		{
			// к оружию нельзя подойти - возврат к патрулированию
			m_mode = PEM_PATROL;
			m_turn_state = TS_INPROGRESS;
			return false;
		}
		// можно идти
		m_activity = CreateMoveActivity(path,ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// оружия нет - возврат к патрулированию
	m_mode = PEM_PATROL;
	m_turn_state = TS_INPROGRESS;
	return false;
}

//=====================================================================================//
//                       bool PatrolEnemyNode::thinkWeaponTake()                       //
//=====================================================================================//
// обработка режима WeaponTake
bool PatrolEnemyNode::thinkWeaponTake(state_type* st)
{
	SendCureMessage(patrol_cure_dist);

	// 2. проверим не подошли ли мы к оружию
	if(PickUpWeaponAndAmmo(m_target_pnt, PatrolWeaponComparator()))
	{
		m_mode = PEM_PATROL;
		return false;
	}

	// 3. проверим не кончились ли у нас мувпоинты
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// не хватило мувпоинтов
		AIAPI::print(m_entity, "|P|thinkWeaponTake: не хватило мувпоинтов");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 4. проверим есть ли где-нить рядом оружие
	if(AIAPI::getWeaponLocation(m_entity, &m_target_pnt))
	{
		// недалеко есть оружие - пойдем к нему
		pnt_vec_t base;
		pnt_vec_t vec;
		AIAPI::getBaseField(m_target_pnt, 1, &base);
		AIAPI::getReachableField(base, &vec, true);
		m_target_pnt = AIAPI::getRandomPoint(vec);
		PathUtils::CalcPassField(AIAPI::getPtr(m_entity),
			0, 0, 0);
		pnt_vec_t path;
		PathUtils::CalcPath(AIAPI::getPtr(m_entity),
			m_target_pnt, path);
		if(path.empty())
		{
			// к оружию нельзя подойти - возврат к патрулированию
			m_mode = PEM_PATROL;
			m_turn_state = TS_INPROGRESS;
			return false;
		}
		// можно идти
		m_activity = CreateMoveActivity(path, ActivityFactory::CT_ENEMY_MOVEIGNORE);
		m_activity->Attach(&m_activity_observer,
			ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// оружия нет - возврат к патрулированию
	m_mode = PEM_PATROL;
	m_turn_state = TS_INPROGRESS;
	return false;
}

//=====================================================================================//
//                          bool PatrolEnemyNode::thinkCure()                          //
//=====================================================================================//
// обработка режима Cure
bool PatrolEnemyNode::thinkCure(state_type* st)
{
	// 1. проверим свое здоровье, если оно < половины - пошлем сообщение
	SendCureMessage(patrol_cure_dist);

	// 2. если я вижу врага - пошлю сообщение и перейду в атаку
	AIAPI::entity_list_t enemies;
	if(AIAPI::getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// я кого-то вижу
		if(eid_t enemy = AIAPI::getMostDangerousEnemy(enemies,
			EnemyComparator(m_entity))
			)
		{
			AIAPI::print(m_entity, "|P|thinkCure: я увидел врага!");
			// выбрал самого опасного врага
			// пошлю сообщение своей команде
			getSubteam()->sendMessage(this,
				AIEnemyNode::MT_ENEMYSPOTTED,
				&enemy,
				patrol_enemy_dist);
			// переход в состояние атаки
			m_mode = PEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_last_enemy_pnt = AIAPI::getPos2(enemy);
			AIAPI::setRun(m_entity);

			switchToAttack(enemy);
			return false;
		}
	}

	// 3. Если в меня стреляли - будем преследовать
	if(m_shooted)
	{
		AIAPI::print(m_entity, "|P|thinkCure: в меня стреляли - будем преследовать");
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
		AIAPI::getReachableField(field, &vec, true);
		// получим пока случайный хекс
		m_target_pnt = AIAPI::getRandomPoint(vec);
		pnt_vec_t path;
		PathUtils::CalcPath(AIAPI::getPtr(m_entity),
			m_target_pnt, path);
		// можно идти
		AIAPI::setWalk(m_entity);
		m_activity = CreateMoveActivity(path, ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer,
			ActivityObserver::EV_MOVEPNTS_EXPIRIED);

		m_mode = PEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 4. если мне сообщили о враге - пойдем к нему
	if(m_enemy_id)
	{
		AIAPI::print(m_entity, "|P|thinkCure: мне сообщили о враге");
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
		AIAPI::getReachableField(field, &vec, true);
		// получим пока случайный хекс
		m_target_pnt = AIAPI::getRandomPoint(vec);
		pnt_vec_t path;
		PathUtils::CalcPath(AIAPI::getPtr(m_entity),
			m_target_pnt, path);
		// можно идти
		AIAPI::setWalk(m_entity);
		m_activity = CreateMoveActivity(path, ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer,
			ActivityObserver::EV_MOVEPNTS_EXPIRIED);

		m_mode = PEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 5. если здоровье меньше максимума - попробуем себя полечить
	SelfCure();

	// 6. если некого или нечем лечить
	if( (!m_cure_id) ||
		(AIAPI::getHealthPercent(m_cure_id) >= 50.0f) ||
		(!AIAPI::haveMedikit(m_entity))
		)
	{
		AIAPI::print(m_entity, "|P|thinkCure: некого или нечем лечить");
		// некого или нечем лечить - переход к патрулированию
		m_cure_id = 0;
		m_mode = PEM_PATROL;
		m_turn_state = TS_INPROGRESS;
		AIAPI::setWalk(m_entity);
		return false;
	}

	// 7. если подошел к юниту - лечим его
	if(dist(AIAPI::getPos2(m_entity),
		AIAPI::getPos2(m_cure_id)) <= 1.5f)
	{
		// подошел - лечим
		AIAPI::print(m_entity, "|P|thinkCure: лечим");
		// возьмем в руки аптечку
		AIAPI::takeMedikit(m_entity);
		// полечим юнита
		AIAPI::cure(m_entity, m_cure_id);
		// возьмем в руки оружие
		if(!AIAPI::takeGrenade(m_entity))
		{
			AIAPI::takeBestWeapon(m_entity, PatrolWeaponComparator());
		}
		// продолжим
		m_turn_state = TS_INPROGRESS;
		return true;
	}
	
	// 8. проверим не кончились ли у нас мувпоинты
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// не хватило мувпоинтов
		AIAPI::print(m_entity, "|P|thinkCure: не хватило мувпоинтов");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 9. идем к юниту, которого нужно лечить
	
	// рассчитаем поле проходимости
	PathUtils::CalcPassField(AIAPI::getPtr(m_entity),
		0, 0, PathUtils::F_CALC_NEAR_PNTS);
	// проверим, существуют ли точки рядом с существом
	if(!PathUtils::IsNear(AIAPI::getPtr(m_cure_id)))
	{
		// к юниту нельзя подойти
		m_cure_id = 0;
		m_mode = PEM_PATROL;
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	m_target_pnt = PathUtils::GetNearPnt(AIAPI::getPtr(m_cure_id)).m_pnt;
	pnt_vec_t path;
	PathUtils::CalcPath(AIAPI::getPtr(m_entity),
		m_target_pnt, path);
	// можно идти
	AIAPI::setRun(m_entity);
	m_activity = CreateMoveActivity(path, ActivityFactory::CT_ENEMY_MOVE);
	m_activity->Attach(&m_activity_observer,
		ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	
	m_turn_state = TS_INPROGRESS;
	return false;
}

//=====================================================================================//
//                        bool PatrolEnemyNode::thinkPursuit()                         //
//=====================================================================================//
// обработка режима Pursuit
bool PatrolEnemyNode::thinkPursuit(state_type* st)
{
	// 1. проверим свое здоровье, если оно < половины - пошлем сообщение
	SendCureMessage(patrol_cure_dist);

	// 2. если я вижу врага - пошлю сообщение и перейду в атаку
	AIAPI::entity_list_t enemies;
	if(AIAPI::getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// я кого-то вижу
		if(eid_t enemy = AIAPI::getMostDangerousEnemy(enemies,
			EnemyComparator(m_entity))
			)
		{
			AIAPI::print(m_entity, "|P|thinkPursuit: я увидел врага!");
			// выбрал самого опасного врага
			// пошлю сообщение своей команде
			getSubteam()->sendMessage(this,
				AIEnemyNode::MT_ENEMYSPOTTED,
				&enemy,
				patrol_enemy_dist);
			// переход в состояние атаки
			m_mode = PEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_last_enemy_pnt = AIAPI::getPos2(enemy);
			AIAPI::setRun(m_entity);

			switchToAttack(enemy);
			return false;
		}
	}

	// 3. проверим не кончились ли у нас мувпоинты
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// не хватило мувпоинтов
		AIAPI::print(m_entity, "|P|thinkPursuit: не хватило мувпоинтов");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 4. проверим не пришли ли мы в заданную точку
	if(m_target_pnt == AIAPI::getPos2(m_entity))
	{
		// мы пришли
		// 4.1 проверим, повернут ли в точку, где был враг
		m_target_dir = norm_angle(Dir2Angle(AIAPI::convertPos2ToPos3(m_last_enemy_pnt) - getPosition()));
		float cur_angle = norm_angle(AIAPI::getAngle(m_entity));
		if( fabs(m_target_dir - cur_angle) >= angle_eps )
		{
			// нужно поворачиваться к врагу
			m_activity = CreateRotateActivity(m_target_dir, ActivityFactory::CT_ENEMY_ROTATE);
			m_activity->Attach(&m_activity_observer,
				ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_turn_state = TS_INPROGRESS;
			return false;
		}
		// 4.2 повернут - переход в состояние патрулирования
		AIAPI::print(m_entity, "|P|thinkPursuit: никого не догнал - будем патрулировать");
		// переходим в состояние патрулирования
		m_mode = PEM_PATROL;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 5. идем в заданную точку
	AIAPI::print(m_entity, "|P|thinkPursuit: идем в заданную точку");
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
	m_activity = CreateMoveActivity(path,ActivityFactory::CT_ENEMY_MOVE);
	m_activity->Attach(&m_activity_observer,
		ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

//=====================================================================================//
//                             bool PatrolEnemyNode::die()                             //
//=====================================================================================//
// умри!
bool PatrolEnemyNode::die()
{
	m_mode = PEM_KILLED;
	return need2Delete();
}

//=====================================================================================//
//                      bool PatrolEnemyNode::need2Delete() const                      //
//=====================================================================================//
// нужно ли удалять узел
bool PatrolEnemyNode::need2Delete() const
{
	if( (m_mode == PEM_KILLED) && !m_activity) return true;
	return false;
}

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
// ����������� - id ��������
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

    // ���� �������� - ����� ��������
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
		// �����������
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
		// ��������
		int tmp;
		st >> tmp; m_mode = static_cast<PatrolEnemyMode>(tmp);
		st >> tmp; m_turn_state = static_cast<TurnState>(tmp);
		st >> m_entity;
		st >> tmp; m_activity_observer.setLastEvent(static_cast<ActivityObserver::event_type>(tmp));

		st >> m_enemy_id >> m_enemy_pnt.x >> m_enemy_pnt.y >> m_cure_id
			>> m_cure_pnt.x >> m_cure_pnt.y >> m_shoot_pnt.x >> m_shoot_pnt.y
			>> m_shooted >> m_target_pnt.x >> m_target_pnt.y >> m_target_dir
			>> m_last_enemy_pnt.x >> m_last_enemy_pnt.y;
		
		// ��, ��� ����������������� ��� ������
		m_activity.Reset();
		m_initialising = false;
	}
}

//=====================================================================================//
//                       void PatrolEnemyNode::recieveMessage()                        //
//=====================================================================================//
// ��������� ��������������� ���������
void PatrolEnemyNode::recieveMessage(MessageType type, void * data)
{
	eid_t id = *static_cast<eid_t*>(data);
	switch(type)
	{
	case MT_ENEMYSPOTTED :
		{
			if(m_enemy_id)
			{
				// ��� �������� � �����, �������� ��� �� ������ �����
				float old_dist = dist(AIAPI::getPos2(m_entity),	m_enemy_pnt);
				float new_dist = dist(AIAPI::getPos2(m_entity),	AIAPI::getPos2(id));
				if(new_dist < old_dist)
				{  	// ������ �����
					m_enemy_id = id;
					m_enemy_pnt = AIAPI::getPos2(m_enemy_id);
				}
				return;
			}
			// ��� ������ ����, � ������� ��������
			m_enemy_id = id;
			m_enemy_pnt = AIAPI::getPos2(m_enemy_id);
			return;
		}
	case MT_NEEDCURE :
		{
			if(m_cure_id)
			{
				// ��� ������� � ������, �������� ��� �� �������� �����
				float old_dist = dist(AIAPI::getPos2(m_entity),	m_cure_pnt);
				float new_dist = dist(AIAPI::getPos2(m_entity),	AIAPI::getPos2(id));
				if(new_dist < old_dist)
				{	// ������ ��������� � ������
					m_cure_id = id;
					m_cure_pnt = AIAPI::getPos2(m_cure_id);
				}
				return;
			}
			// ��� ������ ��������� � ������
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
//��������� �������� �� ��������
void PatrolEnemyNode::OnKillEv(BaseEntity* killer, BaseEntity* victim)
{
	// �������� ������ �� �����
	if(victim == AIAPI::getPtr(m_enemy_id)) m_enemy_id = 0;
	if(victim == AIAPI::getPtr(m_cure_id)) m_cure_id = 0;
}

//=====================================================================================//
//                          void PatrolEnemyNode::OnShootEv()                          //
//=====================================================================================//
// ��������� ��������� �� �������� ��� ���������
void PatrolEnemyNode::OnShootEv(BaseEntity* who, BaseEntity* victim, const point3& where)
{
	if(!m_entity) return;

	// �� ����� ����������� � ������,
	// ���� ���� ��������� � ������������ ����������
	switch(m_mode)
	{
	case PEM_ATTACK: case PEM_WEAPONSEARCH: case PEM_WEAPONTAKE : return;
	default: break;
	}

	if(AIAPI::getPtr(m_entity) == who) return; // ��� � ��� ��������

	float dist = (getPosition() - where).Length();
	if( (dist < alarm_dist) || (AIAPI::getPtr(m_entity) == victim) )
	{
		// ����� �����������
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

	// ������� ������ ���� ���, ����� ����� ������� ����������
	// �� ���� ������� aim, ���� ����� ������ ��� - �������
	// ������ ����, � ������� �����.

	// ������� ����� ���-�� ���������� ����������
	int movpoints = AIAPI::getMovePoints(m_entity);
	int mps4shot = AIAPI::getShootMovePoints(m_entity, SHT_AIMSHOT);
	// ������ ��������� �� ������� � �����
	movpoints -= AIUtils::CalcMps4EntityRotate(PIm2);
	// ������ ��������� �� ����������
	movpoints -= PoL::Inv::MpsForPoseChange;

	// ������ ���������� ���������� ����������� ��� ��������
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
			// ���������� ���� ������������
			PathUtils::CalcPassField(AIAPI::getPtr(m_entity), 0, 0, 0);
			AIAPI::getBaseField(AIAPI::getPos2(m_entity), steps, &field);
			// ������� ������ ��������� ������
			AIAPI::getReachableField(field, &vec, true);
			// ������� ������ ����
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
			// ����� ����
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
// ��������� ������ Patrol
bool PatrolEnemyNode::thinkPatrol(state_type* st)
{
	// 1. �������� ���� ��������, ���� ��� < �������� - ������ ���������
	SendCureMessage(patrol_cure_dist);

	// 2. ���� � ��� ��� ������ - �������� � ��� ������
	if( (!AIAPI::takeGrenade(m_entity)) &&
		(!AIAPI::takeBestWeapon(m_entity, PatrolWeaponComparator()))	)
	{
		AIAPI::print(m_entity, "|P|thinkPatrol: ���� ������ �����!");
		m_mode = PEM_WEAPONSEARCH;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 3. ���� � ���� ����� - ����� ��������� � ������� � �����
	AIAPI::entity_list_t enemies;
	if(AIAPI::getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// � ����-�� ����
		if(eid_t enemy = AIAPI::getMostDangerousEnemy(enemies,EnemyComparator(m_entity))	)
		{
			AIAPI::print(m_entity, "|P|thinkPatrol: � ������ �����!");
			// ������ ������ �������� �����
			// ����� ��������� ����� �������
			getSubteam()->sendMessage(this,	AIEnemyNode::MT_ENEMYSPOTTED,	&enemy,	patrol_enemy_dist);
			// ������� � ��������� �����
			m_mode = PEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_last_enemy_pnt = AIAPI::getPos2(enemy);
			AIAPI::setRun(m_entity);

			switchToAttack(enemy);
			return false;

			//// ������� ������ ���� ���, ����� ����� ������� ����������
			//// �� ���� ������� aim, ���� ����� ������ ��� - �������
			//// ������ ����, � ������� �����.

			//// ������� ����� ���-�� ���������� ����������
			//int movpoints = AIAPI::getMovePoints(m_entity);
			//// ������ ���������� ���������� ����������� ��� ��������
			//movpoints -= AIAPI::getShootMovePoints(m_entity,	SHT_AIMSHOT);
			//// ������ ��������� �� ������� � �����
			//movpoints -= AIUtils::CalcMps4EntityRotate(PIm2);
			//int steps = AIAPI::getSteps(m_entity, movpoints);

			//if(!steps) // ����� ������� ��� ����� ��� ����, ������ ��������� �, �����
			//	return false;
			//// ���������� ���� ������������
			//pnt_vec_t field, vec, path;
			//PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, 0, 0);
			//AIAPI::getBaseField(AIAPI::getPos2(m_entity),	steps, &field);
			//// ������� ������ ��������� ������
			//AIAPI::getReachableField(field, &vec, true);
			//// ������� ������ ����
			//m_target_pnt = AIAPI::getBestPoint(vec, PatrolHexComparator(m_entity, enemy));
			//PathUtils::CalcPath(AIAPI::getPtr(m_entity),	m_target_pnt, path);
			//// ����� ����
			//m_activity = CreateMoveActivity(path,	ActivityFactory::CT_ENEMY_MOVE);
			//m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			//m_activity->Attach(&m_activity_observer,ActivityObserver::EV_ENEMY_REACTED);
			//return false;
		}
	}

	// 4. ���� � ���� �������� - ����� ������������
	if(m_shooted)
	{
		AIAPI::print(m_entity, "|P|thinkPatrol: � ���� �������� - ����� ������������");
		// � ���� ��� ����� �� ���� ��������
		m_shooted = false;
		// ������� ����� � ������� 3 ������ �� ��������, ������� ��������
		// ������ ���� � �������� ��� ���� � ��������� �������������

		// ���������� ���� ������������
		pnt_vec_t field, vec, path;
		PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, 0, 0);
		AIAPI::getBaseField(m_shoot_pnt,	3, &field);
		// ������� ������ ��������� ������
		AIAPI::getReachableField(field, &vec, true);
		// ������� ���� ��������� ����
		m_target_pnt = AIAPI::getRandomPoint(vec);
		PathUtils::CalcPath(AIAPI::getPtr(m_entity),	m_target_pnt, path);
		// ����� ����
		m_activity = CreateMoveActivity(path,ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer,	ActivityObserver::EV_MOVEPNTS_EXPIRIED);

		m_mode = PEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 5. ���� ��� �������� � ����� - ������ � ����
	if(m_enemy_id)
	{
		AIAPI::print(m_entity, "|P|thinkPatrol: ��� �������� � �����");
		// ��������
		m_enemy_id = 0;
		// ������� ����� � ������� 1 ����� �� �����
		// ������ ���� � �������� ��� ���� � ��������� �������������

		// ���������� ���� ������������
		PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, 0, 0);
		pnt_vec_t field;
		AIAPI::getBaseField(m_enemy_pnt,	1, &field);
		// ������� ������ ��������� ������
		pnt_vec_t vec;
		AIAPI::getReachableField(field, &vec, true);
		// ������� ���� ��������� ����
		m_target_pnt = AIAPI::getRandomPoint(vec);
		pnt_vec_t path;
		PathUtils::CalcPath(AIAPI::getPtr(m_entity),	m_target_pnt, path);
		// ����� ����
		m_activity = CreateMoveActivity(path,ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);

		m_mode = PEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 6. ���� ���� ���� ������ - ��������� � ��������� �������
	if(m_cure_id)
	{
		// �������� �������� �����
		if( (AIAPI::getHealthPercent(m_cure_id) < 50.0f) &&
			(AIAPI::haveMedikit(m_entity))	)
		{
			AIAPI::print(m_entity, "|P|thinkPatrol: ����� ������");
			// ��� ����� ������
			m_mode = PEM_CURE;
			m_turn_state = TS_INPROGRESS;
			return false;
		}
		// ��� �� ����� ������
		m_cure_id = 0;
	}

	// 7. ���� �������� ������ ��������� - ��������� ���� ��������
	SelfCure();

	// 8. ���� ������� �� ������ ������, ������� ��� ������� - 
	// ������� � ��������� ������ ����
	ipnt2_t pos;
	if(AIAPI::getThingLocation(m_entity,	&pos,TT_WEAPON|TT_AMMO|TT_MEDIKIT)	)
	{
		AIAPI::print(m_entity, "|P|thinkPatrol: ������ �� ������ ������, ������� ��� �������");
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
			// ����� ����
			m_target_pnt = pos;
			m_activity = CreateMoveActivity(path,ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_turn_state = TS_INPROGRESS;
			m_mode = PEM_THINGTAKE;
			return false;
		}
	}

	// 9. ���� ����� �� �������� ����� - �������� �����
	if(m_target_pnt == AIAPI::getPos2(m_entity))
	{
		AIAPI::print(m_entity, "|P|thinkPatrol: ����� �� �������� ����� - �������� �����");
		// ���������� ���� ������������
		pnt_vec_t allhexes;
		PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, &allhexes, 0);
		if(allhexes.empty())
		{
			// �������� ��� - ���� ������
			AIAPI::print(m_entity, "|P|thinkPatrol: ��� ������ ������!!!");
			m_turn_state = TS_END;
			return false;
		}
		// ������� ��������� �����
		m_target_pnt = AIAPI::getRandomPoint(allhexes);
	}

	// 10. �������� �� ��������� �� � ��� ���������
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// �� ������� ����������
		AIAPI::print(m_entity, "|P|thinkPatrol: �� ������� ����������");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 10. ���� � �������� �����
	AIAPI::print(m_entity, "|P|thinkPatrol: ���� � �������� �����");
	PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, 0, 0);
	pnt_vec_t path;
	PathUtils::CalcPath(AIAPI::getPtr(m_entity),	m_target_pnt, path);
	if(path.empty())
	{
		// ����� ��������� ������������ - ����� �������, ��� ��� ������
		// ���� ����
		m_target_pnt = AIAPI::getPos2(m_entity);
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	// ����� ����
	m_activity = CreateMoveActivity(path,ActivityFactory::CT_ENEMY_MOVE);
	m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

//=====================================================================================//
//                         bool PatrolEnemyNode::thinkAttack()                         //
//=====================================================================================//
// ��������� ������ Attack
bool PatrolEnemyNode::thinkAttack(state_type* st)
{
	// 1. �������� ���� ��������, ���� ��� < �������� - ������ ���������
	SendCureMessage(patrol_cure_dist);
	// 2. ���� � ��� ��� ������ - �������� � ��� ������
	if( (!AIAPI::takeGrenade(m_entity)) &&
		(!AIAPI::takeBestWeapon(m_entity, PatrolWeaponComparator()))	)
	{
		AIAPI::print(m_entity, "|P|thinkAttack: ���� ������ �����!");
		m_mode = PEM_WEAPONSEARCH;
		m_turn_state = TS_INPROGRESS;
		m_activity_observer.clear();
		AIAPI::setWalk(m_entity);
		return false;
	}

	// 3. ������� �����
	AIAPI::entity_list_t enemies;
	eid_t enemy = 0;
	if(AIAPI::getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// ��� ����� �����, ���� ��� ������ ���� ��� ������� - ������� ������ �������������
		if(m_turn_state == TS_START)	m_ignored_enemies.clear();

		// ��������� ����� �������� ������� ���� ������ � �������������� ������
		EntityList_t diff_list;
		setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);

		if(diff_list.empty())// ������� ������ �������� ����� �� ����
			enemy = AIAPI::getMostDangerousEnemy(enemies,EnemyComparator(m_entity));
		else// ������� ������ �������� ����� �� �������������������
			enemy = AIAPI::getMostDangerousEnemy(diff_list,	EnemyComparator(m_entity));

		m_last_enemy_pnt = AIAPI::getPos2(enemy);
	}
	
	// 4. ���� ��� ������ - ��������� � ��������� �������������
	if(!enemy)
	{
		// ������ ��� - ����� ������������ ����������
		AIAPI::print(m_entity, "|P|thinkAttack: ������ ��� - ����� ������������ ����������");
		m_mode = PEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		m_target_pnt = m_last_enemy_pnt;
		m_activity_observer.clear();
		AIAPI::setWalk(m_entity);
		return false;
	}
	// ����� ����...

	// 5. ���� ��� ������ ������ ����
	if(m_turn_state == TS_START)
	{
		// �������� ������ ����� � ���� � ���
		AIAPI::print(m_entity, "|P|thinkAttack: �������� ������ ����� � ���� � ���");

		// ������� ������ ���� ���, ����� ����� ������� ����������
		// �� ���� ������� aim, ���� ����� ������ ��� - �������
		// ������ ����, � ������� �����.
		
		m_turn_state = TS_INPROGRESS;
		AIAPI::setRun(m_entity);

		if(switchToAttack(enemy))
		{
			return false;
		}
	}

	// 6. ��� ����������� ����
	// 6.1 ���� ����� �� ������� ���������� - �� �������� ���
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// ����������� - �������� ���
		AIAPI::print(m_entity, "|P|thinkAttack: ����������� ���������");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 6.2 ���� ��������� ������� ��� ������ ������ ����� - ���������
	//       ������ ����
	if( (m_activity_observer.getLastEvent() == ActivityObserver::EV_ENEMY_REACTED) ||
		(m_activity_observer.getLastEvent() == ActivityObserver::EV_ENEMY_SPOTTED)	)
	{
		AIAPI::print(m_entity, "|P|thinkAttack: ��������� ������� ��� ������ ������ �����");
		m_activity_observer.clear();
		// ��������� ������ ����
		m_ignored_enemies.clear();
		m_turn_state = TS_START;
		return true;
	}

	// 6.3 ���� ��� ����� ���� - ��������� ����� � ������ �������������
	// ������, ���� ����� ����� ��� ����� ��������� � ������ �������������
	// ��������� ������� � ������ �� �������������� ������
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_NO_LOF)
	{
		// ����������� - �������� ���
		AIAPI::print(m_entity, "|P|thinkAttack: ��� ����� ���� - ��������� ����� � ������ �������������");
		m_activity_observer.clear();
		// ������� � ������ �������������
		m_ignored_enemies.insert(enemy);
		EntityList_t diff_list;
		setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);
		if(diff_list.empty())
		{
			// ��������� ������� ������� � �����
			AIAPI::print(m_entity, "|P|thinkAttack: �������� �������");
			// ���������� ���� ������������
			PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, 0, PathUtils::F_CALC_NEAR_PNTS);
			EntityList_t::iterator i = enemies.begin();
			// ������ ��������, � �������� ����� �������
			bool found = false;
			while(i != enemies.end())
			{
				// ��������, ���������� �� ����� ����� � ���������
				if(PathUtils::IsNear(AIAPI::getPtr(*i)))
				{
					// � ����� ����� �������
					found = true;
					m_target_pnt = PathUtils::GetNearPnt(AIAPI::getPtr(*i)).m_pnt;
					break;
				}
				++i;
			}
			pnt_vec_t path;
			if(!found)
			{
				// ����������� ����� ��� - ������������ ��������� �������
				pnt_vec_t base, vec;
				AIAPI::getBaseField(AIAPI::getPos2(m_entity),	patrol_enemy_noLOF_move,&base);
				AIAPI::getReachableField(base, &vec, true);
				m_target_pnt = AIAPI::getRandomPoint(vec);
				PathUtils::CalcPath(AIAPI::getPtr(m_entity),	m_target_pnt, path);
			}
			else
			{
				// ���� ����, � �������� ����� ������� - �������� �� �������� ����
				PathUtils::CalcPath(AIAPI::getPtr(m_entity),	m_target_pnt, path);
				// ������� �������� ����
				path.erase(&path[0], &path[path.size() / 2]);
			}
			if(path.empty())
			{
				// ��������� ���
				m_turn_state = TS_END;
				return false;
			}
			// ����� ����
			AIAPI::setWalk(m_entity);
			m_activity = CreateMoveActivity(path,ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_REACTED);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_SPOTTED);
			
			m_turn_state = TS_INPROGRESS;
			return false;
		} 
		// ���������� ��� ������ ������ ������
		m_turn_state = TS_INPROGRESS;
		return true;
	}

	// 6.4 ���� �� �������� � ����� - �������������� � ����������� ��� ��� ����
	m_target_dir = norm_angle(Dir2Angle(AIAPI::getPos3(enemy) - getPosition()));
	float cur_angle = norm_angle(AIAPI::getAngle(m_entity));
	if( fabs(m_target_dir - cur_angle) >= angle_eps )
	{
		// ����� �������������� � �����
		m_activity = CreateRotateActivity(m_target_dir,ActivityFactory::CT_ENEMY_ROTATE);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	
	// 6.5 �������� � ����� - ���� ������ ����� ���������
	AIAPI::print(m_entity, "|P|thinkAttack: ����� ��������, ���� ��� ��������!");
	// ���� ���� - ��������� �� ���� ���� ��� ��������

	// ������� ��������� �������� (���� ���� � ���� ���������)
	if(AIAPI::takeGrenade(m_entity))
	{
		// ������� � ����� - ��������� ������ ��
		std::string reason;
		if(AIAPI::shoot(m_entity, enemy, &m_activity, &reason))
		{
			// ������� ������ ������ �������
			m_turn_state = TS_INPROGRESS;
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_NO_LOF);
			return false;
		}
		// ������ ������ ������� �� �������
		std::string str = "|P|thinkAttack: ������ ������ �������: ";
		str += reason;
		AIAPI::print(m_entity, str.c_str());
	}

	// ������� ��� ���, ��� ������� �� �� �������
	// ������ ��������� ���������� �� ������
	if(!AIAPI::takeBestWeapon(m_entity, PatrolWeaponComparator()))
	{
		m_turn_state = TS_END;
		return false;
	}
	// ������ � �����
	AIAPI::setShootType(m_entity, 5.0f, 10.0f);
	std::string reason;
	if(AIAPI::shoot(m_entity, enemy, &m_activity, &reason))
	{
		// ������� ������ ��������
		m_turn_state = TS_INPROGRESS;
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_NO_LOF);
	}
	else
	{
		if(AIUtils::CalcEfficiency(AIAPI::getPtr(m_entity),AIAPI::getPos3(enemy)) == 0)
		{
			// ������� ������������� -- ��������� ������ ����, ���� �������� ������ 10 ����������
			if(AIAPI::getSteps(m_entity,AIAPI::getMovePoints(m_entity)) > 2)
			{
				m_turn_state = TS_START;
				return false;
			}
		}
		std::string str = "|P|thinkAttack: �������� ������: ";
		str += reason;
		AIAPI::print(m_entity, str.c_str());
		m_turn_state = TS_END;
	}
	return false;
}

//=====================================================================================//
//                      bool PatrolEnemyNode::thinkWeaponSearch()                      //
//=====================================================================================//
// ��������� ������ WeaponSearch
bool PatrolEnemyNode::thinkWeaponSearch(state_type* st)
{
	// 1. �������� ���� ��������, ���� ��� < �������� - ������ ���������
	SendCureMessage(patrol_cure_dist);

	// 2. �������� ���� �� ���-���� ����� ������
	if(AIAPI::getWeaponLocation(m_entity, &m_target_pnt))
	{
		// �������� ���� ������ - ������� � ��� �������
		AIAPI::print(m_entity, "|P|thinkWeaponSearch: �������� ���� ������ - ������� � ��� �������!");
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
			// � ������ ������ ������� - ������� ���
			m_turn_state = TS_END;
			return false;
		}
		// ����� ����
		m_activity = CreateMoveActivity(path,ActivityFactory::CT_ENEMY_MOVEIGNORE);
		m_activity->Attach(&m_activity_observer,	ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		m_mode = PEM_WEAPONTAKE;
		return false;
	}

	// 3. ���� �������� ������ ��������� - ��������� ���� ��������
	SelfCure();

	// 4. ���� ����� �� �������� ����� - �������� �����
	if(m_target_pnt == AIAPI::getPos2(m_entity))
	{
		AIAPI::print(m_entity, "|P|thinkWeaponSearch: ����� �� �������� ����� - �������� �����");
		// ���������� ���� ������������
		pnt_vec_t allhexes;
		PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, &allhexes, 0);
		if(allhexes.empty())
		{
			// �������� ��� - ���� ������
			AIAPI::print(m_entity, "|P|thinkWeaponSearch: ��� ������ ������!!!");
			m_turn_state = TS_END;
			return false;
		}
		// ������� ��������� �����
		m_target_pnt = AIAPI::getRandomPoint(allhexes);
	}

	// 5. �������� �� ��������� �� � ��� ���������
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// �� ������� ����������
		AIAPI::print(m_entity, "|P|thinkWeaponSearch: �� ������� ����������");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 6. ���� � �������� �����
	AIAPI::print(m_entity, "|P|thinkWeaponSearch: ���� � �������� �����");
	PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, 0, 0);
	pnt_vec_t path;
	PathUtils::CalcPath(AIAPI::getPtr(m_entity), m_target_pnt, path);
	if(path.empty())
	{
		// ����� ��������� ������������ - ����� �������, ��� ��� ������
		// ���� ����
		m_target_pnt = AIAPI::getPos2(m_entity);
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	// ����� ����
	m_activity = CreateMoveActivity(path,ActivityFactory::CT_ENEMY_MOVE);
	m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

//=====================================================================================//
//                       bool PatrolEnemyNode::thinkThingTake()                        //
//=====================================================================================//
// ��������� ������ ThingTake
bool PatrolEnemyNode::thinkThingTake(state_type* st)
{
	// 1. �������� ���� ��������, ���� ��� < �������� - ������ ���������
	SendCureMessage(patrol_cure_dist);

	// 2. ���� � ���� ����� - ����� ��������� � ������� � �����
	AIAPI::entity_list_t enemies;
	if(AIAPI::getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// � ����-�� ����
		if(eid_t enemy = AIAPI::getMostDangerousEnemy(enemies, EnemyComparator(m_entity)) )
		{
			AIAPI::print(m_entity, "|P|thinkThingTake: � ������ �����!");
			// ������ ������ �������� �����
			// ����� ��������� ����� �������
			getSubteam()->sendMessage(this, AIEnemyNode::MT_ENEMYSPOTTED, &enemy, patrol_enemy_dist);
			// ������� � ��������� �����
			m_mode = PEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_last_enemy_pnt = AIAPI::getPos2(enemy);
			AIAPI::setRun(m_entity);

			switchToAttack(enemy);
			return false;
		}
	}

	// 3. �������� �� ������� �� �� � ����
	if(m_target_pnt == AIAPI::getPos2(m_entity))
	{
		// �� ������� � ���������� ����� - ������� ��� ������
		AIAPI::print(m_entity, "|P|thinkThingTake: �� ������� � ���������� ����� - ������� ��� ������");
		AIAPI::pickupAllNearAmmo(m_entity);
		AIAPI::pickupAllNearMedikit(m_entity);
		AIAPI::pickupAllNearWeapon(m_entity);
		AIAPI::dropUselessWeapon(m_entity, PatrolWeaponComparator());
		// ��������� � ��������� ��������������
		m_mode = PEM_PATROL;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 4. ���� � ���� �������� - ����� ������������
	if(m_shooted)
	{
		AIAPI::print(m_entity, "|P|thinkThingTake: � ���� �������� - ����� ������������");
		// � ���� ��� ����� �� ���� ��������
		m_shooted = false;
		// ������� ����� � ������� 3 ������ �� ��������, ������� ��������
		// ������ ���� � �������� ��� ���� � ��������� �������������

		// ���������� ���� ������������
		PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, 0, 0);
		pnt_vec_t field;
		AIAPI::getBaseField(m_shoot_pnt,	3, &field);
		// ������� ������ ��������� ������
		pnt_vec_t vec;
		AIAPI::getReachableField(field, &vec, true);
		// ������� ���� ��������� ����
		m_target_pnt = AIAPI::getRandomPoint(vec);
		pnt_vec_t path;
		PathUtils::CalcPath(AIAPI::getPtr(m_entity),	m_target_pnt, path);
		// ����� ����
		m_activity = CreateMoveActivity(path,ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);

		m_mode = PEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 5. ���� ��� �������� � ����� - ������ � ����
	if(m_enemy_id)
	{
		AIAPI::print(m_entity, "|P|thinkThingTake: ��� �������� � �����");
		// ��������
		m_enemy_id = 0;
		// ������� ����� � ������� 1 ����� �� �����
		// ������ ���� � �������� ��� ���� � ��������� �������������

		// ���������� ���� ������������
		PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, 0, 0);
		pnt_vec_t field;
		AIAPI::getBaseField(m_enemy_pnt,	1, &field);
		// ������� ������ ��������� ������
		pnt_vec_t vec;
		AIAPI::getReachableField(field, &vec, true);
		// ������� ���� ��������� ����
		m_target_pnt = AIAPI::getRandomPoint(vec);
		pnt_vec_t path;
		PathUtils::CalcPath(AIAPI::getPtr(m_entity),	m_target_pnt, path);
		// ����� ����
		m_activity = CreateMoveActivity(path,ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);

		m_mode = PEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 6. ���� ���� ���� ������ - ��������� � ��������� �������
	if(m_cure_id)
	{
		// �������� �������� �����
		if( (AIAPI::getHealthPercent(m_cure_id) < 50.0f) &&	(AIAPI::haveMedikit(m_entity)))
		{
			AIAPI::print(m_entity, "|P|thinkThingTake: ����� ������");
			// ��� ����� ������
			m_mode = PEM_CURE;
			m_turn_state = TS_INPROGRESS;
			return false;
		}
		// ��� �� ����� ������
		m_cure_id = 0;
	}
	
	// 7. ���� �������� ������ ��������� - ��������� ���� ��������
	SelfCure();

	// 8. �������� �� ��������� �� � ��� ���������
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// �� ������� ����������
		AIAPI::print(m_entity, "|P|thinkThingTake: �� ������� ����������");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 9. �������� ���� �� ���-���� ����� ������
	if(AIAPI::getThingLocation(m_entity, &m_target_pnt,	TT_AMMO|TT_WEAPON|TT_MEDIKIT)	)
	{
		// �������� ���� ������ - ������ � ����
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
			// � ������ ������ ������� - ������� � ��������������
			m_mode = PEM_PATROL;
			m_turn_state = TS_INPROGRESS;
			return false;
		}
		// ����� ����
		m_activity = CreateMoveActivity(path,ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// ������ ��� - ������� � ��������������
	m_mode = PEM_PATROL;
	m_turn_state = TS_INPROGRESS;
	return false;
}

//=====================================================================================//
//                       bool PatrolEnemyNode::thinkWeaponTake()                       //
//=====================================================================================//
// ��������� ������ WeaponTake
bool PatrolEnemyNode::thinkWeaponTake(state_type* st)
{
	SendCureMessage(patrol_cure_dist);

	// 2. �������� �� ������� �� �� � ������
	if(PickUpWeaponAndAmmo(m_target_pnt, PatrolWeaponComparator()))
	{
		m_mode = PEM_PATROL;
		return false;
	}

	// 3. �������� �� ��������� �� � ��� ���������
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// �� ������� ����������
		AIAPI::print(m_entity, "|P|thinkWeaponTake: �� ������� ����������");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 4. �������� ���� �� ���-���� ����� ������
	if(AIAPI::getWeaponLocation(m_entity, &m_target_pnt))
	{
		// �������� ���� ������ - ������ � ����
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
			// � ������ ������ ������� - ������� � ��������������
			m_mode = PEM_PATROL;
			m_turn_state = TS_INPROGRESS;
			return false;
		}
		// ����� ����
		m_activity = CreateMoveActivity(path, ActivityFactory::CT_ENEMY_MOVEIGNORE);
		m_activity->Attach(&m_activity_observer,
			ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// ������ ��� - ������� � ��������������
	m_mode = PEM_PATROL;
	m_turn_state = TS_INPROGRESS;
	return false;
}

//=====================================================================================//
//                          bool PatrolEnemyNode::thinkCure()                          //
//=====================================================================================//
// ��������� ������ Cure
bool PatrolEnemyNode::thinkCure(state_type* st)
{
	// 1. �������� ���� ��������, ���� ��� < �������� - ������ ���������
	SendCureMessage(patrol_cure_dist);

	// 2. ���� � ���� ����� - ����� ��������� � ������� � �����
	AIAPI::entity_list_t enemies;
	if(AIAPI::getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// � ����-�� ����
		if(eid_t enemy = AIAPI::getMostDangerousEnemy(enemies,
			EnemyComparator(m_entity))
			)
		{
			AIAPI::print(m_entity, "|P|thinkCure: � ������ �����!");
			// ������ ������ �������� �����
			// ����� ��������� ����� �������
			getSubteam()->sendMessage(this,
				AIEnemyNode::MT_ENEMYSPOTTED,
				&enemy,
				patrol_enemy_dist);
			// ������� � ��������� �����
			m_mode = PEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_last_enemy_pnt = AIAPI::getPos2(enemy);
			AIAPI::setRun(m_entity);

			switchToAttack(enemy);
			return false;
		}
	}

	// 3. ���� � ���� �������� - ����� ������������
	if(m_shooted)
	{
		AIAPI::print(m_entity, "|P|thinkCure: � ���� �������� - ����� ������������");
		// � ���� ��� ����� �� ���� ��������
		m_shooted = false;
		// ������� ����� � ������� 3 ������ �� ��������, ������� ��������
		// ������ ���� � �������� ��� ���� � ��������� �������������

		// ���������� ���� ������������
		PathUtils::CalcPassField(AIAPI::getPtr(m_entity),
			0, 0, 0);
		pnt_vec_t field;
		AIAPI::getBaseField(m_shoot_pnt,
			3, &field);
		// ������� ������ ��������� ������
		pnt_vec_t vec;
		AIAPI::getReachableField(field, &vec, true);
		// ������� ���� ��������� ����
		m_target_pnt = AIAPI::getRandomPoint(vec);
		pnt_vec_t path;
		PathUtils::CalcPath(AIAPI::getPtr(m_entity),
			m_target_pnt, path);
		// ����� ����
		AIAPI::setWalk(m_entity);
		m_activity = CreateMoveActivity(path, ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer,
			ActivityObserver::EV_MOVEPNTS_EXPIRIED);

		m_mode = PEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 4. ���� ��� �������� � ����� - ������ � ����
	if(m_enemy_id)
	{
		AIAPI::print(m_entity, "|P|thinkCure: ��� �������� � �����");
		// ��������
		m_enemy_id = 0;
		// ������� ����� � ������� 1 ����� �� �����
		// ������ ���� � �������� ��� ���� � ��������� �������������

		// ���������� ���� ������������
		PathUtils::CalcPassField(AIAPI::getPtr(m_entity),
			0, 0, 0);
		pnt_vec_t field;
		AIAPI::getBaseField(m_enemy_pnt,
			1, &field);
		// ������� ������ ��������� ������
		pnt_vec_t vec;
		AIAPI::getReachableField(field, &vec, true);
		// ������� ���� ��������� ����
		m_target_pnt = AIAPI::getRandomPoint(vec);
		pnt_vec_t path;
		PathUtils::CalcPath(AIAPI::getPtr(m_entity),
			m_target_pnt, path);
		// ����� ����
		AIAPI::setWalk(m_entity);
		m_activity = CreateMoveActivity(path, ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer,
			ActivityObserver::EV_MOVEPNTS_EXPIRIED);

		m_mode = PEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 5. ���� �������� ������ ��������� - ��������� ���� ��������
	SelfCure();

	// 6. ���� ������ ��� ����� ������
	if( (!m_cure_id) ||
		(AIAPI::getHealthPercent(m_cure_id) >= 50.0f) ||
		(!AIAPI::haveMedikit(m_entity))
		)
	{
		AIAPI::print(m_entity, "|P|thinkCure: ������ ��� ����� ������");
		// ������ ��� ����� ������ - ������� � ��������������
		m_cure_id = 0;
		m_mode = PEM_PATROL;
		m_turn_state = TS_INPROGRESS;
		AIAPI::setWalk(m_entity);
		return false;
	}

	// 7. ���� ������� � ����� - ����� ���
	if(dist(AIAPI::getPos2(m_entity),
		AIAPI::getPos2(m_cure_id)) <= 1.5f)
	{
		// ������� - �����
		AIAPI::print(m_entity, "|P|thinkCure: �����");
		// ������� � ���� �������
		AIAPI::takeMedikit(m_entity);
		// ������� �����
		AIAPI::cure(m_entity, m_cure_id);
		// ������� � ���� ������
		if(!AIAPI::takeGrenade(m_entity))
		{
			AIAPI::takeBestWeapon(m_entity, PatrolWeaponComparator());
		}
		// ���������
		m_turn_state = TS_INPROGRESS;
		return true;
	}
	
	// 8. �������� �� ��������� �� � ��� ���������
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// �� ������� ����������
		AIAPI::print(m_entity, "|P|thinkCure: �� ������� ����������");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 9. ���� � �����, �������� ����� ������
	
	// ���������� ���� ������������
	PathUtils::CalcPassField(AIAPI::getPtr(m_entity),
		0, 0, PathUtils::F_CALC_NEAR_PNTS);
	// ��������, ���������� �� ����� ����� � ���������
	if(!PathUtils::IsNear(AIAPI::getPtr(m_cure_id)))
	{
		// � ����� ������ �������
		m_cure_id = 0;
		m_mode = PEM_PATROL;
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	m_target_pnt = PathUtils::GetNearPnt(AIAPI::getPtr(m_cure_id)).m_pnt;
	pnt_vec_t path;
	PathUtils::CalcPath(AIAPI::getPtr(m_entity),
		m_target_pnt, path);
	// ����� ����
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
// ��������� ������ Pursuit
bool PatrolEnemyNode::thinkPursuit(state_type* st)
{
	// 1. �������� ���� ��������, ���� ��� < �������� - ������ ���������
	SendCureMessage(patrol_cure_dist);

	// 2. ���� � ���� ����� - ����� ��������� � ������� � �����
	AIAPI::entity_list_t enemies;
	if(AIAPI::getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// � ����-�� ����
		if(eid_t enemy = AIAPI::getMostDangerousEnemy(enemies,
			EnemyComparator(m_entity))
			)
		{
			AIAPI::print(m_entity, "|P|thinkPursuit: � ������ �����!");
			// ������ ������ �������� �����
			// ����� ��������� ����� �������
			getSubteam()->sendMessage(this,
				AIEnemyNode::MT_ENEMYSPOTTED,
				&enemy,
				patrol_enemy_dist);
			// ������� � ��������� �����
			m_mode = PEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_last_enemy_pnt = AIAPI::getPos2(enemy);
			AIAPI::setRun(m_entity);

			switchToAttack(enemy);
			return false;
		}
	}

	// 3. �������� �� ��������� �� � ��� ���������
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// �� ������� ����������
		AIAPI::print(m_entity, "|P|thinkPursuit: �� ������� ����������");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 4. �������� �� ������ �� �� � �������� �����
	if(m_target_pnt == AIAPI::getPos2(m_entity))
	{
		// �� ������
		// 4.1 ��������, �������� �� � �����, ��� ��� ����
		m_target_dir = norm_angle(Dir2Angle(AIAPI::convertPos2ToPos3(m_last_enemy_pnt) - getPosition()));
		float cur_angle = norm_angle(AIAPI::getAngle(m_entity));
		if( fabs(m_target_dir - cur_angle) >= angle_eps )
		{
			// ����� �������������� � �����
			m_activity = CreateRotateActivity(m_target_dir, ActivityFactory::CT_ENEMY_ROTATE);
			m_activity->Attach(&m_activity_observer,
				ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_turn_state = TS_INPROGRESS;
			return false;
		}
		// 4.2 �������� - ������� � ��������� ��������������
		AIAPI::print(m_entity, "|P|thinkPursuit: ������ �� ������ - ����� �������������");
		// ��������� � ��������� ��������������
		m_mode = PEM_PATROL;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 5. ���� � �������� �����
	AIAPI::print(m_entity, "|P|thinkPursuit: ���� � �������� �����");
	PathUtils::CalcPassField(AIAPI::getPtr(m_entity),
		0, 0, 0);
	pnt_vec_t path;
	PathUtils::CalcPath(AIAPI::getPtr(m_entity),
		m_target_pnt, path);
	if(path.empty())
	{
		// ����� ��������� ������������ - ����� �������, ��� ��� ������
		// ���� ����
		m_target_pnt = AIAPI::getPos2(m_entity);
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	// ����� ����
	m_activity = CreateMoveActivity(path,ActivityFactory::CT_ENEMY_MOVE);
	m_activity->Attach(&m_activity_observer,
		ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

//=====================================================================================//
//                             bool PatrolEnemyNode::die()                             //
//=====================================================================================//
// ����!
bool PatrolEnemyNode::die()
{
	m_mode = PEM_KILLED;
	return need2Delete();
}

//=====================================================================================//
//                      bool PatrolEnemyNode::need2Delete() const                      //
//=====================================================================================//
// ����� �� ������� ����
bool PatrolEnemyNode::need2Delete() const
{
	if( (m_mode == PEM_KILLED) && !m_activity) return true;
	return false;
}

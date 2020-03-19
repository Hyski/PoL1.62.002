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
// ����������� - id ��������
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
    //��������� ��������
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

	// ���� �� ��� ��� - �� �����
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

	// ���� m_turn_state == TS_NONE, �� ��� ������ ������ ����
	if(m_turn_state == TS_NONE)
	{
		m_turn_state = TS_START;
		// ������ �������������
		PanicPlayer::Init(AIAPI::getPtr(m_entity));
		m_initialising = true;
	}

	// �������� ���� �������������
	if(m_initialising)
	{
		if(PanicPlayer::Execute())
		{
			// ����� ���������� ������� �������������
			*st = ST_INCOMPLETE;
			return complexity;
		}
		// ������������� ���������
		m_initialising = false;
	}

    // ���� �������� - ����� ��������
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
		// �����������
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
		// ��������
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
		
		// ��, ��� ����������������� ��� ������
		m_activity = 0;
		m_initialising = false;
	}
}

//=====================================================================================//
//                     void PatrolTechEnemyNode::recieveMessage()                      //
//=====================================================================================//
// ��������� ��������������� ���������
void PatrolTechEnemyNode::recieveMessage(MessageType type, void * data)
{
	eid_t id = *static_cast<eid_t*>(data);
	if(type == MT_ENEMYSPOTTED)
	{
		if(m_enemy_id)
		{
			// ��� �������� � �����
			// �������� ��� �� ������ �����
			float old_dist = dist(AIAPI::getPos2(m_entity),
				m_enemy_pnt);
			float new_dist = dist(AIAPI::getPos2(m_entity),
				AIAPI::getPos2(id));
			if(new_dist < old_dist)
			{
				// ������ �����
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
}

//=====================================================================================//
//                   point3 PatrolTechEnemyNode::getPosition() const                   //
//=====================================================================================//
// ������ ����������� �������
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
// ��������� ��������� �� �������� ��� ���������
void PatrolTechEnemyNode::OnShootEv(BaseEntity* who, BaseEntity* victim, const point3& where)
{
	if(!m_entity) return;

	// �� ����� ����������� � ������,
	// ���� ������� ��������� � ������������ ����������
	if(m_mode == PTEM_ATTACK) return;

	if(AIAPI::getPtr(m_entity) == who) return; // ��� � ��� ��������

	float dist = (getPosition() - where).Length();
	if( (dist < alarm_dist) || (AIAPI::getPtr(m_entity) == victim))
	{
		// ����� �����������
		m_shoot_pnt = AIAPI::getPos2(who);
		m_shooted = true;
	}
}

//=====================================================================================//
//                       bool PatrolTechEnemyNode::thinkPatrol()                       //
//=====================================================================================//
// ��������� ������ Patrol
bool PatrolTechEnemyNode::thinkPatrol(state_type* st)
{
	// 1. ���� � ���� ����� - ����� ��������� � ������� � �����
	AIAPI::entity_list_t enemies;
	if(AIAPI::getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// � ����-�� ����
		if(eid_t enemy = AIAPI::getMostDangerousEnemy(enemies,
			EnemyComparator(m_entity))
			)
		{
			AIAPI::print(m_entity, "|PT|thinkPatrol: � ������� �����!");
			// ������a ������ �������� �����
			// ����� ��������� ����� �������
			getSubteam()->sendMessage(this,
				AIEnemyNode::MT_ENEMYSPOTTED,
				&enemy,
				patrol_tech_enemy_dist);
			// ������� � ��������� �����
			m_mode = PTEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_last_enemy_pnt = AIAPI::getPos2(enemy);

			// ������� ������ ���� ���, ����� ����� ������� ����������
			// �� ���� �������, ���� ����� ������ ��� - �������
			// ������ ����, � ������� �����.

			// ������� ����� ���-�� ���������� ����������
			int movpoints = AIAPI::getMovePoints(m_entity);
			// ������ ���������� ���������� ����������� ��� ��������
			movpoints -= AIAPI::getShootMovePoints(m_entity,
				/*�� ���� �������� ������*/SHT_AIMSHOT);
			// ������ ��������� �� ������� � �����
			movpoints -= AIUtils::CalcMps4EntityRotate(PIm2);
			int steps = AIAPI::getSteps(m_entity, movpoints);
			if(!steps)
			{
				// ����� ������� ��� ����� ��� ����, ������ ��������� �
				// �����
				return false;
			}
			// ���������� ���� ������������
			PathUtils::CalcPassField(AIAPI::getPtr(m_entity),
				0, 0, 0);
			pnt_vec_t field;
			AIAPI::getBaseField(AIAPI::getPos2(m_entity),
				steps, &field);
			// ������� ������ ��������� ������
			pnt_vec_t vec;
			AIAPI::getReachableField(field, &vec);
			// ������� ������ ����
			m_target_pnt = AIAPI::getBestPoint(vec,
				WalkingTechHexComparator(m_entity, enemy));
			pnt_vec_t path;
			PathUtils::CalcPath(AIAPI::getPtr(m_entity),
				m_target_pnt, path);
			// ����� ����
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

	// 2. ���� � ���� �������� - ����� ������������
	if(m_shooted)
	{
		AIAPI::print(m_entity, "|PT|thinkPatrol: � ���� �������� - ����� ������������");
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
		AIAPI::getReachableField(field, &vec);
		// ������� ���� ��������� ����
		m_target_pnt = AIAPI::getRandomPoint(vec);
		pnt_vec_t path;
		PathUtils::CalcPath(AIAPI::getPtr(m_entity),
			m_target_pnt, path);
		// ����� ����
		m_activity = ActivityFactory::CreateMove(AIAPI::getPtr(m_entity),
			path,
			ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer,
			ActivityObserver::EV_MOVEPNTS_EXPIRIED);

		m_mode = PTEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 3. ���� ��� �������� � ����� - ������ � ����
	if(m_enemy_id)
	{
		AIAPI::print(m_entity, "|PT|thinkPatrol: ��� �������� � �����");
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
		AIAPI::getReachableField(field, &vec);
		// ������� ���� ��������� ����
		m_target_pnt = AIAPI::getRandomPoint(vec);
		pnt_vec_t path;
		PathUtils::CalcPath(AIAPI::getPtr(m_entity),
			m_target_pnt, path);
		// ����� ����
		m_activity = ActivityFactory::CreateMove(AIAPI::getPtr(m_entity),
			path,
			ActivityFactory::CT_ENEMY_MOVE);
		m_activity->Attach(&m_activity_observer,
			ActivityObserver::EV_MOVEPNTS_EXPIRIED);

		m_mode = PTEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 4. ���� ����� �� �������� ����� - �������� �����
	if(m_target_pnt == AIAPI::getPos2(m_entity))
	{
		AIAPI::print(m_entity, "|PT|thinkPatrol: ����� �� �������� ����� - �������� �����");
		// ���������� ���� ������������
		pnt_vec_t allhexes;
		PathUtils::CalcPassField(AIAPI::getPtr(m_entity),
			0, &allhexes, 0);
		if(allhexes.empty())
		{
			// �������� ��� - ���� ������
			AIAPI::print(m_entity, "|PT|thinkPatrol: ��� ������ ������!!!");
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
		AIAPI::print(m_entity, "|PT|thinkPatrol: �� ������� ����������");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 6. ���� � �������� �����
	AIAPI::print(m_entity, "|PT|thinkPatrol: ���� � �������� �����");
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
// ��������� ������ Attack
bool PatrolTechEnemyNode::thinkAttack(state_type* st)
{
	// 1. ���� � ������� ��� ������ - �������� � �������������
	if(!AIAPI::haveTechWeapon(m_entity))
	{
		AIAPI::print(m_entity, "|PT|thinkAttack: ��� ������ - ����� ������������");
		m_mode = PTEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		m_activity_observer.clear();
		return false;
	}

	// 2. ������� �����
	AIAPI::entity_list_t enemies;
	eid_t enemy = 0;
	if(AIAPI::getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// ��� ����� �����

		// ���� ��� ������ ���� ��� ������� - ������� ������ �������������
		if(m_turn_state == TS_START)
		{
			m_ignored_enemies.clear();
		}
		// ��������� ����� �������� ������� ���� ������ � �������������� ������
		EntityList_t diff_list;
		setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);
		if(diff_list.empty())
		{
			// ������� ������ �������� ����� �� ����
			enemy = AIAPI::getMostDangerousEnemy(enemies,
				EnemyComparator(m_entity));
		}
		else
		{
			// ������� ������ �������� ����� �� �������������������
			enemy = AIAPI::getMostDangerousEnemy(diff_list,
				EnemyComparator(m_entity));
		}
		m_last_enemy_pnt = AIAPI::getPos2(enemy);
	}
	
	// 3. ���� ��� ������ - ��������� � ��������� �������������
	if(!enemy)
	{
		// ������ ��� - ����� ������������ ����������
		AIAPI::print(m_entity, "|PT|thinkAttack: ������ ��� - ����� ������������ ����������");
		m_mode = PTEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		m_target_pnt = m_last_enemy_pnt;
		m_activity_observer.clear();
		return false;
	}
	// ����� ����...

	// 4. ���� ��� ������ ������ ���� ��� ��������� �������
	if(m_turn_state == TS_START)
	{
		// �������� ������ ����� � ���� � ���
		AIAPI::print(m_entity, "|PT|thinkAttack: �������� ������ ����� � ���� � ���");

		// ������� ������ ���� ���, ����� ����� ������� ����������
		// �� ���� �������, ���� ����� ������ ��� - �������
		// ������ ����, � ������� �����.
		
		m_turn_state = TS_INPROGRESS;
		// ������� ����� ���-�� ���������� ����������
		int movpoints = AIAPI::getMovePoints(m_entity);
		// ������ ���������� ���������� ����������� ��� ��������
		movpoints -= AIAPI::getShootMovePoints(m_entity,
			/*������ �� ���� ��������*/SHT_AIMSHOT);
		// ������ ��������� �� ������� � �����
		movpoints -= AIUtils::CalcMps4EntityRotate(PIm2);
		int steps = AIAPI::getSteps(m_entity, movpoints);
		if(!steps)
		{
			// ����� ������� ��� ����� ��� ����, ������ ��������� �
			// �����
			return false;
		}
		// ���������� ���� ������������
		PathUtils::CalcPassField(AIAPI::getPtr(m_entity),
			0, 0, 0);
		pnt_vec_t field;
		AIAPI::getBaseField(AIAPI::getPos2(m_entity),
			steps, &field);
		// ������� ������ ��������� ������
		pnt_vec_t vec;
		AIAPI::getReachableField(field, &vec);
		// ������� ������ ����
		m_target_pnt = AIAPI::getBestPoint(vec,
			WalkingTechHexComparator(m_entity, enemy));
		pnt_vec_t path;
		PathUtils::CalcPath(AIAPI::getPtr(m_entity),
			m_target_pnt, path);
		// ����� ����
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

	// 5. ��� ����������� ����

	// 5.1 ���� �� ������� ���������� - �������� ���
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// ����������� - �������� ���
		AIAPI::print(m_entity, "|PT|thinkAttack: ����������� ���������");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 5.2 ���� ��������� ������� ��� ������ ������ ����� - ���������
	//       ������ ����
	if( (m_activity_observer.getLastEvent() == ActivityObserver::EV_ENEMY_REACTED) ||
		(m_activity_observer.getLastEvent() == ActivityObserver::EV_ENEMY_SPOTTED)
		)
	{
		AIAPI::print(m_entity, "|PT|thinkAttack: ��������� ������� ��� ������ ������ �����");
		m_activity_observer.clear();
		// ��������� ������ ����
		m_ignored_enemies.clear();
		m_turn_state = TS_START;
		return true;
	}

	// 5.3 ���� ��� ����� ���� - ��������� ����� � ������ �������������
	// ������, ���� ����� ����� ��� ����� ��������� � ������ �������������
	// ��������� ������� � ������ �� ���
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_NO_LOF)
	{
		// ����������� - �������� ���
		AIAPI::print(m_entity, "|PT|thinkAttack: ��� ����� ���� - ��������� ����� � ������ �������������");
		m_activity_observer.clear();
		// ������� � ������ �������������
		m_ignored_enemies.insert(enemy);
		EntityList_t diff_list;
		setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);
		if(diff_list.empty())
		{
			// ��������� ������� ������� � �����
			AIAPI::print(m_entity, "|PT|thinkAttack: �������� �������");
			// ���������� ���� ������������
			PathUtils::CalcPassField(AIAPI::getPtr(m_entity),
				0, 0, PathUtils::F_CALC_NEAR_PNTS);
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
				// ���� ����, � �������� ����� ������� - �������� �� �������� ����
				PathUtils::CalcPath(AIAPI::getPtr(m_entity),
					m_target_pnt, path);
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
		// ���������� ��� ������ ������ ������
		m_turn_state = TS_INPROGRESS;
		return true;
	}

	// 5.4 ���� �� ��������a � ����� - �������������� � ����������� ��� ��� ����
	m_target_dir = norm_angle(Dir2Angle(AIAPI::getPos3(enemy) - getPosition()));
	float cur_angle = norm_angle(AIAPI::getAngle(m_entity));
	if( fabs(m_target_dir - cur_angle) >= angle_eps )
	{
		// ����� �������������� � �����
		m_activity = ActivityFactory::CreateRotate(AIAPI::getPtr(m_entity),
			m_target_dir,
			ActivityFactory::CT_ENEMY_ROTATE);
		m_activity->Attach(&m_activity_observer,
			ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	
	// 5.5 ��������a � ����� - ���� ������ ����� ���������
	AIAPI::print(m_entity, "|PT|thinkAttack: ����� ��������, ���� ��� ��������!");
	// ���� ���� - ��������� �� ���� ���� ��� ��������
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
		std::string str = "|PT|thinkAttack: �������� ������: ";
		str += reason;
		AIAPI::print(m_entity, str.c_str());
		m_turn_state = TS_END;
	}
	return false;
}

//=====================================================================================//
//                      bool PatrolTechEnemyNode::thinkPursuit()                       //
//=====================================================================================//
// ��������� ������ Pursuit
bool PatrolTechEnemyNode::thinkPursuit(state_type* st)
{
	// 1. ���� � ���� ����� - ����� ��������� � ������� � �����
	AIAPI::entity_list_t enemies;
	if(AIAPI::getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// � ����-�� ����
		if(eid_t enemy = AIAPI::getMostDangerousEnemy(enemies,
			EnemyComparator(m_entity))
			)
		{
			AIAPI::print(m_entity, "|PT|thinkPursuit: � ������a �����!");
			// ������a ������ �������� �����
			// ����� ��������� ����� �������
			getSubteam()->sendMessage(this,
				AIEnemyNode::MT_ENEMYSPOTTED,
				&enemy,
				patrol_tech_enemy_dist);
			// ������� � ��������� �����
			m_mode = PTEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_last_enemy_pnt = AIAPI::getPos2(enemy);

			// ������� ������ ���� ���, ����� ����� ������� ����������
			// �� ���� �������, ���� ����� ������ ��� - �������
			// ������ ����, � ������� �����.

			// ������� ����� ���-�� ���������� ����������
			int movpoints = AIAPI::getMovePoints(m_entity);
			// ������ ���������� ���������� ����������� ��� ��������
			movpoints -= AIAPI::getShootMovePoints(m_entity,
				/*������ �� ���� ��������*/SHT_AIMSHOT);
			// ������ ��������� �� ������� � �����
			movpoints -= AIUtils::CalcMps4EntityRotate(PIm2);
			int steps = AIAPI::getSteps(m_entity, movpoints);
			if(!steps)
			{
				// ����� ������� ��� ����� ��� ����, ������ ��������� �
				// �����
				return false;
			}
			// ���������� ���� ������������
			PathUtils::CalcPassField(AIAPI::getPtr(m_entity),
				0, 0, 0);
			pnt_vec_t field;
			AIAPI::getBaseField(AIAPI::getPos2(m_entity),
				steps, &field);
			// ������� ������ ��������� ������
			pnt_vec_t vec;
			AIAPI::getReachableField(field, &vec);
			// ������� ������ ����
			m_target_pnt = AIAPI::getBestPoint(vec,
				WalkingTechHexComparator(m_entity, enemy));
			pnt_vec_t path;
			PathUtils::CalcPath(AIAPI::getPtr(m_entity),
				m_target_pnt, path);
			// ����� ����
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

	// 2. �������� �� ��������� �� � ��� ���������
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// �� ������� ����������
		AIAPI::print(m_entity, "|PT|thinkPursuit: �� ������� ����������");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 3. �������� �� ������ �� �� � �������� �����
	if(m_target_pnt == AIAPI::getPos2(m_entity))
	{
		// �� ������
		// 3.1 ��������, ��������a �� � �����, ��� ��� ����
		m_target_dir = norm_angle(Dir2Angle(AIAPI::convertPos2ToPos3(m_last_enemy_pnt) - getPosition()));
		float cur_angle = norm_angle(AIAPI::getAngle(m_entity));
		if( fabs(m_target_dir - cur_angle) >= angle_eps )
		{
			// ����� �������������� � �����
			m_activity = ActivityFactory::CreateRotate(AIAPI::getPtr(m_entity),
				m_target_dir,
				ActivityFactory::CT_ENEMY_ROTATE);
			m_activity->Attach(&m_activity_observer,
				ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_turn_state = TS_INPROGRESS;
			return false;
		}
		// 3.2 ��������a - ������� � ��������� ��������������
		AIAPI::print(m_entity, "|PT|thinkPursuit: ������ �� ������a - ����� �������������");
		// ��������� � ��������� ��������������
		m_mode = PTEM_PATROL;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 4. ���� � �������� �����
	AIAPI::print(m_entity, "|PT|thinkPursuit: ���� � �������� �����");
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
// ����!
bool PatrolTechEnemyNode::die()
{
	m_mode = PTEM_KILLED;
	return need2Delete();
}

//=====================================================================================//
//                    bool PatrolTechEnemyNode::need2Delete() const                    //
//=====================================================================================//
// ����� �� ������� ����
bool PatrolTechEnemyNode::need2Delete() const
{
	if( (m_mode == PTEM_KILLED) && !m_activity) return true;
	return false;
}

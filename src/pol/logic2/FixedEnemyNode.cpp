#include "logicdefs.h"
#include "PathUtils.h"
#include "SubteamNode.h"
#include "FixedEnemyNode.h"

/////////////////////////////////////////////////////////////////////////////
//
// ���� ��� ������������� �����
//
/////////////////////////////////////////////////////////////////////////////

DCTOR_IMP(FixedEnemyNode)

//=====================================================================================//
//                          FixedEnemyNode::FixedEnemyNode()                           //
//=====================================================================================//
//. ����������� - id �������� � �������, ��� �� ����� ������
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

    // ���� �������� - ����� ��������
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
		// �����������
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
		// ��������
		int tmp;
		st >> tmp; m_mode = static_cast<FixedEnemyMode>(tmp);
		st >> tmp; m_turn_state = static_cast<TurnState>(tmp);
		st >> tmp; m_lookround_state = static_cast<LookroundState>(tmp);
		st >> m_entity;
		st >> tmp; m_activity_observer.setLastEvent(static_cast<ActivityObserver::event_type>(tmp));

		st >> m_basepnt.x >> m_basepnt.y >> m_basedir >> m_turn >> m_target_dir
			>> m_target_pnt.x >> m_target_pnt.y >> m_prev_dir;
		
		// ��, ��� ����������������� ��� ������
		m_activity.Reset();
		AIAPI::getBaseField(m_basepnt, 10, &m_base_field);
		m_initialising = false;
	}
}


//=====================================================================================//
//                          void FixedEnemyNode::OnSpawnMsg()                          //
//=====================================================================================//
//��������� ��������� � �����������
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
// ��������� ��������� �� �������� ��� ���������
void FixedEnemyNode::OnShootEv(BaseEntity* who, BaseEntity* victim, const point3& where)
{
	BaseEntity* be = AIAPI::getPtr(m_entity);
	if( be && (be == who) ) return; // ��� � ��� ��������

	float dist = (getPosition() - where).Length();
	if( (dist < alarm_dist) || (be == victim))
	{	// ����� ����������� - ������ �� ��������� ���
		m_turn = fixed_turns-1;
	}
}

//=====================================================================================//
//                          bool FixedEnemyNode::thinkBase()                           //
//=====================================================================================//
// ��������� ������ Base
bool FixedEnemyNode::thinkBase(state_type* st)
{
	// 1. �������� ���� ��������, ���� ��� < �������� - ������ ���������
	SendCureMessage(fixed_cure_dist);

	// 2. ���� � ��� ��� ������ - �������� � ��� ������
	if( (!AIAPI::takeBestWeapon(m_entity, FixedWeaponComparator())) &&	(!AIAPI::takeGrenade(m_entity))	)
	{
		AIAPI::print(m_entity, "|F|thinkBase: ���� ������ �����!");
		m_mode = FEM_WEAPONSEARCH;
		m_turn_state = TS_INPROGRESS;
		m_turn = 0;
		return false;
	}
	
	// 3. ���� � ���� ����� - ����� ��������� � ������� � �����
	AIAPI::entity_list_t enemies;
	if(AIAPI::getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// � ����-�� ����
		if(eid_t enemy = AIAPI::getMostDangerousEnemy(enemies,
			EnemyComparator(m_entity)) 	)
		{
			AIAPI::print(m_entity, "|F|thinkBase: � ������ �����!");
			// ������ ������ �������� �����, ����� ��������� ����� �������
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
				// ������� � ��������� �����, ������� ������ ���� � ������� 10 ������ �� ������� �����:
				// ���������� ���� ������������
				PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, 0, 0);
				// ������� ������ ��������� ������ �������� ����
				pnt_vec_t vec;
				AIAPI::getReachableField(m_base_field, &vec, true);
				// ������� ������ ����
				m_target_pnt = AIAPI::getBestPoint(vec,	AssaultHexComparator(m_entity, enemy));
				pnt_vec_t path;
				PathUtils::CalcPath(AIAPI::getPtr(m_entity),	m_target_pnt, path);
				// ����� ����
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

	// 4. ���� ��� ���������� ������� ����� - ������� � �����
	enemies.clear();
	if(AIAPI::getEnemiesVisibleBySubteam(getSubteam()->getName(),&enemies)  )
	{
		// ������� - ������� ������ ��������
		if(eid_t enemy = AIAPI::getMostDangerousEnemy(enemies,
			EnemyComparator(m_entity)))
		{
			AIAPI::print(m_entity, "|F|thinkBase: ��� ������� ������� �����!");
			// ������ ������ �������� �����, ������� � ��������� �����
			// ������� ������ ���� � ������� 10 ������ �� ������� �����:
			// ���������� ���� ������������
			PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, 0, 0);
			// ������� ������ ��������� ������ �������� ����
			pnt_vec_t vec;
			AIAPI::getReachableField(m_base_field, &vec, true);
			// ������� ������ ����
			m_target_pnt = AIAPI::getBestPoint(vec,	FixedHexComparator(m_entity, enemy));
			pnt_vec_t path;
			PathUtils::CalcPath(AIAPI::getPtr(m_entity),m_target_pnt, path);
			// ����� ����
			m_activity = CreateMoveActivity(path, ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer,ActivityObserver::EV_ENEMY_REACTED);
			m_mode = FEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_turn = 0;
			return false;
		}
	}

	// 5. ������ ���
	// 5.1 ���� �������� ������ ��������� - ��������� ���� ��������
	SelfCure();

	// 5.2 ���� ���� ������� � ��� ��� ����� - ������ �� ���
	if(needAndSeeMedkit(&m_target_pnt))
	{
		PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, 0, 0);
		pnt_vec_t path;
		PathUtils::CalcPath(AIAPI::getPtr(m_entity), m_target_pnt, path);
		if(!path.empty())
		{
			// ����� ����
			m_activity = CreateMoveActivity(path, ActivityFactory::CT_ENEMY_MOVEIGNORE);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_mode = FEM_MEDKITTAKE;
			m_turn_state = TS_INPROGRESS;
			return false;
		}
	}

	// 5.3 ���� ����� ������������� - ��������
	if(++m_turn < fixed_turns)
	{
		AIAPI::print(m_entity, "|F|thinkBase: ��� �� ����� ����������");
		// ��� �� ����� �������������
		m_turn_state = TS_END;
		return false;
	}
	// ���� ����������
	AIAPI::print(m_entity, "|F|thinkBase: ���� �������������");
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
// ��������� ������ LookRound
bool FixedEnemyNode::thinkLookRound(state_type* st)
{
	// 1. �������� ���� ��������, ���� ��� < �������� - ������ ���������
	SendCureMessage(fixed_cure_dist);

	// 2. ���� � ���� ����� - ����� ��������� � ������� � �����
	AIAPI::entity_list_t enemies;
	if(AIAPI::getEnemiesVisibleByEntity(m_entity, &enemies))
	{	// � ����-�� ����
		if(eid_t enemy = AIAPI::getMostDangerousEnemy(enemies,EnemyComparator(m_entity))	)
		{
			AIAPI::print(m_entity, "|F|thinkLookRound: � ������ �����!");
			// ������ ������ �������� �����
			// ����� ��������� ����� �������
			getSubteam()->sendMessage(this,	AIEnemyNode::MT_ENEMYSPOTTED,&enemy,fixed_enemy_dist);
			// ������� � ��������� �����, ������� ������ ���� � ������� 10 ������ �� ������� �����:
			// ���������� ���� ������������
			PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, 0, 0);
			// ������� ������ ��������� ������ �������� ����
			pnt_vec_t vec;
			AIAPI::getReachableField(m_base_field, &vec, true);
			// ������� ������ ����
			m_target_pnt = AIAPI::getBestPoint(vec,	FixedHexComparator(m_entity, enemy));
			pnt_vec_t path;
			PathUtils::CalcPath(AIAPI::getPtr(m_entity),	m_target_pnt, path);
			// ����� ����
			m_activity = CreateMoveActivity(path,ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer,ActivityObserver::EV_ENEMY_REACTED);
			m_mode = FEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_turn = 0;
			return false;
		}
	}

	// 3. ���� ��� ���������� ������� ����� - ������� � �����
	enemies.clear();
	if(AIAPI::getEnemiesVisibleBySubteam(getSubteam()->getName(),&enemies))
	{
		// ������� - ������� ������ ��������
		if(eid_t enemy = AIAPI::getMostDangerousEnemy(enemies,EnemyComparator(m_entity))	)
		{
			AIAPI::print(m_entity, "|F|thinkLookRound: ��� ������� ������� �����!");
			// ������ ������ �������� �����, ������� � ��������� �����
			// ������� ������ ���� � ������� 10 ������ �� ������� �����:
			// ���������� ���� ������������
			PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, 0, 0);
			// ������� ������ ��������� ������ �������� ����
			pnt_vec_t vec;
			AIAPI::getReachableField(m_base_field, &vec, true);
			// ������� ������ ����
			m_target_pnt = AIAPI::getBestPoint(vec,	FixedHexComparator(m_entity, enemy));
			pnt_vec_t path;
			PathUtils::CalcPath(AIAPI::getPtr(m_entity),	m_target_pnt, path);
			// ����� ����
			m_activity = CreateMoveActivity(path, ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer,	ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer,	ActivityObserver::EV_ENEMY_REACTED);
			m_mode = FEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			m_turn = 0;
			return false;
		}
	}

	// 4. ������� ������� ����, �� ������� ��������� ��������
	float cur_angle = norm_angle(AIAPI::getAngle(m_entity));
	if(fabs(m_target_dir - cur_angle) < angle_eps)
	{
		// �������� ��������� ����� ���������
		switch(m_lookround_state)
		{
		case LS_FIRST : 
			{
				AIAPI::print(m_entity, "|F|thinkLookRound: �������� ������ ����� ���������");
				// �������� ������ ����� ���������
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
				AIAPI::print(m_entity, "|F|thinkLookRound: �������� ������ ����� ���������");
				// �������� �������������� ����� ���������
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
				AIAPI::print(m_entity, "|F|thinkLookRound: ��������� �������� ��������");
				// ��������� � ������� ���������
				m_mode = FEM_BASE;
				m_turn_state = TS_END;
				break;
			}
		default : break;
		}
		return false;
	}

	// 5. ���� �� ������� ���������� - ��������� �� �������� ���
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		AIAPI::print(m_entity, "|F|thinkLookRound: �� ������� ���������� - ��������� �� �������� ���");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// ��������� ��������
	m_activity = CreateRotateActivity(m_target_dir,ActivityFactory::CT_ENEMY_LOOKROUND);
	
	m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_activity->Attach(&m_activity_observer,ActivityObserver::EV_ENEMY_SPOTTED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

//=====================================================================================//
//                         bool FixedEnemyNode::thinkAttack()                          //
//=====================================================================================//
// ��������� ������ Attack
bool FixedEnemyNode::thinkAttack(state_type* st)
{
	// 1. �������� ���� ��������, ���� ��� < �������� - ������ ���������
	SendCureMessage(fixed_cure_dist);

	// 2. ���� � ��� ��� ������ - �������� � ��� ������
	if( (!AIAPI::takeBestWeapon(m_entity, FixedWeaponComparator())) &&
		(!AIAPI::takeGrenade(m_entity))	)
	{
		AIAPI::print(m_entity, "|F|thinkAttack: ���� ������ �����!");
		m_mode = FEM_WEAPONSEARCH;
		m_turn_state = TS_INPROGRESS;
		m_activity_observer.clear();
		AIAPI::setStandPose(m_entity);
		return false;
	}

	// 3. ������� �����
	AIAPI::entity_list_t enemies;
	eid_t enemy = 0;
	if(AIAPI::getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// ��� ����� �����
		// ���� ��� ������ ���� ��� ������� - ������� ������ �������������
		if(m_turn_state == TS_START)
		{
			m_ignored_enemies.clear();
			// � ������ �������� ���� ��� �� �����
			AIAPI::setSitPose(m_entity);
		}
		// ��������� ����� �������� ������� ���� ������ � �������������� ������
		EntityList_t diff_list;
		setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);
		if(diff_list.empty()) // ������� ������ �������� ����� �� ����
			enemy = AIAPI::getMostDangerousEnemy(enemies,EnemyComparator(m_entity));
		else // ������� ������ �������� ����� �� �������������������
			enemy = AIAPI::getMostDangerousEnemy(diff_list,	EnemyComparator(m_entity));
	}
	
	if(!enemy)
	{
		// ��� ����� �� �����
		enemies.clear();
		if(AIAPI::getEnemiesVisibleBySubteam(getSubteam()->getName(),&enemies)	)
		{
			// ���� ����� ��� ����������
			// ���� ��� ������ ���� ��� ������� - ������� ������ �������������
			if(m_turn_state == TS_START)
			{
				m_ignored_enemies.clear();
				// � ������ �������� ���� ��� �� �����
				AIAPI::setSitPose(m_entity);
			}
			// ��������� ����� �������� ������� ���� ������ � �������������� ������
			EntityList_t diff_list;
			setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);
			if(diff_list.empty())// ������� ������ �������� ����� �� ����
				enemy = AIAPI::getMostDangerousEnemy(enemies,EnemyComparator(m_entity));
			else// ������� ������ �������� ����� �� �������������������
				enemy = AIAPI::getMostDangerousEnemy(diff_list,	EnemyComparator(m_entity));
		}
	}
	
	// 4. ���� ��� ������ - ������� � ������� �����
	if(!enemy)
	{
		// ������ ��� ������ - ������� � ��������� ������� � ������� �����
		AIAPI::print(m_entity, "|F|thinkAttack: ������ ��� - �� ����");
		m_mode = FEM_RETURN2BASE;
		m_turn_state = TS_INPROGRESS;
		m_activity_observer.clear();
		AIAPI::setStandPose(m_entity);
		return false;
	}
	// ����� ����...

	// 5. ���� ��� ������ ������ ����
	if(m_turn_state == TS_START)
	{
		// �������� ������ ����� � ���� � ���
		AIAPI::setStandPose(m_entity);
		AIAPI::print(m_entity, "|F|thinkAttack: �������� ������ ����� � ���� � ���");
		// ������� ������ ���� � ������� 10 ������ �� ������� �����:
		// ���������� ���� ������������
		PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, 0, 0);
		// ������� ������ ��������� ������ �������� ����
		pnt_vec_t vec;
		AIAPI::getReachableField(m_base_field, &vec, true);
		// ������� ������ ����
		m_target_pnt = AIAPI::getBestPoint(vec,	FixedHexComparator(m_entity, enemy));

		m_turn_state = TS_INPROGRESS;

		if(m_target_pnt != AIAPI::getPos2(m_entity))
		{
			pnt_vec_t path;
			PathUtils::CalcPath(AIAPI::getPtr(m_entity),	m_target_pnt, path);
			// ����� ����
			m_activity = CreateMoveActivity(path, ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer,	ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer,	ActivityObserver::EV_ENEMY_REACTED);
			m_activity->Attach(&m_activity_observer,	ActivityObserver::EV_ENEMY_SPOTTED);
			
			return false;
		}
	}

	// 6. ��� ����������� ����

	// 6.1 ���� ����� �� ������� ���������� - �� �������� ���
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		AIAPI::print(m_entity, "|F|thinkAttack: ����������� ���������");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}
	
	// 6.2 ���� ��������� ������� ��� ������ ������ ����� - ���������
	//       ������ ����
	if( (m_activity_observer.getLastEvent() == ActivityObserver::EV_ENEMY_REACTED) ||
		(m_activity_observer.getLastEvent() == ActivityObserver::EV_ENEMY_SPOTTED) 	)
	{
		AIAPI::print(m_entity, "|F|thinkAttack: ��������� ������� ��� ������ ������ �����");
		m_activity_observer.clear();
		// ��������� ������ ����
		m_ignored_enemies.clear();
		m_turn_state = TS_START;
		return false;
	}

	// 6.3 ���� ��� ����� ���� - ��������� ����� � ������ �������������
	// ������, ���� ����� ����� ��� ����� ��������� � ������ �������������
	// �������� ���
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_NO_LOF)
	{
		AIAPI::print(m_entity, "|F|thinkAttack: ��� ����� ���� - ��������� ����� � ������ �������������");
		m_activity_observer.clear();
		// ������� � ������ �������������
		m_ignored_enemies.insert(enemy);

		EntityList_t diff_list;
		setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);

		if(!diff_list.empty())
		{
			// ���������� ��� ������ ������ ������
			m_turn_state = TS_INPROGRESS;
			return true;
		}
		
		if(AIAPI::isSitPose(m_entity))
		{
			// �������
			AIAPI::setStandPose(m_entity);
			// ������� ������� ����������������� ������
			m_ignored_enemies.clear();
			// ��������� �������� ������ ��� ���
			m_turn_state = TS_INPROGRESS;
			return false;
		}
		// �������� ���
		AIAPI::setSitPose(m_entity);
		m_turn_state = TS_END;
		return false;
	}

	// 6.4 ���� ����� ����� ������ ����� - �������� ������ ����, ��������� �� ����������
	eid_t weak_enemy = AIAPI::getLeastDangerousEnemy(enemies, EnemyComparator(m_entity));
	if(enemy != weak_enemy)
	{
		// ������ ����� ������ - ��������� ������ � ������� ������
		if(AIAPI::takeShield(m_entity))
		{
			AIAPI::print(m_entity, "|F|thinkAttack: � �� ��� ������!");
			AIAPI::throwShield(m_entity, weak_enemy, &m_activity);
			m_turn_state = TS_INPROGRESS;
			return false;
		}
	}

	// 6.5 ���� �� �������� � ����� - �������������� � ����������� ��� ��� ����
	m_target_dir = norm_angle(Dir2Angle(AIAPI::getPos3(enemy) - getPosition()));
	float cur_angle = norm_angle(AIAPI::getAngle(m_entity));
	if( fabs(m_target_dir - cur_angle) >= angle_eps )
	{
		// ����� �������������� � �����
		AIAPI::setStandPose(m_entity);
		m_activity = CreateRotateActivity(m_target_dir, ActivityFactory::CT_ENEMY_ROTATE);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	// 6.6 �������� � ����� - ���� ������ ����� ���������, ��������
	AIAPI::setSitPose(m_entity);
	
	AIAPI::print(m_entity, "|F|thinkAttack: ����� ��������, ���� ��� ��������!");
	// ���� ���� - ��������� �� ���� ���� ��� ��������
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
		std::string str = "|F|thinkAttack: �������� ������: ";
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
// ��������� ������ Return2Base
bool FixedEnemyNode::thinkReturn2Base(state_type* st)
{
	// 1. �������� ������ �� ��� � ������� ����� ��� ���
	if(m_basepnt == AIAPI::getPos2(m_entity))
	{
		// ������ - ������� � ������� ���������
		AIAPI::print(m_entity, "|F|thinkReturn2Base: ������ � ������� �����");
		m_mode = FEM_BASE;
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	// 2. �� ����������� �� ���������?
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED
		|| AIAPI::getMovePoints(m_entity) == 0)
	{
		// ����������� - ������� ���
		AIAPI::print(m_entity, "|F|thinkReturn2Base: ����������� ���������");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// �� ��������� �� � ������� �����
	// 3. ��������, ����� �� ������ ����� �� ������� �����
	pnt_vec_t path;
	PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, 0, 0);
	PathUtils::CalcPath(AIAPI::getPtr(m_entity),	m_basepnt, path);
	if(!path.empty())
	{
		// ����� - ������ � ���
		m_target_pnt = m_basepnt;
		m_activity = CreateMoveActivity(path, ActivityFactory::CT_ENEMY_MOVEIGNORE);
		m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// � ������� ����� ����� ������
	// 4. ��������� �� �� �� ������� ����?
	if(AIAPI::isEntityInBaseField(m_entity, m_base_field))
	{
		// �� - ������� � ������� ���������
		AIAPI::print(m_entity, "|F|thinkReturn2Base: ������ � ������� ����");
		m_mode = FEM_BASE;
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	
	// �� ��� �������� ����
	// 5. ����� �� ����� �� �����-������ ����� � ������� ����?
	pnt_vec_t vec;
	AIAPI::getReachableField(m_base_field, &vec, true);
	if(vec.empty())
	{
		// ������ - ������� � ������� ���������
		AIAPI::print(m_entity, "|F|thinkReturn2Base: �� ���� ����� ���� �� �������� ����!");
		m_mode = FEM_BASE;
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	// �����
	// ������� ��������� �����
	m_target_pnt = AIAPI::getRandomPoint(vec);
	PathUtils::CalcPath(AIAPI::getPtr(m_entity),	m_target_pnt, path);
	// ������ � ���
	m_activity = CreateMoveActivity(path, ActivityFactory::CT_ENEMY_MOVEIGNORE);
	m_activity->Attach(&m_activity_observer,		ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

//=====================================================================================//
//                      bool FixedEnemyNode::thinkWeaponSearch()                       //
//=====================================================================================//
// ��������� ������ WeaponSearch
bool FixedEnemyNode::thinkWeaponSearch(state_type* st)
{
	// 1. �������� ���� ��������, ���� ��� < �������� - ������ ���������
	SendCureMessage(fixed_cure_dist);
	// 2. �������� ���� �� ���-���� ����� ������
	if(AIAPI::getWeaponLocation(m_entity, &m_target_pnt))
	{
		// �������� ���� ������ - ������� � ��� �������
		pnt_vec_t base, vec, path;
		AIAPI::getBaseField(m_target_pnt, 1, &base);
		AIAPI::getReachableField(base, &vec, true);
		m_target_pnt = AIAPI::getRandomPoint(vec);

		PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, 0, 0);
		PathUtils::CalcPath(AIAPI::getPtr(m_entity),	m_target_pnt, path);
		if(path.empty())// � ������ ������ ������� - ������� ���
		{
			m_turn_state = TS_END;
			return false;
		}
		// ����� ����
		m_activity = CreateMoveActivity(path, ActivityFactory::CT_ENEMY_MOVEIGNORE);
		m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		m_mode = FEM_WEAPONTAKE;
		return false;
	}

	// ������ ����� ��� - �������� �� ��������� �� � ��� ���������
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED
		|| AIAPI::getMovePoints(m_entity) == 0)
	{
		// �� ������� ����������
		AIAPI::print(m_entity, "|F|thinkWeaponSearch: �� ������� ����������");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}
	
	// ���� �������� ������ ��������� - ��������� ���� ��������
	SelfCure();

	// ��������� ���� - ������� ��������� ����� � ��������� ����
	pnt_vec_t vec, path;
	AIAPI::getReachableField(m_base_field, &vec, true);
	m_target_pnt = AIAPI::getRandomPoint(vec);

	PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, 0, 0);
	PathUtils::CalcPath(AIAPI::getPtr(m_entity),m_target_pnt, path);
	if(path.empty())
	{
		// ������ ���� - ��������� ���
		m_turn_state = TS_END;
		return false;
	}
	// ����� ����
	m_activity = CreateMoveActivity(path, ActivityFactory::CT_ENEMY_MOVEIGNORE);
	m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

//=====================================================================================//
//                       bool FixedEnemyNode::thinkWeaponTake()                        //
//=====================================================================================//
// ��������� ������ WeaponTake
bool FixedEnemyNode::thinkWeaponTake(state_type* st)
{
	// 1. �������� �� ������� �� �� � ������
	if(PickUpWeaponAndAmmo(m_target_pnt, FixedWeaponComparator()))
	{
		m_mode = FEM_RETURN2BASE;
		return false;
	}

	// 2. �������� �� ��������� �� � ��� ���������
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// �� ������� ����������
		AIAPI::print(m_entity, "|F|thinkWeaponTake: �� ������� ����������");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 3. �������� ���� �� ���-���� ����� ������
	if(AIAPI::getWeaponLocation(m_entity, &m_target_pnt))
	{
		// �������� ���� ������ - ������ � ����
		pnt_vec_t base, vec, path;
		AIAPI::getBaseField(m_target_pnt, 1, &base);
		AIAPI::getReachableField(base, &vec, true);
		m_target_pnt = AIAPI::getRandomPoint(vec);

		PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, 0, 0);
		PathUtils::CalcPath(AIAPI::getPtr(m_entity),m_target_pnt, path);
		if(path.empty())
		{
			// � ������ ������ ������� - ������� �� ����
			m_mode = FEM_RETURN2BASE;
			m_turn_state = TS_INPROGRESS;
			return false;
		}
		// ����� ����
		m_activity = CreateMoveActivity(path, ActivityFactory::CT_ENEMY_MOVEIGNORE);
		m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// ������ ��� - ������� �� ����
	m_mode = FEM_RETURN2BASE;
	m_turn_state = TS_INPROGRESS;
	return false;
}

//=====================================================================================//
//                       bool FixedEnemyNode::thinkMedkitTake()                        //
//=====================================================================================//
// ��������� ������ MedkitTake
bool FixedEnemyNode::thinkMedkitTake(state_type* st)
{
	if(!MedkitTake(m_target_pnt,st))	m_mode = FEM_RETURN2BASE;
	return false;

}

//=====================================================================================//
//                             bool FixedEnemyNode::die()                              //
//=====================================================================================//
// ����!
bool FixedEnemyNode::die()
{
	m_mode = FEM_KILLED;
	return need2Delete();
}

//=====================================================================================//
//                      bool FixedEnemyNode::need2Delete() const                       //
//=====================================================================================//
// ����� �� ������� ����
bool FixedEnemyNode::need2Delete() const
{
	if( (m_mode == FEM_KILLED) && !m_activity) return true;
	return false;
}

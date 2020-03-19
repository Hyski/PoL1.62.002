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
// ����������� - id ��������
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
    //��������� ��������
    if(m_activity){

        if(st) *st = ST_INCOMPLETE;

        if(!m_activity->Run(AC_TICK)){
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
		// �����������
        st << static_cast<int>(m_mode);
        st << static_cast<int>(m_turn_state);
		st << m_entity;
		st << static_cast<int>(m_activity_observer.getLastEvent());

		st << m_basepnt.x << m_basepnt.y << m_basedir << m_target_dir;
    }
	else
	{
		// ��������
		int tmp;
		st >> tmp; m_mode = static_cast<FixedTechEnemyMode>(tmp);
		st >> tmp; m_turn_state = static_cast<TurnState>(tmp);
		st >> m_entity;
		st >> tmp; m_activity_observer.setLastEvent(static_cast<ActivityObserver::event_type>(tmp));

		st >> m_basepnt.x >> m_basepnt.y >> m_basedir >> m_target_dir;
		
		// ��, ��� ����������������� ��� ������
		m_activity = 0;
		m_initialising = false;
	}
}

//=====================================================================================//
//                   point3 FixedTechEnemyNode::getPosition() const                    //
//=====================================================================================//
// ������ ����������� �������
point3 FixedTechEnemyNode::getPosition() const
{
	return AIAPI::getPos3(m_entity);
}

//=====================================================================================//
//                        void FixedTechEnemyNode::OnSpawnMsg()                        //
//=====================================================================================//
//��������� ��������� � �����������
void FixedTechEnemyNode::OnSpawnMsg()
{
	m_basepnt = AIAPI::getPos2(m_entity);
	m_basedir = norm_angle(AIAPI::getAngle(m_entity));
}

//=====================================================================================//
//                        bool FixedTechEnemyNode::thinkBase()                         //
//=====================================================================================//
// ��������� ������ Base
bool FixedTechEnemyNode::thinkBase(state_type* st)
{
	// 1. ���� � ���� ����� - ����� ��������� � ������� � �����
	AIAPI::entity_list_t enemies;
	if(AIAPI::getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// � ����-�� ����
		if(eid_t enemy = AIAPI::getMostDangerousEnemy(enemies,
			EnemyComparator(m_entity))	)
		{
			AIAPI::print(m_entity, "|FT|thinkBase: � ������ �����!");
			// ������ ������ �������� �����, ����� ��������� ����� �������
			getSubteam()->sendMessage(this,	AIEnemyNode::MT_ENEMYSPOTTED, &enemy, fixed_tech_enemy_dist);
			// ������� � ��������� �����
			m_mode = FTEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			return false;
		}
	}

	// 2. ���� ��� ���������� ������� ����� - ������� � �����
	enemies.clear();
	if(AIAPI::getEnemiesVisibleBySubteam(getSubteam()->getName(),&enemies) )
	{
		// ������� - ������� ������ ��������
		if(eid_t enemy = AIAPI::getMostDangerousEnemy(enemies,
			EnemyComparator(m_entity))	)
		{
			AIAPI::print(m_entity, "|FT|thinkBase: ��� ������� ������� �����!");
			// ������ ������ �������� �����, ������� � ��������� �����
			m_mode = FTEM_ATTACK;
			m_turn_state = TS_INPROGRESS;
			return false;
		}
	}

	// 3. ������ ���
	AIAPI::print(m_entity, "|FT|thinkBase: ������ ���");
	m_turn_state = TS_END;
	return false;
}

//=====================================================================================//
//                       bool FixedTechEnemyNode::thinkAttack()                        //
//=====================================================================================//
// ��������� ������ Attack
bool FixedTechEnemyNode::thinkAttack(state_type* st)
{
	// 1. ������� �����
	AIAPI::entity_list_t enemies;
	eid_t enemy = 0;
	if(AIAPI::getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// ���� ����� �����
		// ���� ��� ������ ���� ��� ������� - ������� ������ �������������
		if( (m_turn_state == TS_START) ||(m_activity_observer.getLastEvent() == ActivityObserver::EV_ENEMY_REACTED)	)
		{
			m_ignored_enemies.clear();
		}
		// ��������� ����� �������� ������� ���� ������ � �������������� ������
		EntityList_t diff_list;
		setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);
		if(diff_list.empty())
		{
			// ������� ������ �������� ����� �� ����
			enemy = AIAPI::getMostDangerousEnemy(enemies,EnemyComparator(m_entity));
		}
		else
		{
			// ������� ������ �������� ����� �� �������������������
			enemy = AIAPI::getMostDangerousEnemy(diff_list, EnemyComparator(m_entity));
		}
	}
	
	if(!enemy)
	{
		// ���a ����� �� �����
		enemies.clear();
		if(AIAPI::getEnemiesVisibleBySubteam(getSubteam()->getName(), &enemies) 	)
		{
			// ���� ����� �� ����������
			// ���� ��� ������ ���� ��� ������� - ������� ������ �������������
			if( (m_turn_state == TS_START) ||(m_activity_observer.getLastEvent() == ActivityObserver::EV_ENEMY_REACTED) 	)
			{
				m_ignored_enemies.clear();
			}
			// ��������� ����� �������� ������� ���� ������ � �������������� ������
			EntityList_t diff_list;
			setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);
			if(diff_list.empty())
			{
				// ������� ������ �������� ����� �� ����
				enemy = AIAPI::getMostDangerousEnemy(enemies, EnemyComparator(m_entity));
			}
			else
			{
				// ������� ������ �������� ����� �� �������������������
				enemy = AIAPI::getMostDangerousEnemy(diff_list, EnemyComparator(m_entity));
			}
		}
	}
	
	// 2. ���� ��� ������ - �������� � ������� �����������
	if(!enemy)
	{
		// ������ ��� ������ - �������� � ������� �����������
		AIAPI::print(m_entity, "|FT|thinkAttack: ������ ��� - �� ����");
		m_mode = FTEM_ROTATE2BASE;
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	// ����� ����...

	// 3. ���� ������� �� ������� ���������� - ��a �������� ���
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// ����������� - �������� ���
		AIAPI::print(m_entity, "|FT|thinkAttack: ����������� ���������");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 4. ���� ��� ����� ���� - ��������� ����� � ������ �������������
	// ������, ���� ����� ����� ��� ����� ��������� � ������ �������������
	// �������� ���
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_NO_LOF)
	{
		// ����������� - �������� ���
		AIAPI::print(m_entity, "|FT|thinkAttack: ��� ����� ���� - ��������� ����� � ������ �������������");
		m_activity_observer.clear();
		// ������� � ������ �������������
		m_ignored_enemies.insert(enemy);
		EntityList_t diff_list;
		setDifferenceBetweenLists(enemies, m_ignored_enemies, &diff_list);
		if(diff_list.empty())
		{
			// �������� ���
			m_turn_state = TS_END;
			return false;
		}
		// ���������� ��� ������ ������ ������
		m_turn_state = TS_INPROGRESS;
		return true;
	}


	// 5 ���� �� �������� � ����� - �������������� � ����������� ��� ��� ����
	m_target_dir = norm_angle(Dir2Angle(AIAPI::getPos3(enemy) - getPosition()));
	float cur_angle = norm_angle(AIAPI::getAngle(m_entity));
	if( fabs(m_target_dir - cur_angle) >= angle_eps )
	{
		// ����� �������������� � �����
		m_activity = ActivityFactory::CreateRotate(AIAPI::getPtr(m_entity),m_target_dir, 	ActivityFactory::CT_ENEMY_ROTATE);
		m_activity->Attach(&m_activity_observer,	ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	
	// 6. ��������a � ����� - ���� ������ ����� ���������
	AIAPI::print(m_entity, "|FT|thinkAttack: ����� ��������, ���� ��� ��������!");
	// ���� ���� - ��������� �� ���� ���� ��� ��������
	std::string reason;
	if(AIAPI::shootByVehicle(m_entity, enemy, &m_activity, &reason))
	{
		m_turn_state = TS_INPROGRESS;
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_activity->Attach(&m_activity_observer, ActivityObserver::EV_NO_LOF);
	}
	else
	{
		std::string str = "|FT|thinkAttack: �������� ������: ";
		str += reason;
		AIAPI::print(m_entity, str.c_str());
		m_turn_state = TS_END;
	}
	return false;
}

//=====================================================================================//
//                     bool FixedTechEnemyNode::thinkRotate2Base()                     //
//=====================================================================================//
// ��������� ������ Rotate2Base
bool FixedTechEnemyNode::thinkRotate2Base(state_type* st)
{
	// 1. �������� ���������� �� ������� � ������� �����������
	float cur_angle = AIAPI::getAngle(m_entity);
	if( fabs(m_basedir - cur_angle) < angle_eps )
	{
		// ���������� - �������� � ������� ���������
		AIAPI::print(m_entity, "|FT|thinkRotate2Base: ����������");
		m_mode = FTEM_BASE;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 2. �������� ���������
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// ����������� - �������� ���
		AIAPI::print(m_entity, "|FT|thinkRotate2Base: ����������� ���������");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 3. �������� � ������� �����������
	AIAPI::print(m_entity, "|FT|thinkRotate2Base: ���������������...");
	m_activity = ActivityFactory::CreateRotate(AIAPI::getPtr(m_entity),m_basedir,	ActivityFactory::CT_ENEMY_ROTATE);
	m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

//=====================================================================================//
//                           bool FixedTechEnemyNode::die()                            //
//=====================================================================================//
// ����!
bool FixedTechEnemyNode::die()
{
	m_mode = FTEM_KILLED;
	return need2Delete();
}

//=====================================================================================//
//                    bool FixedTechEnemyNode::need2Delete() const                     //
//=====================================================================================//
// ����� �� ������� ����
bool FixedTechEnemyNode::need2Delete() const
{
	if( (m_mode == FTEM_KILLED) && !m_activity) return true;
	return false;
}


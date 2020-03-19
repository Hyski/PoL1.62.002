#include "logicdefs.h"

#include "AIUtils.h"
#include "PathUtils.h"
#include "SubteamNode.h"
#include "PanicPlayer.h"
#include "CommonEnemyNode.h"

//=====================================================================================//
//                     point3 CommonEnemyNode::getPosition() const                     //
//=====================================================================================//
point3 CommonEnemyNode::getPosition() const
{
	return AIAPI::getPos3(m_entity);
}

//=====================================================================================//
//                         CommonEnemyNode::CommonEnemyNode()                          //
//=====================================================================================//
CommonEnemyNode::CommonEnemyNode(eid_t id)
:	m_turn_state(TS_NONE),
	m_lookround_state(LS_FIRST),
	m_entity(id),
	m_initialising(false)
{
}

//=====================================================================================//
//                         CommonEnemyNode::~CommonEnemyNode()                         //
//=====================================================================================//
CommonEnemyNode::~CommonEnemyNode()
{
}

//=====================================================================================//
//                          void CommonEnemyNode::InitPanic()                          //
//=====================================================================================//
void CommonEnemyNode::InitPanic()
{
	PanicPlayer::Init(AIAPI::getPtr(m_entity));
}

//=====================================================================================//
//                         bool CommonEnemyNode::ThinkShell()                          //
//=====================================================================================//
bool CommonEnemyNode::ThinkShell(state_type* st)
{
	if(m_activity.Run(AC_TICK,&m_activity_observer))
	{
		if(st) *st = ST_INCOMPLETE;
		return true;
	}

	if(!st)// ���� �� ��� ��� - �� �����
	{
		m_turn_state = TS_NONE;
		return true;
	}

	if(UnlimitedMoves())
	{
		if(st) *st = ST_FINISHED;
		return true;
	}

	if(m_turn_state == TS_NONE)	// ���� m_turn_state == TS_NONE, �� ��� ������ ������ ����
	{
		m_turn_state = TS_START;
		// ������ �������������
		InitPanic();
		m_initialising = true;
	}
	
	if(m_initialising)// �������� ���� �������������
	{
		if(PanicPlayer::Execute())
		{	// ����� ���������� ������� �������������
			*st = ST_INCOMPLETE;
			return true;
		}
		// ������������� ���������
		m_initialising = false;
	}

	return false;
}

//=====================================================================================//
//                        void CommonEnemyNode::CheckFinished()                        //
//=====================================================================================//
void CommonEnemyNode::CheckFinished(state_type *st,float *comp)
{
	if(m_turn_state == TS_END || AIAPI::getMovePoints(m_entity) == 0)
	{
		*st = ST_FINISHED;
		m_turn_state = TS_START;
		*comp = 1.0f;
	}
	else *st = ST_INCOMPLETE;
}

//=====================================================================================//
//                      bool CommonEnemyNode::needAndSeeMedkit()                       //
//=====================================================================================//
/// ���������, ����� �� ��� ������� � ���� �� � ��
bool CommonEnemyNode::needAndSeeMedkit(ipnt2_t* target)
{
	// �������� ���������� ������� � �����
	if(AIAPI::getThingCount(m_entity, TT_MEDIKIT) >= 2)
		return false;
	// ��������� ���� �� �� ���� � ����� �������
	if(AIAPI::getThingLocation(m_entity, target, TT_MEDIKIT))
		return true;
	return false;
}

//=====================================================================================//
//                          bool CommonEnemyNode::SelfCure()                           //
//=====================================================================================//
bool CommonEnemyNode::SelfCure()
{
	if(AIAPI::getHealthPercent(m_entity) < 100.0f)
	{
		AIAPI::print(m_entity, " �������� �����������!");
		if(AIAPI::haveMedikit(m_entity))
		{
			// � ���� ���� �������
			// ������ �� � ����
			AIAPI::takeMedikit(m_entity);
			// ������ ����
			AIAPI::cure(m_entity, m_entity);
			return true;
		}
	}
	return false;
}

//=====================================================================================//
//                     bool CommonEnemyNode::PickUpWeaponAndAmmo()                     //
//=====================================================================================//
bool CommonEnemyNode::PickUpWeaponAndAmmo(const ipnt2_t &m_target_pnt, WeaponComparator &comparator)
{
	if(m_target_pnt == AIAPI::getPos2(m_entity))
	{
		// �� ������� � ����������� ������ - ������� ��� ������
		AIAPI::print(m_entity, "�� ������� � ����������� ������ - ������� ��� ������");
		// ������� ��� �������
		AIAPI::pickupAllNearAmmo(m_entity);
		// �������� ���� �� ������ � ��� ���������� ������
		if( (!AIAPI::takeGrenade(m_entity)) &&
			(!AIAPI::takeBestWeapon(m_entity, comparator))	)
		{
			// ���������� ����� ��� ��� ��� ����� ��������� ����� �� �����,
			// ���� ��� �� �������� ��� �� ������ ����������
			while(AIAPI::pickupNearWeapon(m_entity))
			{
				if((AIAPI::takeBestWeapon(m_entity, comparator)) ||
					(AIAPI::takeGrenade(m_entity)))
					break;
			}
			// ������� ��� ������
			AIAPI::dropUselessWeapon(m_entity, comparator);
		}
		// ��������� � ��������� ��������������
		m_turn_state = TS_INPROGRESS;
		return true;
	}
	return false;
}

//=====================================================================================//
//                       void CommonEnemyNode::SendCureMessage()                       //
//=====================================================================================//
void CommonEnemyNode::SendCureMessage(float dist)
{
	if(AIAPI::getHealthPercent(m_entity) < 50.0f)
	{
		AIAPI::print(m_entity, "���� ���� ������!");
		getSubteam()->sendMessage(this,	AIEnemyNode::MT_NEEDCURE,&m_entity,	dist);
	}
}

//=====================================================================================//
//            std::auto_ptr<Activity> CommonEnemyNode::CreateMoveActivity()            //
//=====================================================================================//
ActivityHolder CommonEnemyNode::CreateMoveActivity(const pnt_vec_t& path, unsigned int flags)
{
	return ActivityHolder(ActivityFactory::CreateMove(
			AIAPI::getPtr(m_entity),
			path,
			flags
		));
}

//=====================================================================================//
//           std::auto_ptr<Activity> CommonEnemyNode::CreateRotateActivity()           //
//=====================================================================================//
ActivityHolder CommonEnemyNode::CreateRotateActivity(float angle, unsigned int flags)
{
	return ActivityHolder(ActivityFactory::CreateRotate(
			AIAPI::getPtr(m_entity),
			angle,
			flags
		));
}

//=====================================================================================//
//                         bool CommonEnemyNode::MedkitTake()                          //
//=====================================================================================//
bool CommonEnemyNode::MedkitTake(const ipnt2_t &m_target_pnt, state_type* st)
{
	// 1. �������� �� ������� �� �� � �������
	if(m_target_pnt == AIAPI::getPos2(m_entity))
	{
		// �� ������� � ������� ������� - �������, ���� ������
		AIAPI::print(m_entity, "thinkMedkitTake: �� ������� � ������� - �������, ���� ������");
		// ������� ��� �������
		AIAPI::pickupAllNearMedikit(m_entity);
		// ��������� � ��������� �������� � ������� �����
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 2. �������� �� ��������� �� � ��� ���������
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// �� ������� ����������
		AIAPI::print(m_entity, "thinkMedkitTake: �� ������� ����������");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return true;
	}
	
	// ��������� ���� � ��������
	PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, 0, 0);
	pnt_vec_t path;
	PathUtils::CalcPath(AIAPI::getPtr(m_entity),	m_target_pnt, path);
	if(path.empty())
	{
		// � �������� ������ ������� - ������� �� ����
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	// ����� ����
	m_activity = CreateMoveActivity(path, ActivityFactory::CT_ENEMY_MOVEIGNORE);
	m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return true;
}


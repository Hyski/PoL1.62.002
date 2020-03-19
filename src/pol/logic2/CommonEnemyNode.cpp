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

	if(!st)// если не наш ход - на выход
	{
		m_turn_state = TS_NONE;
		return true;
	}

	if(UnlimitedMoves())
	{
		if(st) *st = ST_FINISHED;
		return true;
	}

	if(m_turn_state == TS_NONE)	// если m_turn_state == TS_NONE, то это начало нашего хода
	{
		m_turn_state = TS_START;
		// начнем инициализацию
		InitPanic();
		m_initialising = true;
	}
	
	if(m_initialising)// проверим флаг инициализации
	{
		if(PanicPlayer::Execute())
		{	// нужно продолжать процесс инициализации
			*st = ST_INCOMPLETE;
			return true;
		}
		// инициализаци€ закончена
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
/// проверить, нужна ли мне аптечка и вижу ли € ее
bool CommonEnemyNode::needAndSeeMedkit(ipnt2_t* target)
{
	// проверим количество аптечек у юнита
	if(AIAPI::getThingCount(m_entity, TT_MEDIKIT) >= 2)
		return false;
	// посмотрим есть ли на виду у юнита аптечки
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
		AIAPI::print(m_entity, " попробую самолечение!");
		if(AIAPI::haveMedikit(m_entity))
		{
			// у мен€ есть аптечка
			// возьму ее в руки
			AIAPI::takeMedikit(m_entity);
			// полечу себ€
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
		// мы подошли к вал€ющемус€ оружию - возьмем что сможем
		AIAPI::print(m_entity, "мы подошли к вал€ющемус€ оружию - возьмем что сможем");
		// возьмем все патроны
		AIAPI::pickupAllNearAmmo(m_entity);
		// проверим есть ли теперь у нас зар€женное оружие
		if( (!AIAPI::takeGrenade(m_entity)) &&
			(!AIAPI::takeBestWeapon(m_entity, comparator))	)
		{
			// зар€женной пушки все еще нет будем поднимать пушки по одной,
			// пока они не кончатс€ или не найдем подход€щую
			while(AIAPI::pickupNearWeapon(m_entity))
			{
				if((AIAPI::takeBestWeapon(m_entity, comparator)) ||
					(AIAPI::takeGrenade(m_entity)))
					break;
			}
			// выкинем все лишнее
			AIAPI::dropUselessWeapon(m_entity, comparator);
		}
		// переходим в состо€ние патрулировани€
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
		AIAPI::print(m_entity, "мен€ надо лечить!");
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
	// 1. проверим не подошли ли мы к аптечке
	if(m_target_pnt == AIAPI::getPos2(m_entity))
	{
		// мы подошли к лежащей аптечке - возьмем, если сможем
		AIAPI::print(m_entity, "thinkMedkitTake: мы подошли к аптечке - возьмем, если сможем");
		// возьмем все аптечки
		AIAPI::pickupAllNearMedikit(m_entity);
		// переходим в состо€ние возврата в базовую точку
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 2. проверим не кончились ли у нас мувпоинты
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// не хватило мувпоинтов
		AIAPI::print(m_entity, "thinkMedkitTake: не хватило мувпоинтов");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return true;
	}
	
	// продолжим идти к аптечкам
	PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, 0, 0);
	pnt_vec_t path;
	PathUtils::CalcPath(AIAPI::getPtr(m_entity),	m_target_pnt, path);
	if(path.empty())
	{
		// к аптечкам нельз€ подойти - возврат на базу
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	// можно идти
	m_activity = CreateMoveActivity(path, ActivityFactory::CT_ENEMY_MOVEIGNORE);
	m_activity->Attach(&m_activity_observer,ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return true;
}


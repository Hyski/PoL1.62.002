#include "logicdefs.h"

#include "AIUtils.h"
#include "HexGrid.h"
#include "PathUtils.h"
#include "SubteamNode.h"
#include "AssaultEnemyNode.h"
#include "../Common/Utils/VFileWpr.h"
#include "Entity.h"

DCTOR_IMP(AssaultEnemyNode)

AssaultEnemyNode::ThinkFn_t AssaultEnemyNode::m_modes[AEM_COUNT] = 
{
	thinkFixed,
	thinkLookround,
	thinkAttack,
	thinkWeaponSearch,
	thinkWeaponTake,
	thinkPursuit,
	thinkKilled,
	thinkMedkitTake,
	thinkSeek
};

namespace AssaultEnemyNodeDetails
{

//=====================================================================================//
//                                  class SeekerInfo                                   //
//=====================================================================================//
class SeekerInfo
{
public:
	typedef std::list<std::string> Route_t;
	typedef std::hash_map<std::string,Route_t> Routes_t;

private:
	Routes_t m_routes;

public:
	SeekerInfo()
	{
		using mll::utils::table;

		PoL::VFileWpr vfile("scripts/pol/seeker.txt");
		PoL::VFileBuf vbuf(vfile.get());
		std::istream in(&vbuf);

		table tbl;
		in >> tbl;

		for(int i = 1; i < tbl.height(); ++i)
		{
			std::string sysid = tbl.col(0)[i];
			Route_t route;

			for(int j = 1; j < tbl.width(); ++j)
			{
				std::string route_point = tbl.col(j)[i];
				if(!route_point.empty()) route.push_back(route_point);
			}

			m_routes.insert(std::make_pair(sysid,route));
		}
	}

	Route_t getRoute(const std::string &s) const
	{
		Routes_t::const_iterator i = m_routes.find(s);
		if(i == m_routes.end()) return Route_t();
		return i->second;
	}
};

//=====================================================================================//
//                                 bool getZoneCoord()                                 //
//=====================================================================================//
bool getZoneCoord(const std::string &zone, ipnt2_t *result)
{
	HexGrid *grid = HexGrid::GetInst();

	HexGrid::hg_slice hs = grid->GetSlice();    
	typedef HexGrid::const_cell_iterator Itor_t;
	
	for(Itor_t citor = grid->first_cell(); citor != grid->last_cell(); ++citor)
	{
		HexGrid::const_prop_iterator hex_prop;
		hex_prop = grid->first_prop() + citor->GetProperty();

		if(hex_prop->m_zone == zone)
		{
			*result = hs.off2pnt(citor - grid->first_cell());
			return true;
		}
	}

	return false;
}

}

using namespace AssaultEnemyNodeDetails;

//=====================================================================================//
//                        AssaultEnemyNode::AssaultEnemyNode()                         //
//=====================================================================================//
// конструктор - id существа
AssaultEnemyNode::AssaultEnemyNode(eid_t id)
:	CommonEnemyNode(id),
	m_mode(AEM_FIXED),
	m_turn(0),
	m_enemy_id(0),
	m_enemy_pnt(0, 0),
	m_shoot_pnt(0, 0),
	m_shooted(false),
	m_target_pnt(0, 0),
	m_target_dir(0),
	m_last_enemy_pnt(0, 0),
	m_prev_dir(0)
{
    GameEvMessenger::GetInst()->Attach(this, EV_SHOOT);
    GameEvMessenger::GetInst()->Attach(this, EV_KILL);

    if(!m_entity) return;

	OnSpawnMsg();
}

//=====================================================================================//
//                        AssaultEnemyNode::AssaultEnemyNode()                         //
//=====================================================================================//
AssaultEnemyNode::AssaultEnemyNode(eid_t id, const std::string &sysname)
:	CommonEnemyNode(id),
	m_mode(AEM_FIXED),
	m_turn(0),
	m_enemy_id(0),
	m_enemy_pnt(0, 0),
	m_shoot_pnt(0, 0),
	m_shooted(false),
	m_target_pnt(0, 0),
	m_target_dir(0),
	m_last_enemy_pnt(0, 0),
	m_prev_dir(0),
	m_sysname(sysname)
{
	if(!m_sysname.empty())
	{
		static SeekerInfo si;
        
		SeekerInfo::Route_t route = si.getRoute(m_sysname);

		for(SeekerInfo::Route_t::iterator i = route.begin(); i != route.end(); ++i)
		{
			ipnt2_t pos;
			if(getZoneCoord(*i,&pos))
			{
				m_ptlist.push_back(pos);
			}
		}
	}

    GameEvMessenger::GetInst()->Attach(this, EV_SHOOT);
    GameEvMessenger::GetInst()->Attach(this, EV_KILL);

    if(!m_entity) return;

	OnSpawnMsg();
}

//=====================================================================================//
//                        AssaultEnemyNode::~AssaultEnemyNode()                        //
//=====================================================================================//
AssaultEnemyNode::~AssaultEnemyNode()
{
    GameEvMessenger::GetInst()->Detach(this);
}

//=====================================================================================//
//                           float AssaultEnemyNode::Think()                           //
//=====================================================================================//
float AssaultEnemyNode::Think(state_type* st)
{
    //проиграть действие
	if(ThinkShell(st)) return 0.0f;

	if(UnlimitedMoves())
	{
		if(st) *st = ST_FINISHED;
		return 1.0f;
	}

    // юнит свободен - можно подумать
    while((this->*m_modes[m_mode])(st));

	if(HumanEntity *hum = AIAPI::getPtr(m_entity)->Cast2Human())
	{
		hum->GetEntityContext()->GetTraits()->AddMovepnts(-1);
	}
	else if(VehicleEntity *veh = AIAPI::getPtr(m_entity)->Cast2Vehicle())
	{
		hum->GetEntityContext()->GetTraits()->SetMovepnts(hum->GetEntityContext()->GetTraits()->GetMovepnts()-1);
	}

	float complexity = 0.0f;
	CheckFinished(st, &complexity);
	return complexity;
}

//=====================================================================================//
//                      int AssaultEnemyNode::getModeInt() const                       //
//=====================================================================================//
int AssaultEnemyNode::getModeInt() const
{
	return static_cast<int>(m_mode);
}

//=====================================================================================//
//                         void AssaultEnemyNode::setModeInt()                         //
//=====================================================================================//
void AssaultEnemyNode::setModeInt(int m)
{
	m_mode = static_cast<AssaultEnemyMode>(m);
}

//=====================================================================================//
//                        void AssaultEnemyNode::MakeSaveLoad()                        //
//=====================================================================================//
void AssaultEnemyNode::MakeSaveLoad(SavSlot& st)
{
    if(st.IsSaving())
	{
		// сохраняемся
        st << getModeInt();
        st << static_cast<int>(m_turn_state);
        st << static_cast<int>(m_lookround_state);
		st << m_entity;
		st << static_cast<int>(m_activity_observer.getLastEvent());
		st << m_turn;
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
		st << m_prev_dir;

		st << m_sysname;
		st << m_ptlist.size();

		for(std::list<ipnt2_t>::iterator i = m_ptlist.begin(); i != m_ptlist.end(); ++i)
		{
			st << i->x << i->y;
		}
    }
	else
	{
		// читаемся
		int tmp;
		st >> tmp; setModeInt(tmp);
		st >> tmp; m_turn_state = static_cast<TurnState>(tmp);
		st >> tmp; m_lookround_state = static_cast<LookroundState>(tmp);
		st >> m_entity;
		st >> tmp; m_activity_observer.setLastEvent(static_cast<ActivityObserver::event_type>(tmp));
		st >> m_turn;
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
		st >> m_prev_dir;

		st >> m_sysname;

		size_t size;
		st >> size;

		for(size_t i = 0; i < size; ++i)
		{
			int x,y;
			st >> x >> y;
			m_ptlist.push_back(ipnt2_t(x,y));
		}

		// то, что восстанавливается без чтения
		m_activity.Reset();
		m_initialising = false;
	}
}

//=====================================================================================//
//                       void AssaultEnemyNode::recieveMessage()                       //
//=====================================================================================//
// обработка внутрикомандных сообщений
void AssaultEnemyNode::recieveMessage(MessageType type, void * data)
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
	}
}

//=====================================================================================//
//                           void AssaultEnemyNode::Update()                           //
//=====================================================================================//
void AssaultEnemyNode::Update(subject_t subj, event_t event, info_t ptr)
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
//                          void AssaultEnemyNode::OnKillEv()                          //
//=====================================================================================//
//обработка собщения об убийстве
void AssaultEnemyNode::OnKillEv(BaseEntity* killer, BaseEntity* victim)
{
	// почистим ссылки на людей
	if(victim == AIAPI::getPtr(m_enemy_id)) m_enemy_id = 0;
}

//=====================================================================================//
//                         void AssaultEnemyNode::OnShootEv()                          //
//=====================================================================================//
// обработка сообщения об выстреле или попадании
void AssaultEnemyNode::OnShootEv(BaseEntity* who, BaseEntity* victim, const point3& where)
{
	if(!m_entity) return;

	// не будем реагировать в случае,
	// если юнит находится в определенных состояниях
	switch(m_mode)
	{
	case AEM_ATTACK: case AEM_WEAPONSEARCH: case AEM_WEAPONTAKE : return;
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
//                       bool AssaultEnemyNode::switchToAttack()                       //
//=====================================================================================//
bool AssaultEnemyNode::switchToAttack(eid_t enemy)
{
	int index = 0;
	const shot_type stypes[] = {SHT_AIMSHOT,SHT_AUTOSHOT,SHT_SNAPSHOT};
	const ActivityFactory::controller_type ctrlType = ActivityFactory::CT_ENEMY_MOVE;

	m_mode = AEM_ATTACK;
	m_turn_state = TS_INPROGRESS;

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
			m_target_pnt = AIAPI::getBestPoint(vec,	AssaultHexComparator(m_entity, enemy));

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
//                        bool AssaultEnemyNode::comeToWeapon()                        //
//=====================================================================================//
bool AssaultEnemyNode::comeToWeapon()
{
	pnt_vec_t base, vec, path;

	// недалеко есть оружие - переход к его захвату
	AIAPI::print(m_entity, "|A|thinkWeaponSearch: недалеко есть оружие - переход к его захвату!");
	AIAPI::getBaseField(m_target_pnt, 1, &base);
	AIAPI::getReachableField(base, &vec, true);

	m_target_pnt = AIAPI::getRandomPoint(vec);
	PathUtils::CalcPassField(AIAPI::getPtr(m_entity), 0, 0, 0);
	PathUtils::CalcPath(AIAPI::getPtr(m_entity), m_target_pnt, path);
	if(path.empty()) return false;

	// можно идти
	m_activity = CreateMoveActivity(path, ActivityFactory::CT_ENEMY_MOVEIGNORE);
	m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	m_mode = AEM_WEAPONTAKE;
	return true;
}

//=====================================================================================//
//                      bool AssaultEnemyNode::processShootInMe()                      //
//=====================================================================================//
bool AssaultEnemyNode::processShootInMe()
{
	if(!m_shooted) return false;

	AIAPI::print(m_entity, "|A|thinkLookRound: в меня стреляли - будем преследовать");
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
	PathUtils::CalcPath(AIAPI::getPtr(m_entity),m_target_pnt, path);
	// можно идти
	AIAPI::setRun(m_entity);
	m_activity = CreateMoveActivity(path, ActivityFactory::CT_ENEMY_MOVE);
	m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);

	m_mode = AEM_PURSUIT;
	m_turn_state = TS_INPROGRESS;
	return true;
}

//=====================================================================================//
//                    bool AssaultEnemyNode::processEnemySpotted()                     //
//=====================================================================================//
bool AssaultEnemyNode::processEnemySpotted()
{
	if(!m_enemy_id) return false;

	AIAPI::print(m_entity, "|A|thinkLookRound: мне сообщили о враге");
	// сообщили
	m_enemy_id = 0;
	// выберем точку в радиусе 1 хекса от врага
	// пойдем туда и перейдем при этом в состояние преследования

	// рассчитаем поле проходимости
	PathUtils::CalcPassField(AIAPI::getPtr(m_entity), 0, 0, 0);
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
	AIAPI::setRun(m_entity);
	m_activity = CreateMoveActivity(path, ActivityFactory::CT_ENEMY_MOVE);
	m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);

	m_mode = AEM_PURSUIT;
	m_turn_state = TS_INPROGRESS;
	return true;
}

//=====================================================================================//
//                         bool AssaultEnemyNode::tryToSeek()                          //
//=====================================================================================//
bool AssaultEnemyNode::tryToSeek()
{
	// рассчитаем поле проходимости
	PathUtils::CalcPassField(AIAPI::getPtr(m_entity), 0, 0, 0);
	pnt_vec_t field;
	AIAPI::getBaseField(m_ptlist.front(), 3, &field);

	if(field.empty()) return false;

	// получим вектор доступных хексов
	pnt_vec_t vec;
	AIAPI::getReachableField(field, &vec, true);

	if(vec.empty()) return false;

	// получим пока случайный хекс
	m_target_pnt = AIAPI::getRandomPoint(vec);
	pnt_vec_t path;
	PathUtils::CalcPath(AIAPI::getPtr(m_entity), m_target_pnt, path);

	if(path.empty()) return false;

	// можно идти
	if(aiNormRand() < 0.5f)
	{
		AIAPI::setRun(m_entity);
	}
	else
	{
		AIAPI::setWalk(m_entity);
	}

	m_activity_observer.clear();
	m_activity = CreateMoveActivity(path, ActivityFactory::CT_ENEMY_MOVE);
	m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);

	m_mode = AEM_SEEK;
	m_turn_state = TS_INPROGRESS;
	return true;
}

//=====================================================================================//
//                         bool AssaultEnemyNode::thinkFixed()                         //
//=====================================================================================//
// обработка режима Fixed
bool AssaultEnemyNode::thinkFixed(state_type* st)
{
	// 1. проверим свое здоровье, если оно < половины - пошлем сообщение
	SendCureMessage(fixed_cure_dist);

	// 2. если у нас нет оружия - перейдем к его поиску
	if( (!AIAPI::takeBestWeapon(m_entity, AssaultWeaponComparator())) &&
		(!AIAPI::takeGrenade(m_entity))
		)
	{
		AIAPI::print(m_entity, "|A|thinkFixed: надо искать пушку!");
		m_mode = AEM_WEAPONSEARCH;
		m_turn_state = TS_INPROGRESS;
		m_turn = 0;
		return false;
	}
	
	// 3. если я вижу врага - пошлю сообщение и перейду в атаку
	AIAPI::entity_list_t enemies;
	if(AIAPI::getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// я кого-то вижу
		// gvozdoder: здесь в оригинале стояло условие на нулевой enemy - оно не возможно
		// ai-todo: здесь получить ближайшего врага
		eid_t enemy = AIAPI::getMostDangerousEnemy(enemies,EnemyComparator(m_entity));
		assert( enemy );

		AIAPI::print(m_entity, "|A|thinkFixed: я увидел врага!");
		// выбрал самого опасного врага
		// пошлю сообщение своей команде
		getSubteam()->sendMessage(this,
			AIEnemyNode::MT_ENEMYSPOTTED,
			&enemy,
			assault_enemy_dist);
		// переход в состояние атаки
		m_turn = 0;

		if(switchToAttack(enemy))
		{
			return false;
		}
	}

	// 3.5 если я должен следовать к определенной точке, то побегу к ней
	if(!m_ptlist.empty())
	{
		if(tryToSeek()) return false;
	}

	// 4. Если в меня стреляли - будем преследовать
	if(processShootInMe())
	{
		m_turn = 0;
		return false;
	}

	// 5. если мне сообщили о враге - пойдем к нему
	if(processEnemySpotted())
	{
		m_turn = 0;
		return false;
	}

	// 6. если здоровье меньше максимума - попробуем себя полечить
	SelfCure();

	// 7. если вижу аптечку и она мне нужна - побегу за ней
	if(needAndSeeMedkit(&m_target_pnt))
	{
		PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, 0, 0);
		pnt_vec_t path;
		PathUtils::CalcPath(AIAPI::getPtr(m_entity),	m_target_pnt, path);
		if(!path.empty())
		{
			// можно идти
			m_activity = CreateMoveActivity(path, ActivityFactory::CT_ENEMY_MOVEIGNORE);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_mode = AEM_MEDKITTAKE;
			m_turn_state = TS_INPROGRESS;
			return false;
		}
	}

	// 8. стало быть врагов не видно и если время оборачиваться - обернусь
	if(++m_turn < assault_turns)
	{
		AIAPI::print(m_entity, "|A|thinkFixed: еще не время обернуться");
		// еще не время оборачиваться
		m_turn_state = TS_END;
		return false;
	}
	// пора обернуться
	AIAPI::print(m_entity, "|A|thinkFixed: пора оборачиваться");
	m_turn = 0;
	m_turn_state = TS_INPROGRESS;
	m_mode = AEM_LOOKROUND;
	m_lookround_state = LS_FIRST;
	m_prev_dir = norm_angle(AIAPI::getAngle(m_entity));
	m_target_dir = norm_angle(m_prev_dir + 2*PId3);
	m_activity = CreateRotateActivity(m_target_dir, ActivityFactory::CT_ENEMY_LOOKROUND);
	m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_SPOTTED);
	return false;
}

//=====================================================================================//
//                         bool AssaultEnemyNode::thinkSeek()                          //
//=====================================================================================//
bool AssaultEnemyNode::thinkSeek(state_type *st)
{
	m_mode = AEM_FIXED;

	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_NONE)
	{
		if(!m_ptlist.empty()) m_ptlist.erase(m_ptlist.begin());
	}
	else if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		m_turn_state = TS_END;
		return false;
	}

	return true;
}

//=====================================================================================//
//                       bool AssaultEnemyNode::thinkLookround()                       //
//=====================================================================================//
// обработка режима Lookround
bool AssaultEnemyNode::thinkLookround(state_type* st)
{
	// 1. проверим свое здоровье, если оно < половины - пошлем сообщение
	SendCureMessage(assault_cure_dist);

	// 2. если я вижу врага - пошлю сообщение и перейду в атаку
	AIAPI::entity_list_t enemies;
	if(AIAPI::getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// я кого-то вижу
		if(eid_t enemy = AIAPI::getMostDangerousEnemy(enemies,EnemyComparator(m_entity))	)
		{
			AIAPI::print(m_entity, "|A|thinkLookRound: я увидел врага!");
			// выбрал самого опасного врага
			// пошлю сообщение своей команде
			getSubteam()->sendMessage(this,	AIEnemyNode::MT_ENEMYSPOTTED,	&enemy,	assault_enemy_dist);

			// переход в состояние атаки
			if(switchToAttack(enemy))
			{
				return false;
			}
		}
	}

	if(!m_ptlist.empty())
	{
		if(tryToSeek()) return false;
	}

	// 3. Если в меня стреляли - будем преследовать
	if(processShootInMe()) return false;

	// 4. если мне сообщили о враге - пойдем к нему
	if(processEnemySpotted()) return false;

	// 5. получим текущий угол, на который повернуто существо
	float cur_angle = norm_angle(AIAPI::getAngle(m_entity));
	if(fabs(m_target_dir - cur_angle) < angle_eps)
	{
		// закончен очередной прием разворота
		switch(m_lookround_state)
		{
		case LS_FIRST : 
			{
				AIAPI::print(m_entity, "|A|thinkLookRound: закончен первый прием разворота");
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
				AIAPI::print(m_entity, "|A|thinkLookRound: закончен второй прием разворота");
				// начинаем заключительный прием разворота
				m_turn_state = TS_INPROGRESS;
				m_lookround_state = LS_2BASE;
				m_target_dir = m_prev_dir;
				m_activity = CreateRotateActivity(m_target_dir, ActivityFactory::CT_ENEMY_LOOKROUND);
				m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
				m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_SPOTTED);
				break;
			}
		case LS_2BASE : 
			{
				AIAPI::print(m_entity, "|A|thinkLookRound: полностью закончил разворот");
				// переходим в базовое состояние
				m_mode = AEM_FIXED;
				m_turn_state = TS_END;
				break;
			}
		default : break;
		}
		return false;
	}

	// 6. если не хватило мувпоинтов - отдадим ход
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		AIAPI::print(m_entity, "|A|thinkLookRound: не хватило мувпоинтов");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// продолжим действие
	m_activity = CreateRotateActivity(m_target_dir,ActivityFactory::CT_ENEMY_LOOKROUND);
	
	m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_SPOTTED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

//=====================================================================================//
//                        bool AssaultEnemyNode::thinkAttack()                         //
//=====================================================================================//
// обработка режима Attack
bool AssaultEnemyNode::thinkAttack(state_type* st)
{
	// 1. проверим свое здоровье, если оно < половины - пошлем сообщение
	SendCureMessage(assault_cure_dist);

	// 2. если у нас нет оружия - перейдем к его поиску
	if( (!AIAPI::takeBestWeapon(m_entity, AssaultWeaponComparator())) &&
		(!AIAPI::takeGrenade(m_entity))
		)
	{
		AIAPI::print(m_entity, "|A|thinkAttack: надо искать пушку!");
		m_mode = AEM_WEAPONSEARCH;
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
		}
		// попробуем найти разность списков всех врагов и игнорированных врагов
		EntityList_t diff_list;
		std::set_difference(
			enemies.begin(),enemies.end(),
			m_ignored_enemies.begin(),m_ignored_enemies.end(),
			std::inserter(diff_list,diff_list.end()));

		if(diff_list.empty())
		{
			// выберем самого опасного врага из всех
			enemy = AIAPI::getMostDangerousEnemy(enemies, EnemyComparator(m_entity));
		}
		else
		{
			// выберем самого опасного врага из непроигнорированных
			enemy = AIAPI::getMostDangerousEnemy(diff_list,	EnemyComparator(m_entity));
		}
		m_last_enemy_pnt = AIAPI::getPos2(enemy);
	}
	
	if(!m_ptlist.empty())
	{
		if(tryToSeek()) return false;
	}

	// 4. если нет врагов - переходим в состояние преследования
	if(!enemy)
	{
		// врагов нет - будем преследовать последнего
		AIAPI::print(m_entity, "|A|thinkAttack: врагов нет - будем преследовать последнего");
		m_mode = AEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		m_target_pnt = m_last_enemy_pnt;
		m_activity_observer.clear();
		AIAPI::setStandPose(m_entity);
		return false;
	}
	// враги есть...

	// 5. если это начало нового хода или сработала реакция
	if(m_turn_state == TS_START)
	{
		if(switchToAttack(enemy))
		{
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_SPOTTED);
			return false;
		}
	}

	// 6. это продолжение хода

	// 6.1 если юниту не хватило мувпоинтов - он передает ход
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// закончились - закончим ход
		AIAPI::print(m_entity, "|A|thinkAttack: закончились мувпоинты");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 6.2 если сработала реакция или увидел нового врага - имитируем
	//       начало хода
	if( (m_activity_observer.getLastEvent() == ActivityObserver::EV_ENEMY_REACTED) ||
		(m_activity_observer.getLastEvent() == ActivityObserver::EV_ENEMY_SPOTTED)	)
	{
		AIAPI::print(m_entity, "|A|thinkAttack: сработала реакция или увидел нового врага");
		m_activity_observer.clear();
		// имитируем начало хода
		m_ignored_enemies.clear();
		m_turn_state = TS_START;
		return true;
	}

	// 6.3 если нет линии огня - добавляем врага в список игнорирования
	// причем, если после этого все враги оказались в списке игнорирования
	// попробуем подойти к одному из них
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_NO_LOF)
	{
		// закончились - закончим ход
		AIAPI::print(m_entity, "|A|thinkAttack: нет линии огня - добавляем врага в список игнорирования");
		m_activity_observer.clear();

		// добавим в список игнорирования
		m_ignored_enemies.insert(enemy);
		if(std::includes(m_ignored_enemies.begin(),m_ignored_enemies.end(),enemies.begin(),enemies.end()))
		{
			// попробуем подойти поближе к врагу
			AIAPI::print(m_entity, "|A|thinkAttack: подойдем поближе");
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
			m_activity = CreateMoveActivity(path, ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_REACTED);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_SPOTTED);
			
			m_turn_state = TS_INPROGRESS;
			return false;
		} // if(diff_list.empty())
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
		m_activity = CreateRotateActivity(m_target_dir, ActivityFactory::CT_ENEMY_ROTATE);
		m_activity->Attach(&m_activity_observer,
			ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	
	// 6.5 повернут к врагу - если сможем будем атаковать
	AIAPI::print(m_entity, "|A|thinkAttack: будем стрелять, если это возможно!");
	// враг есть - выстрелим по нему если это возможно
	// присядем в вероятностью 60%
	if(aiNormRand() < 0.6f)
	{
		AIAPI::setSitPose(m_entity);
	}
	else
	{
		AIAPI::setStandPose(m_entity);
	}

	// если есть граната и можно атаковать несколько врагов сразу -
	// попробуем кинуть ее
	bool have_ready_gun = AIAPI::takeBestWeapon(m_entity,
		AssaultWeaponComparator());
	if(AIAPI::takeGrenade(m_entity) &&
		
		( (!have_ready_gun)  ||  
		AIAPI::canAttackByGrenadeMultipleTarget(m_entity,
		enemy,
		enemies)
		)
		)
	{
		// граната в руках - попробуем кинуть ее
		std::string reason;
		if(AIAPI::shoot(m_entity, enemy, &m_activity, &reason))
		{
			// начали бросок
			m_turn_state = TS_INPROGRESS;
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_NO_LOF);
			return false;
		}

		// кинуть не удалось
		std::string str = "|A|thinkAttack: нельзя кинуть гранату: ";
		str += reason;
		AIAPI::print(m_entity, str.c_str());
		m_turn_state = TS_END;
	}

	// теперь попробуем стрельнуть из оружия
	if(AIAPI::takeBestWeapon(m_entity, AssaultWeaponComparator()))
	{
		// оружие в руках
		AIAPI::setShootType(m_entity, 30.0f, 15.0f);
		if(AIAPI::getMovePoints(m_entity) <= AIAPI::getShootMovePoints(m_entity,AIAPI::getPtr(m_entity)->Cast2Human()->GetEntityContext()->GetShotType()))
		{
			AIAPI::getPtr(m_entity)->Cast2Human()->GetEntityContext()->SetShotType(SHT_SNAPSHOT);
		}
		std::string reason;
		if(AIAPI::shoot(m_entity, enemy, &m_activity, &reason))
		{
			// стрельба началась
			m_turn_state = TS_INPROGRESS;
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_NO_LOF);
			return false;
		}
		if(AIUtils::CalcEfficiency(AIAPI::getPtr(m_entity),AIAPI::getPos3(enemy)) == 0)
		{
			// Нулевая эффективность -- имитируем начало хода, если осталось больше 10 мувпойнтов
			if(AIAPI::getSteps(m_entity,AIAPI::getMovePoints(m_entity)) > 2)
			{
				m_turn_state = TS_START;
				return false;
			}
		}
		// стрельнуть не удалось
		std::string str = "|A|thinkAttack: стрелять нельзя: ";
		str += reason;
		AIAPI::print(m_entity, str.c_str());
		m_turn_state = TS_END;
	}
	m_turn_state = TS_END;
	return false;
}

//=====================================================================================//
//                     bool AssaultEnemyNode::thinkWeaponSearch()                      //
//=====================================================================================//
// обработка режима WeaponSearch
bool AssaultEnemyNode::thinkWeaponSearch(state_type* st)
{
	// 1. проверим свое здоровье, если оно < половины - пошлем сообщение
	SendCureMessage(assault_cure_dist);

	// 2. проверим есть ли где-нить рядом оружие
	if(AIAPI::getWeaponLocation(m_entity, &m_target_pnt))
	{
		if(!comeToWeapon()) m_turn_state = TS_END;
		return false;
	}

	if(!m_ptlist.empty())
	{
		if(tryToSeek()) return false;
	}

	// 3. если здоровье меньше максимума - попробуем себя полечить
	SelfCure();

	// 4. если дошел до заданной точки - выбирает новую
	if(m_target_pnt == AIAPI::getPos2(m_entity))
	{
		AIAPI::print(m_entity, "|A|thinkWeaponSearch: дошел до заданной точки - выбирает новую");
		// рассчитаем поле проходимости
		pnt_vec_t allhexes;
		PathUtils::CalcPassField(AIAPI::getPtr(m_entity),
			0, &allhexes, 0);
		if(allhexes.empty())
		{
			// передаем ход - идти некуда
			AIAPI::print(m_entity, "|A|thinkWeaponSearch: мне некуда ходить!!!");
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
		AIAPI::print(m_entity, "|A|thinkWeaponSearch: не хватило мувпоинтов");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 6. идем в заданную точку
	AIAPI::print(m_entity, "|A|thinkWeaponSearch: идем в заданную точку");
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
		return true;
	}
	// можно идти
	m_activity = CreateMoveActivity(path, ActivityFactory::CT_ENEMY_MOVE);
	m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

//=====================================================================================//
//                      bool AssaultEnemyNode::thinkWeaponTake()                       //
//=====================================================================================//
// обработка режима WeaponTake
bool AssaultEnemyNode::thinkWeaponTake(state_type* st)
{
	// 1. проверим свое здоровье, если оно < половины - пошлем сообщение
	SendCureMessage(assault_cure_dist);

	// 2. проверим не подошли ли мы к оружию
	if(PickUpWeaponAndAmmo(m_target_pnt, AssaultWeaponComparator()))
	{
		m_mode = AEM_FIXED;
		return false;
	}

	// 3. проверим не кончились ли у нас мувпоинты
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// не хватило мувпоинтов
		AIAPI::print(m_entity, "|A|thinkWeaponTake: не хватило мувпоинтов");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 4. проверим есть ли где-нить рядом оружие
	if(AIAPI::getWeaponLocation(m_entity, &m_target_pnt))
	{
		if(!comeToWeapon())
		{
			m_mode = AEM_FIXED;
			m_turn_state = TS_INPROGRESS;
		}
		return false;
	}

	// оружия нет - возврат к стационарному состоянию
	m_mode = AEM_FIXED;
	m_turn_state = TS_INPROGRESS;
	return false;
}

//=====================================================================================//
//                        bool AssaultEnemyNode::thinkPursuit()                        //
//=====================================================================================//
// обработка режима Pursuit
bool AssaultEnemyNode::thinkPursuit(state_type* st)
{
	// 1. проверим свое здоровье, если оно < половины - пошлем сообщение
	SendCureMessage(assault_cure_dist);

	// 2. если я вижу врага - пошлю сообщение и перейду в атаку
	AIAPI::entity_list_t enemies;
	if(AIAPI::getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// я кого-то вижу
		if(eid_t enemy = AIAPI::getMostDangerousEnemy(enemies,EnemyComparator(m_entity))	)
		{
			AIAPI::print(m_entity, "|A|thinkPursuit: я увидел врага!");

			// выбрал самого опасного врага
			// пошлю сообщение своей команде
			getSubteam()->sendMessage(this,	AIEnemyNode::MT_ENEMYSPOTTED,&enemy,assault_enemy_dist);

			// переход в состояние атаки
			AIAPI::setWalk(m_entity);

			switchToAttack(enemy);
			return false;
		}
	}

	if(!m_ptlist.empty())
	{
		if(tryToSeek()) return false;
	}


	// 3. проверим не кончились ли у нас мувпоинты
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// не хватило мувпоинтов
		AIAPI::print(m_entity, "|A|thinkPursuit: не хватило мувпоинтов");
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
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_turn_state = TS_INPROGRESS;
			return false;
		}
		// 4.2 повернут - переход стационарное состояние
		AIAPI::print(m_entity, "|A|thinkPursuit: никого не догнал - переход стационарное состояние");
		// переходим в стационарное состояние
		m_mode = AEM_FIXED;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 5. идем в заданную точку
	AIAPI::print(m_entity, "|A|thinkPursuit: идем в заданную точку");
	PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, 0, 0);
	pnt_vec_t path;
	PathUtils::CalcPath(AIAPI::getPtr(m_entity), m_target_pnt, path);
	if(path.empty())
	{
		// точка оказалась недостижимой - будем считать, что уже пришли
		// куда надо
		m_target_pnt = AIAPI::getPos2(m_entity);
		m_turn_state = TS_INPROGRESS;
		return true;
	}
	// можно идти
	AIAPI::setRun(m_entity);
	m_activity = CreateMoveActivity(path, ActivityFactory::CT_ENEMY_MOVE);
	m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

//=====================================================================================//
//                      bool AssaultEnemyNode::thinkMedkitTake()                       //
//=====================================================================================//
// обработка режима MedkitTake
bool AssaultEnemyNode::thinkMedkitTake(state_type* st)
{
	if(!MedkitTake(m_target_pnt,st)) m_mode = AEM_FIXED;
	return false;
}

//=====================================================================================//
//                            bool AssaultEnemyNode::die()                             //
//=====================================================================================//
// умри!
bool AssaultEnemyNode::die()
{
	m_mode = AEM_KILLED;
	return need2Delete();
}

//=====================================================================================//
//                     bool AssaultEnemyNode::need2Delete() const                      //
//=====================================================================================//
// нужно ли удалять узел
bool AssaultEnemyNode::need2Delete() const
{
	if( (m_mode == AEM_KILLED) && !m_activity) return true;
	return false;
}
// 1050
// 1079
// 1112
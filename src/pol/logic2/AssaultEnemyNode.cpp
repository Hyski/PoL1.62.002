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
// ����������� - id ��������
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
    //��������� ��������
	if(ThinkShell(st)) return 0.0f;

	if(UnlimitedMoves())
	{
		if(st) *st = ST_FINISHED;
		return 1.0f;
	}

    // ���� �������� - ����� ��������
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
		// �����������
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
		// ��������
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

		// ��, ��� ����������������� ��� ������
		m_activity.Reset();
		m_initialising = false;
	}
}

//=====================================================================================//
//                       void AssaultEnemyNode::recieveMessage()                       //
//=====================================================================================//
// ��������� ��������������� ���������
void AssaultEnemyNode::recieveMessage(MessageType type, void * data)
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
//��������� �������� �� ��������
void AssaultEnemyNode::OnKillEv(BaseEntity* killer, BaseEntity* victim)
{
	// �������� ������ �� �����
	if(victim == AIAPI::getPtr(m_enemy_id)) m_enemy_id = 0;
}

//=====================================================================================//
//                         void AssaultEnemyNode::OnShootEv()                          //
//=====================================================================================//
// ��������� ��������� �� �������� ��� ���������
void AssaultEnemyNode::OnShootEv(BaseEntity* who, BaseEntity* victim, const point3& where)
{
	if(!m_entity) return;

	// �� ����� ����������� � ������,
	// ���� ���� ��������� � ������������ ����������
	switch(m_mode)
	{
	case AEM_ATTACK: case AEM_WEAPONSEARCH: case AEM_WEAPONTAKE : return;
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
//                       bool AssaultEnemyNode::switchToAttack()                       //
//=====================================================================================//
bool AssaultEnemyNode::switchToAttack(eid_t enemy)
{
	int index = 0;
	const shot_type stypes[] = {SHT_AIMSHOT,SHT_AUTOSHOT,SHT_SNAPSHOT};
	const ActivityFactory::controller_type ctrlType = ActivityFactory::CT_ENEMY_MOVE;

	m_mode = AEM_ATTACK;
	m_turn_state = TS_INPROGRESS;

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
//                        bool AssaultEnemyNode::comeToWeapon()                        //
//=====================================================================================//
bool AssaultEnemyNode::comeToWeapon()
{
	pnt_vec_t base, vec, path;

	// �������� ���� ������ - ������� � ��� �������
	AIAPI::print(m_entity, "|A|thinkWeaponSearch: �������� ���� ������ - ������� � ��� �������!");
	AIAPI::getBaseField(m_target_pnt, 1, &base);
	AIAPI::getReachableField(base, &vec, true);

	m_target_pnt = AIAPI::getRandomPoint(vec);
	PathUtils::CalcPassField(AIAPI::getPtr(m_entity), 0, 0, 0);
	PathUtils::CalcPath(AIAPI::getPtr(m_entity), m_target_pnt, path);
	if(path.empty()) return false;

	// ����� ����
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

	AIAPI::print(m_entity, "|A|thinkLookRound: � ���� �������� - ����� ������������");
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
	PathUtils::CalcPath(AIAPI::getPtr(m_entity),m_target_pnt, path);
	// ����� ����
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

	AIAPI::print(m_entity, "|A|thinkLookRound: ��� �������� � �����");
	// ��������
	m_enemy_id = 0;
	// ������� ����� � ������� 1 ����� �� �����
	// ������ ���� � �������� ��� ���� � ��������� �������������

	// ���������� ���� ������������
	PathUtils::CalcPassField(AIAPI::getPtr(m_entity), 0, 0, 0);
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
	// ���������� ���� ������������
	PathUtils::CalcPassField(AIAPI::getPtr(m_entity), 0, 0, 0);
	pnt_vec_t field;
	AIAPI::getBaseField(m_ptlist.front(), 3, &field);

	if(field.empty()) return false;

	// ������� ������ ��������� ������
	pnt_vec_t vec;
	AIAPI::getReachableField(field, &vec, true);

	if(vec.empty()) return false;

	// ������� ���� ��������� ����
	m_target_pnt = AIAPI::getRandomPoint(vec);
	pnt_vec_t path;
	PathUtils::CalcPath(AIAPI::getPtr(m_entity), m_target_pnt, path);

	if(path.empty()) return false;

	// ����� ����
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
// ��������� ������ Fixed
bool AssaultEnemyNode::thinkFixed(state_type* st)
{
	// 1. �������� ���� ��������, ���� ��� < �������� - ������ ���������
	SendCureMessage(fixed_cure_dist);

	// 2. ���� � ��� ��� ������ - �������� � ��� ������
	if( (!AIAPI::takeBestWeapon(m_entity, AssaultWeaponComparator())) &&
		(!AIAPI::takeGrenade(m_entity))
		)
	{
		AIAPI::print(m_entity, "|A|thinkFixed: ���� ������ �����!");
		m_mode = AEM_WEAPONSEARCH;
		m_turn_state = TS_INPROGRESS;
		m_turn = 0;
		return false;
	}
	
	// 3. ���� � ���� ����� - ����� ��������� � ������� � �����
	AIAPI::entity_list_t enemies;
	if(AIAPI::getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// � ����-�� ����
		// gvozdoder: ����� � ��������� ������ ������� �� ������� enemy - ��� �� ��������
		// ai-todo: ����� �������� ���������� �����
		eid_t enemy = AIAPI::getMostDangerousEnemy(enemies,EnemyComparator(m_entity));
		assert( enemy );

		AIAPI::print(m_entity, "|A|thinkFixed: � ������ �����!");
		// ������ ������ �������� �����
		// ����� ��������� ����� �������
		getSubteam()->sendMessage(this,
			AIEnemyNode::MT_ENEMYSPOTTED,
			&enemy,
			assault_enemy_dist);
		// ������� � ��������� �����
		m_turn = 0;

		if(switchToAttack(enemy))
		{
			return false;
		}
	}

	// 3.5 ���� � ������ ��������� � ������������ �����, �� ������ � ���
	if(!m_ptlist.empty())
	{
		if(tryToSeek()) return false;
	}

	// 4. ���� � ���� �������� - ����� ������������
	if(processShootInMe())
	{
		m_turn = 0;
		return false;
	}

	// 5. ���� ��� �������� � ����� - ������ � ����
	if(processEnemySpotted())
	{
		m_turn = 0;
		return false;
	}

	// 6. ���� �������� ������ ��������� - ��������� ���� ��������
	SelfCure();

	// 7. ���� ���� ������� � ��� ��� ����� - ������ �� ���
	if(needAndSeeMedkit(&m_target_pnt))
	{
		PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, 0, 0);
		pnt_vec_t path;
		PathUtils::CalcPath(AIAPI::getPtr(m_entity),	m_target_pnt, path);
		if(!path.empty())
		{
			// ����� ����
			m_activity = CreateMoveActivity(path, ActivityFactory::CT_ENEMY_MOVEIGNORE);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_mode = AEM_MEDKITTAKE;
			m_turn_state = TS_INPROGRESS;
			return false;
		}
	}

	// 8. ����� ���� ������ �� ����� � ���� ����� ������������� - ��������
	if(++m_turn < assault_turns)
	{
		AIAPI::print(m_entity, "|A|thinkFixed: ��� �� ����� ����������");
		// ��� �� ����� �������������
		m_turn_state = TS_END;
		return false;
	}
	// ���� ����������
	AIAPI::print(m_entity, "|A|thinkFixed: ���� �������������");
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
// ��������� ������ Lookround
bool AssaultEnemyNode::thinkLookround(state_type* st)
{
	// 1. �������� ���� ��������, ���� ��� < �������� - ������ ���������
	SendCureMessage(assault_cure_dist);

	// 2. ���� � ���� ����� - ����� ��������� � ������� � �����
	AIAPI::entity_list_t enemies;
	if(AIAPI::getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// � ����-�� ����
		if(eid_t enemy = AIAPI::getMostDangerousEnemy(enemies,EnemyComparator(m_entity))	)
		{
			AIAPI::print(m_entity, "|A|thinkLookRound: � ������ �����!");
			// ������ ������ �������� �����
			// ����� ��������� ����� �������
			getSubteam()->sendMessage(this,	AIEnemyNode::MT_ENEMYSPOTTED,	&enemy,	assault_enemy_dist);

			// ������� � ��������� �����
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

	// 3. ���� � ���� �������� - ����� ������������
	if(processShootInMe()) return false;

	// 4. ���� ��� �������� � ����� - ������ � ����
	if(processEnemySpotted()) return false;

	// 5. ������� ������� ����, �� ������� ��������� ��������
	float cur_angle = norm_angle(AIAPI::getAngle(m_entity));
	if(fabs(m_target_dir - cur_angle) < angle_eps)
	{
		// �������� ��������� ����� ���������
		switch(m_lookround_state)
		{
		case LS_FIRST : 
			{
				AIAPI::print(m_entity, "|A|thinkLookRound: �������� ������ ����� ���������");
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
				AIAPI::print(m_entity, "|A|thinkLookRound: �������� ������ ����� ���������");
				// �������� �������������� ����� ���������
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
				AIAPI::print(m_entity, "|A|thinkLookRound: ��������� �������� ��������");
				// ��������� � ������� ���������
				m_mode = AEM_FIXED;
				m_turn_state = TS_END;
				break;
			}
		default : break;
		}
		return false;
	}

	// 6. ���� �� ������� ���������� - ������� ���
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		AIAPI::print(m_entity, "|A|thinkLookRound: �� ������� ����������");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// ��������� ��������
	m_activity = CreateRotateActivity(m_target_dir,ActivityFactory::CT_ENEMY_LOOKROUND);
	
	m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_SPOTTED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

//=====================================================================================//
//                        bool AssaultEnemyNode::thinkAttack()                         //
//=====================================================================================//
// ��������� ������ Attack
bool AssaultEnemyNode::thinkAttack(state_type* st)
{
	// 1. �������� ���� ��������, ���� ��� < �������� - ������ ���������
	SendCureMessage(assault_cure_dist);

	// 2. ���� � ��� ��� ������ - �������� � ��� ������
	if( (!AIAPI::takeBestWeapon(m_entity, AssaultWeaponComparator())) &&
		(!AIAPI::takeGrenade(m_entity))
		)
	{
		AIAPI::print(m_entity, "|A|thinkAttack: ���� ������ �����!");
		m_mode = AEM_WEAPONSEARCH;
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
		}
		// ��������� ����� �������� ������� ���� ������ � �������������� ������
		EntityList_t diff_list;
		std::set_difference(
			enemies.begin(),enemies.end(),
			m_ignored_enemies.begin(),m_ignored_enemies.end(),
			std::inserter(diff_list,diff_list.end()));

		if(diff_list.empty())
		{
			// ������� ������ �������� ����� �� ����
			enemy = AIAPI::getMostDangerousEnemy(enemies, EnemyComparator(m_entity));
		}
		else
		{
			// ������� ������ �������� ����� �� �������������������
			enemy = AIAPI::getMostDangerousEnemy(diff_list,	EnemyComparator(m_entity));
		}
		m_last_enemy_pnt = AIAPI::getPos2(enemy);
	}
	
	if(!m_ptlist.empty())
	{
		if(tryToSeek()) return false;
	}

	// 4. ���� ��� ������ - ��������� � ��������� �������������
	if(!enemy)
	{
		// ������ ��� - ����� ������������ ����������
		AIAPI::print(m_entity, "|A|thinkAttack: ������ ��� - ����� ������������ ����������");
		m_mode = AEM_PURSUIT;
		m_turn_state = TS_INPROGRESS;
		m_target_pnt = m_last_enemy_pnt;
		m_activity_observer.clear();
		AIAPI::setStandPose(m_entity);
		return false;
	}
	// ����� ����...

	// 5. ���� ��� ������ ������ ���� ��� ��������� �������
	if(m_turn_state == TS_START)
	{
		if(switchToAttack(enemy))
		{
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_SPOTTED);
			return false;
		}
	}

	// 6. ��� ����������� ����

	// 6.1 ���� ����� �� ������� ���������� - �� �������� ���
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// ����������� - �������� ���
		AIAPI::print(m_entity, "|A|thinkAttack: ����������� ���������");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 6.2 ���� ��������� ������� ��� ������ ������ ����� - ���������
	//       ������ ����
	if( (m_activity_observer.getLastEvent() == ActivityObserver::EV_ENEMY_REACTED) ||
		(m_activity_observer.getLastEvent() == ActivityObserver::EV_ENEMY_SPOTTED)	)
	{
		AIAPI::print(m_entity, "|A|thinkAttack: ��������� ������� ��� ������ ������ �����");
		m_activity_observer.clear();
		// ��������� ������ ����
		m_ignored_enemies.clear();
		m_turn_state = TS_START;
		return true;
	}

	// 6.3 ���� ��� ����� ���� - ��������� ����� � ������ �������������
	// ������, ���� ����� ����� ��� ����� ��������� � ������ �������������
	// ��������� ������� � ������ �� ���
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_NO_LOF)
	{
		// ����������� - �������� ���
		AIAPI::print(m_entity, "|A|thinkAttack: ��� ����� ���� - ��������� ����� � ������ �������������");
		m_activity_observer.clear();

		// ������� � ������ �������������
		m_ignored_enemies.insert(enemy);
		if(std::includes(m_ignored_enemies.begin(),m_ignored_enemies.end(),enemies.begin(),enemies.end()))
		{
			// ��������� ������� ������� � �����
			AIAPI::print(m_entity, "|A|thinkAttack: �������� �������");
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
			m_activity = CreateMoveActivity(path, ActivityFactory::CT_ENEMY_MOVE);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_REACTED);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_ENEMY_SPOTTED);
			
			m_turn_state = TS_INPROGRESS;
			return false;
		} // if(diff_list.empty())
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
		m_activity = CreateRotateActivity(m_target_dir, ActivityFactory::CT_ENEMY_ROTATE);
		m_activity->Attach(&m_activity_observer,
			ActivityObserver::EV_MOVEPNTS_EXPIRIED);
		m_turn_state = TS_INPROGRESS;
		return false;
	}
	
	// 6.5 �������� � ����� - ���� ������ ����� ���������
	AIAPI::print(m_entity, "|A|thinkAttack: ����� ��������, ���� ��� ��������!");
	// ���� ���� - ��������� �� ���� ���� ��� ��������
	// �������� � ������������ 60%
	if(aiNormRand() < 0.6f)
	{
		AIAPI::setSitPose(m_entity);
	}
	else
	{
		AIAPI::setStandPose(m_entity);
	}

	// ���� ���� ������� � ����� ��������� ��������� ������ ����� -
	// ��������� ������ ��
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
		// ������� � ����� - ��������� ������ ��
		std::string reason;
		if(AIAPI::shoot(m_entity, enemy, &m_activity, &reason))
		{
			// ������ ������
			m_turn_state = TS_INPROGRESS;
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_NO_LOF);
			return false;
		}

		// ������ �� �������
		std::string str = "|A|thinkAttack: ������ ������ �������: ";
		str += reason;
		AIAPI::print(m_entity, str.c_str());
		m_turn_state = TS_END;
	}

	// ������ ��������� ���������� �� ������
	if(AIAPI::takeBestWeapon(m_entity, AssaultWeaponComparator()))
	{
		// ������ � �����
		AIAPI::setShootType(m_entity, 30.0f, 15.0f);
		if(AIAPI::getMovePoints(m_entity) <= AIAPI::getShootMovePoints(m_entity,AIAPI::getPtr(m_entity)->Cast2Human()->GetEntityContext()->GetShotType()))
		{
			AIAPI::getPtr(m_entity)->Cast2Human()->GetEntityContext()->SetShotType(SHT_SNAPSHOT);
		}
		std::string reason;
		if(AIAPI::shoot(m_entity, enemy, &m_activity, &reason))
		{
			// �������� ��������
			m_turn_state = TS_INPROGRESS;
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_NO_LOF);
			return false;
		}
		if(AIUtils::CalcEfficiency(AIAPI::getPtr(m_entity),AIAPI::getPos3(enemy)) == 0)
		{
			// ������� ������������� -- ��������� ������ ����, ���� �������� ������ 10 ����������
			if(AIAPI::getSteps(m_entity,AIAPI::getMovePoints(m_entity)) > 2)
			{
				m_turn_state = TS_START;
				return false;
			}
		}
		// ���������� �� �������
		std::string str = "|A|thinkAttack: �������� ������: ";
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
// ��������� ������ WeaponSearch
bool AssaultEnemyNode::thinkWeaponSearch(state_type* st)
{
	// 1. �������� ���� ��������, ���� ��� < �������� - ������ ���������
	SendCureMessage(assault_cure_dist);

	// 2. �������� ���� �� ���-���� ����� ������
	if(AIAPI::getWeaponLocation(m_entity, &m_target_pnt))
	{
		if(!comeToWeapon()) m_turn_state = TS_END;
		return false;
	}

	if(!m_ptlist.empty())
	{
		if(tryToSeek()) return false;
	}

	// 3. ���� �������� ������ ��������� - ��������� ���� ��������
	SelfCure();

	// 4. ���� ����� �� �������� ����� - �������� �����
	if(m_target_pnt == AIAPI::getPos2(m_entity))
	{
		AIAPI::print(m_entity, "|A|thinkWeaponSearch: ����� �� �������� ����� - �������� �����");
		// ���������� ���� ������������
		pnt_vec_t allhexes;
		PathUtils::CalcPassField(AIAPI::getPtr(m_entity),
			0, &allhexes, 0);
		if(allhexes.empty())
		{
			// �������� ��� - ���� ������
			AIAPI::print(m_entity, "|A|thinkWeaponSearch: ��� ������ ������!!!");
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
		AIAPI::print(m_entity, "|A|thinkWeaponSearch: �� ������� ����������");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 6. ���� � �������� �����
	AIAPI::print(m_entity, "|A|thinkWeaponSearch: ���� � �������� �����");
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
		return true;
	}
	// ����� ����
	m_activity = CreateMoveActivity(path, ActivityFactory::CT_ENEMY_MOVE);
	m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

//=====================================================================================//
//                      bool AssaultEnemyNode::thinkWeaponTake()                       //
//=====================================================================================//
// ��������� ������ WeaponTake
bool AssaultEnemyNode::thinkWeaponTake(state_type* st)
{
	// 1. �������� ���� ��������, ���� ��� < �������� - ������ ���������
	SendCureMessage(assault_cure_dist);

	// 2. �������� �� ������� �� �� � ������
	if(PickUpWeaponAndAmmo(m_target_pnt, AssaultWeaponComparator()))
	{
		m_mode = AEM_FIXED;
		return false;
	}

	// 3. �������� �� ��������� �� � ��� ���������
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// �� ������� ����������
		AIAPI::print(m_entity, "|A|thinkWeaponTake: �� ������� ����������");
		m_activity_observer.clear();
		m_turn_state = TS_END;
		return false;
	}

	// 4. �������� ���� �� ���-���� ����� ������
	if(AIAPI::getWeaponLocation(m_entity, &m_target_pnt))
	{
		if(!comeToWeapon())
		{
			m_mode = AEM_FIXED;
			m_turn_state = TS_INPROGRESS;
		}
		return false;
	}

	// ������ ��� - ������� � ������������� ���������
	m_mode = AEM_FIXED;
	m_turn_state = TS_INPROGRESS;
	return false;
}

//=====================================================================================//
//                        bool AssaultEnemyNode::thinkPursuit()                        //
//=====================================================================================//
// ��������� ������ Pursuit
bool AssaultEnemyNode::thinkPursuit(state_type* st)
{
	// 1. �������� ���� ��������, ���� ��� < �������� - ������ ���������
	SendCureMessage(assault_cure_dist);

	// 2. ���� � ���� ����� - ����� ��������� � ������� � �����
	AIAPI::entity_list_t enemies;
	if(AIAPI::getEnemiesVisibleByEntity(m_entity, &enemies))
	{
		// � ����-�� ����
		if(eid_t enemy = AIAPI::getMostDangerousEnemy(enemies,EnemyComparator(m_entity))	)
		{
			AIAPI::print(m_entity, "|A|thinkPursuit: � ������ �����!");

			// ������ ������ �������� �����
			// ����� ��������� ����� �������
			getSubteam()->sendMessage(this,	AIEnemyNode::MT_ENEMYSPOTTED,&enemy,assault_enemy_dist);

			// ������� � ��������� �����
			AIAPI::setWalk(m_entity);

			switchToAttack(enemy);
			return false;
		}
	}

	if(!m_ptlist.empty())
	{
		if(tryToSeek()) return false;
	}


	// 3. �������� �� ��������� �� � ��� ���������
	if(m_activity_observer.getLastEvent() == ActivityObserver::EV_MOVEPNTS_EXPIRIED)
	{
		// �� ������� ����������
		AIAPI::print(m_entity, "|A|thinkPursuit: �� ������� ����������");
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
			m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
			m_turn_state = TS_INPROGRESS;
			return false;
		}
		// 4.2 �������� - ������� ������������ ���������
		AIAPI::print(m_entity, "|A|thinkPursuit: ������ �� ������ - ������� ������������ ���������");
		// ��������� � ������������ ���������
		m_mode = AEM_FIXED;
		m_turn_state = TS_INPROGRESS;
		return false;
	}

	// 5. ���� � �������� �����
	AIAPI::print(m_entity, "|A|thinkPursuit: ���� � �������� �����");
	PathUtils::CalcPassField(AIAPI::getPtr(m_entity),	0, 0, 0);
	pnt_vec_t path;
	PathUtils::CalcPath(AIAPI::getPtr(m_entity), m_target_pnt, path);
	if(path.empty())
	{
		// ����� ��������� ������������ - ����� �������, ��� ��� ������
		// ���� ����
		m_target_pnt = AIAPI::getPos2(m_entity);
		m_turn_state = TS_INPROGRESS;
		return true;
	}
	// ����� ����
	AIAPI::setRun(m_entity);
	m_activity = CreateMoveActivity(path, ActivityFactory::CT_ENEMY_MOVE);
	m_activity->Attach(&m_activity_observer, ActivityObserver::EV_MOVEPNTS_EXPIRIED);
	m_turn_state = TS_INPROGRESS;
	return false;
}

//=====================================================================================//
//                      bool AssaultEnemyNode::thinkMedkitTake()                       //
//=====================================================================================//
// ��������� ������ MedkitTake
bool AssaultEnemyNode::thinkMedkitTake(state_type* st)
{
	if(!MedkitTake(m_target_pnt,st)) m_mode = AEM_FIXED;
	return false;
}

//=====================================================================================//
//                            bool AssaultEnemyNode::die()                             //
//=====================================================================================//
// ����!
bool AssaultEnemyNode::die()
{
	m_mode = AEM_KILLED;
	return need2Delete();
}

//=====================================================================================//
//                     bool AssaultEnemyNode::need2Delete() const                      //
//=====================================================================================//
// ����� �� ������� ����
bool AssaultEnemyNode::need2Delete() const
{
	if( (m_mode == AEM_KILLED) && !m_activity) return true;
	return false;
}
// 1050
// 1079
// 1112
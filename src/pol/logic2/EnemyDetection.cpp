/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: �������� ����������� �����
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                
#pragma warning(disable:4786)

#include "logicdefs.h"
#include "EnemyDetection.h"
#include "entity.h"
#include "xlsreader.h"
#include "spawn.h"

#include "AIContext.h"

#ifdef _HOME_VERSION
#	define REL_LOG(L)	MLL_MAKE_LOG("rels.log",L)
#else
#	define REL_LOG(L)
#endif

namespace EnemyDetectionDetail
{
    const char* alignment_xls_name = "scripts/logic/alignment.txt";
    enum alignment_column_type
	{
         ACT_SUBTEAM,
         ACT_ENEMIES,
		 ACT_FRIENDS,
		 //ACT_NAMES,
            
         ACT_MAX_COLUMNS,            
    };

    column alignment_columns[ACT_MAX_COLUMNS] = 
    {
        column( "subteam", ACT_SUBTEAM),
        column( "enemies", ACT_ENEMIES),
        column( "friends", ACT_FRIENDS)
		//column( "name",	   ACT_NAMES)
    };

	class EnemyDetectionObserver : public SpawnObserver
	{
	public:
		virtual void Update(Spawner* spawner, event_t type, info_t info)
		{
			// TODO: �������� ���, ��������������� �������e ������
			EnemyDetector::getInst()->clearCurLevelInfo();
		}
	};

}

using namespace EnemyDetectionDetail;

//=====================================================================================//
//                         void EnemyDetector::loadAlignment()                         //
//=====================================================================================//
// ��������� �������������� �������� � ����������� ��� �� xls'��
void EnemyDetector::loadAlignment()
{
	TxtFilePtr txt(alignment_xls_name);
	
	txt.ReadColumns(alignment_columns, alignment_columns + ACT_MAX_COLUMNS);

	for(int i = 1; i < txt->SizeY(); ++i)
	{
		// ������� �������� ����������
		std::string subteam_name;
		txt->GetCell(i, alignment_columns[ACT_SUBTEAM].m_index, subteam_name);
		// ��������, ���� �� ����� ���������� � ����� ������� ���������
		if(m_subteams.find(subteam_name) == m_subteams.end())
		{
			// ����� ���������� ��� �� ����������� - ������� �� � ����� �������
			m_subteams[subteam_name] = m_next_subteam_number++;
		}
		
		// ������� ������� ��� ����������
		std::string usual_name = PoL::getLocStr("alignment." + subteam_name + ".name");
		//txt->GetCell(i, alignment_columns[ACT_NAMES].m_index, usual_name);
		// ������� ��� � ����� �����������
		m_convert_map[subteam_name] = usual_name;

		// ������� �������� ��������� - ������ �������
		std::string enemies;  // �������� ��������� ����� ������� � ������
		txt->GetCell(i, alignment_columns[ACT_ENEMIES].m_index, enemies);
		std::istringstream stream(enemies);
		std::string word;
		subteam_set_t enemy_set;
		// ��������� �� ���� ��������� ���������-������
		while((stream >> word) && (word != "none"))
		{
			// word �������� ��� ����������-����� - �������� ���� �� ����� �
			// ����� ����� ������� ���������
			if(m_subteams.find(word) == m_subteams.end())
			{
				// ����� ���������� ��� �� ����������� - ������� �� � ����� �������
				m_subteams[word] = m_next_subteam_number++;
			}

			// ������� ����� ����������-����� � ������
			enemy_set.insert(m_subteams[word]);
		}
		// ������� � ����� ��������� ����� ������������ ��������� ������
		// ��������: ���� ��� ��� ���� ���������� � ����� �� ������, ��
		// ���������� ����� ������������
		m_relations[m_subteams[subteam_name]] = enemy_set;

		// ������� �������� ��������� - ������ �������
		txt->GetCell(i, alignment_columns[ACT_FRIENDS].m_index, enemies);
		std::istringstream stream2(enemies);
		enemy_set.clear();
		// ��������� �� ���� ��������� ���������-������
		while((stream2 >> word) && (word != "none"))
		{
			// word �������� ��� ����������-����� - �������� ���� �� ����� �
			// ����� ����� ������� ���������
			if(m_subteams.find(word) == m_subteams.end())
			{
				// ����� ���������� ��� �� ����������� - ������� �� � ����� �������
				m_subteams[word] = m_next_subteam_number++;
			}

			// ������� ����� ����������-����� �� ���������
			enemy_set.insert(m_subteams[word]);
		}
		// ������� � ����� ������ ��������� ������
		// ��������: ���� ��� ��� ���� ���������� � ����� �� ������, ��
		// ���������� ����� ������������
		m_friends[m_subteams[subteam_name]] = enemy_set;
	}
}

//////////////////////////////////////////////////////////////////////////////
//
// ����� ��� �������� ������������������ ������������ � ����������� ��
//
//////////////////////////////////////////////////////////////////////////////

// ���������������� �����������
void RelationsObserversContainer::attach(RelationsObserver* observer, RelationsObserver::event_type type)
{
	switch(type)
	{
	case RelationsObserver::ET_RELATION2PLAYER_CHANGED :
		   {
			   m_player_observers.insert(observer);
			   break;
		   }
	case RelationsObserver::ET_RELATIONS_CHANGED :
		   {
			   m_anybody_observers.insert(observer);
			   break;
		   }
	}
}

// ����������� �����������
void RelationsObserversContainer::detach(RelationsObserver* observer)
{
	m_player_observers.erase(observer);
	m_anybody_observers.erase(observer);
}

// ����������
RelationsObserversContainer::~RelationsObserversContainer()
{
	m_player_observers.clear();
	m_anybody_observers.clear();
}

// ����������� � ���, ��� ���� ������� ��������� � ������
void RelationsObserversContainer::notifyEntityChangedRelationWithPlayer(eid_t entity)
{
	Observers::iterator i = m_player_observers.begin();
	Observers::iterator end = m_player_observers.end();
	while(i != end)
	{
		RelationsObserver::EntityDesc ed(entity);
		(*i)->changed(RelationsObserver::ET_RELATION2PLAYER_CHANGED, &ed);
		++i;
	}
}

// ����������� � ���, ��� ���������� ����� ������ ������
void RelationsObserversContainer::notifySubteamChangedRelationWithPlayer(const std::string& name)
{
	EntityPool::iterator i = EntityPool::GetInst()->begin(ET_ALL_ENTS,
		PT_ENEMY | PT_CIVILIAN);
	EntityPool::iterator end = EntityPool::GetInst()->end();
	while(i != end)
	{
		if(BaseEntity* be = &*i)
		{
			// �������� ����
			if(AIContext* context = be->GetAIContext())
			{
				// �������� ���� - ������� ��� ����������
				std::string str = context->getSubteam();
				if(str == name)
				{
					// �������� ����� ������
					notifyEntityChangedRelationWithPlayer(be->GetEID());
					be->Notify(EntityObserver::EV_RELATION_CHANGE);
				}
			}
		}
		++i;
	}
}

// ����������� � ���, ��� ���������� �������� ��������� � ������ ����������
void RelationsObserversContainer::notifySubteam1ChangedRelationWithSubteam2(BaseEntity* entity1, BaseEntity* entity2, RelationType type)
{
	std::string subteam1;
	std::string subteam2;

	// ��������� ���������� �������

	// ������ ��������
	switch(entity1->GetPlayer())
	{
	case PT_PLAYER : 
		{
			subteam1 = "player";
			break;
		}
	case PT_ENEMY : case PT_CIVILIAN :
		{
			if(AIContext* context = entity1->GetAIContext())
			{
				subteam1 = context->getSubteam();
			}
			break;
		}
	default:;
	}

	// ������ ��������
	switch(entity2->GetPlayer())
	{
	case PT_PLAYER : 
		{
			subteam2 = "player";
			break;
		}
	case PT_ENEMY : case PT_CIVILIAN :
		{
			if(AIContext* context = entity2->GetAIContext())
			{
				subteam2 = context->getSubteam();
			}
			break;
		}
	default:;
	}
	// ������ ���������
	notifySubteam1ChangedRelationWithSubteam2(subteam1, subteam2, type);
}

// ����������� � ���, ��� ���������� �������� ��������� � ������ ����������
void RelationsObserversContainer::notifySubteam1ChangedRelationWithSubteam2(const std::string& subteam1, const std::string& subteam2, RelationType type)
{
	Observers::iterator i = m_anybody_observers.begin();
	Observers::iterator end = m_anybody_observers.end();
	while(i != end)
	{
		RelationsObserver::RelationDesc rd;
		rd.m_first_subteam = subteam1;
		rd.m_second_subteam = subteam2;
		rd.m_relation_type = type;
		(*i)->changed(RelationsObserver::ET_RELATIONS_CHANGED, &rd);
		++i;
	}
}

// �������� ������������
void RelationsObserversContainer::clear()
{
	m_player_observers.clear();
	m_anybody_observers.clear();
}

///////////////////////////////////////////////////////////////////////////////
//
// �����, ��������������� ��������� ��� ����������� �����
//
///////////////////////////////////////////////////////////////////////////////

// ����� ����� ������ ����������� ���������� ������
EnemyDetector::Container EnemyDetector::m_container;
// �������� ��������� �� ������������ ������ ������
EnemyDetector* EnemyDetector::getInst()
{
	// ���� ������ �� ������ - �������� ���
	if(!m_container.m_detector)
	{
		m_container.m_detector = new EnemyDetector;
	}

	return m_container.m_detector;
}

// ������������������� ����� ����������� ������
void EnemyDetector::init()
{
	m_next_subteam_number = 1;
	loadAlignment();
}


// �����������
EnemyDetector::EnemyDetector() :
m_loadlevel_observer(new EnemyDetectionObserver)
{
	// ���������������� �� ��������� ��������� � �������� ������
	Spawner::Attach(m_loadlevel_observer, SpawnObserver::ET_ENTRY_LEVEL);
	// ���������������� �� ��������� ��������� � ������� �������
    //GameEvMessenger::GetInst()->Attach(this, EV_HURT);
    GameEvMessenger::GetInst()->Attach(this, EV_KILL);
}

// ����������
EnemyDetector::~EnemyDetector()
{
	// ��������� �� ��������� ��������� � �������� ������

	//Punch: ���� Spawner ������������ ������ �.�.�����    
    //Punch: Spawner::Detach(m_loadlevel_observer);

	delete m_loadlevel_observer;

    // ��������� �� ��������� ��������� � ������� �������

	//Punch: ���� GameEvMessenger ������������ ������ �.�.�����    
	//Punch: GameEvMessenger::GetInst()->Detach(this);
}

// ��������� �������� �� ������ �������� ��� ������� �������� ������
bool EnemyDetector::isHeEnemy4Me(BaseEntity * me, BaseEntity * he)
{
	// ��������� ���������� ���������� (��� eid ��������� ���� �� ��� �� ����) me
	int my_number = 0;
	int my_eid = 0;
	switch(me->GetPlayer())
	{
	case PT_PLAYER : 
		my_number = m_subteams["player"];
		break;
	case PT_ENEMY :
		// ���� � ����� ��� ���������, �� �� ������ �����!
		if(!me->GetAIContext()) return false;
		my_number = m_subteams[me->GetAIContext()->getSubteam()];
		break;
	case PT_CIVILIAN :
		if(!me->GetAIContext())
		{
			// � ������� ������ ��� ���������, ��� ����������
			// ���� ��� ������� ��� �������� �������� ��� �����
			if((me->Cast2Human()) ||
				(me->Cast2Trader())
				) my_eid = static_cast<int>(me->GetEID());
			else return false;
			break;
		}
		my_number = m_subteams[me->GetAIContext()->getSubteam()];
		break;
	default:;
	}

	// ��������� ���������� ���������� (��� eid ��������� ���� �� ��� �� ����) he
	int his_number = 0;
	int his_eid = 0;
	switch(he->GetPlayer())
	{
	case PT_PLAYER : 
		his_number = m_subteams["player"];
		break;
	case PT_ENEMY : 
		// ���� � ����� ��� ���������, �� �� ������ �����!
		if(!he->GetAIContext()) return false;
		his_number = m_subteams[he->GetAIContext()->getSubteam()];
		break;
	case PT_CIVILIAN :
		if(!he->GetAIContext())
		{
			// � ������� ������ ��� ���������, ��� ����������
			if((he->Cast2Human()) ||
				(he->Cast2Trader())
				) his_eid = static_cast<int>(he->GetEID());
			else return false;
			break;
		}
		his_number = m_subteams[he->GetAIContext()->getSubteam()];
		break;
	default:;
	}

	if(my_number && his_number)
	{
		// � me, � he ����������� ������������ �����������
		// �������� ������� �� me ������ ���������� he

		// ���� ��������� ������ me ������, ��� �� ����
		if(m_relations[my_number].empty()) return false;
		// �������� ���� �� ����� ����� ���������� he ����� ������ me
		subteam_set_t::iterator it = m_relations[my_number].find(his_number);
		if(it == m_relations[my_number].end()) return false;
		// ������ ����, - �� ��� ����
		return true;
	}
	if(my_number && his_eid)
	{
		// � me ���� ����������, � he �������� ������ ������� �� �������������
		// �� ����� �� ���������

		// �������� ���� �� � ���� ���������� ����� ����� ������ �������
		if(m_subteam_vs_civilians[my_number].empty()) return false;

		// ����, �������� ���� �� ����� ��� � ������� he_eid
		subteam_set_t::iterator it = m_subteam_vs_civilians[my_number].find(his_eid);
		if(it == m_subteam_vs_civilians[my_number].end()) return false;
		// ������ ����, - �� ��� ����
		return true;
	}

	if(my_eid && his_number)
	{
		// � - ������ ������, �� ������������� �� ����� ����������,
		// � � he ���� ����������

		// �������� ���� �� � ���� ����� ����� ���������
		if(m_civilian_vs_subteams[my_eid].empty()) return false;

		// ����, �������� ���� �� ����� ��� � ������� his_number
		subteam_set_t::iterator it = m_civilian_vs_subteams[my_eid].find(his_number);
		if(it == m_civilian_vs_subteams[my_eid].end()) return false;
		// ������ ����, - �� ��� ����
		return true;
	}

	if(my_eid && his_eid)
	{
		// � me, � he �������� ������� ��������, �� �������������� �� ����� �� ���������

		// �������� ���� �� � ���� ����� ����� ������ �������
		if(m_civilian_vs_civilians[my_eid].empty()) return false;

		// ����, �������� ���� �� ����� ��� � ������� his_eid
		subteam_set_t::iterator it = m_civilian_vs_civilians[my_eid].find(his_eid);
		if(it == m_civilian_vs_civilians[my_eid].end()) return false;
		// ������ ����, - �� ��� ����
		return true;
	}
	return false;
}

//=====================================================================================//
//                           bool EnemyDetector::isHeEnemy()                           //
//=====================================================================================//
bool EnemyDetector::isHeEnemy(BaseEntity * he)
{
	if(he->GetPlayer() == PT_PLAYER) return false;

	// ��������� ���������� ���������� (��� eid ��������� ���� �� ��� �� ����) me
	int my_number = m_subteams["player"];

	// ��������� ���������� ���������� (��� eid ��������� ���� �� ��� �� ����) he
	int his_number = 0;
	int his_eid = 0;

	switch(he->GetPlayer())
	{
	case PT_ENEMY : 
		// ���� � ����� ��� ���������, �� �� ������ �����!
		if(!he->GetAIContext()) return false;
		his_number = m_subteams[he->GetAIContext()->getSubteam()];
		break;
	case PT_CIVILIAN :
		if(!he->GetAIContext())
		{
			// � ������� ������ ��� ���������, ��� ����������
			if((he->Cast2Human()) ||
				(he->Cast2Trader())
				) his_eid = static_cast<int>(he->GetEID());
			else return false;
			break;
		}
		his_number = m_subteams[he->GetAIContext()->getSubteam()];
		break;
	default:;
	}

	if(my_number && his_number)
	{
		// � me, � he ����������� ������������ �����������
		// �������� ������� �� me ������ ���������� he

		// ���� ��������� ������ me ������, ��� �� ����
		if(m_relations[my_number].empty()) return false;
		// �������� ���� �� ����� ����� ���������� he ����� ������ me
		subteam_set_t::iterator it = m_relations[my_number].find(his_number);
		if(it == m_relations[my_number].end()) return false;
		// ������ ����, - �� ��� ����
		return true;
	}

	if(my_number && his_eid)
	{
		// � me ���� ����������, � he �������� ������ ������� �� �������������
		// �� ����� �� ���������

		// �������� ���� �� � ���� ���������� ����� ����� ������ �������
		if(m_subteam_vs_civilians[my_number].empty()) return false;

		// ����, �������� ���� �� ����� ��� � ������� he_eid
		subteam_set_t::iterator it = m_subteam_vs_civilians[my_number].find(his_eid);
		if(it == m_subteam_vs_civilians[my_number].end()) return false;
		// ������ ����, - �� ��� ����
		return true;
	}

	return false;
}

// ������� ��� ��������� ����� ������� � ���������
RelationType EnemyDetector::getRelationBetweenPlayerAndHe(BaseEntity * he)
{
	RelationType rt = RT_NEUTRAL;
	int player = m_subteams["player"];
	// ��������� ���������� ���������� (��� eid ��������� ���� �� ��� �� ����) he
	int his_number = 0;
	int his_eid = 0;
	switch(he->GetPlayer())
	{
	case PT_PLAYER : 
		{
			// �� ����� �������
			rt = RT_FRIEND;
			return rt; 
		}
	case PT_ENEMY : 
		// ���� � ����� ��� ���������, �� �� ������ �����!
		if(!he->GetAIContext()) 
		{
			return rt;
		}
		his_number = m_subteams[he->GetAIContext()->getSubteam()];
		break;
	case PT_CIVILIAN :
		if(!he->GetAIContext())
		{
			// � ������� ������ ��� ���������, ��� ����������
			if((he->Cast2Human()) ||
				(he->Cast2Trader())
				) 
			{
				his_eid = static_cast<int>(he->GetEID());
			}
			else 
			{
				return rt;
			}
			break;
		}
		his_number = m_subteams[he->GetAIContext()->getSubteam()];
		break;
	default:;
	}

	if(his_number)
	{
		// � ���� ���� ���������� - �������� ��������� ���������

		// �� �����
		if(!m_relations[player].empty())
		{
			// �������� ���� �� ����� ����� ���������� he ����� ������ player
			subteam_set_t::iterator it = m_relations[player].find(his_number);
			if(it != m_relations[player].end())
			{
				// ������ ����, - �� ���� player'a
				rt = RT_ENEMY;
				return rt;
			}
		}

		// �� �����
		if(!m_friends[player].empty())
		{
			// �������� ���� �� ����� ����� ���������� he ����� ������ player
			subteam_set_t::iterator it = m_friends[player].find(his_number);
			if(it != m_friends[player].end())
			{
				// ������ ����, - �� ���� player'a
				rt = RT_FRIEND;
				return rt;
			}
		}

		// ������ �����������
		return rt;
	}

	if(his_eid)
	{
		// he �������� ������ ������� �� ������������� �� ����� �� ���������

		// �������� ���� �� � ���������� player ����� ����� ������ �������
		if(!m_subteam_vs_civilians[player].empty())
		{
			// ����, �������� ���� �� ����� ��� � ������� he_eid
			subteam_set_t::iterator it = m_subteam_vs_civilians[player].find(his_eid);
			if(it != m_subteam_vs_civilians[player].end())
			{
				// ������ ����, - �� ���� player'a
				rt = RT_ENEMY;
				return rt;
			}
		}
		// �����������
		return rt;
	}

	// �� ���� ���� ������ �������
	return rt;
}

// ��������� ��������� � ������� �������� �� ������
void EnemyDetector::Update(subject_t subj, event_t event, info_t ptr)
{
    hurt_info* info = static_cast<hurt_info*>(ptr);

    if(    !info->m_killer || !info->m_victim || info->m_killer == info->m_victim)
        return;

    // ��������� ���������� ���������� (��� eid ��������� ���� �� ��� �� ����) ������ 
	int killer_number = 0;
	int killer_eid = 0;
	switch(info->m_killer->GetPlayer())
	{
	case PT_PLAYER : 
			killer_number = m_subteams["player"];
			break;
	case PT_ENEMY : 
		{
			if(!info->m_killer->GetAIContext())	// � ����� ��� ���������!
				return;
			// �������� ����, �������� ���� �� � ���� ����������
			std::string str = info->m_killer->GetAIContext()->getSubteam();
			if( str=="" || str=="none" )// ���������� ��� - ������ �� ������
			{
				break;
			}
			// ���� ����������
			killer_number = m_subteams[str];
			break;
		}
	case PT_CIVILIAN :
		{
			if(!info->m_killer->GetAIContext())
			{
				// � ������� ������ ��� ��������� - 
				// ���� ��� ������� ��� �������� - �������� ��� �����
				if((info->m_killer->Cast2Human()) ||
					(info->m_killer->Cast2Trader())
					) killer_eid = static_cast<int>(info->m_killer->GetEID());
				else return;
				break;
			}
			// �������� ����, �������� ���� �� � ���� ����������
			std::string str = info->m_killer->GetAIContext()->getSubteam();
			if( str=="" || str=="none" )// ���������� ��� - ������ �� ������
				break;
			// ���� ����������
			killer_number = m_subteams[str];
			break;
		}
	default: return;
	}
	
	// ��������� ���������� ���������� (��� eid ��������� ���� �� ��� �� ����) ������
	int victim_number = 0;
	int victim_eid = 0;
	switch(info->m_victim->GetPlayer())
	{
	case PT_PLAYER : 
			// ��� ��� ������ - �����, �� ������� ��������� �� ��������
			return;
	case PT_ENEMY : 
		{
			if(!info->m_victim->GetAIContext())	// � ����� ��� ���������!
				return;

			// �������� ����, �������� ���� �� � ���� ����������
			std::string str = info->m_victim->GetAIContext()->getSubteam();
			if( str=="" || str=="none" )// ���������� ��� - ������ �� ������
				break;
			// ���� ����������
			victim_number = m_subteams[str];
			break;
		}
	case PT_CIVILIAN :
		{
			if(!info->m_victim->GetAIContext())	// � ������� ������ ��� ��������� -
			{
				// ���� ��� ������� ��� �������� - �������� ��� �����
				if((info->m_victim->Cast2Human()) || info->m_victim->Cast2Trader())
					victim_eid = static_cast<int>(info->m_victim->GetEID());
				else return;
				break;
			}
			// �������� ����, �������� ���� �� � ���� ����������
			std::string str = info->m_victim->GetAIContext()->getSubteam();
			if( str=="" || str=="none" )// ���������� ��� - ������ �� ������
				break;
			// ���� ����������
			victim_number = m_subteams[str];
			break;
		}
	default: return;
	}

	if(killer_number && victim_number)
	{
		if(killer_number == victim_number) return; // ����� ���-�� �� �����
		// � ������, � ������ ����������� ������������ �����������

		// ���� ���������� ������ ����� ������ � ����������� �����, �� ���
		// �� �������
		//if(event!=EV_KILL || info->m_victim->GetPlayer()!=PT_ENEMY || info->m_killer->GetPlayer()!=PT_PLAYER)
		{
			if(m_friends[victim_number].find(killer_number) != m_friends[victim_number].end())
			{
				return;
			}
		}

		// �������, ����� ���������� ������ ������ ������� ����������
		// ������ ����� ������
		std::pair<subteam_set_t::iterator, bool> res = m_relations[victim_number].insert(killer_number);
		if(res.second) // ��������� �� �������� ����������
		{
			// ������ ����������� �� ��������� ��������� ����� ������������
			m_observers_container.notifySubteam1ChangedRelationWithSubteam2(
				info->m_victim,
				info->m_killer,
				RT_ENEMY);
			// ���������, ����� �� �������� ����������� ��������� � �������
			if(info->m_victim->GetPlayer() == PT_PLAYER)
			{
				// ������ �����������
				info->m_killer->Notify(EntityObserver::EV_RELATION_CHANGE);
				m_observers_container.notifySubteamChangedRelationWithPlayer(info->m_killer->GetAIContext()->getSubteam());
			}
			GameEvMessenger::GetInst()->Notify(GameObserver::EV_RELCHANGED,0);
		}

		// ���� ������ - ����� - ������� ����� ���������� ������ ����� ��� ������
		if(info->m_killer->GetPlayer() == PT_PLAYER)
		{
			std::pair<subteam_set_t::iterator, bool> res = m_relations[killer_number].insert(victim_number);
			if(res.second) // ���������� ������� ���������
			{
				// ������ �����������
				m_observers_container.notifySubteam1ChangedRelationWithSubteam2(
					info->m_killer,
					info->m_victim,
					RT_ENEMY);
				// � ��� �������
				info->m_victim->Notify(EntityObserver::EV_RELATION_CHANGE);
				m_observers_container.notifySubteamChangedRelationWithPlayer(info->m_victim->GetAIContext()->getSubteam());
				GameEvMessenger::GetInst()->Notify(GameObserver::EV_RELCHANGED,0);
			}
		}
		return;
	}

	if(killer_number && victim_eid)
	{
		// � ������ ���� ����������, � ������ �������� ������ ������� �� �������������
		// �� ����� �� ��������� - ������� ���������� ������ ������� ������
		m_civilian_vs_subteams[victim_eid].insert(killer_number);
		// ���� ������ - �����, �� ������� ������� ������ ������ �������
		if(info->m_killer->GetPlayer() == PT_PLAYER)
		{
			std::pair<subteam_set_t::iterator, bool> res = m_subteam_vs_civilians[killer_number].insert(victim_eid);
			if(res.second) // ���������� ���������
			{
				// ������ �����������
				info->m_victim->Notify(EntityObserver::EV_RELATION_CHANGE);
				m_observers_container.notifyEntityChangedRelationWithPlayer(victim_eid);
				GameEvMessenger::GetInst()->Notify(GameObserver::EV_RELCHANGED,0);
			}
		}
		return;
	}

	if(killer_eid && victim_number)
	{
		// ������ - ������ ������, �� ������������� �� ����� ����������,
		// � � ������ ���� ����������. ������� ������� ������ ������ ����������
		std::pair<subteam_set_t::iterator, bool> res = m_subteam_vs_civilians[victim_number].insert(killer_eid);
		// ���������, ����� �� �������� �����������
		if(info->m_victim->GetPlayer() == PT_PLAYER)
		{
			if(res.second) // ���������� ���������
			{
				// ������ �����������
				info->m_killer->Notify(EntityObserver::EV_RELATION_CHANGE);
				m_observers_container.notifyEntityChangedRelationWithPlayer(killer_eid);
				GameEvMessenger::GetInst()->Notify(GameObserver::EV_RELCHANGED,0);
			}
		}
		return;
	}

	if(killer_eid && victim_eid)
	{
		// � ������, � ������ �������� ������� ��������, �� �������������� �� ����� �� ���������
		// ������� ������ ������� ������ ������
		m_civilian_vs_civilians[victim_eid].insert(killer_eid);
		GameEvMessenger::GetInst()->Notify(GameObserver::EV_RELCHANGED,0);
		return;
	}

	// ���� � �������:
	//throw CASUS("EnemyDetector::OnHurtEv: �������������� ������ ��������� �����������");
}

//����������/�������������
void EnemyDetector::MakeSaveLoad(SavSlot& st)
{
    //��������/����������� ��������� ����������
	if(st.IsSaving())
	{
		// �����������

		// �������� ����� ������� ���������
		st << m_subteams.size();
		{
			subteam_map_t::iterator i = m_subteams.begin();
			subteam_map_t::iterator end = m_subteams.end();
			while(i != end)
			{
				st << (*i).first;
				st << (*i).second;
				++i;
			}
		}

		// �������� ����� ��������� ����� ������������
		st << m_relations.size();
		{
			relation_map_t::iterator i = m_relations.begin();
			relation_map_t::iterator end = m_relations.end();
			while(i != end)
			{
				st << (*i).first;
				st << (*i).second.size();
				subteam_set_t::iterator j = (*i).second.begin();
				subteam_set_t::iterator enD = (*i).second.end();
				while(j != enD)
				{
					st << (*j);
					j++;
				}
				i++;
			}
		}

		// �������� ����� ������ ���������
		st << m_friends.size();
		{
			relation_map_t::iterator i = m_friends.begin();
			relation_map_t::iterator end = m_friends.end();
			while(i != end)
			{
				st << (*i).first;
				st << (*i).second.size();
				subteam_set_t::iterator j = (*i).second.begin();
				subteam_set_t::iterator enD = (*i).second.end();
				while(j != enD)
				{
					st << (*j);
					j++;
				}
				i++;
			}
		}

		// �������� ����� ��� ��������� ����������
		st << m_next_subteam_number;

		// �������� �����: ���� - ����� ����������, �������� - ������ ������� ������ ������� (������)
		st << m_subteam_vs_civilians.size();
		{
			relation_map_t::iterator i = m_subteam_vs_civilians.begin();
			relation_map_t::iterator end = m_subteam_vs_civilians.end();
			while(i != end)
			{
				st << (*i).first;
				st << (*i).second.size();
				subteam_set_t::iterator j = (*i).second.begin();
				subteam_set_t::iterator enD = (*i).second.end();
				while(j != enD)
				{
					st << (*j);
					j++;
				}
				i++;
			}
		}

		// �������� �����: ���� - ����� ������� ������, �������� - ������ ������� ��������� (������)
		st << m_civilian_vs_subteams.size();
		{
			relation_map_t::iterator i = m_civilian_vs_subteams.begin();
			relation_map_t::iterator end = m_civilian_vs_subteams.end();
			while(i != end)
			{
				st << (*i).first;
				st << (*i).second.size();
				subteam_set_t::iterator j = (*i).second.begin();
				subteam_set_t::iterator enD = (*i).second.end();
				while(j != enD)
				{
					st << (*j);
					j++;
				}
				i++;
			}
		}

		// �������� �����: ���� - ����� ������� ������, �������� - ������ ������� ������ ������� (������)
		st << m_civilian_vs_civilians.size();
		{
			relation_map_t::iterator i = m_civilian_vs_civilians.begin();
			relation_map_t::iterator end = m_civilian_vs_civilians.end();
			while(i != end)
			{
				st << (*i).first;
				st << (*i).second.size();
				subteam_set_t::iterator j = (*i).second.begin();
				subteam_set_t::iterator enD = (*i).second.end();
				while(j != enD)
				{
					st << (*j);
					j++;
				}
				i++;
			}
		}

		// �������� ����� �����������:
		st << m_convert_map.size();
		{
			convert_map_t::iterator i = m_convert_map.begin();
			convert_map_t::iterator end = m_convert_map.end();
			while(i != end)
			{
				st << (*i).first;
				st << (*i).second;
				i++;
			}
		}

		return;
	}
	// ��������

	// ��������� ����� ������� ���������
	int size;
	st >> size;
	m_subteams.clear();
	for(int i = 0; i < size; i++)
	{
		std::string str;
		int num;
		st >> str;
		st >> num;
		m_subteams[str] = num;
	}

	// ��������� ����� ��������� ����� ������������
	st >> size;
	m_relations.clear();
	for(int i = 0; i < size; i++)
	{
		int subteam_num;
		st >> subteam_num;
		int set_size;
		st >> set_size;
		subteam_set_t set;
		for(int j = 0; j < set_size; j++)
		{
			int s_num;
			st >> s_num;
			set.insert(s_num);
		}
		m_relations[subteam_num] = set;
	}

	// ��������� ����� ������ ���������
	st >> size;
	m_friends.clear();
	for(int i = 0; i < size; i++)
	{
		int subteam_num;
		st >> subteam_num;
		int set_size;
		st >> set_size;
		subteam_set_t set;
		for(int j = 0; j < set_size; j++)
		{
			int s_num;
			st >> s_num;
			set.insert(s_num);
		}
		m_friends[subteam_num] = set;
	}

	// ��������� ����� ��� ��������� ����������
	st >> m_next_subteam_number;

	// ��������� �����: ���� - ����� ����������, �������� - ������ ������� ������ ������� (������)
	st >> size;
	m_subteam_vs_civilians.clear();
	for(int i = 0; i < size; i++)
	{
		int subteam_num;
		st >> subteam_num;
		int set_size;
		st >> set_size;
		subteam_set_t set;
		for(int j = 0; j < set_size; j++)
		{
			int s_num;
			st >> s_num;
			set.insert(s_num);
		}
		m_subteam_vs_civilians[subteam_num] = set;
	}

	// ��������� �����: ���� - ����� ������� ������, �������� - ������ ������� ��������� (������)
	st >> size;
	m_civilian_vs_subteams.clear();
	for(int i = 0; i < size; i++)
	{
		int subteam_num;
		st >> subteam_num;
		int set_size;
		st >> set_size;
		subteam_set_t set;
		for(int j = 0; j < set_size; j++)
		{
			int s_num;
			st >> s_num;
			set.insert(s_num);
		}
		m_civilian_vs_subteams[subteam_num] = set;
	}

	// ��������� �����: ���� - ����� ������� ������, �������� - ������ ������� ������ ������� (������)
	st >> size;
	m_civilian_vs_civilians.clear();
	for(int i = 0; i < size; i++)
	{
		int subteam_num;
		st >> subteam_num;
		int set_size;
		st >> set_size;
		subteam_set_t set;
		for(int j = 0; j < set_size; j++)
		{
			int s_num;
			st >> s_num;
			set.insert(s_num);
		}
		m_civilian_vs_civilians[subteam_num] = set;
	}
	
	// ��������� ����� �����������:
	st >> size;
	m_convert_map.clear();
	for(int i = 0; i < size; i++)
	{
		std::string key;
		std::string data;
		st >> key;
		st >> data;
		m_convert_map[key] = data;
	}
}

// �������� ����������, �������� ������ ��� �������� ������
void EnemyDetector::clearCurLevelInfo()
{
	m_subteam_vs_civilians.clear();
	m_civilian_vs_subteams.clear();
	m_civilian_vs_civilians.clear();
}

// ���������������� �����������
void EnemyDetector::attach(RelationsObserver* observer, RelationsObserver::event_type type)
{
	m_observers_container.attach(observer, type);
}

// ����������� �����������
void EnemyDetector::detach(RelationsObserver* observer)
{
	m_observers_container.detach(observer);
}

//=====================================================================================//
//                           static std::string relToText()                            //
//=====================================================================================//
static std::string relToText(RelationType rt)
{
	switch(rt)
	{
	case RT_FRIEND: return "friend";
	case RT_NEUTRAL: return "neutral";
	case RT_ENEMY: return "enemy";
	}

	return "unknown";
}

// ������������� �������� ��������� ����� ����� ������������.
// ����� ������ ���������� 1 ����� ���������� � ���������� 2 ��� � new_relation
void EnemyDetector::setRelation(const std::string& subteam1,
								const std::string& subteam2,
								RelationType new_relation)
{
	// ������ ������� ��������� ���������� 1 � ���������� 2
	RelationType old_relation = RT_NEUTRAL; // �� ��������� - �����������
	int snum1 = m_subteams[subteam1];
	int snum2 = m_subteams[subteam2];
	if( (!snum1) || (!snum2) ) return;
	
	// �� �����
	if(!m_relations[snum1].empty())
	{
		// �������� ���� �� ����� ����� ���������� snum2 ����� ������ snum1
		subteam_set_t::iterator it = m_relations[snum1].find(snum2);
		if(it != m_relations[snum1].end())
		{
			// ������ ����, - �� ���� 
			old_relation = RT_ENEMY;
		}
	}
	
	// �� �����
	if(!m_friends[snum1].empty())
	{
		// �������� ���� �� ����� ����� ���������� he ����� ������ player
		subteam_set_t::iterator it = m_friends[snum1].find(snum2);
		if(it != m_friends[snum1].end())
		{
			// ������ ����, - �� ���� player'a
			old_relation = RT_FRIEND;
		}
	}

	if(old_relation == new_relation)
	{
		// ������ ������ �� ����
		return;
	}

	// ��������� ����������
	// ������ ���������� 2 �� ��������� ������ � ������ ���������� 1
	clearRelation(subteam1, subteam2);
	switch(new_relation)
	{
	case RT_ENEMY :
		{
			// ������� ���������� 2 � ��������� ������ ���������� 1
			setEnemyRelation(subteam1, subteam2);
			break;
		}
	case RT_FRIEND :
		{
			// ������� ���������� 2 � ��������� ������ ���������� 1
			setFriendRelation(subteam1, subteam2);
			break;
		}
	}

	// ������ �����������
	m_observers_container.notifySubteam1ChangedRelationWithSubteam2(subteam1,
		subteam2,
		new_relation);
	// �, ���� �����, ��� ����
	if(subteam1 == "player")
	{
		// ������ ����������� ��� ��������� ����������
		m_observers_container.notifySubteamChangedRelationWithPlayer(subteam2);
	}

	GameEvMessenger::GetInst()->Notify(GameObserver::EV_RELCHANGED,0);

	REL_LOG("Relation for [" << subteam1 << "] and [" << subteam2 << "] changed from ["
		<< relToText(old_relation) << "] to [" << relToText(new_relation) << "]\n");
}

// ������� ���������� 2 �� ��������� ������ � ������ ���������� 1
void EnemyDetector::clearRelation(const std::string& subteam1, const std::string& subteam2)
{
	// ���� ����� �� ��������� ��� � ����� ������� - ������ ������ �� ����
	if(m_subteams.find(subteam1) == m_subteams.end()) return;
	if(m_subteams.find(subteam2) == m_subteams.end()) return;

	// ��������, ���� �� ����� ������ ���������� 1 ���������� 2
	if(m_relations[m_subteams[subteam1]].find(m_subteams[subteam2]) != m_relations[m_subteams[subteam1]].end())
	{
		// ����, ������ ������
		m_relations[m_subteams[subteam1]].erase(m_subteams[subteam2]);
		GameEvMessenger::GetInst()->Notify(GameObserver::EV_RELCHANGED,0);
	}

	// ��������, ���� �� ����� ������ ���������� 1 ���������� 2
	if(m_friends[m_subteams[subteam1]].find(m_subteams[subteam2]) != m_friends[m_subteams[subteam1]].end())
	{
		// ����, ������ ������
		m_friends[m_subteams[subteam1]].erase(m_subteams[subteam2]);
		GameEvMessenger::GetInst()->Notify(GameObserver::EV_RELCHANGED,0);
	}
}

//=====================================================================================//
//                               static void doOutput()                                //
//=====================================================================================//
static void doOutput(std::ostringstream &s, int &x, int &y)
{
	const int limy = 40;
	const int stepx = 140;
	const int stepy = 14;

	DebugInfo::Add(150 + stepx*x, 25 + stepy*y, (char *)s.str().c_str());
	s.str(std::string());
	s.clear();

	if(++y >= limy)
	{
		++x;
		y = 0;
	}
}

//=====================================================================================//
//                                 struct TeamBySecond                                 //
//=====================================================================================//
struct TeamBySecond
{
	int m_t;
	TeamBySecond(int t) : m_t(t) {}

	template<typename T>
	bool operator()(const T &t)
	{
		return t.second == m_t;
	}
};

//=====================================================================================//
//                       void EnemyDetector::debugOutRelations()                       //
//=====================================================================================//
void EnemyDetector::debugOutRelations()
{
	int x = 0;
	int y = 0;

	//DebugInfo::Add(150, 25, "ENEMY");
	//DebugInfo::Add(150+stepx, 25, "FRIEND");

	for(subteam_map_t::iterator i = m_subteams.begin(); i != m_subteams.end(); ++i)
	{
		int team1 = i->second;

		std::ostringstream sstr;
		sstr << i->first;

		doOutput(sstr,x,y);

		if(m_relations.find(team1) != m_relations.end())
		{
			for(subteam_set_t::iterator j = m_relations[team1].begin(); j != m_relations[team1].end(); ++j)
			{
				subteam_map_t::iterator s = std::find_if(m_subteams.begin(),m_subteams.end(),TeamBySecond(*j));
				sstr << "X " << s->first;
				doOutput(sstr,x,y);
			}
		}

		if(m_friends.find(team1) != m_relations.end())
		{
			for(subteam_set_t::iterator j = m_friends[team1].begin(); j != m_friends[team1].end(); ++j)
			{
				subteam_map_t::iterator s = std::find_if(m_subteams.begin(),m_subteams.end(),TeamBySecond(*j));
				sstr << "+ " << s->first;
				doOutput(sstr,x,y);
			}
		}

		/*
		for(subteam_map_t::iterator j = m_subteams.begin(); j != m_subteams.end(); ++j)
		{
			int team2 = j->second;

			RelationType rt = RT_NEUTRAL;

			// �� �����
			if(!m_relations[team1].empty())
			{
				// �������� ���� �� ����� ����� ���������� snum2 ����� ������ snum1
				subteam_set_t::iterator it = m_relations[team1].find(team2);
				if(it != m_relations[team1].end())
				{
					// ������ ����, - �� ���� 
					rt = RT_ENEMY;
				}
			}
			
			// �� �����
			if(!m_friends[team1].empty())
			{
				// �������� ���� �� ����� ����� ���������� he ����� ������ player
				subteam_set_t::iterator it = m_friends[team1].find(team2);
				if(it != m_friends[team1].end())
				{
					// ������ ����, - �� ���� player'a
					rt = RT_FRIEND;
				}
			}

			if(rt == RT_NEUTRAL) continue;

			std::ostringstream sstr;
			sstr << i->first << " - " << j->first;

			int rti = (int)rt;

			DebugInfo::Add(150 + stepx*(rti-1), 25 + stepy*y[rt], (char *)sstr.str().c_str());
			++y[rt];
		}
		*/

		doOutput(sstr,x,y);
	}
}

// ������� ���������� 2 ������ ���������� 1
void EnemyDetector::setEnemyRelation(const std::string& subteam1, const std::string& subteam2)
{
	// ���� ����� �� ��������� ��� � ����� ������� - ������ ������ �� ����
	if(m_subteams.find(subteam1) == m_subteams.end()) return;
	if(m_subteams.find(subteam2) == m_subteams.end()) return;

	// ��������, ���� �� ����� ������ ���������� 1 ���������� 2
	if(m_relations[m_subteams[subteam1]].find(m_subteams[subteam2]) == m_relations[m_subteams[subteam1]].end())
	{
		// ���, ������� ������
		m_relations[m_subteams[subteam1]].insert(m_subteams[subteam2]);
		GameEvMessenger::GetInst()->Notify(GameObserver::EV_RELCHANGED,0);
	}
}

// ������� ���������� 2 ������ ���������� 1
void EnemyDetector::setFriendRelation(const std::string& subteam1, const std::string& subteam2)
{
	// ���� ����� �� ��������� ��� � ����� ������� - ������ ������ �� ����
	if(m_subteams.find(subteam1) == m_subteams.end()) return;
	if(m_subteams.find(subteam2) == m_subteams.end()) return;

	// ��������, ���� �� ����� ������ ���������� 1 ���������� 2
	if(m_friends[m_subteams[subteam1]].find(m_subteams[subteam2]) == m_friends[m_subteams[subteam1]].end())
	{
		// ���, ������� ������
		m_friends[m_subteams[subteam1]].insert(m_subteams[subteam2]);
		GameEvMessenger::GetInst()->Notify(GameObserver::EV_RELCHANGED,0);
	}
}

// �������� ��� ���������� �� �� ���������� �����
std::string EnemyDetector::getName(const std::string& sys_name)
{
	return m_convert_map[sys_name];
}

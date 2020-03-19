/**************************************************************                 

Virtuality                                         

(c) by MiST Land 2000                                    
---------------------------------------------------                     
Description: �������� ��������� AI


Author:   Alexander Garanin (Punch)
Mikhail L. Lepakhin (Flif)
***************************************************************/                
#pragma warning(disable:4786)

#include "logicdefs.h"
#include "PathUtils.h"
#include "HexUtils.h"
#include "HexGrid.h"
#include "Activity.h"
#include "AIBase.h"
#include "AIUtils.h"
#include "thing.h"
#include "aicontext.h"
#include "strategy.h"
#include "Entity.h"
#include "player.h"
#include "DirtyLinks.h"
#include "Graphic.h"
//#include "EnemyAI.h"
#include "CivilianAI.h"
#include "xlsreader.h"
#include <undercover/interface/console.h>

#include "AlarmistNode.h"
#include "FixedEnemyNode.h"
#include "PatrolEnemyNode.h"
#include "AssaultEnemyNode.h"
#include "FixedTechEnemyNode.h"
#include "PatrolTechEnemyNode.h"
#include "AssaultTechEnemyNode.h"

DCTOR_IMP(BusNode)
DCTOR_IMP(CivilRoot)
DCTOR_IMP(EnemyRoot)
DCTOR_IMP(FixedNPCNode)
DCTOR_IMP(FeralCarNode)
DCTOR_IMP(CivilianNode)

namespace AIBase
{
}

using namespace AIBase;

namespace AIBase
{
	const char* ai_xls_name = "scripts/spawn/ai.txt";

	enum ai_column_type
	{
		ACT_LABEL,
		ACT_TYPE,
		ACT_SUBTEAM,

		ACT_MAX_COLUMNS,            
	};

	column ai_columns[ACT_MAX_COLUMNS] = 
	{
		column(  "label",    ACT_LABEL),
		column(   "type",    ACT_TYPE),
		column("subteam",    ACT_SUBTEAM),
	};

	// ������� � �������
	void Print(const std::string& str);
};

//=====================================================================================//
//                               CivilRoot::CivilRoot()                                //
//=====================================================================================//
CivilRoot::CivilRoot()
{
	Spawner::GetInst()->Attach(this, ET_ENTITY_SPAWN);
	GameEvMessenger::GetInst()->Attach(this, EV_TURN);
	GameEvMessenger::GetInst()->Attach(this, EV_KILL);
	GameEvMessenger::GetInst()->Attach(this, EV_DISAPPEAR);
}

//=====================================================================================//
//                               CivilRoot::~CivilRoot()                               //
//=====================================================================================//
CivilRoot::~CivilRoot()
{
	Spawner::GetInst()->Detach(this);
	GameEvMessenger::GetInst()->Detach(this);
}

//=====================================================================================//
//                              void CivilRoot::Update()                               //
//=====================================================================================//
void CivilRoot::Update(Spawner* spawner, SpawnObserver::event_t type, SpawnObserver::info_t info)
{
	BaseEntity* entity = static_cast<spawn_info_s*>(info)->m_entity;

	if(entity && entity->GetPlayer() == PT_CIVILIAN)
	{
		if(boost::shared_ptr<AINode> node = CreateNode(entity))
		{
			insert(node);
		}
	}
}

//=====================================================================================//
//                              void CivilRoot::Update()                               //
//=====================================================================================//
void CivilRoot::Update(GameObserver::subject_t subj, GameObserver::event_t type, GameObserver::info_t ptr)
{
	switch(type)
	{
	case EV_TURN:
		{
			if(static_cast<turn_info*>(ptr)->m_player == PT_CIVILIAN)
			{
				resetState();
			}
		}
		break;

	case EV_DISAPPEAR:
	case EV_KILL:
		{
			kill_info* info = static_cast<kill_info*>(ptr);
			deleteChildren(info->m_victim->GetEID());
		}
		break;
	}
}

//=====================================================================================//
//                           void CivilRoot::MakeSaveLoad()                            //
//=====================================================================================//
void CivilRoot::MakeSaveLoad(SavSlot& st)
{
	//��������/����������� ��������� AI
	if(st.IsSaving())
	{
		for(Iterator_t itor = begin(); itor != end(); ++itor)
		{
			st << 1;

			DynUtils::SaveObj(st, itor->get());
			(*itor)->MakeSaveLoad(st);
		}
		st << 0;
	}
	else
	{
		int flag;

		st >> flag;
		while(flag)
		{
			AINode* node = 0;
			DynUtils::LoadObj(st, node);
			node->MakeSaveLoad(st);

			insert(Ptr_t(node));
			st >> flag;
		}
	}
}

//=====================================================================================//
//                              void CivilRoot::CutWord()                              //
//=====================================================================================//
void CivilRoot::CutWord(std::string& word, std::string& str)
{
	//������ �������
	unsigned int k = 0;
	for(; k < str.size() && (isspace(str[k]) || str[k] == ')' || str[k] == '(' || str[k] == ','); k++);
	str.erase(str.begin(), str.begin()+k);

	word.clear();
	for(k = 0; k < str.size() && iscsym(str[k]); k++){
		word += str[k];
	}

	str.erase(str.begin(), str.begin()+k);
}

//=====================================================================================//
//                              void CivilRoot::ToLower()                              //
//=====================================================================================//
void CivilRoot::ToLower(std::string& str)
{
	for(unsigned int i = 0; i < str.size(); i++){
		str[i] = tolower(str[i]);
	}
}

//=====================================================================================//
//                  boost::shared_ptr<AINode> CivilRoot::CreateNode()                  //
//=====================================================================================//
CivilRoot::Ptr_t CivilRoot::CreateNode(BaseEntity* entity)
{
	std::string param;
	std::string subteam;
	getAINodeDesc(entity->GetEntityContext()->GetAIModel(),
		param,
		subteam);

	if((subteam != "none") && (subteam != ""))
	{
		entity->SetAIContext(AIContextFactory::GetInst()->Create(AIContextFactory::AI_CIVILIAN, subteam));
	}

	if(param.empty()) return Ptr_t();

	ToLower(param);

	//���� �������, �� ���� ������� ���� ����������, ���� ��������� ���,
	// ���� �������� ���, ���� ��������� ���
	if(entity->Cast2Vehicle())
	{

		std::string schema;
		CutWord(schema, param);

		if(schema == "bus")
		{
			//������ �������
			std::string line;
			CutWord(line, param);

			//������ ��� ��������
			std::string type;
			CutWord(type, param);

			return Ptr_t(new BusNode(entity->GetEID(), DirtyLinks::CreateBusRoute(line), type == "nonstop" ?  BusNode::BT_BUSY : BusNode::BT_USUAL));
		}

		if(schema == "car")
		{
			std::string line;
			CutWord(line, param);

			return Ptr_t(new FeralCarNode(entity->GetEID(), DirtyLinks::CreateCheckPoints(line)));
		}

		if(schema == "a_bot")
		{
			return Ptr_t(new AdvertisingBotNode(entity->GetEID()));
		}

		if(schema == "t_bot")
		{
			return Ptr_t(new TradingBotNode(entity->GetEID()));
		}

		if(schema == "r_bot")
		{
			return Ptr_t(new RepairBotNode(entity->GetEID()));
		}
	}

	//���� ������� �� ���� ������ ���� ������ ��� �����.
	if(entity->Cast2Trader())
	{
		std::string schema;
		CutWord(schema, param);

		if(schema == "fixednpc") return Ptr_t(new FixedNPCNode(entity->GetEID()));
	}

	//���� �������, �� ���� �������, ���� �������������, ���� �������������
	if(entity->Cast2Human())
	{
		std::string schema;
		CutWord(schema, param);

		if(schema == "setednpc")
		{
			std::string line;
			CutWord(line, param);

			return Ptr_t(new CivilianNode(entity->GetEID(), DirtyLinks::CreateCheckPoints(line)));
		}

		if(schema == "randomnpc")
		{
			std::string radius;
			CutWord(radius, param);

			return Ptr_t(new CivilianNode(entity->GetEID(), DirtyLinks::CreateCirclePoints(entity, atoi(radius.c_str()))));
		}

		if(schema == "fixednpc") return Ptr_t(new FixedNPCNode(entity->GetEID()));

		if(schema == "indifferent") return Ptr_t(new IndifferentNode(entity->GetEID()));

		if(schema == "alarmist") return Ptr_t(new AlarmistNode(entity->GetEID()));
	}

	return Ptr_t();
}

//=====================================================================================//
//                       float CivilRoot::getComplexity() const                        //
//=====================================================================================//
// ������ ������ ��������� ����
//float CivilRoot::getComplexity() const
//{
//	float complexity = 0.0f;
//
//	// �������� ����� ����� ���������� ���������
//	for(ConstIterator_t i = begin(); i != end(); ++i)
//	{
//		complexity += (*i)->getComplexity();
//	}
//
//	return complexity;
//}

//=====================================================================================//
//                               EnemyRoot::EnemyRoot()                                //
//=====================================================================================//
EnemyRoot::EnemyRoot()
{
	GameEvMessenger::GetInst()->Attach(this, EV_TURN);
	GameEvMessenger::GetInst()->Attach(this, EV_KILL);
	GameEvMessenger::GetInst()->Attach(this, EV_DISAPPEAR);
	Spawner::GetInst()->Attach(this, ET_ENTITY_SPAWN);
}

//=====================================================================================//
//                               EnemyRoot::~EnemyRoot()                               //
//=====================================================================================//
EnemyRoot::~EnemyRoot()
{
	Spawner::GetInst()->Detach(this);
	GameEvMessenger::GetInst()->Detach(this);
}

//=====================================================================================//
//                              float EnemyRoot::Think()                               //
//=====================================================================================//
//float EnemyRoot::Think(state_type* st)
//{
//	killDeadOnes();
//
//	if(st == 0) return m_complexity;
//
//	//���� ������ ��� ���� �����
//	if(m_cur == end())
//	{     
//		*st = ST_FINISHED;
//		Print("EnemyRoot: ��� ������ ��������.");
//		return m_complexity;
//	}
//
//	//����� �������� �������� ����
//	float tmp_complexity = (*m_cur)->Think(st);
//
//	//���� ���� �������� �������� � ����������
//	if(*st == ST_FINISHED)
//	{
//		m_complexity += (*m_cur)->getComplexity();
//		tmp_complexity = 0.0f;
//		++m_cur;
//	}
//
//	*st = ST_INCOMPLETE;
//	return m_complexity + tmp_complexity;
//}

namespace AIBase
{

	//=====================================================================================//
	//                               struct SubteamNamePred                                //
	//=====================================================================================//
	struct SubteamNamePred
	{
		std::string m_name; 

		SubteamNamePred(const std::string &name) : m_name(name) {}

		bool operator()(const boost::shared_ptr<SubteamNode> &node) const
		{
			return node->getName() == m_name;
		}
	};

}

//=====================================================================================//
//                              void EnemyRoot::Update()                               //
//=====================================================================================//
//��������� ����������� ���������
void EnemyRoot::Update(Spawner* spawner, SpawnObserver::event_t type, SpawnObserver::info_t info)
{
	BaseEntity* entity = static_cast<spawn_info_s*>(info)->m_entity;

	if(entity && entity->GetPlayer() == PT_ENEMY)
	{
		// ���� ������ ��� ������� ������� ��� ��������������
		std::string type;
		std::string subteam;
		getAINodeDesc(entity->GetEntityContext()->GetAIModel(),
			type,
			subteam);

		// �������� ����� ���������������� ����
		if(boost::shared_ptr<AIEnemyNode> node = CreateNode(entity, type))
		{
			// ��������, ���������� �� ����������, � ������� ����� ���������� ����
			Iterator_t it = std::find_if(begin(),end(),SubteamNamePred(subteam));
			if(it == end())
			{
				// ����� ������� ��� ��� - �������� ��
				it = insert(Ptr_t(new SubteamNode(subteam)));
			}

			// ������ ������� ����� ����������, ������� ����� � ��� �������
			entity->SetAIContext(AIContextFactory::GetInst()->Create(AIContextFactory::AI_CIVILIAN, subteam));
			node->setSubteam(it->get());
			(*it)->addNode(node);
		}
	}
}

//=====================================================================================//
//                              void EnemyRoot::Update()                               //
//=====================================================================================//
//��������� ������ ����
void EnemyRoot::Update(GameObserver::subject_t subj, GameObserver::event_t type, GameObserver::info_t ptr)
{
	switch(type)
	{
	case EV_TURN:
		{
			if(static_cast<turn_info*>(ptr)->m_player == PT_ENEMY)
			{
				resetState();
			}
		}
		break;

	case EV_KILL:
	case EV_DISAPPEAR:
		{
			kill_info* info = static_cast<kill_info*>(ptr);
			deleteChildren(info->m_victim->GetEID());
		}
		break;
	}
}

//=====================================================================================//
//                        AIEnemyNode* EnemyRoot::CreateNode()                         //
//=====================================================================================//
//������� ���� AI �� ������ ����������
boost::shared_ptr<AIEnemyNode> EnemyRoot::CreateNode(BaseEntity* entity, const std::string type)
{
	typedef boost::shared_ptr<AIEnemyNode> result_t;

	if(type == "fixed")
	{
		// ��� ���� ������������� ����
		return result_t(new FixedEnemyNode(entity->GetEID()));
	}
	if(type == "patrol")
	{
		// ��� ���� ����������� ����
		return result_t(new PatrolEnemyNode(entity->GetEID()));
	}
	if(type == "assault")
	{
		// ��� ���������
		return result_t(new AssaultEnemyNode(entity->GetEID()));
	}
	if(type == "fixedtech")
	{
		// ��� ��������� ������� ������������� ����
		return result_t(new FixedTechEnemyNode(entity->GetEID()));
	}
	if(type == "patroltech")
	{
		// ��� ��������� ������� ����������� ����
		return result_t(new PatrolTechEnemyNode(entity->GetEID()));
	}
	if(type == "assaulttech")
	{
		// ��� ��������� ������� ���������� ����
		return result_t(new AssaultTechEnemyNode(entity->GetEID()));
	}
	if(type.find("seeker_") == 0)
	{
		std::string sysname = type.substr(strlen("seeker_"));
		return result_t(new AssaultEnemyNode(entity->GetEID(),sysname));
	}

	return result_t();
}

//=====================================================================================//
//                           void EnemyRoot::MakeSaveLoad()                            //
//=====================================================================================//
void EnemyRoot::MakeSaveLoad(SavSlot& st)
{
	//��������/����������� ��������� AI
	if(st.IsSaving())
	{
		// �����������
		st << size();
		for(Iterator_t it = begin(); it != end(); ++it)
		{
			st << (*it)->getName();
			DynUtils::SaveObj(st, it->get());
			(*it)->MakeSaveLoad(st);
		}
	}
	else
	{
		// ��������
		int num;
		st >> num;
		for(int i = 0; i < num; i++)
		{
			SubteamNode* node = 0;
			std::string str;
			st >> str;
			DynUtils::LoadObj(st, node);
			node->MakeSaveLoad(st);
			insert(Ptr_t(node));
		}
	}
}

//=====================================================================================//
//                       float EnemyRoot::getComplexity() const                        //
//=====================================================================================//
// ������ ������ ��������� ����
//float EnemyRoot::getComplexity() const
//{
//	float complexity = 0.0f;
//
//	// �������� ����� ����� ���������� ���������
//	for(ConstIterator_t i = begin(); i != end(); ++i)
//	{
//		complexity += (*i)->getComplexity();
//	}
//	return complexity;
//}

//////////////////////////////////////////////////////////////////////////////
//
// ������������ ���� ��� ����� - ��� �������-������ ����� �����
//
//////////////////////////////////////////////////////////////////////////////
namespace AIBase
{
	//
	// �������� �� ����� �������� ���� AI
	//
	void getAINodeDesc(const std::string& label, std::string& type, std::string& subteam)
	{
		TxtFilePtr txt(ai_xls_name);

		txt.ReadColumns(ai_columns, ai_columns + ACT_MAX_COLUMNS);
		std::string str;

		// ������ ������ � ����� ������
		for(int i = 0; i < txt->SizeY(); ++i)
		{
			txt->GetCell(i, ai_columns[ACT_LABEL].m_index, str);
			if(str == label)
			{
				// �����
				txt->GetCell(i, ai_columns[ACT_TYPE].m_index, type);
				txt->GetCell(i, ai_columns[ACT_SUBTEAM].m_index, subteam);
				return;
			}
		}

		// �� �����
		str = "getAINodeDesc: �� ������� ����� ";
		str += label;
		throw CASUS(str);
	}
	// ������� � �������
	void Print(const std::string& str)
	{
		Console::AddString(str.c_str());
	}
};


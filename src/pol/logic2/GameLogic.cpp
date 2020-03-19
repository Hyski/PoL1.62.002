#pragma warning(disable:4786)

#include "logicdefs.h"

#include "form.h"
#include "spawn.h"
#include "Thing.h"
#include "AIBase.h"
#include "Marker.h"
#include "VisMap.h"
#include "entity.h"
#include "sscene.h"
#include "Player.h"    
#include "damager.h" 
#include "Graphic.h"
#include "HexGrid.h"
#include "AIUtils.h"
#include "Strategy.h"
#include "SndUtils.h"
#include "Spectator.h"
#include "cameraman.h"
#include "pathutils.h"
#include "GameLogic.h"
#include "DirtyLinks.h"
//#include "QuestServer.h"
//#include "phrasemanager.h"
#include "entityfactory.h"
#include "enemydetection.h"

#include "Bureau.h"
#include "DeadList.h"
#include "GameObjectsMgr.h"

#include "ExperienceSaver.h"

#include <undercover/options/options.h>
#include <logic_ex/ActMgr.h>

#include <boost/function.hpp>
#include <logic_ex/UnlimitedMovesMgr.h>

#include <common/D3DApp/Input/Input.h>

#include <mmsystem.h>

/////////////////////////////////////////////////////////////////////////
//	Naughty: 14.05.02 для фиксирования бага с Helper'ом
#include <undercover/interface/screens.h>
/////////////////////////////////////////////////////////////////////////

//~~~~~~~~~~~~~ test ~~~~~~~~~~~~~~~~~
#include "HexGrid.h"
#include "GameObserver.h"
//~~~~~~~~~~~~~ test ~~~~~~~~~~~~~~~~~

#include <boost/random.hpp>

static boost::minstd_rand randgen;
static boost::uniform_01<boost::minstd_rand,float> uniformrand(randgen);
static const float randNumMax = static_cast<float>(boost::minstd_rand::max_value);

#ifndef _HOME_VERSION
struct MakeSeed
{
	MakeSeed()
	{
		randgen.seed(time(0));
	}
};
static MakeSeed g_seedMaker;
#endif

int aiRand()
{
	return randgen();
}

int aiRandMax()
{
	assert( boost::minstd_rand::min_value == 0 );
	return boost::minstd_rand::max_value;
}

float aiNormRand()
{
	return uniformrand();
}

int aiRangeRand(int range)
{
	return boost::uniform_int<int>(0,range-1)(randgen);
}

namespace DynUtils
{

	namespace Detail
	{
		struct CtorLess
		{
			bool operator()(const DynCtor *l, const DynCtor *r) const
			{
				return strcmp(l->GetClass(),r->GetClass()) < 0;
			}
			bool operator()(const char *l, const DynCtor *r) const
			{
				return strcmp(l,r->GetClass()) < 0;
			}
			bool operator()(const DynCtor *l, const char *r) const
			{
				return strcmp(l->GetClass(),r) < 0;
			}
		};

		typedef std::vector<DynCtor*> Ctors_t;

		DynCtor *m_cached = 0;

		Ctors_t &getCtors()
		{
			static Ctors_t g_ctors;
			return g_ctors;
		}
	}
}

static const char* level_file_name = "./saves/levels.dat";

//=====================================================================================//
//                        static void SaveLoadData2LevelFile()                         //
//=====================================================================================//
static void SaveLoadData2LevelFile(GenericStorage::storage_mode save_load_mode)
{
	try
	{
		Win32File win_file(level_file_name, OPEN_ALWAYS);

		LevelFile lvl_file(&win_file, DirtyLinks::GetLevelRawSysName(), 
			save_load_mode == GenericStorage::SM_SAVE 
			?   LevelFile::OM_WRITE
			:   LevelFile::OM_READ);

		GenericStorage strg(&lvl_file, save_load_mode);

		Bureau::GetInst()->MakeSaveLoad(SavSlot(&strg, "level_bureau_data"));
		GameObjectsMgr::GetInst()->MakeSaveLoad(SavSlot(&strg, "level_objects_data"));
		DeadList::GetVehiclesList()->MakeSaveLoad(SavSlot(&strg, "level_vehicles_dead_list"));
		DeadList::GetRespawnerDeadList()->MakeSaveLoad(SavSlot(&strg, "level_all_dead_list"));
	}
	catch(CasusImprovisus &)
	{
	}
}

//=====================================================================================//
//                                 DynCtor::DynCtor()                                  //
//=====================================================================================//
DynUtils::DynCtor::DynCtor(new_func_t func, const char* cls)
:	m_func(func),
m_class(cls)
{
	using namespace Detail;

	Ctors_t::iterator i = std::lower_bound(getCtors().begin(),getCtors().end(),this,CtorLess());
	getCtors().insert(i,this);
}

//=====================================================================================//
//                       DynUtils::DynCtor *DynUtils::FindCtor()                       //
//=====================================================================================//
DynUtils::DynCtor *DynUtils::DynCtor::FindCtor(const char *cls)
{
	using namespace Detail;
	if(m_cached && !strcmp(m_cached->m_class, cls)) return m_cached;

	Ctors_t::iterator i = std::lower_bound(getCtors().begin(),getCtors().end(),cls,CtorLess());
	if(strcmp((*i)->m_class,cls)) return 0;
	return m_cached = *i;
}


//========================================================

namespace GameLogicDetail
{
	const float lower_load_bar_bound = 0.5f;
	const float upper_load_bar_bound = 1.0f;

	typedef std::auto_ptr<AbstractProgressBar> bar_ptr_t;

	//послать уведоление всем существам
	void NotifyAllEntities(EntityObserver::event_type event, EntityObserver::info_t info = 0)
	{
		EntityPool::iterator itor = EntityPool::GetInst()->begin();
		while(itor != EntityPool::GetInst()->end()){
			itor->Notify(event, info);
			++itor;
		}
	}

	void EnableGraphDestroy(bool flag)
	{
		GraphThing::EnableDestroy(flag);

		if(flag)
			GraphEntity::RaiseFlags(GraphEntity::FT_ENABLE_DESTROY);
		else
			GraphEntity::DropFlags(GraphEntity::FT_ENABLE_DESTROY);
	}

	//перелинковать сущетcва
	void RelinkAllEntities()
	{
		EntityBuilder builder;
		EntityPool::iterator itor = EntityPool::GetInst()->begin();

		while(itor != EntityPool::GetInst()->end()){
			builder.LinkEntity(&*itor, itor->GetGraph()->GetPos2(), itor->GetGraph()->GetAngle());
			++ itor;
		}
	}

	//установить стойку для всех людей
	void SetAllHumanStance()
	{
		EntityPool::iterator itor = EntityPool::GetInst()->begin(ET_HUMAN, PT_PLAYER|PT_ENEMY);
		while(itor != EntityPool::GetInst()->end()){
			AIUtils::ChangeHumanStance(itor->Cast2Human());
			++itor;
		};
	}

	//удалить существа из массива существ
	template <class Condition>
		void DestroyEntities(Condition cond)
	{
		EntityPool::iterator itor = EntityPool::GetInst()->begin();

		for(; itor != EntityPool::GetInst()->end(); ++itor)
		{
			if(cond(&*itor))
			{
				itor->GetGraph()->Destroy();
				EntityPool::GetInst()->Remove(&*itor);
			}
		}
	}

	//расчитать ходы для всех существо на начало игры
	void CalcMovepoints4All()
	{
		EntityPool::iterator itor = EntityPool::GetInst()->begin();
		while(itor != EntityPool::GetInst()->end()){
			AIUtils::CalcMovepnts4Turn(&*itor);
			++itor;
		}
	}

	//
	//условие для удаления по типу и игроку сущетва
	//
	class AttributeCondition{
	public:

		AttributeCondition(unsigned type = ET_ALL_ENTS,
			unsigned team = PT_ALL_PLAYERS,
			unsigned attr = EA_ALL_ATTRIBUTES) :
		m_type(type), m_team(team), m_attr(attr){}

		bool operator()(BaseEntity* entity)
		{
			return      entity->GetType() & m_type                
				&&  entity->GetPlayer() & m_team
				&&  entity->GetAttributes() & m_attr;
		}           

	private:

		unsigned m_type;
		unsigned m_team;
		unsigned m_attr;
	};

	//
	// условие для удаления роботов
	//
	class RobotCondition {
	public:

		bool operator()(BaseEntity* entity)
		{
			VehicleEntity* vehicle = entity->Cast2Vehicle();
			return vehicle && vehicle->GetInfo()->IsRobot();
		}
	};

	//========================================================

	//
	// абстракция progress_bar
	//
	class SaveLoadBar : public AbstractProgressBar{
	public:

		SaveLoadBar(AbstractProgressBar* bar) : m_bar(bar){}
		~SaveLoadBar(){ delete m_bar; }

		//установить движок + текстовую метку
		void SetPos(float value, const std::string& text = std::string())
		{ if(m_bar) m_bar->SetPos(value, text); }

	private:

		AbstractProgressBar* m_bar;
	};

	//========================================================

	//
	// класс для вывода сообщений об изменении соотношений сил
	//
	class PrintForceRelationChange : private RelationsObserver{
	public:

		PrintForceRelationChange()
		{ EnemyDetector::getInst()->attach(this, ET_RELATIONS_CHANGED); }

		~PrintForceRelationChange()
		{ EnemyDetector::getInst()->detach(this); }

	private:

		void changed(event_type et, void* info)
		{        
			RelationDesc* ptr = static_cast<RelationDesc*>(info);

			if(ptr->isFirstPlayer()) return;

			std::string res_str = "unknown_relation";

			switch(ptr->m_relation_type){
	case RT_NEUTRAL:
		res_str = "lgc_neutrals";
		break;

	case RT_ENEMY:
		res_str = "lgc_enemies";
		break;

	case RT_FRIEND:
		res_str = "lgc_friends";
		break;

			} 

			DirtyLinks::Print(mlprintf(DirtyLinks::GetStrRes(res_str).c_str(),
				EnemyDetector::getInst()->getName(ptr->m_first_subteam).c_str(),
				EnemyDetector::getInst()->getName(ptr->m_second_subteam).c_str()));
		}
	};

	//========================================================

	//
	// Отложенная инициализация квестов
	//
	class InitQuestServer : private SpawnObserver{
	public:

		InitQuestServer() : m_sheme(ST_NEXT_LEVEL)
		{
			Spawner::Attach(this, ET_ENTRY_LEVEL);
			Spawner::Attach(this, ET_FINISH_SPAWN);
		}

		~InitQuestServer()
		{
			Spawner::GetInst()->Detach(this);
		}

	private:

		void Update(subject_t subj, event_t event, info_t info)
		{
			switch(event){
	case ET_ENTRY_LEVEL:
		{
			entry_info_s* ptr = static_cast<entry_info_s*>(info);

			if(ptr->m_entry.empty())
				m_sheme = ST_NEW_LEVEL;
			else if(ptr->m_fnew_episode)            
				m_sheme = ST_NEW_EPISODE;
			else 
				m_sheme = ST_NEXT_LEVEL;
		}
		break;

	case ET_FINISH_SPAWN:
		{
			switch(m_sheme){
	case ST_NEXT_LEVEL:
		break;

	case ST_NEW_LEVEL:
		// gvozdoder:old_quests: QuestServer::GetInst()->Init(QuestServer::ST_START_NEW_GAME, Spawner::GetInst()->GetEpisode());
		break;

	case ST_NEW_EPISODE:
		// gvozdoder:old_quests: QuestServer::GetInst()->Init(QuestServer::ST_CHANGE_EPISODE, Spawner::GetInst()->GetEpisode());
		break;
			}
		}
		break;
			}
		}

	private:

		enum sheme_type{
			ST_NEW_LEVEL,
			ST_NEXT_LEVEL,
			ST_NEW_EPISODE
		};

		sheme_type m_sheme;
	};

	//========================================================

	//
	// класс с обычным алгоритмом входа на уровень
	//
	class UsualLevelEnter : public SpawnObserver{
	public:

		UsualLevelEnter()  { Spawner::Attach(this, ET_ENTRY_LEVEL); }
		~UsualLevelEnter() { Spawner::Detach(this); }

	private:

		void Update(Spawner* spawner, event_t type, info_t ptr)
		{
			entry_info_s* info = static_cast<entry_info_s*>(ptr);

			if(info->m_entry.empty())
				Init(ST_NEW_GAME);
			else if(info->m_fnew_episode)            
				Init(ST_NEW_EPISODE);

			GraphUtils::DestroyCorpses();

			if(info->m_entry.size())
			{
				for(EntityPool::iterator i = EntityPool::GetInst()->begin(ET_HUMAN,PT_PLAYER); i != EntityPool::GetInst()->end(); ++i)
				{
					if(HumanEntity *he = i->Cast2Human())
					{
						if(VehicleEntity *ve = he->GetEntityContext()->GetCrew())
						{
							ve->GetEntityContext()->DecCrew(he);
						}
					}
				}

				DestroyEntities(RobotCondition());
				DestroyEntities(AttributeCondition(ET_ALL_ENTS, ~PT_PLAYER));
				DestroyEntities(AttributeCondition(~ET_HUMAN));

				//перелинкуем существа
				EntityPool::iterator itor = EntityPool::GetInst()->begin();
				while(itor != EntityPool::GetInst()->end())
				{
					ipnt2_t pos;

					if(!info->m_builder->GenerateSuitablePos(info->m_entry, &pos, itor->GetVehicleSize()))
					{
						throw CASUS("UsualLevelEnter: не хватет точек входа на уровень!");
					}

					info->m_builder->LinkEntity(&*itor, pos, PIm2*aiNormRand());

					info->m_builder->SendSpawnEvent(&*itor);

					++itor;
				}

				AIUtils::CalcAndShowActiveExits();
			}

			Bureau::GetInst()->HandleLevelEntry();
			Forms::GetInst()->Reset(Forms::RT_NEXT_LEVEL);
			EpisodeMapMgr::GetInst()->Insert(DirtyLinks::GetLevelRawSysName());
		}

		enum sheme_type{
			ST_NEW_GAME,
			ST_NEW_EPISODE,
		};

		void Init(sheme_type type)
		{
			if(type == ST_NEW_GAME){

				Forms::GetInst()->Reset(Forms::RT_SHUT);

				MoneyMgr::GetInst()->Reset();

				EnemyDetector::getInst()->init();
				ForceHandbook::GetInst()->Reset();
				ThingHandbook::GetInst()->Reset();
				EntityHandbook::GetInst()->Reset();

				DiaryManager::GetInst()->Delete(DiaryManager::ROOT_KEY);

				GameEvMessenger::GetInst()->Notify(GameObserver::EV_NEW_GAME);
				Spawner::GetInst()->SetDifficulty(Options::GetInt("game.type"));
			}

			EpisodeMapMgr::GetInst()->Reset();

			DestroyEntities(AttributeCondition(ET_ALL_ENTS, type == ST_NEW_GAME ? PT_ALL_PLAYERS: ~PT_PLAYER));

			AIUtils::CalcAndShowActiveExits();
		}  
	};

	//========================================================

	//
	// класс для отслеживания progress bar'а
	//
	class EntityLoadProgressBar : public GameObserver{
	public:

		EntityLoadProgressBar(AbstractProgressBar* bar, float from, float to) :
		  m_from(from), m_to(to), m_delta(0), m_bar(bar)
		  {
			  GameEvMessenger::GetInst()->Attach(this, EV_INIT_ENTLOAD);
			  GameEvMessenger::GetInst()->Attach(this, EV_ENITITY_LOADED);

			  if(m_bar) m_bar->SetPos(from, DirtyLinks::GetStrRes("lgc_prepare_load"));
		  }

		  ~EntityLoadProgressBar(){ GameEvMessenger::GetInst()->Detach(this); }

	private:

		void Update(subject_t subj, event_t event, info_t info)
		{
			switch(event){
	case EV_ENITITY_LOADED:
		m_bar->SetPos(m_from += m_delta, mlprintf(
			DirtyLinks::GetStrRes("lgc_load_entities").c_str(),
			static_cast<entity_info*>(info)->m_entity->GetInfo()->GetName().c_str()));
		break;

	case EV_INIT_ENTLOAD:
		m_delta = (m_to - m_from)/static_cast<entload_info*>(info)->m_count;
		break;
			}
		}

	private:

		float m_to;
		float m_from;
		float m_delta;

		AbstractProgressBar* m_bar;
	};

	//========================================================

	//
	// класс для отслеживания progress bar'а
	//
	class SpawnProgressBar : public SpawnObserver{
	public:

		SpawnProgressBar(bar_ptr_t& bar, float from, float to) :
		  m_from(from), m_to(to), m_delta(0), m_bar(bar)
		  {
			  Spawner::Attach(this, ET_ENTITY_SPAWN);
			  Spawner::Attach(this, ET_PREPARE_SPAWN);

			  m_bar->SetPos(from, DirtyLinks::GetStrRes("lgc_prepare_spawn"));
		  }

		  ~SpawnProgressBar()
		  {
			  Spawner::Detach(this);
		  }

	private:

		void Update(Spawner* spawner, event_t event, info_t ptr)
		{
			switch(event){
	case ET_ENTITY_SPAWN:
		m_bar->SetPos(m_from += m_delta, mlprintf(
			DirtyLinks::GetStrRes("lgc_entity_spawned").c_str(),
			static_cast<spawn_info_s*>(ptr)->m_entity->GetInfo()->GetName().c_str()));
		break;

	case ET_PREPARE_SPAWN:
		{
			prepare_info_s* info = static_cast<prepare_info_s*>(ptr);
			m_delta = (m_to - m_from)/(info->m_humans + info->m_traders + info->m_vehicles + CountEntities(ET_ALL_ENTS));
		}            
		break;
			}
		}

		//подсчитать кол-во сущетсв опред типа
		int CountEntities(unsigned etype)
		{
			int count = 0;

			EntityPool::iterator itor = EntityPool::GetInst()->begin(etype);
			while(itor != EntityPool::GetInst()->end()){
				++count;
				++itor;
			}

			return count;
		}

	private:

		float m_to;
		float m_from;
		float m_delta;

		bar_ptr_t& m_bar;
	};

	//========================================================

	//
	// класс для отслеживания данных зависимых от уровня
	//
	class UsualLevelData : public SpawnObserver{
	public:

		UsualLevelData()
		{ 
			Spawner::Attach(this, ET_EXIT_LEVEL);
			Spawner::Attach(this, ET_ENTRY_LEVEL);
		}

		~UsualLevelData()
		{
			Spawner::Detach(this);
		}

		void MakeSaveLoad(SavSlot& slot)
		{
			Win32File win_file(level_file_name, slot.IsSaving() ? OPEN_ALWAYS : CREATE_ALWAYS);        
			LevelFile lvl_file(&win_file, DirtyLinks::GetLevelRawSysName(), LevelFile::OM_READ);
			lvl_file.MakeSaveLoad(slot);
		}

	private:

		void Update(Spawner* spawner, event_t type, info_t info)
		{
			switch(type)
			{
			case ET_EXIT_LEVEL:
				SaveLoadData2LevelFile(GenericStorage::SM_SAVE);
				break;

			case ET_ENTRY_LEVEL:
				if(     static_cast<entry_info_s*>(info)->m_entry.empty()
					||  static_cast<entry_info_s*>(info)->m_fnew_episode)
				{
					//уничтожим файл с данными об уровнях
					Win32File file_killer(level_file_name, CREATE_ALWAYS);
				}
				else
				{
					SaveLoadData2LevelFile(GenericStorage::SM_LOAD);
				}
				break;
			}
		}
	};

	//========================================================

	//
	// запрещение расстановки убитых существ
	//
	class DoNotSpawnKilledEntityRule : public Spawner::Rule
	{
	public:

		DoNotSpawnKilledEntityRule(DeadList* lst, entity_type ent_type) :
		  m_list(lst), m_entity(ent_type) {}

		  bool IsSuitable(const SpawnTag& tag) const
		  { 
			  return     tag.GetEntityType() == m_entity
				  &&  m_list->GetKilledCount(tag);
		  }

		  int CalcCount(const SpawnTag& tag, int xls_count){ return 0; }

	private:

		DeadList*   m_list;
		entity_type m_entity;
	};

	//
	// запрещение расстановки существ присутсв. в команде игрока
	//
	class DoNotSpawnTeammateRule : public Spawner::Rule,
		private SpawnObserver
	{
	public:

		DoNotSpawnTeammateRule(entity_type ent_type, player_type player) :
		  m_ent_type(ent_type)
		  {
			  //составим список всех перешедших с нами на уровень
			  EntityPool::iterator itor = EntityPool::GetInst()->begin(m_ent_type, player);
			  while(itor != EntityPool::GetInst()->end()){
				  m_peoples.insert(itor->GetInfo()->GetRID());
				  ++itor;
			  }

			  Spawner::GetInst()->Attach(this, ET_ENTRY_LEVEL);
		  }

		  ~DoNotSpawnTeammateRule() { Spawner::GetInst()->Detach(this); }

		  bool IsSuitable(const SpawnTag& tag) const
		  { 
			  return     tag.GetEntityType() == m_ent_type
				  &&  m_peoples.count(tag.GetSysName());
		  }

		  int CalcCount(const SpawnTag& tag, int xls_count){ return 0; }

	private:

		void Update(Spawner* spawner, event_t type, info_t info)
		{ 
			if(static_cast<entry_info_s*>(info)->m_entry.empty())
				m_peoples.clear();
		}

	private:

		rid_set_t   m_peoples;
		entity_type m_ent_type; 
	};

	//
	// Запрещение расстановки техники
	//
	class DoNotSpawnKilledVehiclesRule : public Spawner::Rule
	{
	public:

		DoNotSpawnKilledVehiclesRule(DeadList* deads) : m_deads(deads) {}

		bool IsSuitable(const SpawnTag& tag) const
		{ 
			return      tag.GetEntityType() == ET_VEHICLE
				&&  m_deads->GetKilledCount(tag);
		}

		int CalcCount(const SpawnTag& tag, int xls_count)
		{
			rec_s* record = FindRec(tag);

			//если такой записи еще нет создадим ее
			if(record == 0){
				m_records.push_back(rec_s(tag));
				record = &m_records.back();
			}

			record->m_req4spawn += xls_count;
			int ret = record->m_req4spawn - m_deads->GetKilledCount(tag);

			//нормировка
			if(ret < 0) ret = 0;

			return ret > xls_count ? xls_count : ret;
		}

	private:

		struct rec_s;
		rec_s* FindRec(const SpawnTag& tag)
		{
			rec_lst_t::iterator itor = m_records.begin();
			while(itor != m_records.end()){

				if(     itor->m_sys_name == tag.GetSysName()
					&&  itor->m_ai_model == tag.GetAIModel()
					&&  itor->m_spawn_zone == tag.GetSpawnZone())
					return &(*itor);

				++itor;
			}

			return 0;
		}

	private:

		struct rec_s{
			std::string m_sys_name;
			std::string m_ai_model;
			std::string m_spawn_zone;

			int m_req4spawn;

			rec_s() {}

			rec_s(const SpawnTag& tag, int count = 0) : 
			m_req4spawn(count), m_spawn_zone(tag.GetSpawnZone()),
				m_ai_model(tag.GetAIModel()), m_sys_name(tag.GetSysName()){}
		};

		typedef std::list<rec_s> rec_lst_t;
		rec_lst_t m_records;

		DeadList* m_deads;
	};

	//
	// управление количеством людей для расстановки в зависимости от сложности игры
	// Grom
	class DifficultyRespawnerRule : public Spawner::Rule
	{
	public:

		DifficultyRespawnerRule(DeadList* lst) :
		  m_list(lst) {}

		  bool IsSuitable(const SpawnTag& tag) const
		  { 
			  return   true||m_list->GetKilledCount(tag);
		  }

		  int CalcCount(const SpawnTag& tag, int xls_count)
		  {
			  float cA,cB;
			  switch(Spawner::GetInst()->GetDifficulty())
			  {
			  case Spawner::D_EASIEST:
				  cA = 0.4f;
				  cB = 0.0f;
				  break;
			  case Spawner::D_EASY:
				  cA = 0.6f;
				  cB = 0.3f;
				  break;
			  case Spawner::D_NORMAL:
				  cA = 0.8f;
				  cB = 0.5f;
				  break;
			  default:
			  case Spawner::D_HARD:
				  cA = 1.f;
				  cB = 1.f;
				  break;
			  }
			  float to_burn = ceil(cA*xls_count);
			  float killed = m_list->GetKilledCount(tag);
			  return  std::max(static_cast<int>(to_burn - killed),
				  static_cast<int>(floor(to_burn * cB))
				  );		
		  }

	private:

		DeadList*   m_list;
	};

	//========================================================

	//
	// системная обработка игровых событий
	//
	class UsualGameObserver : public GameObserver{
	public:

		UsualGameObserver()
		{
			GameEvMessenger::GetInst()->Attach(this, EV_TURN);
			GameEvMessenger::GetInst()->Attach(this, EV_KILL);
		}

		~UsualGameObserver()
		{  
			GameEvMessenger::GetInst()->Detach(this);
		}

		void Update(subject_t subj, event_t event, info_t info)
		{
			if(event == EV_TURN) OnEvent(static_cast<turn_info*>(info));
			if(event == EV_KILL) OnEvent(static_cast<kill_info*>(info));
		}

	private:

		void OnEvent(turn_info* info)
		{
			DirtyLinks::SendTurnEvent(info->m_player, info->IsBegining());
		}

		void OnEvent(kill_info* info)
		{
			if(info->m_event != KEV_KILL) return;

			//уведомить квесты
			// gvozdoder:old_quests: QuestServer::GetInst()->HandleKillEntity(info->m_killer, info->m_victim);

			std::ostringstream ostr;

			//выыедем сообщение об умирании
			DirtyLinks::Print( mlprintf( DirtyLinks::GetStrRes("lgc_death").c_str() , info->m_victim->GetInfo()->GetName().c_str()));

			//изменим стойку людей кот на нас смотрели
			VisMap::SpecIterator_t itor = VisMap::GetInst()->SpecBegin(info->m_victim->GetEntityContext()->GetMarker(), VisParams::vpVisible);
			for(;itor != VisMap::GetInst()->SpecEnd(); ++itor)
			{
				if(HumanEntity* human = itor->GetEntity() ? itor->GetEntity()->Cast2Human() : 0)
				{
					AIUtils::ChangeHumanStance(human, info->m_victim);
				}
			}

			//обработаем смерть союзника
			if(HumanEntity* human = info->m_victim->Cast2Human())
			{
				std::string vtype, vforce;
				AIBase::getAINodeDesc(info->m_victim->GetEntityContext()->GetAIModel(),vtype,vforce);
				EntityPool::iterator itor = EntityPool::GetInst()->begin(ET_HUMAN);

				for(; itor != EntityPool::GetInst()->end(); ++itor)
				{
					std::string type, force;
					AIBase::getAINodeDesc(itor->GetEntityContext()->GetAIModel(),type,force);
					if(force == vforce)
					{
						AIUtils::CalcHumanAllyDeath(&*itor);
					}
				}
			}

			int experience = 0;

			if(HumanEntity* human = info->m_victim->Cast2Human())
				experience = human->GetInfo()->GetExp4Kill();
			else if(VehicleEntity* vehicle = info->m_victim->Cast2Vehicle())
				experience = vehicle->GetInfo()->GetExp4Kill();

			//обработаем смерть врага
			if(info->m_killer && info->m_killer->GetPlayer() == PT_PLAYER)
				AIUtils::AddExp4Kill(experience, info->m_killer);
		}
	};

	//========================================================

	//
	// смена музыки в зависимости от врагов
	//
	class EnemyMusicChanger : public  MusicChanger,
		private GameObserver,
		private SpawnObserver,
		private SpectatorObserver,
		private RelationsObserver
	{
	public:

		EnemyMusicChanger() :
		  m_flags(F_TURN_ON|F_UPDATE), m_danger(0), m_duration(0)
		  {
			  Spawner::Attach(this, ET_EXIT_LEVEL);
			  Spawner::Attach(this, ET_ENTRY_LEVEL);
		  }

		  ~EnemyMusicChanger()
		  {
			  Shut();
			  Spawner::Detach(this);
		  }

		  //нужно ли сменить музыку?
		  bool NeedUpdate() const { return m_flags & F_UPDATE; }

		  //функция анализа игровой ситуации
		  music_type GetMusic()
		  {
			  if(UnlimitedMoves())
			  {
				  m_danger = 0;
				  m_duration = 0;
				  m_flags &= ~F_UPDATE;
				  return MT_QUIET;
			  }

			  if(m_flags & F_TURN_ON && m_flags & F_UPDATE)
			  {
				  int danger_points = 0;

				  VisMap::MarkIterator_t itor = VisMap::GetInst()->MarkBegin(PlayerSpectator::GetInst()->GetSpectator(), VisParams::vpVisible);
				  for(;itor != VisMap::GetInst()->MarkEnd(); ++itor)
				  {
					  BaseEntity* entity = itor->GetEntity();

					  if(entity &&  EnemyDetector::getInst()->getRelationBetweenPlayerAndHe(entity) == RT_ENEMY)
					  {
						  if(HumanEntity* human = entity->Cast2Human())
						  {
							  danger_points += human->GetInfo()->GetDangerPoints();
						  }
						  else if(VehicleEntity* vehicle = entity->Cast2Vehicle())
						  {
							  danger_points += vehicle->GetInfo()->GetDangerPoints();
						  }
					  }
				  }     

				  if(m_danger < danger_points)
				  {
					  m_danger = danger_points;
					  m_duration = THEME_DURATION;
				  }

				  m_flags &= ~F_UPDATE;
			  }

			  if(m_danger > STRAINED_LIM) return MT_STRAINED;
			  if(m_danger > UNEASY_LIM) return MT_UNEASY;

			  return MT_QUIET;
		  }

		  void MakeSaveLoad(SavSlot& slot)
		  {
			  if(slot.IsSaving())
			  {
				  slot << m_flags;
				  slot << m_danger;
				  slot << m_duration;
			  }
			  else
			  {
				  slot >> m_flags;
				  slot >> m_danger;
				  slot >> m_duration;

				  Init();
			  }
		  }

	private:

		void Update(Spectator* spectator, SpectatorObserver::event_t event, SpectatorObserver::info_t info)
		{ 
			m_flags |= F_UPDATE;
		}

		void changed(RelationsObserver::event_type et, void* info) 
		{ 
			EntityDesc* desc = static_cast<EntityDesc*>(info);

			BaseEntity* entity = EntityPool::GetInst()->Get(static_cast<EntityDesc*>(info)->m_id);
			VisParams flags = VisMap::GetInst()->GetVisFlags(PlayerSpectator::GetInst()->GetSpectator(), entity->GetEntityContext()->GetMarker());
			if(flags.IsVisible()) m_flags |= F_UPDATE;
		}

		void Update(GameObserver::subject_t subj, GameObserver::event_t event, GameObserver::info_t ptr)
		{
			switch(event)
			{
			case EV_TURN:
				{
					turn_info* info = static_cast<turn_info*>(ptr);

					if(info->m_player != PT_PLAYER)
						return;

					m_flags &= ~(F_TURN_OFF|F_TURN_ON);
					m_flags |= F_UPDATE;

					if(info->IsBegining())
					{
						if(--m_duration <= 0) m_danger = 0;
						m_flags |= F_TURN_ON;
					}

					if(info->IsEnd()) m_flags |= F_TURN_OFF;
					break;
				}
			case EV_UNLIMITED_MOVES_STARTED:
				m_flags |= F_UPDATE;
				break;
			}
		}

		void Update(SpawnObserver::subject_t subj, SpawnObserver::event_t event, SpawnObserver::info_t ptr)
		{
			if(event == ET_EXIT_LEVEL) Shut();
			if(event == ET_ENTRY_LEVEL) Init();
		}

		void Init()
		{
			GameEvMessenger::GetInst()->Attach(this, EV_TURN);
			GameEvMessenger::GetInst()->Attach(this, EV_UNLIMITED_MOVES_STARTED);
			EnemyDetector::getInst()->attach(this, ET_RELATION2PLAYER_CHANGED);

			PlayerSpectator::GetInst()->GetSpectator()->AttachSpectatorObserver(this, EV_VISIBLE_MARKER);
			PlayerSpectator::GetInst()->GetSpectator()->AttachSpectatorObserver(this, EV_INVISIBLE_MARKER);

			JukeBox::GetInst()->Reset();    
			JukeBox::GetInst()->SetChanger(this);
		}

		void Shut()
		{
			JukeBox::GetInst()->Reset();        

			EnemyDetector::getInst()->detach(this);
			GameEvMessenger::GetInst()->Detach(this);        
			PlayerSpectator::GetInst()->GetSpectator()->DetachSpectatorObserver(this);
		}

	private:

		enum {

			F_UPDATE   = 1 << 0,
			F_TURN_ON  = 1 << 1,
			F_TURN_OFF = 1 << 2,

			UNEASY_LIM   = 50,
			STRAINED_LIM = 100,

			THEME_DURATION = 3,
		};

		int m_danger;
		int m_duration; 

		unsigned m_flags;
	};

	//========================================================

	//
	// класс для конфигурирования экрана HiddenMovement
	//
	class HiddenMovementConfigurator : private GameObserver
	{
	public:

		HiddenMovementConfigurator()
		{
			GameEvMessenger::GetInst()->Attach(this, EV_TURN);
			GameEvMessenger::GetInst()->Attach(this, EV_START_GAME);
		}

		~HiddenMovementConfigurator() { GameEvMessenger::GetInst()->Detach(this); }

	private:

		void Update(subject_t subj, event_t event, info_t info)
		{
			if(event == EV_START_GAME){
				Cameraman::GetInst()->Configure(Cameraman::F_SHOW_NOTHING);
				return;
			}

			if(event == EV_TURN){

				turn_info* ptr = static_cast<turn_info*>(info);
				if(ptr->IsBegining()) Cameraman::GetInst()->Configure(Config4Player(ptr->m_player));
				return;
			}
		}

		unsigned Config4Player(player_type player)
		{
			if(player == PT_ENEMY || player == PT_CIVILIAN)
			{
				unsigned result = Cameraman::F_SHOW_ALL & (~Cameraman::F_SHOW_ALL_OPTIONS);

				if(DirtyLinks::GetIntOpt(DirtyLinks::OT_HIDDEN_MOVEMENT_ENEMY_RELATED_MOVE))
					result |= Cameraman::F_SHOW_ENEMY_RELATED_MOVE;

				if(DirtyLinks::GetIntOpt(DirtyLinks::OT_HIDDEN_MOVEMENT_ENEMY_RELATED_SHOOT))
					result |= Cameraman::F_SHOW_ENEMY_RELATED_SHOOT;

				if(DirtyLinks::GetIntOpt(DirtyLinks::OT_HIDDEN_MOVEMENT_FRIEND_RELATED_MOVE))
					result |= Cameraman::F_SHOW_FRIEND_RELATED_MOVE;

				if(DirtyLinks::GetIntOpt(DirtyLinks::OT_HIDDEN_MOVEMENT_FRIEND_RELATED_SHOOT))
					result |= Cameraman::F_SHOW_FRIEND_RELATED_SHOOT;

				if(DirtyLinks::GetIntOpt(DirtyLinks::OT_HIDDEN_MOVEMENT_NEUTRAL_RELATED_MOVE))
					result |= Cameraman::F_SHOW_NEUTRAL_RELATED_MOVE;

				if(DirtyLinks::GetIntOpt(DirtyLinks::OT_HIDDEN_MOVEMENT_NEUTRAL_RELATED_SHOOT))
					result |= Cameraman::F_SHOW_NEUTRAL_RELATED_SHOOT;

				if(DirtyLinks::GetIntOpt(DirtyLinks::OT_HIDDEN_MOVEMENT_CIVILIAN_VEHICLES_MOVE))
					result |= Cameraman::F_SHOW_CIVILIAN_VEHICLES_MOVE;

				if(DirtyLinks::GetIntOpt(DirtyLinks::OT_HIDDEN_MOVEMENT_CIVILIAN_VEHICLES_SHOOT))
					result |= Cameraman::F_SHOW_CIVILIAN_VEHICLES_SHOOT;

				if(DirtyLinks::GetIntOpt(DirtyLinks::OT_HIDDEN_MOVEMENT_CIVILIAN_HUMANS_TRADERS_MOVE))
					result |= Cameraman::F_SHOW_CIVILIAN_HUMANS_TRADERS_MOVE;

				if(DirtyLinks::GetIntOpt(DirtyLinks::OT_HIDDEN_MOVEMENT_CIVILIAN_HUMANS_TRADERS_SHOOT))
					result |= Cameraman::F_SHOW_CIVILIAN_HUMANS_TRADERS_SHOOT;

				return result;
			}

			return Cameraman::F_SHOW_NOTHING;
		}
	};

	//========================================================

	//
	// интерфейсный класс инкапсулирующий модуль логики
	//
	class LogicFrame{
	public:

		virtual ~LogicFrame(){}

		DCTOR_ABS_DEF(LogicFrame)

			//здесь можно рисовать
			virtual void Draw() = 0;
		//здесь можно думать
		virtual void Think() = 0;
		//сохранение/загрузка
		virtual void MakeSaveLoad(SavSlot& slot) = 0;  
	};

	//========================================================

	//
	// модуль обычной игры
	//
	class UsualLogicFrame : public LogicFrame
	{
	public:
		//создание новой игры
		UsualLogicFrame(int)
		{
			EnableGraphDestroy(true);

			bar_ptr_t bar_ptr(FormFactory::GetInst()->CreateLoadBar(lower_load_bar_bound, upper_load_bar_bound));

			bar_ptr->SetPos(0);

			InitLogic();

			//заполнить массив игроками
			PlayerPool::GetInst()->Insert(PlayerFactory::GetInst()->CreateHumanPlayer());
			PlayerPool::GetInst()->Insert(PlayerFactory::GetInst()->CreateEnemyPlayer());
			PlayerPool::GetInst()->Insert(PlayerFactory::GetInst()->CreateCivilianPlayer());   

			bar_ptr->SetPos(0.1f);

			UsualLevelEnter  usual_enter;
			SpawnProgressBar spawn_bar(bar_ptr, 0.1f, 0.9f);        
			EntityBuilder    spawn_api;
			Spawner::RuleSet spawn_rules;

			DoNotSpawnTeammateRule          rule1(ET_HUMAN, PT_PLAYER);
			DoNotSpawnKilledEntityRule      rule2(DeadList::GetHeroesList(), ET_HUMAN);
			DoNotSpawnKilledEntityRule      rule3(DeadList::GetTradersList(), ET_TRADER);
			DoNotSpawnKilledVehiclesRule    rule4(DeadList::GetVehiclesList());
			DifficultyRespawnerRule         rule5(DeadList::GetRespawnerDeadList());

			spawn_rules.Insert(&rule1);
			spawn_rules.Insert(&rule2);
			spawn_rules.Insert(&rule3);
			spawn_rules.Insert(&rule4);
			spawn_rules.Insert(&rule5);

			//расставить существ на уровне
			Spawner::GetInst()->Spawn(&spawn_api, &spawn_rules);    

			bar_ptr->SetPos(0.91f, DirtyLinks::GetStrRes("lgc_init_entities"));

			SetAllHumanStance();
			CalcMovepoints4All();

			//пусть все существа обновят данные о команде
			NotifyAllEntities(EntityObserver::EV_JOIN_TEAM);

			GameEvMessenger::GetInst()->Notify(GameObserver::EV_START_GAME);

			bar_ptr->SetPos(1.0f);
		}

		~UsualLogicFrame()
		{
		}

		DCTOR_DEF(UsualLogicFrame)

		//здесь можно рисовать
		void Draw()
		{
			GraphGrid::GetInst()->Draw();
			//RiskArea::GetInst()->TestSmth();
			return;

#ifdef _HOME_VERSION

			// ~~~~~~~~~~~~~ test ~~~~~~~~~~~~~
			HexGrid::cell_iterator  first_cell = HexGrid::GetInst()->first_cell(),
				last_cell  = HexGrid::GetInst()->last_cell();

			HexGrid::const_prop_iterator props = HexGrid::GetInst()->first_prop();
			point3   pnt3;
			BBox     piece;

			int counter = 0;

			while(first_cell != last_cell){

				pnt3 = *first_cell;
				piece.Box(pnt3, 0.15);

				//if((counter++ % 50) == 0) first_cell->SetFireDuration(2);

				if( first_cell->IsBusRoute()) DirtyLinks::DrawBBox(piece, 0xff00ff);

				//if(!first_cell->IsDefPnt()) DirtyLinks::DrawBBox(piece, 0xffffff);
				if( first_cell->GetEntity()) DirtyLinks::DrawBBox(piece, 0x00ff00);
				//if( first_cell->IsLandPnt()) DirtyLinks::DrawBBox(piece, 0xff00ff);
				//if(first_cell->IsPassable()) DirtyLinks::DrawBBox(piece, 0x00ffff);
				++first_cell;
			}

#endif

			// ~~~~~~~~~~~~~ test ~~~~~~~~~~~~~    
		}

		//здесь можно думать
		void Think()
		{
			if(HexGrid::GetInst())
			{
				PlayerPool::GetInst()->Think();
				//VisMap::GetInst()->UpdateSpectator(PlayerSpectator::GetInst()->GetSpectator()/*,VisMap::UE_FOS_CHANGE*/);
				//VisMap::GetInst()->CommitRelationChanges();

				/////////////////////////////////////////////////////////////////////////
				//	Naughty: 14.05.02 для фиксирования бага с Helper'ом
				//if(!Screens::Instance()->IsOptionsPresent()) HelperManager::GetInst()->Tick();
				/////////////////////////////////////////////////////////////////////////
			}
			
			GraphEntity::Update();
			JukeBox::GetInst()->Tick();

#ifdef _HOME_VERSION

			//~~~~~~~~~~~~~~ test ~~~~~~~~~~~~~~~

			if(Input::KeyBack(DIK_V))
			{
				static int counter = 0;

				if(++counter % 2)
				{
					GraphEntity::DropFlags(GraphEntity::FT_ALWAYS_VISIBLE);
				}
				else
				{
					GraphEntity::RaiseFlags(GraphEntity::FT_ALWAYS_VISIBLE);
				}
			}

			// gvozdoder:old_quests: if(Input::KeyBack(DIK_T)) QuestServer::GetInst()->TestSmth(); 

#endif // _HOME_VERSION
		}

		struct load_notifier{

			load_notifier(SavSlot& slot) : m_fload(!slot.IsSaving())
			{ if(m_fload) GameEvMessenger::GetInst()->Notify(GameObserver::EV_INIT_GAME_LOAD); }

			~load_notifier()
			{ if(m_fload) GameEvMessenger::GetInst()->Notify(GameObserver::EV_START_GAME); }

		private:

			bool m_fload;
		};

		//сохранение/загрузка
		void MakeSaveLoad(SavSlot& slot)
		{
			load_notifier notifier(slot);

			SaveLoadBar load_bar(slot.IsSaving() ? 0 : FormFactory::GetInst()->CreateLoadBar(0.5f, 1.0f));

			//запретиь удаление графич оболочек, чтобы не убить уже загруженные
			EnableGraphDestroy(false);

			load_bar.SetPos(0);

			{
				EntityLoadProgressBar ent_load_bar(&load_bar, 0, 0.4f);

				//если загрузка обычной игры созданим каркас заново
				if(!slot.IsSaving())
				{
					Forms::GetInst()->Reset(Forms::RT_SHUT);
					InitLogic();
					DestroyEntities(AttributeCondition());
				}

				EntityPool::GetInst()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_entities"));
				if(!slot.IsSaving()) RelinkAllEntities();  
			}

			load_bar.SetPos(0.40f, DirtyLinks::GetStrRes("lgc_load_level_data"));

			//разберемся с данными об уровнях
			m_level_data.MakeSaveLoad(SavSlot(slot.GetStore(), "logic_level_data"));
			Bureau::GetInst()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_bureau"));

			load_bar.SetPos(0.46f);

			NewsPool::GetInst()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_news"));
			MoneyMgr::GetInst()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_money"));
			DiaryManager::GetInst()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_diary"));

			load_bar.SetPos(0.52f);

			EpisodeMapMgr::GetInst()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_levels"));        
			CameraPool::GetInst()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_camera_pool"));
			ShieldPool::GetInst()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_shield_pool"));

			load_bar.SetPos(0.58f);

			GameObjectsMgr::GetInst()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_objects_mgr"));
			// gvozdoder:phrasemgr: PhraseManager::GetFirstPhrases()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_first_phrase"));
			EnemyDetector::getInst()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_enemy_detector"));

			load_bar.SetPos(0.64f, DirtyLinks::GetStrRes("lgc_load_players_data"));

			PlayerSpectator::GetInst()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_player_spectator"));
			Depot::GetInst()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_depot"));
			PlayerPool::GetInst()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_players"));

			load_bar.SetPos(0.70f);

			if(slot.GetStore()->GetVersion() > 3)
				ForceHandbook::GetInst()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_force_handbook"));
			else
				ForceHandbook::GetInst()->Reset();

			ThingHandbook::GetInst()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_thing_handbook"));
			EntityHandbook::GetInst()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_entity_handbook"));
			DeadList::GetHeroesList()->MakeSaveLoad(SavSlot(slot.GetStore(), "level_heroes_dead_list"));

			load_bar.SetPos(0.76f);

			DeadList::GetTradersList()->MakeSaveLoad(SavSlot(slot.GetStore(), "level_traders_dead_list"));
			DeadList::GetVehiclesList()->MakeSaveLoad(SavSlot(slot.GetStore(), "level_vehicles_dead_list"));
			DeadList::GetRespawnerDeadList()->MakeSaveLoad(SavSlot(slot.GetStore(), "level_all_dead_list"));

			load_bar.SetPos(0.84f);

			// gvozdoder:old_quests: QuestServer::GetInst()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_quests"));       
			// HelperManager::GetInst()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_helper"));

			load_bar.SetPos(0.90f, DirtyLinks::GetStrRes("lgc_load_menu_data"));

			SSceneMgr::GetInst()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_scr_scenes"));
			Spawner::GetInst()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_spawner"));
			Forms::GetInst()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_menus"));

			load_bar.SetPos(0.96f);

			RiskArea::GetInst()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_risk_area"));
			m_changer.MakeSaveLoad(SavSlot(slot.GetStore(), "logic_music_changer"));

			//разрешить удаление графич оболочек
			EnableGraphDestroy(true); 

			load_bar.SetPos(1.0f, DirtyLinks::GetStrRes("lgc_load_complete"));

			//fixme::grom
			if(!slot.IsSaving())
			{
				GraphGrid::GetInst()->Show(GraphGrid::HT_JOINTS, true);
				AIUtils::CalcAndShowActiveExits();
			}

			GameEvMessenger::GetInst()->Notify(GameObserver::EV_UPDATE_UNLIMITED_VISIBILITY);
		}

	private:

		//загрузка новой игры из Save'a
		UsualLogicFrame() {}

		//создать каркас игры
		void InitLogic()
		{
			GraphGrid::GetInst()->Show(GraphGrid::HT_JOINTS, true);

			BusDispatcher::Reset();

			Depot::GetInst()->Reset();
			Bureau::GetInst()->Reset();
			VisMap::GetInst()->Reset();
			PlayerPool::GetInst()->Reset();
			CameraPool::GetInst()->Reset();
			ShieldPool::GetInst()->Reset();

			GameObjectsMgr::GetInst()->Reset();
			PlayerSpectator::GetInst()->Reset();

			VisMap::GetInst()->Insert(PlayerSpectator::GetInst()->GetSpectator());

			//создать обычную
			Spawner::CreateUsual();
		}

	private:
		EnemyMusicChanger m_changer;
		UsualGameObserver m_game_obs;
		UsualLevelData m_level_data;

		InitQuestServer m_quests;
		PrintForceRelationChange m_relations;

		HiddenMovementConfigurator m_hidden_movent_config;
	};

	DCTOR_IMP(UsualLogicFrame)

}

using namespace GameLogicDetail;

static std::auto_ptr<LogicFrame> g_frame;
static bool g_initialized = false;

//=====================================================================================//
//                               void GameLogic::Init()                                //
//=====================================================================================//
void GameLogic::Init()
{
	if(!g_initialized)
	{
		JukeBox::GetInst()->Init();
		NewsPool::GetInst()->Init();        
		SSceneMgr::GetInst()->Init();
		SndPlayer::GetInst()->Init();
		DeadList::GetHeroesList()->Init();
		DeadList::GetRespawnerDeadList()->Init();
		DeadList::GetTradersList()->Init();
		DeadList::GetVehiclesList()->Init();
		//HelperManager::GetInst()->Init();

		// gvozdoder:phrasemgr: PhraseManager::GetUsePhrases()->Init();
		// gvozdoder:phrasemgr: PhraseManager::GetFirstPhrases()->Init();

		g_initialized = true;
	}
}

//=====================================================================================//
//                               void GameLogic::Shut()                                //
//=====================================================================================//
void GameLogic::Shut()
{
	if(g_initialized)
	{
		g_frame.reset();

		Forms::GetInst()->Reset(Forms::RT_SHUT);

		Depot::GetInst()->Reset();
		Bureau::GetInst()->Reset();
		VisMap::GetInst()->Reset();
		JukeBox::GetInst()->Reset();
		CameraPool::GetInst()->Reset();
		ShieldPool::GetInst()->Reset();

		ForceHandbook::GetInst()->Reset();
		ThingHandbook::GetInst()->Reset();
		EntityHandbook::GetInst()->Reset();

		NewsPool::GetInst()->Shut();
		SSceneMgr::GetInst()->Shut();
		SndPlayer::GetInst()->Shut();
		// gvozdoder:old_quests: QuestServer::GetInst()->Shut();
		//HelperManager::GetInst()->Shut();
		DeadList::GetHeroesList()->Shut();
		DeadList::GetTradersList()->Shut();
		DeadList::GetVehiclesList()->Shut();
		DeadList::GetRespawnerDeadList()->Shut();

		// gvozdoder:phrasemgr: PhraseManager::GetUsePhrases()->Shut();
		// gvozdoder:phrasemgr: PhraseManager::GetFirstPhrases()->Shut();

		//уничтожаем игроков в конце
		PlayerPool::GetInst()->Reset();

		DestroyEntities(AttributeCondition());
		PoL::ExperienceSaver::inst()->reset();

		g_initialized = false;
	}
}

//=====================================================================================//
//                               void GameLogic::Think()                               //
//=====================================================================================//
void GameLogic::Think()
{
	if(PoL::ActMgr::isPlayingScene())
	{
		PoL::ActMgr::play();
	}
	else
	{
		g_frame->Think();
	}
}

//=====================================================================================//
//                               void GameLogic::Draw()                                //
//=====================================================================================//
void GameLogic::Draw()
{
	g_frame->Draw();
}

//=====================================================================================//
//                           void GameLogic::MakeSaveLoad()                            //
//=====================================================================================//
void GameLogic::MakeSaveLoad(GenericStorage& st)
{
	SavSlot slot(&st, "game_logic");

	if(!slot.IsSaving())
	{
		PoL::UnlimitedMovesMgr::instance()->enable(false);
	}

	if(slot.IsSaving())
	{
		DynUtils::SaveObj(slot, &(*g_frame));
	}
	else
	{
		g_frame.reset();

		LogicFrame* frame = 0;
		DynUtils::LoadObj(slot, frame);

		g_frame.reset(frame);
	}

	g_frame->MakeSaveLoad(slot);

	if(!slot.IsSaving())
	{
		PoL::UnlimitedMovesMgr::instance()->enable(true);
	}

	if(slot.IsSaving() || slot.GetStore()->GetVersion() >= 16)
	{
		PoL::ExperienceSaver::inst()->MakeSaveLoad(slot);
	}
}

//=====================================================================================//
//                            void GameLogic::BegNewGame()                             //
//=====================================================================================//
void GameLogic::BegNewGame()
{
	g_frame.reset();
	g_frame.reset(new UsualLogicFrame(0));
}

//=====================================================================================//
//                            void GameLogic::BegNetGame()                             //
//=====================================================================================//
void GameLogic::BegNetGame()
{
	g_frame.reset();
}


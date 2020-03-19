#include "logicdefs.h"

#include "Entity.h"
#include "DirtyLinks.h"
#include "SSceneMgrImp.h"
//#include "HeroKillGameExit.h"
//#include "ScriptSceneFactory.h"
//#include "KillEntsAndExitGame.h"
//#include "UsualScriptSceneFactory.h"
#include "../logic_ex/ActMgr.h"
//#include "../ScriptScene/ScriptSceneManager.h"

//породить скриптовую сцену для проигрывания
//=====================================================================================//
//                     Activity* SSceneMgrImp::CreateScriptScene()                     //
//=====================================================================================//
//Activity* SSceneMgrImp::CreateScriptScene()
//{
//	//for(size_t k = 0; k < m_factories.size(); k++)
//	//{
//	//	if(Activity* act = m_factories[k]->Create())
//	//	{
//	//		return act;
//	//	}
//	//}
//
//	return 0;
//}

//сохранить скриптовыве сцены
//=====================================================================================//
//                          void SSceneMgrImp::MakeSaveLoad()                          //
//=====================================================================================//
void SSceneMgrImp::MakeSaveLoad(SavSlot& slot)
{ 
	if(slot.IsSaving())
	{
		MakeSave(slot);
	}
	else
	{
		if(slot.GetStore()->GetVersion() >= 7)
		{
			MakeLoad7(slot);
		}
	}
}

//=====================================================================================//
//                            void SSceneMgrImp::MakeSave()                            //
//=====================================================================================//
void SSceneMgrImp::MakeSave(SavSlot &slot)
{
	std::ostringstream sstr;
	PoL::ActMgr::store(sstr);
	slot.Save(sstr.str().c_str(),sstr.str().size());
}

//=====================================================================================//
//                           void SSceneMgrImp::MakeLoad7()                            //
//=====================================================================================//
void SSceneMgrImp::MakeLoad7(SavSlot &slot)
{
	std::string data;
	data.resize(slot.GetSize());
	slot.Load(&data[0],slot.GetSize());
	std::istringstream sstr(data,std::ios::binary);
	PoL::ActMgr::restore(sstr, slot.GetStore()->GetVersion());
}

//=====================================================================================//
//                              void SSceneMgrImp::Init()                              //
//=====================================================================================//
void SSceneMgrImp::Init()
{
    Spawner::GetInst()->Attach(this, ET_EXIT_LEVEL);
    Spawner::GetInst()->Attach(this, ET_POL_ENTRY_LEVEL);
	Spawner::GetInst()->Attach(this, ET_ENTITY_SPAWN);
	Spawner::GetInst()->Attach(this, ET_PHASE_CHANGE);

    GameEvMessenger::GetInst()->Attach(this, EV_USE);
    GameEvMessenger::GetInst()->Attach(this, EV_KILL);
	GameEvMessenger::GetInst()->Attach(this, EV_TURN);
	GameEvMessenger::GetInst()->Attach(this, EV_DESTROY);
	GameEvMessenger::GetInst()->Attach(this, EV_RELCHANGED);
    GameEvMessenger::GetInst()->Attach(this, EV_CANNOT_USE);
    GameEvMessenger::GetInst()->Attach(this, EV_QUEST_FINISHED);
}

//=====================================================================================//
//                              void SSceneMgrImp::Shut()                              //
//=====================================================================================//
void SSceneMgrImp::Shut()
{
	Spawner::GetInst()->Detach(this);
	GameEvMessenger::GetInst()->Detach(this);
}

//=====================================================================================//
//                             void SSceneMgrImp::Update()                             //
//=====================================================================================//
void SSceneMgrImp::Update(GameObserver::subject_t subj,
						  GameObserver::event_t event,
						  GameObserver::info_t info)
{
	PoL::EventData data;
	data.m_level = DirtyLinks::GetLevelSysName();
	data.m_phase = Spawner::GetInst()->GetPhase();
	data.m_episode = Spawner::GetInst()->GetEpisode();

    switch(event)
	{
		case EV_DESTROY:
		{
			entity_info* ptr = static_cast<entity_info*>(info);
			data.m_subject = ptr->m_entity->GetInfo()->GetRID();
			data.m_subject_id = ptr->m_entity->GetEID();

			PoL::ActMgr::onDestroyEntity(data);
			break;
		}
		case EV_RELCHANGED:
		{
			PoL::ActMgr::onRelChanged(data);
			break;
		}
		case EV_USE:
		case EV_CANNOT_USE:
		{
			use_info* ptr = static_cast<use_info*>(info);
			data.m_object = ptr->m_actor->GetInfo()->GetRID();

			if(!ptr->m_obj4use.empty())
			{
				data.m_subject = ptr->m_obj4use;
				PoL::ActMgr::onUseObject(data);
			}
			else
			{
				data.m_subject = ptr->m_ent4use->GetInfo()->GetRID();
				data.m_subject_id = ptr->m_ent4use->GetEID();
				PoL::ActMgr::onUseEntity(data);
			}
			break;
		}
		case EV_KILL:
		{
			kill_info* ptr = static_cast<kill_info*>(info);
			data.m_subject = ptr->m_victim->GetInfo()->GetRID();
			data.m_subject_id = ptr->m_victim->GetEID();

			switch(ptr->m_event)
			{
			case KEV_KILL:
				if(ptr->m_killer)
				{
					data.m_object = ptr->m_killer->GetInfo()->GetRID();
					data.m_object_id = ptr->m_killer->GetEID();
				}
				PoL::ActMgr::onKillEntity(data);
				break;
			case KEV_TEAM_CHANGE:
				//PoL::ActMgr::onEntityTeamChanged(data);
				break;
			}
			break;
		}
		case EV_TURN:
		{
			turn_info *ptr = static_cast<turn_info*>(info);
			if(ptr->m_player == PT_PLAYER)
			{
				if(ptr->IsBegining())
				{
					PoL::ActMgr::onStartTurn(data);
				}
				else if(ptr->IsEnd())
				{
					PoL::ActMgr::onEndTurn(data);
				}
			}
			break;
		}
	}
}

//=====================================================================================//
//                             void SSceneMgrImp::Update()                             //
//=====================================================================================//
void SSceneMgrImp::Update(SpawnObserver::subject_t subj,
						  SpawnObserver::event_t event,
						  SpawnObserver::info_t info)
{
	PoL::EventData data;
	data.m_level = DirtyLinks::GetLevelSysName();
	data.m_phase = Spawner::GetInst()->GetPhase();
	data.m_episode = Spawner::GetInst()->GetEpisode();

	switch(event)
	{
	case ET_EXIT_LEVEL:
		{
			data.m_subject = *reinterpret_cast<const std::string *>(info);
			PoL::ActMgr::onLevelExit(data);
			break;
		}
	case ET_POL_ENTRY_LEVEL:
		{
			entry_info_s *ptr = static_cast<entry_info_s*>(info);
		    
			//если новая игра или новый эпизод
			//if(ptr->m_entry.empty())
			//{
			//	PoL::ActMgr::onStartGame(data);
			//}

			if(ptr->m_fnew_episode)
			{
				PoL::ActMgr::onStartEpisode(data);
			}

			// Обработка прихода на новый уровень
			PoL::ActMgr::onLevelEnter(data);
		}
		break;
	case ET_ENTITY_SPAWN:
		{
			spawn_info_s *ptr = static_cast<spawn_info_s*>(info);
			data.m_subject = ptr->m_entity->GetInfo()->GetRID();
			data.m_subject_id = ptr->m_entity->GetEID();

			if(!ptr->m_team_change)
			{
				PoL::ActMgr::onEntitySpawned(data);
			}
			else
			{
				PoL::ActMgr::onEntityTeamChanged(data);
			}
		}
		break;
	case ET_PHASE_CHANGE:
		{
			PoL::ActMgr::onPhaseChanged(data);
		}
		break;
	}
}

//=====================================================================================//
//                           SSceneMgr* SSceneMgr::GetInst()                           //
//=====================================================================================//
SSceneMgr* SSceneMgr::GetInst()
{
    static SSceneMgrImp imp;
    return &imp;
}

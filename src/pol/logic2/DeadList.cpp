#include "logicdefs.h"
#include "DeadList.h"
#include "GameObserver.h"
#include "Spawn.h"
#include "Entity.h"
#include "AIUtils.h"

namespace DeadListDetails
{
//=====================================================================================//
//                               class IsTraderCondition                               //
//=====================================================================================//
    class IsTraderCondition
	{
    public:
        bool operator()(BaseEntity* entity)
        {
            return entity->Cast2Trader() != 0;
        }
    };

//=====================================================================================//
//                                class IsHeroCondition                                //
//=====================================================================================//
    class IsHeroCondition
	{
	public:
        bool operator()(BaseEntity* entity)
        {
            HumanEntity* human = entity->Cast2Human();

            return      human
                    && (    human->GetInfo()->IsHero()
                        ||  human->GetInfo()->IsAntihero()
                        ||  human->GetInfo()->IsQuestHolder());
        }
    };
}

using namespace DeadListDetails;

//=====================================================================================//
//                                 class DeadListImp1                                  //
//=====================================================================================//
template<class Condition>
class DeadListImp1 : public DeadList, private GameObserver, private SpawnObserver
{
public:
    void Shut() 
    {
        Spawner::GetInst()->Detach(this);
        GameEvMessenger::GetInst()->Detach(this);
    }

    void Init()
    {
        Spawner::GetInst()->Attach(this, ET_ENTRY_LEVEL);
        GameEvMessenger::GetInst()->Attach(this, EV_KILL);
    }

    //добавить существо в список
    void Insert(BaseEntity* entity)
    {
        if(m_condition(entity) && !m_entities.count(entity->GetInfo()->GetRID()))
		{
			m_entities.insert(entity->GetInfo()->GetRID());            
		}
    }

    //сохранить/загрузить данные
    void MakeSaveLoad(SavSlot& slot) { AIUtils::SaveLoadRIDSet(slot, m_entities); }
    
    //есть ли существо в списке
    int GetKilledCount(const SpawnTag& tag) const { return m_entities.count(tag.GetSysName()); }

private:
    
    void Update(GameObserver::subject_t subj, GameObserver::event_t event, GameObserver::info_t info)
    {
        if(static_cast<kill_info*>(info)->IsKill()) Insert(static_cast<kill_info*>(info)->m_victim);
    }

    void Update(SpawnObserver::subject_t subj, SpawnObserver::event_t event, SpawnObserver::info_t info)
    {            
        if(     static_cast<entry_info_s*>(info)->m_entry.empty()
            /*||  static_cast<entry_info_s*>(info)->m_fnew_episode*/)
            m_entities.clear();
    }

private:
    rid_set_t m_entities; 
    Condition m_condition;
};

//=====================================================================================//
//                                class VehicleDeadList                                //
//=====================================================================================//
// список убитой техники
class VehicleDeadList : public DeadList,
                        private GameObserver,
                        private SpawnObserver
{
public:

    void Init()
    {
        Spawner::GetInst()->Attach(this, ET_EXIT_LEVEL);
        Spawner::GetInst()->Attach(this, ET_PREPARE_SPAWN);
        GameEvMessenger::GetInst()->Attach(this, EV_KILL);
    }

    void Shut()
    {
        Spawner::GetInst()->Detach(this);
        GameEvMessenger::GetInst()->Detach(this);
    }

    //добавить существо в список
    void Insert(BaseEntity* entity)
    {   
        VehicleEntity* vehicle = entity->Cast2Vehicle();
        
        //это подходящее существо?
        if(vehicle == 0 || !vehicle->GetInfo()->IsTech())
		{
            return;
		}

        //есть ли такое в списке?
        if(rec_s* rec = FindRec( vehicle->GetInfo()->GetRID(),
                                    vehicle->GetEntityContext()->GetAIModel(),
                                    vehicle->GetEntityContext()->GetSpawnZone()))
		{
            rec->m_count++;
            return;
        }

        //добавиться в список
        m_records.push_back(rec_s(vehicle));
    }

    void MakeSaveLoad(SavSlot& slot)
    {
        if(slot.IsSaving())
		{
            slot << m_records.size();

            rec_lst_t::iterator itor = m_records.begin();
            while(itor != m_records.end())
			{
                itor->MakeSaveLoad(slot);
                ++itor;
            }
        }
		else
		{
            size_t size;
            slot >> size;

            m_records.clear();

            while(size--)
			{
                m_records.push_back(rec_s());                    
                m_records.back().MakeSaveLoad(slot);
            }
        }
    }

    int GetKilledCount(const SpawnTag& tag) const
    { 
        VehicleDeadList* This = const_cast<VehicleDeadList*>(this);

        if(rec_s* rec = This->FindRec(  tag.GetSysName(),
                                        tag.GetAIModel(),
                                        tag.GetSpawnZone()))
            return rec->m_count;

        return 0;
    }

private:
    
    void Update(GameObserver::subject_t subj, GameObserver::event_t event, GameObserver::info_t info)
    {
        if(static_cast<kill_info*>(info)->IsKill()) Insert(static_cast<kill_info*>(info)->m_victim);
    }

    void Update(SpawnObserver::subject_t subj, SpawnObserver::event_t event, SpawnObserver::info_t info)
    {   
        switch(event)
		{
        case ET_EXIT_LEVEL:
            {
                EntityPool::iterator itor = EntityPool::GetInst()->begin(ET_VEHICLE, PT_PLAYER);

                //поместим в список технику которая уходит вместе с нами
                while(itor != EntityPool::GetInst()->end())
				{
                    Insert(&(*itor));
                    ++itor;
                }
            }
            break;

        case ET_PREPARE_SPAWN:
            m_records.clear();
            break;
        }
    }

    struct rec_s;

    rec_s* FindRec(const rid_t& sys_name, const std::string& ai, const std::string& spawn_zone)
    {
        rec_lst_t::iterator itor = m_records.begin();
        while(itor != m_records.end())
		{
            if(     itor->m_sys_name == sys_name 
                &&  itor->m_ai_model == ai
                &&  itor->m_spawn_zone == spawn_zone)
                return &(*itor);

            ++itor;
        }

        return 0;
    }

private:

    //запись об убитой технике
    struct rec_s
	{
        rid_t       m_sys_name;
        std::string m_ai_model;
        std::string m_spawn_zone;
        
        int m_count;

        void MakeSaveLoad(SavSlot& slot)
        {
            if(slot.IsSaving())
			{
                slot << m_count;
                slot << m_sys_name;
                slot << m_ai_model;
                slot << m_spawn_zone;
            }
			else
			{
                slot >> m_count;
                slot >> m_sys_name;
                slot >> m_ai_model;
                slot >> m_spawn_zone;
            }
        }
        
        rec_s() : m_count(0) {}
        
        rec_s(BaseEntity* entity) :
            m_count(1), m_sys_name(entity->GetInfo()->GetRID()),
            m_ai_model(entity->GetEntityContext()->GetAIModel()),
            m_spawn_zone(entity->GetEntityContext()->GetSpawnZone()) {}
    };

    typedef std::list<rec_s> rec_lst_t;
    rec_lst_t   m_records;       
};

//=====================================================================================//
//                               class RespawnerDeadList                               //
//=====================================================================================//
// список убитых всех
class RespawnerDeadList : public DeadList, private GameObserver, private SpawnObserver
{
	struct info
	{
		SpawnTag tag;  //какой тэг
		int		 count;	//сколько убитых
		int		 phase; //в какой фазе

		info() : count(0), phase(0) {}
		info(const SpawnTag &t) : count(0), tag(t), phase(0) {}
		info(const info &t): count(t.count), tag(t.tag), phase(t.phase) {}

        void MakeSaveLoad(SavSlot& slot)
        {
            if(slot.IsSaving())
			{
                slot << count;
				slot<< phase;
                slot << tag.GetSysName();
                slot << tag.GetAIModel();
                slot << tag.GetSpawnZone();
				slot << (unsigned int)tag.GetEntityType();
            }
			else
			{
				slot>>count;
				slot>> phase;
				std::string s;
                slot >> s; tag.SetSysName(s);
                slot >> s; tag.SetAIModel(s);
                slot >> s; tag.SetSpawnZone(s);
				unsigned int et;
				slot >> et; tag.SetEntityType((entity_type)et);
            }
        }
	};

	mutable std::vector<info> m_list;
	unsigned cur_phase;

public:
    void Init()
    {
        Spawner::GetInst()->Attach(this, ET_PREPARE_SPAWN);
        GameEvMessenger::GetInst()->Attach(this, EV_KILL);
    }

    void Shut()
    {
        Spawner::GetInst()->Detach(this);
        GameEvMessenger::GetInst()->Detach(this);
    }

    //добавить существо в список
    void Insert(BaseEntity* entity)
    {   
		add_deceased(entity);
    }

    void MakeSaveLoad(SavSlot& slot)
    {
        if(slot.IsSaving())
		{
			slot<<m_list.size();
			for(unsigned int i=0; i<m_list.size(); i++)
			{
				m_list[i].MakeSaveLoad(slot);
			}
        }
		else
		{
			m_list.clear();
			try
			{
				unsigned int count;
				slot>>count;
				m_list.resize(count);
				for(unsigned int i=0; i<count; i++)
				{
					m_list[i].MakeSaveLoad(slot);
				}
			}
			catch(CasusImprovisus &)
			{
			}
        }
    }

    int GetKilledCount(const SpawnTag& tag) const
    { 
		for(unsigned int i=0; i<m_list.size(); i++)
		{
			info *inf = &m_list[i];

			if( tag.GetSysName() != inf->tag.GetSysName()) continue;
			if(tag.GetSpawnZone() != inf->tag.GetSpawnZone()) continue;
			if(inf->tag.GetAIModel() != tag.GetAIModel()) continue;
			if(tag.GetEntityType() != inf->tag.GetEntityType()) continue;
			if( inf->phase != cur_phase) continue;
			
			return m_list[i].count;
		}

		{
			info i(tag);
			i.phase = cur_phase;
			m_list.push_back(i);
		}
        return 0;
    }

private:
    
    void Update(GameObserver::subject_t subj, GameObserver::event_t event, GameObserver::info_t info)
    {
        if(static_cast<kill_info*>(info)->IsKill()) Insert(static_cast<kill_info*>(info)->m_victim);
    }

    void Update(SpawnObserver::subject_t subj, SpawnObserver::event_t event, SpawnObserver::info_t info)
    {   
        switch(event)
		{
        case ET_PREPARE_SPAWN:
            m_list.clear();
			cur_phase = Spawner::GetInst()->GetPhase();
            break;
        }
    }


private:
	void add_deceased(BaseEntity *who)
	{
		std::string sys_name = who->GetInfo()->GetRID();
		EntityContext *context = who->GetEntityContext();

		for(unsigned int i=0; i<m_list.size(); i++)
		{
			info *inf = &m_list[i];
			
			if( inf->tag.GetSysName() != sys_name ) continue;
			if( inf->tag.GetSpawnZone() != context->GetSpawnZone()) continue;
			if( inf->tag.GetAIModel() != context->GetAIModel()) continue;
			if( inf->phase != cur_phase ) continue;
			
			inf->count++;
			break;
		}
	}
};

//=====================================================================================//
//                     DeadList* DeadList::GetRespawnerDeadList()                      //
//=====================================================================================//
DeadList* DeadList::GetRespawnerDeadList()
{
    static RespawnerDeadList imp;
    return &imp;
}

//=====================================================================================//
//                        DeadList* DeadList::GetTradersList()                         //
//=====================================================================================//
DeadList* DeadList::GetTradersList()
{
    static DeadListImp1<IsTraderCondition> imp;
    return &imp;
}

//=====================================================================================//
//                         DeadList* DeadList::GetHeroesList()                         //
//=====================================================================================//
DeadList* DeadList::GetHeroesList()
{
    static DeadListImp1<IsHeroCondition> imp;
    return &imp;
}

//=====================================================================================//
//                        DeadList* DeadList::GetVehiclesList()                        //
//=====================================================================================//
DeadList* DeadList::GetVehiclesList()
{
    static VehicleDeadList imp;
    return &imp;
}

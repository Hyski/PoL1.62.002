//
// –асстановка существ на уровн€х
//

#ifndef _PUNCH_SPAWN_H_
#define _PUNCH_SPAWN_H_

class Spawner;
class BaseEntity;
class EntityBuilder;

MLL_MAKE_EXCEPTION(SpawnError,CasusImprovisus);
	MLL_MAKE_EXCEPTION(ItemSetError,SpawnError);
		MLL_MAKE_EXCEPTION(NoItemSetError,ItemSetError);
		MLL_MAKE_EXCEPTION(BadItemSetError,ItemSetError);
	MLL_MAKE_EXCEPTION(ZoneError,SpawnError);

//
// интерфейс наблюдател€ за расстановщик
//
class SpawnObserver : public Observer<Spawner*>
{
public:
    enum event_type
	{
        ET_EXIT_LEVEL,		// сообщение о выходе с уровн€
        ET_ENTRY_LEVEL,		// сообщение о входе на уровень
        ET_PHASE_CHANGE,	// сообщение об изменении фазы
        ET_ENTITY_SPAWN,	// сообщение о расстановке существа
        ET_PREPARE_SPAWN,	// сообщение посылаетс€ перед расстановкой существ
        ET_FINISH_SPAWN,	// сообщение посылаетс€ после расстановки существ
		ET_POL_ENTRY_LEVEL,	// ѕохоже на ET_ENTRY_LEVEL, только вызываетс€ после споуна существ
		ET_DESTROY_ACTIVITY	// ”ничтожить активность в GameScreen, если такова€ существует
    };

    //информаци€ дл€ ET_ENTITY_SPAWN
    struct spawn_info_s
	{
        BaseEntity* m_entity;
		bool m_team_change;		// gvozdoder: флажок, показывающий изменение команды
        spawn_info_s(BaseEntity* entity = 0) : m_entity(entity), m_team_change(false) {}
    };

    //информаци€ дл€ ET_PREPARE_SPAWN
    struct prepare_info_s{
        int m_humans;
        int m_traders;
        int m_vehicles;

        prepare_info_s(int humans, int traders, int vehicles) :
            m_humans(humans), m_traders(traders), m_vehicles(vehicles){}
    };

    //информаци€ дл€ ET_ENTRY_LEVEL
    struct entry_info_s{

        bool  m_fnew_episode;

        pnt_vec_t      m_entry;    
        EntityBuilder* m_builder;

        entry_info_s(EntityBuilder* builder, bool fnew_episode = false, pnt_vec_t entry = pnt_vec_t()) :
            m_entry(entry), m_builder(builder), m_fnew_episode(fnew_episode) {}            
    };
};

//
// тег расстановки
//
class SpawnTag{
public:
    
    SpawnTag(entity_type type = ET_NONE) : m_ent_type(type) {}
    
    entity_type GetEntityType() const { return m_ent_type; }
    void SetEntityType(entity_type type) { m_ent_type = type; }
    
    const std::string& GetSysName() const { return m_sys_name; }
    const std::string& GetAIModel() const { return m_ai_model; }
    const std::string& GetSpawnZone() const { return m_spawn_zone; } 
    
    void SetSysName(const std::string& str) { m_sys_name = str; }
    void SetAIModel(const std::string& str) { m_ai_model = str; }
    void SetSpawnZone(const std::string& str) { m_spawn_zone = str; }
    
private:
    
    entity_type m_ent_type;
    
    std::string m_sys_name;
    std::string m_ai_model;
    std::string m_spawn_zone;
};

//
// Ѕазовый класс дл€ всех расcтановщиков
//

class Spawner{
public:
		enum DIFFICULTY {D_EASIEST=0, D_EASY=1, D_NORMAL=2, D_HARD=3};
    virtual ~Spawner(){}

    //создать обычный расстановщик
    static void CreateUsual();

    //работа с наблюдател€ми
    static void Detach(SpawnObserver* observer);
    static void Attach(SpawnObserver* observer, SpawnObserver::event_t event);
    static void Notify(SpawnObserver::event_t event, SpawnObserver::info_t info = 0);

    //получить текущий расстановщик
    static Spawner* GetInst() { return m_instance.get(); }  

    //обработка сохранени€
    virtual void MakeSaveLoad(SavSlot& st) = 0;

    //узнать/установить фазу на уровне
    virtual int GetPhase() const = 0;
    virtual void SetPhase(int val) = 0;

    //узнать текущий эпизод
    virtual int GetEpisode() const = 0;
	//узнать/установить текущую сложность
	virtual int GetDifficulty() const = 0;
	virtual void SetDifficulty(int new_dif) = 0;

    //выйти на другой уровень
    virtual void ExitLevel(const std::string& new_lvl) = 0;
    //выйти на другой уровень
	virtual void ExitLevel(const std::string& new_lvl, const std::string &old_lvl) = 0;
    //можно выйти на другой уровень?
    virtual bool CanExit(unsigned joints, std::string* level) = 0;    
    //разрешить/запретить выход на уровень
    virtual void EnableExit(const std::string& level, bool flag) = 0;

    class RuleSet;
    
    // расставить существ на уровне
    virtual void Spawn(EntityBuilder* builder, RuleSet* rules = 0) = 0;
	// расставить существ на уровне дл€ акта
	virtual void PolSpawn(EntityBuilder* builder, RuleSet* rules = 0) = 0;

    //
    // ѕравило расстановки существ
    // 
    class Rule
	{
    public:
        virtual ~Rule(){}
        
        //рассчитать кол-во существ дл€ расст.
        virtual int CalcCount(const SpawnTag& tag, int xls_count) = 0;

		//подходит ли данное правило
		virtual bool IsSuitable(const SpawnTag& tag) const = 0;
    };
    
    //
    // Ќабор правил дл€ расстановки существ
    //
    class RuleSet
	{
	public:
		//найти нужное правило
		Spawner::Rule *FindRule(const SpawnTag& tag)
		{
			for(rule_lst_t::iterator itor = m_rules.begin(); itor != m_rules.end(); ++itor)
			{
				if((*itor)->IsSuitable(tag))
					return *itor;
			}

			return 0;
		}

		//поместить правило в массив
		void Insert(Spawner::Rule *rule) { m_rules.push_back(rule); }

	private:
		typedef std::list<Spawner::Rule*> rule_lst_t;
		rule_lst_t m_rules;
    };

protected:

    Spawner() {}

private:

    typedef ObserverManager<SpawnObserver> spawn_obsmgr_t;
    static spawn_obsmgr_t m_observers;

    typedef std::auto_ptr<Spawner> spawn_ptr_t;
    static spawn_ptr_t m_instance;
};

#endif // _PUNCH_SPAWN_H_
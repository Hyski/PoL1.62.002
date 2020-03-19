//
// ���������� �� �������� ���������
//

#ifndef _PUNCH_GAMEOBSERVER_H_
#define _PUNCH_GAMEOBSERVER_H_

class BaseEntity;

//
// ����� - ����������� �� �������� ���������
//
class GameObserver : public Observer<int>{
public:

    //������� �������
    enum event_type{

        EV_NONE,

        EV_HIT,       //���� ���������
        EV_TURN,      //�������� ����
        EV_KILL,      //�������� ��������
		EV_DISAPPEAR, //������� �������� (�� ���������� �����)
        EV_HURT,      //������� ��������
        EV_SHOOT,     //���-�� ��������
        EV_USE,       //use 
		EV_DESTROY,   //����������� ��������

		EV_RELCHANGED,//�������� ��������� ����� ���-��

        EV_NEW_NEWS,   //������ ������� �������, ��������� �����
        EV_NEW_GAME,   //������ ����� ����        
        EV_NEW_QUEST,  //������ ������ ������
        EV_NEW_SSCENE, //��������� ����� ���������� �����

        EV_MENU,         //������������ ����
        EV_START_GAME,   //����������� ���� ��� ������ ����
        EV_DIARY_CHANGE, //��������� ������

        EV_INIT_GAME_LOAD,  //���������� � �������� ����
        EV_INIT_ENTLOAD,    //���������� � �������� �������
        EV_ENITITY_LOADED,  //�� Save'a ��������� ��������
        EV_INIT_LEVEL_EXIT, //���������� � ������ � ������

        EV_QUEST_FINISHED,  //�������� �����
		EV_CANNOT_USE,

		EV_UNLIMITED_MOVES_STARTED,	// ��������� ����� �������������� �����
		EV_UNLIMITED_MOVES_ENDED,	// ����� �������������� ����� ����������
		EV_UPDATE_UNLIMITED_VISIBILITY	// �������� ��������� ��� ������ �������������� �����
    };

    //���������� ��� ������ � ������
    struct exit_level_info{
        std::string m_2level;
        exit_level_info(const std::string& level) : m_2level(level) {}        
    };

    //���������� � �������� �������
    struct entload_info{
        int m_count;
        entload_info(int count = 0) : m_count(count) {}
    };

    //����������� � ��������
    struct entity_info{
        BaseEntity* m_entity;
        entity_info(BaseEntity* ent) : m_entity(ent) {}
    };

    //���� ����
    enum menu_type{
        MT_NONE,
        MT_GAME,
        MT_TRADE,
        MT_HIDDEN,
        MT_JOURNAL,
        MT_INVENTORY,
    };

    //���������� � ����
    struct menu_info{
        menu_type   m_menu;
        menu_info(menu_type type = MT_NONE) : m_menu(type){}
    };

    //������� ��������� � �������� ����
    enum turn_event_type{
        TEV_BEG,         //������ ����
        TEV_END,         //����� ����  
    };
   
    //���������� � �������� ����
    struct turn_info {
        player_type     m_player;
        turn_event_type m_event;

        bool IsEnd() const      { return m_event == TEV_END; }
        bool IsBegining() const { return m_event == TEV_BEG; }

        turn_info(player_type player, turn_event_type event) : 
            m_player(player), m_event(event) {}       
    };

    //������� ��������� c ���������
    enum kill_event_type{
        KEV_KILL,        //�������� ��������
        KEV_TEAM_CHANGE, //����� �������
    };

    //���������� �� �������� ��������
    struct kill_info {
        BaseEntity*  m_victim;
        BaseEntity*  m_killer;

        kill_event_type m_event;

        bool IsKill() const { return m_event == KEV_KILL; }

        kill_info(BaseEntity* k, BaseEntity* v, kill_event_type ev = KEV_KILL) :
            m_killer(k), m_victim(v), m_event(ev) {}
    };

    //���������� � ������� ��������
    struct hurt_info {
        BaseEntity* m_victim;
        BaseEntity* m_killer;

        hurt_info(BaseEntity* k, BaseEntity* v) :
            m_killer(k), m_victim(v) {}
    };

    //���������� � ��������
    struct shoot_info{
        BaseEntity* m_actor;  //��� �������
        BaseEntity* m_victim; //����c��� �� �������� ��������
        rid_t       m_object; //������ �� �������� ��������
        point3      m_point;  //����� ��������

        shoot_info(const point3& pnt, BaseEntity* actor = 0, BaseEntity* victim = 0, const rid_t& object = rid_t()) : 
            m_point(pnt), m_actor(actor), m_victim(victim), m_object(object) {}
    };

    //���������� � ���������
    struct hit_info{
        BaseEntity* m_actor;  //��� ����� 
        BaseEntity* m_victim; //�������� �� �������� ������

        point3 m_point;       //����� ���������
        rid_t  m_hit_effect;  //�������� ������� �� ���������
        
        hit_info(BaseEntity* actor, BaseEntity* victim, const point3& point, const rid_t& effect) :
            m_actor(actor), m_victim(victim), m_point(point), m_hit_effect(effect) {}
    };

    //���������� � use'�
    struct use_info{
        BaseEntity* m_actor;
        BaseEntity* m_ent4use;
        rid_t       m_obj4use; 
		bool		m_wasFailed;

        use_info(BaseEntity* actor, BaseEntity* used) :
            m_actor(actor), m_ent4use(used), m_wasFailed(false) {}

        use_info(BaseEntity* actor, const rid_t& rid) :
            m_actor(actor), m_obj4use(rid), m_ent4use(0), m_wasFailed(false) {}
    };

    //���������� �� ���������� � �������
    struct diary_change{

        enum change_type{
            CT_ADD,  //�������� ������
            CT_DEL,  //������� ������   
        };

        change_type m_change_type;

        bool IsAdd() const { return m_change_type == CT_ADD; }
        bool IsDel() const { return m_change_type == CT_DEL; }

        int m_record;

        diary_change(change_type change, int rec) : m_change_type(change), m_record(rec) {}
    };

    //���������� � �������
    struct quest_info{
        std::string m_quest;
        quest_info(const std::string& quest) : m_quest(quest) {}
    };
};

//
// �������� ������� ���������
//
class GameEvMessenger{
public:

    //singleton
    static GameEvMessenger* GetInst();

    //����������� �� �������� ���������
    void Attach(GameObserver* observer, GameObserver::event_t event)
    { m_observers.Attach(observer, event); }

    //���������� �� ���������
    void Detach(GameObserver* observer)
    { m_observers.Detach(observer); }

    //��������� ���� ������������� � �������
    void Notify(GameObserver::event_t event, GameObserver::info_t info = 0)
    { m_observers.Notify(0, event, info); }

private:

    GameEvMessenger() {}

private:

    typedef ObserverManager<GameObserver> obs_mgr_t;   
    obs_mgr_t m_observers;
};

#endif // _PUNCH_GAMEOBSERVER_H_


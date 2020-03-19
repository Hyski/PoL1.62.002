//
// ���������� �������� ����
//

#ifndef _PUNCH_GAMEFORMIMP_H_
#define _PUNCH_GAMEFORMIMP_H_

#include "form.h"
#include "spawn.h"
#include "EntityRef.h"
#include "EntityAux.h"
#include "enemydetection.h"
#include "SpectatorObserver.h"

#include <undercover/interface/gamescreen.h>

class Activity;

class BaseEntity;
class HumanEntity;
class TraderEntity;
class VehicleEntity;

class Marker;
class ShotTracer;
class EntityObserver;

class GameScreenController;
class DialogScreenController;

//
// ����� ��� ������������ ��������  
//
class ActionManager{
public:

    virtual ~ActionManager(){}

    //������� �������� ��� ������� � ����� action'a
    virtual Activity* CreateMove() { return 0; }
    //������� �������� ��� action'a
    virtual Activity* CreateAction() { return 0; }
    //������� ������� ��� use'�
    virtual Activity* CreateRotate() { return 0; }

    //����� �� ��������� � ����� action'�?
    virtual bool NeedMove() const { return false; }
    //����� �� ������������ � ����� action'a?
    virtual bool NeedRotate() const { return false; }

    //��������� action
    virtual void Shut() {}
};

//
// �������������� �������� ��� �����
//
class CursorDevice{
public:

    virtual ~CursorDevice(){}

    enum menu_part
	{
        MP_NONE,    //��� ���������� �����
        MP_WEAPON,  //���� ��� ���� ������
    };

    //������ �������� �� ��� ��������� ����
    virtual BaseEntity* GetEntity() const = 0;
    //������ �������� ������� �� ��� ��������� ����
    virtual bool GetObject(rid_t* str) const = 0;
    //�������� ����� �������������
    virtual bool GetTracePnt(point3* pnt) const = 0;
    //�������� hex � ��� ��������� ����
    virtual bool GetLevelHex(ipnt2_t* pnt) const = 0;
    //������ �� ��� ���������� ���� � ����������
    virtual bool GetMenuPart(menu_part* part) const = 0;

    enum cursor_type
	{
        CT_USE,
        CT_MEDIC,
        CT_SELECT,
        CT_UNLOAD,
        CT_NORMAL,
        CT_PICKUP,
        CT_SPECIAL,
        CT_SHIPMENT,
        CT_MOVE_ENABLE,
        CT_MOVE_DISABLE,
        CT_ATTACK_ENABLE,
        CT_ATTACK_DISABLE,
		CT_SWAP
    };

    enum color_type
	{
        DEFAULT_COLOR = 0x00ff00,
        DENIAL_COLOR  = 0xff0000,
    };

    //��������� ���� �������
    struct text
	{
        std::string m_text;
        unsigned    m_color;

        text(int value, unsigned color = DEFAULT_COLOR)
        {
            m_color = color;
            std::ostringstream ostr;
            ostr << value;
            m_text = ostr.str();
        }

        text(const std::string& text = std::string(), unsigned color = DEFAULT_COLOR)
		:	m_text(text), m_color(color) {}
    };

    //���������� ��������� ���� � ��� �������
    virtual void SetCursor( cursor_type type,
                            const text &t1 = text(),
                            const text &t2 = text(), 
                            const text &t3 = text(),
							const text &t4 = text()) = 0;
};

//
// ��������� �� ������
//
class CursorManager{
public:

    virtual ~CursorManager(){}

    //��������� ������� �������
    virtual void Execute() = 0;
    //����� �� ��������� ������� �������
    virtual bool CanExecute(CursorDevice* tracer) = 0;
};

//
// ���������� �������� � ������� ���������� � ����
//
class CursorAdviser{
public:

    virtual ~CursorAdviser(){}

    //����� ������ �������������
    virtual bool NeedUse() = 0;
    //����� ������ ������
    virtual bool NeedHack() = 0;
    //����� ������ �����
    virtual bool NeedAttack() = 0;
    //����� ������ ��������
    virtual bool NeedRotate() = 0;
	// ����� ������ ������ �������
	virtual bool NeedSwap() = 0;
	// ����� ������ �������
	virtual bool NeedDebug() = 0;
};

//
// ����� ��� ������ ��������� �������
//
class CursorSelector
{
public:
    virtual ~CursorSelector(){}

    //��������� ���� ��������
    enum cursor_type
	{
        CUR_NULL,
        CUR_USE,		// �������������
        CUR_MENU,		// ������ ����
        CUR_HACK,		// �����
        CUR_MOVE,		// ��������
        CUR_SELECT,		// ����� ���������
        CUR_ROTATE,		// �������
        CUR_ATTACK,		// �����
        CUR_LANDING,	// ������� �� �������
        CUR_SHIPMENT,	// ������� � �������
        CUR_TREATMENT,	// �������
		CUR_SWAP,		// ���������� ������� � �������
		CUR_DEBUG,		// ���������� ������

        MAX_CURSORS,
    };

    //������� ������ ��������� ����������� �������
    virtual cursor_type Select(CursorDevice* tracer, CursorAdviser* adviser) = 0;
};

//=====================================================================================//
//                                  class SmartCursor                                  //
//=====================================================================================//
// ����� ������
class SmartCursor
{
public:
    SmartCursor();
    ~SmartCursor();
    
    //���������� ���������� ������
    void SetUsual();
    //��������� ����� ������ �� ����������
    void Invalidate(){ m_requests++; }
    //�������� ������� ������
    void Update();

    //������ ����� ���������� click'�
    unsigned int GetLastClickTime() const { return m_click_time; }

private:
    int m_requests;

    float m_upd_time;
    unsigned int m_click_time; //����� ���������� ����� �����

    CursorManager*  m_current;
    CursorAdviser*  m_adviser;
    CursorSelector* m_selector;

	std::auto_ptr<CursorManager> m_cursors[CursorSelector::MAX_CURSORS];
};

//
// ����� ��� ���������� �������
//
class Walkers : private SpawnObserver,
                private EntityObserver                
{
public:
    Walkers();
    ~Walkers();

    //��������� �� �� ����������
    bool IsGoing(BaseEntity *ent) const;
    //�������� � ���c�� ������� ��� ����������
    void Insert(BaseEntity *entity, const ipnt2_t& to);

    //�������� �������� ��� ������������ ActionManager
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //���� ������� ���������� 0 ==> �������� ������ ��� 
    //
    ActionManager *CreateAction(BaseEntity* entity = 0);   

    //����������/�������� 
    void MakeSaveLoad(SavSlot& slot);

private:

    //����� ��������� � �������
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // ���������� -1 ���� ���-�� ���
    int Ent2Index(BaseEntity* entity) const;


    //������� �����������
    void LinkObserver(BaseEntity* entity);
    //��������� ������� �������� �� �������
    void Update(Spawner* spawn, SpawnObserver::event_t event, SpawnObserver::info_t info);
    //��������� �������� ��������
    void Update(BaseEntity* entity, EntityObserver::event_t event, EntityObserver::info_t info);

    struct info_s;

    //������� �������� �� �������
    void Remove(info_s& info);
    //������� �������� �� ���������� ������ 
    ActionManager* CreateAction(info_s& info);

private:

    struct info_s
	{
        info_s() : m_entity(0){}

        //���������/��������� ���������
        void MakeSaveLoad(SavSlot& slot);
        //���� ����� ���-��
        bool IsSame(BaseEntity* entity) const;
        //����� �� ����
        bool NeedMove() const;
        //������ �����
        bool IsEmpty() const { return m_entity == 0; }
        
        BaseEntity* m_entity;
        ipnt2_t     m_point;
    };

    info_s m_infos[MAX_TEAMMATESTOTAL];
};

//=====================================================================================//
//                                  class EnemyIcons                                   //
//=====================================================================================//
// ����� �� ������ ������
class EnemyIcons :  private SpawnObserver,
                    private SpectatorObserver,
                    private RelationsObserver
{
	PoL::BaseEntityRef m_current;

public:
    EnemyIcons();
    ~EnemyIcons();

    //�������� �������� � ����� ��������
    BaseEntity* Icon2Entity(unsigned index);
    //��������� ������ ��������
    void HandleSelectReq(BaseEntity* entity);
    //���������/���������� ������ ������
    void MakeSaveLoad(SavSlot& slot);

private:
    //�������� ������� ����������� �������
    void Update(Spawner* spawner, SpawnObserver::event_t event, SpawnObserver::info_t info);
    //��������� ��������� �������� �����������
    void Update(Spectator* spectator, SpectatorObserver::event_t event, SpectatorObserver::info_t);    
    //��������� ��������� ������������ ������
    void changed(RelationsObserver::event_type et, void* info);

    //�������� ������ �����
    void Insert(Marker* marker);
    //������� ������ �����
    void Remove(Marker* marker);
    //���������� �����
    void SetFlags(BaseEntity* entity, Marker* marker);

    //������������/����������� ������������ 
    void AttachObservers();
    void DetachObservers();
};

//=====================================================================================//
//                                  class HumanIcons                                   //
//=====================================================================================//
// ����� �� ������ �����
class HumanIcons : private SpawnObserver,
                   private EntityObserver                   
{
    static const float m_wound_lvl;
    static const int   m_dead_lvl;

public:
    HumanIcons();
    ~HumanIcons();
    
    //����������� ������ � ��������
    HumanEntity* Icon2Entity(int id);
    //������������� ������ � ��������
    int Entity2Icon(HumanEntity* human);
    //����������/�������� 
    void MakeSaveLoad(SavSlot& slot);
 
private:
    //������. ����������� � ��������
    void LinkObserver(HumanEntity* human);

    //�������� ������
    void Insert(HumanEntity* human, bool insert2front);

    enum remove_type
	{
        RT_USUAL,
        RT_DEATH,
    };

    //������� ������
    void Remove(HumanEntity* human, remove_type type = RT_USUAL);
    
    //������ heartbeat
    int GetHumanHeartbeat(HumanEntity* human);
    //������ ����� ������
    unsigned GetHumanFlags(HumanEntity* human);

    //��������� ��������� � �����������
    void Update(Spawner* spawner, SpawnObserver::event_t type, SpawnObserver::info_t info);
    //��������� ��������� ��������� ��������
    void Update(BaseEntity* entity, EntityObserver::event_t event, EntityObserver::info_t info);
};

//=====================================================================================//
//                                   class TechIcons                                   //
//=====================================================================================//
// ������ �� �������
class TechIcons : private SpawnObserver,
                  private EntityObserver
{
    struct info_s
	{
        int            m_requests;
        VehicleEntity* m_vehicle;

        info_s() : m_requests(0), m_vehicle(0){}
    };
    
    info_s    m_icons[MAX_TECHINTEAM];

public:
    TechIcons();
    ~TechIcons();

    //�������� ������ �� �������
    void Update();
    //�������� ������ �� �������
    void Insert(VehicleEntity* vehicle);
    //������� ������ �� �������
    void Remove(VehicleEntity* vehicle);

    //�������� ������ �� ������� �� ������ ������
    VehicleEntity* Icon2Entity(unsigned icon);
    //�������� ����� ������ �� ������ �� �������
    int Entity2Icon(VehicleEntity* vehicle);

    //����������/�������� 
    void MakeSaveLoad(SavSlot& slot);

	int GetMaxCount() const { return MAX_TECHINTEAM; }

private:
    //������������ �����������
    void LinkObserver(BaseEntity* vehicle);
    //���������� ������ ��� �������
    void DrawIcon(int icon_id, VehicleEntity* vehicle);

    //��������� ��������� � �����������
    void Update(Spawner* spawner, SpawnObserver::event_t type, SpawnObserver::info_t info);
    //��������� ��������� ��������� ��������
    void Update(BaseEntity* entity, EntityObserver::event_t event, EntityObserver::info_t info);
};

//=====================================================================================//
//                                  class RobotIcons                                   //
//=====================================================================================//
// ������ ���������� �������
class RobotIcons :  private SpawnObserver,
                    private EntityObserver
{
public:
	RobotIcons();
    ~RobotIcons();

    //������ �������� � ������� index 
    VehicleEntity* Icon2Entity(unsigned index);
    //���������/���������� ������ ���������� �������
    void MakeSaveLoad(SavSlot& slot);

private:
    //�������� ������
    void Insert(VehicleEntity* robot);
    //������� ������
    void Remove(VehicleEntity* robot);
    //���������� ��������� ������ �� ��������
    void SetRobotState(VehicleEntity* robot);

    //��������� ��������� � �����������
    void Update(Spawner* spawner, SpawnObserver::event_t type, SpawnObserver::info_t info);
    //��������� ��������� ��������� ��������
    void Update(BaseEntity* entity, EntityObserver::event_t event, EntityObserver::info_t info);
};

//
// ����� ��� ��������� �����
//
class InputManager
{
public:
    virtual ~InputManager(){}

    //���������� ���� ������������
    virtual void HandleInput() = 0;
};

//
// ��������� ��� ����������� ������
//
class Reloader{
public:

    Reloader();
    ~Reloader();

    //����� �� ���������� �����������
    bool CanReload(HumanEntity* human) const;

    //������������ ������ ��������
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //��������: 
    //         � ����� ����������� �������������� ��� ��������
    //         ����� ���������� ������ ���� CanReload() == true    
    //
    void DoReload(HumanEntity* human);

private:

    //������ �������� ���������? 
    bool CanLoadAmmo(const WeaponThing* weapon) const;
};

//
// ���������� ���� ������� ����
//
class GameFormImp : public  BaseForm,
                    private SpawnObserver,
                    private EntityObserver,
                    private GameScreenController
{
public:

    GameFormImp();
    ~GameFormImp();

    //�������� ����
    void Show();
    //�������������
    void Init(const ini_s& ini);
    //��������� ����� 
    void HandleInput(state_type* st);
    //��������� ����
    void MakeSaveLoad( SavSlot& st);
    //��������/�������� ����
    void ShowControls(bool flag);

    //
    // ������� ���� ����� ��� ������ *Icons, Controller, SmartCursor
    //
   
    //�������� ������ � ��������� ����������
    Walkers* GetWalkers();
    //������ ������� �������� � ���������
    BaseEntity* GetCurEnt();
    //�������� ������ � �������
    SmartCursor* GetCursor();
    //�������� ������ � ��������� �����������
    Reloader* GetReloader();
    //�������� ������ � ������� ����������
    HumanIcons* GetHumanIcons();
    //�������� ������ � ������� �������
    TechIcons* GetTechIcons();
    //�������� ������ � ������� ������
    EnemyIcons* GetEnemyIcons();
    //�������� ������ � ������� �������
    RobotIcons* GetRobotIcons();

    //����� �� �������� ���� ���������
    bool CanShowFOS() const;
    //����� �� ���������
    bool CanAttack() const;
    //����� �� ������� use
    bool CanMakeUse() const;
    //����� �� ������� Hack
    bool CanMakeHack() const;

    //�������� ������� ������
    void ShowUsualCursor();

    //���������� ������� �� ������ use
    void HandleSwitchUseReq();
    //��������� ������� �� ������ ������ ���� ���������
    void HandleSwitchFOSReq();
    //��������� ������� �� ������ hack
    void HandleSwitchHackReq();
    //��������� ������� �� ������
    void HandleSwitchAttackReq();
    //��������� ����� ����
    void HandleEndOfTurnReq();
    //��������� ������ �������
    void HandleShowJournalReq();
    //��������� ������� �� ����� ���� �����
    void HandleShowOptionsReq();

    enum selection_type
	{
        SEL_SIMPLE,  //������ ��������� (���� � ����� ��������)
        SEL_COMMAND, //����� ������� �� ��������� ��������� 
    };

    //��������� ������� ������ ��������
    void HandleSelectReq(BaseEntity* entity, selection_type type = SEL_SIMPLE);
    //��������� ������� �� use
    void HandlePlayActionReq(ActionManager* mgr);
    //��������� ������ ���� inventory
    void HandleShowInventoryReq(HumanEntity* human, AbstractGround* ground, AbstractScroller* scroller, AbstractBin* bin);
    //��������� ������ ���� ��������
    void HandleShowShopReq(HumanEntity* hero, TraderEntity* trader, AbstractGround* ground, AbstractScroller* scroller, AbstractBin* bin);
  
    //��������� ������������ ���� ��������
    void HandleReloadWeapon();
    void HandleSwitch2Aimshot();
    void HandleSwitch2Autoshot();
    void HandleSwitch2Snapshot();

    void HandleSwitchHandsMode();
    void HandleNextFastAccessThing();
    void HandlePrevFastAccessThing();

    //������������ ������ ��������
    void HandleSwitch2Run();
    void HandleSwitch2Sit();
    void HandleSwitch2Stand();

    //��������� ��������
    void RunActivity(int activity_command);

private:

    //�������. ����� � ������ ��� ��� �������
    void Init();

    //����������� ����� �� ������ �������
    bool CanExitLevel(std::string* level);

    //��������� ��������� ������������ use
    bool TrapActionEnd();
    //��������� ��������� ������� ��� use
    bool TrapActionMoveEnd();
    //��������� ��������� �������� ������� ��� use
    bool TrapActionRotateEnd();

    //������������ ��������� ��� �������� ������ ������� � Save'�
    void RestoreSelection();
    //�������� ������ ���������� ��������
    void ResetCursorModeButtons();

    //��������� ��������� � �����������
    void Update(Spawner* spawner, SpawnObserver::event_t type, SpawnObserver::info_t info);
    //��������� ��������� ��������� ��������
    void Update(BaseEntity* entity, EntityObserver::event_t event, EntityObserver::info_t info);

    //
    // ��������� ������� ����
    //

    //������� �� ���� ���� �� ������ ������
    void OnWeaponIconClick();
    //������� �� ���� ���� �� ������ �����
    void OnEnemyIconClick(Marker* marker);
    //��������� ������� ������
    void OnButtonClick(GameScreen::BUTTON id);
    //��������� ������� ������
    void OnCheckBoxClick(GameScreen::CHECKBOX id);
    //������� �� ���� ���� �� ������ ���������
    void OnIconClick(unsigned int id,GameScreen::BN_CLICK click_type);
    //������� �� ���� ���� �� ������ �����
    void OnHackVehicleClick(VehicleEntity* ve,GameScreen::BN_CLICK click_type);
    //������� �� ���� ���� �� ������ �������
    void OnVehicleIconClick(unsigned int icon_number,GameScreen::BN_CLICK click_type);
    
private:
    enum activity_type
	{
        AT_NONE,			//��� ���������� 
        AT_ACTION,			//use �� ��������
        AT_ACTION_MOVE,		//������ ��� use �� ��������
        AT_ACTION_ROTATE	//������� ��� use �� ��������
    };

    activity_type m_activity_type;
    ActionManager *m_action_mgr;

    enum turn_state
	{
        TS_START_TURN,		//������ ���
        TS_INCOMPLETE,		//��� ��� �� ��������
        TS_EXIT_LEVEL,		//���������� ����� �� ������ �������
        TS_WAIT_LEVEL,		//����� ����� �������� ������   
        TS_RUN_WALKERS,		//��������� ����������
        TS_FINISHED,		//�������� ��� ���������� 
        TS_TEST_LEVEL_EXIT	//��������� ����� � ������
    };

    turn_state m_turn_state;

    enum input_type
	{
        IT_EMPTY,        //������ ���������� �����
        IT_USUAL,        //���������� �������� ����������������� �����
        IT_ACTIVITY,     //���������� ����� ��� ���������� ��������

        MAX_INPUT_MGRS
    };

    InputManager *m_input_mgr[MAX_INPUT_MGRS];

    enum flag_type
	{
        FT_CAN_USE			= 1 << 0,
        FT_SHOW_FOS			= 1 << 1,
        FT_CAN_HACK			= 1 << 2,
        FT_CAN_ATTACK		= 1 << 3,
        FT_UPDATE_JOURNAL	= 1 << 4
    };

    unsigned int m_flags;

    Activity *m_activity;
    BaseEntity *m_cur_ent;

    Walkers m_walkers;
    TechIcons m_vehicles;
    Reloader m_reloader;
    HumanIcons m_humans;
    RobotIcons m_robots;
    EnemyIcons m_enemies;
    SmartCursor m_cursor;

    float m_last_move;
    float m_auto_turn;

    float m_update_time;
    float m_update_delta;

	std::string m_targetLevel;
};

//~~~~~~~~~~~~~~~~~~~~~~~ inline ~~~~~~~~~~~~~~~~~~~~~~~~~~

inline BaseEntity* GameFormImp::GetCurEnt()
{
    return m_cur_ent;
}

inline HumanIcons* GameFormImp::GetHumanIcons()
{
    return &m_humans;
}

inline TechIcons* GameFormImp::GetTechIcons()
{
    return &m_vehicles;
}

inline Walkers* GameFormImp::GetWalkers()
{
    return &m_walkers;
}

inline SmartCursor* GameFormImp::GetCursor()
{
    return &m_cursor;
}

inline EnemyIcons* GameFormImp::GetEnemyIcons()
{
    return &m_enemies;
}

inline RobotIcons* GameFormImp::GetRobotIcons()
{
    return &m_robots;
}

inline bool GameFormImp::CanShowFOS() const
{
    return (m_flags & FT_SHOW_FOS) != 0;
}

inline Reloader* GameFormImp::GetReloader()
{
    return &m_reloader;
}

#endif // _PUNCH_GAMEFORMIMP_H_

/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ��������� ������ �������
                                                                                
                                                                                
   Author: Alexander Garanin (Punch)
		   Mikhail L. Lepakhin (Flif)
***************************************************************/                

#if !defined(__CIVILIAN_AI_H__)
#define __CIVILIAN_AI_H__

#include "AIBase.h"
#include "Activity.h"
#include "GameObserver.h"

class Activity;
class BusRoute;
class CheckPoints;
class AINode;

/////////////////////////////////////////////////////////////////////////////
//
// ���� �� �������
//
/////////////////////////////////////////////////////////////////////////////
class BusNode: public AINode
{
public:

    enum bus_type{
        BT_BUSY,    //������� � ��� ������ �����
        BT_USUAL,   //������� �������
    };

    BusNode(eid_t ent_id = 0, BusRoute* route = 0, bus_type type = BT_USUAL);
    ~BusNode();

    DCTOR_DEF(BusNode)

    float Think(state_type* st);
    void MakeSaveLoad(SavSlot& st);

	// ������ ������ ��������� ����
	virtual float getComplexity() const { return 1.0f; }

	// ������� ������������� ���������� � ����� ���������� ��������
	// ���������� ����, ���� ��� ����� � ���������
	virtual eid_t getEntityEID() const { return m_entity; }

	// ����!
	virtual bool die();
	// ����� �� ������� ����
	virtual bool need2Delete() const;
private:
    //�������� �������
    void Hide();

    //��������� ������� �� �������
    void Spawn();

    //�������������
    bool OnInit();
    //��������� ��������
    bool OnMove();
    //respawn
    bool OnRespawn();
    //��������� ���������
    bool OnStop();
    //��������� �������� ���������
    bool OnDest();

    //����� ������� �� ���� ����� ����?
    bool CanMove() const;

private:

    enum bus_mode{
        BM_MOVE,
        BM_STOP,
        BM_DEST,
        BM_INIT,
        BM_QUIT,    
        BM_SPAWN,
		BM_KILLED,
    };
 
    bus_mode  m_mode;
    
    int       m_delay;
    ipnt2_t   m_to;
    eid_t     m_entity;
 
    BusRoute* m_route;
    Activity* m_activity;
};

/////////////////////////////////////////////////////////////////////////////
//
// ���� ��� ����� � ���������������� ���������
//
/////////////////////////////////////////////////////////////////////////////
class FeralCarNode : public AINode, private GameObserver{
public:

    FeralCarNode(eid_t ent = 0, CheckPoints* points = 0);
    ~FeralCarNode();

    DCTOR_DEF(FeralCarNode)

    float Think(state_type* st);
    void MakeSaveLoad(SavSlot& st);
	// ������ ������ ��������� ����
	virtual float getComplexity() const { return 1.0f; }

private:

    void Update(subject_t subj, event_t event, info_t info);

    //�������� ������
    void Hide();
    //�������������
    bool OnInit();
    //respawn
    bool OnSpawn();
    //respawn delay
    bool OnDelay();

private:

    enum car_mode{
        CM_INIT,
        CM_QUIT,
        CM_SPAWN,
        CM_DELAY,
    };

    car_mode m_mode;

    eid_t     m_ent; 
    int          m_delay;
    Activity*    m_activity;
    CheckPoints* m_checks;  
};

/////////////////////////////////////////////////////////////////////////////
//
// ���� ��� ������ ��������
//
//
/////////////////////////////////////////////////////////////////////////////
class CivilianNode: public AINode, private GameObserver
{
public:

    CivilianNode(eid_t ent_id = 0, CheckPoints* checks = 0);
    ~CivilianNode();

    DCTOR_DEF(CivilianNode)

    float Think(state_type* st);
    void MakeSaveLoad(SavSlot& st);
	// ������ ������ ��������� ����
	virtual float getComplexity() const { return 1.0f; }

	// ������� ������������� ���������� � ����� ���������� ��������
	// ���������� ����, ���� ��� ����� � ���������
	virtual eid_t getEntityEID() const { return m_entity; }

	// ����!
	virtual bool die();
	// ����� �� ������� ����
	virtual bool need2Delete() const;
private:

    //�������������
    bool OnInitMode(state_type* st);
    //������������ ������
    bool OnMoveMode(state_type* st);
    //��������� ����� ��������
    bool OnQuitMode(state_type* st);
    //��������� ���������
    bool OnStopMode(state_type* st);
    //��������� �����
    bool OnAttackMode(state_type* st);
    //��������� ����� ������
    bool OnEndMoveMode(state_type* st);
    //��������� ��������
    bool OnRunAwayMode(state_type* st);
    //����� ������ � ����
    bool OnWeapon2Hands(state_type* st);
    //��������� ������� �� ������
    bool OnThingTakeMode(state_type* st);
    //�������� �����
    bool OnLoadWeaponMode(state_type* st);
    //����� ��������� �� ������
    bool OnThingSearchMode(state_type* st);

    //������ ���� �� ������ � unit'�
    bool HaveWeapon() const;
    
    void Update(subject_t subj, event_t event, info_t info);

    //��������� ��������� � �����������
    void OnSpawnMsg();
    //��������� ��������� � ��������
    void OnShootEv(BaseEntity* who, const point3& where);
    //��������� �������� � �����������
    void OnHurtEv(BaseEntity* killer, BaseEntity* victim);

    //������������ �������� � ����� � m_anim
    void PlayAnim(state_type* st);

private:

    enum civilian_mode{
        CM_INIT,        // �������������
        CM_QUIT,
        CM_STOP,        // ��������� special
        CM_MOVE,        // ����� � ����� �����
        CM_RUNAWAY,     // �������� � ������ �����������
        CM_ENDMOVE,
        CM_ATTACK,      // �������� ��������� �����
        CM_THING_TAKE,  // ��������� �������
        CM_THING_SEARCH,// ���������� � ������� ������ ��� ��������
        CM_WEAPON2HANDS,// ����� ������ � ����
        CM_WEAPON_LOAD, // �������� ����� 
        CM_KILLED, // ������������ ������� 
    };

    civilian_mode m_mode;

    enum anim_type{
        AT_NONE,
        AT_FEAR,
        AT_SPECIAL,
    };

    anim_type m_anim;
    anim_type m_sav_anim;
 
    int    m_duration;
    float  m_time;

    CheckPoints* m_checks;

    bool   m_was_hurt;
    int    m_delay;
    int    m_run;
    point3 m_dir;

    eid_t  m_enemy;
    eid_t  m_entity;

    Activity* m_activity;
};

/////////////////////////////////////////////////////////////////////////////
//
// ���� ��� ��������, ������� ����� � ������ ��������
//
/////////////////////////////////////////////////////////////////////////////
class FixedNPCNode: public  AINode 
{
public:

    FixedNPCNode(eid_t ent = 0);
    ~FixedNPCNode();

    DCTOR_DEF(FixedNPCNode)

    float Think(state_type* st);
    void MakeSaveLoad(SavSlot& st);
	// ������ ������ ��������� ����
	virtual float getComplexity() const { return 1.0f; }

	// ������� ������������� ���������� � ����� ���������� ��������
	// ���������� ����, ���� ��� ����� � ���������
	virtual eid_t getEntityEID() const { return m_entity; }

	// ����!
	virtual bool die() { return true; }
private:
    float m_time;
    eid_t m_entity;
};

/////////////////////////////////////////////////////////////////////////////
//
// ����� - ����������� �� ����������� ��������
//
/////////////////////////////////////////////////////////////////////////////
class CivilianActivityObserver : public ActivityObserver
{
public:
	CivilianActivityObserver() { m_last_event = EV_NONE; }
    //���������� ����������� � �������
    virtual void Update(Activity* subj, event_t event, info_t info)
	{
		setLastEvent(static_cast<event_type>(event));
	}

	event_type getLastEvent() { return m_last_event; }
	void setLastEvent(event_type event) { m_last_event = event; }
	void clear() { m_last_event = EV_NONE; }
private:
	event_type m_last_event;
};

/////////////////////////////////////////////////////////////////////////////
//
// ���� ��� ���������� ����
//
/////////////////////////////////////////////////////////////////////////////
class AdvertisingBotNode: public AINode
{
public:
	// ����������� - id ��������
	AdvertisingBotNode(eid_t id = 0);
	~AdvertisingBotNode();

	DCTOR_DEF(AdvertisingBotNode)

	float Think(state_type* st);
	void MakeSaveLoad(SavSlot& st);
	// ������ ������ ��������� ����
	virtual float getComplexity() const { return 1.0f; }

	// ������� ������������� ���������� � ����� ���������� ��������
	// ���������� ����, ���� ��� ����� � ���������
	virtual eid_t getEntityEID() const { return m_entity; }

	// ����!
	virtual bool die();
	// ����� �� ������� ����
	virtual bool need2Delete() const;
private:
	// ��������� ������ Searching
	bool thinkSearching(state_type* st);
	// ��������� ������ Attack
	bool thinkAttack(state_type* st);
	// ��������� ������ Rest
	bool thinkRest(state_type* st);

	// ������������ �������, � ������� ����� ��������� ���
	enum AdvertisingBotMode
	{
		ABM_SEARCHING,
		ABM_ATTACK,
		ABM_REST,
		ABM_KILLED,
	};

	// ������������ ��������� ����
	enum TurnState
	{
		TS_NONE,
		TS_START,
		TS_INPROGRESS,
		TS_END
	};

	// ������� �����
	AdvertisingBotMode m_mode;
	// ������� ��������� ����
	TurnState m_turn_state;

	// ������������� ��������
    eid_t  m_entity;

	// ������� ����������
    Activity* m_activity;

	// ����������� �� ����������� ��������
	CivilianActivityObserver m_activity_observer;

	// ������������� ��������, �� ������� ���� ���������
	eid_t m_enemy_id;
	// �������� �����
	ipnt2_t m_target_pnt;
	// ������� ����, ��� ���� ��������� � �������� �������������
	bool m_initialising;
	// ���������� ����� � ��������� ������
	int m_rest_turn;
	// ���������� ����� � ��������� �����
	int m_attack_turn;
};

/////////////////////////////////////////////////////////////////////////////
//
// ���� ��� ��������� ����
//
/////////////////////////////////////////////////////////////////////////////
class TradingBotNode: public AINode
{
public:
	// ����������� - id ��������
	TradingBotNode(eid_t id = 0);
	~TradingBotNode();

	DCTOR_DEF(TradingBotNode)

	float Think(state_type* st);
	void MakeSaveLoad(SavSlot& st);
	// ������ ������ ��������� ����
	virtual float getComplexity() const { return 1.0f; }

	// ������� ������������� ���������� � ����� ���������� ��������
	// ���������� ����, ���� ��� ����� � ���������
	virtual eid_t getEntityEID() const { return m_entity; }

	// ����!
	virtual bool die();
	// ����� �� ������� ����
	virtual bool need2Delete() const;
private:
    //��������� ��������� � �����������
    void OnSpawnMsg();

	// ��������� ������ Searching
	bool thinkSearching(state_type* st);
	// ��������� ������ Attack
	bool thinkAttack(state_type* st);
	// ��������� ������ Rest
	bool thinkRest(state_type* st);

	// ������������ �������, � ������� ����� ��������� ���
	enum TradingBotMode
	{
		TBM_SEARCHING,
		TBM_ATTACK,
		TBM_REST,
		TBM_KILLED,
	};

	// ������������ ��������� ����
	enum TurnState
	{
		TS_NONE,
		TS_START,
		TS_INPROGRESS,
		TS_END
	};

	// ������� �����
	TradingBotMode m_mode;
	// ������� ��������� ����
	TurnState m_turn_state;

	// ������������� ��������
    eid_t  m_entity;

	// ������� ����������
    Activity* m_activity;

	// ����������� �� ����������� ��������
	CivilianActivityObserver m_activity_observer;

	// ������������� ��������, �� ������� ���� ���������
	eid_t m_enemy_id;
	// ������� ����� ��������� ����
	ipnt2_t m_base_pnt;
	// ������ ������ �������� ����
	pnt_vec_t m_base_field;
	// �������� �����
	ipnt2_t m_target_pnt;
	// ������� ����, ��� ���� ��������� � �������� �������������
	bool m_initialising;
	// ���������� ����� � ��������� ������
	int m_rest_turn;
	// ���������� ����� � ��������� �����
	int m_attack_turn;
};


/////////////////////////////////////////////////////////////////////////////
//
// ���� ��� ���������� ����
//
/////////////////////////////////////////////////////////////////////////////
class RepairBotNode: public AINode
{
public:
	// ����������� - id ��������
	RepairBotNode(eid_t id = 0);
	~RepairBotNode();

	DCTOR_DEF(RepairBotNode)

	float Think(state_type* st);
	void MakeSaveLoad(SavSlot& st);
	// ������ ������ ��������� ����
	virtual float getComplexity() const { return 1.0f; }

	// ������� ������������� ���������� � ����� ���������� ��������
	// ���������� ����, ���� ��� ����� � ���������
	virtual eid_t getEntityEID() const { return m_entity; }

	// ����!
	virtual bool die();
	// ����� �� ������� ����
	virtual bool need2Delete() const;
private:
	// ��������� ������ Searching
	bool thinkGoing(state_type* st);
	// ��������� ������ Attack
	bool thinkAttack(state_type* st);

	// ������������ �������, � ������� ����� ��������� ���
	enum RepairBotMode
	{
		RBM_GOING,
		RBM_ATTACK,
		RBM_KILLED,
	};

	// ������������ ��������� ����
	enum TurnState
	{
		TS_NONE,
		TS_START,
		TS_INPROGRESS,
		TS_END
	};

	// ������� �����
	RepairBotMode m_mode;
	// ������� ��������� ����
	TurnState m_turn_state;

	// ������������� ��������
    eid_t  m_entity;

	// ������� ����������
    Activity* m_activity;

	// ����������� �� ����������� ��������
	CivilianActivityObserver m_activity_observer;

	// �������� �����
	ipnt2_t m_target_pnt;
	// ������� ����, ��� ���� ��������� � �������� �������������
	bool m_initialising;
	// ���������� ����� � ��������� �����
	int m_attack_turn;
};


/////////////////////////////////////////////////////////////////////////////
//
// ���� ��� ���������
//
/////////////////////////////////////////////////////////////////////////////
class IndifferentNode: public AINode
{
public:
	// ����������� - id ��������
	IndifferentNode(eid_t id = 0);
	~IndifferentNode();

	DCTOR_DEF(IndifferentNode)

	float Think(state_type* st);
	void MakeSaveLoad(SavSlot& st);
	// ������ ������ ��������� ����
	virtual float getComplexity() const { return 1.0f; }

	// ������� ������������� ���������� � ����� ���������� ��������
	// ���������� ����, ���� ��� ����� � ���������
	virtual eid_t getEntityEID() const { return m_entity; }

	// ����!
	virtual bool die() { return true; }
private:
    //��������� ��������� � �����������
    void OnSpawnMsg();

	// ������������ ��������� ����
	enum TurnState
	{
		TS_NONE,
		TS_START,
		TS_INPROGRESS,
		TS_END
	};
	// ������� ��������� ����
	TurnState m_turn_state;

	// ������������� ��������
    eid_t  m_entity;
	// ������� ����, ��� ���� ��������� � �������� �������������
	bool m_initialising;
};

#endif // __CIVILIAN_AI_H__
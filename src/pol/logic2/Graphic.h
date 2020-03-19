//
// ������ ����������� ������ � ����������� ������ ����
//

#ifndef _PUNCH_GRAPHIC_H_
#define _PUNCH_GRAPHIC_H_

#ifndef _PUNCH_ENTITYAUX_H_
#include "EntityAux.h"
#endif

#include <undercover/character.h>

class KeyAnimation;

class NetDriver;
class GraphHexGrid;
class ScatteredItem;

class BaseThing;
class BaseEntity;

class GraphHuman;
class GraphVehicle;

class EntityObserver;

namespace PoL { class MovementController; }

//
// ������� ��� ������ � ��������
//
namespace GraphUtils
{
    //�������� �� �������� � ����������� �����
    bool IsInVicinity(const point3& pos, const ipnt2_t& pnt, float eps = 0.3);
    //������� ������� ������������� �������� ������
    void DestroyCorpses();
}

////
//// callbacks ������� ��� ������
////
//class MoveEvents
//{
//public:
//
//    enum cmd_type
//	{
//        CT_MOVE,     //���������� ��������
//        CT_STOP,     //���������� ��������
//        CT_SPEED_UP, //�������� ��������
//    };
//
//    enum event_type{
//        ET_STEP,     //������ ������� � ����� hex
//        ET_END,      //��� ������� � �������� ����� ����
//    };
//
//    //����������� � �������� � ����� hex
//    virtual cmd_type OnStep(event_type event) = 0;
//};

//
// �������� �� ����������� hex_grid
//
class GraphGrid
{
public:

    ~GraphGrid();

    //�������� ��������� ������
    static GraphGrid* GetInst();

    enum hex_type{
        HT_PATH,
        HT_FRONT,
        HT_JOINTS,
        HT_LAND,

        HT_MAX,
    };

    //����������� �� ��������� �����
    void HandleOptionsChange();

    //����������
    void Draw();
    //�������� / �������� ������ �����
    void Show(hex_type type, bool flag);
    //���������� ���� hex'����� ����
    void SetHexes(hex_type type, const pnt_vec_t& vec);

protected:

    GraphGrid();

private:
    
    bool m_fshow[HT_MAX];

    GraphHexGrid* m_graph;
};

//
// �������� ������ ��������
//
class EffectMgr
{
public:

    //singleton
    static EffectMgr* GetInst();

    ~EffectMgr();

    //������ ���������
    void MakeHit(const rid_t& name, const point3& from, const point3& to);
    //������ �������� �� ������
    void MakeFlash(const rid_t& name, const point3& from, const point3& to);
    //������ ������ ����
    float MakeTracer(const rid_t& name, const point3& from, const point3& to);
    //������ �� ������ �������
    float MakeGrenadeFlight(const KeyAnimation& ka, const std::string& skin, point3* end);

    //�������� ������ ������� ��� ��������
    void MakeHumanTreatment(const GraphHuman* graph);
    //�������� �������
    void MakeVehicleExplosion(const GraphVehicle* graph);

    //�������� ���� �����
    void MakeBloodSpot(const point3& where);
    //�������� ������ �����
    void MakeHumanBlood(const point3& from, const point3& to);
    //�������� ����� �����
    void MakeShellOutlet(BaseEntity* entity, const rid_t& name, const point3& from, const point3& to);

    //������� ���������� ������ � ���� ����� ������
    unsigned MakeConst(const rid_t& rid, const point3& to);
    //������� ���������� ������
    void DestroyConst(unsigned id);

    //�������� ������ ������ ��������
    void MakeMeat(const GraphHuman* human, const point3& from);
    
protected:

    EffectMgr();
};

//
// ���������� �������� �������� �� ������
//
class GraphThing
{
public:

    GraphThing(gid_t gid = 0, BaseThing* thing = 0);
    ~GraphThing();

    //���������/��������� ���������� ��������
    static void EnableDestroy(bool flag);

    DCTOR_DEF(GraphThing)
    //����������/�������� ��������
    void MakeSaveLoad(SavSlot& st);

    //������ ������� �������� �� ������
    point3  GetPos3() const;
    ipnt2_t GetPos2() const;

    //�������� ������ �� ��������
    BaseThing* GetParent() {return m_thing;}
    //�������� ������ �����. ��������
    gid_t GetGID() const {return m_gid;}

private:

    BaseThing* m_thing;
    gid_t      m_gid; 

    static bool m_fcan_destroy;
};

//
// ���������� ������������ ��������
//
class GraphEntity : protected EntityObserver
{
public:
    GraphEntity(gid_t gid, BaseEntity* entity);
    virtual ~GraphEntity();

    DCTOR_ABS_DEF(GraphEntity)
    //���������/��������� ������ ��������
    virtual void MakeSaveLoad(SavSlot& st);

    //�������� ���� ������ ��������
    static void Update();
	static void ForceUpdate();

    enum flag_type
	{
        FT_ENABLE_DESTROY = 1 << 0, //��������� ���������� ����������� ��������
        FT_ALWAYS_VISIBLE = 1 << 1, //������� ��� ������ �������� �������� ������

        FT_INIT = FT_ENABLE_DESTROY
    };

    //������ � �������
    static bool IsRaised(unsigned flag);
    static void DropFlags(unsigned flag);
    static void RaiseFlags(unsigned flag);

    enum decor_type
	{
        DT_FOS,
        DT_BANNER,
    };

    //���������/��������� ��������� ��������� 
    static void ShowDecor(decor_type type, bool flag);

    //���������� ������. ��������
    void Destroy();

    //�������� ��������� �� ����������� ���������
    virtual GraphHuman* Cast2Human(){return 0;}
    virtual GraphVehicle* Cast2Vehicle(){return 0;}

    //������ id ������ ��������
    gid_t GetGID() const {return m_gid;}
    //������ �� �������� ���������� ���� ���������
    BaseEntity* GetParent() const {return m_entity;}
    
    //���� ��� ����������� ����
    bool TraceRay(const ray &r, float *Dist, point3 *Norm,bool AsBox=false);
    
    //���������� ������� ��������
    void SetLoc(const point3& pos, float angle);
    void SetLoc(const ipnt2_t& pnt, float angle);
    
    //����������/�������� ���� ��������
    float SetAngle(float angle);
    float GetAngle() const;

    //����� �� rotate ��� �������� �������� � ����������� dir
    bool NeedRotate(const point3& dir);

    //������ ��������� ��������
    point3  GetPos3() const;
    ipnt2_t GetPos2() const;

    //�������� ����� ������� ����� ��������
    void GetVisPoints(pnt3_vec_t* pnts) const;

    //��������� ����� �������� � ��������
    point3 GetShotPoint(const point3& hint = NULLVEC) const;
    //��������� ����� �������� � ��������
    point3 GetShotPointNumber(int n) const;
    //�������� ���������� ����� �������� � ��������
    int GetShotPointCount() const;

    //������ ���������� ������ �����
    void GetBarrel(const point3& barrel, point3* from);
    //������ ���������� ��� ������ ��������
    void GetShellOutlet(const point3& offset, point3* from);

    //�������� ��� ������� � ���������� � ����� ������ ������
    void Visible(bool flag);
    bool IsVisible() const;

	bool IsCameraAttached() const;

    //������� ���� ��������� ��������
    void MoveTo(const ipnt2_t& to);
    
    enum action_type
	{
        AT_USE,
        AT_MOVE,
        AT_STAY,
        AT_DEAD,
        AT_HURT,
        AT_TURN,
        AT_FEAR,
        AT_FALL,
        AT_RELOAD,
        AT_SPECIAL,

        AT_LANDING,  //������� �� �������
        AT_SHIPMENT, //������� � �������

        AT_SHOOT,
        AT_RECOIL,

        AT_AIMSHOOT,
        AT_AIMRECOIL,
    };

    //���������� �� ������������ ��������
    virtual float ChangeAction(action_type action);
    //������� ����������� �������

	/// ���������� ������ ���, ����� ��� �������� ���� ���������� ��������
	virtual void PoLMove(const pnt_vec_t& path, PoL::MovementController *ctrl) = 0;
    virtual void Move(const pnt_vec_t& path, PoL::MovementController *ctrl) = 0;

    //���� �������� ��� �������
    enum effect_type
	{
        ET_FLAME,
        ET_SHOCK,
        ET_ELECTRIC,
        ET_SELECTION,

        ET_MAX,
    };

    //����� ����������� �������
    enum show_mode
	{
        SM_NONE,
        SM_SHOW,        //�������� ������
        SM_HIDE,        //�������� ������
        SM_SLOW_HIDE,   //�������� �������� ������
    };

    //�������� ������ �� ����������� ��������
    void ShowEffect(effect_type type, show_mode mode);
                                                      
protected:

    //�������. �����������
    void LinkObserver();

    //�������� ������ ��������    
    virtual void UpdateEntity();

    enum update_type
	{
        UT_FOS         = 1 << 0,  //�������� ���� ���������
        UT_ARMOR       = 1 << 1,  //�������� ������ �� ��������
        UT_HANDS       = 1 << 2,  //�������� ���������� � �����
        UT_BANNER      = 1 << 3,  //�������� �������
        UT_SELECTION   = 1 << 4,  //�������� ���������
        UT_DMG_EFFECT  = 1 << 5,  //�������� ������ �� �����������
        UT_BASE_FLAGS  = 1 << 6,  //���������� ����� �� ������ ��������
        UT_TEAM_CHANGE = 1 << 7,  //����� ������� ��������

        UT_ALL = UT_FOS|UT_ARMOR|UT_HANDS|UT_BANNER|UT_SELECTION|UT_DMG_EFFECT|UT_BASE_FLAGS,
    };

    //������ � ������� ���������� ��������
    unsigned int GetUpdateFlags() const { return m_update_flags; }
	void SetUpdateFlags(unsigned int n) { m_update_flags = n; }
    //����������� �� ���������
    void Update(BaseEntity* entity, event_t event, info_t info);

private:

    gid_t      m_gid;
    bool       m_visible;
    show_mode  m_show_modes[ET_MAX]; 
    unsigned   m_update_flags;

    BaseEntity*  m_entity;
    GraphEntity* m_next;

    static float m_update_time;
    static float m_update_delta;

    static GraphEntity* m_first;
    static unsigned     m_flags;
};

////=====================================================================================//
////                    class CallbackBridge : public IMovingCallBack                    //
////=====================================================================================//
//class CallbackBridge : public IMovingCallBack
//{
//public:
//    CallbackBridge() : m_events(0) {}
//
//    void SetEvents(MoveEvents* events)
//    {
//        m_events = events;
//    }
//
//    COMMAND VisitHex(EVENT event)
//    {
//        switch(m_events->OnStep(event == EV_STEP ? MoveEvents::ET_STEP : MoveEvents::ET_END)){
//        case MoveEvents::CT_MOVE:     return CMD_CONTINUE;
//        case MoveEvents::CT_STOP:     return CMD_STOP;
//        case MoveEvents::CT_SPEED_UP: return CMD_SPEED_UP;
//        }
//
//        throw CASUS("CallbackBridge: ���������� �������!!!");
//    }
//
//private:
//    MoveEvents* m_events;
//};


//=====================================================================================//
//                        class GraphHuman : public GraphEntity                        //
//=====================================================================================//
// ����������� �������
class GraphHuman : public GraphEntity
{
public:
    GraphHuman(gid_t gid = 0, BaseEntity* human = 0);
    ~GraphHuman();

    DCTOR_DEF(GraphHuman)
    //���������/��������� ������. ��������
    void MakeSaveLoad(SavSlot& st);

    GraphHuman* Cast2Human(){return this;}

    enum pose_type
	{
        PT_NONE,
        PT_SIT,
        PT_STAND,
    };

    enum move_type
	{
        MT_NONE,
        MT_RUN,
        MT_WALK,
    };

    enum stance_type
	{
        ST_NONE,
        ST_WAR,
        ST_USUAL,
    };

	/// ������� ����� ��������� ���� ����������� ������
	void Reset();

	/// ���������� ������������� ������
	void Relax();

	//������� ������
    void ChangePose(pose_type type);
    //�������� ��� ������������
    void ChangeMoveType(move_type type);
    //���������� ������
    void ChangeStance(stance_type type);

    //�������� ������
	pose_type GetPose() const { return m_pose; }
    //�������� ��� ������������
	move_type GetMoveType() const { return m_move; }
    //�������� ������
	stance_type GetStance() const { return m_stance; }

    //������� �����
    bool IsSitPose() const {return m_pose == PT_SIT;}
    //������� ������ ������
    bool IsRunMove() const {return m_move == MT_RUN;}

    //���� ��������� ���. ��������
    float ChangeAction(action_type action);
    //����������� �������� �� ��� ����
    void PoLMove(const pnt_vec_t& path, PoL::MovementController *ctrl);
    //����������� �������� �� ��� ����
    void Move(const pnt_vec_t& path, PoL::MovementController *ctrl);

    //���� ������ ������� ����� ������
    enum sound_type
	{
        SND_NO_LOF,
        SND_CRITICAL_HURT,
        SND_LUCKY_STRIKE,
        SND_DEATH,
        SND_ENEMY_SIGHTED,
        SND_PRETTY_GIRL,
        SND_SELECT_COMMAND,
        SND_MOVE_COMMAND,
        SND_ATTACK_COMMAND,
        SND_CRITICAL_MISS,
        SND_UNDER_FIRE,
        SND_TREATMENT_MEDIUM,
        SND_TREATMENT_LOW,
        SND_TREATMENT_HIGH,
        SND_TEAMMATE_NEW,
        SND_TEAMMATE_COME,
        SND_TEAMMATE_LEAVE,
        SND_TEAMMATE_DISMISS,
        SND_ENEMY_DIED,
        SND_ENEMY_EXPLODED,
        SND_OPEN_SUCCEED,
        SND_OPEN_FAILED,
        SND_CAR_DRIVE_FAILED,
        SND_LEVELUP,        
        SND_GOT_SHOTGUN,
        SND_GOT_ROCKET_LAUNCHER,
        SND_GOT_AUTOCANNON,
        SND_GOT_ENERGY_GUN,
        SND_USE_PLASMA_GRENADE,
        SND_TERMOPLASMA_EXPLOSION,
        SND_INSERT_SCANNER, 
        SND_REMOVE_SCANNER,
    };

    //��������� ���� ����� ����
    void PlaySound(sound_type sound);
	void AddShotBone(const std::string &bone);

private:

    //�������� ������. ��������
    void UpdateEntity();
    //���������� ������
    void SetWeapon(hold_type type, const std::string& name);
    //���������� ������
    void SetSuit(armor_type suit_type);

private:

    hold_type   m_hold;
    move_type   m_move;
    pose_type   m_pose;
    stance_type m_stance;

    //CallbackBridge m_bridge;
};

//=====================================================================================//
//                       class GraphVehicle : public GraphEntity                       //
//=====================================================================================//
// ����������� �������
class GraphVehicle : public GraphEntity
{
public:
    GraphVehicle(gid_t gid = 0, BaseEntity* vehicle = 0);
    ~GraphVehicle();

    DCTOR_DEF(GraphVehicle)

    GraphVehicle* Cast2Vehicle(){return this;}

    //����������� �������� �� ��� ����
    void PoLMove(const pnt_vec_t& path, PoL::MovementController *ctrl);
    //����������� �������� �� ��� ����
    void Move(const pnt_vec_t& path, PoL::MovementController *ctrl);

	void AddShotBone(const std::string &);

private:

    //�������� ������ ��������
    void UpdateEntity();
};

//=====================================================================================//
//                                 class GraphFactory                                  //
//=====================================================================================//
// ������� ����������� ��������
class GraphFactory
{
public:
    //������� ����������� �������
    static GraphThing* CreateGraphThing(BaseThing* thing, const ipnt2_t& pos);
    //������� ������������ ��������
    static GraphHuman* CreateGraphHuman(BaseEntity* human);
    //������� ����������� �������
    static GraphVehicle* CreateGraphVehicle(BaseEntity* vehicle);
};

//~~~~~~~~~~~~~~~~~ inlines ~~~~~~~~~~~~~~~~~

inline bool GraphEntity::IsVisible() const
{
    return m_visible;
}

#endif // _PUNCH_GRAPHIC_H_
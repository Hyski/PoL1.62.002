#pragma once

//������������ - ����������� ����
enum game_limits
{
    MAX_BUSES     = 32,   //������������ ���-�� ��������� �� ������  
    MAX_TEAMS     = 7,    //������������ ���-�� ������� � ����

	MAX_HUMAN_PLAYERS = 5, //������������ ���-�� ������� � ����

    MAX_ENTITIES  = 256,  //������������ ���-�� ������� � ����
    MAX_JOINTS    = 8,    //������������ ���-�� ������� � ������
    MAX_CREW      = 8,    //������������ ���-�� ����� � ������� 
    MAX_TEAMMATES = 5,    //������������ ���-�� unit'� � ������� ������
    MAX_NON_DISMISSABLE_TEAMMATES = 3,    //������������ ���-�� unit'� � ������� ������
    MAX_TEAMMATESTOTAL = MAX_TEAMMATES + MAX_NON_DISMISSABLE_TEAMMATES,    //������������ ���-�� unit'� � ������� ������
	MAX_TECHINTEAM = 6,

    DMG_DURATION  = 3,   //��������� �����������  �����������

    MAX_FRONT_RADIUS = 120, //������������ ������ �������

    GROUND_FIRE_DURATION    = 3, //��������� ������ �����
    GROUND_FIRE_PATH_LENGTH = 5, //����� ���� ���. ���� ������ �� ���� ����� ��������� �������� 

    BUS_AND_CAR_STOP_DURATION = 5, //������������ �������� �������� ��� �������� � ������ 

    USUAL_PANIC_DURATION   = 0,  //��������� ������  
    SHOCK_PANIC_DURATION   = 4,  //��������� ������  
    BERSERK_PANIC_DURATION = 0,  //��������� ������  

    MPS_FOR_HACK        = 10, //���-�� ����� ��� ������ 
    //MPS_FOR_ROTATE      = 1,  //���-�� ����� �� �������
    MPS_FOR_LANDING     = 20, //���-�� ����� �� ��������
    //MPS_FOR_POSE_CHANGE = 3,  //���-�� ����� �� ����� ����
};

//����������� �������� �����
typedef std::string rid_t;  //resource identifier
typedef unsigned    eid_t;  //entity identifier         
typedef unsigned    gid_t;  //graphic id (id ����������� ��������)
typedef unsigned    tid_t;  //thing id

typedef std::set<rid_t>      rid_set_t;
typedef std::vector<ipnt2_t> pnt_vec_t;
typedef std::vector<point3>  pnt3_vec_t;

//����� ����������� ����� ��� ������ � ������������
//inline float NormRand(){return static_cast<float>(rand())/static_cast<float>(RAND_MAX);}
//������� ����������� � ���� �������� ��������
inline float Dir2Angle(const point3& dir){return atan2(dir.x, -dir.y);}
//������������� ��������� ����� �� 0 �� range
//inline int RangeRand(int range)
//{
//    double index = (range - 1) * NormRand();
//    return modf(index, &index) < 0.5f ? index : index + 1f;
//}

//
//  ��������:  �� ���������� entity � id == 0
//

//
// ������������ ���� ��������� ����� �������
// ������ �������� ���� � 0 - 7
//
enum player_type
{
    PT_NONE     = 1 << 0,     

    PT_ENEMY    = 1 << 1, // ���������
    PT_PLAYER   = 1 << 2, // ��������� �����
    PT_CIVILIAN = 1 << 3, // ������� ������ �������
    PT_PLAYER1  = PT_PLAYER, // ��������� ����� 1
    PT_PLAYER2  = 1 << 4, // ��������� ����� 2
    PT_PLAYER3  = 1 << 5, // ��������� ����� 3
    PT_PLAYER4  = 1 << 6, // ��������� ����� 4
    PT_PLAYER5  = 1 << 7, // ��������� ����� 5

    PT_ALL_PLAYERS = PT_ENEMY | PT_NONE | PT_CIVILIAN |
					 PT_PLAYER1 | PT_PLAYER2 | PT_PLAYER3 |
					 PT_PLAYER4 | PT_PLAYER5
};

//
// ������������ ���� ��������� ����� ������� 
// ������ �������� ���� � 8-15
//
enum entity_type
{
    ET_NONE    = 0,

    ET_HUMAN   = 1 << 8, //�������
    ET_VEHICLE = 1 << 9, //�������
    ET_TRADER  = 1 << 10,
 
    ET_ALL_ENTS = ET_HUMAN | ET_VEHICLE | ET_TRADER
};

//
// �������� �������: ������ �������� ���� � 16 - 23
//
enum entity_attribute
{
    EA_NOT_INCREW    = 1 << 16,
    EA_INCREW        = 1 << 17, 
    EA_WAS_IN_INV    = 1 << 19, //������� ��� � inventory
    EA_LOCK_TRAITS   = 1 << 20, //��������� ��������� ������� ��������
    EA_EXPLOSIVE_DMG = 1 << 21, //�������� ��� �������� �������� �������

    EA_ALL_ATTRIBUTES = EA_NOT_INCREW|EA_INCREW|EA_WAS_IN_INV|EA_LOCK_TRAITS|EA_EXPLOSIVE_DMG
};

//
// ��������� ����
//
enum state_type
{
    ST_INCOMPLETE,   //���������
    ST_FINISHED,     //��������
    ST_DESTROYED     //������ ��� ���� AI - ���� ���������
};

//
// ���� ��������� � Inventory
//
enum thing_type
{
    TT_WEAPON  = 1 << 0,
    TT_AMMO    = 1 << 1,
    TT_GRENADE = 1 << 2,
    TT_IMPLANT = 1 << 3,
    TT_ARMOUR  = 1 << 4,
    TT_MEDIKIT = 1 << 5,
    TT_MONEY   = 1 << 6,
    TT_KEY     = 1 << 7,
    TT_CAMERA  = 1 << 8,
    TT_SHIELD  = 1 << 9,
    TT_SCANNER = 1 << 10, 

    MAX_THINGS,   

    TT_ALL_ITEMS = TT_WEAPON|TT_MONEY|TT_GRENADE|TT_KEY|TT_SHIELD|TT_ARMOUR|TT_AMMO|TT_MEDIKIT|TT_CAMERA|TT_IMPLANT|TT_SCANNER,

    //�������� ��� ��� ��������� ���� ������� ��� �������
    TT_THROWABLE = TT_GRENADE|TT_CAMERA|TT_SHIELD
};

//
// ����� ����
//
enum body_parts_type
{
    BPT_NONE,
    BPT_HEAD,
    BPT_BODY,
    BPT_HANDS,
    BPT_LEGS,

    MAX_BODY_PARTS
};

//
// ������ ��� ��������
//
enum hold_type
{
    HT_NONE,
    HT_RIFLE,
    HT_PISTOL,
    HT_CANNON,
    HT_AUTOCANNON,
    HT_ROCKET,
    HT_SPECIAL,
    HT_GRENADE,
    HT_FREEHANDS
};

//
// ��� ���������
//
enum damage_type
{
    DT_NONE,
    DT_STRIKE,
    DT_SHOCK,
    DT_ENERGY,
    DT_ELECTRIC,
    DT_EXPLOSIVE,
    DT_FLAME,

	DT_COUNT
};

//
// ��������� 
//
struct damage_s
{
    int         m_val;
    damage_type m_type;

    damage_s() : m_val(0), m_type(DT_NONE) {}
};

//
// ���� �������� 
//
enum shot_type
{
    SHT_AIMSHOT,
    SHT_SNAPSHOT,
    SHT_AUTOSHOT
};

//
// ������������ ������ �� ��������
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ��������: 
//      ������� ���������� �������� �����, ��� ��� ���������
//      ����� �������� ����� ������ � �������� (pack_map)
//
enum human_pack_type
{
    HPK_NONE,
    HPK_HEAD,
    HPK_BODY,
    HPK_HANDS,
    HPK_LKNEE,
    HPK_RKNEE,
    HPK_IMPLANTS,
    HPK_BACKPACK,
    HPK_ALL,
        
    MAX_PACKS
};

//
// ���� ������
//
enum human_panic_type
{
    HPT_NONE,
    HPT_USUAL,
    HPT_SHOCK,
    HPT_BERSERK,

    MAX_PANICS
};

//
// ���� ������������
//
enum armor_type
{
    ARM_NONE,
    ARM_NORM,
    ARM_SUIT1,
    ARM_SUIT2, 
    ARM_SUIT3,
    ARM_SUIT4,
    ARM_SCUBA
};

//
// ������������ ����� ��������� ����� ���������� � ������
//
enum RelationType
{
	RT_NEUTRAL,
	RT_ENEMY,
	RT_FRIEND
};

enum vehicle_size
{
    VS_TINY,
    VS_HUGE,
    VS_SMALL,
    VS_LARGE,
	VS_UNKNOWN
};

#include "observer.h" //������� ������������
#include "DynUtils.h" //������� �������� �������� runtime
#include "Invariants.h"
#include <logic_ex/LocRes.h>

#ifdef _HOME_VERSION
    extern CLog logFile;
    #define theLog logFile["logic.log"]
#else
    #define theLog /##/
#endif

bool UnlimitedMoves();
bool IsGameLost();

int aiRand();
float aiNormRand();
int aiRandMax();
int aiRangeRand(int range);

bool isHidMovScreen();


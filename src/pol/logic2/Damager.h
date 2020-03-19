//
// ��������� ��� �������� ����������� ������� � ��������
//

#ifndef _PUNCH_DAMAGER_H_
#define _PUNCH_DAMAGER_H_

class BaseEntity;

//
// ���������� �������� �����
//
class RiskSite{
public:

    virtual ~RiskSite(){}

    // �������� ����������� ��� ��������
    virtual float GetEntityFlameDmg() const = 0;
};

//
// ������ ������� ��� �� ������
// 
class RiskArea{
public:

    virtual ~RiskArea(){}

    //�������� ������ � ���������� ������
    static RiskArea* GetInst();

    //�������� ��� ������� ����
    virtual void Reset() = 0;

    //���������/��������� ������� ���� ������
    virtual void MakeSaveLoad(SavSlot& slot) = 0;
    //���������� ����� � �������� ����� ������
    virtual void InsertFlame(const point3& center, float radius, float damage) = 0;

    //������������
    virtual void TestSmth() = 0;
};

//
// ���������� � ���������
//
struct hit_info_s{
    
    enum {          
        BW_DMG,   //basic weapon damage
        AW_DMG,   //additional weapon damage
        AA_DMG,   //additional ammo damage
            
        MAX_DMGS,
    };
   
    point3   m_to;            //����
    point3   m_from;          //������
    float    m_radius;        //������ ������
    damage_s m_dmg[MAX_DMGS]; //������ �����������

	float	m_fall_off;

	hit_info_s() : m_fall_off(0.0f) {}

    hit_info_s(const point3& from, const point3& to, float radius, const damage_s dmg[]) :
        m_from(from), m_to(to), m_radius(radius), m_fall_off(0.0f)
    { m_dmg[BW_DMG] = dmg[BW_DMG]; m_dmg[AW_DMG] = dmg[AW_DMG]; m_dmg[AA_DMG] = dmg[AA_DMG];}
};

//������� ���
typedef hit_info_s hit_t;

//
// ������ ����������� �������
//
class EntityDamager{
public:

    virtual ~EntityDamager(){}

    enum flag_type{
        FT_MAKE_EFFECTS      = 1 << 0, //����������� �������
        FT_MAKE_DAMAGES      = 1 << 1, //������� ����������� 
        FT_PRINT_MESSAGES    = 1 << 2, //�������� ��������� � ���
        FT_SEND_GAME_EVENTS  = 1 << 3, //��������� ������� ��������� 
        FT_PLAY_ENTITY_HURT  = 1 << 4, //����������� �������� �����������
        FT_PLAY_ENTITY_DEATH = 1 << 5, //����������� ������ ������� ���� ����� ���������� ����������� �������� == 0
        
        CALC_TURN_FALGS = FT_MAKE_EFFECTS|FT_MAKE_DAMAGES|FT_PRINT_MESSAGES|FT_SEND_GAME_EVENTS,
        DEFAULT_FLAGS   = FT_MAKE_EFFECTS|FT_MAKE_DAMAGES|FT_PRINT_MESSAGES|FT_SEND_GAME_EVENTS|FT_PLAY_ENTITY_DEATH|FT_PLAY_ENTITY_HURT,
    };

    //singleton
    static EntityDamager* GetInst();

    //������� ���������� �������� ��-�� ������� ���� �������
    virtual void Damage(BaseEntity* victim, RiskSite* site, unsigned flags = DEFAULT_FLAGS) = 0;
    //������� ����������� �������� ������� ��������
    virtual void Damage(BaseEntity* actor, BaseEntity* victim, const hit_t& hit, unsigned flags = DEFAULT_FLAGS) = 0;

protected:

    EntityDamager(){}
};

//
// ����� - ������������ �������� ����� � ����������� �� ���� ���������
//
class Damager{
public:

    virtual ~Damager(){}

    //singleton
    static Damager* GetInst();

    //����������� � ��������� � ������
    virtual void HandleAirHit(BaseEntity* actor, const hit_t& hit, unsigned flags = EntityDamager::DEFAULT_FLAGS) = 0;
    //����������� � ��������� � �������
    virtual void HandleLevelHit(BaseEntity* actor, const hit_t& hit, unsigned flags = EntityDamager::DEFAULT_FLAGS) = 0;
    //����������� � ��������� � ���
    virtual void HandleShieldHit(BaseEntity* actor, const hit_t& hit, unsigned flags = EntityDamager::DEFAULT_FLAGS) = 0; 
    //����������� � ��������� � ������
    virtual void HandleObjectHit(BaseEntity* actor, const rid_t& obj, const hit_t& hit, unsigned flags = EntityDamager::DEFAULT_FLAGS) = 0;
    //����������� � ��������� � ��������
    virtual void HandleEntityHit(BaseEntity* actor, BaseEntity* victim, const hit_t& hit, unsigned flags = EntityDamager::DEFAULT_FLAGS) = 0;

protected:

    Damager(){}
};

#endif // _PUNCH_DAMAGER_H_
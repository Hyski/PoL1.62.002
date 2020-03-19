//
// �������� ������� ��������� �������� ����
//

#ifndef _PUNCH_THING_H_
#define _PUNCH_THING_H_

#ifndef _PUNCH_INFOUTILS_H_
#include "infoutils.h"
#endif

class Spectator;

class StoreBox;
class BaseEntity;
class GraphThing;

//=====================================================================================//
//                                   class ThingInfo                                   //
//=====================================================================================//
// ������� ���������� � ��������  
class ThingInfo
{
public:
    ThingInfo(const rid_t& rid, thing_type type);
    virtual ~ThingInfo();

    //�������� ��� ��������
    thing_type GetType() const;

    bool IsAmmo() const    { return m_type == TT_AMMO;    }
    bool IsMoney() const   { return m_type == TT_MONEY;   }
    bool IsArmor() const   { return m_type == TT_ARMOUR;  }
    bool IsCamera() const  { return m_type == TT_CAMERA;  }
    bool IsShield() const  { return m_type == TT_SHIELD;  }
    bool IsWeapon() const  { return m_type == TT_WEAPON;  }
    bool IsGrenade() const { return m_type == TT_GRENADE; } 
    bool IsImplant() const { return m_type == TT_IMPLANT; }
    bool IsMedikit() const { return m_type == TT_MEDIKIT; }
    bool IsScanner() const { return m_type == TT_SCANNER; }

    //��������� ������� ��������
	int GetRank() const { return m_rank; }
    int GetCost() const;
    float GetWeight() const;
    const rid_t& GetRID() const;
    const ipnt2_t& GetSize() const;
    const std::string& GetName() const;
    const std::string& GetDesc() const;
    const std::string& GetShader() const;
    const std::string& GetInitials() const;
    const std::string& GetItemModel() const;

    //����� ��� ���� �������� ��������� � ������������
    struct traits_string{
        std::string m_left_col;  //����� �������
        std::string m_right_col; //������ �������
    };

    virtual void GetTraitsString(traits_string* traits) const;

protected:

    friend class EquipmentInfoReader;

	int m_rank;	// ������

    rid_t      m_rid;
    thing_type m_type;

    int     m_cost;   //���������
    ipnt2_t m_size;   //������ item'a � ����
    float   m_weight; //���    

    std::string m_name;      //��� 
    std::string m_desc;      //��������
    std::string m_shader;    //�������� ������� ��� �������� � Inventory
    std::string m_initials;  //����������� �������� ����������
    std::string m_item_model;//�������� ��� inventory
};

//=====================================================================================//
//                        class GrenadeInfo : public ThingInfo                         //
//=====================================================================================//
// ���������� � �������
class GrenadeInfo : public ThingInfo
{
public:
    GrenadeInfo(const rid_t& rid);

    enum grenade_type
	{
        MINE,
        GRENADE,
    };

    bool IsPlasmaGrenade() const;

    int GetMp2Act() const;
    int GetWisdom() const;
    int GetStrength() const;

    float GetDmgRadius() const;

    const damage_s& GetADmg() const;
    const damage_s& GetBDmg() const;

    const std::string& GetHitEffect() const;
    const std::string& GetHoldModel() const;

    void GetTraitsString(traits_string* traits) const;

private:

    friend class GrenadeInfoReader;

    damage_s m_bdmg;    
    damage_s m_admg;
        
    int m_mp2act;
    int m_wisdom;
    int m_strength;

    float m_dmg_radius;
    
    std::string  m_hit_effect;    
    std::string  m_hold_model;

    grenade_type m_gtype;
};

//=====================================================================================//
//                          class AmmoInfo : public ThingInfo                          //
//=====================================================================================//
// ���������� � �������� 
class AmmoInfo : public ThingInfo
{
public:
    AmmoInfo(const rid_t& rid);

    int GetRange() const;
    int GetQuantity() const;
    int GetFalloff() const;

    float GetCaliber() const;
    float GetDmgRadius() const;
	float GetDamageFalloff() const { return m_damage_falloff; }

    const damage_s& GetADmg() const;
    const damage_s& GetBDmg() const;

    const std::string& GetHitEffect() const;
    const std::string& GetTraceEffect() const;
    const std::string& GetShellsEffect() const;

    void GetTraitsString(traits_string* traits) const;

private:

    friend class AmmoInfoReader;

    damage_s m_bdmg;
    damage_s m_admg;

    int   m_range;
    int   m_quality;
    int   m_falloff;
    float m_caliber;   
    float m_dmg_radius;
	float m_damage_falloff;

    std::string m_hit_effect;
    std::string m_trace_effect;
    std::string m_shells_effect;
};

//=====================================================================================//
//                         class WeaponInfo : public ThingInfo                         //
//=====================================================================================//
// ���������� �� ������
class WeaponInfo : public ThingInfo
{
public:
    WeaponInfo(const rid_t& rid);
    
    const damage_s& GetADmg() const;
    const damage_s& GetBDmg() const;

    //����� �� �������� ���������
    bool IsAuto() const;
    //��� �������
    bool IsCannon() const;

    int GetRange() const;
    int GetWisdom() const;
    int GetAimQty() const;
    int GetAutoQty() const;
    int GetQuantity() const;
    int GetStrength() const;
    int GetAccuracy() const;
    int GetMovepnts() const;
    float GetCaliber() const;
	int GetMps2Reload() const;
	float GetMaxScatter() const { return m_max_scatter; }

    hold_type GetHoldType() const;

    bool IsRocket() const;
    bool IsAutoCannon() const;

    const point3& GetBarrel() const;

    //����� ����� ��� ��������
    bool HaveShellOutlet() const;
    const point3& GetShellOutlet() const;
    
    const std::string& GetHoldModel() const;
    const std::string& GetFlashEffect() const;
    const std::string& GetAmmoLoadSound() const;
    const std::string& GetShotSound(shot_type type) const;

    void GetTraitsString(traits_string* traits) const;

private:

    friend class WeaponInfoReader;

    damage_s m_bdmg;
    damage_s m_admg;

    int   m_range;
    int   m_wisdom;
    int   m_aim_qty;
    int   m_quality;
    int   m_strength;
    int   m_auto_qty;
    int   m_accuracy;
    int   m_movepnts;
    float m_caliber;
	float m_max_scatter;

    point3 m_shell;
    point3 m_barrel;

    hold_type m_htype;

    std::string m_shot_sound;
    std::string m_autoshot_sound;
    std::string m_ammo_load_sound;

    std::string m_hold_model;
    std::string m_flash_effect;
	//Grom
	int m_mp2reload;
};

//=====================================================================================//
//                        class ImplantInfo : public ThingInfo                         //
//=====================================================================================//
// ���������� �� �������
class ImplantInfo : public ThingInfo
{
public:
    ImplantInfo(const rid_t& rid);

    enum parameter_type
	{
        PT_SIGHT,
        PT_WISDOM,
        PT_HEALTH,
        PT_STRENGTH,
        PT_REACTION,
        PT_ACCURACY,
        PT_SHOCK_RES,
        PT_FLAME_RES,
        PT_ELECTRIC_RES,
    };

    int GetPower() const;
    int GetWisdom() const;
    int GetMp2Act() const;

    parameter_type GetParameter() const;    

    void GetTraitsString(traits_string* traits) const;

private:

    friend class EquipmentInfoReader;

    parameter_type m_parameter;

    int m_power;
    int m_wisdom;    
    int m_mp2act;
};

//=====================================================================================//
//                         class ArmorInfo : public ThingInfo                          //
//=====================================================================================//
// �������� �������
class ArmorInfo : public ThingInfo
{
public:
    ArmorInfo(const rid_t& rid);

    bool IsHelmet() const;
    bool IsSpaceSuit() const;

    float GetBody() const;
    float GetLegs() const;
    float GetHead() const;
    float GetHands() const;

    int GetMp2Act() const;
    int GetStrength() const;

    armor_type GetSuit() const;
    damage_type GetDmgType() const; 

    float GetRearAmplify() const;

    void GetTraitsString(traits_string* traits) const;

private:
    friend class ArmorInfoReader;

    armor_type  m_suit;
    damage_type m_dmg_type;

    float m_body;
    float m_legs;
    float m_head;
    float m_hands;

    int m_mp2act;
    int m_strength;
    
    float m_rear_amplify;    
};

//=====================================================================================//
//                        class MedikitInfo : public ThingInfo                         //
//=====================================================================================//
// ���������� �� �������
class MedikitInfo : public ThingInfo
{
public:
    MedikitInfo(const rid_t& rid);

    int GetDose() const;
    int GetWisdom() const;
    int GetMp2Act() const;
    int GetCharge() const;

    void GetTraitsString(traits_string* traits) const;

private:

    friend class EquipmentInfoReader;

    int m_dose;
    int m_wisdom;    
    int m_mp2act;
    int m_charge;
};

//=====================================================================================//
//                         class MoneyInfo : public ThingInfo                          //
//=====================================================================================//
// ���������� � �������
class MoneyInfo : public ThingInfo
{
public:
    MoneyInfo();

    const std::string& GetUseSound() const;

private:

    friend class EquipmentInfoReader;

    std::string m_use_sound;
};

//=====================================================================================//
//                          class KeyInfo : public ThingInfo                           //
//=====================================================================================//
// ���������� � �����
class KeyInfo : public ThingInfo
{
public:
    KeyInfo(const rid_t& rid);

    const std::string& GetUseSound() const;

private:
    friend class EquipmentInfoReader;

    std::string m_use_sound;
};

//=====================================================================================//
//                         class CameraInfo : public ThingInfo                         //
//=====================================================================================//
// ���������� � ������
class CameraInfo : public ThingInfo
{
public:
    CameraInfo(const rid_t& rid);

    int GetMp2Act() const;
    int GetWisdom() const;
    int GetCharge() const;
    float GetRadius() const;    

    const std::string& GetHoldModel() const;
    const std::string& GetVisualEffect() const;

    void GetTraitsString(traits_string* traits) const;

private:

    int m_charge;
    int m_mp2act;
    int m_wisdom;

    float m_radius;

    std::string m_hold_model;
    std::string m_effect_name;

    friend class EquipmentInfoReader;
};

//=====================================================================================//
//                         class ShieldInfo : public ThingInfo                         //
//=====================================================================================//
// ���������� � ����
class ShieldInfo : public ThingInfo
{
public:

    ShieldInfo(const rid_t& rid);

    int GetMp2Act() const;
    int GetWisdom() const;
    int GetCharge() const;
    float GetRange() const;

    const std::string& GetHoldModel() const;
    const std::string& GetVisualEffect() const;


    void GetTraitsString(traits_string* traits) const;

private:

    int m_charge;
    int m_mp2act;
    int m_wisdom;

    float m_range;

    std::string m_hold_model;
    std::string m_effect_name;

    friend class EquipmentInfoReader;
};

//=====================================================================================//
//                        class ScannerInfo : public ThingInfo                         //
//=====================================================================================//
// ���������� � ��������
class ScannerInfo : public ThingInfo
{
public:
    ScannerInfo(const rid_t& rid);

    int GetMp2Act() const;
    int GetWisdom() const;    
    float GetRadius() const;

    const std::string& GetUseSound() const;
    const std::string& GetHoldModel() const;

    void GetTraitsString(traits_string* traits) const;

private:
    int   m_mp2act;
    int   m_wisdom;
    float m_radius;

    std::string m_use_sound;
    std::string m_hold_model;

    friend class EquipmentInfoReader;
};

//=====================================================================================//
//                               class ThingInfoFactory                                //
//=====================================================================================//
// ������� ��� �������� ����������
class ThingInfoFactory
{
public:
    //������� ������� ���� type, � ����� rid
    ThingInfo* Create(thing_type type, const rid_t& rid);
};

//
// ��������� ���������� � ���������
//
typedef InfoArchive<ThingInfo, thing_type, ThingInfoFactory> ThingInfoArchive;

//
// ������������ ��������� ��� ������
//
typedef InfoHandbook<ThingInfoArchive> ThingHandbook;

//
// ������� ����� �� Thing
//
class BaseThing
{
public:
    
    BaseThing(thing_type type, const rid_t& rid);
    virtual ~BaseThing();

    DCTOR_ABS_DEF(BaseThing)

    //���������/��������� ������
    virtual void MakeSaveLoad(SavSlotBase &st);

    //������/���������� �������� ���. ����������� �������
    BaseEntity* GetEntity();
    virtual void SetEntity(BaseEntity* ent);

    //������/���������� ������ �� ���� � ��� ����� �������
    //StoreBox* GetStoreBox();
    //virtual void SetStoreBox(StoreBox* box);

    //����������/������ tid_t
    void  SetTID(tid_t tid) { m_tid = tid;  }
    tid_t GetTID() const    { return m_tid; }

    //��������/���������� ���������� �������� � ��������
    const ipnt2_t& GetPakPos() const;
    void SetPakPos(const ipnt2_t& pnt);

    // �������� ��������� ��������
    virtual int GetCost() const;
    // �������� ��� ��������
    virtual float GetWeight() const;

	// �������� ����������� ����������� ������ ��� ��������� ��������
	virtual int GetMinRank() const { return m_info->GetRank(); }

    //�������� ������ � ���������� � ��������
    const ThingInfo* GetInfo() const;

private:

    tid_t   m_tid;
    ipnt2_t m_pak_pos;

    ThingInfo*  m_info; 
    BaseEntity* m_entity;
    //StoreBox*   m_store_box;
};

//
// ��������� ��������� ���������
//
bool operator < (const BaseThing& b1, const BaseThing& b2);

inline bool operator != (const BaseThing& b1, const BaseThing& b2) { return b1 < b2 || b2 < b1; }
inline bool operator == (const BaseThing& b1, const BaseThing& b2) { return !(b1 < b2) && !(b2 < b1); }

//=====================================================================================//
//                         class AmmoThing : public BaseThing                          //
//=====================================================================================//
// ����� �� �������
class AmmoThing : public BaseThing
{
public:

    AmmoThing(const rid_t& rid = rid_t());
    ~AmmoThing();

    DCTOR_DEF(AmmoThing)

    //���������� ������ � ������
    int GetBulletCount() const; 
    //�������� �������� � ������
    void SetBulletCount(int count);
    
    //������ ���������
    int GetCost() const;
    //������ ��� ���������
    float GetWeight() const;

    //����������/��������
    void MakeSaveLoad(SavSlotBase& st);   

    //�������� ������ �� ����������
    const AmmoInfo* GetInfo() const;

private:

    int m_count;
};

//=====================================================================================//
//                        class WeaponThing : public BaseThing                         //
//=====================================================================================//
// ����� �� ������
class WeaponThing : public BaseThing
{
public:

    WeaponThing(const rid_t rid = rid_t());
    ~WeaponThing();

    DCTOR_DEF(WeaponThing)

    //����������/��������
    void MakeSaveLoad(SavSlotBase& st);   

    //��� ����� ������� ������� ������� � ��� ��������
    void SetEntity(BaseEntity* ent);
    void SetStoreBox(StoreBox* box);

    //�������� ������ �� �������
    AmmoThing* GetAmmo() const;
    //�������� ������, ���������
    AmmoThing* Load(AmmoThing* new_ammo);

    //������ ���������
    int GetCost() const;
    //������ ���
    float GetWeight() const;
    //�������� ������ �� ����������
    const WeaponInfo* GetInfo() const; 

    //������ ���-�� ����� �� �������
    int GetMovepnts(shot_type type) const;
    //������ ���-�� ��������� � ����� ������
    int GetShotCount(shot_type type) const;
    
private:

    AmmoThing*   m_ammo;    
};

//=====================================================================================//
//                        class GrenadeThing : public BaseThing                        //
//=====================================================================================//
//����� �� �������
class GrenadeThing : public BaseThing
{
public:

    GrenadeThing(const rid_t& rid = rid_t());
    ~GrenadeThing();

    DCTOR_DEF(GrenadeThing)

    //�������� ������ �� ����������
    const GrenadeInfo* GetInfo() const;
};

//=====================================================================================//
//                        class ImplantThing : public BaseThing                        //
//=====================================================================================//
//����� �� Implants
class ImplantThing : public BaseThing
{
public:

    ImplantThing(const rid_t& rid = rid_t());
    ~ImplantThing();

    DCTOR_DEF(ImplantThing)

    //�������� ������ �� ����������
    const ImplantInfo* GetInfo() const;
};

//=====================================================================================//
//                        class MedikitThing: public BaseThing                         //
//=====================================================================================//
// ����� �� �������
class MedikitThing: public BaseThing
{
public:

    MedikitThing(const rid_t& rid = rid_t());
    ~MedikitThing();

    DCTOR_DEF(MedikitThing)
   
    void MakeSaveLoad(SavSlotBase& st);   

    //������ ����� �������
    int GetCharge() const;
    //�������� �������
    void SetCharge(int charge);
    //�������� ������ �� ����������
    const MedikitInfo* GetInfo() const;

private:
    
    int m_charge;    
};

//=====================================================================================//
//                         class ArmorThing : public BaseThing                         //
//=====================================================================================//
//����� �� �������
class  ArmorThing : public BaseThing
{
public:

    ArmorThing(const rid_t& rid = rid_t());
    ~ArmorThing();

    DCTOR_DEF(ArmorThing)

    //��������� ����. ���������
    float GetAbsorber(damage_type type, body_parts_type part, bool fback) const;
    //�������� ����� �� ���������� � ��������
    const ArmorInfo* GetInfo() const;
};

//=====================================================================================//
//                         class MoneyThing : public BaseThing                         //
//=====================================================================================//
//����� �� ������
class MoneyThing : public BaseThing
{
public:

    MoneyThing(int sum = 0);
    ~MoneyThing();

    DCTOR_DEF(MoneyThing)
   
    void MakeSaveLoad(SavSlotBase& st);   

    //������ ����� �����
    int GetSum() const;
    //���������� ����� �����
    void SetSum(int sum);

    const MoneyInfo* GetInfo() const;

private:

    int m_sum;
};

//=====================================================================================//
//                          class KeyThing : public BaseThing                          //
//=====================================================================================//
// ����� �� �����
class KeyThing : public BaseThing
{
public:

    KeyThing(const rid_t& rid = rid_t());
    ~KeyThing();

    DCTOR_DEF(KeyThing)

    const KeyInfo* GetInfo() const;
};

//
// ����� �� ������
//
class CameraThing : public BaseThing{
public:

    CameraThing(const rid_t& rid = rid_t());
    
    DCTOR_DEF(CameraThing)

    //�������� ���������� � ��������
    const CameraInfo* GetInfo() const;
};

//=====================================================================================//
//                        class ShieldThing : public BaseThing                         //
//=====================================================================================//
// ����� �� ������� ����
class ShieldThing : public BaseThing
{
public:
    
    ShieldThing(const rid_t& rid = rid_t());

    DCTOR_DEF(ShieldThing)

    //�������� ���������� � ��������
    const ShieldInfo* GetInfo() const;
};

//=====================================================================================//
//                        class ScannerThing : public BaseThing                        //
//=====================================================================================//
// ������� - �������
class ScannerThing : public BaseThing
{
public:
    ScannerThing(const rid_t& rid = rid_t());

    DCTOR_DEF(ScannerThing)

	bool IsInUse() const { return m_inUse; }
	void SetInUse(bool v) { m_inUse = v; }

    //�������� ���������� � ��������
    const ScannerInfo* GetInfo() const;

private:
	bool m_inUse;
};

//=====================================================================================//
//                                     class Depot                                     //
//=====================================================================================//
// ���������� ����� ��������� �� ������
class Depot
{
public:

    ~Depot();

    //singleton
    static Depot* GetInst();

    //�������� ������
    void Reset();
    //���������/��������� ������
    void MakeSaveLoad(SavSlot& st);
    //�������� ������� � ������
    tid_t Push(const ipnt2_t& pnt, BaseThing* bit);
    //�������� ������ �� ������� �� id
    BaseThing* Get(tid_t tid);
    
    enum rm_mode{
        RM_DELETE,  //������� �������
        RM_UNLINK,  //������ ������. �������
    };

    //������� ������� �� ������, ���� ��� id
    void Remove(tid_t tid, rm_mode mode = RM_UNLINK);

    //
    //����������� ��� ������� �� ������
    //
    class Iterator;
    typedef Iterator iterator;

    //������� �������, �� ���������
    void Remove(iterator it, rm_mode mode = RM_UNLINK);

    //�������� �� ��������� � ���� ��������� ��������
    iterator begin(BaseEntity* entity, unsigned type = TT_ALL_ITEMS);
    //�������� �� ��������� � ����� ������� ������ �����
    iterator begin(const ipnt2_t& center, float radius = 1.0f, unsigned type = TT_ALL_ITEMS);

    iterator end();

protected:

    Depot();

private:

    friend class Iterator;

    struct info_s
	{
        BaseThing*  m_thing;
        GraphThing* m_graph;

        info_s() : m_thing(0), m_graph(0) {}
    };

    typedef std::vector<info_s> info_vec_t;
    info_vec_t m_infos;
};

//=====================================================================================//
//                                class Depot::Iterator                                //
//=====================================================================================//
class Depot::Iterator
{
public:
    //������� ��������
    class Condition
	{
    public:
        virtual ~Condition(){}

        //��� �������� ���� �������?
        virtual bool IsSuitable(BaseThing* thing, GraphThing* graph) const = 0;
    };

    Iterator(Depot::info_vec_t* vec = 0, unsigned first = 0, Condition* condition = 0);
    ~Iterator();

    Iterator& operator++();
    Iterator  operator++(int)
    { Iterator tmp = *this; ++(*this); return tmp;}

    //������ � ����
    BaseThing* operator-> (){ return (*m_infos)[m_first].m_thing;   }
    BaseThing& operator * (){ return *((*m_infos)[m_first].m_thing);}

    //�������� ������ � ������ ��������
    GraphThing* GetGraph() {return (*m_infos)[m_first].m_graph;}

    friend bool operator != (const Iterator& i1, const Iterator& i2)
    { return i1.m_first != i2.m_first; }

private:

    friend class Depot;

    Condition* m_condition;

    unsigned m_first;
    Depot::info_vec_t* m_infos;
};

#endif // _PUNCH_THING_H_
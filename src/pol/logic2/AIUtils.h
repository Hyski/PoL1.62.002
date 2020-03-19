//
// ������� ��� ������ � AI
//

#ifndef _PUNCH_AIUTILS_H_
#define _PUNCH_AIUTILS_H_

struct damage_s;

class SpawnTag;
class Activity;

class BaseThing;
class AmmoThing;
class WeaponThing;
class ShieldThing;
class CameraThing;
class MedikitThing;
class GrenadeThing;
class ScannerThing;

class Spectator;

class BaseEntity;
class HumanEntity;
class VehicleEntity;

class HumanContext;
class VehicleContext;

//
// ������� ����� �� levelup'�
//
class LevelupMgr{
public:

    static LevelupMgr* GetInst();

    //������ ������� �������
    int GetCurrentLevel(int experience);
    //������ ���-�� ����� �� ������� level
    int GetLevelPoints(int level);

protected:

    LevelupMgr();

private:

    typedef std::vector<int> int_vec_t;
    int_vec_t m_points; 
};

//
// �������� �����
//
class MoneyMgr{
public:

    static MoneyMgr* GetInst();

    void Reset();

    //������ ���������� �����
    int GetMoney() const;
    //���������� ���������� �����
    void SetMoney(int count);
    //�����������/������
    void MakeSaveLoad(SavSlot& slot);

private:

    MoneyMgr();

    int m_count;
};


//
// ����������� �� ������� ������
//
class PlayerSpectator
{
public:
    ~PlayerSpectator();

    static PlayerSpectator* GetInst();

    //�������� ��������� �� �����������
    Spectator* GetSpectator() { return m_spectator.get(); } 

    //�������� ������
    void Reset();
    //��������/����������
    void MakeSaveLoad(SavSlot& slot);

private:
    PlayerSpectator();

	std::auto_ptr<Spectator> m_spectator;
};

//
// ������ ��������� �� ������ �����
//
class CameraPool{
public:

    static CameraPool* GetInst();

    ~CameraPool();

    //���������� ���� � ������
    void Reset();
    //����������/�������� �����
    void MakeSaveLoad(SavSlot& slot);
    //��������� ������ � ������
    void Insert(player_type player, const CameraThing* camera, const point3& pos);

    //��������� �������� ����
    void HandleNextTurn();

private:

    CameraPool(){}

private:

    typedef std::vector<Spectator*> cam_vec_t;
    cam_vec_t   m_cams;
};

//
// ������ ��������� �� ����� ������� �����
//
class ShieldPool{
public:

    //
    // ���������� ��� ������ � ����� ��������� �� �������
    //
    class ShieldTraits{
    public:

        ShieldTraits(const std::string& effect, const point3& pos, float radius, int turns);
        ~ShieldTraits();

        //�������� ������
        float GetRadius() const { return m_radius; }
        //�������� �������
        const point3& GetPos() const { return m_pos; }
        //������ �������� ������� ��� ����
        const std::string& GetEffectName() const { return m_effect_name; }
        
        //������ ���
        void DecTurns() { m_turns --; }
        //������ ���-�� �����
        int GetTurns() const { return m_turns; }

    private:

        point3   m_pos;
        int      m_turns;
        float    m_radius;

        unsigned    m_effect;
        std::string m_effect_name;
    };

    static ShieldPool* GetInst();

    ~ShieldPool();

    //���������� ����� � ������
    void Reset();
    //�������� / ��������
    void MakeSaveLoad(SavSlot& slot);
    //��������� ��� � ������
    void Insert(const ShieldThing* shield, const point3& pos);

    //��������� ���� ����
    void HandleNextTurn();

    class Iterator; typedef Iterator iterator;

    iterator end();
    iterator begin();

private:

    ShieldPool();

private:

    friend class Iterator;

    typedef std::vector<ShieldTraits*> info_vec_t;
    info_vec_t m_infos;
};

//
// �������� �� ������� �����
//
class ShieldPool::Iterator{
public:
    
    Iterator(info_vec_t* vec = 0, size_t first = 0);

    Iterator& operator ++();
    Iterator  operator ++(int)
    { Iterator tmp = *this; operator++(); return tmp; }
    
    //��������� ��� �������� ������
    const ShieldTraits* operator ->(){ return (*m_infos)[m_first];  }
    const ShieldTraits& operator * (){ return *(*m_infos)[m_first]; }
    
    //��������� �� !=
    friend bool operator != (const Iterator& i1, const Iterator& i2)
    { return i1.m_first != i2.m_first; } 
    
private:
    
    size_t      m_first;
    info_vec_t* m_infos;
};

//=====================================================================================//
//                                 class ThingDelivery                                 //
//=====================================================================================//
// �������� ������� �������� ��������
class ThingDelivery
{
public:
	enum reason_type
	{
		RT_NONE,

		RT_CAN_NOT_USE_SCUBA,

		RT_PACK_IS_FULL,
		RT_UNSUITABLE_PACK,
		RT_NOT_ENOUGH_WISDOM, 
		RT_NOT_ENOUGH_STRENGTH,
		RT_NOT_ENOUGH_MOVEPNTS,

		RT_CANNON_WITHOUT_SPACESUIT,
		RT_SPACESUIT_WITH_NO_CANNON,
	};

	//�������� ��������� �������� ������� ������
	static std::string GetReason(reason_type reason);

	enum scheme_type
	{
		ST_GROUND,		// ������ ��� �������� � �����
		ST_USUAL,		// ������ ��� �������� ��������
		ST_FASTACCESS	// ������ ��� �������� ��� fast access
	};

	//������������ �������� ������� (�������� ����� ������)
	static void Activate(scheme_type scheme, HumanEntity* human, BaseThing* thing, human_pack_type hpk);
	//������������ �������� ������� (�������� ����� ������)
	static void Activate(scheme_type scheme, HumanEntity* human, BaseThing* thing,
						 human_pack_type hpk, bool betweenPlayers);

	//����� �� ������� ������������ ��������� � ���������
	static bool IsSuitable(BaseThing* thing, BaseThing* second);
	//����� �� ������� ������������ �������
	static bool IsSuitable(HumanEntity* human, BaseThing* thing);

	//����� �� �������� ������� � ����
	static bool CanInsert(scheme_type scheme, HumanEntity* human, BaseThing* thing,
						  human_pack_type hpk, reason_type* res = 0, bool betwHuman = false);
};

//
// ���������� �� ������� �� ��� �� ����
//
class EpisodeMapMgr{
public:

    static EpisodeMapMgr* GetInst();

    //����� ���������?
    bool IsUpdated() const;
    //�������� ������� ����������
    void DropUpdatedFlag();

    //�������� ���������� ������
    void Reset();
    //�������� ������� � ������
    void Insert(const rid_t& rid);
    //����������/������������
    void MakeSaveLoad(SavSlot& slot);

    class Iterator{
    public:

        Iterator(const rid_set_t::iterator& first) : m_first(first) {}

        const rid_t& operator*() { return *m_first; }
        
        Iterator& operator++ ()  { m_first++; return *this; }
        Iterator  operator++ (int) { Iterator it = *this; ++(*this); return it;}

        friend bool operator != (const Iterator& it1, const Iterator& it2)
        { return it1.m_first != it2.m_first; }
        
    private:

        rid_set_t::iterator m_first;
    };
        
    typedef Iterator iterator;

    //����������� ��� ������� �� ������
    iterator begin();
    iterator end();

private:

    EpisodeMapMgr();

private:

    rid_set_t m_levels;
    bool      m_fupdated;
};

//
// ��������� ��������
//
class NewsPool {
public:

    virtual ~NewsPool(){}

    //singleton
    static NewsPool* GetInst();

    //�������������
    virtual void Init() = 0;
    //���������������
    virtual void Shut() = 0;

    //
    //���������� � �������
    //
    class Info{
    public:

        virtual ~Info(){}

        virtual const rid_t& GetRID() const = 0;

        virtual const std::string& GetDesc() const = 0;
        virtual const std::string& GetShader() const = 0;
        virtual const std::string& GetChannel() const = 0;
    };

    //�������� ������� � ���������
    virtual void Push(const rid_t& rid) = 0;
    //����������/�������� ��������
    virtual void MakeSaveLoad(SavSlot& slot) = 0;

    //
    // ����������� �������� �� ���������
    // 
    class Iterator{
    public:
        
        virtual ~Iterator(){}

        //������� �� �������� ������� 
        virtual void Next() = 0;
        //������� �� ������ �������
        virtual void First() = 0;

        //�������� ������ � ��������
        virtual const Info* Get() = 0;
        //������������ ��� �� ���������?
        virtual bool IsNotDone() const = 0;
    };

    //�������� ����������� ��������
    virtual Iterator* CreateIterator() = 0;  
    
protected:

    NewsPool(){}    
};

//
// ��������� ������� ��������
//
class DiaryManager{
public:

    //singleton
    static DiaryManager* GetInst();

    typedef int key_t;

    enum {
        ROOT_KEY = 0,
        NULL_KEY = -1,
    };

    //
    // ������ � ��������
    //
    class Record{
    public:

        Record(const std::string& info = std::string(), key_t key_up = ROOT_KEY,
               unsigned flags = 0, key_t key = ROOT_KEY):
            m_info(info), m_key(key), m_key_up(key_up), m_flags(flags){}

        key_t GetKey() const { return m_key; }
        key_t GetKeyUp() const { return m_key_up; }
        const std::string& GetInfo() const { return m_info; }

        //��������������� ��� ������� � �������
        void SetKey(key_t key) { m_key = key; }

        enum flag_type{
            F_NEW = 1 << 0, //������� ����� ������
        };

        //������ � �������
        void RaiseFlags(unsigned flags) { m_flags |= flags; }
        void DropFlags(unsigned flags) { m_flags &= ~flags; }
        bool IsRaised(unsigned flags) const { return (m_flags & flags) != 0; }

        friend SavSlotBase& operator << (SavSlotBase& slot, const Record& rec)
        { 
            slot << rec.m_key << rec.m_key_up << rec.m_info << rec.m_flags; 
            return slot;
        }

        friend SavSlotBase& operator >> (SavSlotBase& slot, Record& rec)
        {
            slot >> rec.m_key >> rec.m_key_up >> rec.m_info >> rec.m_flags;
            return slot;
        }

        void SetInfo(const std::string& info) { m_info = info; }

    private:

        key_t m_key;
        key_t m_key_up;

        std::string m_info;
        unsigned    m_flags;
    };

    //�������� ������ �� ������
    virtual Record* Get(key_t key) = 0;
    //������� ������
    virtual void Delete(key_t key) = 0;
    //�������� ������
    virtual key_t Insert(const Record& rec) = 0;

    //���������/��������� �������
    virtual void MakeSaveLoad(SavSlot& slot) = 0;

    //
    // �������� 
    //
    class Iterator{
    public:

        virtual ~Iterator(){}

        //������� �� �������� ������� 
        virtual void Next() = 0;
        //������� �� ������ �������
        virtual void First(key_t key_up = NULL_KEY) = 0;

        //�������� ������ � ��������
        virtual Record* Get() = 0;
        //������������ ��� �� ���������?
        virtual bool IsNotDone() const = 0;
    };

    //������� ��������
    virtual Iterator* CreateIterator() = 0;

protected:

    DiaryManager(){}
};

//
// ���������� ����������
//
//class HelperManager{
//public:
//
//    virtual ~HelperManager(){}
//
//    //�������� ��������� �� ���������
//    static HelperManager* GetInst();
//
//    //�������������/���������������
//    virtual void Init() = 0;
//    virtual void Shut() = 0;
//
//    //��������� ���������
//    virtual void Tick() = 0;
//    //����������/�������� helper'a
//    virtual void MakeSaveLoad(SavSlot& slot) = 0;
//};

namespace AIUtils{

    enum reason_type{
        RT_NONE,          //��� �������
        RT_NO_AMMO,       //��� ��������
        RT_NO_WEAPON,     //��� ������
        RT_NO_MOVEPNTS,   //��� �����
        RT_OUT_OF_RANGE,  //��� �������� ������������
        RT_NOT_THROWABLE, //���� ������� �� ���������
        RT_IN_CREW        //�������� ��������� � �������
    };

    //�������� hex'��
    void HideLandField();
    void HidePassField();

    //��������� ����������� ���� ���������
    class fos_s
	{
        float m_back, m_sqrBack;
        float m_front, m_sqrFront;
        float m_angle, m_cosHalfAngle;
		float m_dirAngle, m_cosAngle, m_sinAngle;

	public:
		void setBack(float b)
		{
			m_back = b;
			m_sqrBack = b*b;
		}

		void setFront(float f)
		{
			m_front = f;
			m_sqrFront = f*f;
		}

		void setAngle(float angle)
		{
			m_angle = angle;
			m_cosHalfAngle = cosf(m_angle*0.5f);
		}

		void setDirAngle(float angle)
		{
			m_dirAngle = angle;
			m_cosAngle = cosf(m_dirAngle - PId2);
			m_sinAngle = sinf(m_dirAngle - PId2);
		}

		float getDirAngle() const { return m_dirAngle; }
		float getBack() const { return m_back; }
		float getSqrBack() const { return m_sqrBack; }
		float getFront() const { return m_front; }
		float getSqrFront() const { return m_sqrFront; }
		float getAngle() const { return m_angle; }
		float getSinAngle() const { return m_sinAngle; }
		float getCosAngle() const { return m_cosAngle; }
		float getCosHalfAngle() const { return m_cosHalfAngle; }
    };

    //�������� ���� ��������� ��������
    void GetEntityFOS(BaseEntity* entity, fos_s* fos);

    //�������� ������ � ������ ��� ��������
    std::string GetThingNumber(BaseThing* thing);
    
    //���������� �������� ������ rid_t'��
    void SaveLoadRIDSet(SavSlot& slot, rid_set_t& rid_set);

    //���������� ������ ��������
    void ChangeHumanStance(HumanEntity* human, BaseEntity* skip_entity = 0);

	int CalcGrenadeAccuracy(HumanEntity *human, float distance);
	float CalcGrenadeRange(HumanEntity *human, float distance);

    //��������� � �������� �������� ���� ������
    void CalcAndShowActiveExits();

    //��������� � �������� ���� ����� �����������
    void CalcAndShowPassField(BaseEntity* entity);
    //��������� � �������� ���� ������� �� �������
    void CalcAndShowLandField(VehicleEntity* vehicle);

    //��������� ��������� �������� ����� ������ ��������
    void CalcHumanAllyDeath(BaseEntity* entity);
    //��������� ���-�� ����� �� ������ ����
    void CalcMovepnts4Turn(BaseEntity* entity);

    //�������� ����� 
    void AddExp4Kill(int experience, BaseEntity* killer);
    void AddExp4Hack(int experience, HumanEntity* human);
    void AddExp4Quest(int experience, int player);
    void AddExp4Quest(int experience, BaseEntity* actor);

	void GiveMoneyToPlayer(int money);

	void UseScanner(HumanEntity *he, ScannerThing *scanner);

    //����� �� ������� � �������� �� ���� ���
    bool IsAccessibleEntity(BaseEntity* walker, BaseEntity* near_ent);

    //������� ������� ������ � ������
    std::string Reason2Str(reason_type reason);
    //������� ���� �������� � ������
    std::string Shot2Str(shot_type type);
    //������� ������ � ��� ���������
    damage_type Str2Dmg(const std::string& str);
    //������� ��������� � ������
    std::string Dmg2Str(damage_type type, bool full = true);

    //������� ������� ��������
    void ChangeEntityPlayer(BaseEntity* entity, player_type new_player);

    //��������� ����������� ������ �������
    float CalcHackProbability(HumanEntity* actor, VehicleEntity* vehicle);

    //��������� ������������� �������� ��������
    float NormalizeAccuracy(BaseEntity* entity, int accuracy);

	//�������� ����� �� ��������
	float CalcDamage(float distance, float base_damage, float fall_off);

    //������ �������� ��� �������� � �����. ����� ����� ������� (������ �.�. ��������)
    int CalcShootAccuracy(HumanEntity* human, WeaponThing* weapon, const point3& to);
    //����� �� �������� ��������� �� ����� ������ � �������� �����
    bool CanShoot(VehicleEntity* vehicle, const point3& to, reason_type* reason = 0);
    bool CanShoot(HumanEntity* human, WeaponThing* weapon, const point3& to, reason_type* reason = 0);

    //������� ��������� ������ �������
    float CalcThrowRange(HumanEntity* human);
    //����� �� ������� ������� �������?
    bool CanThrow(HumanEntity* human, BaseThing* thing, const point3& to, reason_type* reason = 0);

    //����� �� guard ������������� �� intruder
    bool CanReact(BaseEntity* intruder, BaseEntity* guard);
    //����� �� ������� ������� ����
    bool CanChangePose(HumanEntity* human, reason_type* reason = 0);

    //��������� ���-�� ����� ��� ��������� ��������
    int CalcMps2Act(BaseThing* thing);
    //���������� ���-�� ����� �� �������� �������� c �����
    int CalcMps4ThingPickUp(BaseThing* thing);
    //��������� ���-�� ����� ��� �������� ��������
    int CalcMps4EntityRotate(BaseEntity* entity, const point3& dir);
    int CalcMps4EntityRotate(BaseEntity* entity, float dir);
    int CalcMps4EntityRotate(float delta);
    //��������� ���-�� ����� ��� levelup'a
    int CalcLevelupPoints(HumanEntity* human, int cur_lvl, int old_lvl);

	int CalcEfficiency(BaseEntity *entity, const point3 &to);

    //���������� �������
    void MakeTreatment(HumanEntity *doctor, HumanEntity* sick, MedikitThing* medikit);
	// ���������� ����
	void MakeSwap(HumanEntity *source, BaseEntity *target);

	/// ���������� �����������
	void DisplayDamages(BaseEntity *entity, const damage_s *dmgs, int count, bool print_if_none = true);

	// ���������� ��������� (�� � � �) ���������� ����� ����������� �������
	float dist2D(const point3& p1, const point3& p2);
}

#endif // _PUNCH_AIUTILS_H_
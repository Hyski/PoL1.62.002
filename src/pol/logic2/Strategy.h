//
// ��������� - ���������� ��������� ������ � ����������
//

#ifndef _PUNCH_STRATEGY_H_
#define _PUNCH_STRATEGY_H_

class Marker;
class Spectator;

class BaseEntity;
class HumanEntity;
class VehicleEntity;

//
// ������� ����� ��� ���� ���������
//
class BaseStrategy{
public:
    virtual ~BaseStrategy(){}
    //�������� �������� runtime
    DCTOR_ABS_DEF(BaseStrategy)
    //���������� / ��������
    virtual void MakeSaveLoad(SavSlot& st) = 0;   
};

//
// ����� - ������������� ��������� ������ � ������
//
class CrewStrategy : public BaseStrategy{
public:
    //����� ������� � �������?
    virtual bool CanJoin(VehicleEntity* vehicle, HumanEntity* human) const = 0;
    //�������� � �������
    virtual void Inc(VehicleEntity* vehicle, HumanEntity* human) = 0;
    //������� �� �������
    virtual void Dec(VehicleEntity* vehicle, HumanEntity* human) = 0;
    //���������/��������� ������� (����� ��� ���������)
    virtual void Enable(VehicleEntity* vehicle, bool flag) = 0;
    //������ ������ �������
    virtual int GetCrewSize(VehicleEntity* vehicle) const = 0;
    //������ ������� ��� ���� �������
    virtual HumanEntity* GetDriver(VehicleEntity* vehicle) = 0;
};

//
// ������������� ��������� ������ ��������
//
class DeathStrategy : public BaseStrategy{
public:

    // ��������� �������� ������
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // �������:  
    //          true  - ������� �������� �� ������� �������
    //          false - �� ������� �������� �� ������� �������
    //
    virtual bool MakeDeath(BaseEntity* actor, BaseEntity* killer) = 0;
};

//
// ������������� ��������� ������������
//
class MoveStrategy : public BaseStrategy{
public:
    
    //������ ���-�� ����� ��� ����� ������� ��������
    virtual int GetSteps() const = 0;
    //������ ��������� ������ hex'��
    virtual int GetHexCost() const = 0;
};

//
// ��������� ������ ������ ������
//
class PanicTypeSelector : public BaseStrategy{
public:

    //��������� ��������� � ������
    virtual human_panic_type Select(HumanEntity* human) = 0;
    //������ ���������� ��������
    virtual human_panic_type GetPanic(HumanEntity* human) const = 0;
    //������������� ���������� ������ ������
    virtual human_panic_type SetPanic(HumanEntity* human, human_panic_type type) = 0;
};

//
// ��������� �������� ������� � ��������� ��������
//
class FastAccessStrategy : public BaseStrategy{
public:

    //
    // �������� �� ��������� ������� ����� �������� � ����
    //
    class Iterator{
    public:

        virtual ~Iterator(){}

        //���������� �� ������ ������������������
        virtual void First(BaseThing* from = 0, unsigned mask = TT_ALL_ITEMS) = 0;

        //������� � ��������� ��������
        virtual void Next() = 0;
        //������������ ��� �� ���������?
        virtual bool IsDone() const = 0;

        //�������� ������ �� �������
        virtual BaseThing* Get() = 0;
    };

    enum iter_type{
        IT_FORWARD,     //������������ ������
        IT_BACKWARD,    //������������ �����
    };

    //������ ������� ������� �������� �������
    virtual BaseThing* GetThing() = 0;
    //���������� ������� ������� �������� �������
    virtual void SetThing(BaseThing* thing) = 0;

    //�������� �� ������� ������ � ��������� ��� ���������
    virtual bool CanMakeFastAccess() = 0;
    //������� �������� �� ��������� ��������
    virtual Iterator* CreateIterator(iter_type type) = 0;
};

//
// ��������� ��� ������������ �����
//
class SoundStrategy : public BaseStrategy{
public:

};

//=====================================================================================//
//                                class StrategyFactory                                //
//=====================================================================================//
// ������� ����������� ������ ��� �������
class StrategyFactory
{
public:
    enum spectator_type{
        ST_USUAL,   //������� ���������
        ST_TRADER,  //��������� ��� ��������
    };

    enum crew_type{
        CT_USUAL,    //������� ���������
        CT_BUSY,     //��������� ������� ������
        CT_BUS,      //��� ��������
    };

    enum death_type{
        DT_HUMAN,    //������ ��������
        DT_TRADER,   //������ ��������
        DT_VEHICLE,  //������ �������
        DT_FERALCAR, //������ ���������������� ������
    };

    enum move_type{
        MT_HUMAN,    //����������� ��������
        MT_VEHICLE,  //����������� ������
    };

    enum marker_type{
        MRK_USUAL, //�������
    };

    enum panic_selector_type{
        PST_USUAL,
    };

    enum sound_type{
        SND_HUMAN,
        SND_TRADER,
    };

    enum fast_access_type{
        FAT_USUAL,
    };

    //�������� ������������
    static Spectator* CreateSpectator(spectator_type tepe, BaseEntity* entity);
    //�������� ��������� ������
    static CrewStrategy* CreateCrew(crew_type type, VehicleEntity* vehicle);
    //�������� �������� ������ ��������
    static DeathStrategy* CreateDeath(death_type, BaseEntity* entity);
    //������� �������� �����������
    static MoveStrategy* CreateMove(move_type type, BaseEntity* entity);
    //������� ������
    static Marker* CreateMarker(marker_type type, BaseEntity* entity);
    //�������� ��������� ������� ������
    static PanicTypeSelector* CreatePanicSelector(panic_selector_type type);
    //������� ��������� �����
    static SoundStrategy* CreateSound(sound_type type, BaseEntity* entity);
    static FastAccessStrategy* CreateFastAccess(fast_access_type type, HumanEntity* entity);
};

#endif // _PUNCH_STRATEGY_H_
//
// ������ ��� ��������� � ������ ������ �� ������ �������
//

#ifndef _PUNCH_ENTITYAUX_H_
#define _PUNCH_ENTITYAUX_H_

class BaseThing;

class BaseEntity;
class HumanEntity;
class TraderEntity;
class VehicleEntity;

//
// ����� ���������� ��� ��������
//
class EntityVisitor{
public:

    virtual ~EntityVisitor(){}

    virtual void Visit(HumanEntity* human) {}
    virtual void Visit(TraderEntity* trader) {}
    virtual void Visit(VehicleEntity* vehicle) {}
};

//
// ����� ��� ���������� pattern'a �����������
//
class EntityObserver : public Observer<BaseEntity*>{
public:

    virtual ~EntityObserver() ;

    //������� ��� ������������ ����������
	enum event_type{
		EV_NONE,

		EV_DESTROY,      //�������� ��������� � ������ (delete entity)
		EV_QUIT_TEAM,    //����� �� �������
		EV_JOIN_TEAM,    //������ � �������
		EV_FOS_CHANGE,   //��������� ���� ���������
		EV_DEATH_PLAYED, //���������� ����� ������ �������� DeathStrategy
		EV_PREPARE_DEATH,//���������� �� ������ �������� DeathStrategy

		EV_CREW_CHANGE,      //��������� ������� ������� (humans and vehicle)
		EV_HEALTH_CHANGE,    //��������� �������� �������� (humans and vehicle)
		EV_LEVELUP_CHANGE,   //��������� ���-�� ����� �� levelup (humans only)
		EV_MOVEPNTS_CHANGE,  //��������� ���-�� movepoints �������� (humans and vehicle)
		EV_SELECTION_CHANGE, //��������� ��������� ��������
		EV_BODY_PART_CHANGE, //��������� ��������� ����� ���� ��������

		EV_WISDOM_CHANGE,    //��������� ��������� �������� (humans only)
		EV_MORALE_CHANGE,    //��������� ��������� ������ (humans only)
		EV_ACCURACY_CHANGE,  //��������� ��������� �������� (humans only)
		EV_REACTION_CHANGE,  //��������� ��������� ������� (humans only)
		EV_MECHANICS_CHANGE, //��������� ��������� �������� (humans only)
		EV_EXPERIENCE_CHANGE,//��������� ��������� ����� (humans only) 

		EV_NO_LOF,           //����������� ����� ��������
		EV_PANIC_CHANGE,     //��������� ������ ������        
		EV_HEX_COST_CHANGE,  //��������� ��������� hex'a
		EV_DEXTERITY_CHANGE, //��������� �����������
		EV_MOVE_TYPE_CHANGE, //��������� ���� ������������ (humans only)
		EV_POSE_TYPE_CHANGE, //����� ���� (humans only)
		EV_WEIGHT_CHANGE,    //��������� ���������� ���� �������� (humans only)
		EV_STRENGTH_CHANGE,  //��������� ��������� ����

		EV_INSERT_THING,     //�������� �������
		EV_REMOVE_THING,     //������� �������

		EV_CAN_MAKE_FAST_ACCESS, //���������� �������� ������� � �������� ����� ������������

		EV_BODY_PACK_CHANGE,   //��������� ����������� �� ���� (humans only)
		EV_HANDS_PACK_CHANGE,  //��������� ����������� � ����� (humans only)
		EV_LONG_DAMAGE_CHANGE, //��������� ������� ��������������� ����������� (humans only)

		EV_TAKE_MONEY,         //�������� ���� ������  
		EV_INGOING_CHANGE,     //��������� ������ ���������� (humans + vehicles)
		EV_RELATION_CHANGE,    //��������� ��������� ���������� ������ � ��������

		EV_WEAPON_RELOAD,      //����������� ������  
		EV_WEAPON_STATE_CHANGE,//��������� ���-�� �������� � ������ �������� (humans and vehicles)

		EV_LUCKY_STRIKE,       //�������� ������� ����������� ����������� �������
		EV_CRITICAL_HURT,      //���� ����������� ����������� (humans only)

		EV_MOVE_COMMAND,       //������� �� ������������
		EV_SELECT_COMMAND,     //������� �� ��������� ���������
		EV_ATTACK_COMMAND,     //������� �� ����� 

		EV_ACTIVITY_INFO,      //��������� �������� ���������� � ���������

		EV_TEAMMATE_DISMISS,   //���������� ��������� �� ������� (humans only)
		EV_TEAMMATE_LEAVE,     //�������� ������ �� ������� (humans only)
		EV_TEAMMATE_COME,      //�������� ������ � ������� (humans only)
		EV_TEAMMATE_JOIN,      //�������� �������� �� ������� (humans only)

		EV_OPEN_FAILED,
		EV_OPEN_SUCCEED,

		EV_LEVELUP,            //��������� levelup
		EV_CAR_DRIVE_FAILED,   //�� ���������� ����� ������

		EV_USE_PLASMA_GRENADE, //������ ����������� ������� 

		EV_RANK_UP,			   // ��������� �� ������
		EV_MAXHEALTH_CHANGE,   // ��������� ������������� ��������
		EV_MAXMOVEPNTS_CHANGE  // ��������� ������������� ���������� ����� ����
	};

    //���������� � �������� 
    struct entity_info_s{
        BaseEntity* m_entity;
        entity_info_s(BaseEntity* entity) : m_entity(entity) {}
    };

    //��������� ��������
    struct activity_info_s{

        enum activity_event {
            AE_BEG, //������
            AE_END, //�����
        };

        activity_event m_event;

        bool IsEnd() const      { return m_event == AE_END; } 
        bool IsBegining() const { return m_event == AE_BEG; }

        activity_info_s(activity_event event) : m_event(event) {}
    };    

    //���������� � ��������
    struct thing_info_s{
        BaseThing* m_thing;

        thing_info_s(BaseThing* thing) : m_thing(thing) {}
    };
};

#endif // _PUNCH_ENTITYAUX_H_
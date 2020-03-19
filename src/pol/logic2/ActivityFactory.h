#pragma once

#include "Entity.h"
#include "EntityRef.h"

class Activity;

//=====================================================================================//
//                                class ActivityFactory                                //
//=====================================================================================//
// фабрика действий
class ActivityFactory
{
public:
    enum controller_type
	{
        CT_NONE        = 0,
        CT_UPDATE_MPS  = 1 << 0,   //считать ходы во врем€ действи€ 
        CT_UPDATE_VIS  = 1 << 1,   //пересчитывать видимость в конце действи€
        CT_MPS_LIMIT   = 1 << 2,   //ограничить действие количеством ходов  
        CT_STEP_LIMIT  = 1 << 3,   //ограничить действие колличеством шагов
        CT_PATH_SHOW   = 1 << 4,   //показывать путь со врем€ действи€
        CT_USUAL_PASS  = 1 << 5,   //проверка проходмости дл€ ходьбы
        CT_BUS_PASS    = 1 << 6,   //проверка проходимости дл€ автобуса
        CT_STAY_CHANGE = 1 << 7,   //смена стойки в зависимости от кол-ва врагов

        CT_ENEMY_STOP         = 1 << 8,  //остановить сущеcтво если виден враг 
        CT_PLAY_REACTION      = 1 << 9,  //проиграть реакцию персонажа
        CT_PLAY_FIRST_PHRASE  = 1 << 10, //проиграть первую фразу

        CT_ACCURACY_DECREASE  = 1 << 11, //уменьшать точность если существо бежит

        CT_HURT_SHOOT  = 1 << 12,  //стратеги€ стрельбы считающа€ поражени€
        CT_TRACE_LOF   = 1 << 13,  //трассировать линию стрельбы 

        CT_SEND_EVENTS = 1 << 14,  //рассылать событи€
        CT_PRINT_MSGS  = 1 << 15,  //выводить сообщени€
        CT_PRINT_DMGS  = 1 << 16,  //выводить сообщени€ о повреждении

        CT_SWITCH_OBJECT = 1 << 17, //переключить состо€ние объекта (UseActivity only)

		CT_NO_AMMO_DEC = 1 << 18, // не убавл€ть патроны
		CT_NO_MOVE_INTERRUPT = 1 << 19, // Ќе прерывать перемещение другими действи€ми

        //profile дл€ поворота
        CT_CIVILIAN_ROTATE = CT_UPDATE_MPS|CT_UPDATE_VIS,
        CT_ROTATE_FOR_USE  = CT_UPDATE_VIS,
        CT_TALK_ROTATE     = CT_UPDATE_VIS|CT_STAY_CHANGE,
        CT_PLAYER_ROTATE   = CT_UPDATE_MPS|CT_UPDATE_VIS|CT_MPS_LIMIT|CT_STAY_CHANGE|CT_ENEMY_STOP|CT_PRINT_MSGS,
		CT_ENEMY_LOOKROUND = CT_UPDATE_MPS|CT_UPDATE_VIS|CT_MPS_LIMIT|CT_STAY_CHANGE|CT_ENEMY_STOP,
		CT_ENEMY_ROTATE    = CT_UPDATE_MPS|CT_UPDATE_VIS|CT_MPS_LIMIT|CT_STAY_CHANGE,

        //profile дл€ перемещени€
        CT_BUS_MOVE       = CT_UPDATE_VIS/*|CT_PATH_SHOW*/|CT_BUS_PASS,
        CT_CIVILIAN_CAR_MOVE = CT_UPDATE_MPS|CT_UPDATE_VIS|CT_MPS_LIMIT|CT_BUS_PASS,
        CT_PLAYER_MOVE    = CT_UPDATE_MPS|CT_UPDATE_VIS|CT_STEP_LIMIT|CT_PATH_SHOW|CT_USUAL_PASS|CT_STAY_CHANGE|CT_PLAY_FIRST_PHRASE|CT_ENEMY_STOP|CT_PLAY_REACTION|CT_PRINT_MSGS|CT_ACCURACY_DECREASE,
		CT_PLAYER_MOVE_ACT = CT_USUAL_PASS|CT_NO_MOVE_INTERRUPT,
        CT_CIVILIAN_MOVE  = CT_UPDATE_MPS|CT_UPDATE_VIS|CT_MPS_LIMIT/*|CT_PATH_SHOW*/|CT_USUAL_PASS,
        CT_ENEMY_MOVE     = CT_UPDATE_MPS|CT_UPDATE_VIS|CT_MPS_LIMIT/*|CT_PATH_SHOW*/|CT_USUAL_PASS|CT_STAY_CHANGE|CT_ENEMY_STOP|CT_PLAY_REACTION,
        CT_ENEMY_MOVEIGNORE  = CT_UPDATE_MPS|CT_UPDATE_VIS|CT_MPS_LIMIT/*|CT_PATH_SHOW*/|CT_USUAL_PASS|CT_PLAY_REACTION,
        CT_ENEMY_RETREAT = CT_UPDATE_MPS|CT_UPDATE_VIS|CT_MPS_LIMIT/*|CT_PATH_SHOW*/|CT_USUAL_PASS|CT_PLAY_REACTION,
 
        //profile дл€ use
        CT_PLAYER_USE_FAILED  = CT_UPDATE_VIS|CT_PRINT_MSGS|CT_STAY_CHANGE|CT_UPDATE_MPS,
        CT_PLAYER_USE_SUCCEED = CT_UPDATE_VIS|CT_PRINT_MSGS|CT_STAY_CHANGE|CT_SWITCH_OBJECT|CT_UPDATE_MPS,
		CT_PLAYER_USE_SSCENE  = CT_UPDATE_VIS|CT_PRINT_MSGS|CT_STAY_CHANGE|CT_SEND_EVENTS|CT_UPDATE_MPS,

        //profile дл€ погрузки
        CT_PLAYER_SHIPMENT    = CT_UPDATE_MPS|CT_UPDATE_VIS|CT_PRINT_MSGS|CT_STAY_CHANGE,

        //profile дл€ выгрузки
        CT_PLAYER_LANDING     = CT_UPDATE_MPS|CT_UPDATE_VIS|CT_PRINT_MSGS|CT_STAY_CHANGE,

        //profile дл€ стрельбы
        CT_PLAYER_SHOOT       = CT_PRINT_DMGS|CT_UPDATE_VIS|CT_UPDATE_MPS|CT_HURT_SHOOT|CT_STAY_CHANGE|CT_SEND_EVENTS|CT_PRINT_MSGS|CT_TRACE_LOF,
        CT_PLAYER_SHOOT_ACT   = CT_UPDATE_VIS|CT_SEND_EVENTS|CT_NO_AMMO_DEC,
        CT_CIVILIAN_SHOOT     = CT_PRINT_DMGS|CT_UPDATE_VIS|CT_UPDATE_MPS|CT_MPS_LIMIT|CT_HURT_SHOOT|CT_STAY_CHANGE|CT_SEND_EVENTS|CT_TRACE_LOF,
        CT_ENEMY_SHOOT        = CT_PRINT_DMGS|CT_UPDATE_VIS|CT_UPDATE_MPS|CT_MPS_LIMIT|CT_HURT_SHOOT|CT_STAY_CHANGE|CT_SEND_EVENTS|CT_TRACE_LOF,
        CT_REACTION_SHOOT     = CT_PRINT_DMGS|CT_UPDATE_VIS|CT_UPDATE_MPS|CT_MPS_LIMIT|CT_HURT_SHOOT|CT_STAY_CHANGE|CT_SEND_EVENTS|CT_TRACE_LOF,
        CT_ENEMY_SHOOT2       = CT_PRINT_DMGS|CT_UPDATE_MPS|CT_MPS_LIMIT|CT_STAY_CHANGE,
        CT_BERSERK_SHOOT      = CT_PRINT_DMGS|CT_UPDATE_VIS|CT_UPDATE_MPS|CT_MPS_LIMIT|CT_HURT_SHOOT|CT_STAY_CHANGE|CT_SEND_EVENTS,
        CT_ENEMY_THROW        = CT_PRINT_DMGS|CT_UPDATE_VIS|CT_UPDATE_MPS|CT_MPS_LIMIT|CT_HURT_SHOOT|CT_STAY_CHANGE|CT_SEND_EVENTS,
        CT_SCRIPT_SCENE_SHOOT = 0,    

        //profile дл€ броска гранаты
        CT_PLAYER_GRENADE_THROW = CT_PRINT_DMGS|CT_UPDATE_VIS|CT_UPDATE_MPS|CT_HURT_SHOOT|CT_STAY_CHANGE|CT_SEND_EVENTS|CT_PRINT_MSGS
    };

    //информаци€ о стрельбе (необх дл€ просчета линнии стрельбы)
    class shoot_info_s;

    //создать поворот существа
    static Activity* CreateRotate(BaseEntity* entity, float angle, unsigned controller);
    //создать поворот существа
    static Activity* CreateRotate(BaseEntity* entity, float angle, int maxMovePoints, unsigned controller);
    //создать use
    static Activity* CreateUse(BaseEntity* entity, const rid_t& rid, unsigned deorators = CT_NONE);
    //создать высадку из техники
    static Activity* CreateLanding(HumanEntity* human,const ipnt2_t& to, unsigned decorators = CT_NONE);
    //создать движение существом
    static Activity* CreateMove(BaseEntity* entity, const pnt_vec_t& path, unsigned controller = CT_NONE);
    //создать посадку в технику
    static Activity* CreateShipment(HumanEntity* human, VehicleEntity* vehicle, unsigned decorators = CT_NONE);
    //создать бросок гранаты
    static Activity* CreateThrow(HumanEntity* human, unsigned flags, const shoot_info_s& info, float accuracy = 0.0f);
    //создать стрельбу
    static Activity* CreateShoot(BaseEntity* entity, float accuracy, unsigned controller, const shoot_info_s& info);
	// —оздать мультиперемещение
	static Activity *CreateMultiMove(const std::vector<BaseEntity *> &, const std::vector<pnt_vec_t> &, unsigned ctrl = CT_NONE);
};

//=====================================================================================//
//                         class ActivityFactory::shoot_info_s                         //
//=====================================================================================//
class ActivityFactory::shoot_info_s
{
	enum TargetType
	{
		tEmpty,
		tPoint,
		tObject,
		tEntity
	};

	point3 m_point;	 // стрел€ем в точку
    rid_t m_object;  // стрел€ем в объект
	PoL::BaseEntityRef m_entity;  // стрел€ем в существо
	TargetType m_type;

	mutable point3 m_shotAim;
	mutable point3 m_approxPos;

public:	
	//shoot_info_s() : m_entity(0) {}
    shoot_info_s(eid_t eid);
    shoot_info_s(const rid_t& rid) : m_object(rid), m_entity(0), m_type(tObject) {}
    shoot_info_s(const point3& pt) : m_point(pt), m_entity(0), m_type(tPoint) {}

	const rid_t &GetObject() const { return m_object; }
	BaseEntity *GetEntity() const;
	eid_t GetEntityId() const { return IsEntity() ? m_entity->GetEID() : 0; }
	const point3 &GetPoint() const { return m_point; }

	point3 GetApproxPos() const;
	point3 GetShotAim() const;

	void ClearEntity() { if(IsEntity()) { m_entity.reset(); m_type = tEmpty; } }

	bool IsObject() const { return m_type == tObject; }
	bool IsEntity() const { return m_type == tEntity; }
	bool IsPoint() const { return m_type == tPoint; }
};

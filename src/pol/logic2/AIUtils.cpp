//
// реализация утилит для AI
//
#pragma warning(disable:4786)

#include "logicdefs.h"

#include "form.h"
#include "Spawn.h"
#include "thing.h"
#include "entity.h"
#include "VisMap.h"
#include "Marker.h"
#include "aiutils.h"
#include "graphic.h"
#include "hexgrid.h"
#include "Damager.h"
#include "sndutils.h"
#include "activity.h"
#include "hexutils.h"
#include "Relation.h"
#include "Spectator.h"
#include "cameraman.h"
#include "xlsreader.h"
#include "entityaux.h"
#include "PathUtils.h"
#include "TableUtils.h"
#include "traceutils.h"
#include "DirtyLinks.h"
#include "gameobserver.h"
//#include "phrasemanager.h"
#include "enemydetection.h"
#include "panicbehaviour.h"
#include "PlayerTeamSpectator.h"
#include <undercover/gamelevel/GameLevel.h>
#include <common/3deffects/effectmanager.h>

#ifdef _HOME_VERSION
#include <common/CmdLine/CmdLine.h>
#endif

namespace AIUtilsDetail
{
}

using namespace AIUtilsDetail;

//==============================================================

namespace AIUtilsDetail
{

    const float vehicle_back_fos_radius = 1.5f;

    const char* bartolomiu_sys_name = "bartolomiu";

	const std::string news_xls = "scripts/logic/news.txt";
    const char* helper_xls = "scripts/logic/helper.txt";
    const char* levelup_xls_name = "scripts/logic/levelups.txt";
    const char* phrases_xls_name = "scripts/logic/phrases_";

    //узнать радиус стрельбы (предполагается что оружие заряжено)
    bool GetShootRadius(WeaponThing* weapon, const point3& from, const point3& to, float* radius)  
    {
		// By Flif
		/*
        point3 dir = to - from;
        *radius = dir.Length();*/
		*radius = AIUtils::dist2D(from, to);
		// End By Flif
        return *radius < weapon->GetInfo()->GetRange() + weapon->GetAmmo()->GetInfo()->GetRange(); 
    }

    void ApplyHealthChange(HumanContext* human, int delta)
    {   
        HumanContext::Traits* traits = human->GetTraits();
        HumanContext::Limits* limits = human->GetLimits();
        
        if(traits->GetHealth() != limits->GetHealth()){
            
            float factor = static_cast<float>(delta)/static_cast<float>(limits->GetHealth());
            
            //traits->AddStrength(factor * limits->GetStrength());
            //traits->AddReaction(factor * limits->GetReaction());
            traits->AddAccuracy(factor * limits->GetAccuracy());
            traits->AddDexterity(factor * limits->GetDexterity());
            //traits->AddMechanics(factor * limits->GetMechanics());
            //traits->AddFrontRadius(factor * limits->GetFrontRadius());
            
        } else {
            
            //traits->AddStrength(limits->GetStrength());
            //traits->AddReaction(limits->GetReaction());
            traits->AddAccuracy(limits->GetAccuracy());
            traits->AddDexterity(limits->GetDexterity());
            //traits->AddMechanics(limits->GetMechanics());
            //traits->AddFrontRadius(limits->GetFrontRadius());
        }
    }

    void ApplyBodyPartsDamage(HumanContext* human, int delta)
    {
        if(human->GetTraits()->GetHealth() == human->GetLimits()->GetHealth()){

            for(int k = 0; k < MAX_BODY_PARTS; k++){
                body_parts_type bp = static_cast<body_parts_type>(k);
                human->SetBodyPartDamage(bp, 0);
            }

            return;
        }

        int bp_dmg = delta/MAX_BODY_PARTS;        
        if(bp_dmg == 0) bp_dmg = delta > 0 ? 1 : -1;
        
        for(int k = 0; k < MAX_BODY_PARTS; k++){
            body_parts_type bp = static_cast<body_parts_type>(k);
            human->SetBodyPartDamage(bp, human->GetBodyPartDamage(bp) + bp_dmg);
        }
    }  
    
    void DisplayHealthIncrease(HumanEntity* human, int delta)
    {
        DirtyLinks::Print( mlprintf( DirtyLinks::GetStrRes("aiu_got_health").c_str(), human->GetInfo()->GetName().c_str(), delta));
        EffectMgr::GetInst()->MakeHumanTreatment(human->GetGraph()->Cast2Human());
    }
}

//==============================================================

GameEvMessenger* GameEvMessenger::GetInst()
{
    static GameEvMessenger imp;
    return &imp;
}

//=====================================================================================//
//                           void AIUtils::DisplayDamages()                            //
//=====================================================================================//
void AIUtils::DisplayDamages(BaseEntity* entity, const damage_s *dmgs, int count, bool print_if_none)
{
    std::ostringstream ostr;

    for(int k = 0; k < count; k++)
	{
        if(dmgs[k].m_val && dmgs[k].m_type != DT_NONE)
		{
            ostr << ' ' << dmgs[k].m_val << '(' << AIUtils::Dmg2Str(dmgs[k].m_type, false) << ')';
        }
    }

    if(ostr.str().size() || print_if_none)
	{
        DirtyLinks::Print(  mlprintf(DirtyLinks::GetStrRes("aiu_entity_damaged").c_str(), 
                            entity->GetInfo()->GetName().c_str(),
                            ostr.str().c_str()));
    }
}

//==============================================================

namespace AIUtilsDetail
{
    //
    // узнать может ли дойти существо
    //
    class AccessibilityCalcer : public EntityVisitor{
    public:

        AccessibilityCalcer(BaseEntity* to) :
          m_entity(to), m_result(false){}

        void Visit(HumanEntity* human)
        {
            CalcAccessibility(human->GetEntityContext());
        }

        void Visit(VehicleEntity* vehicle)
        {
            CalcAccessibility(vehicle->GetEntityContext());
        }

        bool GetResult() const { return m_result;}

    private:

        void CalcAccessibility(ActiveContext* active)
        {            
            m_result =      PathUtils::IsNear(m_entity)
                        &&  (	UnlimitedMoves()
							||	PathUtils::GetNearPnt(m_entity).m_cost <= active->GetStepsCount()+1)
						;            
        }

    private:

        bool m_result;
        BaseEntity* m_entity;
    };
}

//=========================================================

namespace AIUtilsDetail{

    //
    // узнать реакцию существа
    //
    class EntityReactionCalcer : public EntityVisitor{
    public:

        EntityReactionCalcer() : m_result(0) {}

        void Visit(HumanEntity* human)
        {
            m_result = human->GetEntityContext()->GetTraits()->GetReaction();
        }

        void Visit(VehicleEntity* vehicle)
        {
            m_result = vehicle->GetInfo()->GetReaction();
        }

        int GetResult() const{ return m_result; }

    private:

        int m_result;
    };
}

//=========================================================

namespace AIUtilsDetail
{
    //
    // расчитать параметры существа на ход
    //
    class EntityTurnCalcer : public EntityVisitor
	{
    public:
        void Visit(HumanEntity* human)
        {
            HumanContext* context = human->GetEntityContext();
			if(context->GetTraits()->GetHealth() == 0) return;

            //подсчитать количество ходов
            context->GetTraits()->SetMovepnts(context->GetLimits()->GetMovepnts());

            //рост морали за ход
            if(context->GetTraits()->GetMorale() < context->GetLimits()->GetMorale())
			{
                context->GetTraits()->AddMorale(10 * human->GetInfo()->GetFibre());
			}
            
            damage_s dmgs[3];

            //учтем поражения продолж. в несколько ходов
			if(int health_damage = context->GetAffect(HumanContext::apHealth)) // GetLongDamageSum(context, DT_FLAME))
			{
                context->GetTraits()->AddHealth(-health_damage);
                
                ApplyHealthChange(context, -health_damage);
                ApplyBodyPartsDamage(context, health_damage);

                dmgs[0].m_type = DT_FLAME;
                dmgs[0].m_val = health_damage;

                //паночка померла?
                if(context->GetTraits()->GetHealth() == 0)
				{
                    context->PlayDeath(0);
                    return;
                }
            }

			if(int accuracy_add = context->GetLastAffect(HumanContext::apAccuracy) + context->GetLastAccuractRunAffect())
			{
                context->GetTraits()->PoLAddAccuracy(accuracy_add);
            }

			if(int dexterity_add = context->GetLastAffect(HumanContext::apDexterity))
			{
                context->GetTraits()->PoLAddDexterity(dexterity_add);
            }

			if(int movepnts_damage = context->GetAffect(HumanContext::apMaxMovepoints))
			{
                context->GetTraits()->PoLAddMovepnts(-movepnts_damage);
            }

			context->ShiftLongDamages();

			AIUtils::DisplayDamages(human, dmgs, 3, false);

            if(context->GetCrew() == 0)
			{
                ipnt2_t pos2 = human->GetGraph()->GetPos2();
                RiskSite* risk = HexGrid::GetInst()->Get(pos2).GetRisk();
                if(risk) EntityDamager::GetInst()->Damage(human, risk, EntityDamager::CALC_TURN_FALGS);

                //паночка померла?
                if(context->GetTraits()->GetHealth() == 0)
				{
                    context->PlayDeath(0);
                    return;
                }
            }

            //запомним текущий уровень здоровья
            int previous_health = context->GetTraits()->GetHealth();

            //учтем прибавок от импланта
            context->GetTraits()->AddHealth(context->GetTraits()->GetConstHealth());

            //увеличить/уменьшить парамтры здоровья
            if(int delta = context->GetTraits()->GetHealth() - previous_health)
			{
                DisplayHealthIncrease(human, delta);
                ApplyHealthChange(context, delta);
                ApplyBodyPartsDamage(context, - delta);
            }
        }

        void Visit(VehicleEntity* vehicle)
        {         
            vehicle->GetEntityContext()->SetMovepnts(vehicle->GetInfo()->GetMovepnts());
        }
    };
}

//=========================================================

LevelupMgr::LevelupMgr()
{
    TxtFilePtr txt(levelup_xls_name);
    
    std::string str;
    std::istringstream istr;
    
    for(int line = 0; line < txt->SizeY(); line++){
        txt->GetCell(line, 0, str);

        istr.str(str);
        istr.clear();

        m_points.push_back(0);
        istr >> m_points.back();
    }            
}

LevelupMgr* LevelupMgr::GetInst()
{
    static LevelupMgr imp;
    return &imp;
}

int LevelupMgr::GetCurrentLevel(int experience)
{
    for(size_t k = 0; k < m_points.size(); k++){
        if(experience < m_points[k])
            return k - 1;
    }

    return m_points.size() - 1;
}

int LevelupMgr::GetLevelPoints(int level)
{
    if(level < (int)m_points.size()) return m_points[level];
    return m_points.back();
}

//=========================================================

void AIUtils::ChangeHumanStance(HumanEntity* human, BaseEntity* skip_entity)
{
    if(human->GetInfo()->IsCivilian())
	{
        human->GetGraph()->Cast2Human()->ChangeStance(GraphHuman::ST_USUAL);
        return;
    }

    VisMap::MarkIterator_t itor = VisMap::GetInst()->MarkBegin(human->GetEntityContext()->GetSpectator(), VisParams::vpVisible);
    for(; itor != VisMap::GetInst()->MarkEnd(); ++itor)
	{
        BaseEntity* visible = itor->GetEntity();

        if(     visible 
            &&  (skip_entity == 0 || visible->GetEID() != skip_entity->GetEID())
            &&  EnemyDetector::getInst()->isHeEnemy4Me(human, visible)){
            human->GetGraph()->Cast2Human()->ChangeStance(GraphHuman::ST_WAR);
            return;
        }
    }
    
    human->GetGraph()->Cast2Human()->ChangeStance(GraphHuman::ST_USUAL);
}

//=====================================================================================//
//                         bool AIUtils::IsAccessibleEntity()                          //
//=====================================================================================//
bool AIUtils::IsAccessibleEntity(BaseEntity* walker, BaseEntity* near_entity)
{
    AccessibilityCalcer calcer(near_entity);
    walker->Accept(calcer);
    return calcer.GetResult();
}

//=====================================================================================//
//                        void AIUtils::CalcAndShowPassField()                         //
//=====================================================================================//
void AIUtils::CalcAndShowPassField(BaseEntity* entity)
{
    pnt_vec_t front;
    PathUtils::CalcPassField(entity, &front);
	if(!entity->Cast2Human() || !entity->Cast2Human()->GetEntityContext()->GetCrew())
	{
		GraphGrid::GetInst()->Show(GraphGrid::HT_FRONT, true);
		GraphGrid::GetInst()->SetHexes(GraphGrid::HT_FRONT, front);
	}
}

//=====================================================================================//
//                        void AIUtils::CalcAndShowLandField()                         //
//=====================================================================================//
void AIUtils::CalcAndShowLandField(VehicleEntity* vehicle)
{
    pnt_vec_t front;
    PathUtils::CalcLandField(vehicle, &front);
    GraphGrid::GetInst()->Show(GraphGrid::HT_LAND, true);
    GraphGrid::GetInst()->SetHexes(GraphGrid::HT_LAND, front);
}

void AIUtils::HidePassField()
{
    GraphGrid::GetInst()->Show(GraphGrid::HT_FRONT, false);
}

void AIUtils::HideLandField()
{
    GraphGrid::GetInst()->Show(GraphGrid::HT_LAND, false);
}

damage_type AIUtils::Str2Dmg(const std::string& str)
{
    if(str == "none")  return DT_NONE;
    if(str == "strike") return DT_STRIKE;
    if(str == "shock") return DT_SHOCK;
    if(str == "energy") return DT_ENERGY;
    if(str == "electric") return DT_ELECTRIC;
    if(str == "flame") return DT_FLAME;
    if(str == "explosive") return DT_EXPLOSIVE;
    
    return DT_NONE;
}

std::string AIUtils::Dmg2Str(damage_type type, bool full)
{
    if(type == DT_NONE)  return full ? DirtyLinks::GetStrRes("dt_none") : DirtyLinks::GetStrRes("dt_n");
    if(type == DT_STRIKE) return full ? DirtyLinks::GetStrRes("dt_strike") : DirtyLinks::GetStrRes("dt_st");
    if(type == DT_SHOCK) return full ? DirtyLinks::GetStrRes("dt_shock") : DirtyLinks::GetStrRes("dt_sh");
    if(type == DT_ENERGY) return full ? DirtyLinks::GetStrRes("dt_energy") : DirtyLinks::GetStrRes("dt_en");
    if(type == DT_ELECTRIC) return full ? DirtyLinks::GetStrRes("dt_electric") : DirtyLinks::GetStrRes("dt_el");
    if(type == DT_FLAME) return full ? DirtyLinks::GetStrRes("dt_flame") : DirtyLinks::GetStrRes("dt_fl");
    if(type == DT_EXPLOSIVE) return full ? DirtyLinks::GetStrRes("dt_explosive") : DirtyLinks::GetStrRes("dt_ex");
    
    return DirtyLinks::GetStrRes("dt_unknown");
}

//=====================================================================================//
//                                static float dist2d()                                //
//=====================================================================================//
static float dist2d(const point3 &a, const point3 &b)
{
	const float dx = a.x - b.x;
	const float dy = a.y - b.y;
	return dx*dx + dy+dy;
}

//=====================================================================================//
//                      static ipnt2_t GetNearestHexForLanding()                       //
//=====================================================================================//
static ipnt2_t GetBestHexForLanding(const point3 &center)
{
	HexGrid *grid = HexGrid::GetInst();
	HexGrid::hg_slice slice = grid->GetSlice();
	pnt_vec_t targets;

	int mincost = 0x7FFFFFFF;
	for(HexGrid::cell_iterator i = grid->first_cell(); i != grid->last_cell(); ++i)
	{
		if(i->GetEntity()) continue;
		if(i->IsDefPnt()) continue;

		assert( i->GetCost() >= 0 );

		if(mincost >= i->GetCost())
		{
			if(mincost != i->GetCost())
			{
				targets.clear();
				mincost = i->GetCost();
			}

			targets.push_back(slice.off2pnt(i-grid->first_cell()));
		}
	}

	if(targets.empty())
	{
		float curdist = 1e+10f;
		for(HexGrid::cell_iterator i = grid->first_cell(); i != grid->last_cell(); ++i)
		{
			if(i->GetEntity()) continue;
			ipnt2_t pos2 = slice.off2pnt(i - grid->first_cell());
			point3 pos = HexUtils::hex2scr(pos2);
			const float dist = dist2d(pos,center);
			if(curdist >= dist)
			{
				if(dist != curdist)
				{
					targets.clear();
					curdist = dist;
				}

				targets.push_back(pos2);
			}
		}
	}

	assert( !targets.empty() );

	if(targets.size() == 1)
	{
		return targets[0];
	}

	size_t maxhex = 0;
	ipnt2_t result;
	for(pnt_vec_t::iterator i = targets.begin(); i != targets.end(); ++i)
	{
		pnt_vec_t hexes;
		PathUtils::CalcPassField(*i,20.0f,&hexes);
		if(hexes.size() >= maxhex)
		{
			result = *i;
			maxhex = hexes.size();
		}
	}

	return result;
}

//=====================================================================================//
//                               static bool NotLinked()                               //
//=====================================================================================//
static bool NotLinked(BaseEntity *e)
{
	HexGrid *grid = HexGrid::GetInst();
	for(HexGrid::cell_iterator i = grid->first_cell(); i != grid->last_cell(); ++i)
	{
		if(i->GetEntity() == e) return false;
	}

	return true;
}

//=====================================================================================//
//                         void AIUtils::ChangeEntityPlayer()                          //
//=====================================================================================//
void AIUtils::ChangeEntityPlayer(BaseEntity* entity, player_type new_player)
{
	player_type old_player = entity->GetPlayer();
	if(HumanEntity *human = entity->Cast2Human())
	{
		if(old_player == PT_PLAYER && new_player != PT_PLAYER)
		{
			if(VehicleEntity *car = human->GetEntityContext()->GetCrew())
			{
				assert( NotLinked(human) );
				PathUtils::CalcPassField(car->GetGraph()->GetPos2(),1e+10f,0);
				ipnt2_t pos = GetBestHexForLanding(car->GetGraph()->GetPos3());
			    human->GetGraph()->SetLoc(pos, human->GetGraph()->GetAngle());
				car->GetEntityContext()->DecCrew(human);
				PathUtils::LinkEntity(human);
				if(car->GetEntityContext()->IsSelected())
				{
					car->GetEntityContext()->Select(false);
				}
			}
		}
	}

	//уберем существо из старой команды
    GameObserver::kill_info kill_info(0, entity, GameObserver::KEV_TEAM_CHANGE);
    GameEvMessenger::GetInst()->Notify(GameObserver::EV_KILL, &kill_info);

    //удалим старый ai контекст
    entity->SetAIContext(0);

    if(entity->GetPlayer() == PT_PLAYER) DirtyLinks::Print(mlprintf(DirtyLinks::GetStrRes("aiu_leave").c_str(), entity->GetInfo()->GetName().c_str()));

    //сменим команду
    entity->DropFlags(entity->GetPlayer());
    entity->RaiseFlags(new_player);

    //кол-во ходов как на начало хода
    CalcMovepnts4Turn(entity);

    //сбросить режим паники в нормальный
    if(HumanEntity* human = entity->Cast2Human()) human->GetEntityContext()->SetPanicType(HPT_NONE);
    
    EntityContext* context = entity->GetEntityContext();

    VisMap::GetInst()->UpdateMarker(context->GetMarker(), UE_POS_CHANGE); 
    VisMap::GetInst()->UpdateSpectator(context->GetSpectator(), UE_FOS_CHANGE);
    
    //уведомим игрока о присоеденении к команде
    SpawnObserver::spawn_info_s info(entity);
	info.m_team_change = true;
    Spawner::GetInst()->Notify(SpawnObserver::ET_ENTITY_SPAWN, &info);

    //на всякий пожарный снимем выделение
    if(HumanEntity* human = entity->Cast2Human())
	{
        human->GetEntityContext()->Select(false);

        //занесем предметы на человеке в энциклопедию
        if(human->GetPlayer() == PT_PLAYER)
		{
            HumanContext::iterator itor = human->GetEntityContext()->begin(HPK_ALL);

            while(itor != human->GetEntityContext()->end())
			{
                ThingHandbook::GetInst()->Push(itor->GetInfo(), ThingHandbook::F_NEW_REC);
                ++itor;
            }

            //пошлем события для проигрывания звуков
            entity->Notify(EntityObserver::EV_TEAMMATE_COME);
            
            //приход в команду игрока озвучивается
            EntityPool::iterator ent = EntityPool::GetInst()->begin(ET_HUMAN, PT_PLAYER);
            while(ent != EntityPool::GetInst()->end())
			{
                if(entity->GetEID() != ent->GetEID()) ent->Notify(EntityObserver::EV_TEAMMATE_JOIN);                
                ++ent;
            }
        }
		else
		{
            //уберем слот быстрого доступа он теперь не нужен
            human->GetEntityContext()->SetFastAccessStrategy(0);
        }
    }

    //if(VehicleEntity* vehicle = entity->Cast2Vehicle()) vehicle->GetEntityContext()->Select(false);
    if(entity->GetPlayer() == PT_PLAYER) DirtyLinks::Print(mlprintf(DirtyLinks::GetStrRes("aiu_join").c_str(), entity->GetInfo()->GetName().c_str()));
}

//=====================================================================================//
//                           std::string AIUtils::Shot2Str()                           //
//=====================================================================================//
std::string AIUtils::Shot2Str(shot_type type)
{
    switch(type)
	{
    case SHT_AIMSHOT: return DirtyLinks::GetStrRes("sht_aimshot");
    case SHT_SNAPSHOT: return DirtyLinks::GetStrRes("sht_snapshot");
    case SHT_AUTOSHOT: return DirtyLinks::GetStrRes("sht_autoshot");
    }

    return DirtyLinks::GetStrRes("sht_unknown");
}

//=====================================================================================//
//                          std::string AIUtils::Reason2Str()                          //
//=====================================================================================//
std::string AIUtils::Reason2Str(reason_type reason)
{
    switch(reason){
    case RT_NO_AMMO: return DirtyLinks::GetStrRes("aiu_no_ammo");
    case RT_NO_WEAPON: return DirtyLinks::GetStrRes("aiu_no_weap");
    case RT_NO_MOVEPNTS: return DirtyLinks::GetStrRes("aiu_no_mps");
    case RT_OUT_OF_RANGE: return DirtyLinks::GetStrRes("aiu_out_of_rng");
    case RT_NOT_THROWABLE: return DirtyLinks::GetStrRes("aiu_not_throwable");
    case RT_IN_CREW: return DirtyLinks::GetStrRes("aiu_in_crew");
    }

    return DirtyLinks::GetStrRes("aiu_unknown");
}

//=====================================================================================//
//                         int AIUtils::CalcGrenadeAccuracy()                          //
//=====================================================================================//
int AIUtils::CalcGrenadeAccuracy(HumanEntity *human, float distance)
{
	return saturate((int)ceilf(PoL::Inv::GrenadeAccuracyFactor / CalcGrenadeRange(human,distance)),0,100);
}

//=====================================================================================//
//                          float AIUtils::CalcGrenadeRange()                          //
//=====================================================================================//
float AIUtils::CalcGrenadeRange(HumanEntity *human, float distance)
{
	const float dexterity = static_cast<float>(human->GetEntityContext()->GetTraits()->GetDexterity());
	const float radius = (PoL::Inv::GrenadeThrowFactor1
						  - PoL::Inv::GrenadeThrowFactor2*(dexterity - PoL::Inv::GrenadeThrowFactor3)
						  ) * distance / PoL::Inv::GrenadeThrowFactor4;
	return saturate(radius,0.0f,1e+10f);
}

//=====================================================================================//
//                             float AIUtils::CalcDamage()                             //
//=====================================================================================//
float AIUtils::CalcDamage(float distance, float base_damage, float fall_off)
{
	return base_damage - distance*fall_off/PoL::Inv::DamageFallOffFactor;
}

//=====================================================================================//
//                            int AIUtils::CalcEfficiency()                            //
//=====================================================================================//
int AIUtils::CalcEfficiency(BaseEntity *entity, const point3 &to)
{
	int efficiency = 0;
	const float dist = (entity->GetGraph()->GetPos3() - to).Length();
	float falloff = 0.0f;

	if(HumanEntity *h = entity->Cast2Human())
	{
		HumanContext *ctx = h->GetEntityContext();

		if(WeaponThing *weap = static_cast<WeaponThing*>(ctx->GetThingInHands(TT_WEAPON)))
		{
			if(AmmoThing *ammo = weap->GetAmmo())
			{
				falloff = ammo->GetInfo()->GetDamageFalloff();
				efficiency += ammo->GetInfo()->GetADmg().m_val;
				efficiency += ammo->GetInfo()->GetBDmg().m_val;
			}

			efficiency += weap->GetInfo()->GetADmg().m_val;
			efficiency += weap->GetInfo()->GetBDmg().m_val;
		}
		else if(GrenadeThing *gren = static_cast<GrenadeThing*>(ctx->GetThingInHands(TT_GRENADE)))
		{
			efficiency += gren->GetInfo()->GetADmg().m_val;
			efficiency += gren->GetInfo()->GetBDmg().m_val;
		}
	}
	else if(VehicleEntity *v = entity->Cast2Vehicle())
	{
		falloff = v->GetInfo()->GetFallOff();
		efficiency += v->GetInfo()->GetADmg().m_val;
		efficiency += v->GetInfo()->GetBDmg().m_val;
	}
	else
	{
		// error
	}

	return saturate(CalcDamage(dist,efficiency,falloff),0.0f,1e+10f);
}


//=====================================================================================//
//                          int AIUtils::CalcShootAccuracy()                           //
//=====================================================================================//
int AIUtils::CalcShootAccuracy(HumanEntity* human, WeaponThing* weapon, const point3& to_)
{
	point3 to = to_;
	const float min_scatter = PoL::Inv::AccuracyMinScatter; // 0.15f;
	const float max_scatter = PoL::Inv::AccuracyMaxScatter; // 0.25f;
	const float delta_scatter = max_scatter - min_scatter;
    float shoot_dist = 0;

	const float scatter = weapon->GetInfo()->GetMaxScatter() / 180.0f * PI;
	const float scatter_per_meter = tanf(scatter);
	const float distance = AIUtils::dist2D(human->GetGraph()->GetPos3(),to);
	if(scatter_per_meter * distance < min_scatter) return 100;
	if(scatter_per_meter * distance < max_scatter)
	{
		const point3 dir = to - human->GetGraph()->GetPos3();
		const float factor = dir.Length() / distance;
		to = to + ((max_scatter * factor / (dir.Length()*scatter_per_meter)) - 1.0f)*dir;
		assert( fabsf(AIUtils::dist2D(human->GetGraph()->GetPos3(),to) * scatter_per_meter - max_scatter) < 1e-2f );
	}

	GetShootRadius(weapon, human->GetGraph()->GetPos3(), to, &shoot_dist);
    //if(!GetShootRadius(weapon, human->GetGraph()->GetPos3(), to, &shoot_dist))
    //    return 0;

    HumanContext* context = human->GetEntityContext();
	const float unit_accuracy = static_cast<float>(context->GetTraits()->GetAccuracy());
	const float ammo_fall_off = static_cast<float>(weapon->GetAmmo()->GetInfo()->GetFalloff());
	const float dist = PoL::Inv::AccuracyDistanceFactor * shoot_dist;
	const float weapon_accuracy = static_cast<float>(weapon->GetInfo()->GetAccuracy());

	const float distp = powf(dist,PoL::Inv::AccuracyDistancePower);
	const float subtr =   PoL::Inv::AccuracyAmmoFactor * ammo_fall_off * distp
						* (PoL::Inv::AccuracyWeaponValue - weapon_accuracy/PoL::Inv::AccuracyWeaponFactor);

	float accuracy = PoL::Inv::AccuracyCommonFactor * (PoL::Inv::AccuracyUnitFactor * unit_accuracy - subtr);

    //если unit сидит точность повышается на 15%
	if(human->GetGraph()->IsSitPose()) accuracy *= 1.0f + PoL::Inv::AccuracySitFactor;

	switch(context->GetShotType())
	{
		case SHT_SNAPSHOT: accuracy /= PoL::Inv::SnapshotAccuracyFactor; break;
		case SHT_AUTOSHOT: accuracy /= PoL::Inv::AutoshotAccuracyFactor; break;
	}

	if(scatter_per_meter * distance < max_scatter)
	{
		const float interpolate_factor = 1.0f - (scatter_per_meter * distance - min_scatter) / delta_scatter;
		const float targacc = saturate(accuracy, 0.0f, 100.0f);
		return (100.0f - targacc) * interpolate_factor + targacc;
	}
   
    return saturate(accuracy, 0.0f, 100.0f); 
}

//=====================================================================================//
//                              bool AIUtils::CanShoot()                               //
//=====================================================================================//
bool AIUtils::CanShoot(HumanEntity* human, WeaponThing* weapon, const point3& to, reason_type* reason)
{
    if(weapon->GetAmmo() == 0)
	{
        if(reason) *reason = RT_NO_AMMO;
        return false;
    }

    float shoot_dist = 0;

	// gvozdoder: в pol нет out_of_range
	//if(!GetShootRadius(weapon, human->GetGraph()->GetPos3(), to, &shoot_dist))
	//{
	//	if(reason) *reason = RT_OUT_OF_RANGE;
	//	return false;
	//}

    HumanContext* context = human->GetEntityContext();
    
    if(context->GetTraits()->GetMovepnts() < weapon->GetMovepnts(context->GetShotType()) && !UnlimitedMoves())
	{
        if(reason) *reason = RT_NO_MOVEPNTS;
        return false;
    }

    if(reason) *reason = RT_NONE;
    return true;
}

//=====================================================================================//
//                              bool AIUtils::CanShoot()                               //
//=====================================================================================//
bool AIUtils::CanShoot(VehicleEntity* vehicle, const point3& to, reason_type* reason)
{
    if(vehicle->GetInfo()->GetAmmoCount() == 0)
	{
        if(reason) *reason = RT_NO_WEAPON;
        return false;
    }

    if(vehicle->GetEntityContext()->GetAmmoCount() == 0)
	{
        if(reason) *reason = RT_NO_AMMO;
        return false;
    }

    if(vehicle->GetEntityContext()->GetMovepnts() < vehicle->GetInfo()->GetMp4Shot() && !UnlimitedMoves())
	{
        if(reason) *reason = RT_NO_MOVEPNTS;
        return false;
    }

	if(AIUtils::dist2D(vehicle->GetGraph()->GetPos3(), to) > vehicle->GetInfo()->GetRange())
	{
		if(reason) *reason = RT_OUT_OF_RANGE;
		return false;
	}

    if(reason) *reason = RT_NONE;
    return true;
}

//=====================================================================================//
//                           float AIUtils::CalcThrowRange()                           //
//=====================================================================================//
float AIUtils::CalcThrowRange(HumanEntity* human)
{
	return static_cast<float>(human->GetEntityContext()->GetTraits()->GetStrength()) / PoL::Inv::GrenadeDistanceFactor;
}

//=====================================================================================//
//                              bool AIUtils::CanThrow()                               //
//=====================================================================================//
bool AIUtils::CanThrow(HumanEntity* human, BaseThing* thing, const point3& to, reason_type* reason)
{
    HumanContext::Traits* traits = human->GetEntityContext()->GetTraits();

    if(!(thing->GetInfo()->GetType() & TT_THROWABLE)){
        if(reason) *reason = RT_NOT_THROWABLE;
        return false;
    }

    if(traits->GetMovepnts() < CalcMps2Act(thing) && !UnlimitedMoves())
	{
        if(reason) *reason = RT_NO_MOVEPNTS;
        return false;
    }

	// By Flif
	/*
    if((human->GetGraph()->GetPos3() - to).Length() > CalcThrowRange(human)){
        if(reason) *reason = RT_OUT_OF_RANGE;
        return false;
    }
	*/
    if(AIUtils::dist2D(human->GetGraph()->GetPos3(), to) > CalcThrowRange(human))
	{
        if(reason) *reason = RT_OUT_OF_RANGE;
        return false;
    }
	// End By Flif

    return true;
}

//=====================================================================================//
//                             int AIUtils::CalcMps2Act()                              //
//=====================================================================================//
int AIUtils::CalcMps2Act(BaseThing* thing)
{
    switch(thing->GetInfo()->GetType()){
    case TT_CAMERA: return static_cast<CameraThing*>(thing)->GetInfo()->GetMp2Act();
    case TT_SHIELD: return static_cast<ShieldThing*>(thing)->GetInfo()->GetMp2Act();
    case TT_GRENADE: return static_cast<GrenadeThing*>(thing)->GetInfo()->GetMp2Act();
    case TT_IMPLANT: return static_cast<ImplantThing*>(thing)->GetInfo()->GetMp2Act();
    case TT_MEDIKIT: return static_cast<MedikitThing*>(thing)->GetInfo()->GetMp2Act();
    case TT_SCANNER: return static_cast<ScannerThing*>(thing)->GetInfo()->GetMp2Act();
    }

    return 0;
}

//=====================================================================================//
//                        std::string AIUtils::GetThingNumber()                        //
//=====================================================================================//
std::string AIUtils::GetThingNumber(BaseThing* thing)
{
    std::ostringstream ostr;
    
    switch(thing->GetInfo()->GetType()){                
    case TT_AMMO:
        ostr << static_cast<AmmoThing*>(thing)->GetBulletCount();
        break;
        
    case TT_MONEY:
        ostr << static_cast<MoneyThing*>(thing)->GetSum();
        break;
        
    case TT_MEDIKIT:
        ostr << static_cast<MedikitThing*>(thing)->GetCharge();
        break;
        
    case TT_WEAPON:
        {
            WeaponThing* weapon = static_cast<WeaponThing*>(thing);
            
            if(weapon->GetAmmo() && weapon->GetAmmo()->GetBulletCount())
                ostr << weapon->GetAmmo()->GetBulletCount();            
        }
        break;
    }

    return ostr.str();
}

//=====================================================================================//
//                         float AIUtils::NormalizeAccuracy()                          //
//=====================================================================================//
float AIUtils::NormalizeAccuracy(BaseEntity* entity, int accuracy)
{
    return static_cast<float>(accuracy) / 100.0f;
}

void AIUtils::CalcHumanAllyDeath(BaseEntity* entity)
{
    if(entity->Cast2Vehicle()) return;
    
    HumanEntity* human = entity->Cast2Human();
    HumanContext::Traits* traits = human->GetEntityContext()->GetTraits();

	traits->AddMorale( - PoL::Inv::FriendlyDeathMoraleFactor*(1.0f - human->GetInfo()->GetFibre()));
}

//=====================================================================================//
//                              bool AIUtils::CanReact()                               //
//=====================================================================================//
bool AIUtils::CanReact(BaseEntity* intruder, BaseEntity* guard)
{
	//являются ли существа врагами?
	if(!EnemyDetector::getInst()->isHeEnemy4Me(intruder, guard))
		return false;

	if(HumanEntity* human = guard->Cast2Human())
	{
		WeaponThing* weapon = static_cast<WeaponThing*>(human->GetEntityContext()->GetThingInHands(TT_WEAPON));

		if(weapon == 0 || !CanShoot(human, weapon, intruder->GetGraph()->GetShotPoint()))
			return false;
	}
	else if(VehicleEntity* vehicle = guard->Cast2Vehicle())
	{
		if(!CanShoot(vehicle,intruder->GetGraph()->GetShotPoint()))
			return false;
	}
	else
	{
		return false;
	}

	EntityReactionCalcer calcer;

	intruder->Accept(calcer);
	int intruder_reaction = calcer.GetResult();

	guard->Accept(calcer);
	int guard_reaction = calcer.GetResult();

	return aiNormRand() < (guard_reaction - intruder_reaction)/PoL::Inv::ReactPrbFactor;
}

//=====================================================================================//
//                          void AIUtils::GiveMoneyToPlayer()                          //
//=====================================================================================//
void AIUtils::GiveMoneyToPlayer(int money)
{
	//увеличить наш счет
	MoneyMgr::GetInst()->SetMoney(MoneyMgr::GetInst()->GetMoney() + money);                

	MoneyInfo *tinfo = (MoneyInfo *)ThingInfoArchive::GetInst()->CreateRef(TT_MONEY, "money");

	//проиграем звук зачисления денег на счет
	SndPlayer::GetInst()->Play(tinfo->GetUseSound());
}

void AIUtils::AddExp4Kill(int experience, BaseEntity* killer)
{
    if(experience == 0) return;

    if(killer->GetPlayer() == PT_PLAYER) DirtyLinks::Print( mlprintf( DirtyLinks::GetStrRes("team_got_exp").c_str(), experience));

    HumanEntity* human = killer->Cast2Human(); 
    if(human == 0) human = killer->Cast2Vehicle()->GetEntityContext()->GetDriver();

    int teammates = 0;

    //посчитаем кол-во народу в команде
    EntityPool::iterator itor = EntityPool::GetInst()->begin(ET_HUMAN, killer->GetPlayer());
    while(itor != EntityPool::GetInst()->end())
	{
        teammates++;
        ++itor;
    }

    //если нет никого - опыт давать некому
    if(teammates == 0) return;

    //если существо одно - ему отходит все
    if(teammates == 1)
	{
        human = EntityPool::GetInst()->begin(ET_HUMAN, killer->GetPlayer())->Cast2Human();
        human->GetEntityContext()->GetTraits()->AddExperience(experience);
        return;
    }

    //иначе 70% убийце остальное поровну
    if(human)
	{
        human->GetEntityContext()->GetTraits()->AddExperience(0.7f*experience);

        //учтем то, что дали
        teammates--;
        experience -= 0.7f*experience;
    }

    experience = experience/teammates;
    if(experience == 0) experience = 1;
    
    //добавить опыта людям всей команды
    itor = EntityPool::GetInst()->begin(ET_HUMAN, killer->GetPlayer());
    while(itor != EntityPool::GetInst()->end()){
        
        if(human == 0 || itor->GetEID() != human->GetEID())
            itor->Cast2Human()->GetEntityContext()->GetTraits()->AddExperience(experience);

        ++itor;
    }
}

//=====================================================================================//
//                            void AIUtils::AddExp4Quest()                             //
//=====================================================================================//
void AIUtils::AddExp4Quest(int experience, int player)
{
    if(experience == 0) return;

    int teammates = 0;
	int remainder = 0;
	int index = 0;

    //посчитаем кол-во народу в команде
	EntityPool *epool = EntityPool::GetInst();
    EntityPool::iterator itor;

    for(itor = epool->begin(ET_HUMAN,player); itor != epool->end(); ++itor)
	{
        ++teammates;
    }
    
	if(teammates)
	{
		remainder = experience % teammates;
		experience = experience / teammates;
	}
	else
	{
		experience = 1;
	}

    if(experience == 0) experience = 1;
    
    //добавить опыта людям всей комманды
    for(itor = epool->begin(ET_HUMAN,player); itor != epool->end(); ++itor, ++index)
	{
		itor->Cast2Human()->GetEntityContext()->GetTraits()->AddExperience(experience+(index<remainder?1:0));
    }
}

//=====================================================================================//
//                            void AIUtils::AddExp4Quest()                             //
//=====================================================================================//
void AIUtils::AddExp4Quest(int experience, BaseEntity* actor)
{
    if(experience == 0) return;

    int teammates = 0;
	int remainder = 0;
	int index = 0;

    //посчитаем кол-во народу в команде
	EntityPool *epool = EntityPool::GetInst();
    EntityPool::iterator itor;

    for(itor = epool->begin(ET_HUMAN,actor->GetPlayer()); itor != epool->end(); ++itor)
	{
        ++teammates;
    }
    
	if(teammates)
	{
		remainder = experience % teammates;
		experience = experience / teammates;
	}
	else
	{
		experience = 1;
	}

    if(experience == 0) experience = 1;
    
    //добавить опыта людям всей комманды
    for(itor = epool->begin(ET_HUMAN,actor->GetPlayer()); itor != epool->end(); ++itor, ++index)
	{
		itor->Cast2Human()->GetEntityContext()->GetTraits()->AddExperience(experience+(index<remainder?1:0));
    }
}

void AIUtils::AddExp4Hack(int experience, HumanEntity* human)
{
    if(experience == 0) return;

    human->GetEntityContext()->GetTraits()->AddExperience(experience);
    if(human->GetPlayer() == PT_PLAYER) DirtyLinks::Print( mlprintf( DirtyLinks::GetStrRes("aiu_got_exp").c_str(), human->GetInfo()->GetName().c_str(), experience));
}

//=====================================================================================//
//                         int AIUtils::CalcMps4ThingPickUp()                          //
//=====================================================================================//
int AIUtils::CalcMps4ThingPickUp(BaseThing* thing)
{
	int ret = ceilf(thing->GetWeight()*PoL::Inv::PickUpMpsPerKg);
    return ret ? ret : 1;
}

//=====================================================================================//
//                            bool AIUtils::CanChangePose()                            //
//=====================================================================================//
bool AIUtils::CanChangePose(HumanEntity* human, reason_type* reason)
{
	if(human->GetEntityContext()->GetTraits()->GetMovepnts() < PoL::Inv::MpsForPoseChange)
	{
        if(reason) *reason = RT_NO_MOVEPNTS;
        return false;
    }

	//if(human->GetEntityContext()->GetCrew())
	//{
	//	if(reason) *reason = RT_IN_CREW;
	//	return false;
	//}

    return true;
}

//=====================================================================================//
//                             void AIUtils::UseScanner()                              //
//=====================================================================================//
void AIUtils::UseScanner(HumanEntity *he, ScannerThing *scanner)
{
	if(he->GetEntityContext()->GetTraits()->GetMovepnts() >= scanner->GetInfo()->GetMp2Act())
	{
		scanner->SetInUse(true);
		he->Notify(EntityObserver::EV_FOS_CHANGE);
		scanner->SetInUse(false);
		he->Notify(EntityObserver::EV_FOS_CHANGE);

		//вычислим movepoints
		he->GetEntityContext()->GetTraits()->PoLAddMovepnts(- scanner->GetInfo()->GetMp2Act());
		GraphHuman *graph = he->GetGraph()->Cast2Human();
		point3 pos = graph->GetPos3();
		IWorld::Get()->GetEffects()->CreateHitEffect("hit_scanner", pos, pos, pos, 1.7f); 
		SndPlayer::GetInst()->Play(he->GetGraph()->GetPos3(),scanner->GetInfo()->GetUseSound());
	}
}

//=====================================================================================//
//                            void AIUtils::MakeTreatment()                            //
//=====================================================================================//
void AIUtils::MakeTreatment(HumanEntity* doctor, HumanEntity* sick, MedikitThing* medikit)
{
    int dose = medikit->GetInfo()->GetDose();

    //уменьшим содержимое аптечки
    if(dose > medikit->GetCharge())
	{
        dose = medikit->GetCharge();
        medikit->SetCharge(0);
    }
	else
	{
        medikit->SetCharge(medikit->GetCharge() - dose);
    }

    //вычислим movepoints
    doctor->GetEntityContext()->GetTraits()->PoLAddMovepnts(- medikit->GetInfo()->GetMp2Act());

    //лечим больного
    HumanContext* sick_cont = sick->GetEntityContext();

    int previous = sick_cont->GetTraits()->GetHealth();
    sick_cont->GetTraits()->AddHealth(dose);

    int delta = sick_cont->GetTraits()->GetHealth() - previous;

    DisplayHealthIncrease(sick, delta);
    ApplyHealthChange(sick_cont, delta);
    ApplyBodyPartsDamage(sick_cont, - delta);
}

//=====================================================================================//
//                              void AIUtils::MakeSwap()                               //
//=====================================================================================//
void AIUtils::MakeSwap(HumanEntity *source, BaseEntity *target)
{
    //вычислим movepoints
	source->GetEntityContext()->GetTraits()->PoLAddMovepnts(-PoL::Inv::MovepointsForSwap);

	const ipnt2_t spos = source->GetGraph()->GetPos2();
	const ipnt2_t tpos = target->GetGraph()->GetPos2();
	const float sangle = source->GetGraph()->GetAngle();
	const float tangle = target->GetGraph()->GetAngle();

	PathUtils::UnlinkEntity(source);
	PathUtils::UnlinkEntity(target);

	source->GetGraph()->SetLoc(tpos,sangle);
	target->GetGraph()->SetLoc(spos,tangle);

	PathUtils::LinkEntity(source);
	PathUtils::LinkEntity(target);
}


//=====================================================================================//
//                         int AIUtils::CalcMps4EntityRotate()                         //
//=====================================================================================//
int AIUtils::CalcMps4EntityRotate(float delta)
{
	return (int)ceilf(delta/PoL::Inv::TurnAngle4OneMp);
}

//=====================================================================================//
//                         int AIUtils::CalcMps4EntityRotate()                         //
//=====================================================================================//
int AIUtils::CalcMps4EntityRotate(BaseEntity* entity, float dir)
{
    float delta = fabsf(entity->GetGraph()->GetAngle() - dir);
    
    delta = fmodf(delta, PIm2);
    if(delta > PI) delta = PIm2 - delta;
    
	return CalcMps4EntityRotate(delta);

 //   while(delta > 0.0f)
	//{
 //       cost += MPS_FOR_ROTATE;
	//	delta -= PId2;
 //       //delta -= PId2;
	//	//Grom
 //       //delta -= delta;
 //   }

    //return cost;
}

//=====================================================================================//
//                         int AIUtils::CalcMps4EntityRotate()                         //
//=====================================================================================//
int AIUtils::CalcMps4EntityRotate(BaseEntity* entity, const point3& dir)
{
	return CalcMps4EntityRotate(entity,Dir2Angle(dir));
}

void AIUtils::SaveLoadRIDSet(SavSlot& slot, rid_set_t& rid_set)
{
    if(slot.IsSaving()){
        
        slot << rid_set.size();
        
        rid_set_t::const_iterator itor = rid_set.begin();
        while(itor != rid_set.end()){
            slot << *itor;
            ++itor;
        }
        
    }else{
        
        rid_set.clear();
        
        rid_t  rid;
        size_t size;
        
        slot >> size;
        while(size --){
            slot >> rid;
            rid_set.insert(rid);
        }
    }
}

int AIUtils::CalcLevelupPoints(HumanEntity* human, int cur_lvl, int old_lvl)
{ 
    int ret  = 0,
        base = human->GetInfo()->GetLevelupBase(),
        rise = human->GetInfo()->GetLevelupRise();

    for(int lvl = (old_lvl + 1); lvl <= cur_lvl; lvl++)
        ret += rise;
        
    return ret;
}

//=====================================================================================//
//                            void AIUtils::GetEntityFOS()                             //
//=====================================================================================//
void AIUtils::GetEntityFOS(BaseEntity* entity, fos_s* fos)
{
    if(HumanEntity* human = entity->Cast2Human())
	{
        HumanContext* context = human->GetEntityContext();
        
        fos->setAngle(context->GetTraits()->GetSightAngle());
		ScannerThing* scanner = static_cast<ScannerThing*>(context->GetThingInHands(TT_SCANNER));

        if(scanner && scanner->IsInUse())
		{
            fos->setBack(scanner->GetInfo()->GetRadius());
            fos->setFront(scanner->GetInfo()->GetRadius());
        }
		else
		{
            fos->setFront(context->GetTraits()->GetFrontRadius());
            fos->setBack(context->GetTraits()->GetBackRadius());
    
            if(fos->getFront() < fos->getBack()) fos->setFront(fos->getBack());
        }
    }
	else if(VehicleEntity* vehicle = entity->Cast2Vehicle())
	{
        fos->setAngle(vehicle->GetInfo()->GetSightAngle());
        fos->setFront(vehicle->GetInfo()->GetSightRadius());
        fos->setBack(vehicle_back_fos_radius);
    }
	else
	{
        fos->setAngle(0.0f);
        fos->setFront(0.0f);
        fos->setBack(0.0f);
    }
}

// возвращает двумерное (по Х и У) расстояние между трехмерными точками
float AIUtils::dist2D(const point3& p1, const point3& p2)
{
	point3 pp1 = p1;
	pp1.z = 0;
	point3 pp2 = p2;
	pp2.z = 0;
	return (pp2 - pp1).Length();
}

void AIUtils::CalcAndShowActiveExits()
{
    unsigned active_exits = 0;
    
    //соберем номера зон из кот можно выйти
    for(int k = 0; k < MAX_JOINTS; k++){
        
        unsigned joint = 1 << k;
        
        if(Spawner::GetInst() && Spawner::GetInst()->CanExit(joint, 0))
            active_exits |= joint;
    }

    pnt_vec_t exits;

    //если с уровня нет выходов
    if(active_exits == 0){
        GraphGrid::GetInst()->SetHexes(GraphGrid::HT_JOINTS, exits);
        return;
    }
    
    HexGrid::const_cell_iterator first = HexGrid::GetInst()->first_cell();
    HexGrid::const_cell_iterator last  = HexGrid::GetInst()->last_cell(); 
    HexGrid::const_prop_iterator hex_prop;
    
    HexGrid::hg_slice hs = HexGrid::GetInst()->GetSlice();    
        
    //пройти по всему полю и найти зоны выхода
    while(first != last){
        hex_prop = HexGrid::GetInst()->first_prop() + first->GetProperty();
        
        if(hex_prop->m_joints & active_exits)
            exits.push_back(hs.off2pnt(first - HexGrid::GetInst()->first_cell()));
        
        ++first;
    }
    
    GraphGrid::GetInst()->SetHexes(GraphGrid::HT_JOINTS, exits);
}

float AIUtils::CalcHackProbability(HumanEntity* actor, VehicleEntity* vehicle)
{
    int value = ((actor->GetEntityContext()->GetTraits()->GetWisdom() - vehicle->GetInfo()->GetWisdom()) * 2.0f + 30);
    return value > 100 ? 1.0f : static_cast<float>(value)/100.0f;
}

void AIUtils::CalcMovepnts4Turn(BaseEntity* entity)
{
     entity->Accept(EntityTurnCalcer());   
}

//=========================================================================

//=====================================================================================//
//                            MoneyMgr* MoneyMgr::GetInst()                            //
//=====================================================================================//
MoneyMgr* MoneyMgr::GetInst()
{
    static MoneyMgr imp;
    return &imp;
}

//=====================================================================================//
//                           int MoneyMgr::GetMoney() const                            //
//=====================================================================================//
int MoneyMgr::GetMoney() const
{
	if(PoL::Inv::Money)
	{
		return m_count;
	}
	else
	{
		return 1<<30;
	}
}

//=====================================================================================//
//                              void MoneyMgr::SetMoney()                              //
//=====================================================================================//
void MoneyMgr::SetMoney(int count)
{
	if(PoL::Inv::Money)
	{
		m_count = count;
	}
}

//=====================================================================================//
//                            void MoneyMgr::MakeSaveLoad()                            //
//=====================================================================================//
void MoneyMgr::MakeSaveLoad(SavSlot& slot)
{
    if(slot.IsSaving())
	{
        slot << m_count;
	}
    else
	{
        slot >> m_count;
	}
}

//=====================================================================================//
//                                MoneyMgr::MoneyMgr()                                 //
//=====================================================================================//
MoneyMgr::MoneyMgr()
:	m_count(PoL::Inv::StartMoney)
{
}

//=====================================================================================//
//                               void MoneyMgr::Reset()                                //
//=====================================================================================//
void MoneyMgr::Reset()
{
	m_count = PoL::Inv::StartMoney;
}

//=========================================================================

//=====================================================================================//
//                         PlayerSpectator::PlayerSpectator()                          //
//=====================================================================================//
PlayerSpectator::PlayerSpectator()
:	m_spectator(new PlayerTeamSpectator)
{
}

//=====================================================================================//
//                         PlayerSpectator::~PlayerSpectator()                         //
//=====================================================================================//
PlayerSpectator::~PlayerSpectator()
{
}

//=====================================================================================//
//                            void PlayerSpectator::Reset()                            //
//=====================================================================================//
void PlayerSpectator::Reset()
{
	m_spectator.reset(new PlayerTeamSpectator);
}

//=====================================================================================//
//                        void PlayerSpectator::MakeSaveLoad()                         //
//=====================================================================================//
void PlayerSpectator::MakeSaveLoad(SavSlot& slot)
{
    GetSpectator()->MakeSaveLoad(slot);
}

//=====================================================================================//
//                     PlayerSpectator* PlayerSpectator::GetInst()                     //
//=====================================================================================//
PlayerSpectator* PlayerSpectator::GetInst()
{
    static PlayerSpectator imp;
    return &imp;
}

//=========================================================================
namespace AIUtilsDetail {

    //
    // видимо ли существо команде игрока    
    //
    bool IsVisible4Player(BaseEntity* entity)
    {       
        return entity->GetGraph()->IsVisible();
    }

    //
    // получить точку фокусировки камеры
    //
    point3 GetFocusPoint(BaseEntity* entity)
    {
        point3 pos = entity->GetGraph()->GetPos3();            
        pos.z += 1.0f;

        return  pos;
    }

    //
    // наводчик на существо
    //
    class EntityFocus : public EntityVisitor{
    public:

        EntityFocus(float time = 0) : m_time(time) {}

        void Visit(HumanEntity* human)
        {
            if(human->GetEntityContext()->GetCrew())
                CenterCamera(human->GetEntityContext()->GetCrew());
            else
                CenterCamera(human);
        }
        
        void Visit(VehicleEntity* vehicle)
        {  CenterCamera(vehicle);  }
        
        void Visit(TraderEntity* trader)
        {  CenterCamera(trader);  }

    private:

        void CenterCamera(BaseEntity* entity)
        { CameraDevice::GetInst()->Move(GetFocusPoint(entity), m_time); }

    private:

        float m_time;
    };

    //
    // декоратор для показа камерой скрытых действий
    //
    class CameraworkDecorator : public Camerawork
	{
    public:

        CameraworkDecorator(Camerawork* work = 0): m_work(work) {}
        ~CameraworkDecorator(){ delete m_work; }

        bool Film(bool factivity_running)
        {
			return m_work ? m_work->Film(factivity_running) : factivity_running;
		}

		void Prepare()
		{
			if(m_work) m_work->Prepare();
		}

    private:

        Camerawork* m_work;
    };

    //
    // декоратор для показа повреждений сущств игрока
    //
    class DamageTake : public  CameraworkDecorator,
                             private GameObserver
    {
		bool m_prepared;

    public:
        DamageTake(Camerawork* work = 0)
		:	CameraworkDecorator(work),
			m_end_time(0),
			m_point(NULLVEC),
			m_prepared(false)
        {

            GameEvMessenger::GetInst()->Attach(this, EV_KILL);
            GameEvMessenger::GetInst()->Attach(this, EV_HURT);
        }

        ~DamageTake()
        {
            CameraDevice::GetInst()->DoFilm(false);
            GameEvMessenger::GetInst()->Detach(this);
        }

		void Prepare()
		{
			CameraworkDecorator::Prepare();
			DoPrepare();
		}

		void DoPrepare()
		{
            //сфокусируем камеру и выключим HM
			if(m_point != NULLVEC)
			{
                DirtyLinks::MoveCamera(m_point, 0.2f);
                CameraDevice::GetInst()->DoFilm(true);
			}
			m_prepared = true;
		}

        bool Film(bool factivity_running)
        {
			if(!m_prepared)
			{
				DoPrepare();
			}

            //выдержим паузу           
            if(m_end_time > Timer::GetSeconds())
                return true;

            //включим экран HM
            if(m_end_time)
			{
                m_end_time = 0;
                CameraDevice::GetInst()->DoFilm(false);
            }

            return CameraworkDecorator::Film(factivity_running);
        }

    private:

        void Update(subject_t subj, event_t event, info_t info)
        {   
            point3 focus;

            switch(event)
			{
            case EV_KILL:
                if(GetFocusPointEx(static_cast<kill_info*>(info)->m_victim, &focus)){
                    m_point    = focus;
                    m_end_time = Timer::GetSeconds() + 0.3f; 
                }
                break;

            case EV_HURT:
                {
                    BaseEntity* entity = static_cast<hurt_info*>(info)->m_victim;

                    if(     GetFocusPointEx(entity, &focus)
                        &&  CameraDevice::GetInst()->IsInCone(focus)){
                        m_point    = focus;
                        m_end_time = Timer::GetSeconds() + 0.2f; 
                    }
                }
                break;
            }
        }

        bool GetFocusPointEx(BaseEntity* entity, point3* pos) const
        {
            if(IsVisible4Player(entity)){    
                *pos = GetFocusPoint(entity);
                return true;
            }

            return false;
        }

    private:

        point3 m_point;
        float  m_end_time;
   };

    //
    // стратегия для показа существа
    //
    class EntityTake : public CameraworkDecorator,
                       private EntityObserver
    {
    public:
        
        EntityTake(BaseEntity* entity, Camerawork* work = 0) :
            m_update_delta(0.2f), m_update_time(0),
            CameraworkDecorator(work), m_entity(entity)            
        {
				m_entity->Attach(this, EV_PREPARE_DEATH);
				m_entity->Attach(this, EV_DESTROY);
		}

        ~EntityTake() 
        { 
            DetachObservers();
            CameraDevice::GetInst()->DoFilm(false, m_entity);
        }

        bool Film(bool factivity_running)
        {
            //действие не окончилось и оно живо 
            if(factivity_running && m_entity)
			{                
                //если существо видимо сфокусируем камеру
                if(IsVisible4Player(m_entity))
				{
                    if(m_update_time < Timer::GetSeconds())
					{
                        m_update_time = Timer::GetSeconds() + m_update_delta;
                        
                        //существо в фокус
                        m_entity->Accept(EntityFocus(0.2f));
                        
                        //установить видимость экрана скрытых перемещ
                        CameraDevice::GetInst()->DoFilm(true, m_entity);
                    }
                    
                    return true;
                }
                
                //не надо пауз и включений экрана
                m_update_time = 0;
            }
            
            //держим паузу
            if(m_update_time > Timer::GetSeconds())
                return true;
            
            if(m_update_time){
                m_update_time = 0;
                CameraDevice::GetInst()->DoFilm(false, m_entity);
            }
            
            return CameraworkDecorator::Film(factivity_running);
        }
          
    private:
        
        void DetachObservers()
        {
            if(m_entity){
                m_entity->Detach(this);
                m_entity = 0;
            }
        }
        
        void Update(BaseEntity* entity, event_t event, info_t info)
        {
            bool fvisible = IsVisible4Player(m_entity);
            
            m_update_time = 0; 
            
            //покажем трупик если он виден
            if(fvisible){
                m_entity->Accept(EntityFocus(0.2f));
                m_update_time = Timer::GetSeconds() + 3.0f;
            }
            
            DetachObservers();

            CameraDevice::GetInst()->DoFilm(fvisible, m_entity);
        }
        
    private:
        
        float m_update_time;
        float m_update_delta;
        
        BaseEntity* m_entity;
    };

    //
    // реализация умной камеры
    //
    class CameramanImp : public Cameraman{
    public:

        CameramanImp() : m_flags(0) {}
        ~CameramanImp(){ }
               
        //сконфигурировать оператора
        void Configure(unsigned flags)
        { m_flags = flags; }
        
        //сфокусировать камеру на существе
        void FocusEntity(BaseEntity* entity, float time = 0.2f)
        { 
				entity->Accept(EntityFocus(time));
		}
        
        //сфокусировать камеру на ходьбе
        Camerawork* FilmMove(BaseEntity* entity)
        {
            Camerawork* work = 0;            
            
//			if(entity->GetPlayer() == PT_PLAYER) return work;//Grom - подавление дребезжания камеры при реакции

            if((m_flags & F_SHOW_MOVE || IsPlayerEntityInPanic(entity)) && CanShowMove(entity))
			{
                work = new EntityTake(entity, work);
			}

            if(m_flags & F_SHOW_DAMAGES) work = new DamageTake(work);

            return work;
        }

        //сфокусировать на повороте
        Camerawork* FilmRotate(BaseEntity* entity)
        {
            Camerawork* work = 0;            

//			if(entity->GetPlayer() == PT_PLAYER) return work;//Grom - подавление дребезжания камеры при реакции

            if((m_flags & F_SHOW_ROTATE || IsPlayerEntityInPanic(entity)) && CanShowMove(entity))
                work = new EntityTake(entity, work);

            if(m_flags & F_SHOW_DAMAGES) work = new DamageTake(work);

            return work;
        }

        //сфокусировать на стрельбе
        Camerawork* FilmShoot(BaseEntity* killer, BaseEntity* victim)
        {
            Camerawork* work  = 0;            
            BaseEntity* actor = SelectActor(killer, victim);

			if(killer && IsPlayerEntityInPanic(killer))
			{
				work = new EntityTake(killer);
			}
			else if(actor && m_flags & F_SHOW_SHOOT)
			{
                if(actor->GetEID() == killer->GetEID())
				{
                    if(CanShowShoot(killer)) work = new EntityTake(actor, work);
                }
				else
				{
                    work = new EntityTake(actor, work);
                }
            }
						//Grom:
            //вообще-то это неправильно, но только так пока можно избавиться от рывков камеры
            //if(m_flags & F_SHOW_DAMAGES) work = new DamageTake(work);
            
            return work;
        }

        //сфокусировать на броске
        Camerawork* FilmThrow(HumanEntity* killer, BaseEntity* victim)
        {
            Camerawork* work  = 0;            
            BaseEntity* actor = SelectActor(killer, victim);
            
            if(actor && (m_flags & F_SHOW_THROW || IsPlayerEntityInPanic(killer)))
			{
                if(actor->GetEID() == killer->GetEID())
				{
					if(CanShowShoot(killer)) work = new EntityTake(actor, work);
                }
				else
				{
                    work = new EntityTake(actor, work);
                }
            }

            if(m_flags & F_SHOW_DAMAGES) work = new DamageTake(work);
            
            return work;
        }

        Camerawork* FilmLanding(HumanEntity* hum) { return 0; }
        Camerawork* FilmUse(BaseEntity* ent, const rid_t& rid) { return 0; }
        Camerawork* FilmShipment(HumanEntity* hum, VehicleEntity* veh) { return 0; }

    private:
       
        BaseEntity* SelectActor(BaseEntity* active, BaseEntity* passive)
        {   
            if(passive && passive->GetGraph()->IsVisible()
							//Grom
							//&&	passive->GetGraph()->
							)

                return passive;
            
            if(active && active->GetGraph()->IsVisible() && active->GetPlayer()!=PT_PLAYER)
                return active;

            return 0;
        }

        bool CanShowMove(BaseEntity* entity) const
        {
			if(IsPlayerEntityInPanic(entity)) return true;

			RelationType relation = EnemyDetector::getInst()->getRelationBetweenPlayerAndHe(entity);

			if(entity->GetPlayer() != PT_CIVILIAN)
			{
                return      (relation == RT_ENEMY && m_flags & F_SHOW_ENEMY_RELATED_MOVE)
                        ||  (relation == RT_FRIEND && m_flags & F_SHOW_FRIEND_RELATED_MOVE) 
                        ||  (relation == RT_NEUTRAL && m_flags & F_SHOW_NEUTRAL_RELATED_MOVE);
            }

            return      (m_flags & F_SHOW_CIVILIAN_VEHICLES_MOVE && entity->Cast2Vehicle())
                    ||  (m_flags & F_SHOW_CIVILIAN_HUMANS_TRADERS_MOVE && !entity->Cast2Vehicle());
        }

		bool IsPlayerEntityInPanic(BaseEntity *entity) const
		{
			if(entity->GetPlayer() == PT_PLAYER)
			{
				if(HumanEntity *hum = entity->Cast2Human())
				{
					if(hum->GetEntityContext()->GetPanicType() != HPT_NONE)
					{
						return true;
					}
				}
			}
			return false;
		}

        bool CanShowShoot(BaseEntity* killer) const
        {
			if(IsPlayerEntityInPanic(killer)) return true;
            RelationType relation = EnemyDetector::getInst()->getRelationBetweenPlayerAndHe(killer);

            if(killer->GetPlayer() != PT_CIVILIAN)
			{
                return      (relation == RT_ENEMY && m_flags & F_SHOW_ENEMY_RELATED_SHOOT)
                        ||  (relation == RT_FRIEND && m_flags & F_SHOW_FRIEND_RELATED_SHOOT) 
                        ||  (relation == RT_NEUTRAL && m_flags & F_SHOW_NEUTRAL_RELATED_SHOOT);
            }

            return      (m_flags & F_SHOW_CIVILIAN_VEHICLES_SHOOT && killer->Cast2Vehicle())
                    ||  (m_flags & F_SHOW_CIVILIAN_HUMANS_TRADERS_SHOOT && !killer->Cast2Vehicle());
        }

    private:
        
        unsigned  m_flags;
    };
}

Cameraman* Cameraman::GetInst()
{
    static CameramanImp imp;
    return &imp;
}

//=========================================================================

namespace AIUtilsDetail
{
    //
    // наблюдатель для камеры
    //
    class CameraSpectator : public Spectator
	{
    public:
        CameraSpectator(player_type pl = PT_NONE,const CameraThing* cam = 0, const point3 pos = NULLVEC) :
            m_pos(pos), m_player(pl), m_effect(0)
        {
            if(cam)
			{

                m_radius = cam->GetInfo()->GetRadius();
                m_turns  = cam->GetInfo()->GetCharge();
                m_effect_name = cam->GetInfo()->GetVisualEffect();

                m_effect = EffectMgr::GetInst()->MakeConst(m_effect_name, m_pos);
            }
        }

        ~CameraSpectator()
		{
			EffectMgr::GetInst()->DestroyConst(m_effect);
		}

		std::string DescribeSelf() const
		{
			return "CameraSpectator";
		}
        
        DCTOR_DEF(CameraSpectator)

        //сохранить / загрузить наблюдателя
        void MakeSaveLoad(SavSlot& slot)
        {
            if(slot.IsSaving())
			{
                slot << m_pos;
                slot << m_turns;
                slot << m_radius;
                slot << m_effect_name;
                
                slot << static_cast<int>(m_player);
            }
			else
			{
                slot >> m_pos;

                slot >> m_turns;
                slot >> m_radius;
                slot >> m_effect_name; 
                
                m_effect = EffectMgr::GetInst()->MakeConst(m_effect_name, m_pos);

                int tmp; slot >> tmp;
                m_player = static_cast<player_type>(tmp);
            }
        }
        
        //расчитать видимость для маркера
        VisParams CalcVisibility(Marker* marker)
        {
            BaseEntity* entity = marker->GetEntity();

            if(entity && entity->GetPlayer() == m_player)
			{
                return VisParams::vpInvisible;
			}

            pnt3_vec_t vec;
            marker->GetPoints(vec);

            LOSTracer tracer(mdTransparent);
            tracer.SetFirst(0, m_pos);

            eid_t eid = entity ? entity->GetEID() : 0;

            for(size_t k = 0; k < vec.size(); k++)
			{
                if(IsInFOS(vec[k]))
				{
                    tracer.SetSecond(eid, vec[k]);
                    if(tracer.CalcLOS())
					{
						return VisParams::vpVisible;
					}
                }
            }

            return VisParams::vpInvisible;
        }

        //расчитать видимость для точки
        VisParams CalcVisibility(const point3& pnt)
        { 
            return VisParams::vpInvisible;
        }

        //создать связь между маркером и наблюдателем
		std::auto_ptr<Relation> CreateRelationFor(Marker* marker)
        {
#ifdef _HOME_VERSION
			if(CmdLine::IsKey("-cc"))
			{
				return std::auto_ptr<Relation>(new CheatCameraMSRelation(this,marker));
			}
			else
			{
#endif
				return std::auto_ptr<Relation>(new CameraMSRelation(this,marker));
#ifdef _HOME_VERSION
			}
#endif
        }

        //узнать тип игрока
        player_type GetPlayer() const { return m_player; }

        //посчтиать кол-во ходов для работы камеры
        void DecTurns()       { m_turns --; }
        //камера все еще активна?
        bool IsActive() const { return m_turns > 0; }

    private:

        bool IsInFOS(const point3& tst) const
        {
			const float dx = m_pos.x - tst.x, dy = m_pos.y - tst.y;
			return dx*dx + dy*dy < m_radius*m_radius;
        }

    private:

        class CheatCameraMSRelation : public Relation
		{
        public:
			CheatCameraMSRelation(Spectator *s, Marker *m) : Relation(m,s) {}
        };

        class CameraMSRelation : public Relation
		{
        public:
			CameraMSRelation(Spectator *s, Marker *m) : Relation(m,s) {}
            
            VisParams DoUpdateRelation(VisUpdateEvent_t ev) const
            {
				switch(ev)
				{
				case UE_POS_CHANGE:
				case UE_FOS_CHANGE:                    
				case UE_INS_MARKER:
				case UE_INS_SPECTATOR:
					return GetSpectator()->CalcVisibility(GetMarker());
                	break;
                }
				return VisParams::vpInvisible;
            }
        };

        point3   m_pos;
        int      m_turns;
        float    m_radius;

        unsigned    m_effect;
        std::string m_effect_name;

        player_type m_player;
    };

    DCTOR_IMP(CameraSpectator)
}

CameraPool* CameraPool::GetInst()
{
    static CameraPool imp;
    return &imp;
}

//=====================================================================================//
//                              CameraPool::~CameraPool()                              //
//=====================================================================================//
CameraPool::~CameraPool()
{
    Reset();
}

//=====================================================================================//
//                              void CameraPool::Reset()                               //
//=====================================================================================//
void CameraPool::Reset()
{
    for(size_t k = 0; k < m_cams.size(); delete m_cams[k++]);
    m_cams.clear();
}

//=====================================================================================//
//                              void CameraPool::Insert()                              //
//=====================================================================================//
void CameraPool::Insert(player_type player, const CameraThing* camera, const point3& pos)
{
    for(size_t k = 0; k < m_cams.size(); k++)
	{
        if(m_cams[k]) continue;

        m_cams[k] = new CameraSpectator(player, camera, pos);
        VisMap::GetInst()->Insert(m_cams[k]);
        return;        
    }

    m_cams.push_back(0);

    Insert(player, camera, pos);
}

//=====================================================================================//
//                           void CameraPool::MakeSaveLoad()                           //
//=====================================================================================//
void CameraPool::MakeSaveLoad(SavSlot& slot)
{
    if(slot.IsSaving())
	{
        //сохраняем список камер
        for(size_t k = 0; k < m_cams.size(); k++)
		{
            if(m_cams[k])
			{
                slot << true;
                DynUtils::SaveObj(slot, m_cams[k]);
                m_cams[k]->MakeSaveLoad(slot);
            }
        }

        //признак конца
        slot << false;
    }
	else
	{
        bool flag;
        slot >> flag;

        while(flag)
		{
            m_cams.push_back(0);
            DynUtils::LoadObj(slot, m_cams.back());
            m_cams.back()->MakeSaveLoad(slot);
            VisMap::GetInst()->Insert(m_cams.back());

            slot >> flag;
        }
    }    
}

//=====================================================================================//
//                          void CameraPool::HandleNextTurn()                          //
//=====================================================================================//
void CameraPool::HandleNextTurn()
{
    for(size_t k = 0; k < m_cams.size(); k++)
	{
        if(m_cams[k] == 0) continue;
       
        static_cast<CameraSpectator*>(m_cams[k])->DecTurns();
        if(!static_cast<CameraSpectator*>(m_cams[k])->IsActive())
		{
            VisMap::GetInst()->Remove(m_cams[k]);
            delete m_cams[k];

            m_cams[k] = 0;
        }
    }
}

//=========================================================================

ShieldPool::ShieldTraits::ShieldTraits(const std::string& effect, const point3& pos, float radius, int turns) :
    m_effect(EffectMgr::GetInst()->MakeConst(effect, pos)),
    m_pos(pos), m_turns(turns), m_radius(radius),m_effect_name(effect)
{
}

ShieldPool::ShieldTraits::~ShieldTraits()
{
    EffectMgr::GetInst()->DestroyConst(m_effect);
}

ShieldPool* ShieldPool::GetInst()
{
    static ShieldPool imp;
    return &imp;
}

ShieldPool::ShieldPool()
{
}

ShieldPool::~ShieldPool()
{
    Reset();
}

void ShieldPool::Reset()
{
    for(size_t k = 0; k < m_infos.size(); delete m_infos[k++]);
    m_infos.clear();
}

void ShieldPool::MakeSaveLoad(SavSlot& slot)
{
    if(slot.IsSaving()){
        
        for(size_t k = 0; k < m_infos.size(); k++){
            if(m_infos[k]){
                slot << true;
                slot << m_infos[k]->GetPos();
                slot << m_infos[k]->GetTurns();
                slot << m_infos[k]->GetRadius();
                slot << m_infos[k]->GetEffectName();
            }
        }

        slot << false;

    }else{

        point3 pos;
        int    turns;
        float  radius;

        std::string eff;

        bool fnext;
        slot >> fnext;
        
        while(fnext){
            
            slot >> pos;
            slot >> turns;
            slot >> radius;
            slot >> eff;

            slot >> fnext;

            m_infos.push_back(new ShieldTraits(eff, pos, radius, turns));
        }
    }
}

void ShieldPool::Insert(const ShieldThing* shield, const point3& pos)
{
    for(size_t k = 0; k < m_infos.size(); k++){

        if(m_infos[k]) continue;

        m_infos[k] = new ShieldTraits(shield->GetInfo()->GetVisualEffect(), pos,
                            shield->GetInfo()->GetRange(), shield->GetInfo()->GetCharge());
        return;        
    }

    m_infos.push_back(0);
    Insert(shield, pos);
}

void ShieldPool::HandleNextTurn()
{
    for(size_t k = 0; k < m_infos.size(); k++){

        if(m_infos[k] == 0) continue;

        m_infos[k]->DecTurns();
        
        if(m_infos[k]->GetTurns() <= 0){
            delete m_infos[k];
            m_infos[k] = 0;
        }        
    }
}

ShieldPool::iterator ShieldPool::end()
{
    return iterator(0, m_infos.size());
}

ShieldPool::iterator ShieldPool::begin()
{ 
    return iterator(&m_infos, 0);
}

ShieldPool::Iterator::Iterator(info_vec_t* vec, size_t first) :
    m_infos(vec), m_first(first)
{ 
    if(m_infos && (m_infos->empty() || (*m_infos)[first] == 0))
        operator++();
}

ShieldPool::Iterator& ShieldPool::Iterator::operator ++()
{
    for(m_first++; m_first < m_infos->size(); m_first++)
        if((*m_infos)[m_first]) return *this;
        
    m_first = m_infos->size();
    return *this;
}

//=========================================================================

namespace AIUtilsDetail
{

//=====================================================================================//
//                              class ThingDeliveryHelper                              //
//=====================================================================================//
class ThingDeliveryHelper
{
public:
	typedef ThingDelivery::scheme_type scheme_type;

    //это слот для активации?
    static bool NeedActivate(BaseThing* thing, human_pack_type hpk)
    {
        const ThingInfo* info = thing->GetInfo();

        switch(hpk){
        case HPK_HEAD:
            return info->IsArmor() && static_cast<const ArmorInfo*>(info)->IsHelmet();

        case HPK_BODY:
            return info->IsArmor() && !static_cast<const ArmorInfo*>(info)->IsHelmet();

        case HPK_HANDS:
            return      info->IsWeapon()
                    ||  info->IsCamera()
                    ||  info->IsGrenade()
                    ||  info->IsMedikit()
                    ||  info->IsScanner()
                    ||  info->IsShield();

        case HPK_IMPLANTS:
            return info->IsImplant();
        }

        return false;
    }

    //вычислить кол-во movepoints для активации при доставке
    static int CalcMovepnts(scheme_type scheme, BaseThing* thing, human_pack_type hpk, bool betwHumans)
    {
        int movepnts = 0;

        const ThingInfo* info = thing->GetInfo();

        if(hpk == HPK_BODY && info->IsArmor()) movepnts = static_cast<const ArmorInfo*>(info)->GetMp2Act();
        if(hpk == HPK_HANDS && info->IsScanner()) movepnts = static_cast<const ScannerInfo*>(info)->GetMp2Act();
        if(hpk == HPK_IMPLANTS && info->IsImplant()) movepnts = static_cast<const ImplantInfo*>(info)->GetMp2Act();


		if(scheme == ThingDelivery::ST_GROUND) movepnts += AIUtils::CalcMps4ThingPickUp(thing);

		if(betwHumans) movepnts += 2;	// количество мувпойнтов на передачу предмета

        return movepnts;
    }

    //можно ли положить предмет в этот слот
    static bool IsSuitablePack(BaseThing* thing, human_pack_type hpk)
    {
        const ThingInfo* info = thing->GetInfo();

        if(info->IsMoney()) return false;

        switch(hpk){
        case HPK_HEAD:
            return info->IsArmor() && static_cast<const ArmorInfo*>(info)->IsHelmet();

        case HPK_BODY:
            return info->IsArmor() && !static_cast<const ArmorInfo*>(info)->IsHelmet();

        case HPK_HANDS:
            return      info->IsWeapon()
                    ||  info->IsCamera()
                    ||  info->IsGrenade()
                    ||  info->IsMedikit()
                    ||  info->IsScanner()
                    ||  info->IsShield();

        case HPK_IMPLANTS:
            return info->IsImplant();

        case HPK_LKNEE:
        case HPK_RKNEE:
        case HPK_BACKPACK:                
            return true;

        case HPK_ALL:
        case HPK_NONE:
            return false;
        }

        return false;
    }

    //подходит ли предмет человеку по интеллекту
    static bool IsEnoughWisdom(HumanEntity* human, BaseThing* thing)
    {
        int   thing_wisdom = 0;
        const ThingInfo* info = thing->GetInfo();

        switch(info->GetType()){
        case TT_WEAPON: 
            thing_wisdom = static_cast<const WeaponInfo*>(info)->GetWisdom();
            break;
        case TT_CAMERA:
            thing_wisdom = static_cast<const CameraInfo*>(info)->GetWisdom();
            break;
        case TT_SHIELD:
            thing_wisdom = static_cast<const ShieldInfo*>(info)->GetWisdom();
            break;
        case TT_MEDIKIT:
            thing_wisdom = static_cast<const MedikitInfo*>(info)->GetWisdom();
            break;
        case TT_GRENADE:
            thing_wisdom = static_cast<const GrenadeInfo*>(info)->GetWisdom();
            break;
        case TT_SCANNER:
            thing_wisdom = static_cast<const ScannerInfo*>(info)->GetWisdom();
            break;
        case TT_IMPLANT:
            thing_wisdom = static_cast<const ImplantInfo*>(info)->GetWisdom();
            break;
        }

        return human->GetEntityContext()->GetTraits()->GetWisdom() >= thing_wisdom; 
    }

    //подходит ли этот предмет человеку по силе
    static bool IsEnoughStrength(HumanEntity* human, BaseThing* thing)
    {
        int   thing_strength = 0;
        const ThingInfo* info = thing->GetInfo();

        switch(info->GetType()){
        case TT_WEAPON:
            thing_strength = static_cast<const WeaponInfo*>(info)->GetStrength();
            break;
        case TT_ARMOUR:
            thing_strength = static_cast<const ArmorInfo*>(info)->GetStrength();
            break;
        case TT_GRENADE:
            thing_strength = static_cast<const GrenadeInfo*>(info)->GetStrength();
            break;
        }

        return human->GetEntityContext()->GetTraits()->GetStrength() >= thing_strength;
    }

	static bool IsEnoughRank(BaseThing *thing)
	{
		HumanEntity *mainHero = 0;

		EntityPool::iterator i = EntityPool::GetInst()->begin(ET_HUMAN,PT_PLAYER);
		for(; i != EntityPool::GetInst()->end(); ++i)
		{
			if(i->Cast2Human() && i->GetInfo()->GetRID() == "player")
			{
				mainHero = i->Cast2Human();
			}
		}

		int rank = thing->GetInfo()->GetRank();
		return rank <= (mainHero ? mainHero->GetEntityContext()->GetRank() : 0xFFFF);
	}

    //это пулемет?
    static bool IsCannon(BaseThing* thing)
    {
        return thing && thing->GetInfo()->IsWeapon() && static_cast<WeaponThing*>(thing)->GetInfo()->IsCannon();
    }

    //это скафандр?
    static bool IsSpacesuit(BaseThing* thing)
    {
        return thing && thing->GetInfo()->IsArmor() && static_cast<ArmorThing*>(thing)->GetInfo()->IsSpaceSuit();
    }

    //совместимы ли предметы?
    static bool CanUseFirstWithSecond(BaseThing* first, BaseThing* second)
    {
        if(first->GetInfo()->IsAmmo() && second->GetInfo()->IsWeapon()){

            AmmoThing*   ammo   = static_cast<AmmoThing*>(first);
            WeaponThing* weapon = static_cast<WeaponThing*>(second);

            return ammo->GetInfo()->GetCaliber() == weapon->GetInfo()->GetCaliber();
        }

        return false;
    }
};

}

//=====================================================================================//
//                           void ThingDelivery::Activate()                            //
//=====================================================================================//
void ThingDelivery::Activate(scheme_type scheme, HumanEntity* human,
							 BaseThing* thing, human_pack_type hpk)
{
	Activate(scheme,human,thing,hpk,false);
}

//=====================================================================================//
//                           void ThingDelivery::Activate()                            //
//=====================================================================================//
void ThingDelivery::Activate(scheme_type scheme, HumanEntity* human,
							 BaseThing* thing, human_pack_type hpk,
							 bool betweenPlayers)
{
	const int movepnts = ThingDeliveryHelper::CalcMovepnts(scheme,thing,hpk,betweenPlayers);
    human->GetEntityContext()->GetTraits()->PoLAddMovepnts(-movepnts);
}

//=====================================================================================//
//                          bool ThingDelivery::IsSuitable()                           //
//=====================================================================================//
bool ThingDelivery::IsSuitable(BaseThing* first, BaseThing* second)
{
    return ThingDeliveryHelper::CanUseFirstWithSecond(first, second)
		|| ThingDeliveryHelper::CanUseFirstWithSecond(second, first);
}

//=====================================================================================//
//                          bool ThingDelivery::IsSuitable()                           //
//=====================================================================================//
bool ThingDelivery::IsSuitable(HumanEntity* human, BaseThing* thing)
{
    return ThingDeliveryHelper::IsEnoughWisdom(human, thing)
		&& ThingDeliveryHelper::IsEnoughStrength(human, thing);
		//&& ThingDeliveryHelper::IsEnoughRank(thing);
}

//=====================================================================================//
//                           bool ThingDelivery::CanInsert()                           //
//=====================================================================================//
bool ThingDelivery::CanInsert(scheme_type scheme, HumanEntity* human,
							  BaseThing* thing, human_pack_type hpk, reason_type* res,
							  bool betwHumans)
{
    if(!ThingDeliveryHelper::IsSuitablePack(thing, hpk))
	{
        if(res) *res = RT_UNSUITABLE_PACK;
        return false;
    }

    HumanContext* context = human->GetEntityContext();

	if(scheme != ST_FASTACCESS)
	{
		if(context->GetTraits()->GetMovepnts() < ThingDeliveryHelper::CalcMovepnts(scheme, thing, hpk, betwHumans) && !UnlimitedMoves())
		{
			if(res) *res = RT_NOT_ENOUGH_MOVEPNTS;
			return false;
		}
	}

    if(ThingDeliveryHelper::NeedActivate(thing, hpk))
	{
        if(!ThingDeliveryHelper::IsEnoughWisdom(human, thing))
		{
            if(res) *res = RT_NOT_ENOUGH_WISDOM;
            return false;
        }
        
        if(!ThingDeliveryHelper::IsEnoughStrength(human, thing))
		{
            if(res) *res = RT_NOT_ENOUGH_STRENGTH;
            return false;
        }                
    }

    HumanContext::iterator itor = context->begin(HPK_BODY);

	//if(hpk == HPK_HANDS && ThingDeliveryHelper::IsCannon(thing) 
	//	&& !ThingDeliveryHelper::IsSpacesuit(itor != context->end() ? &*itor : 0))
	//{
	//	if(res) *res = RT_CANNON_WITHOUT_SPACESUIT;
	//	return false;
	//}

	//if(hpk == HPK_HANDS && !ThingDeliveryHelper::IsCannon(thing)
	//	&& ThingDeliveryHelper::IsSpacesuit(itor != context->end() ? &*itor : 0)
	//	)
	//{
	//	if(res) *res = RT_SPACESUIT_WITH_NO_CANNON;
	//	return false;
	//}

    bool b1 = ThingDeliveryHelper::IsSpacesuit(thing),
            b2 = !human->GetInfo()->CanUseScuba();

    if(hpk == HPK_BODY && ThingDeliveryHelper::IsSpacesuit(thing)
		&& !human->GetInfo()->CanUseScuba())
	{
        if(res) *res = RT_CAN_NOT_USE_SCUBA;
        return false;
    }

	//if(hpk == HPK_BODY && ThingDeliveryHelper::IsSpacesuit(thing)
	//	&& !ThingDeliveryHelper::IsCannon(context->GetThingInHands()))
	//{
	//	if(res) *res = RT_SPACESUIT_WITH_NO_CANNON;
	//	return false;
	//}

	//if(hpk == HPK_BODY && !ThingDeliveryHelper::IsSpacesuit(thing)
	//	&& ThingDeliveryHelper::IsCannon(context->GetThingInHands()))
	//{
	//	if(res) *res = RT_CANNON_WITHOUT_SPACESUIT;
	//	return false;
	//}

    if(res) *res = RT_NONE;
    return true;
}

//=====================================================================================//
//                       std::string ThingDelivery::GetReason()                        //
//=====================================================================================//
std::string ThingDelivery::GetReason(reason_type reason)
{
    switch(reason)
	{
		case RT_UNSUITABLE_PACK: return DirtyLinks::GetStrRes("aui_unsuitable_pack");
		case RT_NOT_ENOUGH_WISDOM: return DirtyLinks::GetStrRes("aui_no_wisdom");
		case RT_NOT_ENOUGH_STRENGTH: return DirtyLinks::GetStrRes("aui_no_strength");
		case RT_NOT_ENOUGH_MOVEPNTS: return DirtyLinks::GetStrRes("aui_no_movepnts");
		case RT_CAN_NOT_USE_SCUBA: return  DirtyLinks::GetStrRes("aui_can_not_use_scuba");
		case RT_CANNON_WITHOUT_SPACESUIT: return DirtyLinks::GetStrRes("aui_cannon_spacesuit");
		case RT_SPACESUIT_WITH_NO_CANNON: return DirtyLinks::GetStrRes("aui_spacesuit_cannon");
		case RT_PACK_IS_FULL: return DirtyLinks::GetStrRes("aiu_pack_is_full");
    }

    return std::string();
}

//=========================================================================

namespace AIUtilsDetail{

	//
	// реализация новостной записи
	//
	class NewsInfo : public NewsPool::Info{
	public:

		NewsInfo(){}
		~NewsInfo(){}

		const rid_t& GetRID() const { return m_rid; }
		const std::string& GetDesc() const { return m_desc; }
		const std::string& GetShader() const { return m_shader; }
		const std::string& GetChannel() const { return m_channel; }
		
	private:
		
		friend class NewsInfoReader;
		
		rid_t       m_rid;		
		std::string m_desc;
		std::string m_shader;
		std::string m_channel;
	};

//=====================================================================================//
//              class NewsInfoReader : public InfoReader<NewsInfoReader>               //
//=====================================================================================//
// считывание информации c диска
class NewsInfoReader : public InfoReader<NewsInfoReader>
{

#define POL_NEWS_INFO_TABLE(P,D,DELIM)								\
	P(m_sys_name,		"sys_name",			REQUIRED, D) DELIM()	\
	P(m_phase,			"phase",			REQUIRED, D) DELIM()	\
	P(m_episode,		"episode",			REQUIRED, D) DELIM()	\
	P(m_channel,		"channel",			REQUIRED, D) DELIM()	\
	P(m_image,			"image",			REQUIRED, D)

	POL_NEWS_INFO_TABLE(POL_MAKE_DECLARATION,(NewsInfoReader),POL_EMPTY)

public:
    class Acceptor
	{
    public:

        virtual ~Acceptor(){}

        //принять считанную информацию
        virtual void Accept(const NewsInfo& news) = 0;
    };


	static const std::string &GetTableName()
	{
		return news_xls;
	}

	static void ReadColumns()
	{
		table::row_type hdr = GetHdr();
		std::ostringstream errout;

		POL_NEWS_INFO_TABLE(POL_MAKE_INITIALIZATION,(NewsInfoReader),POL_EMPTY)

		if(!errout.str().empty())
		{
			throw CASUS(errout.str());
		}
	}

    static void Read(Acceptor* acceptor, int episode, int phase)
    {
		using mll::utils::table;

		int line_count = GetTbl().height();
        NewsInfo info;

        //найдем нужную строку
        for(int line = 1; line < line_count; ++line)
		{
			if(IsSuitable(m_episode[line], episode) && IsSuitable(m_phase[line], phase))
			{
				ReadInfo(line,info);
				acceptor->Accept(info);
            }
        }
     }

    static void Read(Acceptor* acceptor, const rid_t& rid)
    {
 		using mll::utils::table;

		int line_count = GetTbl().height();
        NewsInfo info;

        //найдем нужную строку
        for(int line = 1; line < line_count; ++line)
		{
			if(m_sys_name[line] == rid)
			{
				ReadInfo(line,info);
				acceptor->Accept(info);
				return;
            }
        }

        throw CASUS(("NewsInfoReader: не найден profile для <" +  rid + ">!").c_str());            
    }

private:
    static void ReadInfo(int line, NewsInfo& info)
    {
		info.m_rid = m_sys_name[line];
		info.m_desc = PoL::getLocStr("news." + info.m_rid + ".news");
		info.m_shader = m_image[line];
		info.m_channel = m_channel[line];
    }

	static bool IsSuitable(const std::string& str, int number)
	{
		typedef boost::char_separator<char> Sep_t;
		typedef boost::tokenizer<Sep_t> Tok_t;
		Sep_t sep(".,;: ");
		Tok_t tok(str,sep);

		for(Tok_t::iterator it = tok.begin(); it != tok.end(); ++it)
		{
			try
			{
				if(boost::lexical_cast<int>(*it) == number) return true;
			}
			catch(boost::bad_lexical_cast &)
			{
			}
		}

		return false;
	}
};

POL_NEWS_INFO_TABLE(POL_MAKE_DEFINITION,(NewsInfoReader),POL_EMPTY)

    //
    // Реализация хранилища новостей
    //
    class NewsInfoPoolImp : public  NewsPool,
                            private SpawnObserver,
                            private NewsInfoReader::Acceptor
    {
    public:

        NewsInfoPoolImp(){}
        ~NewsInfoPoolImp(){}

        void Init()
        {
            Spawner::Attach(this, ET_ENTRY_LEVEL);
            Spawner::Attach(this, ET_PHASE_CHANGE);
        }

        void Shut()
        {
            Spawner::Detach(this);
        }

        //добавить новость в хранилище
        void Push(const rid_t& rid)
        {
			NewsInfoReader::Read(this,rid);
			MessageCounter::IncMessageCounter();
        }

        //сохранение/загрузка новостей
        void MakeSaveLoad(SavSlot& slot)
        {
            if(slot.IsSaving())
			{
                slot << m_news.size();

                for(size_t k = 0; k < m_news.size(); k++)
                    slot << m_news[k].GetRID();                          
            }
			else
			{
                m_news.clear();

                size_t count;
                slot >> count;

                while(count--)
				{
                    rid_t  rid;
                    slot >> rid;

					NewsInfoReader::Read(this, rid);
                }
            }
        }

        //породить полиморфный итератор
        Iterator* CreateIterator()
        {
            return new IteratorImp(m_news);
        }

    private:

        void Accept(const NewsInfo& info)
        {
            m_news.push_back(info);
        }

        void Update(subject_t subj, event_t event, info_t info)
        {
            if(event == ET_ENTRY_LEVEL){

                entry_info_s* ptr = static_cast<entry_info_s*>(info);

                //если это выход на новый эпизод или вход нв новый уровень
                if(ptr->m_fnew_episode || ptr->m_entry.empty()){
                    ReReadNews(Spawner::GetInst()->GetEpisode(),
                               Spawner::GetInst()->GetPhase());
                    return;
                }
            }

            if(event == ET_PHASE_CHANGE){
                ReReadNews(Spawner::GetInst()->GetEpisode(),
                           Spawner::GetInst()->GetPhase());
            }
        }

        void ReReadNews(int episode, int phase)
        {
            m_news.clear(); 
            
			NewsInfoReader::Read(this,episode,phase);

            GameEvMessenger::GetInst()->Notify(GameObserver::EV_NEW_NEWS);
        }

    private:

        typedef std::vector<NewsInfo> news_vec_t;

        class IteratorImp : public NewsPool::Iterator{
        public:

            IteratorImp(news_vec_t& news) : m_news(news)
            { m_first = m_news.rbegin(); }

            void Next() { m_first++; }

            void First() { m_first = m_news.rbegin(); }

            bool IsNotDone() const { return m_first != m_news.rend(); }

            const Info* Get(){ return m_first != m_news.rend() ? &*m_first : 0; }

        private:

            news_vec_t& m_news;
            news_vec_t::reverse_iterator m_first;
        };

    private:

        typedef std::vector<NewsInfo> news_vec_t;
        news_vec_t m_news;
    };    
}

NewsPool* NewsPool::GetInst()
{
    static NewsInfoPoolImp imp;
    return &imp;
}

//=========================================================================

namespace AIUtilsDetail{

    class DiaryManagerImp : public DiaryManager
	{
    public:

        DiaryManagerImp() : m_key_gen(0) {}
        ~DiaryManagerImp(){}

        struct game_notifier
		{
            game_notifier(GameObserver::event_type event, GameObserver::info_t info = 0) : 
                m_info(info), m_event(event) {}

            ~game_notifier() { GameEvMessenger::GetInst()->Notify(m_event, m_info); }
        
            GameObserver::info_t     m_info;
            GameObserver::event_type m_event;
        };

        //удалить запись
        void Delete(key_t key)
        {
            GameObserver::diary_change info(GameObserver::diary_change::CT_DEL, key);
            game_notifier notifier(GameObserver::EV_DIARY_CHANGE, &info);

            if(key == ROOT_KEY){
                m_records.clear();
                return;
            }

            record_lst_t::iterator itor = FindByKey(key);

            //если такой записи нет - выход
            if(itor == m_records.end())
                return;

            //удалим саму запись
            key_t key_up = itor->GetKey(); 
            m_records.erase(itor);

            //удалим все записи связанные с этой
            itor = m_records.begin();
            while(itor != m_records.end()){

                if(itor->GetKeyUp() == key_up){
                    itor = m_records.erase(itor);
                    continue;
                }

                ++itor;
            }
        }

        //добавить запись
        key_t Insert(const Record& rec)
        {
            m_records.push_back(rec);
            m_records.back().SetKey(++m_key_gen);
            
            GameObserver::diary_change info(GameObserver::diary_change::CT_ADD, m_key_gen);
            GameEvMessenger::GetInst()->Notify(GameObserver::EV_DIARY_CHANGE, &info);

			MessageCounter::IncMessageCounter();

            return m_key_gen;
        }

        Record* Get(key_t key)
        {
            record_lst_t::iterator itor = FindByKey(key);
            return itor != m_records.end() ? &*itor : 0;
        }

        //создать итератор
        Iterator* CreateIterator() { return new IteratorImp(m_records); }

        void MakeSaveLoad(SavSlot& slot)
        {
            if(slot.IsSaving()){

                slot << m_key_gen;
                slot << m_records.size();

                record_lst_t::iterator itor = m_records.begin();
                while(itor != m_records.end()){
                    slot << *itor;
                    ++itor;
                }

            }else{

                slot >> m_key_gen;

                size_t size;
                slot >> size;

                m_records.clear();
                while(size--){                    
                    m_records.push_back(Record());
                    slot >> m_records.back();
                }
            }
        }

    private:

        typedef std::list<Record> record_lst_t;

        record_lst_t::iterator FindByKey(key_t key)
        {
            record_lst_t::iterator itor = m_records.begin();
            while(itor != m_records.end()){
                
                if(itor->GetKey() == key) 
                    return itor;

                ++itor;
            }

            return m_records.end();
        }

        class IteratorImp : public DiaryManager::Iterator{
        public:

            IteratorImp(record_lst_t& recs) : 
              m_recs(recs), m_key_up(NULL_KEY) { First(m_key_up); }            

            void Next()
            {
                for(m_first++; m_first != m_recs.end(); m_first++){
                    if(IsSuitable(*m_first))
                        return;               
                }
            }

            void First(key_t key_up)
            {
                m_key_up = key_up;
                m_first  = m_recs.begin();

                if(m_first != m_recs.end() && !IsSuitable(*m_first))
                    Next();
            }
            
            bool IsNotDone() const { return m_first != m_recs.end(); } 

            Record* Get() { return m_first != m_recs.end() ? &*m_first : 0; }

        private:

            bool IsSuitable(const Record& rec) const
            { return m_key_up == NULL_KEY || rec.GetKeyUp() == m_key_up; }

        private:

            record_lst_t& m_recs;
            unsigned      m_key_up;

            record_lst_t::iterator m_first;
        };

    private:

        int          m_key_gen;
        record_lst_t m_records;
    };
}

DiaryManager* DiaryManager::GetInst()
{
    static DiaryManagerImp imp;
    return &imp;
}

//=========================================================================

EpisodeMapMgr* EpisodeMapMgr::GetInst()
{
    static EpisodeMapMgr imp;
    return &imp;
}

EpisodeMapMgr::EpisodeMapMgr() : 
    m_fupdated(false)
{
}

void EpisodeMapMgr::Reset()
{
    m_levels.clear();
}

void EpisodeMapMgr::Insert(const rid_t& rid)
{
    if(m_levels.count(rid) == 0)
	{
        m_fupdated = true;
        m_levels.insert(rid);
		MessageCounter::IncMessageCounter();
    }
}

void EpisodeMapMgr::MakeSaveLoad(SavSlot& slot)
{
    AIUtils::SaveLoadRIDSet(slot, m_levels);

    if(slot.IsSaving())
        slot << m_fupdated;
    else
        slot >> m_fupdated;
}

EpisodeMapMgr::iterator EpisodeMapMgr::begin()
{
    return iterator(m_levels.begin());
}

EpisodeMapMgr::iterator EpisodeMapMgr::end()
{
    return iterator(m_levels.end());
}

bool EpisodeMapMgr::IsUpdated() const
{
    return m_fupdated;
}

void EpisodeMapMgr::DropUpdatedFlag()
{
    m_fupdated = false;
}

//================================================================================

namespace AIUtilsDetail{

    enum helper_type{
        HT_NEW_HERO,
        HT_NEW_GAME,
        HT_NEW_QUEST,
            
        HT_GAME_MENU,
        HT_TRADE_MENU,
        HT_JOURNAL_MENU,
        HT_INVENTORY_MENU,

        HT_FIRST_ENEMY,        
        HT_BARTOLOMIU_TALK,
        HT_CHINA_TOWN_LOAD,
        
        MAX_HELPER,
    };

    typedef unsigned hid_t;

    const char* HelperType2SysName(helper_type type)
    {
        switch(type){
        case HT_NEW_HERO: return "new_hero";
        case HT_NEW_GAME: return "new_game";
        case HT_NEW_QUEST: return "new_quest";
        case HT_GAME_MENU: return "game_menu";
        case HT_TRADE_MENU: return "trade_menu";
        case HT_FIRST_ENEMY: return "first_enemy";
        case HT_JOURNAL_MENU: return "journal_menu";
        case HT_INVENTORY_MENU: return "inventory_menu";
        case HT_BARTOLOMIU_TALK: return "bartolomiu_talk";
        case HT_CHINA_TOWN_LOAD: return "china_town_load";
        }
        
        return "unknown";
    }

    class HelpInfo{
    public:

        HelpInfo(helper_type type, hid_t hid)
        {
            TxtFilePtr txt(helper_xls);
            
            static bool first_time = true;
            
            if(first_time)
			{
                txt.ReadColumns(m_columns, m_columns + MAX_COLUMNS);
                first_time = false;
            }
            
            rid_t rid = HelperType2SysName(type);
            
            if(hid < (hid_t)txt->SizeY())
			{
                //найдем нужную строку
                std::string str;          
                txt->GetCell(hid, m_columns[CT_SYSNAME].m_index, str);
                
                //простейшая защита от считывания неверного helper'a
                if(str == rid)
				{                
                    txt->GetCell(hid, m_columns[CT_IMAGE].m_index, m_image);
					m_text = PoL::getLocStr("helper." + rid + ".message");
                    //txt->GetCell(hid, m_columns[CT_MESSAGE].m_index, m_text);
                    return;
                }
            }
           
            throw CASUS(("HelpInfo: не найден profile для <" +  rid + ">!").c_str());                                
        }

        const std::string& GetText() const { return m_text; }
        const std::string& GetImage() const { return m_image; }

    private:

        std::string m_text;
        std::string m_image;

        enum column_type{
            CT_SYSNAME,
            //CT_MESSAGE,
            CT_IMAGE,
                
            MAX_COLUMNS,
        };
        
        static column m_columns[MAX_COLUMNS];
    };

    column HelpInfo::m_columns[] = 
    {
        column("sys_name", CT_SYSNAME),
        //column("message",  CT_MESSAGE),
        column("image",    CT_IMAGE),
    };
    
    //
    // управляющая структура для показа помощи
    //    
    class Helper{
    public:
        
        virtual ~Helper(){}

        enum command_type{
            CMD_NONE,
            CMD_SHOW,
        };

        DCTOR_ABS_DEF(Helper)
        
        //обработка тика
        virtual void Tick(command_type command) = 0;
        //сохранение/загрузка 
        virtual void MakeSaveLoad(SavSlot& slot) = 0;       
    };
    
    //
    // общая реализация для helper'ов
    //
    class CommonHelper : public Helper{
    public:

        CommonHelper() : m_flags(F_FIRST_TIME|F_ENABLE_CUR) {}

        void MakeSaveLoad(SavSlot& slot)
        {
            if(slot.IsSaving()){

                slot << m_flags;
                slot << m_fenable_all;

            }else{

                slot >> m_flags;
                slot >> m_fenable_all;
            }
        }

    protected:

        bool IsEnabled() const
        {
            return m_flags & F_ENABLE_CUR && m_fenable_all;
        }

        void Show(helper_type type, hid_t hid)
        {   
            if(DirtyLinks::GetIntOpt(DirtyLinks::OT_ENABLE_HELPER)){
                
                HelpInfo info(type, hid);

                if(m_flags & F_FIRST_TIME) m_flags = 0;
                
                Forms::helper_s helper(
                    info.GetImage(), 
                    info.GetText(),
                    (m_flags & F_ENABLE_CUR)!=0,
                    m_fenable_all);
                
                Forms::GetInst()->ShowHelper(helper);
                
                m_fenable_all = helper.m_fenable_all;
                m_flags = helper.m_fenable_cur ? F_ENABLE_CUR : 0;
            }
        }

    private:

        enum flag_type{
            F_FIRST_TIME = 1 << 0,
            F_ENABLE_CUR = 1 << 1,
        };

        unsigned    m_flags;
        static bool m_fenable_all;
    };

    bool CommonHelper::m_fenable_all = true;

    //
    // помощник для меню
    //
    class MenuHelper : public CommonHelper, private GameObserver{
    public:

        enum flag_type{
            F_CAN_SHOW   = 1 << 0,
            F_NEED_SHOW  = 1 << 1,
            F_WAS_SHOWN  = 1 << 2,    

            F_ONLY_ONE   = 1 << 3,
        };

        MenuHelper(){}

        MenuHelper(helper_type type, hid_t hid, GameObserver::menu_type menu, unsigned flags = 0) : 
            m_hid(hid), m_type(type), m_flags(flags), m_menu(menu) { AttachObserver(); }

        ~MenuHelper() { GameEvMessenger::GetInst()->Detach(this);  }

        DCTOR_DEF(MenuHelper)

        void MakeSaveLoad(SavSlot& slot)
        {
            CommonHelper::MakeSaveLoad(slot);

            if(slot.IsSaving()){                

                slot << m_hid;
                slot << m_flags;

                slot << static_cast<int>(m_menu);
                slot << static_cast<int>(m_type);
            
            }else{    
                
                slot >> m_hid;
                slot >> m_flags;

                int tmp;

                slot >> tmp; m_menu = static_cast<menu_type>(tmp);
                slot >> tmp; m_type  = static_cast<helper_type>(tmp);

                AttachObserver();
            }
        }

        void Tick(command_type command)
        {
            if(     (m_flags & F_NEED_SHOW && IsEnabled())
                ||  (m_flags & F_CAN_SHOW && command == CMD_SHOW)){
                Show(m_type, m_hid);            
                m_flags &= ~F_NEED_SHOW;
                m_flags |= F_WAS_SHOWN;
            }
        }

    private:

        void Update(subject_t subj, event_t event, info_t info)
        {
            menu_info* ptr = static_cast<menu_info*>(info);

            if(ptr->m_menu != m_menu){
                m_flags = 0;
                return;
            }
            
            m_flags |= F_CAN_SHOW;
            
            if(!(m_flags & F_WAS_SHOWN) || !(m_flags & F_ONLY_ONE))
                m_flags |= F_NEED_SHOW;            
        }

        void AttachObserver() { GameEvMessenger::GetInst()->Attach(this, EV_MENU); }

    private:

        hid_t m_hid;

        unsigned    m_flags;
        menu_type   m_menu;
        helper_type m_type;
    };

    DCTOR_IMP(MenuHelper)
    
    //
    // выдача подсказки о новой игре
    //
    class NewEventHelper : public CommonHelper, private GameObserver{
    public:

        NewEventHelper(){}

        NewEventHelper(helper_type htype, hid_t hid, GameObserver::event_type event) :
          m_fneed_show(false), m_helper(htype), m_hid(hid), m_event(event)
        { GameEvMessenger::GetInst()->Attach(this, m_event); }

        ~NewEventHelper() { GameEvMessenger::GetInst()->Detach(this); }

        DCTOR_DEF(NewEventHelper)

        void Tick(command_type command)
        {
            if(m_fneed_show && IsEnabled()){
                Show(m_helper, m_hid);
                m_fneed_show = false;
            }
        }

        void MakeSaveLoad(SavSlot& slot)
        {
            CommonHelper::MakeSaveLoad(slot);

            if(slot.IsSaving()){
            
                slot << m_hid;
                slot << m_fneed_show;

                slot << static_cast<int>(m_event);
                slot << static_cast<int>(m_helper);
            
            }else{

                slot >> m_hid;
                slot >> m_fneed_show;

                int tmp;

                slot >> tmp; m_event  = static_cast<event_type>(tmp);
                slot >> tmp; m_helper = static_cast<helper_type>(tmp);

                GameEvMessenger::GetInst()->Attach(this, m_event); 
            }
        }

    private:

        void Update(subject_t subj, event_t event, info_t info)
        { m_fneed_show = true; }

    private:

        bool  m_fneed_show;

        hid_t       m_hid;
        event_type  m_event;
        helper_type m_helper;
    };

    DCTOR_IMP(NewEventHelper)

    //
    // помошник для загрузки уровня
    //
    class LevelLoadHelper : public CommonHelper, private SpawnObserver{
    public:

        LevelLoadHelper(){}

        LevelLoadHelper(hid_t hid, const std::string& level) : 
          m_level(level), m_fneed_show(false), m_hid(hid)
        { Spawner::GetInst()->Attach(this, ET_ENTRY_LEVEL); }

        ~LevelLoadHelper() { Spawner::GetInst()->Detach(this); }

        DCTOR_DEF(LevelLoadHelper)

        void MakeSaveLoad(SavSlot& slot)
        {
            CommonHelper::MakeSaveLoad(slot);

            if(slot.IsSaving()){                
                slot << m_hid;
                slot << m_level;
                slot << m_fneed_show;
            }else{                
                slot >> m_hid;
                slot >> m_level;
                slot >> m_fneed_show;
            }
        }

        void Tick(command_type command)
        {
            if(m_fneed_show && IsEnabled()){
                m_fneed_show = false;
                Show(HT_CHINA_TOWN_LOAD, m_hid);
            }
        }
        
    private:

        void Update(subject_t subj, event_t event, info_t info)
        { m_fneed_show = m_level == DirtyLinks::GetLevelSysName(); }

    private:
        
        hid_t m_hid;
        bool  m_fneed_show;

        std::string m_level;
    };

    DCTOR_IMP(LevelLoadHelper)

    //
    // подсказка при добавлениии нового героя
    //
    class NewHeroHelper : public CommonHelper, private SpawnObserver{
    public:

        NewHeroHelper(hid_t hid = 0) : m_flags(0), m_hid(hid) { AttachObserver(); }

        ~NewHeroHelper() { Spawner::GetInst()->Detach(this); }

        DCTOR_DEF(NewHeroHelper)

        void Tick(command_type type)
        {
            if(m_flags & F_NEW_HERO && IsEnabled()){
                m_flags &= ~F_NEW_HERO;
                Show(HT_NEW_HERO, m_hid);
            }
        }

        void MakeSaveLoad(SavSlot& slot)
        {
            CommonHelper::MakeSaveLoad(slot);

            if(slot.IsSaving()){
                slot << m_hid;
                slot << m_flags;
            }else{
                slot >> m_hid;
                slot >> m_flags;
            }
        }

    private:

        void Update(subject_t subj, event_t event, info_t info)
        {
            switch(event){
            case ET_EXIT_LEVEL: 
                m_flags = 0;
                break;

            case ET_FINISH_SPAWN:
                m_flags |= F_SPAWN_FINISHED;
                break;

            case ET_ENTITY_SPAWN:
                if(m_flags & F_SPAWN_FINISHED){

                    BaseEntity* ent = static_cast<spawn_info_s*>(info)->m_entity;

                    if(ent && ent->Cast2Human() && ent->GetPlayer() == PT_PLAYER)
                        m_flags |= F_NEW_HERO;                    
                }
                break;
            }
        }

        void AttachObserver()
        {
            Spawner::GetInst()->Attach(this, ET_EXIT_LEVEL);
            Spawner::GetInst()->Attach(this, ET_ENTITY_SPAWN);
            Spawner::GetInst()->Attach(this, ET_FINISH_SPAWN);
        }

    private:

        enum flag_type{
            F_NEW_HERO       = 1 << 0,
            F_SPAWN_FINISHED = 1 << 1,
        };

        hid_t    m_hid;
        unsigned m_flags;        
    };

    DCTOR_IMP(NewHeroHelper)
   
    //
    // вывод сообщения при появлении скриптовой сцены
    //
    class ScriptSceneHelper : public CommonHelper, private GameObserver{
    public:

        ScriptSceneHelper(hid_t hid = 0) : m_flags(0), m_hid(hid)
        {
            GameEvMessenger::GetInst()->Attach(this, EV_USE);
            GameEvMessenger::GetInst()->Attach(this, EV_NEW_SSCENE);
        }

        ~ScriptSceneHelper() { GameEvMessenger::GetInst()->Detach(this); }

        DCTOR_DEF(ScriptSceneHelper)

        void Tick(command_type type) { m_flags = 0; }

        void MakeSaveLoad(SavSlot& slot)
        {
            CommonHelper::MakeSaveLoad(slot);

            if(slot.IsSaving()){
                slot << m_hid;
                slot << m_flags;
            }else{
                slot >> m_hid;
                slot >> m_flags;
            }
        }

    private:

        void Update(subject_t subj, event_t event, info_t info)
        {
            switch(event){
            case EV_USE:
                {
                    use_info* ptr = static_cast<use_info*>(info);

                    if(     ptr->m_actor && ptr->m_actor->GetPlayer() == PT_PLAYER
                        &&  ptr->m_ent4use && ptr->m_ent4use->GetInfo()->GetRID() == bartolomiu_sys_name)
                        m_flags |= F_WAS_TALK;                        
                }
                break;

            case EV_NEW_SSCENE:
                m_flags |= F_NEW_SCENE;
                break;
            }

            if(m_flags == (F_WAS_TALK|F_NEW_SCENE) && IsEnabled())
                Show(HT_BARTOLOMIU_TALK, m_hid);
        }

    private:

        enum flag_type{
            F_WAS_TALK  = 1 << 0,
            F_NEW_SCENE = 1 << 1,
        };

        hid_t    m_hid;
        unsigned m_flags;
    };

    DCTOR_IMP(ScriptSceneHelper)

    //
    // вывод сообщения если увидели первого врага
    //
    class FirstEnemyHelper : public  CommonHelper, 
                             private GameObserver,
                             private SpawnObserver,
                             private SpectatorObserver,
                             private RelationsObserver
    {
    public:

        FirstEnemyHelper(){}

        FirstEnemyHelper(hid_t hid) : m_hid(hid), m_flags(0) { AttachObservers(); }

        ~FirstEnemyHelper(){ DetachObservers(); }

        DCTOR_DEF(FirstEnemyHelper)

        void Tick(command_type type)
        {
            if(     IsEnabled()
                &&  m_flags & F_NEED_SHOW
                &&  m_flags & F_MENU_SHOWN){
                
                m_flags &= ~F_NEED_SHOW;
                m_flags |=  F_WAS_SHOWN;

                DetachObservers();

                Show(HT_FIRST_ENEMY, m_hid);
            }
        }

        void MakeSaveLoad(SavSlot& slot)
        {
            CommonHelper::MakeSaveLoad(slot);

            if(slot.IsSaving()){
                
                slot << m_hid;
                slot << m_flags;

            }else{

                slot >> m_hid;
                slot >> m_flags;

                AttachObservers();
            }
        }

    private:

        void Update(GameObserver::subject_t subj, GameObserver::event_t event, GameObserver::info_t info)
        {
            menu_info* ptr = static_cast<menu_info*>(info);

            if(ptr->m_menu == MT_GAME)
                m_flags |= F_MENU_SHOWN;
            else
                m_flags &= ~F_MENU_SHOWN;
        }

        void Update(SpawnObserver::subject_t subj, SpawnObserver::event_t event, SpawnObserver::info_t info)
        {
            if(event == ET_EXIT_LEVEL)
			{
                m_flags &= ~F_WAS_SPAWN; 
                if(Spectator* sp = PlayerSpectator::GetInst()->GetSpectator()) sp->DetachSpectatorObserver(this);
            }

            if(event == ET_FINISH_SPAWN)
			{
                m_flags |= F_WAS_SPAWN;

                Spectator* spectator = PlayerSpectator::GetInst()->GetSpectator();

                spectator->AttachSpectatorObserver(this, EV_VISIBLE_MARKER);
                VisMap::MarkIterator_t itor = VisMap::GetInst()->MarkBegin(spectator, VisParams::vpVisible);
                
                for(; itor != VisMap::GetInst()->MarkEnd(); ++itor)
				{
                    if(     itor->GetEntity()
                        &&  EnemyDetector::getInst()->getRelationBetweenPlayerAndHe(itor->GetEntity()) == RT_ENEMY)
					{
                        m_flags |= F_NEED_SHOW;
                        break;
                    }
                }
            }
        }

        void Update(SpectatorObserver::subject_t subj, SpectatorObserver::event_t event, SpectatorObserver::info_t info)
        {
            BaseEntity*  entity   = static_cast<marker_info*>(info)->m_marker->GetEntity();
            RelationType relation = EnemyDetector::getInst()->getRelationBetweenPlayerAndHe(entity);

            if(relation == RT_ENEMY) m_flags |= F_NEED_SHOW; 
        }

        void AttachObservers()
        { 
            if(m_flags & F_WAS_SHOWN) return;

            Spawner::GetInst()->Attach(this, ET_EXIT_LEVEL);
            Spawner::GetInst()->Attach(this, ET_FINISH_SPAWN);

            GameEvMessenger::GetInst()->Attach(this, EV_MENU);

            EnemyDetector::getInst()->attach(this, ET_RELATION2PLAYER_CHANGED);

            if(m_flags & F_WAS_SPAWN) PlayerSpectator::GetInst()->GetSpectator()->AttachSpectatorObserver(this, EV_VISIBLE_MARKER);
        }

        void DetachObservers()
        {
            Spawner::GetInst()->Detach(this);
            EnemyDetector::getInst()->detach(this);
            GameEvMessenger::GetInst()->Detach(this);

            if(Spectator* sp = PlayerSpectator::GetInst()->GetSpectator()) sp->DetachSpectatorObserver(this);
        }

        void changed(RelationsObserver::event_type et, void* param)
        {
            BaseEntity*  entity   = EntityPool::GetInst()->Get(static_cast<EntityDesc*>(param)->m_id);
            RelationType relation = EnemyDetector::getInst()->getRelationBetweenPlayerAndHe(entity);
            VisParams flags    = VisMap::GetInst()->GetVisFlags(PlayerSpectator::GetInst()->GetSpectator(),
                                                                   entity->GetEntityContext()->GetMarker());

            if(relation == RT_ENEMY && flags.IsVisible()) m_flags |= F_NEED_SHOW;          
        }
        
    private:

        enum flag_type{
            F_NEED_SHOW   = 1 << 0,
            F_WAS_SHOWN   = 1 << 1,
            F_WAS_SPAWN   = 1 << 2,
            F_MENU_SHOWN  = 1 << 3,
        };

        hid_t    m_hid;
        unsigned m_flags;
    };

    DCTOR_IMP(FirstEnemyHelper)

    //
    // фабрика помошников
    //
    class HelperFactory{
    public:

        //приемшик пмошников
        class Acceptor{
        public:

            virtual ~Acceptor(){}

            virtual void Accept(Helper* helper) = 0;
        };

        //породить помошников
        void Create(Acceptor* acceptor)
        {
            TxtFilePtr txt(helper_xls);
            column     col("sys_name", 0);
            
            txt.ReadColumns(&col, &col + 1);

            for(int k = 0; k < MAX_HELPER; k++){

                helper_type type = static_cast<helper_type>(k);
                rid_t       rid = HelperType2SysName(type);
            
                //ищем все helpers этого типа
                for(int line = 0; line < txt->SizeY(); line++){
                
                    std::string str;          
                    txt->GetCell(line, col.m_index, str);

                    //нашли строку с помошником
                    if(str == rid) acceptor->Accept(MakeHelper(type, line));
                }
            }
        }

    private:

        Helper* MakeHelper(helper_type type, hid_t hid)
        {
            switch(type){
            case HT_FIRST_ENEMY:
                return new FirstEnemyHelper(hid);

            case HT_NEW_HERO:
                return new NewHeroHelper(hid);

            case HT_NEW_GAME:
                return new NewEventHelper(HT_NEW_GAME, hid, GameObserver::EV_NEW_GAME);

            case HT_NEW_QUEST:  
                return new NewEventHelper(HT_NEW_QUEST, hid, GameObserver::EV_NEW_QUEST);

            case HT_GAME_MENU:
                return new MenuHelper(HT_GAME_MENU, hid, GameObserver::MT_GAME, MenuHelper::F_ONLY_ONE);

            case HT_TRADE_MENU:
                return new MenuHelper(HT_TRADE_MENU, hid, GameObserver::MT_TRADE);

            case HT_JOURNAL_MENU:
                return new MenuHelper(HT_JOURNAL_MENU, hid, GameObserver::MT_JOURNAL);

            case HT_INVENTORY_MENU:        
                return new MenuHelper(HT_INVENTORY_MENU, hid, GameObserver::MT_INVENTORY);

            case HT_BARTOLOMIU_TALK:
                return new ScriptSceneHelper(hid);

            case HT_CHINA_TOWN_LOAD:
                return new LevelLoadHelper(hid, "china_town");
            }

            return 0;
        }
    };

//    //
//    // управление помощником
//    //
//    class HelperManagerImp : public HelperManager //, private HelperFactory::Acceptor
//    {
//    public:
//
//        HelperManagerImp() { }
//
//        void Init()
//        {
//            //HelperFactory factory;
//            //factory.Create(this);
//        }
//
//        void Shut()
//        {
//            //for(size_t k = 0; k < m_helpers.size(); delete m_helpers[k++]);
//            //m_helpers.clear();
//        }
//
//        void Tick()
//        {
//            //Helper::command_type command =      Input::KeyBack(DIK_F1) 
//            //                                ?   Helper::CMD_SHOW
//            //                                :   Helper::CMD_NONE;
//
//            //for(size_t k = 0; k < m_helpers.size(); k++)
//            //    m_helpers[k]->Tick(command);            
//        }
//
//        void MakeSaveLoad(SavSlot& slot)
//        {
//			//if(slot.IsSaving())
//			//{
//			//	slot << m_helpers.size();
//
//			//	for(size_t k = 0; k < m_helpers.size(); k++)
//			//	{
//			//		DynUtils::SaveObj(slot, m_helpers[k]);
//			//		m_helpers[k]->MakeSaveLoad(slot);
//			//	}
//			//}
//			//else
//			//{
//			//	Shut();
//
//			//	size_t size;
//			//	slot >> size;
//
//			//	while(size--)
//			//	{
//			//		Helper* helper = 0;
//
//			//		DynUtils::LoadObj(slot, helper);
//			//		helper->MakeSaveLoad(slot);
//
//			//		m_helpers.push_back(helper);
//			//	}
//			//}
//        }
//
//	//private:
//	//	void Accept(Helper* helper) { if(helper) m_helpers.push_back(helper);}
//
//    //private:
//    //    typedef std::vector<Helper*> helper_vec_t;
//    //    helper_vec_t m_helpers;
//    };
}

//HelperManager* HelperManager::GetInst()
//{
//    static HelperManagerImp imp;
//    return &imp;
//}

//================================================================================

namespace AIUtilsDetail{

    //
    // определение группы из 3-х hex'ов
    //
    struct hex_triad_s{

        HexGrid::cell* m_c1;
        HexGrid::cell* m_c2;
        HexGrid::cell* m_c3;

        point3 CalcCenter() const
        {
            point3 ret;

            ret.x = (m_c1->x + m_c2->x + m_c3->x)/3;
            ret.y = (m_c1->y + m_c2->y + m_c3->y)/3;
            ret.z = DirtyLinks::GetLandHeight(ret);

            return ret;
        }

        //сформировать ключ
        unsigned GetKey() const
        {
            return      reinterpret_cast<unsigned>(m_c1)
                    +   reinterpret_cast<unsigned>(m_c2)
                    +   reinterpret_cast<unsigned>(m_c3);
        }

        //совпадают ли триады
        bool IsCongruent(const hex_triad_s& t) const
        {
            return      (m_c1 == t.m_c1 || m_c1 == t.m_c2 || m_c1 == t.m_c3)
                    &&  (m_c2 == t.m_c1 || m_c2 == t.m_c2 || m_c2 == t.m_c3)
                    &&  (m_c3 == t.m_c1 || m_c3 == t.m_c2 || m_c3 == t.m_c3); 
        }

        //пересекаются ли триады
        bool IsIntersects(const hex_triad_s& t) const
        {
            return      (m_c1 == t.m_c1 || m_c1 == t.m_c2 || m_c1 == t.m_c3)
                    ||  (m_c2 == t.m_c1 || m_c2 == t.m_c2 || m_c2 == t.m_c3)
                    ||  (m_c3 == t.m_c1 || m_c3 == t.m_c2 || m_c3 == t.m_c3); 
        }

        void MakeSaveLoad(SavSlot& slot)
        {
            HexGrid::cell* first = HexGrid::GetInst()->first_cell();

            if(slot.IsSaving()){

                slot << m_c1 - first;
                slot << m_c2 - first;
                slot << m_c3 - first;

            }else{

                ptrdiff_t offset;

                slot >> offset; m_c1 = first + offset;
                slot >> offset; m_c2 = first + offset;
                slot >> offset; m_c3 = first + offset;
            }
        }
        
        hex_triad_s(){}

        hex_triad_s(HexGrid::cell* c1, HexGrid::cell* c2, HexGrid::cell* c3) :
            m_c1(c1), m_c2(c2), m_c3(c3){}
    };

    //
    // расширение абстракции рискованного места
    //
    class RiskSiteEx : public RiskSite{
    public:

        DCTOR_ABS_DEF(RiskSiteEx)

        //сохранение/загрузка 
        virtual void MakeSaveLoad(SavSlot& slot) = 0;
        //учет кол-ва ходов
        virtual bool Decrease() = 0;
        //тестирование
        virtual void TestSmth() = 0;
    };

    //
    // реализация опасного места
    //
    class RiskSiteImp : public RiskSiteEx{
    public:

        RiskSiteImp() : m_fire(0) {}

        RiskSiteImp(const hex_triad_s& triad, float damage) :
            m_duration(GROUND_FIRE_DURATION),
            m_triad(triad), m_fire(0), m_damage(damage)
        {
            LightFire();
            Link2Grid(this);
        }

        ~RiskSiteImp()
        {
            Link2Grid(0);
            if(m_fire) EffectMgr::GetInst()->DestroyConst(m_fire);
        }

        DCTOR_DEF(RiskSiteImp)

        //сохранение/загрузка 
        void MakeSaveLoad(SavSlot& slot)
        {
            m_triad.MakeSaveLoad(slot);

            if(slot.IsSaving()){

                slot << m_damage;
                slot << m_duration;

            }else{

                slot >> m_damage;
                slot >> m_duration;

                LightFire();
                Link2Grid(this);
            }
        }

        //тестирование
        void TestSmth()
        {
            /*
            BBox box;

            box.Box(*m_triad.m_c1, 0.07);
            DirtyLinks::DrawBBox(box, 0xf0f0f0);

            box.Box(*m_triad.m_c2, 0.07);
            DirtyLinks::DrawBBox(box, 0xf0f0f0);

            box.Box(*m_triad.m_c3, 0.07);
            DirtyLinks::DrawBBox(box, 0xf0f0f0);
            
            box.Box(m_triad.CalcCenter(), 0.1);
            DirtyLinks::DrawBBox(box, 0x00ffff);
            */
        }

        bool Decrease() { return --m_duration < 0; }

        float GetEntityFlameDmg() const { return m_damage; }

    private:

        void Link2Grid(RiskSite* site)
        {
            m_triad.m_c1->SetRisk(site);
            m_triad.m_c2->SetRisk(site);
            m_triad.m_c3->SetRisk(site);
        }

        void LightFire()
        {
            m_fire = EffectMgr::GetInst()->MakeConst("ground_flame", m_triad.CalcCenter());
        }

    private:

        float m_damage;
        int   m_duration; 

        unsigned    m_fire;
        hex_triad_s m_triad;
    };

    DCTOR_IMP(RiskSiteImp)

    //
    // реализация опасной зоны на уровне
    //
    class RiskAreaImp : public RiskArea, private GameObserver{
    public:

        typedef std::vector<hex_triad_s> triad_vec_t;
        typedef std::multimap<unsigned, size_t> index_map_t;

        RiskAreaImp() 
        {
            GameEvMessenger::GetInst()->Attach(this, EV_TURN);
        }

        ~RiskAreaImp()
        { 
            Reset();
            GameEvMessenger::GetInst()->Detach(this);
        }

        //сохранить/загрузить опасные зоны уровня
        void MakeSaveLoad(SavSlot& slot)
        {   
            if(slot.IsSaving()){

                for(unsigned int k = 0; k < m_risks.size(); k++){
                    if(RiskSiteEx* site = m_risks[k]){
                        slot << k;
                        DynUtils::SaveObj(slot, site);
                        site->MakeSaveLoad(slot);
                    }
                }

                //признак конца
                slot << -1;

            }else{

                int index;
                slot >> index;

                while(index >= 0){

                    m_risks.push_back(0);
                    
                    DynUtils::LoadObj(slot, m_risks.back());
                    m_risks.back()->MakeSaveLoad(slot);

                    slot >> index;
                }
            }
        }

        //произвести взрыв в заданной точке уровня
        void InsertFlame(const point3& center, float radius, float damage)
        {
            ipnt2_t center_hex = HexUtils::scr2hex(center);

            //проверка на выход за границу hex'вого поля
            if(HexGrid::GetInst()->IsOutOfRange(center_hex))
                return;

            //расчитать поле проходимости для взрыва
            CalcExplosionPassField(center_hex, radius);

            triad_vec_t all, lights;

            //собрать массив всех триад 
            CollectHexTriads(center_hex, &all);

            //сформировать массив горящих триад
            CollectLightTriads(all, CalcNumberOfLighted(radius), &lights);

            //поместить огонь в массив
            for(size_t k = 0; k < lights.size(); Push2RiskArea(lights[k++], damage));
        }

        void TestSmth()
        {
            for(size_t k = 0; k < m_risks.size(); k++){
                if(m_risks[k]) m_risks[k]->TestSmth();
            }
        }

        void Reset()
        {
            for(size_t k = 0; k < m_risks.size(); delete m_risks[k++]);
            m_risks.clear();
        }

    private:

        //расчитать поле проходимости для взрыва
        void CalcExplosionPassField(const ipnt2_t& center, float radius)
        {
            PathUtils::CalcPassField(center, radius, 0);
        }

        //собрать вектор hex'вых триад
        void CollectHexTriads(const ipnt2_t& center, triad_vec_t* vec)
        {
            ipnt2_t pos;

            hex_triad_s triad;

            pnt_vec_t first_front;
            pnt_vec_t second_front;

            HexGrid::cell* ptr;
            HexGrid::cell* cells[HexTraits::num_sides];

            second_front.push_back(center);

            while(second_front.size()){

                first_front.swap(second_front);
                second_front.clear();

                for(size_t k = 0; k < first_front.size(); k++){

                    //занулим массив
                    memset(cells, 0, sizeof(cells));

                    triad.m_c3 = &HexGrid::GetInst()->Get(first_front[k]);

                    int cost = triad.m_c3->GetCost();                    

                    //собрать сведения о направлениях выхода
                    for(int dir = 0; dir < HexTraits::num_sides; dir++){

                        HexUtils::GetFrontPnts0(first_front[k], dir, &pos);

                        //если выходит за границу hex'a 
                        if(HexGrid::GetInst()->IsOutOfRange(pos))
                            continue;

                        ptr = &HexGrid::GetInst()->Get(pos);
                        
                        //собираем следущий фронт
                        if(ptr->GetCost() > cost){
                            cells[dir] = ptr;
                            second_front.push_back(pos);            
                            continue;
                        }
						

                        //собираем вершины триад
                        if(ptr->GetCost() == cost) cells[dir] = ptr;
                    }

                    //если вершина занята огнем то триад она не добавляет
                    if(triad.m_c3->GetRisk()) continue;

                    //соберем массив триад
                    for(int c1 = 0, c2 = c1 + 1; c1 < HexTraits::num_sides; c1++, c2 = (c1 + 1) % HexTraits::num_sides){

                        triad.m_c1 = cells[c1];
                        triad.m_c2 = cells[c2];
                        
                        if(     triad.m_c1 && triad.m_c1->GetRisk() == 0 
                            &&  triad.m_c2 && triad.m_c2->GetRisk() == 0) 
                            vec->push_back(triad);
                    }
                } 
				second_front.erase(std::unique(second_front.begin(), second_front.end()), second_front.end());
            }
        }

        //сформировать вектор триад с разбросом
        void CollectLightTriads(const triad_vec_t& in, int count, triad_vec_t* out)
        {
            int attempts = in.size()*1.5f;

            while(count && attempts)
			{
                attempts--;

                int k = aiRangeRand(in.size());

                if(!DoesIntersectionExist(*out, in[k]))
				{
                    count--;
                    out->push_back(in[k]);
                }
            }
        }
/*
        //существует ли триада в векторе
        bool DoesCongruentTriadExist(const triad_vec_t& in, const index_map_t& map, const hex_triad_s& triad) const
        {
            index_map_t::const_iterator first = map.lower_bound(triad.GetKey()),
                                        last  = map.upper_bound(triad.GetKey());

            while(first != last){

                if(in[first->second].IsCongruent(triad))
                    return true;

                first++; 
            }
              
            return false;
        }
*/

        //есть ли триады с пересечениями
        bool DoesIntersectionExist(const triad_vec_t& vec, const hex_triad_s& triad) const
        {           
            for(size_t k = 0; k < vec.size(); k++){
                if(vec[k].IsIntersects(triad))
                    return true;
            }

            return false;            
        }

        //расчитать кол-во зазженных hex'ов
        int CalcNumberOfLighted(float radius) const
        {
            return 0.8*radius + 1;
        }

        void Push2RiskArea(const hex_triad_s& triad, float damage)
        {
            for(size_t k = 0; k < m_risks.size(); k++){
                if(m_risks[k] == 0){
                    m_risks[k] = new RiskSiteImp(triad, damage);
                    return;
                }
            }

            m_risks.push_back(new RiskSiteImp(triad, damage));
        }

        void Update(subject_t subj, event_t event, info_t info)
        {
            turn_info* ptr = static_cast<turn_info*>(info);

            if(ptr->m_player == PT_PLAYER && ptr->IsBegining()){

                for(size_t k = 0; k < m_risks.size(); k++){

                    if(m_risks[k] && m_risks[k]->Decrease()){
                        delete m_risks[k];
                        m_risks[k] = 0;
                    }
                }
            }
        }

    private:

        typedef std::vector<RiskSiteEx*> risk_vec_t;
        risk_vec_t m_risks;
    };
}

RiskArea* RiskArea::GetInst()
{
    static RiskAreaImp imp;
    return &imp;
}

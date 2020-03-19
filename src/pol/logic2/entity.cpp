#pragma warning(disable:4786)

#include "logicdefs.h"

#include "thing.h"
#include "entity.h"
#include "VisMap.h"
#include "Marker.h"
#include "aiutils.h"
#include "graphic.h"
#include "strategy.h"
#include "sndutils.h"
#include "Spectator.h"
#include "xlsreader.h"
#include "pathutils.h"
#include "aicontext.h"
#include "RankTable.h"
#include "gameobserver.h"
#include "entityfactory.h"
#include "enemydetection.h"
#include "tableutils.h"
#include "DirtyLinks.h"

#include "ExperienceSaver.h"

#include "../Common/Utils/VFileWpr.h"

#include <boost/format.hpp>

DCTOR_IMP(HumanContext)
DCTOR_IMP(TraderContext)
DCTOR_IMP(VehicleContext)

DCTOR_IMP(HumanEntity)
DCTOR_IMP(TraderEntity)
DCTOR_IMP(VehicleEntity)

#define LOGERR_LOG(l)	MLL_MAKE_LOG("spawnerr.log",l)

////=====================================================================================//
////                                bool UnlimitedMoves()                                //
////=====================================================================================//
//bool UnlimitedMoves()
//{
//	EntityPool *ep = EntityPool::GetInst();
//
//	for(EntityPool::iterator i = ep->begin(); i != ep->end(); ++i)
//	{
//		if(EnemyDetector::getInst()->isHeEnemy(&*i)) return false;
//	}
//	return true;
//}

const int HumanContext::m_pack_map[MAX_PACKS] = 
{
	0,  //HPK_NONE
		1,  //HPK_HEAD
		2,  //HPK_BODY
		3,  //HPK_HANDS
		4,  //HPK_LKNEE
		8,  //HPK_RKNEE
		12, //HPK_IMPLANTS
		22, //HPK_BACKPACK
		-1, //HPK_ALL
};

const char* tony_sys_name    = "tony";
const char* hacker_sys_name  = "player"; 
const char* oilrill_sys_name = "oilrill";

EntityValidator* BaseEntity::m_validator;

//======================================================================

namespace{

	const point3 null_shell_value = point3(-100, -100, -100);

	const float human_back_sight_radius = 3.0f;

	const std::string tech_xls = "scripts/logic/tech.txt";
	const char* forces_xls = "scripts/logic/forces.txt";
	const std::string humans_xls = "scripts/logic/people.txt";

	void operator >> (std::istringstream& istr, HumanInfo::human_type& type)
	{
		if(istr.str() == "none")
			type = HumanInfo::HT_NONE;
		else if(istr.str() == "hero")
			type =  HumanInfo::HT_HERO;
		else if(istr.str() == "enemy")
			type = HumanInfo::HT_ENEMY;
		else if(istr.str() == "civilian")
			type = HumanInfo::HT_CIVILIAN;
		else if(istr.str() == "antihero")
			type = HumanInfo::HT_ANTIHERO;
		else if(istr.str() == "questholder")
			type = HumanInfo::HT_QUESTHOLDER;
		else        
			throw CASUS("Str2HumanType: ���������� human_type!");    
	}

	void operator >> (std::istringstream& istr, VehicleInfo::vehicle_size& size)
	{
		if(istr.str() == "tiny")
			size = VS_TINY;
		else if(istr.str() == "huge")
			size = VS_HUGE;
		else if(istr.str() == "small")
			size =  VS_SMALL;
		else if(istr.str() == "large")
			size = VS_LARGE; 
		else
			throw CASUS("Str2VehicleSize: ���������� ������ �������!");    
	}

	void operator >> (std::istringstream& istr, VehicleInfo::vehicle_type& type)
	{
		if(istr.str() =="tech")
			type = VehicleInfo::VT_TECH;
		else if(istr.str()== "robot")
			type = VehicleInfo::VT_ROBOT;
		else
			throw CASUS("Str2VehicleSize: ���������� ��� �������!");    
	}

	void operator >> (std::istringstream& istr, damage_s& dmg) 
	{
		if(istr.str() == "none"){
			dmg.m_val  = 0;
			dmg.m_type = DT_NONE;
			return;
		}

		char comma;
		std::string dmg_type;

		istr >> dmg.m_val >> comma >> dmg_type;
		dmg.m_type = AIUtils::Str2Dmg(dmg_type);
	}

	void operator >> (std::istringstream& istr, ForceInfo::force_type& type)
	{
		if(istr.str() =="goverment")
			type = ForceInfo::FT_GOVERMENT;
		else if(istr.str()== "private")
			type = ForceInfo::FT_PRIVATE;
		else
			throw CASUS("ForceInfoReader: ���������� ��� �����������!");    
	}

	std::istringstream& init(std::istringstream& istr, const std::string& str)
	{
		istr.str(str);
		istr.clear();
		return istr;
	}

	//
	//� ����������� ���������� ����� BaseEntity::Notify
	//
	class notify_obs{
	public:

		notify_obs(BaseEntity* entity, EntityObserver::event_t event, EntityObserver::info_t info = 0) :
		  m_entity(entity), m_info(info), m_event(event) {}

		  ~notify_obs()
		  {
			  m_entity->Notify(m_event, m_info);
		  }

	private:

		BaseEntity* m_entity;

		EntityObserver::info_t  m_info; 
		EntityObserver::event_t m_event;
	};

	//
	// ���� fsend == true, � ����������� ���������� ����� BaseEntity::Notify
	//
	class notify_obs_cond{
	public:

		notify_obs_cond(unsigned fsend, BaseEntity* entity, EntityObserver::event_t event, EntityObserver::info_t info = 0) :
		  m_entity(entity), m_info(info), m_event(event), m_fsend(fsend ? true : false) {}          

		  notify_obs_cond(float fsend, BaseEntity* entity, EntityObserver::event_t event, EntityObserver::info_t info = 0) :
		  m_entity(entity), m_info(info), m_event(event), m_fsend(fsend ? true : false) {}          

		  notify_obs_cond(bool fsend, BaseEntity* entity, EntityObserver::event_t event, EntityObserver::info_t info = 0) :
		  m_entity(entity), m_info(info), m_event(event), m_fsend(fsend) {}

		  ~notify_obs_cond()
		  {
			  if(m_fsend) m_entity->Notify(m_event, m_info);
		  }

	private:

		bool        m_fsend;
		BaseEntity* m_entity;

		EntityObserver::info_t  m_info; 
		EntityObserver::event_t m_event;
	};

	//
	// ���� ������� ��������
	//
	class ThingGiver : public EntityVisitor{
	public:

		ThingGiver(BaseThing* thing, human_pack_type pack) :
		  m_thing(thing), m_pack(pack) {}

		  void Visit(HumanEntity* human)
		  {
			  human->GetEntityContext()->InsertThing(m_pack, m_thing);
		  }

		  void Visit(TraderEntity* trader)
		  {
			  trader->GetEntityContext()->InsertThing(m_thing);
		  }

		  void Visit(VehicleEntity* vehicle)
		  {
			  throw CASUS("ThingTaker: ������� �� ����� ����� �������!");
		  }

	private:

		BaseThing*      m_thing;
		human_pack_type m_pack;
	};

	//
	// ����� �� �������� ����� �������
	//
	class CanTakeThing : public EntityVisitor{
	public:

		CanTakeThing(BaseThing* thing, human_pack_type pack) :
		  m_thing(thing), m_pack(pack), m_result(false) {}

		  void Visit(HumanEntity* human)
		  {
			  m_result = !human->GetEntityContext()->IsFull(m_pack);
		  }

		  void Visit(TraderEntity* trader)
		  {
			  m_result = true;
		  }

		  bool GetResult() const
		  {
			  return m_result;
		  }

	private:

		bool       m_result;
		BaseThing* m_thing;

		human_pack_type m_pack;
	};
}

//======================================================================

//
// ����� �������������� ���������� �������
//
class EntityValidator : private EntityObserver{
public:

	EntityValidator() : m_refs(0) {}

	//�������������� � ��������
	unsigned Attach(BaseEntity* entity)
	{
		entity->Attach(this, EV_INSERT_THING);
		entity->Attach(this, EV_HANDS_PACK_CHANGE);
		return ++m_refs;
	}

	//������������� �� ��������
	unsigned Detach(BaseEntity* entity)
	{
		entity->Detach(this);
		return --m_refs;
	}

private:

	void Update(BaseEntity* entity, event_t event, info_t info)
	{
		if(HumanEntity* human = entity->Cast2Human()){
			ValidateGraphHuman(human);
			ValidateHumanShotType(human);
		}
	}

	void ValidateGraphHuman(HumanEntity* human)
	{
		GraphHuman*   graph   = human->GetGraph();
		HumanContext* context = human->GetEntityContext();

		if(!context->CanSit()) graph->ChangePose(GraphHuman::PT_STAND);
		if(!context->CanRun()) graph->ChangeMoveType(GraphHuman::MT_WALK);
	}

	void ValidateHumanShotType(HumanEntity* human)
	{        
		HumanContext* context = human->GetEntityContext();
		BaseThing*    thing   = context->GetThingInHands();

		if(thing && thing->GetInfo()->IsWeapon()){

			const WeaponInfo* info = static_cast<WeaponThing*>(thing)->GetInfo();

			if(!info->IsAuto() && context->GetShotType() == SHT_AUTOSHOT)
				context->SetShotType(SHT_SNAPSHOT);        
		}
	}

private:

	unsigned m_refs;
};

//======================================================================

static std::string killBoundSpaces(const std::string &s)
{
	std::string r(s);
	size_t pos = 0;
	if((pos = r.find_first_not_of(" ")) != 0)
	{
		r.erase(0,pos);
	}
	if((pos = r.find_last_not_of(" ")) != r.length()-1)
	{
		r.erase(pos+1);
	}
	return r;
}

//=====================================================================================//
//                                class HumanInfoReader                                //
//=====================================================================================//
class HumanInfoReader : public InfoReader<HumanInfoReader>
{
public:

#define POL_HUMAN_INFO_TABLE(P,D,DELIM)								\
	P(m_sys_name,		"sys_name",			REQUIRED, D) DELIM()	\
	P(m_name,			"name",				OPTIONAL, D) DELIM()	\
	P(m_type,			"type",				REQUIRED, D) DELIM()	\
	P(m_health,			"health",			REQUIRED, D) DELIM()	\
	P(m_strength,		"strength",			REQUIRED, D) DELIM()	\
	P(m_dexterity,		"dexterity",		REQUIRED, D) DELIM()	\
	P(m_reaction,		"reaction",			REQUIRED, D) DELIM()	\
	P(m_accuracy,		"accuracy",			REQUIRED, D) DELIM()	\
	P(m_morale,			"morale",			REQUIRED, D) DELIM()	\
	P(m_wisdom,			"wisdom",			REQUIRED, D) DELIM()	\
	P(m_mechanics,		"mechanics",		REQUIRED, D) DELIM()	\
	P(m_sight_radius,	"sight_radius",		REQUIRED, D) DELIM()	\
	P(m_sight_angle,	"sight_angle",		REQUIRED, D) DELIM()	\
	P(m_experience,		"experience",		REQUIRED, D) DELIM()	\
	P(m_description,	"description",		OPTIONAL, D) DELIM()	\
	P(m_animations,		"animations",		REQUIRED, D) DELIM()	\
	P(m_skin,			"skin",				OPTIONAL, D) DELIM()	\
	P(m_suit1,			"Suit1",			OPTIONAL, D) DELIM()	\
	P(m_suit2,			"Suit2",			OPTIONAL, D) DELIM()	\
	P(m_suit3,			"Suit3",			OPTIONAL, D) DELIM()	\
	P(m_suit4,			"Suit4",			OPTIONAL, D) DELIM()	\
	P(m_scuba,			"Scuba",			OPTIONAL, D) DELIM()	\
	P(m_shock_res,		"shock_res",		REQUIRED, D) DELIM()	\
	P(m_electric_res,	"electric_res",		REQUIRED, D) DELIM()	\
	P(m_flame_res,		"flame_res",		REQUIRED, D) DELIM()	\
	P(m_base_levelup,	"base_levelup",		REQUIRED, D) DELIM()	\
	P(m_levelup_rise,	"levelup_rise",		REQUIRED, D) DELIM()	\
	P(m_fibre,			"fibre",			REQUIRED, D) DELIM()	\
	P(m_danger_points,	"danger_points",	OPTIONAL, D) DELIM()	\
	P(m_force,			"force",			REQUIRED, D) DELIM()	\
	P(m_shot_bone,		"shot_bone",		REQUIRED, D) DELIM()	\
	P(m_view_bone,		"view_bone",		OPTIONAL, D) DELIM()	\
	P(m_rank,			"rank",				OPTIONAL, D) DELIM()	\
	P(m_cannot_dismiss,	"cannot_dismiss",	OPTIONAL, D) DELIM()	\
	P(m_can_swap,		"can_swap",			OPTIONAL, D) DELIM()	\
	P(m_ai_min_damage,	"ai_min_damage",	OPTIONAL, D) DELIM()	\
	P(m_ai_min_accuracy,"ai_min_accuracy",	OPTIONAL, D) DELIM()	\
	P(m_can_use_scuba,	"can_use_scuba",	OPTIONAL, D)

	POL_HUMAN_INFO_TABLE(POL_MAKE_DECLARATION,(HumanInfoTable),POL_EMPTY)

	static const std::string &GetTableName()
	{
		return humans_xls;
	}

	static void ReadColumns()
	{
		table::row_type hdr = GetHdr();
		std::ostringstream errout;

		POL_HUMAN_INFO_TABLE(POL_MAKE_INITIALIZATION,(HumanInfoTable),POL_EMPTY)

		if(!errout.str().empty())
		{
			throw CASUS(errout.str());
		}
	}

	static void Read(HumanInfo& info)
	{
		using mll::utils::table;

		int line_count = GetTbl().height();

		//������ ������ ������
		for(int line = 1; line < line_count; ++line)
		{
			//������� ������
			if(m_sys_name[line] == info.m_rid)
			{
				std::istringstream istr;

				// �������� ��-���������
				info.m_aiMinDamage = 50.0f;
				info.m_aiMinAccuracy = 70;
				info.m_rank = 0;
				info.m_danger_points = 0;
				info.m_can_dismiss = true;
				info.m_can_swap = true;
				info.m_can_use_scuba = true;

				info.m_name = PoL::getLocStr("people." + info.m_rid + ".name");//m_name[line];
				init(istr,m_type[line]) >> info.m_human_type;
				init(istr,m_health[line]) >> info.m_health;
				init(istr,m_strength[line]) >> info.m_strength;
				init(istr,m_dexterity[line]) >> info.m_dexterity;
				init(istr,m_reaction[line]) >> info.m_reaction;
				init(istr,m_accuracy[line]) >> info.m_accuracy;
				init(istr,m_morale[line]) >> info.m_morale;
				init(istr,m_wisdom[line]) >> info.m_wisdom;
				init(istr,m_mechanics[line]) >> info.m_mechanics;
				init(istr,m_sight_radius[line]) >> info.m_sight_radius;
				init(istr,m_sight_angle[line]) >> info.m_sight_angle;
				info.m_sight_angle = TORAD(info.m_sight_angle);
				init(istr,m_experience[line]) >> info.m_exp4kill;
				info.m_desc = PoL::getLocStr("people." + info.m_rid + ".description");//m_description[line];
				//if(m_description) info.m_desc = m_description[line];
				info.m_model = m_animations[line];
				if(m_skin) info.m_graph_aux[0] = m_skin[line];
				if(m_suit1) info.m_graph_aux[1] = m_suit1[line];
				if(m_suit2) info.m_graph_aux[2] = m_suit2[line];
				if(m_suit3) info.m_graph_aux[3] = m_suit3[line];
				if(m_suit4) info.m_graph_aux[4] = m_suit4[line];
				if(m_scuba) info.m_graph_aux[5] = m_scuba[line];
				if(m_ai_min_damage) init(istr,m_ai_min_damage[line]) >> info.m_aiMinDamage;
				if(m_ai_min_accuracy) init(istr,m_ai_min_damage[line]) >> info.m_aiMinAccuracy;
				init(istr,m_shock_res[line]) >> info.m_shock_res;
				init(istr,m_electric_res[line]) >> info.m_electric_res;
				init(istr,m_flame_res[line]) >> info.m_flame_res;
				init(istr,m_base_levelup[line]) >> info.m_levelup_base;
				init(istr,m_levelup_rise[line]) >> info.m_levelup_rise;
				init(istr,m_fibre[line]) >> info.m_fibre;
				if(m_danger_points) init(istr,m_danger_points[line]) >> info.m_danger_points;
				info.m_force = ForceInfoArchive::GetInst()->CreateRef(0,m_force[line]);
				if(m_view_bone) info.m_viewBone = m_view_bone[line];
				if(m_rank) init(istr,m_rank[line]) >> info.m_rank;
				if(m_can_use_scuba)
				{
					std::string temp;
					init(istr,m_can_use_scuba[line]) >> temp;
					std::transform(temp.begin(),temp.end(),temp.begin(),std::tolower);
					if(temp != "yes")
					{
						info.m_can_use_scuba = false;
					}
				}

				if(m_can_swap)
				{
					std::string temp;
					temp.resize(m_can_swap[line].size());
					std::transform(m_can_swap[line].begin(),m_can_swap[line].end(),&temp[0],std::tolower);
					info.m_can_swap = (temp == "yes");
				}

				{
					typedef boost::char_separator<char> Sep_t;
					typedef boost::tokenizer<Sep_t> Tok_t;
					Sep_t sep(",;");
					Tok_t tok(m_shot_bone[line],sep);

					std::string *outit = info.m_shotBone;

					for(Tok_t::iterator i = tok.begin(); i != tok.end(); ++i)
					{
						*outit++ = killBoundSpaces(*i);
					}
					info.m_shotBoneCount = outit - info.m_shotBone;
				}

				if(m_cannot_dismiss)
				{
					std::string str = killBoundSpaces(m_cannot_dismiss[line]);
					std::transform(str.begin(),str.end(),str.begin(),std::tolower);
					info.m_can_dismiss = (str != "yes" && str != "1" && str != "true");
				}

				info.m_aiMinDamage *= 1e-2f; // ������� �� 100%

				return;
			}
		}

		LOGERR_LOG("HumanInfoReader: �� ������ profile ��� [" << info.m_rid << "]!\n");
		throw CASUS(("HumanInfoReader: �� ������ profile ��� <" + info.m_rid + ">!").c_str());
	}
};

POL_HUMAN_INFO_TABLE(POL_MAKE_DEFINITION,(HumanInfoReader),POL_EMPTY)

//=====================================================================================//
//            class VehicleInfoReader : public InfoReader<HumanInfoReader>             //
//=====================================================================================//
class VehicleInfoReader : public InfoReader<VehicleInfoReader>
{
public:
#define POL_VEHICLE_INFO_TABLE(P,D,DELIM)									\
	P(m_sys_name,				"sys_name",				REQUIRED, D) DELIM()			\
	P(m_size,					"size",					REQUIRED, D) DELIM()			\
	P(m_type,					"type",					REQUIRED, D) DELIM()			\
	P(m_movepoints,				"movepoints",			REQUIRED, D) DELIM()			\
	P(m_basic_damage,			"basic_damage",			REQUIRED, D) DELIM()			\
	P(m_additional_damage,		"additional_damage",	REQUIRED, D) DELIM()			\
	P(m_range,					"range",				REQUIRED, D) DELIM()			\
	P(m_radius,					"radius",				REQUIRED, D) DELIM()			\
	P(m_accuracy,				"accuracy",				REQUIRED, D) DELIM()			\
	P(m_shots,					"shots",				REQUIRED, D) DELIM()			\
	P(m_quality,				"quality",				REQUIRED, D) DELIM()			\
	P(m_mp_shot,				"mp_shot",				REQUIRED, D) DELIM()			\
	P(m_skin,					"skin",					REQUIRED, D) DELIM()			\
	P(m_flash_effect,			"flash_effect",			REQUIRED, D) DELIM()			\
	P(m_shot_sound,				"shot_sound",			REQUIRED, D) DELIM()			\
	P(m_trace_effect,			"trace_effect",			REQUIRED, D) DELIM()			\
	P(m_hit_effect,				"hit_effect",			REQUIRED, D) DELIM()			\
	P(m_move_sound,				"move_sound",			REQUIRED, D) DELIM()			\
	P(m_damage_sound,			"damage_sound",			REQUIRED, D) DELIM()			\
	P(m_explode_sound,			"explode_sound",		REQUIRED, D) DELIM()			\
	P(m_engine_sound,			"engine_sound",			REQUIRED, D) DELIM()			\
	P(m_capacity,				"capacity",				REQUIRED, D) DELIM()			\
	P(m_dexterity,				"dexterity",			REQUIRED, D) DELIM()			\
	P(m_reaction,				"reaction",				REQUIRED, D) DELIM()			\
	P(m_sight_radius,			"sight_radius",			REQUIRED, D) DELIM()			\
	P(m_sight_angle,			"sight_angle",			REQUIRED, D) DELIM()			\
	P(m_drive,					"drive",				REQUIRED, D) DELIM()			\
	P(m_intelligence,			"intelligence",			REQUIRED, D) DELIM()			\
	P(m_armor,					"armor",				REQUIRED, D) DELIM()			\
	P(m_health,					"health",				REQUIRED, D) DELIM()			\
	P(m_expirience_for_kill,	"expirience_for_kill",	REQUIRED, D) DELIM()			\
	P(m_expirience_for_hack,	"expirience_for_hack",	REQUIRED, D) DELIM()			\
	P(m_danger_points,			"danger_points",		REQUIRED, D) DELIM()			\
	P(m_shell,					"shell",				REQUIRED, D) DELIM()			\
	P(m_weapon_icon,			"weapon_icon",			REQUIRED, D) DELIM()			\
	P(m_death_damage,			"death_damage",			REQUIRED, D) DELIM()			\
	P(m_death_radius,			"death_radius",			REQUIRED, D) DELIM()			\
	P(m_death_effect,			"death_effect",			REQUIRED, D) DELIM()			\
	P(m_shells_effect,			"shells_effect",		REQUIRED, D) DELIM()			\
	P(m_shot_bone,				"shot_bone",			REQUIRED, D) DELIM()			\
	P(m_view_bone,				"view_bone",			REQUIRED, D) DELIM()			\
	P(m_lazy_sound,				"lazy_sound",			REQUIRED, D) DELIM()			\
	P(m_fall_off,				"fall_off",				REQUIRED, D) DELIM()			\
	P(m_can_swap,				"can_swap",				OPTIONAL, D) DELIM()			\
	P(m_can_use,				"can_use",				OPTIONAL, D)

	POL_VEHICLE_INFO_TABLE(POL_MAKE_DECLARATION,(VehicleInfoTable),POL_EMPTY)

	static const std::string &GetTableName()
	{
		return tech_xls;
	}

	static void ReadColumns()
	{
		table::row_type hdr = GetHdr();
		std::ostringstream errout;

		POL_VEHICLE_INFO_TABLE(POL_MAKE_INITIALIZATION,(VehicleInfoTable),POL_EMPTY)

		if(!errout.str().empty())
		{
			throw CASUS(errout.str());
		}
	}

	static void Read(VehicleInfo& info)
	{
		using mll::utils::table;

		int line_count = GetTbl().height();

		//������ ������ ������
		for(int line = 1; line < line_count; ++line)
		{
			//������� ������
			if(m_sys_name[line] == info.m_rid)
			{
				std::string str;
				std::istringstream istr;

				info.m_can_swap = true;
				info.m_can_use = true;

				info.m_name = PoL::getLocStr("tech." + info.m_rid + ".name");
				init(istr, m_size[line]) >> info.m_vehicle_size;
				init(istr, m_type[line]) >> info.m_vehicle_type;
				init(istr, m_range[line]) >> info.m_range;
				init(istr, m_movepoints[line]) >> info.m_movepnts;
				init(istr, m_basic_damage[line]) >> info.m_bdmg;
				init(istr, m_additional_damage[line]) >> info.m_admg;
				init(istr, m_radius[line]) >> info.m_dmg_radius;
				init(istr, m_accuracy[line]) >> info.m_accuracy;
				init(istr, m_shots[line]) >> info.m_ammo_count;
				init(istr, m_quality[line]) >> info.m_quality;
				init(istr, m_mp_shot[line]) >> info.m_mp4shot;
				info.m_model = m_skin[line];
				info.m_flash_effect = m_flash_effect[line];
				info.m_shot_sound = m_shot_sound[line];
				info.m_lazy_sound = m_lazy_sound[line];
				info.m_trace_effect = m_trace_effect[line];
				info.m_hit_effect = m_hit_effect[line];
				info.m_move_sound = m_move_sound[line];
				info.m_damage_sound = m_damage_sound[line];
				info.m_explode_sound = m_explode_sound[line];
				info.m_engine_sound = m_engine_sound[line];
				init(istr, m_capacity[line]) >> info.m_capacity;
				init(istr, m_dexterity[line]) >> info.m_dexterity;
				init(istr, m_reaction[line]) >> info.m_reaction;
				init(istr, m_sight_radius[line]) >> info.m_sight_radius;
				init(istr, m_sight_angle[line]) >> info.m_sight_angle;
				info.m_sight_angle = TORAD(info.m_sight_angle);
				init(istr, m_drive[line]) >> info.m_mechanics;
				init(istr, m_intelligence[line]) >> info.m_wisdom;
				info.m_armor_type = AIUtils::Str2Dmg(m_armor[line]);
				init(istr, m_health[line]) >> info.m_health;
				init(istr, m_expirience_for_kill[line]) >> info.m_exp4kill;
				init(istr, m_expirience_for_hack[line]) >> info.m_exp4hack;
				info.m_desc = PoL::getLocStr("tech." + info.m_rid + ".description");
				init(istr, m_danger_points[line]) >> info.m_danger_points;
				info.m_ammo_info = PoL::getLocStr("tech." + info.m_rid + ".ammo_info");
				info.m_weapon_icon = m_weapon_icon[line];
				init(istr, m_death_damage[line]) >> info.m_death_dmg;
				init(istr, m_death_radius[line]) >> info.m_death_radius;
				info.m_death_effect = m_death_effect[line];
				info.m_shells_effect = m_shells_effect[line];
				init(istr, m_fall_off[line]) >> info.m_fall_off;
				info.m_viewBone = m_view_bone[line];

				if(m_can_swap)
				{
					str = m_can_swap[line];
					std::transform(str.begin(),str.end(),str.begin(),std::tolower);
					info.m_can_swap = (str == "yes");
				}

				if(m_can_use)
				{
					str = m_can_use[line];
					std::transform(str.begin(),str.end(),str.begin(),std::tolower);
					info.m_can_use = (str == "yes");
				}

				{
					std::string bones = m_shot_bone[line];

					typedef boost::char_separator<char> Sep_t;
					typedef boost::tokenizer<Sep_t> Tok_t;
					Sep_t sep(",;");
					Tok_t tok(bones,sep);

					std::string *outit = info.m_shotBone;

					for(Tok_t::iterator i = tok.begin(); i != tok.end(); ++i)
					{
						*outit++ = killBoundSpaces(*i);
					}
					info.m_shotBoneCount = outit - info.m_shotBone;
				}

				{
					char ch;
					str = m_shell[line];

					if(str == "-")
					{
						info.m_shell = null_shell_value;
					}
					else
					{
						init(istr, str) >> info.m_shell.x >> ch >> info.m_shell.y >> ch >> info.m_shell.z;
					}
				}
				return;
			}
		}

		LOGERR_LOG("VehicleInfoReader: �� ������ profile ��� [" << info.m_rid << "]!\n");
		throw CASUS(("VehicleInfoReader: �� ������ profile ��� <" + info.m_rid + ">!").c_str());

		//TxtFilePtr txt(tech_xls);

		//static bool first_time = true;
		//if(first_time)
		//{
		//	txt.ReadColumns(m_columns, m_columns + MAX_COLUMNS);
		//	first_time = false;
		//}

		//std::string str;

		////������ ������ ������
		//for(int line = 0; line < txt->SizeY(); line++)
		//{
		//	txt->GetCell(line, m_columns[VCT_SYSNAME].m_index, str);

		//	unsigned int canSwapIdx = 0;
		//	const bool haveCanSwap = txt->FindInRow("can_swap",0,&canSwapIdx);
  //          			
		//	//������� ������
		//	if(str == info.m_rid)
		//	{
		//		std::istringstream istr;

		//		info.m_name = PoL::getLocStr("tech." + info.m_rid + ".name");
		//		//txt->GetCell(line, m_columns[VCT_NAME].m_index, info.m_name);

		//		txt->GetCell(line, m_columns[VCT_SIZE].m_index, str);
		//		init(istr, str) >> info.m_vehicle_size;

		//		txt->GetCell(line, m_columns[VCT_TYPE].m_index, str);
		//		init(istr, str) >> info.m_vehicle_type;

		//		txt->GetCell(line, m_columns[VCT_RANGE].m_index, str);
		//		init(istr, str) >> info.m_range; 

		//		txt->GetCell(line, m_columns[VCT_MOVEPNTS].m_index, str);
		//		init(istr, str) >> info.m_movepnts;

		//		txt->GetCell(line, m_columns[VCT_BASIC_DMG].m_index, str);
		//		init(istr, str) >> info.m_bdmg;

		//		txt->GetCell(line, m_columns[VCT_ADDITIONAL_DMG].m_index, str);
		//		init(istr, str) >> info.m_admg;

		//		txt->GetCell(line, m_columns[VCT_DMG_RADIUS].m_index, str);
		//		init(istr, str) >> info.m_dmg_radius;

		//		txt->GetCell(line, m_columns[VCT_ACCURACY].m_index, str);
		//		init(istr, str) >> info.m_accuracy;

		//		txt->GetCell(line, m_columns[VCT_AMMO_COUNT].m_index, str);
		//		init(istr, str) >> info.m_ammo_count;

		//		txt->GetCell(line, m_columns[VCT_QUALITY].m_index, str);
		//		init(istr, str) >> info.m_quality;

		//		txt->GetCell(line, m_columns[VCT_MP4SHOT].m_index, str);
		//		init(istr, str) >> info.m_mp4shot;

		//		txt->GetCell(line, m_columns[VCT_MODEL].m_index, info.m_model);
		//		txt->GetCell(line, m_columns[VCT_FLASH_EFFECT].m_index, info.m_flash_effect);
		//		txt->GetCell(line, m_columns[VCT_SHOT_SOUND].m_index, info.m_shot_sound);
		//		txt->GetCell(line, m_columns[VCT_LAZY_SOUND].m_index, info.m_lazy_sound);
		//		txt->GetCell(line, m_columns[VCT_TRACE_EFFECT].m_index, info.m_trace_effect);
		//		txt->GetCell(line, m_columns[VCT_HIT_EFFECT].m_index, info.m_hit_effect);
		//		txt->GetCell(line, m_columns[VCT_MOVE_SOUND].m_index, info.m_move_sound);
		//		txt->GetCell(line, m_columns[VCT_DAMAGE_SOUND].m_index, info.m_damage_sound);
		//		txt->GetCell(line, m_columns[VCT_EXPLODE_SOUND].m_index, info.m_explode_sound);
		//		txt->GetCell(line, m_columns[VCT_ENGINE_SOUND].m_index, info.m_engine_sound);

		//		txt->GetCell(line, m_columns[VCT_CAPACITY].m_index, str);
		//		init(istr, str) >> info.m_capacity;

		//		txt->GetCell(line, m_columns[VCT_DEXTERITY].m_index, str);
		//		init(istr, str) >> info.m_dexterity;

		//		txt->GetCell(line, m_columns[VCT_REACTION].m_index, str);
		//		init(istr, str) >> info.m_reaction;

		//		txt->GetCell(line, m_columns[VCT_SIGHT_RADIUS].m_index, str);
		//		init(istr, str) >> info.m_sight_radius;

		//		txt->GetCell(line, m_columns[VCT_SIGHT_ANGLE].m_index, str);
		//		init(istr, str) >> info.m_sight_angle;
		//		info.m_sight_angle = TORAD(info.m_sight_angle);

		//		txt->GetCell(line, m_columns[VCT_MECHANICS].m_index, str);
		//		init(istr, str) >> info.m_mechanics;

		//		txt->GetCell(line, m_columns[VCT_WISDOM].m_index, str);
		//		init(istr, str) >> info.m_wisdom;

		//		txt->GetCell(line, m_columns[VCT_ARMOR].m_index, str);
		//		info.m_armor_type = AIUtils::Str2Dmg(str);

		//		txt->GetCell(line, m_columns[VCT_HEALTH].m_index, str);
		//		init(istr, str) >> info.m_health;

		//		txt->GetCell(line, m_columns[VCT_EXP4KILL].m_index, str);
		//		init(istr, str) >> info.m_exp4kill;

		//		txt->GetCell(line, m_columns[VCT_EXP4HACK].m_index, str);
		//		init(istr, str) >> info.m_exp4hack;

		//		info.m_desc = PoL::getLocStr("tech." + info.m_rid + ".description");
		//		//txt->GetCell(line, m_columns[VCT_DESC].m_index, info.m_desc);

		//		txt->GetCell(line, m_columns[VCT_DANGER_POINTS].m_index, str);
		//		init(istr, str) >> info.m_danger_points;

		//		info.m_ammo_info = PoL::getLocStr("tech." + info.m_rid + ".ammo_info");
		//		//txt->GetCell(line, m_columns[VCT_AMMO_INFO].m_index, info.m_ammo_info);
		//		txt->GetCell(line, m_columns[VCT_WEAPON_ICON].m_index, info.m_weapon_icon);

		//		txt->GetCell(line, m_columns[VCT_DEATH_DAMAGE].m_index, str);
		//		init(istr, str) >> info.m_death_dmg;

		//		txt->GetCell(line, m_columns[VCT_DEATH_RADIUS].m_index, str);
		//		init(istr, str) >> info.m_death_radius;

		//		txt->GetCell(line, m_columns[VCT_DEATH_EFFECT].m_index, info.m_death_effect);

		//		txt->GetCell(line, m_columns[VCT_DEATH_EFFECT].m_index, info.m_death_effect);
		//		txt->GetCell(line, m_columns[VCT_SHELLS_EFFECT].m_index, info.m_shells_effect);

		//		txt->GetCell(line, m_columns[VCT_FALL_OFF].m_index, str);
		//		init(istr, str) >> info.m_fall_off;

		//		txt->GetCell(line, m_columns[VCT_VIEWBONE].m_index, info.m_viewBone);

		//		info.m_can_swap = true;
		//		if(haveCanSwap)
		//		{
		//			txt->GetCell(line,canSwapIdx,str);
		//			std::transform(str.begin(),str.end(),str.begin(),std::tolower);
		//			info.m_can_swap = (str == "yes");
		//		}

		//		{
		//			std::string bones;
		//			txt->GetCell(line, m_columns[VCT_SHOTBONE].m_index, bones);

		//			typedef boost::char_separator<char> Sep_t;
		//			typedef boost::tokenizer<Sep_t> Tok_t;
		//			Sep_t sep(",;");
		//			Tok_t tok(bones,sep);

		//			std::string *outit = info.m_shotBone;

		//			for(Tok_t::iterator i = tok.begin(); i != tok.end(); ++i)
		//			{
		//				*outit++ = killBoundSpaces(*i);
		//			}
		//			info.m_shotBoneCount = outit - info.m_shotBone;
		//		}

		//		char ch;

		//		txt->GetCell(line, m_columns[VCT_SHELL].m_index, str);
		//		if(str == "-") 
		//			info.m_shell = null_shell_value;
		//		else
		//			init(istr, str) >> info.m_shell.x >> ch >> info.m_shell.y >> ch >> info.m_shell.z;

		//		return;
		//	}
		//}

		//throw CASUS(("VehicleInfoReader: �� ������ profile ��� <" + info.m_rid + ">!").c_str());
	}

private:

	//enum column_type
	//{
	//	VCT_SYSNAME,
	//	//VCT_NAME,
	//	VCT_SIZE,
	//	VCT_TYPE,
	//	VCT_MOVEPNTS,
	//	VCT_BASIC_DMG,
	//	VCT_ADDITIONAL_DMG,
	//	VCT_RANGE,
	//	VCT_DMG_RADIUS,
	//	VCT_ACCURACY,
	//	VCT_AMMO_COUNT,
	//	VCT_QUALITY,
	//	VCT_MP4SHOT,
	//	VCT_MODEL,
	//	VCT_FLASH_EFFECT,
	//	VCT_SHOT_SOUND,
	//	VCT_TRACE_EFFECT,
	//	VCT_HIT_EFFECT,
	//	VCT_MOVE_SOUND,
	//	VCT_DAMAGE_SOUND,
	//	VCT_EXPLODE_SOUND,
	//	VCT_ENGINE_SOUND,
	//	VCT_CAPACITY,
	//	VCT_DEXTERITY,
	//	VCT_REACTION,
	//	VCT_SIGHT_RADIUS,
	//	VCT_SIGHT_ANGLE,
	//	VCT_MECHANICS,
	//	VCT_WISDOM,
	//	VCT_ARMOR,
	//	VCT_HEALTH,
	//	VCT_EXP4KILL,
	//	VCT_EXP4HACK,
	//	//VCT_DESC,
	//	VCT_DANGER_POINTS,
	//	VCT_SHELL,
	//	VCT_WEAPON_ICON,
	//	//VCT_AMMO_INFO,
	//	VCT_DEATH_DAMAGE,
	//	VCT_DEATH_RADIUS,
	//	VCT_DEATH_EFFECT,
	//	VCT_SHELLS_EFFECT,
	//	VCT_SHOTBONE,
	//	VCT_VIEWBONE,
	//	VCT_LAZY_SOUND,
	//	VCT_FALL_OFF,

	//	MAX_COLUMNS
	//};

	//static column m_columns[MAX_COLUMNS];
};

POL_VEHICLE_INFO_TABLE(POL_MAKE_DEFINITION,(VehicleInfoReader),POL_EMPTY);

//column VehicleInfoReader::m_columns[MAX_COLUMNS] = 
//{
//	column("sys_name",      VCT_SYSNAME),
//	//column("name",          VCT_NAME),
//	column("size",          VCT_SIZE),
//	column("type",          VCT_TYPE),
//	column("movepoints",    VCT_MOVEPNTS),
//	column("basic_damage",       VCT_BASIC_DMG),
//	column("additional_damage",  VCT_ADDITIONAL_DMG),
//	column("range",         VCT_RANGE),
//	column("radius",        VCT_DMG_RADIUS),
//	column("accuracy",      VCT_ACCURACY),
//	column("shots",         VCT_AMMO_COUNT),
//	column("quality",       VCT_QUALITY),
//	column("mp_shot",       VCT_MP4SHOT),
//	column("skin",          VCT_MODEL),
//	column("flash_effect",  VCT_FLASH_EFFECT),
//	column("shot_sound",    VCT_SHOT_SOUND),
//	column("trace_effect",  VCT_TRACE_EFFECT),
//	column("hit_effect",    VCT_HIT_EFFECT),
//	column("move_sound",    VCT_MOVE_SOUND),
//	column("damage_sound",  VCT_DAMAGE_SOUND),
//	column("explode_sound", VCT_EXPLODE_SOUND),
//	column("engine_sound",  VCT_ENGINE_SOUND),
//	column("capacity",      VCT_CAPACITY),
//	column("dexterity",     VCT_DEXTERITY),
//	column("reaction",      VCT_REACTION),
//	column("sight_radius",  VCT_SIGHT_RADIUS),
//	column("sight_angle",   VCT_SIGHT_ANGLE),
//	column("drive",         VCT_MECHANICS),
//	column("intelligence",  VCT_WISDOM),
//	column("armor",         VCT_ARMOR),
//	column("health",        VCT_HEALTH),
//	column("expirience_for_kill", VCT_EXP4KILL),
//	column("expirience_for_hack", VCT_EXP4HACK),
//	//column("description",   VCT_DESC),
//	column("danger_points", VCT_DANGER_POINTS),
//	column("shell",         VCT_SHELL), 
//	column("weapon_icon",   VCT_WEAPON_ICON), 
//	//column("ammo_info",     VCT_AMMO_INFO), 
//	column("death_damage",  VCT_DEATH_DAMAGE), 
//	column("death_radius",  VCT_DEATH_RADIUS), 
//	column("death_effect",  VCT_DEATH_EFFECT), 
//	column("shells_effect", VCT_SHELLS_EFFECT),
//	column("shot_bone",		VCT_SHOTBONE),
//	column("view_bone",		VCT_VIEWBONE),
//	column("lazy_sound",    VCT_LAZY_SOUND),
//	column("fall_off",		VCT_FALL_OFF)
//};

//======================================================================

class ForceInfoReader{
public:

	static void Read(ForceInfo& info)
	{
		TxtFilePtr txt(forces_xls);

		static bool first_time = true;
		if(first_time){
			txt.ReadColumns(m_columns, m_columns + MAX_COLUMNS);
			first_time = false;
		}

		std::string str;

		//������ ������ ������
		for(int line = 0; line < txt->SizeY(); line++){

			txt->GetCell(line, m_columns[FCT_SYSNAME].m_index, str);

			//������� ������
			if(str == info.m_rid)
			{
				info.m_name = PoL::getLocStr("forces." + info.m_rid + ".name");
				info.m_desc = PoL::getLocStr("forces." + info.m_rid + ".description");
				//txt->GetCell(line, m_columns[FCT_NAME].m_index, info.m_name);
				//txt->GetCell(line, m_columns[FCT_DESC].m_index, info.m_desc);

				std::istringstream istr;

				txt->GetCell(line, m_columns[FCT_TYPE].m_index, str);
				init(istr, str) >> info.m_type;

				return;
			}
		}

		throw CASUS(("ForceInfoReader: �� ������ profile ��� <" + info.m_rid + ">!").c_str());
	}

private:

	enum column_type{
		FCT_SYSNAME,
		//FCT_NAME,
		FCT_TYPE,
		//FCT_DESC,

		MAX_COLUMNS,
	};

	static column m_columns[MAX_COLUMNS];
};

column ForceInfoReader::m_columns[MAX_COLUMNS] = 
{
	column("sys_name",    FCT_SYSNAME),
		//column("name",        FCT_NAME),
		column("type",        FCT_TYPE),
		//column("description", FCT_DESC),
};

//======================================================================

ForceInfo::ForceInfo(const rid_t& rid) :
	m_rid(rid)
	{
		ForceInfoReader::Read(*this);
	}

	ForceInfo::~ForceInfo()
	{
	}

	int ForceInfo::GetType() const
	{
		return 0;
	}

	ForceInfo::force_type ForceInfo::GetForceType() const
	{
		return m_type;
	}

	const rid_t& ForceInfo::GetRID() const
	{
		return m_rid;
	}

	const std::string& ForceInfo::GetName() const
	{
		return m_name;
	}

	const std::string& ForceInfo::GetDesc() const
	{
		return m_desc;
	}

	//======================================================================

	ForceInfo* ForceInfoFactory::Create(int type, const rid_t& rid)
	{
		return new ForceInfo(rid);
	}

	//======================================================================

	EntityInfo::EntityInfo(const rid_t& rid, entity_type type):
	m_rid(rid), m_type(type)
	{
	}

	EntityInfo::~EntityInfo()
	{
	}

	const rid_t& EntityInfo::GetRID() const
	{
		return m_rid;
	}

	const std::string& EntityInfo::GetName() const
	{
		return m_name;
	}

	const std::string& EntityInfo::GetDesc() const
	{
		return m_desc;
	}

	entity_type EntityInfo::GetType() const
	{
		return m_type;
	}

	//======================================================================

	HumanInfo::HumanInfo(const rid_t& rid) : 
	EntityInfo(rid, ET_HUMAN)
	{
		HumanInfoReader::Read(*this);
	}

	HumanInfo::~HumanInfo()
	{
		ForceInfoArchive::GetInst()->DeleteRef(m_force);
	}

	float HumanInfo::GetFibre() const
	{
		return m_fibre;
	}

	const ForceInfo* HumanInfo::GetForce() const
	{
		return m_force;
	}

	int HumanInfo::GetExp4Kill() const
	{
		return m_exp4kill;
	}

	int HumanInfo::GetLevelupBase() const
	{
		return m_levelup_base;
	}

	int HumanInfo::GetLevelupRise() const
	{
		return m_levelup_rise;
	}

	int HumanInfo::GetMorale() const
	{
		return m_morale;
	}

	int HumanInfo::GetWisdom() const
	{
		return m_wisdom;
	}

	int HumanInfo::GetHealth() const
	{
		return m_health;
	}

	int HumanInfo::GetStrength() const
	{
		return m_strength;
	}

	int HumanInfo::GetReaction() const
	{
		return m_reaction;
	}

	int HumanInfo::GetAccuracy() const
	{
		return m_accuracy;
	}

	int HumanInfo::GetMechanics() const
	{
		return m_mechanics;
	}

	int HumanInfo::GetDexterity() const
	{
		return m_dexterity;
	}

	int HumanInfo::GetShockRes() const
	{
		return m_shock_res;
	}

	int HumanInfo::GetFlameRes() const
	{
		return m_flame_res;
	}

	int HumanInfo::GetElectricRes() const
	{
		return m_electric_res;
	}

	int HumanInfo::GetDangerPoints() const
	{
		return m_danger_points;
	}

	const std::string&  HumanInfo::GetModel() const
	{
		return m_model;
	}

	const std::string* HumanInfo::GetSkins() const
	{
		return m_graph_aux;
	}

	float HumanInfo::GetSightAngle() const
	{
		return m_sight_angle;
	}

	float HumanInfo::GetSightRadius() const
	{
		return m_sight_radius;
	}

	bool HumanInfo::IsHacker() const
	{
		return m_rid == hacker_sys_name;
	}

	bool HumanInfo::CanUseScuba() const
	{
		return m_can_use_scuba;
	}

	//======================================================================

	VehicleInfo::VehicleInfo(const rid_t& rid):
	EntityInfo(rid, ET_VEHICLE)
	{
		VehicleInfoReader::Read(*this);
	}

	bool VehicleInfo::IsTinySize() const
	{
		return m_vehicle_size == VS_TINY;
	}

	bool VehicleInfo::IsHugeSize() const
	{
		return m_vehicle_size == VS_HUGE;
	}

	bool VehicleInfo::IsSmallSize() const
	{
		return m_vehicle_size == VS_SMALL;
	}

	bool VehicleInfo::IsLargeSize() const
	{
		return m_vehicle_size == VS_LARGE;
	}

	damage_type VehicleInfo::GetArmorType() const
	{
		return m_armor_type;
	}

	VehicleInfo::vehicle_type VehicleInfo::GetVehicleType() const
	{
		return m_vehicle_type;
	}

	bool VehicleInfo::HaveShellOutlet() const
	{
		return m_shell != null_shell_value;
	}

	const point3& VehicleInfo::GetShellOutlet() const
	{
		return m_shell;
	}

	bool VehicleInfo::IsTech() const
	{
		return m_vehicle_type == VT_TECH;
	}

	bool VehicleInfo::IsRobot() const
	{
		return m_vehicle_type == VT_ROBOT;
	}

	int VehicleInfo::GetRange() const
	{
		return m_range;
	}

	int VehicleInfo::GetWisdom() const
	{
		return m_wisdom;
	}

	int VehicleInfo::GetHealth() const
	{
		return m_health;
	}

	int VehicleInfo::GetQuality() const
	{
		return m_quality;
	}

	int VehicleInfo::GetMp4Shot() const
	{
		return m_mp4shot;
	}

	int VehicleInfo::GetCapacity() const
	{
		return m_capacity;
	}

	int VehicleInfo::GetReaction() const
	{
		return m_reaction;
	}

	int VehicleInfo::GetAccuracy() const
	{
		return m_accuracy;
	}

	int VehicleInfo::GetMovepnts() const
	{
		return m_movepnts;
	}

	int VehicleInfo::GetDexterity() const
	{
		return m_dexterity;
	}

	int VehicleInfo::GetMechanics() const
	{
		return m_mechanics;
	}

	int VehicleInfo::GetDangerPoints() const
	{
		return m_danger_points;
	}

	int VehicleInfo::GetAmmoCount() const
	{
		return m_ammo_count;
	}

	int VehicleInfo::GetExp4Kill() const
	{
		return m_exp4kill;
	}

	int VehicleInfo::GetExp4Hack() const
	{
		return m_exp4hack;
	}

	float VehicleInfo::GetDmgRadius() const
	{
		return m_dmg_radius;
	}

	float VehicleInfo::GetSightAngle() const
	{
		return m_sight_angle;
	}

	float VehicleInfo::GetSightRadius() const
	{
		return m_sight_radius;
	}

	const damage_s& VehicleInfo::GetBDmg() const
	{
		return m_bdmg;
	}

	const damage_s& VehicleInfo::GetADmg() const
	{
		return m_admg;
	}

	const std::string& VehicleInfo::GetModel() const
	{
		return m_model;
	}

	const std::string& VehicleInfo::GetShotSound() const
	{
		return m_shot_sound;
	}

	const std::string& VehicleInfo::GetMoveSound() const
	{
		return m_move_sound;
	}

	const std::string& VehicleInfo::GetLazySound() const
	{
		return m_lazy_sound;
	}

	const std::string& VehicleInfo::GetDamageSound() const
	{
		return m_damage_sound;
	}

	const std::string& VehicleInfo::GetEngineSound() const
	{
		return m_engine_sound;
	}

	const std::string& VehicleInfo::GetExplodeSound() const
	{
		return m_explode_sound;
	}

	const std::string& VehicleInfo::GetHitEffect() const
	{
		return m_hit_effect;
	}

	const std::string& VehicleInfo::GetTraceEffect() const
	{
		return m_trace_effect;
	}

	const std::string& VehicleInfo::GetFlashEffect() const
	{
		return m_flash_effect;
	}

	const std::string& VehicleInfo::GetAmmoInfo() const
	{
		return m_ammo_info;
	}

	const std::string& VehicleInfo::GetWeaponIcon() const
	{
		return m_weapon_icon;
	}

	float VehicleInfo::GetDeathRadius() const
	{
		return m_death_radius;
	}

	const damage_s& VehicleInfo::GetDeathDmg() const
	{
		return m_death_dmg;
	}

	const std::string& VehicleInfo::GetDeathEffect() const
	{
		return m_death_effect;
	}

	const std::string& VehicleInfo::GetShellsEffect() const
	{
		return m_shells_effect;
	}

	//======================================================================

	EntityInfo* EntityInfoFactory::Create(entity_type type, const rid_t& rid)
	{
		switch(type){
case ET_HUMAN: return new HumanInfo(rid);
case ET_TRADER: return new TraderInfo(rid);
case ET_VEHICLE: return new VehicleInfo(rid);
		};

		throw CASUS("EntityInfoFactory: ������� �������� ���������� ��� ������������ ��������!");
	}

	//======================================================================

	BaseEntity::BaseEntity(entity_type type, const rid_t& rid):
	m_flags(type|EA_NOT_INCREW|PT_NONE), m_graph(0), m_entity_context(0), m_info(0), m_ai_context(0), m_eid(0)
	{
		if(rid.size()) m_info = EntityInfoArchive::GetInst()->CreateRef(type, rid);

		//������� �������� ����������� ���� ��� ��� ���
		if(m_validator == 0) m_validator = new EntityValidator();

		//������������� � ��������� �����������
		m_validator->Attach(this);
	}

	BaseEntity::~BaseEntity()
	{
		//������������ �� ��������� �����������
		if(m_validator->Detach(this) == 0){
			delete m_validator;
			m_validator = 0;
		}

		if(m_info) EntityInfoArchive::GetInst()->DeleteRef(m_info);

		delete m_graph;
		delete m_ai_context;
		delete m_entity_context;
	}

	void BaseEntity::MakeSaveLoad(SavSlot& slot)
	{
		if(slot.IsSaving()){

			slot << m_eid;
			slot << m_flags;

			slot << (m_info != 0);
			if(m_info){
				slot << m_info->GetRID();
				slot << static_cast<int>(m_info->GetType()); 
			}

			slot << (m_ai_context != 0);
			if(m_ai_context){
				DynUtils::SaveObj(slot, m_ai_context);
				m_ai_context->MakeSaveLoad(slot);
			}

			slot << (m_entity_context != 0);
			if(m_entity_context){
				DynUtils::SaveObj(slot, m_entity_context);
				m_entity_context->MakeSaveLoad(slot);
			}

			slot << (m_graph != 0);
			if(m_graph){
				DynUtils::SaveObj(slot, m_graph);
				m_graph->MakeSaveLoad(slot);
			}

		}else{

			slot >> m_eid;
			slot >> m_flags;

			bool flag;

			slot >> flag;
			if(flag){

				rid_t rid; slot >> rid;
				int   tmp; slot >> tmp;

				m_info = EntityInfoArchive::GetInst()->CreateRef(static_cast<entity_type>(tmp), rid);
			}

			slot >> flag;
			if(flag){
				DynUtils::LoadObj(slot, m_ai_context);
				m_ai_context->MakeSaveLoad(slot);
			}

			slot >> flag;
			if(flag){
				DynUtils::LoadObj(slot, m_entity_context);
				m_entity_context->MakeSaveLoad(slot);
			}

			slot >> flag;
			if(flag){
				DynUtils::LoadObj(slot, m_graph);
				m_graph->MakeSaveLoad(slot);
			}
		}
	}

	void BaseEntity::Detach(EntityObserver* observer)
	{
		m_observers.Detach(observer);
	}

	void BaseEntity::Attach(EntityObserver* observer, EntityObserver::event_t event)
	{
		m_observers.Attach(observer, event);
	}

	void BaseEntity::Notify(EntityObserver::event_t event, EntityObserver::info_t info)
	{
		m_observers.Notify(this, event, info);
	}

	void BaseEntity::SetGraph(GraphEntity* graph)
	{
		delete m_graph;
		m_graph = graph;
	}

	void BaseEntity::SetEntityContext(EntityContext* context)
	{
		delete m_entity_context;
		m_entity_context = context;
	}

	HumanEntity* BaseEntity::Cast2Human()
	{
		return 0;
	}

	TraderEntity* BaseEntity::Cast2Trader()
	{
		return 0;
	}

	VehicleEntity* BaseEntity::Cast2Vehicle()
	{
		return 0;
	}

	eid_t BaseEntity::GetEID() const
	{
		return m_eid;
	}

	void BaseEntity::SetEID(eid_t eid)
	{
		m_eid = eid;
	}

	void BaseEntity::RaiseFlags(unsigned flags)
	{
		m_flags |= flags;

		//�������� � �������. � �������
		notify_obs_cond notification(flags & PT_ALL_PLAYERS, this, EntityObserver::EV_JOIN_TEAM);
	}

	void BaseEntity::DropFlags(unsigned flags)
	{
		m_flags &= ~flags;

		//�������� � �������. � �������
		notify_obs_cond notification(flags & PT_ALL_PLAYERS, this, EntityObserver::EV_QUIT_TEAM);
	}

	bool BaseEntity::IsRaised(unsigned flag) const
	{
		return (m_flags & flag) != 0;
	}

	entity_type BaseEntity::GetType() const
	{
		return static_cast<entity_type>(m_flags & ET_ALL_ENTS);
	}

	player_type BaseEntity::GetPlayer() const
	{
		return static_cast<player_type>(m_flags & PT_ALL_PLAYERS);
	}

	entity_attribute BaseEntity::GetAttributes() const
	{
		return static_cast<entity_attribute>(m_flags & EA_ALL_ATTRIBUTES);
	}

	AIContext* BaseEntity::GetAIContext()
	{
		return m_ai_context;
	}

	void BaseEntity::SetAIContext(AIContext* new_context)
	{
		delete m_ai_context;
		m_ai_context = new_context;

		EnemyDetector::getInst()->notifyEntityChangedRelationWithPlayer(m_eid);
	}

	GraphEntity* BaseEntity::GetGraph()
	{
		return m_graph;
	}

	EntityContext* BaseEntity::GetEntityContext()
	{
		return m_entity_context;
	}

	const EntityInfo* BaseEntity::GetInfo() const
	{
		return m_info;
	}

	//======================================================================

	EntityContext::EntityContext(BaseEntity* entity) :
	m_sound_strategy(0), m_death_strategy(0),
		m_marker(0), m_entity(entity), m_spectator(0)
	{
	}

	EntityContext::~EntityContext()
	{
		delete m_death_strategy;
		delete m_sound_strategy;

		delete m_marker;
		delete m_spectator;
	}

	void EntityContext::MakeSaveLoad(SavSlot& slot)
	{
		if(slot.IsSaving()){

			slot << m_entity->GetEID();

			slot << (m_marker != 0);
			if(m_marker){
				DynUtils::SaveObj(slot, m_marker);
				m_marker->MakeSaveLoad(slot);
			}        

			slot << (m_spectator != 0);
			if(m_spectator){
				DynUtils::SaveObj(slot, m_spectator);
				m_spectator->MakeSaveLoad(slot);
			}

			slot << (m_death_strategy != 0);
			if(m_death_strategy){
				DynUtils::SaveObj(slot, m_death_strategy);
				m_death_strategy->MakeSaveLoad(slot);
			}

			slot << (m_sound_strategy != 0);
			if(m_sound_strategy){
				DynUtils::SaveObj(slot, m_sound_strategy);
				m_sound_strategy->MakeSaveLoad(slot);
			}

			slot << m_ai_model;
			slot << m_spawn_zone;

		}else{

			eid_t eid;

			slot >> eid;        
			m_entity = EntityPool::GetInst()->Get(eid);

			bool flag;

			slot >> flag;
			if(flag){
				DynUtils::LoadObj(slot, m_marker);
				m_marker->MakeSaveLoad(slot);
			}

			slot >> flag;
			if(flag){
				DynUtils::LoadObj(slot, m_spectator);
				m_spectator->MakeSaveLoad(slot);
			}

			slot >> flag;
			if(flag){
				DynUtils::LoadObj(slot, m_death_strategy);
				m_death_strategy->MakeSaveLoad(slot);
			}

			slot >> flag;
			if(flag){
				DynUtils::LoadObj(slot, m_sound_strategy);
				m_sound_strategy->MakeSaveLoad(slot);
			}

			slot >> m_ai_model;
			slot >> m_spawn_zone;
		}
	}

	const std::string& EntityContext::GetAIModel() const
	{
		return m_ai_model;
	}

	void EntityContext::SetAIModel(const std::string& label)
	{
		m_ai_model = label;
	}

	const std::string& EntityContext::GetSpawnZone() const
	{
		return m_spawn_zone;
	}

	void EntityContext::SetSpawnZone(const std::string& info)
	{
		m_spawn_zone = info;
	}

	void EntityContext::SetDeathStrategy(DeathStrategy* strategy)
	{
		delete m_death_strategy;
		m_death_strategy = strategy;
	}

	void EntityContext::PlayDeath(BaseEntity* killer)
	{
		bool erase_entity = false;

		//����� �������� �����������
		m_entity->Notify(EntityObserver::EV_PREPARE_DEATH);

		//������� ������� ���������
		GameObserver::kill_info kill_info(killer, m_entity);
		GameEvMessenger::GetInst()->Notify(GameObserver::EV_KILL, &kill_info);

		//��������� ��������� ���� ��� ����
		if(m_death_strategy) erase_entity = m_death_strategy->MakeDeath(m_entity, killer);

		//����� �������� �����������
		m_entity->Notify(EntityObserver::EV_DEATH_PLAYED);

		//����� ������� �������� (���� �����)
		if(erase_entity) EntityPool::GetInst()->Remove(m_entity);
	}

	Spectator* EntityContext::GetSpectator()
	{
		return m_spectator;
	}

	void EntityContext::SetSpectator(Spectator* spectator)
	{
		delete m_spectator;
		m_spectator = spectator;

		//�������� �� ��������� ���� ���������
		m_entity->Notify(EntityObserver::EV_FOS_CHANGE);
	}

	Marker* EntityContext::GetMarker()
	{
		return m_marker;
	}

	void EntityContext::SetMarker(Marker* marker)
	{
		delete m_marker;
		m_marker = marker;
	}

	BaseEntity* EntityContext::GetParent() const
	{
		return m_entity;
	}

	void EntityContext::SetSoundStrategy(SoundStrategy* strategy)
	{
		delete m_sound_strategy;
		m_sound_strategy = strategy;
	}

	//======================================================================

	ActiveContext::ActiveContext(BaseEntity* entity) :
	EntityContext(entity), m_entity(entity), m_fselected(false), m_move_strategy(0)
	{
	}

	ActiveContext::~ActiveContext()
	{
		delete m_move_strategy;
	}

	void ActiveContext::SetMoveStrategy(MoveStrategy* strategy)
	{
		delete m_move_strategy;
		m_move_strategy = strategy;
	}

	bool ActiveContext::IsSelected() const
	{
		return m_fselected;
	}

	void ActiveContext::Select(bool flag)
	{
		if(m_fselected == flag) return;

		m_fselected = flag;
		m_entity->Notify(EntityObserver::EV_SELECTION_CHANGE);
	}

	void ActiveContext::MakeSaveLoad(SavSlot& slot)
	{
		EntityContext::MakeSaveLoad(slot);

		if(slot.IsSaving()){

			slot << m_fselected;
			slot << m_entity->GetEID();

			slot << (m_move_strategy != 0);
			if(m_move_strategy){
				DynUtils::SaveObj(slot, m_move_strategy);
				m_move_strategy->MakeSaveLoad(slot);
			}

		}else{

			slot >> m_fselected;

			eid_t eid; slot >> eid;
			m_entity = EntityPool::GetInst()->Get(eid);

			bool flag; slot >> flag;
			if(flag){
				DynUtils::LoadObj(slot, m_move_strategy);
				m_move_strategy->MakeSaveLoad(slot);
			}
		}
	}

	int ActiveContext::GetHexCost() const
	{
		return m_move_strategy ? m_move_strategy->GetHexCost() : 0;
	}

	int ActiveContext::GetStepsCount() const
	{
		return m_move_strategy ? m_move_strategy->GetSteps() : 0;
	}

	//======================================================================

	TraderContext::TraderContext(TraderEntity* trader) :
	EntityContext(trader), m_trader(trader), m_tids(1)
	{
	}

	TraderContext::~TraderContext()
	{
		pack_t::iterator itor = m_pack.begin();

		while(itor != m_pack.end()){
			delete *itor;
			++itor;
		}
	}

	void TraderContext::MakeSaveLoad(SavSlot& slot)
	{
		EntityContext::MakeSaveLoad(slot);

		if(slot.IsSaving()){

			slot << m_tids;
			slot << m_trader->GetEID();

			pack_t::iterator itor = m_pack.begin();
			while(itor != m_pack.end()){

				slot << true;

				DynUtils::SaveObj(slot, *itor);
				(*itor)->MakeSaveLoad(slot);

				++itor;
			}

			slot << false;

		}else{

			slot >> m_tids;

			eid_t eid; slot >> eid;
			m_trader = EntityPool::GetInst()->Get(eid)->Cast2Trader();

			bool flag;

			slot >> flag;
			while(flag){

				BaseThing* thing = 0;

				DynUtils::LoadObj(slot, thing);
				thing->MakeSaveLoad(slot);

				m_pack.insert(thing);

				slot >> flag;
			}
		}
	}

	size_t TraderContext::GetCount(BaseThing* thing) const
	{
		return m_pack.count(thing);
	}

	tid_t TraderContext::InsertThing(BaseThing* thing)
	{
		//��������� ���� ���������� �� ��������
		thing->SetEntity(m_trader);
		thing->SetTID(m_tids++);

		//������� ������� � ������
		m_pack.insert(thing);

		return thing->GetTID();
	}   

	void TraderContext::RemoveThing(BaseThing* thing)
	{
		pack_t::iterator first = m_pack.lower_bound(thing),
			last  = m_pack.upper_bound(thing);

		while(first != last){

			if((*first)->GetTID() == thing->GetTID()){
				thing->SetTID(0);
				thing->SetEntity(0);
				m_pack.erase(first);
				return;
			}

			++first;
		}
	}

	TraderContext::iterator TraderContext::end()
	{
		return iterator(0, m_pack.end(), 0);
	}

	TraderContext::iterator TraderContext::begin(unsigned mask)
	{
		return iterator(&m_pack, m_pack.begin(), mask);
	}

	bool TraderContext::Iterator::IsSuitable(const BaseThing* thing) const
	{ 
		return thing && thing->GetInfo()->GetType() & m_mask;
	}

	TraderContext::Iterator& TraderContext::Iterator::operator ++()
	{
		BaseThing* current = *m_first,
			* next;

		for(m_first++; m_first != m_pack->end(); m_first++){

			if(IsSuitable(next = *m_first))
				return *this;
		}

		return *this;
	}

	TraderContext::Iterator::Iterator(pack_t* pack, pack_t::iterator first, unsigned mask) :
	m_mask(mask), m_first(first), m_pack(pack)
	{ 
		if(m_pack && m_pack->size() && !IsSuitable(*m_first))
			operator++();
	}

	bool TraderContext::less_thing::operator() (const BaseThing* t1, const BaseThing* t2) const
	{
		return *t1 < *t2;
	}

	//======================================================================

	//=====================================================================================//
	//                            HumanContext::HumanContext()                             //
	//=====================================================================================//
	HumanContext::HumanContext(HumanEntity* human)
	:   m_vehicle(0), m_fast_access(0), m_traits(new Traits(human)),
		m_hands_mode(HM_HANDS), m_flame_steps(0), m_panic_selector(0), 
		ActiveContext(human), m_shot_type(SHT_SNAPSHOT), m_human(human),
		m_limits(new Limits(human)), m_pack(m_pack_map[HPK_BACKPACK]+1) 
	{
		memset(m_body_part_damage, 0, sizeof(m_body_part_damage));

		//memset(m_long_shock_damage, 0, sizeof(m_long_shock_damage));
		//memset(m_long_flame_damage, 0, sizeof(m_long_flame_damage));
		//memset(m_long_electric_damage, 0, sizeof(m_long_electric_damage));

		ClearLongDamages();
	}

	//=====================================================================================//
	//                            HumanContext::~HumanContext()                            //
	//=====================================================================================//
	HumanContext::~HumanContext()
	{
		for(size_t k = 0; k < m_pack.size(); delete m_pack[k++]);

		delete m_traits;
		delete m_limits;
		delete m_fast_access;
		delete m_panic_selector;
	}

	//=====================================================================================//
	//                      damage_s HumanContext::GetDamage() const                       //
	//=====================================================================================//
	damage_s HumanContext::GetDamage(int j) const
	{
		damage_s dtypes[DT_COUNT];

		HumanContext *ncc = const_cast<HumanContext *>(this);

		for(HumanContext::iterator i = ncc->begin(HPK_ALL,TT_WEAPON); i != ncc->end(); ++i)
		{
			WeaponThing *weapon = static_cast<WeaponThing*>(&*i);
			if(weapon)
			{
				const damage_s &wdb = weapon->GetInfo()->GetBDmg();
				dtypes[wdb.m_type].m_val += wdb.m_val;
				dtypes[wdb.m_type].m_type = wdb.m_type;
				const damage_s &wda = weapon->GetInfo()->GetADmg();
				dtypes[wda.m_type].m_val += wda.m_val;
				dtypes[wda.m_type].m_type = wda.m_type;

				AmmoThing *ammo = weapon->GetAmmo();
				if(ammo)
				{
					const damage_s &adb = ammo->GetInfo()->GetBDmg();
					dtypes[adb.m_type].m_val += adb.m_val;
					dtypes[adb.m_type].m_type = adb.m_type;
					const damage_s &ada = ammo->GetInfo()->GetADmg();
					dtypes[ada.m_type].m_val += ada.m_val;
					dtypes[ada.m_type].m_type = ada.m_type;
				}
			}
		}

		for(HumanContext::iterator i = ncc->begin(HPK_ALL,TT_GRENADE); i != ncc->end(); ++i)
		{
			GrenadeThing *grenade = static_cast<GrenadeThing*>(&*i);
			if(grenade)
			{
				const damage_s &wdb = grenade->GetInfo()->GetBDmg();
				dtypes[wdb.m_type].m_val += wdb.m_val;
				dtypes[wdb.m_type].m_type = wdb.m_type;
				const damage_s &wda = grenade->GetInfo()->GetADmg();
				dtypes[wda.m_type].m_val += wda.m_val;
				dtypes[wda.m_type].m_type = wda.m_type;
			}
		}

		return dtypes[j];
	}

	//=====================================================================================//
	//                            void HumanContext::AddRank()                             //
	//=====================================================================================//
	void HumanContext::AddRank()
	{
		int oldrank = m_traits->m_rank;
		m_traits->m_rank = std::min(m_traits->m_rank+1,PoL::RankTable::instance()->getCount()-1);
		if(oldrank != m_traits->m_rank)
		{
			GetParent()->Notify(EntityObserver::EV_RANK_UP);
			DirtyLinks::Print((boost::format(DirtyLinks::GetStrRes("pol_add_rank")) % m_human->GetInfo()->GetName()).str());
		}
	}

	//=====================================================================================//
	//                          int HumanContext::GetRank() const                          //
	//=====================================================================================//
	int HumanContext::GetRank() const
	{
		return m_traits->m_rank;
	}

//=====================================================================================//
//                        void HumanContext::SetPanicSelector()                        //
//=====================================================================================//
	void HumanContext::SetPanicSelector(PanicTypeSelector* panic)
	{
		delete m_panic_selector;
		m_panic_selector = panic;    
	}

//=====================================================================================//
//                  human_panic_type HumanContext::SelectPanicType()                   //
//=====================================================================================//
	human_panic_type HumanContext::SelectPanicType()
	{
		human_panic_type  old_panic = GetPanicType(),
			new_panic = m_panic_selector ? m_panic_selector->Select(m_human) : HPT_NONE;

		notify_obs_cond notifier(old_panic != new_panic, m_human, EntityObserver::EV_PANIC_CHANGE);

		return new_panic;
	}

//=====================================================================================//
//                 human_panic_type HumanContext::GetPanicType() const                 //
//=====================================================================================//
	human_panic_type HumanContext::GetPanicType() const
	{
		return m_panic_selector ? m_panic_selector->GetPanic(m_human) : HPT_NONE;
	}

//=====================================================================================//
//                    human_panic_type HumanContext::SetPanicType()                    //
//=====================================================================================//
	human_panic_type HumanContext::SetPanicType(human_panic_type type)
	{
		human_panic_type  old_panic = GetPanicType(),
			new_panic = m_panic_selector ? m_panic_selector->SetPanic(m_human, type) : HPT_NONE;

		notify_obs_cond notifier(old_panic != new_panic, m_human, EntityObserver::EV_PANIC_CHANGE);

		return new_panic;    
	}

//=====================================================================================//
//                          void HumanContext::MakeSaveLoad()                          //
//=====================================================================================//
	void HumanContext::MakeSaveLoad(SavSlot& slot)
	{
		m_traits->MakeSaveLoad(slot);
		m_limits->MakeSaveLoad(slot);

		ActiveContext::MakeSaveLoad(slot);

		if(slot.IsSaving())
		{
			slot << m_human->GetEID();
			slot << (m_vehicle ? m_vehicle->GetEID() : 0);

			for(size_t k = 0; k < m_pack.size(); k++)
			{
				if(m_pack[k] == 0) continue;

				slot << k;
				DynUtils::SaveObj(slot, m_pack[k]);
				m_pack[k]->MakeSaveLoad(slot);
			}

			slot << -1;
			slot << static_cast<int>(m_shot_type);

			slot.Save(m_longDamages, sizeof(m_longDamages));
			slot.Save(m_longAffects, sizeof(m_longAffects));
			slot.Save(m_lastAffects, sizeof(m_lastAffects));

			slot.Save(m_body_part_damage, sizeof(m_body_part_damage));

			slot << (m_panic_selector != 0);

			if(m_panic_selector)
			{
				DynUtils::SaveObj(slot, m_panic_selector);
				m_panic_selector->MakeSaveLoad(slot);
			}

			slot << m_flame_steps;

			//���������� quick slot'a
			if(slot.GetStore()->GetVersion() > 1)
			{
				slot << (m_fast_access != 0);

				if(m_fast_access)
				{
					DynUtils::SaveObj(slot, m_fast_access);
					m_fast_access->MakeSaveLoad(slot);
				}

				slot << static_cast<int>(m_hands_mode);
			}
		}
		else
		{
			eid_t eid; slot >> eid;
			m_human = EntityPool::GetInst()->Get(eid)->Cast2Human();

			slot >> eid;
			if(eid) m_vehicle = EntityPool::GetInst()->Get(eid)->Cast2Vehicle();

			int k;

			slot >> k;
			while(k > 0)
			{
				if((int)m_pack.size() <= k) m_pack.resize(k+1);

				DynUtils::LoadObj(slot, m_pack[k]);
				m_pack[k]->MakeSaveLoad(slot);

				slot >> k;
			}

			slot >> k; m_shot_type = static_cast<shot_type>(k);

			if(slot.GetStore()->GetVersion() < 13)
			{
				ClearLongDamages();

				int lsd[DMG_DURATION], lfd[DMG_DURATION], led[DMG_DURATION];

				slot.Load(lsd, sizeof(lsd));
				slot.Load(lfd, sizeof(lfd));
				slot.Load(led, sizeof(led));

				for(int i = 0; i < DMG_DURATION; ++i)
				{
					m_longDamages[DT_FLAME][i] = lfd[i];
					m_longDamages[DT_SHOCK][i] = lsd[i];
					m_longDamages[DT_ELECTRIC][i] = led[i];

					m_longAffects[apAccuracy][i] = m_longDamages[DT_ELECTRIC][i]/DMG_DURATION;
					m_longAffects[apDexterity][i] = m_longDamages[DT_ELECTRIC][i]/DMG_DURATION;
					m_longAffects[apHealth][i] = m_longDamages[DT_FLAME][i];
				}

				m_longAffects[apMaxMovepoints][0] = m_longDamages[DT_SHOCK][0];
				m_longAffects[apMaxMovepoints][1] = 2*(m_longDamages[DT_SHOCK][0]/DMG_DURATION);
				m_longAffects[apMaxMovepoints][2] = m_longDamages[DT_SHOCK][0]/DMG_DURATION;
			}
			else if(slot.GetStore()->GetVersion() < 14)
			{
				ClearLongDamages();
				slot.Load(m_longDamages, sizeof(m_longDamages));
				slot.Load(m_longAffects, sizeof(m_longAffects[0])*4);
				slot.Load(m_lastAffects, sizeof(m_lastAffects[0])*4);
			}
			else
			{
				slot.Load(m_longDamages, sizeof(m_longDamages));
				slot.Load(m_longAffects, sizeof(m_longAffects));
				slot.Load(m_lastAffects, sizeof(m_lastAffects));
			}

			slot.Load(m_body_part_damage, sizeof(m_body_part_damage));

			bool flag;

			slot >> flag;
			if(flag)
			{
				DynUtils::LoadObj(slot, m_panic_selector);
				m_panic_selector->MakeSaveLoad(slot);
			}

			slot >> m_flame_steps;

			//������� quick slot'a
			if(slot.GetStore()->GetVersion() > 1)
			{
				slot >> flag;
				if(flag)
				{
					DynUtils::LoadObj(slot, m_fast_access);
					m_fast_access->MakeSaveLoad(slot);
				}

				int tmp; slot >> tmp;
				m_hands_mode = static_cast<hands_mode>(tmp);
			}
		}
	}

	int HumanContext::GetBodyPartDamage(body_parts_type type) const
	{
		return m_body_part_damage[type];
	}

	void HumanContext::SetBodyPartDamage(body_parts_type type, int dmg)
	{    
		if(dmg < 0) dmg = 0;
		m_body_part_damage[type] = dmg;

		m_human->Notify(EntityObserver::EV_BODY_PART_CHANGE);
	}

	shot_type HumanContext::GetShotType() const
	{
		return m_shot_type;
	}

	void HumanContext::SetShotType(shot_type type)
	{
		m_shot_type = type;
	}

//=====================================================================================//
//                      bool HumanContext::HaveLongDamage() const                      //
//=====================================================================================//
	bool HumanContext::HaveLongDamage(damage_type type) const
	{
		return m_longDamages[type][0] || m_longDamages[type][1] || m_longDamages[type][2];
		//if(type == DT_FLAME)
		//	return m_long_flame_damage[0] || m_long_flame_damage[1] || m_long_flame_damage[2];
		//else if(type == DT_SHOCK)
		//	return m_long_shock_damage[0] || m_long_shock_damage[1] || m_long_shock_damage[2];
		//else if(type == DT_ELECTRIC)
		//	return m_long_electric_damage[0] || m_long_electric_damage[1] || m_long_electric_damage[2];

		return false;
	}

//=====================================================================================//
//                       int HumanContext::GetLongDamage() const                       //
//=====================================================================================//
	int HumanContext::GetLongDamage(damage_type type, int moment) const
	{
		return m_longDamages[type][moment];
		//if(type == DT_FLAME)
		//	return m_long_flame_damage[moment];
		//else if(type == DT_SHOCK)
		//	return m_long_shock_damage[moment];
		//else if(type == DT_ELECTRIC)
		//	return m_long_electric_damage[moment];

		return 0;
	}

//=====================================================================================//
//                         void HumanContext::AddLongDamage()                          //
//=====================================================================================//
	void HumanContext::AddLongDamage(damage_type type, int amount)
	{
		if(amount <= 0) return;
		
		switch(type)
		{
		case DT_SHOCK:
			ApplyEffect(apMaxMovepoints,amount);
			break;
		case DT_ELECTRIC:
			ApplyEffect(apDexterity,amount);
			ApplyEffect(apAccuracy,amount);
			break;
		}
	}

//=====================================================================================//
//                      void HumanContext::AddAccuracyRunAffect()                      //
//=====================================================================================//
	void HumanContext::AddAccuracyRunAffect(int amount)
	{
		if(amount <= 0) return;
        m_lastAffects[apAccuracyRun] += amount;
	}

//=====================================================================================//
//                          void HumanContext::ApplyEffect()                           //
//=====================================================================================//
	void HumanContext::ApplyEffect(AffectedParameter param, int amount)
	{
		if(param == apHealth)
		{
			for(int i = 0; i < DMG_DURATION; ++i)
			{
				m_longAffects[param][i] += amount;
			}
		}
		else if(param == apMaxMovepoints)
		{
			const int max_amount = GetLimits()->GetMovepnts() - m_longAffects[param][0];
			if(amount > max_amount) amount = max_amount;

			if(amount > 0)
			{
				const int part = amount / DMG_DURATION;
				const int rem = amount % DMG_DURATION;
				const int parts[DMG_DURATION] = { part, part + (rem > 1 ? 1 : 0), part + (rem > 0 ? 1 : 0)};

				m_longAffects[param][0] += parts[0] + parts[1] + parts[2];
				m_longAffects[param][1] += parts[1] + parts[2];
				m_longAffects[param][2] += parts[2];
			}
		}
		else
		{
			int max_amount = 0;
			switch(param)
			{
				case apDexterity:
					max_amount = GetTraits()->GetDexterity();
					break;
				case apAccuracy:
					max_amount = GetTraits()->GetAccuracy();
					break;
			}
			if(amount > max_amount) amount = max_amount;

			if(amount > 0)
			{
				const int part = amount / DMG_DURATION;
				const int rem = amount % DMG_DURATION;
				const int parts[DMG_DURATION] = { part, part + (rem > 1 ? 1 : 0), part + (rem > 0 ? 1 : 0)};

				m_longAffects[param][0] += parts[0];
				m_longAffects[param][1] += parts[1];
				m_longAffects[param][2] += parts[2];
			}
		}
	}

//=====================================================================================//
//                        void HumanContext::ShiftLongDamages()                        //
//=====================================================================================//
	void HumanContext::ShiftLongDamages()
	{
		int accuracyRunLast = m_lastAffects[apAccuracyRun] - PoL::Inv::AccuracyPerTurn;

		for(int j = 0; j < apCount; ++j)
		{
			m_lastAffects[j] = m_longAffects[j][0];
		}

		for(int i = 0; i < DMG_DURATION-1; ++i)
		{
			for(int j = 0; j < DT_COUNT; ++j)
			{
				m_longDamages[j][i] = m_longDamages[j][i+1];
			}

			for(int j = 0; j < apCount; ++j)
			{
				m_longAffects[j][i] = m_longAffects[j][i+1];
			}
		}

		for(int j = 0; j < DT_COUNT; ++j)
		{
			m_longDamages[j][DMG_DURATION-1] = 0;
		}

		for(int j = 0; j < apCount; ++j)
		{
			m_longAffects[j][DMG_DURATION-1] = 0;
		}

		if(accuracyRunLast > 0)
		{
			m_lastAffects[apAccuracyRun] = accuracyRunLast;
		}
	}

//=====================================================================================//
//                        void HumanContext::ClearLongDamages()                        //
//=====================================================================================//
	void HumanContext::ClearLongDamages()
	{
		for(int i = 0; i < DMG_DURATION; ++i)
		{
			for(int j = 0; j < DT_COUNT; ++j)
			{
				m_longDamages[j][i] = 0;
			}

			for(int j = 0; j < apCount; ++j)
			{
				m_longAffects[j][i] = 0;
			}
		}

		for(int j = 0; j < apCount; ++j)
		{
			m_lastAffects[j] = 0;
		}
	}

//=====================================================================================//
//                         void HumanContext::SetLongDamage()                          //
//=====================================================================================//
	//void HumanContext::SetLongDamage(damage_type type, int value, int moment)
	//{
	//	//if(value < 0) value = 0;

	//	//bool changed = false;

	//	//if(type == DT_FLAME)
	//	//{
	//	//	changed = (m_long_flame_damage[moment] != value);
	//	//	m_long_flame_damage[moment] = value;
	//	//}
	//	//else if(type == DT_SHOCK)
	//	//{
	//	//	changed = (m_long_shock_damage[moment] != value);
	//	//	m_long_shock_damage[moment] = value;
	//	//}
	//	//else if(type == DT_ELECTRIC)
	//	//{
	//	//	changed = (m_long_electric_damage[moment] != value);
	//	//	m_long_electric_damage[moment] = value;   
	//	//}

	//	//if(changed)
	//	//{
	//	//	m_human->Notify(EntityObserver::EV_LONG_DAMAGE_CHANGE);
	//	//}
	//}

	HumanContext::Traits* HumanContext::GetTraits()
	{
		return m_traits;
	}

	HumanContext::Limits* HumanContext::GetLimits()
	{
		return m_limits;
	}

	VehicleEntity* HumanContext::GetCrew()
	{
		return m_vehicle;
	}

	void HumanContext::SetCrew(VehicleEntity* vehicle)
	{
		unsigned int flags = m_human->GetFlags();

		if(m_vehicle = vehicle)
		{
			m_human->RaiseFlags(EA_INCREW);
			m_human->DropFlags(EA_NOT_INCREW);
		}
		else
		{
			m_human->RaiseFlags(EA_NOT_INCREW);
			m_human->DropFlags(EA_INCREW);
		}

		if(flags != m_human->GetFlags())
		{
			m_human->Notify(EntityObserver::EV_CREW_CHANGE);
		}
	}

	tid_t HumanContext::InsertThing(human_pack_type pack, BaseThing* thing)
	{
		if(pack == HPK_ALL || pack == HPK_NONE) pack = HPK_BACKPACK;

		//��������� ������� � ������������
		if(m_human->GetPlayer() == PT_PLAYER) ThingHandbook::GetInst()->Push(thing->GetInfo(),ThingHandbook::F_NEW_REC);

		size_t beg = m_pack_map[pack];
		size_t end = pack == HPK_BACKPACK ? m_pack.size() : m_pack_map[pack+1];

		EntityObserver::thing_info_s thing_info(thing);

		//�������� �����������
		notify_obs      notifier0(m_human, EntityObserver::EV_INSERT_THING, &thing_info);
		notify_obs_cond notifier1(thing->GetWeight(), m_human, EntityObserver::EV_WEIGHT_CHANGE);
		notify_obs_cond notifier2(pack == HPK_BODY, m_human, EntityObserver::EV_BODY_PACK_CHANGE);  
		notify_obs_cond notifier3(pack == HPK_HANDS, m_human, EntityObserver::EV_HANDS_PACK_CHANGE);
		//notify_obs_cond notifier4(pack == HPK_HANDS && thing->GetInfo()->IsScanner(), m_human, EntityObserver::EV_FOS_CHANGE);

		//���������� ������� �������
		if(pack == HPK_IMPLANTS && thing->GetInfo()->IsImplant())
		{
			const ImplantInfo* info = static_cast<ImplantThing*>(thing)->GetInfo();
			m_traits->HandleInsertImplant(info->GetParameter(), info->GetPower());
		}

		for(size_t k = beg; k < end; k++)
		{
			if(m_pack[k] == 0)
			{
				m_pack[k] = thing;

				thing->SetTID(k);
				thing->SetEntity(m_human);

				m_human->Notify(EntityObserver::EV_MAXMOVEPNTS_CHANGE);
				return k;
			}
		}

		if(pack == HPK_BACKPACK)
		{
			thing->SetEntity(m_human);
			thing->SetTID(m_pack.size());

			m_pack.push_back(thing);

			m_human->Notify(EntityObserver::EV_MAXMOVEPNTS_CHANGE);
			return thing->GetTID();
		}

		throw CASUS("HumanContext: ��������� ������� ���������� ���������!");
	}

	void HumanContext::RemoveThing(BaseThing* thing)
	{
		if(m_pack[thing->GetTID()] == thing){

			m_pack[thing->GetTID()] = 0;

			//��������� pack �� ������ ��������
			human_pack_type pack = HPK_BACKPACK;

			for(size_t k = 0; k < MAX_PACKS; k++){
				if((tid_t)m_pack_map[k] > thing->GetTID()){
					pack = static_cast<human_pack_type>(k-1);
					break;
				}
			}

			EntityObserver::thing_info_s thing_info(thing);

			//�������� �����������
			notify_obs      notifier0(m_human, EntityObserver::EV_REMOVE_THING, &thing_info);
			notify_obs_cond notifier1(thing->GetWeight(), m_human, EntityObserver::EV_WEIGHT_CHANGE);
			notify_obs_cond notifier2(pack == HPK_BODY, m_human, EntityObserver::EV_BODY_PACK_CHANGE);
			notify_obs_cond notifier3(pack == HPK_HANDS, m_human, EntityObserver::EV_HANDS_PACK_CHANGE);
			notify_obs_cond notifier4(pack == HPK_HANDS && thing->GetInfo()->IsScanner(), m_human, EntityObserver::EV_FOS_CHANGE);

			thing->SetEntity(0);
			thing->SetTID(0);
		}

		m_human->Notify(EntityObserver::EV_MAXMOVEPNTS_CHANGE);
	}

	void HumanContext::RemoveThing(iterator& itor)
	{
		RemoveThing(&*itor);
	}

	bool HumanContext::IsFull(human_pack_type pack) const
	{
		if(pack == HPK_ALL || pack == HPK_BACKPACK || pack == HPK_NONE)
			return false;

		int beg = m_pack_map[pack];
		int end = m_pack_map[pack+1];

		for(int k = beg; k < end; k++){
			if(!m_pack[k]) return false;
		}

		return true;
	}

	int  HumanContext::GetFlameSteps() const
	{
		return m_flame_steps;
	}

	void HumanContext::SetFlameSteps(int steps)
	{
		m_flame_steps = steps;
	}

	FastAccessStrategy* HumanContext::GetFastAccessStrategy()
	{
		if(m_fast_access == 0) m_fast_access = StrategyFactory::CreateFastAccess(StrategyFactory::FAT_USUAL, m_human);
		return m_fast_access;
	}

	void HumanContext::SetFastAccessStrategy(FastAccessStrategy* strategy)
	{
		delete m_fast_access;
		m_fast_access = strategy;
	}

//=====================================================================================//
//                          void HumanContext::SetHandsMode()                          //
//=====================================================================================//
	void HumanContext::SetHandsMode(hands_mode mode)
	{
		//if(!CanSit())
		//{
		//	mode = HM_HANDS;
		//}
		notify_obs_cond notifier(mode != m_hands_mode, m_human, EntityObserver::EV_HANDS_PACK_CHANGE);
		m_hands_mode = mode;  
	}

//=====================================================================================//
//             HumanContext::hands_mode HumanContext::GetHandsMode() const             //
//=====================================================================================//
	HumanContext::hands_mode HumanContext::GetHandsMode() const
	{
		return m_hands_mode;
	}

//=====================================================================================//
//                     BaseThing* HumanContext::GetThingInHands()                      //
//=====================================================================================//
	BaseThing* HumanContext::GetThingInHands(unsigned mask)
	{
		BaseThing* thing = 0;

		// ���� �������� �� ����������� ������, �� ���������� ������������ ����� �������� �������
		if(GetParent()->GetPlayer() == PT_PLAYER && m_hands_mode == HM_FAST_ACCESS)
		{
			thing =     GetFastAccessStrategy()->GetThing()
				?   GetFastAccessStrategy()->GetThing()
				:   m_pack[m_pack_map[HPK_HANDS]];
		}
		else
		{
			thing = m_pack[m_pack_map[HPK_HANDS]];
		}

		return (thing && thing->GetInfo()->GetType() & mask) ? thing : 0;
	}

//=====================================================================================//
//                    BaseThing* HumanContext::PoLGetThingInHands()                    //
//=====================================================================================//
	BaseThing* HumanContext::PoLGetThingInHands(unsigned mask)
	{
		BaseThing* thing = 0;

		thing = m_pack[m_pack_map[HPK_HANDS]];

		return (thing && thing->GetInfo()->GetType() & mask) ? thing : 0;
	}

	HumanContext::iterator HumanContext::begin(human_pack_type pack, unsigned mask)
	{
		size_t beg = (pack == HPK_ALL) ? m_pack_map[HPK_HEAD] : m_pack_map[pack];
		size_t end = (pack == HPK_BACKPACK || pack == HPK_ALL) ? m_pack.size() : m_pack_map[pack + 1];

		return iterator(&m_pack, beg, end, mask);
	}

	HumanContext::iterator HumanContext::end()
	{
		return iterator(0, m_pack.size(), m_pack.size(), 0);
	}

	HumanContext::Iterator::Iterator(HumanContext::pack_t* pack, int first, int last, unsigned mask):
	m_pack(pack), m_mask(mask), m_first(first), m_last(last)
	{
		if(m_pack && !IsSuitable((*m_pack)[m_first])) operator++();
	}

	HumanContext::Iterator& HumanContext::Iterator::operator ++()
	{
		for(m_first++; m_first < m_last; m_first++){
			if(IsSuitable((*m_pack)[m_first]))
				return *this;
		}

		m_first = m_last = m_pack->size(); 
		return *this;
	}

	bool HumanContext::Iterator::IsSuitable(const BaseThing* thing) const
	{
		return thing && thing->GetInfo()->GetType() & m_mask;
	}

	bool HumanContext::CanRun() const
	{
		ArmorThing* body_thing =    m_pack[m_pack_map[HPK_BODY]] && m_pack[m_pack_map[HPK_BODY]]->GetInfo()->IsArmor()
			?  static_cast<ArmorThing*>(m_pack[m_pack_map[HPK_BODY]])
			:  0;

		HumanContext* This = const_cast<HumanContext*>(this);

		WeaponThing* hands_thing = static_cast<WeaponThing*>(This->GetThingInHands(TT_WEAPON));

		return      (body_thing  == 0 || !body_thing->GetInfo()->IsSpaceSuit())
			&&  (hands_thing == 0 || (!hands_thing->GetInfo()->IsRocket() && !hands_thing->GetInfo()->IsAutoCannon()));    
	}

	bool HumanContext::CanSit() const
	{
		ArmorThing* body_thing =   m_pack[m_pack_map[HPK_BODY]] && m_pack[m_pack_map[HPK_BODY]]->GetInfo()->IsArmor()
			?  static_cast<ArmorThing*>(m_pack[m_pack_map[HPK_BODY]])
			:  0;

		return body_thing == 0 || !body_thing->GetInfo()->IsSpaceSuit();
	}

	HumanContext::reload_notifier::reload_notifier(HumanEntity* human, WeaponThing* thing) :
	m_ammo_count(0), m_actor(human), m_weapon(thing)
	{
		if(AmmoThing* ammo = m_weapon->GetAmmo())
		{
			m_ammo_count = ammo->GetBulletCount();            
			m_ammo_rid   = ammo->GetInfo()->GetRID();
		}
	}

	HumanContext::reload_notifier::~reload_notifier()
	{
		if(AmmoThing* ammo = m_weapon->GetAmmo())
		{
			if(ammo->GetInfo()->GetRID() != m_ammo_rid || ammo->GetBulletCount() != m_ammo_count)
				m_actor->Notify(EntityObserver::EV_WEAPON_RELOAD);                
		}
		else
		{
			if(m_ammo_rid.size()) m_actor->Notify(EntityObserver::EV_WEAPON_RELOAD);
		}
	}

	//======================================================================

	HumanContext::Traits::Traits(HumanEntity* human) :
	m_human(human)
	{
		if(m_human)
		{
			const HumanInfo* info = m_human->GetInfo();

			m_sight_angle  = info->GetSightAngle();

			m_rank = info->GetRank();
			m_movepoints = 0;

			m_experience = 0;

			if((m_human->GetInfo()->IsAntihero() || m_human->GetInfo()->IsHero())
				&& PoL::ExperienceSaver::inst()->hasHeroInformation(human->GetInfo()->GetRID()))
			{
				const PoL::ExperienceInfo &info = PoL::ExperienceSaver::inst()->getExperienceInfo(human->GetInfo()->GetRID());
				m_health = info.m_health;
				m_wisdom = info.m_wisdom;
				m_morale = info.m_morale;
				m_accuracy = info.m_accuracy;
				m_strength = info.m_strength;
				m_reaction = info.m_reaction;
				m_mechanics = info.m_mechanics;
				m_dexterity = info.m_dexterity;
				m_front_radius = info.m_front_radius;
				m_experience = info.m_experience;
				m_level = info.m_level;
				m_levelup_points = info.m_levelup_points;
				m_rank = info.m_rank;
			}
			else
			{
				m_health = info->GetHealth();
				m_wisdom = info->GetWisdom();
				m_morale = info->GetMorale();
				m_accuracy = info->GetAccuracy();
				m_strength = info->GetStrength();
				m_reaction = info->GetReaction();
				m_dexterity = info->GetDexterity();
				m_mechanics = info->GetMechanics();
				m_front_radius = info->GetSightRadius();
				m_level = 0;

				if(info->GetLevelupBase())
				{
					PoLAddExperience(LevelupMgr::GetInst()->GetLevelPoints(info->GetLevelupBase() - 1));
				}

				m_levelup_points = 0;
			}

			m_shock_res = info->GetShockRes();
			m_flame_res = info->GetFlameRes();
			m_electric_res = info->GetElectricRes();

			m_const_wisdom = 0;
			m_const_health = 0;
			m_const_accuracy = 0;
			m_const_strength = 0;
			m_const_reaction = 0;

			m_const_shock_res = 0;
			m_const_flame_res = 0;
			m_const_electric_res = 0;

			m_const_front_radius = 0;
		}
	}

	void HumanContext::Traits::MakeSaveLoad(SavSlot& slot)
	{
		if(slot.IsSaving())
		{
			slot << m_rank;

			slot << m_const_wisdom;
			slot << m_const_health;
			slot << m_const_accuracy;
			slot << m_const_strength;
			slot << m_const_reaction;

			slot << m_const_shock_res;
			slot << m_const_flame_res;
			slot << m_const_electric_res;

			slot << m_const_front_radius;

			slot << m_sight_angle;
			slot << m_front_radius;

			slot << m_movepoints;

			slot << m_level;
			slot << m_experience;
			slot << m_levelup_points;

			slot << m_health;
			slot << m_wisdom;
			slot << m_morale;
			slot << m_accuracy;
			slot << m_strength;
			slot << m_reaction;
			slot << m_dexterity;
			slot << m_mechanics;

			slot << m_shock_res;
			slot << m_flame_res;
			slot << m_electric_res;

			slot << m_human->GetEID();

		}
		else
		{
			slot >> m_rank;

			slot >> m_const_wisdom;
			slot >> m_const_health;
			slot >> m_const_accuracy;
			slot >> m_const_strength;
			slot >> m_const_reaction;

			slot >> m_const_shock_res;
			slot >> m_const_flame_res;
			slot >> m_const_electric_res;

			slot >> m_const_front_radius;

			slot >> m_sight_angle;
			slot >> m_front_radius;

			slot >> m_movepoints;

			slot >> m_level;
			slot >> m_experience;
			slot >> m_levelup_points;

			slot >> m_health;
			slot >> m_wisdom;
			slot >> m_morale;
			slot >> m_accuracy;
			slot >> m_strength;
			slot >> m_reaction;
			slot >> m_dexterity;
			slot >> m_mechanics;

			slot >> m_shock_res;
			slot >> m_flame_res;
			slot >> m_electric_res;

			eid_t eid; slot >>eid;
			m_human = EntityPool::GetInst()->Get(eid)->Cast2Human();
		}
	}

	float HumanContext::Traits::GetWeight() const
	{
		float weight = 0;
		iterator itor = m_human->GetEntityContext()->begin(HPK_ALL);

		while(itor != m_human->GetEntityContext()->end())
		{
			weight += itor->GetWeight();
			++itor;
		}

		return weight;
	}

	int HumanContext::Traits::GetHealth() const
	{
		return m_health;
	}

	int HumanContext::Traits::GetConstHealth() const
	{
		return m_const_health;
	}

	int HumanContext::Traits::GetWisdom() const
	{
		return m_wisdom + m_const_wisdom;
	}

	int HumanContext::Traits::GetMorale() const
	{
		return m_morale;
	}

	int HumanContext::Traits::GetMovepnts() const
	{
		return m_movepoints;
	}

	int HumanContext::Traits::GetAccuracy() const
	{
		return m_accuracy + m_const_accuracy;
	}

	int HumanContext::Traits::GetStrength() const
	{
		return m_strength + m_const_strength;
	}

	int HumanContext::Traits::GetReaction() const
	{
		return m_reaction + m_const_reaction;
	}

	int HumanContext::Traits::GetMechanics() const
	{
		return m_mechanics;
	}

	int HumanContext::Traits::GetDexterity() const
	{
		return m_dexterity;
	}

	int HumanContext::Traits::GetShockRes() const
	{
		return m_shock_res + m_const_shock_res;
	}

	int HumanContext::Traits::GetFlameRes() const
	{
		return m_flame_res + m_const_flame_res;
	}

	int HumanContext::Traits::GetElectricRes() const
	{
		return m_electric_res + m_const_electric_res;
	}

	float HumanContext::Traits::GetSightAngle() const
	{
		return m_sight_angle;
	}

	float HumanContext::Traits::GetBackRadius() const
	{
		return human_back_sight_radius;
	}

	float HumanContext::Traits::GetFrontRadius() const
	{
		return m_front_radius + m_const_front_radius;
	}

	int HumanContext::Traits::GetExperience() const
	{
		return m_experience;
	}

	int HumanContext::Traits::GetLevelupPoints() const
	{
		return m_levelup_points;
	}

	//=====================================================================================//
	//                      void HumanContext::Traits::AddAccuracy()                       //
	//=====================================================================================//
	void HumanContext::Traits::AddAccuracy(int val)
	{
		if(m_human->IsRaised(EA_LOCK_TRAITS) || val == 0)
		{
			return;
		}

		HumanContext *ctx = m_human->GetEntityContext();
		m_accuracy = saturate(m_accuracy + val, 0, ctx->GetLimits()->GetAccuracy());
		m_human->Notify(EntityObserver::EV_ACCURACY_CHANGE);
	}

	//=====================================================================================//
	//                     void HumanContext::Traits::PoLAddAccuracy()                     //
	//=====================================================================================//
	void HumanContext::Traits::PoLAddAccuracy(int val)
	{
		if(m_human->IsRaised(EA_LOCK_TRAITS) || val == 0 || UnlimitedMoves())
		{
			return;
		}

		HumanContext *ctx = m_human->GetEntityContext();
		m_accuracy = saturate(m_accuracy + val, 0, ctx->GetLimits()->GetAccuracy());
		m_human->Notify(EntityObserver::EV_ACCURACY_CHANGE);
	}

	//=====================================================================================//
	//                    void HumanContext::Traits::PoLAddDexterity()                     //
	//=====================================================================================//
	void HumanContext::Traits::PoLAddDexterity(int val)
	{
		if(m_human->IsRaised(EA_LOCK_TRAITS) || val == 0 || UnlimitedMoves())
		{
			return;
		}

		m_dexterity = saturate(m_dexterity + val, 0, m_human->GetEntityContext()->GetLimits()->GetDexterity());
		m_human->Notify(EntityObserver::EV_DEXTERITY_CHANGE);
	}


	//=====================================================================================//
	//                       void HumanContext::Traits::AddWisdom()                        //
	//=====================================================================================//
	void HumanContext::Traits::AddWisdom(int val)
	{
		if(m_human->IsRaised(EA_LOCK_TRAITS))
			return;

		HumanContext *ctx = m_human->GetEntityContext();
		m_wisdom = saturate(m_wisdom + val, 0, ctx->GetLimits()->GetWisdom());
		m_human->Notify(EntityObserver::EV_WISDOM_CHANGE);
	}

	//=====================================================================================//
	//                       void HumanContext::Traits::AddMorale()                        //
	//=====================================================================================//
	void HumanContext::Traits::AddMorale(int val)
	{
		if(m_human->IsRaised(EA_LOCK_TRAITS))
			return;

		m_morale = saturate(m_morale + val, 0, m_human->GetEntityContext()->GetLimits()->GetMorale());
		m_human->Notify(EntityObserver::EV_MORALE_CHANGE);
	}

	//=====================================================================================//
	//                       void HumanContext::Traits::AddHealth()                        //
	//=====================================================================================//
	void HumanContext::Traits::AddHealth(int val)
	{
		if(val == 0 || m_human->IsRaised(EA_LOCK_TRAITS))
		{
			return;
		}

		m_health = saturate(m_health + val, 0, m_human->GetEntityContext()->GetLimits()->GetHealth());
		m_human->Notify(EntityObserver::EV_HEALTH_CHANGE);
	}

	//=====================================================================================//
	//                     void HumanContext::Traits::PoLAddMovepnts()                     //
	//=====================================================================================//
	void HumanContext::Traits::PoLAddMovepnts(int val)
	{
		if(m_human->IsRaised(EA_LOCK_TRAITS) || UnlimitedMoves()) return;
		SetMovepnts(m_movepoints + val);
	}

	//=====================================================================================//
	//                      void HumanContext::Traits::AddMovepnts()                       //
	//=====================================================================================//
	void HumanContext::Traits::AddMovepnts(int val)
	{
		if(m_human->IsRaised(EA_LOCK_TRAITS))
			return;

		SetMovepnts(m_movepoints + val);
	}

	//=====================================================================================//
	//                      void HumanContext::Traits::SetMovepnts()                       //
	//=====================================================================================//
	void HumanContext::Traits::SetMovepnts(int val)
	{
		if(m_human->IsRaised(EA_LOCK_TRAITS))
			return;

		m_movepoints = val > 0 ? val : 0;
		m_human->Notify(EntityObserver::EV_MOVEPNTS_CHANGE);
	}

	//=====================================================================================//
	//                      void HumanContext::Traits::AddStrength()                       //
	//=====================================================================================//
	void HumanContext::Traits::AddStrength(int val)
	{
		if(m_human->IsRaised(EA_LOCK_TRAITS))
			return;

		m_strength = saturate(m_strength + val, 0, m_human->GetEntityContext()->GetLimits()->GetStrength());
		m_human->Notify(EntityObserver::EV_STRENGTH_CHANGE);
		m_human->Notify(EntityObserver::EV_MAXMOVEPNTS_CHANGE);
	}

	//=====================================================================================//
	//                      void HumanContext::Traits::AddReaction()                       //
	//=====================================================================================//
	void HumanContext::Traits::AddReaction(int val)
	{
		if(m_human->IsRaised(EA_LOCK_TRAITS))
			return;

		m_reaction = saturate(m_reaction + val, 0, m_human->GetEntityContext()->GetLimits()->GetReaction());
		m_human->Notify(EntityObserver::EV_REACTION_CHANGE);
	}

	//=====================================================================================//
	//                      void HumanContext::Traits::AddShockRes()                       //
	//=====================================================================================//
	void HumanContext::Traits::AddShockRes(int val)
	{
		if(m_human->IsRaised(EA_LOCK_TRAITS))
			return;

		m_shock_res += val;
		if(m_shock_res < 0) m_shock_res = 0; 
	}

	//=====================================================================================//
	//                      void HumanContext::Traits::AddFlameRes()                       //
	//=====================================================================================//
	void HumanContext::Traits::AddFlameRes(int val)
	{
		if(m_human->IsRaised(EA_LOCK_TRAITS))
			return;

		m_flame_res += val;
		if(m_flame_res < 0) m_flame_res = 0;
	}

	void HumanContext::Traits::AddMechanics(int val)
	{
		if(m_human->IsRaised(EA_LOCK_TRAITS))
			return;

		m_mechanics = saturate(m_mechanics + val, 0, m_human->GetEntityContext()->GetLimits()->GetMechanics());
		m_human->Notify(EntityObserver::EV_MECHANICS_CHANGE);
	}

	//=====================================================================================//
	//                      void HumanContext::Traits::AddDexterity()                      //
	//=====================================================================================//
	void HumanContext::Traits::AddDexterity(int val)
	{
		if(m_human->IsRaised(EA_LOCK_TRAITS))
			return;

		m_dexterity = saturate(m_dexterity + val, 0, m_human->GetEntityContext()->GetLimits()->GetDexterity());
		m_human->Notify(EntityObserver::EV_DEXTERITY_CHANGE);
	}

	//=====================================================================================//
	//                     void HumanContext::Traits::AddElectricRes()                     //
	//=====================================================================================//
	void HumanContext::Traits::AddElectricRes(int val)
	{
		if(m_human->IsRaised(EA_LOCK_TRAITS))
			return;

		m_electric_res += val;
		if(m_electric_res < 0) m_electric_res = 0;
	}

	//=====================================================================================//
	//                     void HumanContext::Traits::AddExperience()                      //
	//=====================================================================================//
	void HumanContext::Traits::AddExperience(int val)
	{
		if(m_human->IsRaised(EA_LOCK_TRAITS))
		{
			return;
		}

		int old_lvl = m_level;

		m_experience += val;
		if(m_experience < 0) m_experience = 0;

		int cur_lvl = LevelupMgr::GetInst()->GetCurrentLevel(m_experience);

		//��������� levelup?
		if(cur_lvl > old_lvl)
		{
			m_level = cur_lvl;
			m_human->Notify(EntityObserver::EV_LEVELUP);
			SndPlayer::GetInst()->Play(SndUtils::Snd2Str(SndUtils::SND_LEVELUP));
			AddLevelupPoints(AIUtils::CalcLevelupPoints(m_human, cur_lvl, old_lvl));
		}

		m_human->Notify(EntityObserver::EV_EXPERIENCE_CHANGE);
	}

	//=====================================================================================//
	//                    void HumanContext::Traits::PoLAddExperience()                    //
	//=====================================================================================//
	void HumanContext::Traits::PoLAddExperience(int val)
	{
		if(m_human->IsRaised(EA_LOCK_TRAITS))
		{
			return;
		}

		int old_lvl = m_level;

		m_experience += val;
		if(m_experience < 0) m_experience = 0;

		int cur_lvl = LevelupMgr::GetInst()->GetCurrentLevel(m_experience);

		//��������� levelup?
		if(cur_lvl > old_lvl)
		{
			m_level = cur_lvl;
			m_human->Notify(EntityObserver::EV_LEVELUP);
			AddLevelupPoints(AIUtils::CalcLevelupPoints(m_human, cur_lvl, old_lvl));
		}

		m_human->Notify(EntityObserver::EV_EXPERIENCE_CHANGE);
	}

	void HumanContext::Traits::AddFrontRadius(float val)
	{
		if(m_human->IsRaised(EA_LOCK_TRAITS))
			return;

		m_front_radius = saturate(m_front_radius + val, 0.0f, m_human->GetEntityContext()->GetLimits()->GetFrontRadius());
		m_human->Notify(EntityObserver::EV_FOS_CHANGE);
	}

	void HumanContext::Traits::AddLevelupPoints(int val)
	{
		if(m_human->IsRaised(EA_LOCK_TRAITS))
			return;

		m_levelup_points += val;
		if(m_levelup_points < 0) m_levelup_points = 0;
		m_human->Notify(EntityObserver::EV_LEVELUP_CHANGE);
	}

	void HumanContext::Traits::HandleInsertImplant(int parameter, int value)
	{
		switch(parameter){
case ImplantInfo::PT_SIGHT:
	m_const_front_radius += value;       
	m_human->Notify(EntityObserver::EV_FOS_CHANGE);
	break;

case ImplantInfo::PT_WISDOM:
	m_const_wisdom += value;
	m_human->Notify(EntityObserver::EV_WISDOM_CHANGE);
	break;

case ImplantInfo::PT_HEALTH:
	m_const_health += value;        
	m_human->Notify(EntityObserver::EV_MAXHEALTH_CHANGE);
	break;

case ImplantInfo::PT_STRENGTH:
	m_const_strength += value;
	m_human->Notify(EntityObserver::EV_STRENGTH_CHANGE);
	m_human->Notify(EntityObserver::EV_MAXMOVEPNTS_CHANGE);
	break;

case ImplantInfo::PT_REACTION:
	m_const_reaction += value;
	m_human->Notify(EntityObserver::EV_REACTION_CHANGE);
	break;

case ImplantInfo::PT_ACCURACY:
	m_const_accuracy += value;
	m_human->Notify(EntityObserver::EV_ACCURACY_CHANGE);
	break;

case ImplantInfo::PT_SHOCK_RES:
	m_const_shock_res += value;
	break;

case ImplantInfo::PT_FLAME_RES:
	m_const_flame_res += value;
	break;

case ImplantInfo::PT_ELECTRIC_RES:
	m_const_electric_res += value;
	break;
		}
	}

	int HumanContext::Traits::GetLevel() const
	{
		return m_level;
	}

	//======================================================================

	HumanContext::Limits::Limits(HumanEntity* human)
	:	m_human(human)
	{
		if(m_human)
		{
			if((m_human->GetInfo()->IsAntihero() || m_human->GetInfo()->IsHero())
				&& PoL::ExperienceSaver::inst()->hasHeroInformation(human->GetInfo()->GetRID()))
			{
				const PoL::ExperienceInfo &info = PoL::ExperienceSaver::inst()->getExperienceInfo(human->GetInfo()->GetRID());
				m_health = info.m_health;
				m_wisdom = info.m_wisdom;
				m_morale = info.m_morale;
				m_accuracy = info.m_accuracy;
				m_strength = info.m_strength;
				m_reaction = info.m_reaction;
				m_mechanics = info.m_mechanics;
				m_dexterity = info.m_dexterity;
				m_front_radius = info.m_front_radius;
			}
			else
			{
				m_health = m_human->GetInfo()->GetHealth();
				m_wisdom = m_human->GetInfo()->GetWisdom();
				m_morale = m_human->GetInfo()->GetMorale();
				m_accuracy = m_human->GetInfo()->GetAccuracy();
				m_strength = m_human->GetInfo()->GetStrength();
				m_reaction = m_human->GetInfo()->GetReaction();
				m_mechanics = m_human->GetInfo()->GetMechanics();
				m_dexterity = m_human->GetInfo()->GetDexterity();
				m_front_radius = m_human->GetInfo()->GetSightRadius();
			}
		}
	}

	void HumanContext::Limits::MakeSaveLoad(SavSlot& slot)
	{
		if(slot.IsSaving())
		{
			slot << m_health;
			slot << m_wisdom;
			slot << m_morale;
			slot << m_accuracy;
			slot << m_strength;
			slot << m_reaction;
			slot << m_mechanics;
			slot << m_dexterity;
			slot << m_front_radius;

			slot << m_human->GetEID();
		}
		else
		{
			slot >> m_health;
			slot >> m_wisdom;
			slot >> m_morale;
			slot >> m_accuracy;
			slot >> m_strength;
			slot >> m_reaction;
			slot >> m_mechanics;
			slot >> m_dexterity;
			slot >> m_front_radius;

			eid_t eid; slot >> eid;
			m_human = EntityPool::GetInst()->Get(eid)->Cast2Human();
		}
	}

	//=====================================================================================//
	//                    float HumanContext::Limits::GetWeight() const                    //
	//=====================================================================================//
	float HumanContext::Limits::GetWeight() const
	{
		return m_human->GetEntityContext()->GetTraits()->GetStrength()/PoL::Inv::MaxWeightFactor;
	}

	int HumanContext::Limits::GetWisdom() const
	{
		return m_wisdom;
	}

	int HumanContext::Limits::GetMorale() const
	{
		return m_morale;
	}

	int HumanContext::Limits::GetHealth() const
	{
		return m_health;
	}

	//=====================================================================================//
	//                    int HumanContext::Limits::GetMovepnts() const                    //
	//=====================================================================================//
	int HumanContext::Limits::GetMovepnts() const
	{    
		Traits* traits = m_human->GetEntityContext()->GetTraits();
		return PoL::Inv::MpsForTurnStrengthFactor * traits->GetStrength()
			- PoL::Inv::MpsForTurnWeightFactor * traits->GetWeight();
	}

	int HumanContext::Limits::GetAccuracy() const
	{
		return m_accuracy;
	}

	int HumanContext::Limits::GetStrength() const
	{
		return m_strength;
	}

	int HumanContext::Limits::GetReaction() const
	{
		return m_reaction;
	}

	int HumanContext::Limits::GetDexterity() const
	{
		return m_dexterity;
	}

	int HumanContext::Limits::GetMechanics() const
	{
		return m_mechanics;
	}

	float HumanContext::Limits::GetFrontRadius() const
	{
		return m_front_radius;
	}

	void HumanContext::Limits::SetHealth(int val)
	{
		if(m_human->IsRaised(EA_LOCK_TRAITS))
			return;

		m_health = val;
		m_human->Notify(EntityObserver::EV_MAXHEALTH_CHANGE);
	}

	void HumanContext::Limits::SetWisdom(int val)
	{
		if(m_human->IsRaised(EA_LOCK_TRAITS))
			return;

		m_wisdom = val;
	}

	void HumanContext::Limits::SetAccuracy(int val)
	{
		if(m_human->IsRaised(EA_LOCK_TRAITS))
			return;

		m_accuracy = val;
	}

	void HumanContext::Limits::SetStrength(int val)
	{
		if(m_human->IsRaised(EA_LOCK_TRAITS))
			return;

		m_strength = val;
		m_human->Notify(EntityObserver::EV_MAXMOVEPNTS_CHANGE);
	}

	void HumanContext::Limits::SetReaction(int val)
	{
		if(m_human->IsRaised(EA_LOCK_TRAITS))
			return;

		m_reaction = val;
	}

	void HumanContext::Limits::SetMechanics(int val)
	{
		if(m_human->IsRaised(EA_LOCK_TRAITS))
			return;

		m_mechanics = val;
	}

	void HumanContext::Limits::SetDexterity(int val)
	{
		if(m_human->IsRaised(EA_LOCK_TRAITS))
			return;

		m_dexterity = val;
	}

	void HumanContext::Limits::SetFrontRadius(float val)
	{
		if(m_human->IsRaised(EA_LOCK_TRAITS))
			return;

		m_front_radius = val;
	}

	//=====================================================================================//
	//                          VehicleContext::VehicleContext()                           //
	//=====================================================================================//
	VehicleContext::VehicleContext(VehicleEntity* vehicle)
		:	ActiveContext(vehicle),
		m_vehicle(vehicle),
		m_ammo_count(0)
	{
		if(m_vehicle)
		{
			m_health = m_vehicle->GetInfo()->GetHealth();
			m_movepnts = m_vehicle->GetInfo()->GetMovepnts();
			m_ammo_count = m_vehicle->GetInfo()->GetAmmoCount(); 
		}
	}

	//=====================================================================================//
	//                          VehicleContext::~VehicleContext()                          //
	//=====================================================================================//
	VehicleContext::~VehicleContext()
	{
	}

	//=====================================================================================//
	//                      damage_s HumanContext::GetDamage() const                       //
	//=====================================================================================//
	damage_s VehicleContext::GetDamage(int i) const
	{
		damage_s dtypes[DT_COUNT];

		const damage_s &wdb = GetParent()->Cast2Vehicle()->GetInfo()->GetBDmg();
		dtypes[wdb.m_type].m_val += wdb.m_val;
		dtypes[wdb.m_type].m_type = wdb.m_type;
		const damage_s &wda = GetParent()->Cast2Vehicle()->GetInfo()->GetADmg();
		dtypes[wda.m_type].m_val += wda.m_val;
		dtypes[wda.m_type].m_type = wda.m_type;

		return dtypes[i];
	}


	//=====================================================================================//
	//                      HumanEntity* VehicleContext::GetDriver()                       //
	//=====================================================================================//
	HumanEntity* VehicleContext::GetDriver()
	{
		return m_crew_strategy.get() ? m_crew_strategy->GetDriver(m_vehicle) : 0;
	}

	//=====================================================================================//
	//                       void VehicleContext::SetCrewStrategy()                        //
	//=====================================================================================//
	void VehicleContext::SetCrewStrategy(CrewStrategy* strategy)
	{
		m_crew_strategy.reset(strategy);
	}

	//=====================================================================================//
	//                      bool VehicleContext::CanJoinCrew() const                       //
	//=====================================================================================//
	bool VehicleContext::CanJoinCrew(HumanEntity* human) const
	{
		return m_crew_strategy.get() ? m_crew_strategy->CanJoin(m_vehicle, human) : false;
	}

	//=====================================================================================//
	//                           void VehicleContext::IncCrew()                            //
	//=====================================================================================//
	void VehicleContext::IncCrew(HumanEntity* human)
	{
		if(m_crew_strategy.get()) m_crew_strategy->Inc(m_vehicle, human);
		m_vehicle->Notify(EntityObserver::EV_CREW_CHANGE);
	}

	//=====================================================================================//
	//                           void VehicleContext::DecCrew()                            //
	//=====================================================================================//
	void VehicleContext::DecCrew(HumanEntity* human)
	{
		if(m_crew_strategy.get()) m_crew_strategy->Dec(m_vehicle, human);
		m_vehicle->Notify(EntityObserver::EV_CREW_CHANGE);
	}

	//=====================================================================================//
	//                        void VehicleContext::EnableShipment()                        //
	//=====================================================================================//
	void VehicleContext::EnableShipment(bool flag)
	{
		if(m_crew_strategy.get()) m_crew_strategy->Enable(m_vehicle, flag);
	}

	//=====================================================================================//
	//                       int VehicleContext::GetCrewSize() const                       //
	//=====================================================================================//
	int VehicleContext::GetCrewSize() const
	{
		return m_crew_strategy.get() ? m_crew_strategy->GetCrewSize(m_vehicle) : 0;
	}

	//=====================================================================================//
	//                         void VehicleContext::MakeSaveLoad()                         //
	//=====================================================================================//
	void VehicleContext::MakeSaveLoad(SavSlot& slot)
	{
		ActiveContext::MakeSaveLoad(slot);

		if(slot.IsSaving())
		{
			slot << m_vehicle->GetEID();

			slot << m_health;
			slot << m_movepnts;
			slot << m_ammo_count;

			slot << (m_crew_strategy.get() != 0);
			if(m_crew_strategy.get())
			{
				DynUtils::SaveObj(slot, m_crew_strategy.get());
				m_crew_strategy->MakeSaveLoad(slot);
			}
		}
		else
		{
			eid_t eid; slot >> eid;
			m_vehicle = EntityPool::GetInst()->Get(eid)->Cast2Vehicle();

			slot >> m_health;
			slot >> m_movepnts;
			slot >> m_ammo_count;

			bool flag; slot >> flag;

			if(flag)
			{
				CrewStrategy *cs = 0;
				DynUtils::LoadObj(slot, cs);
				m_crew_strategy.reset(cs);
				m_crew_strategy->MakeSaveLoad(slot);
			}
		}
	}

	int VehicleContext::GetHealth() const
	{
		return m_health;
	}

	void VehicleContext::SetHealth(int value)
	{
		if(m_vehicle->IsRaised(EA_LOCK_TRAITS))
			return;

		m_health = value > 0 ? value : 0;
		m_vehicle->Notify(EntityObserver::EV_HEALTH_CHANGE);
	}

	int VehicleContext::GetMovepnts() const
	{
		return m_movepnts;
	}

	void VehicleContext::SetMovepnts(int value)
	{
		if(m_vehicle->IsRaised(EA_LOCK_TRAITS))
			return;

		int old = m_movepnts;
		m_movepnts = std::max(value,0);
		if(m_movepnts != old) m_vehicle->Notify(EntityObserver::EV_MOVEPNTS_CHANGE);
	}

	void VehicleContext::PoLSetMovepnts(int value)
	{
		if(m_vehicle->IsRaised(EA_LOCK_TRAITS) || UnlimitedMoves()) return;

		int old = m_movepnts;
		m_movepnts = std::max(value,0);
		if(m_movepnts != old) m_vehicle->Notify(EntityObserver::EV_MOVEPNTS_CHANGE);
	}

	int VehicleContext::GetAmmoCount() const
	{
		return m_ammo_count;
	}

	void VehicleContext::SetAmmoCount(int value)
	{
		if(m_vehicle->IsRaised(EA_LOCK_TRAITS))
			return;

		m_ammo_count = value > 0 ? value : 0;
		m_vehicle->Notify(EntityObserver::EV_WEAPON_STATE_CHANGE);    
	}

	//======================================================================

	HumanEntity::HumanEntity(const rid_t& rid)
	:	BaseEntity(ET_HUMAN, rid),
		m_alwaysInPanic(false)
	{
		if(rid.size())
		{
			SetEntityContext(new HumanContext(this));
			SetGraph(GraphFactory::CreateGraphHuman(this));
			for(int i = 0; i < GetInfo()->GetShotBoneCount(); ++i)
			{
				GetGraph()->AddShotBone(GetInfo()->GetShotBone(i));
			}
		}
	}

	HumanEntity::~HumanEntity()
	{
		if(GetInfo()->IsHero() || GetInfo()->IsAntihero())
		{
			PoL::ExperienceSaver::inst()->storeExperienceInfo(this);
		}

		//��������� ���� � ����� �����������
		Notify(EntityObserver::EV_DESTROY);
		GameEvMessenger::GetInst()->Notify(GameObserver::EV_DESTROY,&GameObserver::entity_info(this));
	}

	HumanEntity* HumanEntity::Cast2Human()
	{
		return this;
	}

	void HumanEntity::Accept(EntityVisitor& visitor)
	{
		visitor.Visit(this);
	}

	GraphHuman* HumanEntity::GetGraph()
	{
		return static_cast<GraphHuman*>(BaseEntity::GetGraph());
	}

	HumanContext* HumanEntity::GetEntityContext()
	{
		return static_cast<HumanContext*>(BaseEntity::GetEntityContext());
	}

	const HumanInfo* HumanEntity::GetInfo() const
	{
		return static_cast<const HumanInfo*>(BaseEntity::GetInfo());
	}

	//======================================================================

	TraderEntity::TraderEntity(const rid_t& rid):
	BaseEntity(ET_TRADER, rid)
	{
		if(rid.size())
		{
			SetEntityContext(new TraderContext(this));
			SetGraph(GraphFactory::CreateGraphHuman(this));
			//for(int i = 0; i < GetInfo()->GetShotBoneCount(); ++i)
			//{
			//	GetGraph()->AddShotBone(GetInfo()->GetShotBone(i));
			//}
		}
	}

	TraderEntity::~TraderEntity()
	{
		//��������� ���� � ����� �����������
		Notify(EntityObserver::EV_DESTROY);
		GameEvMessenger::GetInst()->Notify(GameObserver::EV_DESTROY,&GameObserver::entity_info(this));
	}

	TraderEntity* TraderEntity::Cast2Trader()
	{
		return this;
	}

	void TraderEntity::Accept(EntityVisitor& visitor)
	{
		visitor.Visit(this);
	}

	GraphHuman* TraderEntity::GetGraph()
	{
		return static_cast<GraphHuman*>(BaseEntity::GetGraph());
	}

	TraderContext* TraderEntity::GetEntityContext()
	{
		return static_cast<TraderContext*>(BaseEntity::GetEntityContext());
	}

	const TraderInfo* TraderEntity::GetInfo() const
	{
		return static_cast<const TraderInfo*>(BaseEntity::GetInfo());
	}

	//======================================================================

	//=====================================================================================//
	//                           VehicleEntity::VehicleEntity()                            //
	//=====================================================================================//
	VehicleEntity::VehicleEntity(const rid_t& rid)
		:	BaseEntity(ET_VEHICLE, rid)
	{
		if(rid.size())
		{
			SetEntityContext(new VehicleContext(this));
			SetGraph(GraphFactory::CreateGraphVehicle(this));
			for(int i = 0; i < GetInfo()->GetShotBoneCount(); ++i)
			{
				GetGraph()->AddShotBone(GetInfo()->GetShotBone(i));
			}
		}
	}

	VehicleEntity::~VehicleEntity()
	{
		//��������� ���� � ����� �����������
		Notify(EntityObserver::EV_DESTROY);
		GameEvMessenger::GetInst()->Notify(GameObserver::EV_DESTROY,&GameObserver::entity_info(this));
	}

	VehicleEntity* VehicleEntity::Cast2Vehicle()
	{
		return this;
	}

	void VehicleEntity::Accept(EntityVisitor& visitor)
	{
		visitor.Visit(this);
	}

	GraphVehicle* VehicleEntity::GetGraph()
	{
		return static_cast<GraphVehicle*>(BaseEntity::GetGraph());
	}

	VehicleContext* VehicleEntity::GetEntityContext()
	{
		return static_cast<VehicleContext*>(BaseEntity::GetEntityContext());
	}

	const VehicleInfo* VehicleEntity::GetInfo() const
	{
		return static_cast<const VehicleInfo*>(BaseEntity::GetInfo());
	}

	//======================================================================

//=====================================================================================//
//                          EntityPool* EntityPool::GetInst()                          //
//=====================================================================================//
EntityPool* EntityPool::GetInst()
{
	static EntityPool imp;
	return &imp;
}

//=====================================================================================//
//                              EntityPool::EntityPool()                               //
//=====================================================================================//
EntityPool::EntityPool() 
{
	memset(m_ents, 0, sizeof(m_ents));
}

//=====================================================================================//
//                              EntityPool::~EntityPool()                              //
//=====================================================================================//
EntityPool::~EntityPool()
{
	for(int k = 0; k < MAX_ENTITIES; delete m_ents[k++]);
}

//=====================================================================================//
//                           void EntityPool::MakeSaveLoad()                           //
//=====================================================================================//
void EntityPool::MakeSaveLoad(SavSlot& st)
{
	if(st.IsSaving())
	{
		//�������� �������
		for(eid_t k = 0; k < MAX_ENTITIES; k++)
		{
			if(m_ents[k] == 0) continue;

			st << k;
			DynUtils::SaveObj(st, m_ents[k]);
		}

		//������� �����
		st << 0;
	}
	else
	{
		eid_t eid;
		st >> eid;

		int count = 0;

		//����������� ������� � ������� �������
		while(eid)
		{
			count ++;

			DynUtils::LoadObj(st, m_ents[eid]);
			m_ents[eid]->SetEID(eid);
			st >> eid;
		}        

		GameObserver::entload_info info(count);
		GameEvMessenger::GetInst()->Notify(GameObserver::EV_INIT_ENTLOAD, &info);
	}

	//������ ��������/����������� ��������� ��������
	for(eid_t k = 0; k < MAX_ENTITIES; k++)
	{
		if(BaseEntity* entity = m_ents[k])
		{
			entity->MakeSaveLoad(st);

			if(!st.IsSaving())
			{
				GameObserver::entity_info info(entity);
				GameEvMessenger::GetInst()->Notify(GameObserver::EV_ENITITY_LOADED, &info);
			}
		}
	}
}

//=====================================================================================//
//                              void EntityPool::Insert()                              //
//=====================================================================================//
void EntityPool::Insert(BaseEntity* entity)
{  
	for(int i = 1; i < MAX_ENTITIES; i++)
	{
		if(m_ents[i] == 0)
		{
			entity->SetEID(i);
			m_ents[i] = entity;
			return;
		}
	}

	throw CASUS("EntityPool: ������������ ������� �������!");    
}

//=====================================================================================//
//                              void EntityPool::Remove()                              //
//=====================================================================================//
void EntityPool::Remove(BaseEntity* entity)
{
	if(m_ents[entity->GetEID()] == entity)
	{
		m_ents[entity->GetEID()] = 0;
		delete entity;
		return;
	}

	throw CASUS("EntityPool: �������� id ��������!");
}

//=====================================================================================//
//              EntityPool::Iterator& EntityPool::Iterator::operator ++()              //
//=====================================================================================//
EntityPool::Iterator& EntityPool::Iterator::operator ++()
{
	for(++m_first; m_first < m_last; ++m_first)
	{
		if(     *m_first
			&& (*m_first)->GetType() & m_type
			&& (*m_first)->GetPlayer() & m_team
			&& (*m_first)->GetAttributes() & m_attr)
			return *this;
	}

	m_first = m_last;
	return *this;
}

//=====================================================================================//
//                      EntityPool::iterator EntityPool::begin()                       //
//=====================================================================================//
EntityPool::iterator EntityPool::begin(unsigned etype, unsigned ptype, unsigned traits)
{
	return Iterator(m_ents, m_ents + MAX_ENTITIES, etype, ptype, traits);
}

//=====================================================================================//
//                       EntityPool::iterator EntityPool::end()                        //
//=====================================================================================//
EntityPool::iterator EntityPool::end()
{
	return Iterator(m_ents + MAX_ENTITIES, m_ents + MAX_ENTITIES, 0, 0, 0);
}

//=====================================================================================//
//                            BaseEntity* EntityPool::Get()                            //
//=====================================================================================//
BaseEntity* EntityPool::Get(eid_t id)
{
	if(id && id < MAX_ENTITIES) return m_ents[id];
	throw CASUS("EntityPool: �������� id ��������!");
}

//=====================================================================================//
//                          eid_t EntityPool::TraceEntities()                          //
//=====================================================================================//
eid_t EntityPool::TraceEntities(const point3 &o, const point3 &d, eid_t skip, point3 *pnt)
{
	ray r(o,d);
	float mindist = 1e+10f;
	eid_t result = 0;

	for(iterator i = begin(); i != end(); ++i)
	{
		if(i->GetEID() == skip) continue;
		BaseEntity *e = &*i;

		float curdist;
		point3 norm;
		if(e->GetGraph()->TraceRay(r,&curdist,&norm))
		{
			if(mindist > curdist)
			{
				mindist = curdist;
				result = e->GetEID();
			}
		}
	}

	if(result && pnt)
	{
		*pnt = o + mindist*d;
	}

	return result;
}

	//======================================================================

	BaseEntity* EntityBuilder::CreateHuman(const rid_t& rid)
	{
		HumanEntity*  human = new HumanEntity(rid);
		HumanContext* context = human->GetEntityContext();

		//���������� ��������� 
		context->SetMoveStrategy(StrategyFactory::CreateMove(StrategyFactory::MT_HUMAN, human));
		context->SetDeathStrategy(StrategyFactory::CreateDeath(StrategyFactory::DT_HUMAN, human));
		context->SetMarker(StrategyFactory::CreateMarker(StrategyFactory::MRK_USUAL, human));
		context->SetSpectator(StrategyFactory::CreateSpectator(StrategyFactory::ST_USUAL, human));
		context->SetPanicSelector(StrategyFactory::CreatePanicSelector(StrategyFactory::PST_USUAL));
		context->SetSoundStrategy(StrategyFactory::CreateSound(StrategyFactory::SND_HUMAN, human));

		//��������� � ������ �������
		EntityPool::GetInst()->Insert(human);

		return human;
	}

	BaseEntity* EntityBuilder::CreateTrader(const rid_t& rid)
	{
		TraderEntity*  trader  = new TraderEntity(rid);
		TraderContext* context = trader->GetEntityContext();

		//���������� ���������
		context->SetDeathStrategy(StrategyFactory::CreateDeath(StrategyFactory::DT_TRADER, trader));
		context->SetMarker(StrategyFactory::CreateMarker(StrategyFactory::MRK_USUAL, trader));
		context->SetSpectator(StrategyFactory::CreateSpectator(StrategyFactory::ST_TRADER, trader));
		context->SetSoundStrategy(StrategyFactory::CreateSound(StrategyFactory::SND_TRADER, trader));

		//��������� � ������ �������
		EntityPool::GetInst()->Insert(trader);

		return trader;
	}

	BaseEntity* EntityBuilder::CreateVehicle(const rid_t& rid)
	{
		VehicleEntity*  vehicle = new VehicleEntity(rid);
		VehicleContext* context = vehicle->GetEntityContext();

		//���������� ���������
		context->SetCrewStrategy(StrategyFactory::CreateCrew(StrategyFactory::CT_USUAL, vehicle));
		context->SetMoveStrategy(StrategyFactory::CreateMove(StrategyFactory::MT_VEHICLE, vehicle));
		context->SetDeathStrategy(StrategyFactory::CreateDeath(StrategyFactory::DT_VEHICLE, vehicle));
		context->SetMarker(StrategyFactory::CreateMarker(StrategyFactory::MRK_USUAL, vehicle));
		context->SetSpectator(StrategyFactory::CreateSpectator(StrategyFactory::ST_USUAL, vehicle));

		//��������� � ������ �������
		EntityPool::GetInst()->Insert(vehicle);

		return vehicle;
	}

	void EntityBuilder::SetPlayer(BaseEntity* entity, player_type player)
	{
		entity->DropFlags(entity->GetPlayer());
		entity->RaiseFlags(player);
	}

	void EntityBuilder::SetAIModel(BaseEntity* entity, const std::string& label)
	{
		entity->GetEntityContext()->SetAIModel(label);
	}

	bool EntityBuilder::CanTake(BaseEntity* entity, BaseThing* thing, human_pack_type pack)
	{
		CanTakeThing can_take(thing, pack);
		entity->Accept(can_take);
		return can_take.GetResult();
	}

	void EntityBuilder::GiveThing(BaseEntity* entity, BaseThing* thing, human_pack_type pack)
	{    
		ThingGiver giver(thing, pack);
		entity->Accept(giver);
	}

	bool EntityBuilder::IsSuitableLocation(const ipnt2_t& pos, VehicleInfo::vehicle_size vsize)
	{
		return PathUtils::IsEmptyPlace(pos,vsize);
	}

	void EntityBuilder::LinkEntity(BaseEntity* entity, const ipnt2_t& pos, float angle)
	{
		if(entity->IsRaised(EA_NOT_INCREW)){
			entity->GetGraph()->SetLoc(pos, angle);    
			PathUtils::LinkEntity(entity);
		}

		EntityContext* context = entity->GetEntityContext();

		VisMap::GetInst()->Insert(context->GetMarker());
		if(context->GetSpectator()) VisMap::GetInst()->Insert(context->GetSpectator());
	}

	void EntityBuilder::UnlinkEntity(BaseEntity* entity)
	{
		PathUtils::UnlinkEntity(entity);

		EntityContext* context = entity->GetEntityContext();

		VisMap::GetInst()->Remove(context->GetMarker());
		if(context->GetSpectator())  VisMap::GetInst()->Remove(context->GetSpectator());
	}

	bool EntityBuilder::GenerateSuitablePos(const pnt_vec_t& pnts, ipnt2_t* pos, VehicleInfo::vehicle_size vsize)
	{
		//������ ����� ��������� �������
		for(size_t k = 0; k < pnts.size(); k++)
		{        
			*pos = pnts[aiRangeRand(pnts.size())];

			if(IsSuitableLocation(*pos,vsize))
			{
				return true;
			}
		}

		//������ ������ ���������
		for(size_t k = 0; k < pnts.size(); k++)
		{
			if(IsSuitableLocation(pnts[k],vsize))
			{
				*pos = pnts[k];
				return true;
			}                
		}

		return false;
	}

	void EntityBuilder::SetSpawnZone(BaseEntity* entity, const std::string& info)
	{
		entity->GetEntityContext()->SetSpawnZone(info);
	}

	//======================================================================

	namespace{

		//
		// ����� �������� AI ��������� � ���������
		//
		class CommonAIContext : public AIContext
		{
		public:

			DCTOR_DEF(CommonAIContext)

				CommonAIContext(const std::string& sub = std::string()) : m_subteam(sub) {}

				void MakeSaveLoad(SavSlot& slot)
				{
					if(slot.IsSaving())
						slot << m_subteam;
					else
						slot >> m_subteam;
				}

				const std::string& getSubteam() const { return m_subteam; }

		private:

			std::string  m_subteam;
		};

		//
		// �������� AI ��� ������ (�������� - ��������� ����� 100 ���)
		//
		class EnemyAIContext : public CommonAIContext{
		public:        

			DCTOR_DEF(EnemyAIContext)
		};

		//
		// �������� AI ��� ������  (�������� - ��������� ����� 100 ���)
		//
		class CivilianAIContext : public CommonAIContext
		{
		public:

			DCTOR_DEF(CivilianAIContext)
		};

		DCTOR_IMP(EnemyAIContext)
		DCTOR_IMP(CommonAIContext)
		DCTOR_IMP(CivilianAIContext)
	}

	//������� �������� AI ����� ����
	AIContext* AIContextFactory::Create(ai_type type, const std::string& subteam)
	{
		return new CommonAIContext(subteam);
	}

	AIContextFactory* AIContextFactory::GetInst()
	{
		static AIContextFactory imp;
		return &imp;
	}

	EntityObserver::~EntityObserver() 
	{
		//Grom
		//���� ������ �� ���� �� ������ ������������ ������� �� �������
		EntityPool *p = EntityPool::GetInst();
		EntityPool::iterator it,ite=p->end();
		for(it = p->begin();it!=ite;it++)
		{
			if(it->IsAttached(this))//�� ���� ����� �������� ������� �� ������ ���������
			{
				//throw CASUS("�� ������ �����������!"); 
				it->Detach(this);
			}
		}
	}

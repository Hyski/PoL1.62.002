#pragma warning(disable:4786)

#include "logicdefs.h"

#include "Thing.h"
#include "entity.h"
#include "bureau.h"
#include "Graphic.h"
#include "AIUtils.h"
#include "hexgrid.h"
#include "xlsreader.h"
#include "Spectator.h"
#include "entityaux.h"
#include "tableutils.h"
#include "dirtylinks.h"
#include "thingfactory.h"
#include "StoreBox.h"

#include <algorithm>

DCTOR_IMP(KeyThing)
DCTOR_IMP(AmmoThing)
DCTOR_IMP(ArmorThing)
DCTOR_IMP(MoneyThing)
DCTOR_IMP(CameraThing)
DCTOR_IMP(ShieldThing)
DCTOR_IMP(WeaponThing)
DCTOR_IMP(ScannerThing)
DCTOR_IMP(GrenadeThing)
DCTOR_IMP(ImplantThing)
DCTOR_IMP(MedikitThing)


//=======================================================================

namespace ThingDetails
{
	const char* money_rid = "money";

	const std::string ammo_xls = "scripts/logic/ammunition.txt";
	const char* equip_xls = "scripts/logic/equipment.txt";
	const char* armor_xls = "scripts/logic/armors.txt";
	const std::string weapon_xls = "scripts/logic/weapons.txt";
	const std::string grenade_xls = "scripts/logic/grenades.txt";
	const char* item_sets_xls1 = "scripts/logic/item_sets.txt";
	const char* item_sets_xls2 = "scripts/spawn/item_sets.txt";

	std::istringstream& init(std::istringstream& istr, const std::string& str)
	{
		istr.str(str);
		istr.clear();
		return istr;
	}

	void operator >> (std::istringstream& istr, GrenadeInfo::grenade_type& type)
	{
		if(istr.str() == "grenade")
			type = GrenadeInfo::GRENADE;
		else if(istr.str() == "mine")
			type = GrenadeInfo::MINE;
		else
			throw CASUS("Непонятный тип гранат!");
	}

	void operator >> (std::istringstream& istr, hold_type& type)
	{
		if(istr.str() == "none") 
			type = HT_NONE;
		else if(istr.str() == "rifle")
			type = HT_RIFLE;
		else if(istr.str() == "pistol")
			type = HT_PISTOL;
		else if(istr.str() == "cannon")
			type = HT_CANNON;
		else if(istr.str() == "autocannon")
			type = HT_AUTOCANNON;
		else if(istr.str() == "rocket")
			type = HT_ROCKET;
		else if(istr.str() == "special")
			type = HT_SPECIAL;
		else if(istr.str() == "grenade")
			type = HT_GRENADE;
		else if(istr.str() == "freehands")
			type = HT_FREEHANDS;
		else       
			throw CASUS("Thing: непонятный hold_type");    
	}

	void operator >> (std::istringstream& istr, ImplantInfo::parameter_type& type) 
	{
		if(istr.str() == "health")
			type = ImplantInfo::PT_HEALTH;
		else if(istr.str() == "sight")
			type = ImplantInfo::PT_SIGHT;
		else if(istr.str() == "strength")
			type = ImplantInfo::PT_STRENGTH;
		else if(istr.str() == "reaction")
			type = ImplantInfo::PT_REACTION;
		else if(istr.str() == "wisdom")
			type = ImplantInfo::PT_WISDOM;
		else if(istr.str() == "accuracy")
			type = ImplantInfo::PT_ACCURACY;
		else if(istr.str() == "shock")
			type = ImplantInfo::PT_SHOCK_RES;
		else if(istr.str() == "flame")
			type = ImplantInfo::PT_FLAME_RES;
		else if(istr.str() == "electric")
			type = ImplantInfo::PT_ELECTRIC_RES;
		else     
			throw CASUS("Непонятный тип эффекта для оборудования!");
	}

	std::string Parameter2Str(ImplantInfo::parameter_type type)
	{
		switch(type){
		case ImplantInfo::PT_HEALTH: return DirtyLinks::GetStrRes("tip_health");
		case ImplantInfo::PT_SIGHT: return DirtyLinks::GetStrRes("tip_sight");
		case ImplantInfo::PT_STRENGTH: return DirtyLinks::GetStrRes("tip_strength");
		case ImplantInfo::PT_REACTION: return DirtyLinks::GetStrRes("tip_reaction");
		case ImplantInfo::PT_WISDOM: return DirtyLinks::GetStrRes("tip_wisdom");
		case ImplantInfo::PT_ACCURACY: return DirtyLinks::GetStrRes("tip_accuracy");
		case ImplantInfo::PT_SHOCK_RES: return DirtyLinks::GetStrRes("tip_shock");
		case ImplantInfo::PT_FLAME_RES: return DirtyLinks::GetStrRes("tip_flame");
		case ImplantInfo::PT_ELECTRIC_RES: return DirtyLinks::GetStrRes("tip_electric");
		}

		return DirtyLinks::GetStrRes("tip_unknown");
	}

	void operator >> (std::istringstream& istr, armor_type& type)
	{
		if(istr.str() == "norm")
			type = ARM_NORM;
		else if(istr.str() == "suit1")
			type = ARM_SUIT1;
		else if(istr.str() == "suit2")
			type = ARM_SUIT2;
		else if(istr.str() == "suit3")
			type = ARM_SUIT3;
		else if(istr.str() == "suit4")
			type = ARM_SUIT4;
		else if(istr.str() == "scuba")
			type = ARM_SCUBA;
		else        
			throw CASUS("Неверный тип костюма!");
	}
}

using namespace ThingDetails;

//=======================================================================

class GrenadeInfoReader : public InfoReader<GrenadeInfoReader>
{
public:

#define POL_GRENADE_INFO_TABLE(P,D,DELIM)										\
	P(m_sys_name,				"sys_name",				REQUIRED, D) DELIM()	\
	P(m_name,					"name",					OPTIONAL, D) DELIM()	\
	P(m_type,					"type",					REQUIRED, D) DELIM()	\
	P(m_initials,				"initials",				OPTIONAL, D) DELIM()	\
	P(m_hold_type,				"hold_type",			REQUIRED, D) DELIM()	\
	P(m_strength,				"strength",				REQUIRED, D) DELIM()	\
	P(m_wisdom,					"wisdom",				REQUIRED, D) DELIM()	\
	P(m_weight,					"weight",				REQUIRED, D) DELIM()	\
	P(m_mp_to_activate,			"mp_to_activate",		REQUIRED, D) DELIM()	\
	P(m_basic_dmg,				"basic_dmg",			REQUIRED, D) DELIM()	\
	P(m_additional_dmg,			"additional_dmg",		REQUIRED, D) DELIM()	\
	P(m_basic_dmg_type,			"basic_dmg_type",		REQUIRED, D) DELIM()	\
	P(m_additional_dmg_type,	"additional_dmg_type",	REQUIRED, D) DELIM()	\
	P(m_basic_cost,				"basic_cost",			REQUIRED, D) DELIM()	\
	P(m_item_shader,			"item_shader",			REQUIRED, D) DELIM()	\
	P(m_item_width,				"item_width",			REQUIRED, D) DELIM()	\
	P(m_item_height,			"item_height",			REQUIRED, D) DELIM()	\
	P(m_description,			"description",			OPTIONAL, D) DELIM()	\
	P(m_hold_model,				"hold_model",			REQUIRED, D) DELIM()	\
	P(m_item_model,				"item_model",			REQUIRED, D) DELIM()	\
	P(m_range,					"range",				REQUIRED, D) DELIM()	\
	P(m_hit_effect,				"hit_effect",			REQUIRED, D) DELIM()	\
	P(m_min_rank,				"min_rank",				REQUIRED, D)

	POL_GRENADE_INFO_TABLE(POL_MAKE_DECLARATION,(GrenadeInfoReader),POL_EMPTY)

	static const std::string &GetTableName()
	{
		return grenade_xls;
	}

	static void ReadColumns()
	{
		table::row_type hdr = GetHdr();
		std::ostringstream errout;

		POL_GRENADE_INFO_TABLE(POL_MAKE_INITIALIZATION,(GrenadeInfoReader),POL_EMPTY)

		if(!errout.str().empty())
		{
			throw CASUS(errout.str());
		}
	}

	static void Read(GrenadeInfo& info)
	{
		using mll::utils::table;

		int line_count = GetTbl().height();

        //найдем нужную строку
        for(int line = 1; line < line_count; ++line)
		{
            //прочтем строку
			if(m_sys_name[line] == info.m_rid)
			{
                std::istringstream istr;

				info.m_name = PoL::getLocStr("grenades." + info.m_rid + ".name");//m_name[line];
				info.m_initials = PoL::getLocStr("grenades." + info.m_rid + ".initials");//m_initials[line];
				init(istr,m_type[line]) >> info.m_gtype;
				init(istr,m_strength[line]) >> info.m_strength;
				init(istr,m_wisdom[line]) >> info.m_wisdom;
				init(istr,m_weight[line]) >> info.m_weight;
				init(istr,m_mp_to_activate[line]) >> info.m_mp2act;
				init(istr,m_basic_dmg[line]) >> info.m_bdmg.m_val;
				init(istr,m_additional_dmg[line]) >> info.m_admg.m_val;
				info.m_bdmg.m_type = AIUtils::Str2Dmg(m_basic_dmg_type[line]);
				info.m_admg.m_type = AIUtils::Str2Dmg(m_additional_dmg_type[line]);
				init(istr,m_basic_cost[line]) >> info.m_cost;
				info.m_shader = m_item_shader[line];
				init(istr,m_item_width[line]) >> info.m_size.x;
				init(istr,m_item_height[line]) >> info.m_size.y;
				info.m_desc = PoL::getLocStr("grenades." + info.m_rid + ".description");//m_description[line];
				info.m_hold_model = m_hold_model[line];
				info.m_item_model = m_item_model[line];
				init(istr,m_range[line]) >> info.m_dmg_radius;
				info.m_hit_effect = m_hit_effect[line];
				init(istr,m_min_rank[line]) >> info.m_rank;

				return;
			}
		}

		throw CASUS(("GrenadeInfoReader: не найден profile для <" + info.m_rid + ">!").c_str());
	}
};

POL_GRENADE_INFO_TABLE(POL_MAKE_DEFINITION,(GrenadeInfoReader),POL_EMPTY)

//=======================================================================

class AmmoInfoReader : public InfoReader<AmmoInfoReader>
{
public:
#define POL_AMMO_INFO_TABLE(P,D,DELIM)												\
	P(m_sys_name,				"sys_name",					REQUIRED, D) DELIM()	\
	P(m_name,					"name",						OPTIONAL, D) DELIM()	\
	P(m_initials,				"initials",					OPTIONAL, D) DELIM()	\
	P(m_caliber,				"caliber",					REQUIRED, D) DELIM()	\
	P(m_basic_damage,			"basic_damage",				REQUIRED, D) DELIM()	\
	P(m_additional_damage,		"additional_damage",		REQUIRED, D) DELIM()	\
	P(m_basic_damage_type,		"basic_damage_type",		REQUIRED, D) DELIM()	\
	P(m_additional_damage_type,	"additional_damage_type",	REQUIRED, D) DELIM()	\
	P(m_range,					"range",					REQUIRED, D) DELIM()	\
	P(m_falloff_level,			"falloff_level",			REQUIRED, D) DELIM()	\
	P(m_basic_quality,			"basic_quality",			REQUIRED, D) DELIM()	\
	P(m_file_name,				"file_name",				REQUIRED, D) DELIM()	\
	P(m_shot_name,				"shot_name",				REQUIRED, D) DELIM()	\
	P(m_trace_effect,			"trace_effect",				REQUIRED, D) DELIM()	\
	P(m_hit_effect,				"hit_effect",				REQUIRED, D) DELIM()	\
	P(m_basic_cost,				"basic_cost",				REQUIRED, D) DELIM()	\
	P(m_description,			"description",				OPTIONAL, D) DELIM()	\
	P(m_item_shader,			"item_shader",				REQUIRED, D) DELIM()	\
	P(m_item_width,				"item_width",				REQUIRED, D) DELIM()	\
	P(m_item_height,			"item_height",				REQUIRED, D) DELIM()	\
	P(m_item_model,				"item_model",				REQUIRED, D) DELIM()	\
	P(m_damage_radius,			"damage_radius",			REQUIRED, D) DELIM()	\
	P(m_shells_effect,			"shells_effect",			REQUIRED, D) DELIM()	\
	P(m_damage_falloff,			"damage_falloff",			OPTIONAL, D)

	POL_AMMO_INFO_TABLE(POL_MAKE_DECLARATION,(AmmoInfoReader),POL_EMPTY)

	static const std::string &GetTableName()
	{
		return ammo_xls;
	}

	static void ReadColumns()
	{
		table::row_type hdr = GetHdr();
		std::ostringstream errout;

		POL_AMMO_INFO_TABLE(POL_MAKE_INITIALIZATION,(AmmoInfoReader),POL_EMPTY)

		if(!errout.str().empty())
		{
			throw CASUS(errout.str());
		}
	}

	static void Read(AmmoInfo& info)
	{
		using mll::utils::table;

		int line_count = GetTbl().height();

        //найдем нужную строку
        for(int line = 1; line < line_count; ++line)
		{
            //прочтем строку
			if(m_sys_name[line] == info.m_rid)
			{
                std::istringstream istr;

				info.m_weight = 0;
				info.m_damage_falloff = 0.0f;

				info.m_name = PoL::getLocStr("ammunition." + m_sys_name[line] + ".name");//m_name[line];
				info.m_initials = PoL::getLocStr("ammunition." + m_sys_name[line] + ".initials");//m_initials[line];
				init(istr,m_caliber[line]) >> info.m_caliber;
				init(istr,m_basic_damage[line]) >> info.m_bdmg.m_val;
				init(istr,m_additional_damage[line]) >> info.m_admg.m_val;
				info.m_bdmg.m_type = AIUtils::Str2Dmg(m_basic_damage_type[line]);
				info.m_admg.m_type = AIUtils::Str2Dmg(m_additional_damage_type[line]);
				init(istr,m_range[line]) >> info.m_range;
				init(istr,m_falloff_level[line]) >> info.m_falloff;
				init(istr,m_basic_quality[line]) >> info.m_quality;
				info.m_trace_effect = m_trace_effect[line];
				info.m_hit_effect = m_hit_effect[line];
				init(istr,m_basic_cost[line]) >> info.m_cost;
				info.m_desc = PoL::getLocStr("ammunition." + m_sys_name[line] + ".description");//m_description[line];
				info.m_shader = m_item_shader[line];
				init(istr,m_item_width[line]) >> info.m_size.x;
				init(istr,m_item_height[line]) >> info.m_size.y;
				info.m_item_model = m_item_model[line];
				init(istr,m_damage_radius[line]) >> info.m_dmg_radius;
				info.m_shells_effect = m_shells_effect[line];
				if(m_damage_falloff) init(istr,m_damage_falloff[line]) >> info.m_damage_falloff;

				return;
			}
		}

		throw CASUS(("AmmoInfoCacheImp: не найден profile для <" + info.m_rid + ">!").c_str());
	}
};

POL_AMMO_INFO_TABLE(POL_MAKE_DEFINITION,(AmmoInfoReader),POL_EMPTY)

//=======================================================================

class WeaponInfoReader : public InfoReader<WeaponInfoReader>
{
public:
#define POL_WEAPON_INFO_TABLE(P,D,DELIM)											\
	P(m_sys_name,				"sys_name",					REQUIRED, D) DELIM()	\
	P(m_basic_cost,				"basic_cost",				REQUIRED, D) DELIM()	\
	P(m_hold_type,				"hold_type",				REQUIRED, D) DELIM()	\
	P(m_strength,				"strength",					REQUIRED, D) DELIM()	\
	P(m_wisdom,					"wisdom",					REQUIRED, D) DELIM()	\
	P(m_caliber,				"caliber",					REQUIRED, D) DELIM()	\
	P(m_weight,					"weight",					REQUIRED, D) DELIM()	\
	P(m_qty_in_aim_shot,		"qty_in_aim_shot",			REQUIRED, D) DELIM()	\
	P(m_qty_in_auto_shot,		"qty_in_auto_shot",			REQUIRED, D) DELIM()	\
	P(m_mp_to_shot,				"mp_to_shot",				REQUIRED, D) DELIM()	\
	P(m_basic_damage,			"basic_damage",				REQUIRED, D) DELIM()	\
	P(m_additional_damage,		"additional_damage",		REQUIRED, D) DELIM()	\
	P(m_basic_damage_type,		"basic_damage_type",		REQUIRED, D) DELIM()	\
	P(m_additional_damage_type,	"additional_damage_type",	REQUIRED, D) DELIM()	\
	P(m_range,					"range",					REQUIRED, D) DELIM()	\
	P(m_basic_quality,			"basic_quality",			REQUIRED, D) DELIM()	\
	P(m_basic_accuracy,			"basic_accuracy",			REQUIRED, D) DELIM()	\
	P(m_flash_effect_name,		"flash_effect_name",		REQUIRED, D) DELIM()	\
	P(m_item_shader,			"item_shader",				REQUIRED, D) DELIM()	\
	P(m_item_width,				"item_width",				REQUIRED, D) DELIM()	\
	P(m_item_height,			"item_height",				REQUIRED, D) DELIM()	\
	P(m_hold_model,				"hold_model",				REQUIRED, D) DELIM()	\
	P(m_item_model,				"item_model",				REQUIRED, D) DELIM()	\
	P(m_barrel,					"barrel",					REQUIRED, D) DELIM()	\
	P(m_shell,					"shell",					REQUIRED, D) DELIM()	\
	P(m_shot_sound,				"shot_sound",				REQUIRED, D) DELIM()	\
	P(m_autoshot_sound,			"autoshot_sound",			REQUIRED, D) DELIM()	\
	P(m_ammo_load_sound,		"ammo_load_sound",			REQUIRED, D) DELIM()	\
	P(m_mp2reload,				"mp2reload",				REQUIRED, D) DELIM()	\
	P(m_min_rank,				"min_rank",					REQUIRED, D) DELIM()	\
	P(m_max_scatter,			"max_scatter",				OPTIONAL, D)

	POL_WEAPON_INFO_TABLE(POL_MAKE_DECLARATION,(WeaponInfoReader),POL_EMPTY)

	static const std::string &GetTableName()
	{
		return weapon_xls;
	}

	static void ReadColumns()
	{
		table::row_type hdr = GetHdr();
		std::ostringstream errout;

		POL_WEAPON_INFO_TABLE(POL_MAKE_INITIALIZATION,(WeaponInfoReader),POL_EMPTY)

		if(!errout.str().empty())
		{
			throw CASUS(errout.str());
		}
	}

	static void Read(WeaponInfo &info)
	{
		using mll::utils::table;

		int line_count = GetTbl().height();

        //найдем нужную строку
        for(int line = 1; line < line_count; ++line)
		{
            //прочтем строку
			if(m_sys_name[line] == info.m_rid)
			{
				char ch;
                std::istringstream istr;

				info.m_name = PoL::getLocStr("weapons." + info.m_rid + ".name");
				info.m_initials = PoL::getLocStr("weapons." + info.m_rid + ".initials");
				info.m_desc = PoL::getLocStr("weapons." + info.m_rid + ".description");
				
				info.m_max_scatter = 10.0f;

				init(istr, m_basic_cost[line]) >> info.m_cost;
				init(istr, m_hold_type[line]) >> info.m_htype;
				init(istr, m_strength[line]) >> info.m_strength;
				init(istr, m_wisdom[line]) >> info.m_wisdom;
				init(istr, m_caliber[line]) >> info.m_caliber;
				init(istr, m_weight[line]) >> info.m_weight;
				init(istr, m_qty_in_aim_shot[line]) >> info.m_aim_qty;
				init(istr, m_qty_in_auto_shot[line]) >> info.m_auto_qty;
				init(istr, m_mp_to_shot[line]) >> info.m_movepnts;
				init(istr, m_basic_damage[line]) >> info.m_bdmg.m_val;
				init(istr, m_additional_damage[line]) >> info.m_admg.m_val;
				info.m_bdmg.m_type = AIUtils::Str2Dmg(m_basic_damage_type[line]); 
				info.m_admg.m_type = AIUtils::Str2Dmg(m_additional_damage_type[line]); 
				init(istr, m_range[line]) >> info.m_range;
				init(istr, m_basic_quality[line]) >> info.m_quality;
				init(istr, m_basic_accuracy[line]) >> info.m_accuracy;
				info.m_flash_effect = m_flash_effect_name[line];
				info.m_shader = m_item_shader[line];
				init(istr, m_item_width[line]) >> info.m_size.x;
				init(istr, m_item_height[line]) >> info.m_size.y;
				info.m_hold_model = m_hold_model[line];
				info.m_item_model = m_item_model[line];
				init(istr, m_barrel[line]) >> info.m_barrel.x >> ch >> info.m_barrel.y >> ch >> info.m_barrel.z;
				init(istr, m_shell[line]) >> info.m_shell.x >> ch >> info.m_shell.y >> ch >> info.m_shell.z;
				info.m_shot_sound = m_shot_sound[line];
				info.m_autoshot_sound = m_autoshot_sound[line];
				info.m_ammo_load_sound = m_ammo_load_sound[line];
				init(istr,m_mp2reload[line]) >> info.m_mp2reload;
				init(istr,m_min_rank[line]) >> info.m_rank;

				if(m_max_scatter) init(istr,m_max_scatter[line]) >> info.m_max_scatter;
				return;
			}
		}

		throw CASUS(("WeaponInfoReader: не найден profile для <" + info.m_rid + ">!").c_str());
	}
};

POL_WEAPON_INFO_TABLE(POL_MAKE_DEFINITION,(WeaponInfoReader),POL_EMPTY)

//=======================================================================

class EquipmentInfoReader
{
public:

	static void Read(ImplantInfo& info)
	{
		try
		{
			TxtFilePtr txt(equip_xls);

			int line = ReadThingInfo(info, txt);

			std::string str;
			std::istringstream istr;

			txt->GetCell(line, m_columns[ECT_WISDOM].m_index, str);
			init(istr, str) >> info.m_wisdom;

			txt->GetCell(line, m_columns[ECT_MP2ACT].m_index, str);
			init(istr, str) >> info.m_mp2act;

			txt->GetCell(line, m_columns[ECT_BASIC_EFFECT].m_index, str);
			init(istr, str) >> info.m_power;

			txt->GetCell(line, m_columns[ECT_PARAM_TYPE].m_index, str);
			init(istr, str) >> info.m_parameter;
		}
		catch(CasusImprovisus &exc)
		{
			std::ostringstream sstr;
			sstr << "Ошибка в имплантанте [" << info.GetName() << "]: ";
			exc.prepend_info(sstr.str());
		}
	}

	static void Read(MedikitInfo& info)
	{
		TxtFilePtr txt(equip_xls);

		int line = ReadThingInfo(info, txt);

		std::string str;
		std::istringstream istr;

		txt->GetCell(line, m_columns[ECT_WISDOM].m_index, str);
		init(istr, str) >> info.m_wisdom;

		txt->GetCell(line, m_columns[ECT_MP2ACT].m_index, str);
		init(istr, str) >> info.m_mp2act;

		txt->GetCell(line, m_columns[ECT_BASIC_EFFECT].m_index, str);
		init(istr, str) >> info.m_dose;

		txt->GetCell(line, m_columns[ECT_CHARGE].m_index, str);
		init(istr, str) >> info.m_charge;
	}

	static void Read(MoneyInfo& info)
	{
		TxtFilePtr txt(equip_xls);

		int line = ReadThingInfo(info, txt);

		txt->GetCell(line, m_columns[ECT_USE_SOUND].m_index, info.m_use_sound);
	}

	static void Read(KeyInfo& info)
	{
		TxtFilePtr txt(equip_xls);

		int line = ReadThingInfo(info, txt);

		txt->GetCell(line, m_columns[ECT_USE_SOUND].m_index, info.m_use_sound);
	}

	static void Read(CameraInfo& info)
	{
		TxtFilePtr txt(equip_xls);

		int line = ReadThingInfo(info, txt);

		std::string str;
		std::istringstream istr;

		txt->GetCell(line, m_columns[ECT_CHARGE].m_index, str);
		init(istr, str) >> info.m_charge;

		txt->GetCell(line, m_columns[ECT_WISDOM].m_index, str);
		init(istr, str) >> info.m_wisdom;

		txt->GetCell(line, m_columns[ECT_MP2ACT].m_index, str);
		init(istr, str) >> info.m_mp2act;

		txt->GetCell(line, m_columns[ECT_RANGE].m_index, str);
		init(istr, str) >> info.m_radius;

		txt->GetCell(line, m_columns[ECT_HOLD_MODEL].m_index, info.m_hold_model);
		txt->GetCell(line, m_columns[ECT_VISUAL_EFFECT].m_index, info.m_effect_name);
	}

	static void Read(ShieldInfo& info)
	{
		TxtFilePtr txt(equip_xls);

		int line = ReadThingInfo(info, txt);

		std::string str;
		std::istringstream istr;

		txt->GetCell(line, m_columns[ECT_CHARGE].m_index, str);
		init(istr, str) >> info.m_charge;

		txt->GetCell(line, m_columns[ECT_WISDOM].m_index, str);
		init(istr, str) >> info.m_wisdom;

		txt->GetCell(line, m_columns[ECT_MP2ACT].m_index, str);
		init(istr, str) >> info.m_mp2act;

		txt->GetCell(line, m_columns[ECT_RANGE].m_index, str);
		init(istr, str) >> info.m_range;

		txt->GetCell(line, m_columns[ECT_HOLD_MODEL].m_index, info.m_hold_model);
		txt->GetCell(line, m_columns[ECT_VISUAL_EFFECT].m_index, info.m_effect_name);
	}

	static void Read(ScannerInfo& info)
	{
		TxtFilePtr txt(equip_xls);

		int line = ReadThingInfo(info, txt);

		std::string str;
		std::istringstream istr;

		txt->GetCell(line, m_columns[ECT_WISDOM].m_index, str);
		init(istr, str) >> info.m_wisdom;

		txt->GetCell(line, m_columns[ECT_MP2ACT].m_index, str);
		init(istr, str) >> info.m_mp2act;

		txt->GetCell(line, m_columns[ECT_RANGE].m_index, str);
		init(istr, str) >> info.m_radius;

		txt->GetCell(line, m_columns[ECT_USE_SOUND].m_index, info.m_use_sound);
		txt->GetCell(line, m_columns[ECT_HOLD_MODEL].m_index, info.m_hold_model);
	}

private:

	static int ReadThingInfo(ThingInfo& info, TxtFilePtr& txt)
	{
		std::string str;
		std::istringstream istr;

		static bool first_time = true;

		if(first_time){
			txt.ReadColumns(m_columns, m_columns + MAX_COLUMNS);
			first_time = false;
		}

		//найдем нужную строку
		for(int line = 0; line < txt->SizeY(); line++){
			txt->GetCell(line, m_columns[ECT_SYSNAME].m_index, str);

			if(str == info.m_rid)
			{
				info.m_name = PoL::getLocStr("equipment." + info.m_rid + ".name");
				info.m_initials = PoL::getLocStr("equipment." + info.m_rid + ".initials");
				//txt->GetCell(line, m_columns[ECT_NAME].m_index, info.m_name);
				//txt->GetCell(line, m_columns[ECT_INITIALS].m_index, info.m_initials);

				txt->GetCell(line, m_columns[ECT_WEIGHT].m_index, str);
				init(istr, str) >> info.m_weight;

				txt->GetCell(line, m_columns[ECT_COST].m_index, str);
				init(istr, str) >> info.m_cost;

				info.m_desc = PoL::getLocStr("equipment." + info.m_rid + ".description");
				//txt->GetCell(line, m_columns[ECT_DESC].m_index, info.m_desc);
				txt->GetCell(line, m_columns[ECT_SHADER].m_index, info.m_shader);

				txt->GetCell(line, m_columns[ECT_WIDTH].m_index, str);
				init(istr, str) >> info.m_size.x;

				txt->GetCell(line, m_columns[ECT_HEIGHT].m_index, str);
				init(istr, str) >> info.m_size.y;

				txt->GetCell(line, m_columns[ECT_ITEM_MODEL].m_index, info.m_item_model);

				txt->GetCell(line, m_columns[ECT_MINRANK].m_index, str);
				init(istr, str) >> info.m_rank;

				return line;
			}
		}

		throw CASUS(("EquipmentInfoReader: не найден profile для <" + info.m_rid + ">!").c_str());
	}

private:

	enum column_type
	{
		ECT_SYSNAME,
		//ECT_NAME,
		//ECT_INITIALS,
		ECT_TYPE,
		ECT_HOLDTYPE,
		ECT_WISDOM,
		ECT_WEIGHT,
		ECT_STATUS,
		ECT_MP2ACT,
		ECT_BASIC_EFFECT,
		ECT_RANGE,
		ECT_PARAM_TYPE,
		ECT_COST,
		//ECT_DESC,
		ECT_SHADER,
		ECT_WIDTH,
		ECT_HEIGHT,
		ECT_ITEM_MODEL,
		ECT_HOLD_MODEL,
		ECT_CHARGE,
		ECT_VISUAL_EFFECT,
		ECT_USE_SOUND,
		ECT_MINRANK,

		MAX_COLUMNS
	};

	static column m_columns[MAX_COLUMNS];
};

column EquipmentInfoReader::m_columns[] = 
{
	column("sys_name",  ECT_SYSNAME),
	//column("name",      ECT_NAME),
	//column("initials",  ECT_INITIALS),
	column("type",      ECT_TYPE),
	column("hold_type", ECT_HOLDTYPE),
	column("wisdom",    ECT_WISDOM),
	column("weight",    ECT_WEIGHT),
	column("status",    ECT_STATUS),
	column("mp_to_activate", ECT_MP2ACT),
	column("basic_effect",   ECT_BASIC_EFFECT),
	column("range",     ECT_RANGE),
	column("basic_effect_type",  ECT_PARAM_TYPE),
	column("basic_cost",   ECT_COST),
	//column("description",  ECT_DESC),
	column("item_shader",  ECT_SHADER),
	column("item_width",   ECT_WIDTH),
	column("item_height",  ECT_HEIGHT),
	column("item_model",   ECT_ITEM_MODEL),
	column("hold_model",   ECT_HOLD_MODEL),
	column("charge",       ECT_CHARGE),    
	column("effects_visuals", ECT_VISUAL_EFFECT),
	column("use_sound",    ECT_USE_SOUND),
	column("min_rank",     ECT_MINRANK)
};

//=======================================================================

class ArmorInfoReader
{
public:

	static void Read(ArmorInfo& info)
	{
		std::string str;
		std::istringstream istr;

		TxtFilePtr txt(armor_xls);

		static bool first_time = true;

		if(first_time){
			txt.ReadColumns(m_columns, m_columns + MAX_COLUMNS);
			first_time = false;
		}

		//найдем нужную строку
		for(int line = 0; line < txt->SizeY(); line++){
			txt->GetCell(line, m_columns[ARCT_SYSNAME].m_index, str);

			if(str == info.m_rid)
			{
				info.m_name = PoL::getLocStr("armors." + info.m_rid + ".name");
				info.m_initials = PoL::getLocStr("armors." + info.m_rid + ".initials");
				//txt->GetCell(line, m_columns[ARCT_NAME].m_index, info.m_name);
				//txt->GetCell(line, m_columns[ARCT_INITIALS].m_index, info.m_initials);

				txt->GetCell(line, m_columns[ARCT_DMG_TYPE].m_index, str);
				info.m_dmg_type = AIUtils::Str2Dmg(str);

				txt->GetCell(line, m_columns[ARCT_STRENGTH].m_index, str);
				init(istr, str) >> info.m_strength;

				txt->GetCell(line, m_columns[ARCT_WEIGHT].m_index, str);
				init(istr, str) >> info.m_weight;

				txt->GetCell(line, m_columns[ARCT_BODY].m_index, str);
				init(istr, str) >> info.m_body;

				txt->GetCell(line, m_columns[ARCT_LEGS].m_index, str);
				init(istr, str) >> info.m_legs;

				txt->GetCell(line, m_columns[ARCT_HANDS].m_index, str);
				init(istr, str) >> info.m_hands;

				txt->GetCell(line, m_columns[ARCT_HEAD].m_index, str);
				init(istr, str) >> info.m_head;

				txt->GetCell(line, m_columns[ARCT_REAR_AMPLIFY].m_index, str);
				init(istr, str) >> info.m_rear_amplify;

				txt->GetCell(line, m_columns[ARCT_SHADER].m_index, info.m_shader);

				txt->GetCell(line, m_columns[ARCT_WIDTH].m_index, str);
				init(istr, str) >> info.m_size.x;

				txt->GetCell(line, m_columns[ARCT_HEIGHT].m_index, str);
				init(istr, str) >> info.m_size.y;

				txt->GetCell(line, m_columns[ARCT_SUIT].m_index, str);
				init(istr, str) >> info.m_suit;

				txt->GetCell(line, m_columns[ARCT_COST].m_index, str);
				init(istr, str) >> info.m_cost;            

				info.m_desc = PoL::getLocStr("armors." + info.m_rid + ".description");
				//txt->GetCell(line, m_columns[ARCT_DESC].m_index, info.m_desc);
				txt->GetCell(line, m_columns[ARCT_ITEM_MODEL].m_index, info.m_item_model);                

				txt->GetCell(line, m_columns[ARCT_MP_2_ACT].m_index, str);
				init(istr, str) >> info.m_mp2act;            

				txt->GetCell(line, m_columns[ARCT_MINRANK].m_index, str);
				init(istr, str) >> info.m_rank;

				return;
			}
		}

		throw CASUS(("ArmorInfoReader: не найден profile для <" + info.m_rid + ">!").c_str());
	}

private:

	enum column_type
	{
		ARCT_SYSNAME,
		//ARCT_NAME,
		//ARCT_INITIALS,
		ARCT_DMG_TYPE,
		ARCT_STRENGTH,
		ARCT_SLOTS,
		ARCT_WEIGHT,
		ARCT_BODY,
		ARCT_LEGS,
		ARCT_HANDS,
		ARCT_HEAD,
		ARCT_REAR_AMPLIFY,
		ARCT_SHADER,
		ARCT_WIDTH,
		ARCT_HEIGHT,
		ARCT_SUIT,
		ARCT_COST,
		//ARCT_DESC,
		ARCT_ITEM_MODEL,
		ARCT_MP_2_ACT,
		ARCT_MINRANK,

		MAX_COLUMNS,
	};

	static column m_columns[MAX_COLUMNS];
};

column ArmorInfoReader::m_columns[] =
{
	column("sys_name",  ARCT_SYSNAME),
	//column("name",      ARCT_NAME),
	//column("initials",  ARCT_INITIALS),
	column("type",      ARCT_DMG_TYPE),
	column("strength",  ARCT_STRENGTH),
	column("slots",     ARCT_SLOTS),
	column("weight",    ARCT_WEIGHT),
	column("body",      ARCT_BODY),
	column("legs",      ARCT_LEGS),
	column("hands",     ARCT_HANDS),
	column("head",      ARCT_HEAD),
	column("rear_amplify", ARCT_REAR_AMPLIFY),
	column("item_shader",  ARCT_SHADER),
	column("item_width",   ARCT_WIDTH),
	column("item_height",  ARCT_HEIGHT),
	column("suit",         ARCT_SUIT),
	column("basic_cost",   ARCT_COST),
	//column("description",  ARCT_DESC),
	column("item_model",   ARCT_ITEM_MODEL),
	column("mp_to_activate",ARCT_MP_2_ACT), 
	column("min_rank",     ARCT_MINRANK)
};

//===========================================================================

bool operator < (const BaseThing& t1, const BaseThing& t2)
{
	if(t1.GetInfo()->GetType() == t2.GetInfo()->GetType())
	{
		if(t1.GetInfo()->GetRID() == t2.GetInfo()->GetRID())
		{
			if(t1.GetInfo()->IsAmmo())
			{
				const AmmoThing& a1 = static_cast<const AmmoThing&>(t1),
					& a2 = static_cast<const AmmoThing&>(t2);

				return a1.GetBulletCount() < a2.GetBulletCount();
			}

			if(t1.GetInfo()->IsWeapon())
			{
				AmmoThing* a1 = static_cast<const WeaponThing&>(t1).GetAmmo(),
					* a2 = static_cast<const WeaponThing&>(t2).GetAmmo();

				return (a1 == 0 && a2) || (a1 && a2 && a1->GetBulletCount() < a2->GetBulletCount());
			}
		}

		return t1.GetInfo()->GetRID() < t2.GetInfo()->GetRID();
	}

	return t1.GetInfo()->GetType() < t2.GetInfo()->GetType();
}

//===========================================================================

ThingInfo::ThingInfo(const rid_t& rid, thing_type type)
:	m_rid(rid),
m_type(type),
m_weight(0),
m_cost(0),
m_rank(0)
{    
}

ThingInfo::~ThingInfo()
{
}

thing_type ThingInfo::GetType() const
{
	return m_type;
}

int ThingInfo::GetCost() const
{
	return m_cost;
}

float ThingInfo::GetWeight() const
{
	return m_weight;
}

const rid_t& ThingInfo::GetRID() const
{
	return m_rid;
}

const ipnt2_t& ThingInfo::GetSize() const
{
	return m_size;
}

const std::string& ThingInfo::GetName() const
{
	return m_name;
}

const std::string& ThingInfo::GetDesc() const
{
	return m_desc;
}

const std::string& ThingInfo::GetShader() const
{
	return m_shader;
}

const std::string& ThingInfo::GetInitials() const
{
	return m_initials;
}

const std::string& ThingInfo::GetItemModel() const
{
	return m_item_model;
}

void ThingInfo::GetTraitsString(traits_string* traits) const
{    
}

//=======================================================================

GrenadeInfo::GrenadeInfo(const rid_t& rid) :
ThingInfo(rid, TT_GRENADE)
{
	GrenadeInfoReader::Read(*this);
}

int GrenadeInfo::GetMp2Act() const
{
	return m_mp2act;
}

void GrenadeInfo::GetTraitsString(traits_string* traits) const
{
	std::ostringstream ostr;

	traits->m_left_col.clear();

	traits->m_left_col  = DirtyLinks::GetStrRes("tp_strength") + '\n';  ostr << m_strength << std::endl;
	traits->m_left_col += DirtyLinks::GetStrRes("tp_wisdom") + '\n';    ostr << m_wisdom << std::endl;
	traits->m_left_col += DirtyLinks::GetStrRes("tp_weight") + '\n';    ostr << m_weight << std::endl;

	traits->m_left_col += DirtyLinks::GetStrRes("tp_dmg_radius") +'\n'; ostr << m_dmg_radius << std::endl;
	traits->m_left_col += DirtyLinks::GetStrRes("tp_bdmg") + '\n';      ostr << m_bdmg.m_val << " (" << AIUtils::Dmg2Str(m_bdmg.m_type, false) << ")" << std::endl;
	traits->m_left_col += DirtyLinks::GetStrRes("tp_admg") + '\n';      ostr << m_admg.m_val << " (" << AIUtils::Dmg2Str(m_admg.m_type, false) << ")" << std::endl;
	traits->m_left_col += DirtyLinks::GetStrRes("tp_cost") + '\n';      ostr << m_cost;

	traits->m_right_col = ostr.str();
}

int GrenadeInfo::GetWisdom() const
{
	return m_wisdom;
}

int GrenadeInfo::GetStrength() const
{
	return m_strength;
}

float GrenadeInfo::GetDmgRadius() const
{
	return m_dmg_radius;
}

const damage_s& GrenadeInfo::GetADmg() const
{
	return m_admg;
}

const damage_s& GrenadeInfo::GetBDmg() const
{
	return m_bdmg;
}

const std::string& GrenadeInfo::GetHitEffect() const
{
	return m_hit_effect;
}

const std::string& GrenadeInfo::GetHoldModel() const
{
	return m_hold_model;
}

bool GrenadeInfo::IsPlasmaGrenade() const
{
	return m_rid == "plasma_bomb";
}

//=======================================================================

AmmoInfo::AmmoInfo(const rid_t& rid) :
ThingInfo(rid, TT_AMMO)
{
	AmmoInfoReader::Read(*this);
}

int AmmoInfo::GetRange() const
{
	return m_range;
}

int AmmoInfo::GetQuantity() const
{
	return m_quality;
}

int AmmoInfo::GetFalloff() const
{
	return m_falloff;
}

float AmmoInfo::GetCaliber() const
{
	return m_caliber;
}


float AmmoInfo::GetDmgRadius() const
{       
	return m_dmg_radius;
}

const damage_s& AmmoInfo::GetADmg() const
{
	return m_admg;
}

const damage_s& AmmoInfo::GetBDmg() const
{
	return m_bdmg;
}

const std::string& AmmoInfo::GetHitEffect() const
{
	return m_hit_effect;
}

const std::string& AmmoInfo::GetTraceEffect() const
{
	return m_trace_effect;
}

const std::string& AmmoInfo::GetShellsEffect() const
{
	return m_shells_effect;
}

//=====================================================================================//
//                       void AmmoInfo::GetTraitsString() const                        //
//=====================================================================================//
void AmmoInfo::GetTraitsString(traits_string* traits) const
{   
	std::ostringstream ostr;

	traits->m_left_col  = DirtyLinks::GetStrRes("tp_caliber") + '\n';
	ostr << m_caliber << std::endl;
	traits->m_left_col += DirtyLinks::GetStrRes("tp_falloff") + '\n';
	ostr << m_falloff << std::endl;
	traits->m_left_col += DirtyLinks::GetStrRes("tp_dmg_radius") + '\n';
	ostr << m_dmg_radius << std::endl;

	traits->m_left_col += DirtyLinks::GetStrRes("tp_bdmg") + '\n';
	ostr << m_bdmg.m_val << " (" << AIUtils::Dmg2Str(m_bdmg.m_type, false) << ")" << std::endl;
	traits->m_left_col += DirtyLinks::GetStrRes("tp_admg") + '\n';
	ostr << m_admg.m_val << " (" << AIUtils::Dmg2Str(m_admg.m_type, false) << ")" << std::endl;
	traits->m_left_col += DirtyLinks::GetStrRes("tp_stacksize") + '\n';
	ostr << GetQuantity() << std::endl;
	traits->m_left_col += DirtyLinks::GetStrRes("tp_damage_falloff") + '\n';
	ostr << GetDamageFalloff() << std::endl;
	traits->m_left_col += DirtyLinks::GetStrRes("tp_cost") + '\n';
	ostr << GetCost() << std::endl;

	traits->m_right_col = ostr.str();
}

//=======================================================================

WeaponInfo::WeaponInfo(const rid_t& rid) :
ThingInfo(rid, TT_WEAPON)
{
	WeaponInfoReader::Read(*this);
}

const damage_s& WeaponInfo::GetADmg() const
{
	return m_admg;
}

const damage_s& WeaponInfo::GetBDmg() const
{
	return m_bdmg;
}

int WeaponInfo::GetMps2Reload() const
{
	return m_mp2reload;
}

int WeaponInfo::GetRange() const
{
	return m_range;
}

int WeaponInfo::GetWisdom() const
{
	return m_wisdom;
}

int WeaponInfo::GetAimQty() const
{
	return m_aim_qty;
}

int WeaponInfo::GetAutoQty() const
{
	return m_auto_qty;
}

int WeaponInfo::GetQuantity() const
{
	return m_quality;
}

int WeaponInfo::GetStrength() const
{
	return m_strength;
}

int WeaponInfo::GetAccuracy() const
{
	return m_accuracy;
}

int WeaponInfo::GetMovepnts() const
{
	return m_movepnts;
}

float WeaponInfo::GetCaliber() const
{
	return m_caliber;
}

const point3& WeaponInfo::GetBarrel() const
{
	return m_barrel;
}

const point3& WeaponInfo::GetShellOutlet() const
{
	return m_shell;
}

const std::string& WeaponInfo::GetAmmoLoadSound() const
{
	return m_ammo_load_sound;
}

bool WeaponInfo::HaveShellOutlet() const
{
	return m_caliber != 1.1f && m_caliber != 2.3f && m_caliber != 3.1f;
}

hold_type WeaponInfo::GetHoldType() const
{
	return m_htype;
}

const std::string& WeaponInfo::GetShotSound(shot_type type) const
{
	return type == SHT_AUTOSHOT ? m_autoshot_sound : m_shot_sound;
}

const std::string& WeaponInfo::GetHoldModel() const
{
	return m_hold_model;
}

const std::string& WeaponInfo::GetFlashEffect() const
{
	return m_flash_effect;
}

bool WeaponInfo::IsCannon() const
{
	return GetHoldType() == HT_CANNON;
}

bool WeaponInfo::IsAuto() const
{
	return m_auto_qty > 1;
}

void WeaponInfo::GetTraitsString(traits_string* traits) const
{
	using std::ostringstream;
	using std::endl;
	ostringstream ostr;

	traits->m_left_col  = DirtyLinks::GetStrRes("tp_strength") + '\n'; ostr << m_strength << endl;
	traits->m_left_col += DirtyLinks::GetStrRes("tp_wisdom") + '\n';   ostr << m_wisdom << endl;
	traits->m_left_col += DirtyLinks::GetStrRes("tp_caliber")+ '\n';   ostr << m_caliber << endl;
	traits->m_left_col += DirtyLinks::GetStrRes("tp_weight") + '\n';   ostr << m_weight  << endl;
	//traits->m_left_col += DirtyLinks::GetStrRes("tp_range") + '\n';    ostr << m_range << endl;

	traits->m_left_col += DirtyLinks::GetStrRes("tp_bquality") + '\n';  ostr << m_quality << endl;
	traits->m_left_col += DirtyLinks::GetStrRes("tp_baccuracy") + '\n'; ostr << m_accuracy << endl;

	traits->m_left_col += DirtyLinks::GetStrRes("tp_bdmg") + '\n'; ostr << m_bdmg.m_val << " (" << AIUtils::Dmg2Str(m_bdmg.m_type, false) << ")" << endl;
	traits->m_left_col += DirtyLinks::GetStrRes("tp_admg") + '\n'; ostr << m_admg.m_val << " (" << AIUtils::Dmg2Str(m_admg.m_type, false) << ")" << endl;
	traits->m_left_col += DirtyLinks::GetStrRes("tp_cost") + '\n'; ostr << m_cost <<endl;

	traits->m_left_col += DirtyLinks::GetStrRes("tp_shootbullets") + '\n'; 
	ostr << GetAimQty()<<"/"<<GetAimQty()<<"/";
	if(IsAuto()) ostr<<GetAutoQty()<<endl;
	else         ostr<<"-"<<endl;

	int mps = GetMovepnts();
	traits->m_left_col += DirtyLinks::GetStrRes("tp_shootmp") + '\n'; 
	ostr << mps << "/" << (int)(((float)mps)/PoL::Inv::SnapshotMpsFactor) << "/";
	if(IsAuto()) ostr << (int)(((float)mps)/PoL::Inv::AutoshotMpsFactor)<<endl;
	else ostr<<"-"<<endl;

	traits->m_left_col += DirtyLinks::GetStrRes("tp_mp2reload") + '\n'; ostr << m_mp2reload;

	traits->m_right_col = ostr.str();
}

//=======================================================================

ImplantInfo::ImplantInfo(const rid_t& rid) :
ThingInfo(rid, TT_IMPLANT)
{
	EquipmentInfoReader::Read(*this);
}

int ImplantInfo::GetPower() const
{
	return m_power;
}

int ImplantInfo::GetWisdom() const
{
	return m_wisdom;
}

int ImplantInfo::GetMp2Act() const
{
	return m_mp2act;
}

ImplantInfo::parameter_type ImplantInfo::GetParameter() const
{
	return m_parameter;
}

void ImplantInfo::GetTraitsString(traits_string* traits) const
{
	std::ostringstream ostr;

	traits->m_left_col  = DirtyLinks::GetStrRes("tp_wisdom") + '\n'; ostr << m_wisdom << std::endl;
	traits->m_left_col += DirtyLinks::GetStrRes("tp_effect") + '\n'; ostr << m_power  << std::endl;
	traits->m_left_col += DirtyLinks::GetStrRes("tp_mp2act") + '\n';   ostr << m_mp2act << std::endl;
	traits->m_left_col += DirtyLinks::GetStrRes("tp_eff_type") + '\n'; ostr << Parameter2Str(m_parameter) << std::endl;
	traits->m_left_col += DirtyLinks::GetStrRes("tp_cost") + '\n';     ostr << m_cost;

	traits->m_right_col = ostr.str();
}

//=======================================================================

ArmorInfo::ArmorInfo(const rid_t& rid) :
ThingInfo(rid, TT_ARMOUR)
{
	ArmorInfoReader::Read(*this);
}

bool ArmorInfo::IsHelmet() const
{
	return m_dmg_type == DT_ENERGY;
}

float ArmorInfo::GetBody() const
{
	return m_body;
}

float ArmorInfo::GetLegs() const
{
	return m_legs;
}

float ArmorInfo::GetHead() const
{
	return m_head;
}

float ArmorInfo::GetHands() const
{
	return m_hands;
}

int ArmorInfo::GetStrength() const
{
	return m_strength;
}

armor_type ArmorInfo::GetSuit() const
{
	return m_suit;
}

damage_type ArmorInfo::GetDmgType() const
{
	return m_dmg_type;
}

float ArmorInfo::GetRearAmplify() const
{
	return m_rear_amplify;
}

bool ArmorInfo::IsSpaceSuit() const
{
	return GetSuit() == ARM_SCUBA;
}

int ArmorInfo::GetMp2Act() const
{
	return m_mp2act;
}

void ArmorInfo::GetTraitsString(traits_string* traits) const
{
	std::ostringstream ostr;    

	traits->m_left_col  = DirtyLinks::GetStrRes("tp_strength") + '\n'; ostr << m_strength << std::endl;
	traits->m_left_col += DirtyLinks::GetStrRes("tp_weight") + '\n';   ostr << m_weight << std::endl;
	traits->m_left_col += DirtyLinks::GetStrRes("tp_body") + '\n';     ostr << m_body << std::endl;
	traits->m_left_col += DirtyLinks::GetStrRes("tp_legs") + '\n';     ostr << m_legs << std::endl;
	traits->m_left_col += DirtyLinks::GetStrRes("tp_head") + '\n';     ostr << m_head << std::endl;
	traits->m_left_col += DirtyLinks::GetStrRes("tp_hands") + '\n';    ostr << m_hands << std::endl;

	traits->m_left_col += DirtyLinks::GetStrRes("tp_dmg_type") + '\n';     ostr << AIUtils::Dmg2Str(m_dmg_type, false) << std::endl;
	traits->m_left_col += DirtyLinks::GetStrRes("tp_rear_amplify") + '\n'; ostr << m_rear_amplify << std::endl;
	traits->m_left_col += DirtyLinks::GetStrRes("tp_cost") + '\n';         ostr << m_cost;

	traits->m_right_col = ostr.str();
}

//=======================================================================

MedikitInfo::MedikitInfo(const rid_t& rid) :
ThingInfo(rid, TT_MEDIKIT)
{
	EquipmentInfoReader::Read(*this);
}

int MedikitInfo::GetDose() const
{
	return m_dose;
}

int MedikitInfo::GetWisdom() const
{
	return m_wisdom;
}

int MedikitInfo::GetMp2Act() const
{
	return m_mp2act;
}

int MedikitInfo::GetCharge() const
{
	return m_charge;
}

void MedikitInfo::GetTraitsString(traits_string* traits) const
{
	std::ostringstream ostr;

	traits->m_left_col  = DirtyLinks::GetStrRes("tp_wisdom") + '\n'; ostr << m_wisdom << std::endl;
	traits->m_left_col += DirtyLinks::GetStrRes("tp_weight") + '\n'; ostr << m_weight << std::endl;
	traits->m_left_col += DirtyLinks::GetStrRes("tp_charge") + '\n'; ostr << m_charge << std::endl;
	traits->m_left_col += DirtyLinks::GetStrRes("tp_dose") + '\n';   ostr << m_dose   << std::endl;
	traits->m_left_col += DirtyLinks::GetStrRes("tp_mp2act") + '\n'; ostr << m_mp2act << std::endl;
	traits->m_left_col += DirtyLinks::GetStrRes("tp_cost") + '\n';   ostr << m_cost;

	traits->m_right_col = ostr.str();
}

//=======================================================================

MoneyInfo::MoneyInfo() : ThingInfo(money_rid, TT_MONEY)
{
	EquipmentInfoReader::Read(*this);
}

const std::string& MoneyInfo::GetUseSound() const
{
	return m_use_sound;
}

//=======================================================================

KeyInfo::KeyInfo(const rid_t& rid) : ThingInfo(rid, TT_KEY)
{
	EquipmentInfoReader::Read(*this);
}

const std::string& KeyInfo::GetUseSound() const
{
	return m_use_sound;
}

//=======================================================================

CameraInfo::CameraInfo(const rid_t& rid) : ThingInfo(rid, TT_CAMERA)
{
	EquipmentInfoReader::Read(*this);
}

int CameraInfo::GetMp2Act() const
{
	return m_mp2act;
}

float CameraInfo::GetRadius() const
{
	return m_radius;
}

int CameraInfo::GetWisdom() const
{
	return m_wisdom;
}

int CameraInfo::GetCharge() const
{
	return m_charge;
}

void CameraInfo::GetTraitsString(traits_string* traits) const
{
	std::ostringstream ostr;

	traits->m_left_col.clear();

	traits->m_left_col  = DirtyLinks::GetStrRes("tp_wisdom") + '\n'; ostr << m_wisdom << std::endl;
	traits->m_left_col += DirtyLinks::GetStrRes("tp_mp2act") + '\n'; ostr << m_mp2act << std::endl;
	traits->m_left_col += DirtyLinks::GetStrRes("tp_weight") + '\n'; ostr << m_weight << std::endl;
	traits->m_left_col += DirtyLinks::GetStrRes("tp_range") + '\n';  ostr << m_radius << std::endl;
	traits->m_left_col += DirtyLinks::GetStrRes("tp_cost") + '\n';   ostr << m_cost;

	traits->m_right_col = ostr.str();
}

const std::string& CameraInfo::GetHoldModel() const
{
	return m_hold_model;
}

const std::string& CameraInfo::GetVisualEffect() const
{
	return m_effect_name;
}

//=======================================================================

ShieldInfo::ShieldInfo(const rid_t& rid) : ThingInfo(rid, TT_SHIELD)
{
	EquipmentInfoReader::Read(*this);
}

int ShieldInfo::GetMp2Act() const
{
	return m_mp2act;
}

float ShieldInfo::GetRange() const
{
	return m_range;
}

int ShieldInfo::GetWisdom() const
{
	return m_wisdom;
}

void ShieldInfo::GetTraitsString(traits_string* traits) const
{
	std::ostringstream ostr;

	traits->m_left_col.clear();

	traits->m_left_col  = DirtyLinks::GetStrRes("tp_wisdom") + '\n'; ostr << m_wisdom << std::endl;
	traits->m_left_col += DirtyLinks::GetStrRes("tp_mp2act") + '\n'; ostr << m_mp2act << std::endl;
	traits->m_left_col += DirtyLinks::GetStrRes("tp_weight") + '\n'; ostr << m_weight << std::endl;
	traits->m_left_col += DirtyLinks::GetStrRes("tp_range") + '\n';  ostr << m_range  << std::endl;
	traits->m_left_col += DirtyLinks::GetStrRes("tp_cost") + '\n';   ostr << m_cost;

	traits->m_right_col = ostr.str();
}

int ShieldInfo::GetCharge() const
{
	return m_charge;
}

const std::string& ShieldInfo::GetHoldModel() const
{
	return m_hold_model;
}

const std::string& ShieldInfo::GetVisualEffect() const
{
	return m_effect_name;
}

//=======================================================================

ScannerInfo::ScannerInfo(const rid_t& rid)
:	ThingInfo(rid, TT_SCANNER)
{
	EquipmentInfoReader::Read(*this);
}

int ScannerInfo::GetMp2Act() const
{
	return m_mp2act;
}

int ScannerInfo::GetWisdom() const
{
	return m_wisdom;
}

float ScannerInfo::GetRadius() const
{
	return m_radius;
}

const std::string& ScannerInfo::GetHoldModel() const
{
	return m_hold_model;
}

void ScannerInfo::GetTraitsString(traits_string* traits) const
{
	std::ostringstream ostr;

	traits->m_left_col.clear();

	traits->m_left_col  = DirtyLinks::GetStrRes("tp_wisdom") + '\n'; ostr << m_wisdom << std::endl;
	traits->m_left_col += DirtyLinks::GetStrRes("tp_mp2act") + '\n'; ostr << m_mp2act << std::endl;
	traits->m_left_col += DirtyLinks::GetStrRes("tp_weight") + '\n'; ostr << m_weight << std::endl;
	traits->m_left_col += DirtyLinks::GetStrRes("tp_range") + '\n';  ostr << m_radius << std::endl;
	traits->m_left_col += DirtyLinks::GetStrRes("tp_cost") + '\n';   ostr << m_cost;

	traits->m_right_col = ostr.str();
}

const std::string& ScannerInfo::GetUseSound() const
{
	return m_use_sound;
}

//=======================================================================

ThingInfo *ThingInfoFactory::Create(thing_type type, const rid_t& rid)
{
	switch(type)
	{
		case TT_KEY: return new KeyInfo(rid);
		case TT_MONEY: return new MoneyInfo();
		case TT_AMMO: return new AmmoInfo(rid);
		case TT_ARMOUR: return new ArmorInfo(rid);
		case TT_WEAPON: return new WeaponInfo(rid);
		case TT_CAMERA: return new CameraInfo(rid);
		case TT_SHIELD: return new ShieldInfo(rid);
		case TT_SCANNER: return new ScannerInfo(rid);
		case TT_GRENADE: return new GrenadeInfo(rid);
		case TT_IMPLANT: return new ImplantInfo(rid);
		case TT_MEDIKIT: return new MedikitInfo(rid);
	}

	throw CASUS("ThingInfoFactory: попытка создания информации о неизв. предмете");
}

//=======================================================================

//=====================================================================================//
//                               BaseThing::BaseThing()                                //
//=====================================================================================//
BaseThing::BaseThing(thing_type type, const rid_t& rid)
:	m_info(0),
	m_pak_pos(0,0),
	m_tid(0),
	m_entity(0)
	//m_store_box(0)
{
	if(rid.size())
	{
		m_info = ThingInfoArchive::GetInst()->CreateRef(type, rid);
	}
}

BaseThing::~BaseThing()
{
	if(m_info) ThingInfoArchive::GetInst()->DeleteRef(m_info);
}

BaseEntity* BaseThing::GetEntity()
{
	return m_entity;
}

void BaseThing::SetEntity(BaseEntity* ent)
{
	m_entity = ent;
	//m_store_box = 0;
}

//StoreBox* BaseThing::GetStoreBox()
//{
//	return m_store_box;
//}

//void BaseThing::SetStoreBox(StoreBox* box)
//{
//	m_store_box = box;
//	m_entity = 0;
//}

//=====================================================================================//
//                           void BaseThing::MakeSaveLoad()                            //
//=====================================================================================//
void BaseThing::MakeSaveLoad(SavSlotBase& st)
{
	static const int currentVersion = 1;

	if(st.IsSaving())
	{
		st << m_pak_pos.x;
		st << m_pak_pos.y;
		st << m_tid;

		st << m_info->GetRID();
		st << (currentVersion|0xFFFF0000);
		st << static_cast<int>(m_info->GetType());

		st << (m_entity != 0);
		if(m_entity) st << m_entity->GetEID();
	}
	else
	{
		st >> m_pak_pos.x;
		st >> m_pak_pos.y;
		st >> m_tid;

		rid_t rid;  st >> rid;
		int type; st >> type;
		int version = 0;

		if((type & 0xFFFF0000) == 0xFFFF0000)
		{
			version = type & 0xFFFF;
			st >> type;
		}

		//ВНИМАНИЕ: в случае денег поле уже проинициализировано
		if(!m_info) m_info = ThingInfoArchive::GetInst()->CreateRef(static_cast<thing_type>(type), rid);

		bool flag;  st >> flag;

		if(flag)
		{
			eid_t eid; st >> eid;
			m_entity = EntityPool::GetInst()->Get(eid);
		}

		// Для совместимости со старыми сейвами
		if(version < 1)
		{
			rid_t rid;
			st >> flag;
			if(flag) st >> rid;
		}
	}
}

const ipnt2_t& BaseThing::GetPakPos() const
{
	return m_pak_pos;
}

void BaseThing::SetPakPos(const ipnt2_t& pnt)
{
	m_pak_pos = pnt;
}

int BaseThing::GetCost() const
{
	return GetInfo()->GetCost();
}

float BaseThing::GetWeight() const
{
	return GetInfo()->GetWeight();
}

const ThingInfo* BaseThing::GetInfo() const
{ 
	return m_info;
}

//=======================================================================

WeaponThing::WeaponThing(const rid_t rid) :
BaseThing(TT_WEAPON, rid), m_ammo(0)
{
}

WeaponThing::~WeaponThing()
{
	delete m_ammo;
}

//=====================================================================================//
//                            void WeaponThing::SetEntity()                            //
//=====================================================================================//
void WeaponThing::SetEntity(BaseEntity* ent)
{
	BaseThing::SetEntity(ent);

	if(m_ammo)
	{
		m_ammo->SetTID(0);
		m_ammo->SetEntity(ent);
	}
}

//void WeaponThing::SetStoreBox(StoreBox* box)
//{
//	BaseThing::SetStoreBox(box);
//
//	if(m_ammo){
//		m_ammo->SetTID(0);
//		m_ammo->SetStoreBox(box);
//	}
//}

void WeaponThing::MakeSaveLoad(SavSlotBase& st)
{
	BaseThing::MakeSaveLoad(st);

	if(st.IsSaving()){

		st << (m_ammo != 0);

		if(m_ammo){
			DynUtils::SaveObj(st, m_ammo);
			m_ammo->MakeSaveLoad(st);
		}

	}else{

		bool flag; st >> flag;

		if(flag){
			DynUtils::LoadObj(st, m_ammo);
			m_ammo->MakeSaveLoad(st);
		}
	}
}

AmmoThing* WeaponThing::GetAmmo() const
{
	return m_ammo;
}

//=====================================================================================//
//                           AmmoThing* WeaponThing::Load()                            //
//=====================================================================================//
AmmoThing* WeaponThing::Load(AmmoThing* new_ammo)
{
	AmmoThing* old_ammo = m_ammo;

	if(old_ammo)
	{
		old_ammo->SetEntity(0);
	}

	m_ammo = new_ammo;
	if(m_ammo && m_ammo->GetBulletCount() > GetInfo()->GetQuantity())
	{
		m_ammo->SetBulletCount(GetInfo()->GetQuantity());
	}

	if(m_ammo)
	{
		m_ammo->SetTID(0);
		m_ammo->SetEntity(GetEntity());
		//if(GetStoreBox()) m_ammo->SetStoreBox(GetStoreBox());
	}

	if(GetEntity()) GetEntity()->Notify(EntityObserver::EV_WEAPON_STATE_CHANGE);    
	return old_ammo;
}

int WeaponThing::GetCost() const
{
	return m_ammo ? m_ammo->GetCost() + GetInfo()->GetCost() : GetInfo()->GetCost();
}

float WeaponThing::GetWeight() const
{
	return m_ammo ? m_ammo->GetWeight() + GetInfo()->GetWeight() : GetInfo()->GetWeight();
}

const WeaponInfo* WeaponThing::GetInfo() const
{
	return static_cast <const WeaponInfo*>(BaseThing::GetInfo());
}

int WeaponThing::GetMovepnts(shot_type type) const
{
	int movepnts = GetInfo()->GetMovepnts();

	if(type == SHT_SNAPSHOT) movepnts /= PoL::Inv::SnapshotMpsFactor; 
	if(type == SHT_AUTOSHOT) movepnts /= PoL::Inv::AutoshotMpsFactor;

	return movepnts;
}

int WeaponThing::GetShotCount(shot_type type) const
{
	if(m_ammo == 0) return 0;

	int shot_count = 0;

	if(type == SHT_AUTOSHOT)
		shot_count = GetInfo()->GetAutoQty();
	else
		shot_count = GetInfo()->GetAimQty();

	return shot_count > m_ammo->GetBulletCount() ? m_ammo->GetBulletCount() : shot_count;
}

bool WeaponInfo::IsRocket() const
{
	return m_htype == HT_ROCKET;
}

bool WeaponInfo::IsAutoCannon() const
{
	return m_htype == HT_AUTOCANNON;
}

//=======================================================================

AmmoThing::AmmoThing(const rid_t& rid):
BaseThing(TT_AMMO, rid), m_count(0)
{
	if(rid.size()) m_count = GetInfo()->GetQuantity();
}

AmmoThing::~AmmoThing()
{
}

//=====================================================================================//
//                        int AmmoThing::GetBulletCount() const                        //
//=====================================================================================//
int AmmoThing::GetBulletCount() const
{
	return m_count;
}

//=====================================================================================//
//                          void AmmoThing::SetBulletCount()                           //
//=====================================================================================//
void AmmoThing::SetBulletCount(int count)
{
	if(GetEntity() && GetEntity()->IsRaised(EA_LOCK_TRAITS))
	{
		return;
	}

	m_count = std::min(count,GetInfo()->GetQuantity());

	if(GetEntity())
	{
		GetEntity()->Notify(EntityObserver::EV_WEAPON_STATE_CHANGE);
	}
}

//=====================================================================================//
//                           int AmmoThing::GetCost() const                            //
//=====================================================================================//
int AmmoThing::GetCost() const
{
	return static_cast<float>(m_count) * static_cast<float>(GetInfo()->GetCost())/static_cast<float>(GetInfo()->GetQuantity());
}

float AmmoThing::GetWeight() const
{
	return (m_count / GetInfo()->GetQuantity()) * GetInfo()->GetWeight();
}

void AmmoThing::MakeSaveLoad(SavSlotBase& st)
{
	BaseThing::MakeSaveLoad(st);

	if(st.IsSaving())
		st << m_count;
	else
		st >> m_count;
}

const AmmoInfo* AmmoThing::GetInfo() const
{
	return static_cast<const AmmoInfo*>(BaseThing::GetInfo());
}

//=======================================================================

GrenadeThing::GrenadeThing(const rid_t& rid):
BaseThing(TT_GRENADE, rid)
{
}

GrenadeThing::~GrenadeThing()
{
}

const GrenadeInfo* GrenadeThing::GetInfo() const
{
	return static_cast<const GrenadeInfo*>(BaseThing::GetInfo());
}

//=======================================================================

ArmorThing::ArmorThing(const rid_t& rid) :
BaseThing(TT_ARMOUR, rid)
{
} 

ArmorThing::~ArmorThing()
{
}

const ArmorInfo* ArmorThing::GetInfo() const
{
	return static_cast<const ArmorInfo*>(BaseThing::GetInfo());
}

float ArmorThing::GetAbsorber(damage_type type, body_parts_type part, bool fback) const
{
	if(GetInfo()->GetDmgType() != type)
		return 0;

	float ret; 

	switch(part){
case BPT_NONE:
	return 1.0f;        
case BPT_HEAD:
	ret = GetInfo()->GetHead(); 
	break;
case BPT_BODY:
	ret = GetInfo()->GetBody();
	break;
case BPT_HANDS:
	ret = GetInfo()->GetHands(); 
	break;
case BPT_LEGS:
	ret = GetInfo()->GetLegs();
	break;
	}

	return fback ? ret * GetInfo()->GetRearAmplify() : ret;
}

//=======================================================================

MedikitThing::MedikitThing(const rid_t& rid) :
BaseThing(TT_MEDIKIT, rid), m_charge(0)
{
	if(rid.size()) m_charge = GetInfo()->GetCharge();
}

MedikitThing::~MedikitThing()
{
}

int MedikitThing::GetCharge() const
{
	return m_charge;
}

void MedikitThing::SetCharge(int charge)
{
	if(GetEntity() && GetEntity()->IsRaised(EA_LOCK_TRAITS))
		return;

	if(GetEntity()) GetEntity()->Notify(EntityObserver::EV_WEAPON_STATE_CHANGE);

	m_charge = charge;
}

const MedikitInfo* MedikitThing::GetInfo() const
{
	return static_cast<const MedikitInfo*>(BaseThing::GetInfo());
}

void MedikitThing::MakeSaveLoad(SavSlotBase& st)
{
	BaseThing::MakeSaveLoad(st);

	if(st.IsSaving())
		st << m_charge;
	else
		st >> m_charge;
}

//=======================================================================

ImplantThing::ImplantThing(const rid_t& rid) :
BaseThing(TT_IMPLANT, rid)
{    
}

ImplantThing::~ImplantThing()
{
}

const ImplantInfo* ImplantThing::GetInfo() const
{
	return static_cast<const ImplantInfo*>(BaseThing::GetInfo());
}

//=======================================================================

MoneyThing::MoneyThing(int sum) :
m_sum(sum), BaseThing(TT_MONEY, money_rid)
{
	if(!m_sum) m_sum = GetInfo()->GetCost();
}

MoneyThing::~MoneyThing()
{
}

void MoneyThing::MakeSaveLoad(SavSlotBase& st)
{
	BaseThing::MakeSaveLoad(st);

	if(st.IsSaving())
		st << m_sum;
	else
		st >> m_sum;
}

int MoneyThing::GetSum() const
{
	return m_sum;
}

void MoneyThing::SetSum(int sum)
{
	m_sum = sum;
}

const MoneyInfo* MoneyThing::GetInfo() const
{
	return static_cast<const MoneyInfo*>(BaseThing::GetInfo());
}

//=======================================================================

KeyThing::KeyThing(const rid_t& rid) : BaseThing(TT_KEY, rid)
{
}

KeyThing::~KeyThing()
{
}

const KeyInfo* KeyThing::GetInfo() const
{
	return static_cast<const KeyInfo*>(BaseThing::GetInfo());
}

//=======================================================================

CameraThing::CameraThing(const rid_t& rid) : BaseThing(TT_CAMERA, rid) 
{
}

const CameraInfo* CameraThing::GetInfo() const
{
	return static_cast<const CameraInfo*>(BaseThing::GetInfo());
}

//=======================================================================

ShieldThing::ShieldThing(const rid_t& rid) : BaseThing(TT_SHIELD, rid)
{
}

const ShieldInfo* ShieldThing::GetInfo() const
{
	return static_cast<const ShieldInfo*>(BaseThing::GetInfo());
}

//=======================================================================

ScannerThing::ScannerThing(const rid_t& rid)
:	BaseThing(TT_SCANNER, rid),
	m_inUse(false)
{
}

const ScannerInfo* ScannerThing::GetInfo() const
{
	return static_cast<const ScannerInfo*>(BaseThing::GetInfo());
}

//=======================================================================

namespace ThingDetails
{
	class UsualThingFactory : public ThingFactory
	{
	public:
		BaseThing* CreateMedikit(const rid_t& rid, int charge)
		{
			MedikitThing* ret = new MedikitThing(rid);
			if(charge) ret->SetCharge(charge);
			return ret;
		}

		BaseThing* CreateAmmo(const rid_t& rid, int count)
		{
			AmmoThing* ret = new AmmoThing(rid);
			if(count) ret->SetBulletCount(count);
			return ret;
		}

		BaseThing* CreateGrenade(const rid_t& rid)
		{
			return new GrenadeThing(rid);
		}

		BaseThing* CreateImplant(const rid_t& rid)
		{
			return new ImplantThing(rid);
		}

		BaseThing* CreateArmor(const rid_t& rid)
		{
			return new ArmorThing(rid);
		}

		BaseThing* CreateScanner(const rid_t& rid)
		{
			return new ScannerThing(rid);
		}

		BaseThing* CreateWeapon(const rid_t& rid, const rid_t& ammo_rid, int ammo_count)
		{
			WeaponThing* weap = new WeaponThing(rid);    
			if(ammo_rid.size()) weap->Load(static_cast<AmmoThing*>(CreateAmmo(ammo_rid, ammo_count)));
			return weap;
		}

		BaseThing* CreateMoney(int sum)
		{
			return new MoneyThing(sum);
		}

		BaseThing* CreateKey(const rid_t& rid)
		{
			return new KeyThing(rid);
		}

		BaseThing* CreateShield(const rid_t& rid)
		{
			return new ShieldThing(rid);
		}

		BaseThing* CreateCamera(const rid_t& rid)
		{
			return new CameraThing(rid);
		}
	};
}

ThingFactory* ThingFactory::GetInst()
{
	static UsualThingFactory factory;
	return &factory;
}

//=======================================================================

namespace ThingDetails
{

	//
	// условие для итерации по предметам в нек радиусе
	//
	class CircleIterateCondition : public Depot::iterator::Condition {
	public:

		static CircleIterateCondition* GetInst()
		{
			static CircleIterateCondition imp;
			return &imp;
		}

		void Init(const ipnt2_t& center, float radius, unsigned mask)
		{
			m_mask = mask;
			m_center = center;
			m_radius = radius;
		}

		bool IsSuitable(BaseThing* thing, GraphThing* graph) const
		{
			return      thing 
				&&  thing->GetInfo()->GetType() & m_mask
				&&  GraphUtils::IsInVicinity(graph->GetPos3(), m_center, m_radius);
		}

	private:

		unsigned m_mask;
		ipnt2_t  m_center;
		float    m_radius;
	};

	//
	// условие для итерации по предметам в поле видимости
	//
	class FOSIterateCondition : public Depot::iterator::Condition, private EntityVisitor{
	public:

		static FOSIterateCondition* GetInst()
		{ 
			static FOSIterateCondition imp;
			return &imp;
		}

		void Init(BaseEntity* entity, unsigned mask)
		{
			m_mask = mask;
			m_spectator = entity->GetEntityContext()->GetSpectator();
		}

		bool IsSuitable(BaseThing* thing, GraphThing* graph) const
		{
			if(thing == 0 || !(thing->GetInfo()->GetType() & m_mask))
				return false;

			return (m_spectator->CalcVisibility(graph->GetPos3()).IsVisible()) != 0;
		}

	private:

		Spectator* m_spectator;
		unsigned    m_mask;
	};
}

//=====================================================================================//
//                               Depot* Depot::GetInst()                               //
//=====================================================================================//
Depot* Depot::GetInst()
{
	static Depot imp;
	return &imp;
}

//=====================================================================================//
//                                   Depot::Depot()                                    //
//=====================================================================================//
Depot::Depot()
{
}

//=====================================================================================//
//                                   Depot::~Depot()                                   //
//=====================================================================================//
Depot::~Depot()
{
	info_vec_t::iterator itor = m_infos.begin();
	while(itor != m_infos.end())
	{
		delete itor->m_thing;
		delete itor->m_graph;
		++itor;
	}    
}

//=====================================================================================//
//                               BaseThing* Depot::Get()                               //
//=====================================================================================//
BaseThing* Depot::Get(tid_t tid)
{
	return m_infos[tid].m_thing;
}

//=====================================================================================//
//                                 void Depot::Reset()                                 //
//=====================================================================================//
void Depot::Reset()
{
	info_vec_t::iterator itor = m_infos.begin();

	while(itor != m_infos.end())
	{
		delete itor->m_thing;
		itor->m_thing = 0;

		delete itor->m_graph;
		itor->m_graph = 0;

		++itor;
	}
}

//=====================================================================================//
//                             void Depot::MakeSaveLoad()                              //
//=====================================================================================//
void Depot::MakeSaveLoad(SavSlot& st)
{
	if(st.IsSaving())
	{
		info_vec_t::iterator itor = m_infos.begin();

		while(itor != m_infos.end())
		{
			if(itor->m_thing)
			{
				st << itor - m_infos.begin();

				DynUtils::SaveObj(st, itor->m_thing);                
				itor->m_thing->MakeSaveLoad(st);

				DynUtils::SaveObj(st, itor->m_graph);
				itor->m_graph->MakeSaveLoad(st);
			}
			++itor;
		}

		st << -1;

	}
	else
	{
		int   id;
		st >> id;

		while(id >= 0)
		{
			if(id >= (int)m_infos.size()) m_infos.resize(id + 1);

			DynUtils::LoadObj(st, m_infos[id].m_thing);
			m_infos[id].m_thing->MakeSaveLoad(st);

			DynUtils::LoadObj(st, m_infos[id].m_graph);
			m_infos[id].m_graph->MakeSaveLoad(st);

			st >> id;
		}
	}
}

//=====================================================================================//
//                                 tid_t Depot::Push()                                 //
//=====================================================================================//
tid_t Depot::Push(const ipnt2_t& pnt, BaseThing* bit)
{
	info_vec_t::iterator itor = m_infos.begin();
	while(itor != m_infos.end())
	{
		if(itor->m_thing == 0)
		{
			itor->m_thing = bit;
			itor->m_graph = GraphFactory::CreateGraphThing(bit, pnt);

			bit->SetTID(itor - m_infos.begin());
			return bit->GetTID();
		}

		++itor;
	}

	m_infos.push_back(info_s());
	m_infos.back().m_thing = bit;
	m_infos.back().m_graph = GraphFactory::CreateGraphThing(bit, pnt);

	bit->SetTID(m_infos.size() - 1);
	return bit->GetTID();
}

//=====================================================================================//
//                                void Depot::Remove()                                 //
//=====================================================================================//
void Depot::Remove(tid_t tid, rm_mode mode)
{
	if(m_infos[tid].m_thing)
	{
		if(mode == RM_UNLINK)
		{
			m_infos[tid].m_thing->SetTID(0);
			m_infos[tid].m_thing = 0;
		}
		else
		{
			delete m_infos[tid].m_thing;
			m_infos[tid].m_thing = 0;
		}

		delete m_infos[tid].m_graph;
		m_infos[tid].m_graph = 0;
	}
}

//=====================================================================================//
//                                void Depot::Remove()                                 //
//=====================================================================================//
void Depot::Remove(iterator it, rm_mode mode)
{    
	Remove(it.m_first, mode);
}

//=====================================================================================//
//                           Depot::iterator Depot::begin()                            //
//=====================================================================================//
Depot::iterator Depot::begin(const ipnt2_t& center, float radius, unsigned type)
{
	CircleIterateCondition::GetInst()->Init(center, radius, type);
	return iterator(&m_infos, 0, CircleIterateCondition::GetInst());
}

//=====================================================================================//
//                           Depot::iterator Depot::begin()                            //
//=====================================================================================//
Depot::iterator Depot::begin(BaseEntity* entity, unsigned type)
{
	FOSIterateCondition::GetInst()->Init(entity, type);
	return iterator(&m_infos, 0, FOSIterateCondition::GetInst());
}

//=====================================================================================//
//                            Depot::iterator Depot::end()                             //
//=====================================================================================//
Depot::iterator Depot::end()
{
	return iterator(0, m_infos.size(), 0);
}

//=======================================================================

//=====================================================================================//
//                            Depot::Iterator::~Iterator()                             //
//=====================================================================================//
Depot::Iterator::~Iterator()
{
}

//=====================================================================================//
//                             Depot::Iterator::Iterator()                             //
//=====================================================================================//
Depot::Iterator::Iterator(Depot::info_vec_t* vec, unsigned first, Condition* condition)
:	m_first(first),
m_infos(vec),
m_condition(condition)
{
	if(     m_infos
		&&  m_infos->size()
		&&  m_condition 
		&&  !m_condition->IsSuitable((*m_infos)[m_first].m_thing, (*m_infos)[m_first].m_graph))
		operator++();           
}

//=====================================================================================//
//                   Depot::Iterator& Depot::Iterator::operator++()                    //
//=====================================================================================//
Depot::Iterator& Depot::Iterator::operator++()
{
	m_first++;
	while(m_first < m_infos->size())
	{
		if(m_condition->IsSuitable((*m_infos)[m_first].m_thing, (*m_infos)[m_first].m_graph))
			return *this;

		++m_first;
	}

	m_first = m_infos->size();
	return *this;
}

//=======================================================================

ThingScriptParser::ThingScriptParser(Acceptor* acceptor, ThingFactory* factory) :
m_acceptor(acceptor), m_factory(factory)
{
}

ThingScriptParser::~ThingScriptParser()
{
}

void ThingScriptParser::SetAcceptor(Acceptor* acceptor)
{
	m_acceptor = acceptor;
}

void ThingScriptParser::SetFactory(ThingFactory* factory)
{
	m_factory = factory;
}

bool ThingScriptParser::Parse(const std::string& script)
{
	std::string str = script;
	std::transform(str.begin(), str.end(), str.begin(), tolower);

	m_input.str(str);
	m_input.clear();

	try{

		int value;

		BaseThing*  thing;        
		token_type  token;

		//разбор скрипта
		while(true){

			token = GetToken(&str, &value);

			//если дошли до конца скрипта
			if(token == T_END) break;

			switch(token){
case T_AMMO:
	thing = ParseAmmo();
	break;

case T_CAMERA:
	thing = ParseCamera();
	break;

case T_SHIELD:
	thing = ParseShield();
	break;

case T_WEAPON:
	thing = ParseWeapon();
	break;

case T_ARMOR:
	thing = ParseArmor();
	break;

case T_IMPLANT:
	thing = ParseImplant();
	break;

case T_GRENADE:
	thing = ParseGrenade();
	break;

case T_MEDIKIT:
	thing = ParseMedikit();
	break;

case T_MONEY:
	thing = ParseMoney();
	break;

case T_KEY:
	thing = ParseKey();
	break;

case T_SCANNER:
	thing = ParseScanner();
	break;

case T_INFO:
	thing = 0;
	ParseInfo();
	break;

default:

	throw ParseError();
			}

			//передадим предмет приемщику
			if(thing && (m_acceptor == 0 || !m_acceptor->AcceptThing(thing)))
				delete thing;
		}
	}
	catch(LexerError& ){
		return false;
	}
	catch(ParseError& ){
		return false;
	}

	return true;
}

int ThingScriptParser::GetLastPos()
{
	return m_input.tellg();
}

ThingScriptParser::token_type ThingScriptParser::GetToken(std::string* str, int* num)
{    
	//пропустить пробелы
	m_input >> std::ws;

	//дошли до конца?
	if(!m_input.good())
		return T_END; 

	int ch = m_input.peek();

	if(isdigit(ch)){
		m_input >> *num;        
		return T_NUMBER;
	}

	if(iscsym(ch)){

		str->clear();

		do{
			*str += m_input.get();            
		}while(iscsym(m_input.peek()));

		if(*str == "key") return T_KEY;
		if(*str == "ammo") return T_AMMO;
		if(*str == "armor") return T_ARMOR;
		if(*str == "money") return T_MONEY;
		if(*str == "camera") return T_CAMERA;
		if(*str == "shield") return T_SHIELD;
		if(*str == "weapon") return T_WEAPON;
		if(*str == "scanner") return T_SCANNER;
		if(*str == "grenade") return T_GRENADE;
		if(*str == "implant") return T_IMPLANT;
		if(*str == "medikit") return T_MEDIKIT;

		if(*str == "info") return T_INFO;
		if(*str == "human") return T_HUMAN;
		if(*str == "trader") return T_TRADER;
		if(*str == "vehicle") return T_VEHICLE;
		if(*str == "organization") return T_ORGANIZATION; 

		return T_IDENT;
	}

	//символы пунктуации
	if(ch == '('){
		m_input.get();
		return T_LBRACKET;
	}

	if(ch == ')'){
		m_input.get();
		return T_RBRACKET;
	}

	if(ch ==  ','){
		m_input.get();
		return T_COMMA;
	}

	throw LexerError();
}

BaseThing* ThingScriptParser::ParseAmmo()
{
	int value = 0, ammo_num = 0;
	std::string str, ammo_rid;

	if(GetToken(&str, &value) != T_LBRACKET)
		throw ParseError();

	if(GetToken(&ammo_rid, &value) != T_IDENT)
		throw ParseError();

	token_type token = GetToken(&str, &value);    

	if(token == T_COMMA){

		if(GetToken(&str, &ammo_num) != T_NUMBER)
			throw ParseError();

		token = GetToken(&str, &value);    
	}

	if(token != T_RBRACKET) throw ParseError();

	return m_factory ? m_factory->CreateAmmo(ammo_rid, ammo_num) : 0;
}

BaseThing* ThingScriptParser::ParseArmor()
{
	int value = 0;
	std::string str, rid;

	if(GetToken(&str, &value) != T_LBRACKET)
		throw ParseError();

	if(GetToken(&rid, &value) != T_IDENT)
		throw ParseError();

	if(GetToken(&str, &value) != T_RBRACKET)
		throw ParseError();

	return m_factory ? m_factory->CreateArmor(rid) : 0;
}

BaseThing* ThingScriptParser::ParseWeapon()
{
	int value = 0, count = 0;
	std::string wrid, arid, str;

	if(GetToken(&str, &value) != T_LBRACKET)
		throw ParseError();

	if(GetToken(&wrid, &value) != T_IDENT)
		throw ParseError();

	token_type token = GetToken(&wrid, &value);

	if(token == T_COMMA){

		if(GetToken(&arid, &value) != T_IDENT)
			throw ParseError();

		token = GetToken(&str, &value);

		if(token == T_COMMA){
			if(GetToken(&str, &count) != T_NUMBER)
				throw ParseError();

			token = GetToken(&str, &value);
		}
	}

	if(token != T_RBRACKET) throw ParseError();

	return m_factory ? m_factory->CreateWeapon(wrid, arid, count) : 0;
}

BaseThing* ThingScriptParser::ParseImplant()
{
	int value;
	std::string str, rid;

	if(GetToken(&str, &value) != T_LBRACKET)
		throw ParseError();

	if(GetToken(&rid, &value) != T_IDENT)
		throw ParseError();

	if(GetToken(&str, &value) != T_RBRACKET)
		throw ParseError();

	return m_factory ? m_factory->CreateImplant(rid) : 0;
}

BaseThing* ThingScriptParser::ParseGrenade()
{
	int value;
	std::string str, rid;

	if(GetToken(&str, &value) != T_LBRACKET)
		throw ParseError();

	if(GetToken(&rid, &value) != T_IDENT)
		throw ParseError();

	if(GetToken(&str, &value) != T_RBRACKET)
		throw ParseError();

	return m_factory ? m_factory->CreateGrenade(rid) : 0;
}

BaseThing* ThingScriptParser::ParseMedikit()
{
	int value, count = 0;
	std::string str, rid;

	if(GetToken(&str, &value) != T_LBRACKET)
		throw ParseError();

	if(GetToken(&rid, &value) != T_IDENT)
		throw ParseError();

	token_type token = GetToken(&str, &value);
	if(token == T_COMMA){

		if(GetToken(&str, &count) != T_NUMBER)
			throw ParseError();

		token = GetToken(&str, &value);
	}

	if(token != T_RBRACKET) throw ParseError();

	return m_factory ? m_factory->CreateMedikit(rid, count) : 0;
}

BaseThing* ThingScriptParser::ParseMoney()
{
	int value, count = 0;
	std::string str;

	if(GetToken(&str, &value) != T_LBRACKET)
		throw ParseError();

	token_type token = GetToken(&str, &value);

	if(token == T_NUMBER){
		count = value;
		token = GetToken(&str, &value);
	}

	if(token != T_RBRACKET) throw ParseError();

	return m_factory ? m_factory->CreateMoney(count) : 0;
}

BaseThing* ThingScriptParser::ParseKey()
{
	int value;
	std::string str, rid;

	if(GetToken(&str, &value) != T_LBRACKET)
		throw ParseError();

	if(GetToken(&rid, &value) != T_IDENT)
		throw ParseError();

	if(GetToken(&str, &value) != T_RBRACKET)
		throw ParseError();

	return m_factory ? m_factory->CreateKey(rid) : 0;
}

BaseThing* ThingScriptParser::ParseCamera()
{
	int value;
	std::string str, rid;

	if(GetToken(&str, &value) != T_LBRACKET)
		throw ParseError();

	if(GetToken(&rid, &value) != T_IDENT)
		throw ParseError();

	if(GetToken(&str, &value) != T_RBRACKET)
		throw ParseError();

	return m_factory ? m_factory->CreateCamera(rid) : 0;
}

BaseThing* ThingScriptParser::ParseShield()
{
	int value;
	std::string str, rid;

	if(GetToken(&str, &value) != T_LBRACKET)
		throw ParseError();

	if(GetToken(&rid, &value) != T_IDENT)
		throw ParseError();

	if(GetToken(&str, &value) != T_RBRACKET)
		throw ParseError();

	return m_factory ? m_factory->CreateShield(rid) : 0;
}

BaseThing* ThingScriptParser::ParseScanner()
{
	int value;
	std::string str, rid;

	if(GetToken(&str, &value) != T_LBRACKET)
		throw ParseError();

	if(GetToken(&rid, &value) != T_IDENT)
		throw ParseError();

	if(GetToken(&str, &value) != T_RBRACKET)
		throw ParseError();

	return m_factory ? m_factory->CreateScanner(rid) : 0;
}

void ThingScriptParser::ParseInfo()
{
	int value;
	std::string str;

	if(GetToken(&str, &value) != T_LBRACKET)
		throw ParseError();

	token_type token = GetToken(&str, &value);

	if(     token != T_KEY
		&&  token != T_AMMO
		&&  token != T_ARMOR
		&&  token != T_HUMAN
		&&  token != T_CAMERA
		&&  token != T_SHIELD
		&&  token != T_WEAPON
		&&  token != T_TRADER
		&&  token != T_VEHICLE
		&&  token != T_GRENADE
		&&  token != T_IMPLANT
		&&  token != T_MEDIKIT
		&&  token != T_SCANNER
		&&  token != T_ORGANIZATION)
		throw ParseError();

	if(GetToken(&str, &value) != T_COMMA)
		throw ParseError();

	rid_t rid;

	if(GetToken(&rid, &value) != T_IDENT)
		throw ParseError();

	if(GetToken(&str, &value) != T_RBRACKET)
		throw ParseError();

	//прочитали одно описание, передадим его приемщику
	if(m_acceptor){

		switch(token){
case T_KEY: 
	m_acceptor->AcceptThingInfo(TT_KEY, rid);
	break;
case T_AMMO:
	m_acceptor->AcceptThingInfo(TT_AMMO, rid);
	break;
case T_ARMOR:
	m_acceptor->AcceptThingInfo(TT_ARMOUR, rid);
	break;
case T_CAMERA:
	m_acceptor->AcceptThingInfo(TT_CAMERA, rid);
	break;
case T_SHIELD:
	m_acceptor->AcceptThingInfo(TT_SHIELD, rid);
	break;
case T_HUMAN:
	m_acceptor->AcceptEntInfo(ET_HUMAN, rid);
	break;
case T_WEAPON:
	m_acceptor->AcceptThingInfo(TT_WEAPON, rid);
	break;
case T_TRADER:
	m_acceptor->AcceptEntInfo(ET_TRADER, rid);
	break;
case T_VEHICLE:
	m_acceptor->AcceptEntInfo(ET_VEHICLE, rid);
	break;
case T_GRENADE:
	m_acceptor->AcceptThingInfo(TT_GRENADE, rid);
	break;
case T_SCANNER:
	m_acceptor->AcceptThingInfo(TT_SCANNER, rid);
	break;
case T_IMPLANT:
	m_acceptor->AcceptThingInfo(TT_IMPLANT, rid);
	break;
case T_MEDIKIT:
	m_acceptor->AcceptThingInfo(TT_MEDIKIT, rid);
	break;
case T_ORGANIZATION:
	m_acceptor->AcceptOrgInfo(rid);
	break;
		}
	} 
}

//=======================================================================

ItemSetParser::ItemSetParser(Acceptor* acceptor) : 
m_acceptor(acceptor)
{
}

bool ItemSetParser::Parse(const std::string& script)
{
	m_input.str(script);
	m_input.clear();

	try{

		node_s root;
		info_s info;
		token_type token = GetToken(&info);

		if(token == T_NULL || token == T_END)
			return true;

		if(token == T_IDENT){                                        
			root.m_type = T_ADD;
			root.m_rids.push_back(info.m_ident);
			SpawnThings(root);
			return true;
		}

		if(token == T_LBRACKET){
			ParseBlock(&root);
			SpawnThings(root);
			return true;
		}
	}
	catch(LexerErr& ) { return false; }            
	catch(ParseErr& ) { return false; }

	return true;
}

int ItemSetParser::GetLastPos()
{
	return m_input.tellg();
}

void ItemSetParser::SetAcceptor(Acceptor* acceptor)
{
	m_acceptor = acceptor;
}

ItemSetParser::token_type ItemSetParser::GetToken(info_s* info)
{
	//сбросим пробелы
	m_input >> std::ws;

	//проиниц структуру
	info->m_number = 0;
	info->m_ident.clear();

	if(!m_input.good()) return T_END;

	int ch = m_input.peek();

	if(isdigit(ch)){
		m_input >> info->m_number;
		return T_NUMBER;
	}

	if(iscsym(ch)){

		do{
			info->m_ident += m_input.get();
		}while(iscsym(m_input.peek()));

		if(info->m_ident == "none") return T_NULL;

		return T_IDENT;
	}

	if(ch == '~'){
		m_input.get();
		return T_RND;
	}

	if(ch == '+'){
		m_input.get();
		return T_ADD;
	}

	if(ch == '('){
		m_input.get();
		return T_LBRACKET;
	}

	if(ch == ')'){
		m_input.get();
		return T_RBRACKET;
	}

	if(ch == '-'){
		m_input.get();
		return T_NULL;
	}

	throw LexerErr();
}

void ItemSetParser::ParseBlock(node_s* node)
{
	info_s info;
	node->m_type = GetToken(&info);

	if(node->m_type != T_RND && node->m_type != T_ADD)
		throw ParseErr();

	while(true){

		int muls = 1;
		token_type token = GetToken(&info);

		if(token == T_NUMBER){
			muls  = info.m_number;
			token = GetToken(&info);
		}

		switch(token){
case T_NULL:
	while(muls--) node->m_rids.push_back(std::string());
	break;

case T_IDENT:
	while(muls--) node->m_rids.push_back(info.m_ident);
	break;

case T_LBRACKET:                    
	node->m_nodes.push_back(node_s());
	ParseBlock(&node->m_nodes.back());

	muls--;

	while(muls--) node->m_nodes.push_back(node->m_nodes.back());
	break;

case T_RBRACKET:
	return;

default:

	throw ParseErr();
		}             
	}
}

void ItemSetParser::SpawnThings(node_s& node)
{
	size_t k;

	switch(node.m_type){
case T_ADD:
	for(k = 0; k < node.m_rids.size(); SpawnThings(node.m_rids[k++]));
	for(k = 0; k < node.m_nodes.size(); SpawnThings(node.m_nodes[k++]));
	break;

case T_RND:

	k = aiRangeRand(node.m_rids.size() + node.m_nodes.size());

	if(k < node.m_rids.size())
		SpawnThings(node.m_rids[k]);
	else
		SpawnThings(node.m_nodes[k - node.m_rids.size()]);                
	break;
	}
}

void ItemSetParser::SpawnThings(const rid_t& rid)
{
	if(m_acceptor && rid.size()) m_acceptor->Accept(rid);
}

//=======================================================================

bool AIUtils::GetItemSetStr(const rid_t& rid, std::string* script, int* line)
{
	std::string file_name = item_sets_xls1;

	try{
		VFilePtr vfile1(file_name);
	}catch(CasusImprovisus&){
		file_name = item_sets_xls2;
	}

	TxtFilePtr txt(file_name);

	std::string str;                
	for(int k = 0; k < txt->SizeY(); k++){

		txt->GetCell(k, 0, str);

		if(str == rid){
			if(line) *line = k;
			txt->GetCell(k, 1, *script);
			return true;
		}
	}

	return false;
}


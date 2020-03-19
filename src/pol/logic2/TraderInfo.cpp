#include "logicdefs.h"

#include "thing.h"
#include "entity.h"
#include "xlsreader.h"

static const char* trader_xls = "scripts/logic/traders.txt";

//=====================================================================================//
//                               class TraderInfoReader                                //
//=====================================================================================//
class TraderInfoReader
{
	bool m_tableRead;
	mll::utils::table m_table;

	static const mll::utils::table &getTbl()
	{
		static bool tableRead = false;
		static mll::utils::table tbl;

		if(!tableRead)
		{
			PoL::VFileWpr vfile(trader_xls);
			if(vfile.get() && vfile->Size())
			{
				PoL::VFileBuf buf(vfile.get());
				std::istream stream(&buf);

				stream >> tbl;
			}

			tableRead = true;
		}

		return tbl;
	}

public:
    static void Read(TraderInfo &info)
	{
		using mll::utils::table;

		const table &tbl = getTbl();

		table::row_type sysid = tbl.row(0);
		table::col_type hdr = tbl.col(0);
		table::row_type::iterator it = std::find(sysid.begin(),sysid.end(),info.m_rid);

		if(it == sysid.end())
		{
			throw CASUS(("TraderInfoReader: не найден profile для <" + info.m_rid + ">!").c_str());
		}

		table::col_type trader = tbl.col(it);

		info.m_name = PoL::getLocStr("traders." + info.m_rid + ".name");
		info.m_desc = PoL::getLocStr("traders." + info.m_rid + ".description");

		read_mode old_mode = RM_NONE, mode = RM_NONE;
		info.m_tradingForMoney = true;

		for(int line = 1; line != tbl.height(); ++line)
		{
			switch(mode = GetReadMode(hdr[line]))
			{
			case RM_SKIN:
				info.m_skin = trader[line];
				break;
			case RM_ANIMATIONS:
				info.m_model = trader[line];
				break;
			case RM_WEAPONS:
			case RM_AMMUNITION:
			case RM_GRENADES:
			case RM_ARMOUR:
			case RM_MEDIKIT:
			case RM_SCANNER:
			case RM_CAMERA:
			case RM_SHIELD:
			case RM_IMPLANT:
			case RM_KEY:
				old_mode = mode;
				break;
			case RM_LINE:
				AddThing(info,old_mode,hdr[line],trader[line]);
				break;
			case RM_TRADE_FOR:
				info.m_tradingForMoney = (trader[line] != "rank");
				break;
			}
		}
	}

private:
	enum read_mode
	{
		RM_NONE,
		RM_SKIN,
		RM_ANIMATIONS,
		//RM_DESC,
		//RM_NAME,
		RM_SYSNAME,
		RM_PROPERIES,
		RM_WEAPONS,
		RM_AMMUNITION,
		RM_GRENADES,
		RM_ARMOUR,
		RM_MEDIKIT,
		RM_SCANNER,
		RM_CAMERA,
		RM_SHIELD,
		RM_IMPLANT,
		RM_KEY,
		RM_LINE,
		RM_TRADE_FOR
	};

	static read_mode GetReadMode(const std::string & str)
	{
		//if(str == "name")       return RM_NAME;
		//if(str == "description")return RM_DESC;
		if(str == "skin")       return RM_SKIN;
		if(str == "sys_name")   return RM_SYSNAME;
		if(str == "animations") return RM_ANIMATIONS;
		if(str == "properties") return RM_PROPERIES;
		if(str == "weapons")    return RM_WEAPONS;
		if(str == "ammunition") return RM_AMMUNITION;
		if(str == "grenades")   return RM_GRENADES;
		if(str == "armour")     return RM_ARMOUR;
		if(str == "medikit")    return RM_MEDIKIT;
		if(str == "scanner")    return RM_SCANNER;
		if(str == "camera")     return RM_CAMERA;
		if(str == "shield")     return RM_SHIELD;
		if(str == "implant")    return RM_IMPLANT;
		if(str == "keys")       return RM_KEY;
		if(str == "trade_for")	return RM_TRADE_FOR;

		return RM_LINE;
	}

	static bool ParseTag(TraderInfo::ratio_s* tag, const std::string& str)
	{   
		tag->m_buy = tag->m_sell = -1;

		char comma;
		std::istringstream istr(str);

		istr >> tag->m_sell >> comma >> tag->m_buy;
		return istr.good();
	}

	static void AddThing(TraderInfo &info, read_mode rm, const std::string &sysid, const std::string &stag)
	{
		TraderInfo::ratio_s tag;

		if(ParseTag(&tag, stag))
		{
			switch(rm)
			{
			case RM_WEAPONS:
				info.m_ratioMaps[TT_WEAPON][sysid] = tag;
				break;
			case RM_AMMUNITION:
				info.m_ratioMaps[TT_AMMO][sysid] = tag;
				break;
			case RM_GRENADES:
				info.m_ratioMaps[TT_GRENADE][sysid] = tag;
				break;
			case RM_ARMOUR:
				info.m_ratioMaps[TT_ARMOUR][sysid] = tag;
				break;
			case RM_MEDIKIT:
				info.m_ratioMaps[TT_MEDIKIT][sysid] = tag;
				break;
			case RM_SCANNER:
				info.m_ratioMaps[TT_SCANNER][sysid] = tag;
				break;
			case RM_CAMERA:
				info.m_ratioMaps[TT_CAMERA][sysid] = tag;
				break;
			case RM_SHIELD:
				info.m_ratioMaps[TT_SHIELD][sysid] = tag;
				break;
			case RM_IMPLANT:
				info.m_ratioMaps[TT_IMPLANT][sysid] = tag;
				break;
			case RM_KEY:
				info.m_ratioMaps[TT_KEY][sysid] = tag;
				break;
			}
		}
	}
};


//=====================================================================================//
//                              TraderInfo::TraderInfo()                               //
//=====================================================================================//
TraderInfo::TraderInfo(const rid_t& rid)
:	EntityInfo(rid, ET_TRADER)
{
	TraderInfoReader().Read(*this);
}

//=====================================================================================//
//                   const std::string& TraderInfo::GetSkin() const                    //
//=====================================================================================//
const std::string& TraderInfo::GetSkin() const
{
	return m_skin;
}

//=====================================================================================//
//                   const std::string& TraderInfo::GetModel() const                   //
//=====================================================================================//
const std::string& TraderInfo::GetModel() const
{
	return m_model;
}

//=====================================================================================//
//                        int TraderInfo::CalcBuyPrice() const                         //
//=====================================================================================//
int TraderInfo::CalcBuyPrice(const ratio_map_t& map, const BaseThing* thing) const
{
	ratio_map_t::const_iterator itor = map.find(thing->GetInfo()->GetRID());
	if(itor != map.end()) return thing->GetCost() * itor->second.m_buy;

	ratio_s def;
	return thing->GetCost() * def.m_buy;
}

//=====================================================================================//
//                        int TraderInfo::CalcSellPrice() const                        //
//=====================================================================================//
int TraderInfo::CalcSellPrice(const ratio_map_t& map, const BaseThing* thing) const
{
	ratio_map_t::const_iterator itor = map.find(thing->GetInfo()->GetRID());
	if(itor != map.end()) return thing->GetCost() * itor->second.m_sell;

	ratio_s def;
	return thing->GetCost() * def.m_sell;
}

//=====================================================================================//
//                         int TraderInfo::GetBuyPrice() const                         //
//=====================================================================================//
int TraderInfo::GetBuyPrice(const BaseThing* thing) const
{
	if(m_tradingForMoney)
	{
		RatioMaps_t::const_iterator it = m_ratioMaps.find(thing->GetInfo()->GetType());
		int result = it != m_ratioMaps.end() ? CalcBuyPrice(it->second,thing) : 0;
		return std::max(1,result);
	}
	else
	{
		return 0;
	}
}

//=====================================================================================//
//                        int TraderInfo::GetSellPrice() const                         //
//=====================================================================================//
int TraderInfo::GetSellPrice(const BaseThing* thing) const
{
	if(m_tradingForMoney)
	{
		RatioMaps_t::const_iterator it = m_ratioMaps.find(thing->GetInfo()->GetType());
		int result = it != m_ratioMaps.end() ? CalcSellPrice(it->second,thing) : 0;
		return std::max(1,result);
	}
	else
	{
		return 0;
	}
}

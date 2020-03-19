#include "precomp.h"
#include "Grid.h"
#include "ParseUtils.h"
#include "DynObjectOldData.h"
#include <mll/utils/animation.h>
#include <mll/utils/named_vars.h>
#include <Common/Utils/OptSlot.h>

namespace DynObjectOldDataDetail
{

//=====================================================================================//
//                                 struct KeyValuePair                                 //
//=====================================================================================//
struct KeyValuePair
{
	std::string m_key;
	std::string m_value;

	KeyValuePair()
	{
	}
	KeyValuePair( const std::string &k )
	:	m_key(k)
	{
	}

	bool operator<( const KeyValuePair &l ) const
	{
		return m_key < l.m_key;
	}
	bool operator==( const KeyValuePair &l ) const
	{
		return m_key == l.m_key;
	}
};

//=====================================================================================//
//                                 struct exp_types_t                                  //
//=====================================================================================//
struct exp_type_t
{
	DestructibleInfo::PrimaryEffect m_pe;
	DestructibleInfo::SecondaryEffect m_se;
	DestructibleInfo::TertiaryEffect m_te;

	hit_s::damage_type m_dam;
	float m_val;
	std::string m_name;

	exp_type_t(DestructibleInfo::PrimaryEffect pe, hit_s::damage_type dt, float val, const std::string &name)
	:	m_pe(pe),
		m_se(DestructibleInfo::seNone),
		m_te(DestructibleInfo::teNone),
		m_dam(dt),
		m_val(val),
		m_name(name)
	{
	}

	exp_type_t(DestructibleInfo::SecondaryEffect se, hit_s::damage_type dt, float val, const std::string &name)
	:	m_pe(DestructibleInfo::peNone),
		m_se(se),
		m_te(DestructibleInfo::teNone),
		m_dam(dt),
		m_val(val),
		m_name(name)
	{
	}

	exp_type_t(DestructibleInfo::TertiaryEffect te, hit_s::damage_type dt, float val, const std::string &name)
	:	m_pe(DestructibleInfo::peNone),
		m_se(DestructibleInfo::seNone),
		m_te(te),
		m_dam(dt),
		m_val(val),
		m_name(name)
	{
	}
};

static const exp_type_t exp_types[] = {
	exp_type_t(DestructibleInfo::peGlass,      hit_s::DT_STRIKE, 5,      "glass"),
	exp_type_t(DestructibleInfo::teBlast,      hit_s::DT_STRIKE, 10,     "blast"),
	exp_type_t(DestructibleInfo::peMetal,      hit_s::DT_STRIKE, 3,      "metal"),
	exp_type_t(DestructibleInfo::peWood,       hit_s::DT_STRIKE, 2,       "wood"),
	exp_type_t(DestructibleInfo::peStone,      hit_s::DT_STRIKE, 3,      "stone"),
	exp_type_t(DestructibleInfo::peCarton,     hit_s::DT_NONE,   0,     "carton"),
	exp_type_t(DestructibleInfo::seBlackSmoke, hit_s::DT_STRIKE, 0, "blacksmoke"),
	exp_type_t(DestructibleInfo::seWhiteSmoke, hit_s::DT_STRIKE, 0, "whitesmoke"),
	exp_type_t(DestructibleInfo::seFire,       hit_s::DT_FLAME,  3,       "fire"),
	exp_type_t(DestructibleInfo::seSparkles,   hit_s::DT_STRIKE, 0,   "sparkles")
};

static const size_t exp_types_count = sizeof(exp_types)/sizeof(exp_types[0]);

struct damage_type_t
{
	std::string m_name;
	hit_s::damage_type m_dt;
};

static const damage_type_t damage_types[] = {
	{"strike:",    hit_s::DT_STRIKE},
	{"shock:",     hit_s::DT_SHOCK},
	{"energy:",    hit_s::DT_ENERGY},
	{"electric:",  hit_s::DT_ELECTRIC},
	{"explosive:", hit_s::DT_EXPLOSIVE},
	{"flame:",     hit_s::DT_FLAME}
};
static const size_t damage_types_count = sizeof(damage_types)/sizeof(damage_types[0]);


struct damage_t
{
	hit_s::damage_type m_dam;
	float m_val;
	int m_start;

	bool operator<(const damage_t &r) const { return m_start < r.m_start; }
};

}

using namespace DynObjectOldDataDetail;

//=====================================================================================//
//                        DynObjectOldData::DynObjectOldData()                         //
//=====================================================================================//
DynObjectOldData::DynObjectOldData()
:	m_name("@#absolutely_wrong_object_name#@"),
    m_type(0),
    m_physType(0),
    m_animState(0.0f),
    m_doorLocked(false)
{
}

//=====================================================================================//
//                        DynObjectOldData::DynObjectOldData()                         //
//=====================================================================================//
DynObjectOldData::DynObjectOldData( SavSlot &slot )
:	m_name("@#absolutely_wrong_object_name#@"),
    m_type(0),
    m_physType(0),
    m_animState(0.0f),
    m_doorLocked(false)
{
	Read(slot);
}

//=====================================================================================//
//                          void DynObjectOldData::LoadVars()                          //
//=====================================================================================//
void DynObjectOldData::ReadVars(const mll::utils::named_vars &vars, int floor)
{
	if(vars.exists("Name")) m_name = vars["Name"].value();
	else if(vars.exists("name")) m_name = vars["name"].value();
	else throw CASUS("DynObjectOldData::ReadVars: No object name");

	std::transform(m_name.begin(), m_name.end(), m_name.begin(), std::tolower);

	m_isRoof = (floor==2);

	Grid *grid = (Grid *) HexGrid::GetInst();
	if(grid && !grid->ManDirect[GetName()].empty() || !grid->ManInvert[GetName()].empty())
	{
		m_type |= otDoor;
	}

	if(grid && !grid->HeightDirect[GetName()].empty())
	{
		m_type |= otElevator;
	}

	typedef mll::utils::named_vars::const_iterator Itor_t;

	for(Itor_t it = vars.begin(); it != vars.end(); ++it)
	{
		std::string key = it->name();
		std::string value = it->value();
		std::transform(key.begin(),key.end(),key.begin(),std::tolower);
		std::transform(value.begin(),value.end(),value.begin(),std::tolower);

		if(key == "invisible")
		{
			if(value == "1" || value == "yes" || value == "true")
			{
				m_type |= otInvisible;
			}
		}
		else if(key == "use")
		{
			m_type |= otUsable;
			m_useInfo.m_useHint = value;
		}
		else if(key == "usekey")
		{
			m_type |= otUsable;
			m_useInfo.m_key = value;
		}
		else if(key == "usewisdom")
		{
			m_type |= otUsable;
			std::istringstream(value) >> m_useInfo.m_wisdom;
		}
		else if(key == "animated")
		{
			if(value == "auto")
			{
				m_type |= otAnimated;
			}
		}
		else if(key == "ghost")
		{
			if(value == "yes")
			{
				m_physType |= ptGhost;
			}
		}
		else if(key == "transparent")
		{
			if(value == "yes")
			{
				m_physType |= ptTransparent;
			}
		}
		else if(key == "station")
		{
			assert(!value.empty());
			m_type |= otTrain;
			ReadStation(value);
		}
		else if(key == "sounds")
		{
			ReadSounds(value);
		}
		else if(key == "crushtype" || key == "crashtype")
		{
			ReadDestructible(value);
			//if(m_destructibleInfo.m_hitPoints != 10000) m_type |= otDestructible;
		}
		else if(key == "hitpoints")
		{
			std::istringstream(value) >> m_destructibleInfo.m_hitPoints;
			if(m_destructibleInfo.m_hitPoints != 10000)
			{
				m_type |= otDestructible;
			}
		}
		else if(key == "storage")
		{
			assert(!value.empty());
			m_type |= otUsable | otStorage;
			ReadLevelCase(value);
		}
		else if(key == "database")
		{
			ReadDatabase(value);
			m_type |= otDatabase | otUsable;
		}
		else if(key == "curstate")
		{
			std::istringstream(value) >> m_animState;
			m_doorLocked = m_animState > 0.5f && IsDoor();
		}
	}

	if(!m_useInfo.m_key.empty() && !m_useInfo.m_wisdom)
	{
		m_useInfo.m_wisdom = 150; // если дверь с ключом и не указан порог ума для взлома, то по умолчанию 150
	}

	// fixme: сделать по-другому
	if(IsTrain() && IsAnimated())
	{
		m_type ^= otAnimated;
	}
}

//=====================================================================================//
//                            void DynObjectOldData::Read()                            //
//=====================================================================================//
void DynObjectOldData::Read( SavSlot &slot )
{
	*this = DynObjectOldData();
	assert(slot.GetStore()->GetVersion() <= 4);

	Read4(slot);
}

//=====================================================================================//
//                           void DynObjectOldData::Read4()                            //
//=====================================================================================//
void DynObjectOldData::Read4( SavSlot &slot )
{
	OptSlot opts;
	opts.Load(slot);

	typedef std::vector<KeyValuePair> PropMap_t;
	typedef PropMap_t::iterator Itor_t;

	PropMap_t props;

	int q;
	opts >> q;

	props.resize(q);


	for(int i = 0; i < q; ++i)
	{
		opts >> props[i].m_key >> props[i].m_value;
	}

	// Прочтем имя объекта
	{
		std::pair<Itor_t, Itor_t> itp = std::equal_range(props.begin(), props.end(), KeyValuePair("name"));
		assert(std::distance(itp.first, itp.second) == 1);

		m_name = itp.first->m_value;
		std::transform(m_name.begin(), m_name.end(), m_name.begin(), std::tolower);
	}

	Grid *grid = (Grid *) HexGrid::GetInst();
	if(grid && !grid->ManDirect[GetName()].empty() || !grid->ManInvert[GetName()].empty())
	{
		m_type |= otDoor;
	}

	if(grid && !grid->HeightDirect[GetName()].empty())
	{
		m_type |= otElevator;
	}

	for(Itor_t it = props.begin(); it != props.end(); ++it)
	{
		if(it->m_key == "use")
		{
			m_type |= otUsable;
			m_useInfo.m_useHint = it->m_value;
		}
		else if(it->m_key == "usekey")
		{
			m_type |= otUsable;
			m_useInfo.m_key = it->m_value;
		}
		else if(it->m_key == "usewisdom")
		{
			m_type |= otUsable;
			std::istringstream(it->m_value) >> m_useInfo.m_wisdom;
		}
		else if(it->m_key == "animated")
		{
			if(it->m_value == "auto")
				m_type |= otAnimated;
		}
		else if(it->m_key == "ghost")
		{
			if(it->m_value == "yes")
				m_physType |= ptGhost;
		}
		else if(it->m_key == "transparent")
		{
			if(it->m_value == "yes")
				m_physType |= ptTransparent;
		}
		else if(it->m_key == "station")
		{
			assert(!it->m_value.empty());
			m_type |= otTrain;
			ReadStation(it->m_value);
		}
		else if(it->m_key == "sounds")
		{
			ReadSounds(it->m_value);
		}
		else if(it->m_key == "crushtype" || it->m_key == "crashtype")
		{
			ReadDestructible(it->m_value);
			//if(m_destructibleInfo.m_hitPoints != 10000) m_type |= otDestructible;
		}
		else if(it->m_key == "hitpoints")
		{
			std::istringstream(it->m_value) >> m_destructibleInfo.m_hitPoints;
			if(m_destructibleInfo.m_hitPoints != 10000)
				m_type |= otDestructible;
		}
		else if(it->m_key == "storage")
		{
			assert(!it->m_value.empty());
			m_type |= otUsable | otStorage;
			ReadLevelCase(it->m_value);
		}
		else if(it->m_key == "database")
		{
			ReadDatabase(it->m_value);
		}
		else if(it->m_key == "curstate")
		{
			std::istringstream(it->m_value) >> m_animState;
			m_doorLocked = m_animState > 0.5f && IsDoor();
		}
	}

	if(!m_useInfo.m_key.empty() && !m_useInfo.m_wisdom)
	{
		m_useInfo.m_wisdom = 150; // если дверь с ключом и не указан порог ума для взлома, то по умолчанию 150
	}

	// fixme: сделать по-другому
	if(IsTrain() && IsAnimated())
	{
		m_type ^= otAnimated;
	}
}

//=====================================================================================//
//                         void DynObjectOldData::ReadSounds()                         //
//=====================================================================================//
void DynObjectOldData::ReadSounds( const std::string &data )
{
	typedef boost::char_separator<char> sep_t;
	typedef boost::tokenizer<sep_t> tok_t;
	sep_t sep(", ;\t#");
	tok_t tok(data, sep);

	int index = 0;
	for(tok_t::iterator it = tok.begin(); it != tok.end() && index < SoundInfo::statesCount; ++it, ++index)
	{
		m_soundInfo.m_sounds[index] = *it;
	}

	const std::string none = "none", empty;
	std::replace(m_soundInfo.m_sounds, m_soundInfo.m_sounds + SoundInfo::statesCount, none, empty);
}

//=====================================================================================//
//                        void DynObjectOldData::ReadStation()                         //
//=====================================================================================//
void DynObjectOldData::ReadStation( const std::string &data )
{
	std::string str = KillSpaces(data);

	m_trainInfo.m_stopFrame = ParseUtils::GetInt(str, "stopanimation") / 30.0f;
	m_trainInfo.m_jointToSwitch = std::max(ParseUtils::GetInt(str, "switchjoint") - 1, 0);
	m_trainInfo.m_turnsAtStation = ParseUtils::GetInt(str, "stopturns");
	m_trainInfo.m_turnsOutOfLevel = ParseUtils::GetInt(str, "outturns");
	//m_trainInfo.m_turnsToGo			= rand() % m_trainInfo.m_turnsOutOfLevel;
}

//=====================================================================================//
//                        void DynObjectOldData::ReadDatabase()                        //
//=====================================================================================//
void DynObjectOldData::ReadDatabase( const std::string &data )
{
	std::string str = KillSpaces(data);

	assert(m_useInfo.m_exp == 0 && "Неоднозначно задан опыт от использования предмета");
	assert(m_useInfo.m_wisdom == 0 && "Неоднозначно задан интеллект, необходимый для использования предмета");
	assert(m_caseInfo.m_setName.empty() && "Неоднозначно задан набор предметов для объекта");
	m_useInfo.m_exp = ParseUtils::GetInt(str, "experience");
	m_useInfo.m_wisdom = ParseUtils::GetInt(str, "wisdom");
	m_caseInfo.m_setName = ParseUtils::GetStr(str, "items");
}

//=====================================================================================//
//                      void DynObjectOldData::ReadDestructible()                      //
//=====================================================================================//
void DynObjectOldData::ReadDestructible( const std::string &dta )
{
	std::string data = dta;
	m_destructibleInfo.m_primaryEffect = DestructibleInfo::peGlass;
	m_destructibleInfo.m_damage.m_dmg[0].Type = hit_s::DT_NONE;
	m_destructibleInfo.m_damage.m_dmg[1].Type = hit_s::DT_NONE;
	m_destructibleInfo.m_damage.m_dmg[2].Type = hit_s::DT_NONE;

	if(data.find("#") != std::string::npos)
	{
		std::transform(data.begin(), data.end(), data.begin(), std::tolower);
		std::string explosion_type = ParseUtils::GetStr(data,"type");
		m_destructibleInfo.m_damage.m_radius = ParseUtils::GetFloat(data,"rad");
		if(m_destructibleInfo.m_damage.m_radius == 0.0f) m_destructibleInfo.m_damage.m_radius = 1.f;

		bool skipother=false;
		for(int t=0; t != exp_types_count; t++)
		{
			if(explosion_type.find(exp_types[t].m_name) != std::string::npos)
			{
				m_destructibleInfo.m_primaryEffect = exp_types[t].m_pe;
				m_destructibleInfo.m_secondaryEffect = exp_types[t].m_se;
				m_destructibleInfo.m_tertiaryEffect = exp_types[t].m_te;

				if(!skipother)
				{
					skipother=true;
					m_destructibleInfo.m_damage.m_dmg[0].Type = exp_types[t].m_dam;
					m_destructibleInfo.m_damage.m_dmg[0].Value = exp_types[t].m_val;
				}
			}
		}

		std::string damage = ParseUtils::GetStr(data,"dam");

		if(!damage.empty())
		{
			static const unsigned int damages_count = 2;
			damage_t damages[damages_count];

			for(unsigned int i = 0; i < damages_count; ++i)
			{
				damages[i].m_dam = hit_s::DT_NONE;
			}

			int number=0;
			for(int j=0; j<damage_types_count && number<damages_count; j++)
			{
				size_t start = damage.find(damage_types[j].m_name);
				if(start != std::string::npos)
				{
					start += damage_types[j].m_name.length();

					size_t end = damage.find_first_of(",.;",start);
					if(end == std::string::npos)
					{
						end = damage.size();
					}

					//damages[number].val = atof(std::string(Damage,start+len,end-start-len).c_str());
					damages[number].m_val = boost::lexical_cast<float>(damage.substr(start,end-start));
					damages[number].m_start = start;
					damages[number].m_dam = damage_types[j].m_dt;

					number++;
				}
			}
			std::sort(damages,damages+damages_count);
		}
	}
	else
	{
		//новый вариант
		if(data.find("glass") != std::string::npos)
		{
			m_destructibleInfo.m_primaryEffect = DestructibleInfo::peGlass;
		}
		else if(data.find("metal") != std::string::npos)
		{
			m_destructibleInfo.m_primaryEffect = DestructibleInfo::peMetal;
		}
		else if(data.find("wood") != std::string::npos)
		{
			m_destructibleInfo.m_primaryEffect = DestructibleInfo::peWood;
		}
		else if(data.find("stone") != std::string::npos)
		{
			m_destructibleInfo.m_primaryEffect = DestructibleInfo::peStone;
		}
		else if(data.find("carton") != std::string::npos)
		{
			m_destructibleInfo.m_primaryEffect = DestructibleInfo::peCarton;
		}

		if(data.find("blacksmoke") != std::string::npos)
		{
			m_destructibleInfo.m_secondaryEffect = DestructibleInfo::seBlackSmoke;
		}
		else if(data.find("whitesmoke") != std::string::npos)
		{
			m_destructibleInfo.m_secondaryEffect = DestructibleInfo::seWhiteSmoke;
		}
		else if(data.find("fire") != std::string::npos)
		{
			m_destructibleInfo.m_secondaryEffect = DestructibleInfo::seFire;
		}
		else if(data.find("sparkles") != std::string::npos)
		{
			m_destructibleInfo.m_secondaryEffect = DestructibleInfo::seSparkles;
		}

		if(data.find("blast") != std::string::npos)
		{
			m_destructibleInfo.m_tertiaryEffect = DestructibleInfo::teBlast;
		}

		m_destructibleInfo.m_damage.m_radius = 2.0f;
		m_destructibleInfo.m_damage.m_dmg[0].Type = hit_s::DT_STRIKE;
		m_destructibleInfo.m_damage.m_dmg[0].Value = 20.0f;//fixme:число от балды
	}
}

//=====================================================================================//
//                       void DynObjectOldData::ReadLevelCase()                        //
//=====================================================================================//
void DynObjectOldData::ReadLevelCase( const std::string &data )
{
	std::string str = KillSpaces(data);

	assert(m_useInfo.m_exp == 0 && "Неоднозначно задан опыт от использования предмета");
	assert(m_useInfo.m_wisdom == 0 && "Неоднозначно задан интеллект, необходимый для использования предмета");
	assert(m_caseInfo.m_setName.empty() && "Неоднозначно задан набор предметов для объекта");
	m_useInfo.m_exp = ParseUtils::GetInt(str, "experience");
	m_useInfo.m_wisdom = ParseUtils::GetInt(str, "wisdom");
	m_caseInfo.m_respawn = ParseUtils::GetInt(str, "respawn");
	m_caseInfo.m_setName = ParseUtils::GetStr(str, "items");
	m_caseInfo.m_weight = ParseUtils::GetInt(str, "weight");
}

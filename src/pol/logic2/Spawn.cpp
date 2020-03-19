//
// методы spawn
//
#include "logicdefs.h"

#include "spawn.h"
#include "graphic.h"
#include "Thing.h"
#include "entity.h"
#include "hexgrid.h"
#include "aiutils.h"
#include "xlsreader.h"
#include "dirtylinks.h"
#include "thingfactory.h"
#include "entityfactory.h"
#include "GameObserver.h"
#include "ExperienceSaver.h"

#include <undercover/options/options.h>
#include <common/utils/vfilewpr.h>

#define LOGERR_LOG(l)	MLL_MAKE_LOG("spawnerr.log",l)

namespace SpawnDetail
{
}

using namespace SpawnDetail;

Spawner::spawn_ptr_t    Spawner::m_instance;
Spawner::spawn_obsmgr_t Spawner::m_observers;

typedef std::vector<std::string> str_vec_t;

typedef std::string exit_map_t[MAX_JOINTS];
typedef str_vec_t   entry_map_t[MAX_JOINTS];    
typedef pnt_vec_t   joint_pnts_t[MAX_JOINTS];

typedef std::map<std::string, pnt_vec_t> zone_map_t;

//=================================================================

void EntityBuilder::SendSpawnEvent(BaseEntity* entity)
{
	SpawnObserver::spawn_info_s info(entity);
	Spawner::GetInst()->Notify(SpawnObserver::ET_ENTITY_SPAWN, &info);
}

//=================================================================

namespace SpawnDetail
{

	const char* file_ext = ".txt";
	const char* spawn_xls_dir = "scripts/spawn/";
	const char* levels_xls_name = "scripts/levels.txt";
	const char* joints_xls_name = "scripts/levels.txt";
	const char* traders_xls_name = "scripts/logic/traders.txt";

	const char* first_human_column_name = "people";
	const char* first_trader_column_name = "traders";
	const char* first_vehicle_column_name = "vehicles";

	bool IsHumanHeader(const std::string& str)
	{
		return str == first_human_column_name;
	}

	bool IsTraderHeader(const std::string& str)
	{
		return str == first_trader_column_name;
	}

	bool IsVehicleHeader(const std::string& str)
	{
		return str == first_vehicle_column_name;
	}

	bool IsSpawnHeader(const std::string& str)
	{
		return IsHumanHeader(str) || IsTraderHeader(str) || IsVehicleHeader(str);
	}

	bool IsSuitablePhase(int phase, const std::string& str_)
	{
		// Уберем пробелы из строки
		std::string str = str_;
		str.erase(std::remove_if(str.begin(),str.end(),std::isspace)-str.begin());

		if(str.empty() && phase == 0xFFFFFFFF) return true;

		typedef boost::char_separator<char> Sep_t;
		typedef boost::tokenizer<Sep_t> Tok_t;
		Sep_t sep(".,;:");
		Tok_t tok(str,sep);

		for(Tok_t::iterator it = tok.begin(); it != tok.end(); ++it)
		{
			try
			{
				if(boost::lexical_cast<int>(*it) == phase) return true;
			}
			catch(boost::bad_lexical_cast &)
			{
			}
		}

		return false;
	}

	static std::istringstream& init(std::istringstream& istr, const std::string& str)
	{
		istr.str(str); istr.clear(); return istr;
	}

	void operator >> (std::istringstream& istr, player_type& type)
	{
		std::string str;
		istr >> str;

		if(str == "player" || str == "p")
			type = PT_PLAYER;
		else if(str == "enemy" || str == "e")
			type = PT_ENEMY;
		else if(str == "civilian" || str == "c")
			type = PT_CIVILIAN;
		else if(str == "none" || str == "n")
			type = PT_NONE;
		else
			throw CASUS("Spawner: неизвестный тип команды <" + str + "> !");
	}  

	void ScanGrid(zone_map_t& zones, joint_pnts_t& joints)
	{
		HexGrid::hg_slice hs = HexGrid::GetInst()->GetSlice();    

		HexGrid::const_cell_iterator citor = HexGrid::GetInst()->first_cell();                                
		HexGrid::const_prop_iterator hex_prop;

		while(citor != HexGrid::GetInst()->last_cell()){

			hex_prop = HexGrid::GetInst()->first_prop() + citor->GetProperty();

			if(hex_prop->m_zone.size())
				zones[hex_prop->m_zone].push_back(hs.off2pnt(citor - HexGrid::GetInst()->first_cell()));

			for(int i = 0; i < MAX_JOINTS && hex_prop->m_joints; i++){                
				if((hex_prop->m_joints & HexGrid::GetInst()->GetActiveJoints()) & (0x1 << i))
					joints[i].push_back(hs.off2pnt(citor - HexGrid::GetInst()->first_cell()));                
			}    

			++citor;
		}        
	}

	//
	// класс для разбора строки направления
	//
	class DirParser{
	public:

		DirParser(){}

		float Parse(const std::string& str) 
		{
			init(m_input, str);
			m_ranges.clear();

			range_s range;             
			float   value = 0;

			token_type token = GetToken(&value);

			//произведем разбор
			while(true){

				//если наткнулись на конец или на ошибку
				if(token == T_END || token == T_ERROR)
					break;

				//если это не число тогда выход - это ошибка
				if(token != T_NUMBER)
					break;

				range.m_length = 0;
				range.m_begin  = value;

				//проверим есть ли дальше тире
				token = GetToken(&value);

				if(token == T_DASH){

					token = GetToken(&value);

					//если это не число выход - это ошибка
					if(token != T_NUMBER)
						break;

					range.m_length = value - range.m_begin;

					//возьмем след. токен
					token = GetToken(&value);
				}

				m_ranges.push_back(range);
			}

			if(m_ranges.empty()) return 0;

			//выберем случайным образом нужный диапазон
			range = m_ranges[aiRangeRand(m_ranges.size())];

			//вернем случайное направление в диапазоне
			return range.m_begin + range.m_length*aiNormRand();
		}

	private:

		enum token_type{
			T_DASH,
			T_NUMBER,

			T_END,
			T_ERROR,
		};

		enum {
			OFFSET     = 180,
			MAX_DIGITS = 12,
			DIGIT_COST = 360/MAX_DIGITS,
		};

		token_type GetToken(float* ptr)
		{ 
			int ch;            

			while(m_input.good()){

				ch = m_input.peek();

				if(isspace(ch) || ch == ',' || ch == ';'){
					m_input.get();
					continue;
				}

				if(ch == '-'){
					m_input.get();
					return T_DASH;
				}

				if(isdigit(ch)){

					m_input >> ch;

					//нормировка
					if(ch > MAX_DIGITS) ch %= MAX_DIGITS;                    
					if(ch == 0) ch = MAX_DIGITS;                     

					//вычислим угол по значению часов
					*ptr = - TORAD(ch * DIGIT_COST + OFFSET);

					return T_NUMBER;
				}

				return T_ERROR;
			}

			return T_END;
		}

	private:

		struct range_s{
			float m_begin;
			float m_length;
		};

		typedef std::vector<range_s> range_vec_t;
		range_vec_t m_ranges;

		std::istringstream m_input;
	};

	//
	// класс для покладания предметов в существо
	//
	class EntityThingAcceptor : public ThingScriptParser::Acceptor{
	public:

		EntityThingAcceptor(EntityBuilder* builder) : 
		  m_builder(builder) {}

		  bool AcceptThing(BaseThing* thing)
		  {
			  if(m_builder->CanTake(m_entity, thing, m_pack))
			  {
				  m_builder->GiveThing(m_entity, thing, m_pack);
				  return true;
			  }

			  return false;
		  }

		  void AcceptOrgInfo(const rid_t rid) {}
		  void AcceptEntInfo(entity_type type, const rid_t& rid) {}
		  void AcceptThingInfo(thing_type type, const rid_t& rid) {}

		  void SetEntity(BaseEntity* entity)
		  {
			  m_entity = entity;
		  }

		  void SetPack(human_pack_type pack)
		  {
			  m_pack = pack;
		  }

	private:

		human_pack_type m_pack;
		BaseEntity*     m_entity;
		EntityBuilder*  m_builder;
	};

	//
	// класс порождающий предметы для торговца
	//
	class TraderThingFactory{
	public:

		TraderThingFactory(ThingScriptParser::Acceptor* acceptor) : 
		  m_acceptor(acceptor) {}

		  void GiveThings(const rid_t& rid)
		  {
			  std::string str, sys;

			  TxtFilePtr txt(traders_xls_name);

			  //найдем торговца
			  for(int col = 1; col < txt->SizeX(0); col++){            

				  txt->GetCell(0, col, str);

				  if(str == rid){
					  ReadAndSpawn(txt, col, IsAmmo, CreateAmmo);
					  ReadAndSpawn(txt, col, IsArmors, CreateArmor);
					  ReadAndSpawn(txt, col, IsWeapons, CreateWeapon);
					  ReadAndSpawn(txt, col, IsMedikits, CreateMedikit);
					  ReadAndSpawn(txt, col, IsImplants, CreateImplant);
					  ReadAndSpawn(txt, col, IsShields, CreateShield);
					  ReadAndSpawn(txt, col, IsCameras, CreateCamera);
					  ReadAndSpawn(txt, col, IsScanners, CreateScanner);
					  ReadAndSpawn(txt, col, IsGrenades, CreateGrenade);
					  ReadAndSpawn(txt, col, IsKeys, CreateKey);
					  return;
				  }
			  }

			  throw CASUS("TraderThingFactory: не найден список для <" + rid + ">!");
		  }

	private:

		template<class IsMyHeaderOp, class CreateThingOp>
			void ReadAndSpawn(TxtFilePtr& txt, int trader_column, IsMyHeaderOp is_my_hdr, CreateThingOp creator)
		{
			std::string str, sys;

			//найти нужный раздел
			int line = 0;
			for(; line < txt->SizeY(); line ++){
				txt->GetCell(line, 0, str);
				if(is_my_hdr(str)) break;
			}

			//раздать оружие
			for(line ++; line < txt->SizeY(); line ++){

				txt->GetCell(line, 0, sys);

				if(sys.empty() || IsHeader(sys))
					break;

				txt->GetCell(line, trader_column, str);

				int count = GetSpawnCount(str); 
				while(count--) m_acceptor->AcceptThing(creator(sys));
			}
		}

		static BaseThing* CreateWeapon(const rid_t& rid)
		{ 
			return ThingFactory::GetInst()->CreateWeapon(rid, rid_t(), 0);
		}

		static BaseThing* CreateGrenade(const rid_t& rid)
		{ 
			return ThingFactory::GetInst()->CreateGrenade(rid);
		}

		static BaseThing* CreateAmmo(const rid_t& rid)
		{ 
			return ThingFactory::GetInst()->CreateAmmo(rid, 0);
		}

		static BaseThing* CreateArmor(const rid_t& rid)
		{ 
			return ThingFactory::GetInst()->CreateArmor(rid);
		}

		static BaseThing* CreateImplant(const rid_t& rid)
		{ 
			return ThingFactory::GetInst()->CreateImplant(rid);
		}

		static BaseThing* CreateKey(const rid_t& rid)
		{ 
			return ThingFactory::GetInst()->CreateKey(rid);
		}

		static BaseThing* CreateMedikit(const rid_t& rid)
		{ 
			return ThingFactory::GetInst()->CreateMedikit(rid, 0);
		}

		static BaseThing* CreateShield(const rid_t& rid)
		{
			return ThingFactory::GetInst()->CreateShield(rid);
		}

		static BaseThing* CreateCamera(const rid_t& rid)
		{
			return ThingFactory::GetInst()->CreateCamera(rid);
		}

		static BaseThing* CreateScanner(const rid_t& rid)
		{
			return ThingFactory::GetInst()->CreateScanner(rid);
		}

		static bool IsWeapons(const std::string& str)
		{
			return str == "weapons";
		}

		static bool IsAmmo(const std::string& str)
		{
			return str == "ammunition";
		}

		static bool IsGrenades(const std::string& str)
		{
			return str == "grenades";
		}

		static bool IsArmors(const std::string& str)
		{
			return str == "armour";
		}

		static bool IsImplants(const std::string& str)
		{
			return str == "implant";
		}

		static bool IsKeys(const std::string& str)
		{
			return str == "keys";
		}

		static bool IsMedikits(const std::string& str)
		{
			return str == "medikit";
		}

		static bool IsScanners(const std::string& str)
		{
			return str == "scanner";
		}

		static bool IsCameras(const std::string& str)
		{
			return str == "camera";
		}

		static bool IsShields(const std::string& str)
		{
			return str == "shield";
		}

		bool IsHeader(const std::string& str)
		{
			return      IsWeapons(str) 
				||  IsAmmo(str) 
				||  IsArmors(str)
				||  IsShields(str)
				||  IsCameras(str)
				||  IsGrenades(str)
				||  IsImplants(str)
				||  IsMedikits(str)
				||  IsScanners(str)
				||  IsKeys(str);
		}

		int GetSpawnCount(const std::string& str)
		{
			if(str == "0") return 0;

			char  ch;
			float p;

			std::istringstream istr(str);

			istr >> p >> ch >> p >> ch >> p >> ch;

			//выпадает ли вероятность?
			if(aiNormRand() > p) return 0;

			int base = 1, range = 0;

			if(istr.good())
			{
				istr >> base >> ch;

				if(istr.good()) istr >> range;

				range -= base;

				if(range < 0) range = 0;
			}

			return range ? base + aiRangeRand(range) : base;
		}

	private:

		ThingScriptParser::Acceptor* m_acceptor;
	};

	//
	// направить набор данных на парсинг
	//
	class ItemSetAcceptor : public ItemSetParser::Acceptor
	{
	public:

		ItemSetAcceptor(ThingScriptParser* parser) : m_parser(parser) {}

		void Accept(const rid_t& rid)
		{
			int line;
			std::string script;

			if(!AIUtils::GetItemSetStr(rid, &script, &line))
			{
				LOGERR_LOG("GetItemSetStr: нет набора предметов [" << rid << "]!\n");
				throw NoItemSetError("GetItemSetStr: нет набора предметов <" + rid +">!");
			}

			if(m_parser && !m_parser->Parse(script))
			{ 
				std::ostringstream ostr;
				ostr << "ThingScriptParser: ошибка в наборе предметов [" <<  rid << "]" << std::endl; 
				ostr << "строка: " << line + 1 << std::endl;
				ostr << "символ: " << m_parser->GetLastPos() << std::endl;
				LOGERR_LOG(ostr.str() << "\n");
				throw BadItemSetError(ostr.str());
			}
		}

	private:

		ThingScriptParser* m_parser;
	};
}

//=================================================================

namespace SpawnDetail{

	//
	// прочитать таблицу точек выхода
	//
	class LevelsXlsReader{
	public:

		static bool Read(const std::string& level, int* episode, exit_map_t* exit, entry_map_t* entry)
		{
			TxtFilePtr txt(levels_xls_name);

			int line;
			std::string str;
			std::istringstream istr;

			txt.ReadColumns(m_columns, m_columns + LCT_MAX_COLUMNS);

			//найдем строку
			for(line = 0; line < txt->SizeY(); ++line){

				txt->GetCell(line, m_columns[LCT_LEVEL].m_index, str);

				if(str == level){

					txt->GetCell(line, m_columns[LCT_EPISODE].m_index, str);
					init(istr, str) >> *episode;

					txt->GetCell(line, m_columns[LCT_JOINT1].m_index, (*exit)[0]);
					txt->GetCell(line, m_columns[LCT_JOINT2].m_index, (*exit)[1]);
					txt->GetCell(line, m_columns[LCT_JOINT3].m_index, (*exit)[2]);
					txt->GetCell(line, m_columns[LCT_JOINT4].m_index, (*exit)[3]);
					txt->GetCell(line, m_columns[LCT_JOINT5].m_index, (*exit)[4]);
					txt->GetCell(line, m_columns[LCT_JOINT6].m_index, (*exit)[5]);
					txt->GetCell(line, m_columns[LCT_JOINT7].m_index, (*exit)[6]);
					txt->GetCell(line, m_columns[LCT_JOINT8].m_index, (*exit)[7]);

					txt->GetCell(line, m_columns[LCT_ENTRY1].m_index, str);
					ParseLevelEntry(str, (*entry)[0]);

					txt->GetCell(line, m_columns[LCT_ENTRY2].m_index, str);
					ParseLevelEntry(str, (*entry)[1]);

					txt->GetCell(line, m_columns[LCT_ENTRY3].m_index, str);
					ParseLevelEntry(str, (*entry)[2]);

					txt->GetCell(line, m_columns[LCT_ENTRY4].m_index, str);
					ParseLevelEntry(str, (*entry)[3]);

					txt->GetCell(line, m_columns[LCT_ENTRY5].m_index, str);
					ParseLevelEntry(str, (*entry)[4]);

					txt->GetCell(line, m_columns[LCT_ENTRY6].m_index, str);
					ParseLevelEntry(str, (*entry)[5]);

					txt->GetCell(line, m_columns[LCT_ENTRY7].m_index, str);
					ParseLevelEntry(str, (*entry)[6]);

					txt->GetCell(line, m_columns[LCT_ENTRY8].m_index, str);
					ParseLevelEntry(str, (*entry)[7]);

					return true;
				}         
			}

			return false;
		}

	private:

		static void ParseLevelEntry(const std::string str, str_vec_t& strs)
		{
			std::istringstream istr(str);

			while(istr.good()){
				strs.push_back(std::string());
				istr >> strs.back();
			}
		}

	private:

		enum levels_column_type{
			LCT_LEVEL,
			LCT_EPISODE,

			LCT_JOINT1,
			LCT_JOINT2,
			LCT_JOINT3,
			LCT_JOINT4,
			LCT_JOINT5,
			LCT_JOINT6,
			LCT_JOINT7,
			LCT_JOINT8,

			LCT_ENTRY1,
			LCT_ENTRY2,
			LCT_ENTRY3,
			LCT_ENTRY4,
			LCT_ENTRY5,
			LCT_ENTRY6,
			LCT_ENTRY7,
			LCT_ENTRY8,

			LCT_MAX_COLUMNS,
		};

		static column m_columns[LCT_MAX_COLUMNS];
	};

	column LevelsXlsReader::m_columns[LCT_MAX_COLUMNS] =
	{
		column("level",  LCT_LEVEL),
			column("episode",LCT_EPISODE),

			column("exit_1", LCT_JOINT1),
			column("exit_2", LCT_JOINT2),
			column("exit_3", LCT_JOINT3),
			column("exit_4", LCT_JOINT4),
			column("exit_5", LCT_JOINT5),
			column("exit_6", LCT_JOINT6),
			column("exit_7", LCT_JOINT7),
			column("exit_8", LCT_JOINT8),

			column("entry_1", LCT_ENTRY1),
			column("entry_2", LCT_ENTRY2),
			column("entry_3", LCT_ENTRY3),
			column("entry_4", LCT_ENTRY4),
			column("entry_5", LCT_ENTRY5),
			column("entry_6", LCT_ENTRY6),
			column("entry_7", LCT_ENTRY7),
			column("entry_8", LCT_ENTRY8),
	};
}

//=================================================================

namespace SpawnDetail{

	//
	// прочесть кол-во сущетв для расст.
	//
	class EntityCountReader
	{
	public:
		EntityCountReader(const std::string& file_name, int phase)
		:	m_file_name(file_name),
			m_phase(phase)
		{
		}

		template<class IsMyHdrOp>
		int Count(IsMyHdrOp is_my_hdr)
		{
			TxtFilePtr txt(m_file_name);

			txt.ReadColumns(m_columns, m_columns + CCT_MAX_COLUMNS);

			std::string str;                
			int k = 0;
			for(; k < txt->SizeY(); k++)
			{            
				txt->GetCell(k, 0, str);
				if(is_my_hdr(str)) break;
			}

			int count = 0, quantity = 0;
			std::istringstream istr;

			for(k++; k < txt->SizeY(); k++)
			{
				txt->GetCell(k, 0, str);
				if(IsSpawnHeader(str)) return count;

				txt->GetCell(k, m_columns[CCT_PHASE].m_index, str);
				if(!IsSuitablePhase(m_phase, str)) continue;

				txt->GetCell(k, m_columns[CCT_QUANTITY].m_index, str); 
				init(istr, str) >> quantity;

				count += quantity;
			} 

			return 0;
		}

	private:

		int         m_phase;
		std::string m_file_name;

		enum column_type{
			CCT_PHASE,
			CCT_QUANTITY,

			CCT_MAX_COLUMNS,
		};

		static column m_columns[CCT_MAX_COLUMNS];
	};

	column EntityCountReader::m_columns[CCT_MAX_COLUMNS]= 
	{
		column("phase",     CCT_PHASE),
			column("quantity",  CCT_QUANTITY),
	};
}

//=================================================================

namespace SpawnDetail
{
	//
	// прочесть таблицу 
	//
	class HumanXlsSpawner
	{
	public:
		HumanXlsSpawner(const std::string& xls, int phase, const zone_map_t& zones)
		:	m_xls_name(xls),
			m_phase(phase),
			m_zones(zones)
		{
		}

		void Spawn(EntityBuilder* builder, Spawner::RuleSet* rules)
		{
			TxtFilePtr txt(m_xls_name);

			std::string str;
			std::istringstream istr;
			bool fheader_found = false;
			int dont_spawn_if_no_space = -1;

			//читаем заголовок
			int line = 0;
			for(; line < txt->SizeY(); line++)
			{
				txt->GetCell(line, 0, str);

				if(IsHumanHeader(str))
				{
					fheader_found = true;
					txt.ReadColumns(m_columns, m_columns + HCT_MAX_COLUMNS, line);
					for(int ii = 0; ii != txt->SizeX(line); ++ii)
					{
						std::string cell;
						txt->GetCell(line,ii,cell);
						if(cell == "dont.spawn.if.no_space")
						{
							dont_spawn_if_no_space = ii;
							break;
						}
					}

					break;
				}
			}

			if(!fheader_found) throw CASUS("HumanXlsSpawner: не найдены заголовки для расст. людей!");

			ipnt2_t   pos; 
			int       quantity = 0;

			DirParser           dir_parser;
			EntityThingAcceptor thing_acceptor(builder);
			ThingScriptParser   thing_parser(&thing_acceptor, ThingFactory::GetInst());
			ItemSetAcceptor     item_set_acceptor(&thing_parser);
			ItemSetParser       item_set_parser(&item_set_acceptor); 

			player_type team = PT_NONE;
			SpawnTag    tag(ET_HUMAN);

			//читаем таблицу расстановки
			for(line ++; line < txt->SizeY(); line++)
			{
				txt->GetCell(line, 0, str);

				//если пошла другая расстановка выход
				if(IsSpawnHeader(str)) return;

				//читаем информацию о фазе
				txt->GetCell(line, m_columns[HCT_PHASE].m_index, str);
				if(!IsSuitablePhase(m_phase, str)) continue;

				//читаем сист. имя 
				txt->GetCell(line, m_columns[HCT_SYSNAME].m_index, str);
				tag.SetSysName(str);

				//читаем label
				txt->GetCell(line, m_columns[HCT_LABEL].m_index, str);
				tag.SetAIModel(str);

				//читаем зону
				txt->GetCell(line, m_columns[HCT_ZONE].m_index, str);
				tag.SetSpawnZone(str);

				//читаем информацию о кол-ве расстановки
				txt->GetCell(line, m_columns[HCT_QUANTITY].m_index, str);
				init(istr, str) >> quantity;

				//если есть правило применить его
				if(Spawner::Rule* rule = rules->FindRule(tag))
				{
					quantity = rule->CalcCount(tag, quantity);                
				}

				//ставим столько сколько нужно
				while(quantity--)
				{
					zone_map_t::const_iterator itor = m_zones.find(tag.GetSpawnZone());

					if(itor == m_zones.end())
					{
						if(dont_spawn_if_no_space != -1)
						{
							std::string dsins;
							txt->GetCellOrEmptyString(line,dont_spawn_if_no_space,dsins);
							std::transform(dsins.begin(),dsins.end(),dsins.begin(),std::tolower);
							if(dsins == "yes")
							{
								continue;
							}
						}

						LOGERR_LOG("Spawner: на уровне нет зоны [" << tag.GetSpawnZone() << "]!");
						continue;
					}

					if(!builder->GenerateSuitablePos(itor->second, &pos, VS_UNKNOWN))
					{
						if(dont_spawn_if_no_space != -1)
						{
							std::string dsins;
							txt->GetCellOrEmptyString(line,dont_spawn_if_no_space,dsins);
							std::transform(dsins.begin(),dsins.end(),dsins.begin(),std::tolower);
							if(dsins == "yes")
							{
								continue;
							}
						}

						LOGERR_LOG("Spawner: нет места в зоне расстановки [" << tag.GetSpawnZone() << "]!");
						continue;
					}

					BaseEntity* human = builder->CreateHuman(tag.GetSysName());

					//читаем команду
					txt->GetCell(line, m_columns[HCT_TEAM].m_index, str);
					init(istr, str) >> team;
					builder->SetPlayer(human, team);      

					//установить AI
					builder->SetAIModel(human, tag.GetAIModel());

					//установить зону высадки
					builder->SetSpawnZone(human, tag.GetSpawnZone());

					int size = itor->second.size();

					//читаем направление и ставим человека
					txt->GetCell(line, m_columns[HCT_DIRECTION].m_index, str);
					builder->LinkEntity(human, pos, dir_parser.Parse(str)); 

					thing_acceptor.SetEntity(human);

					struct Helper
					{
						human_pack_type m_packType;
						int m_columnIndex;
					};

					Helper helper[] = {{HPK_HEAD,HCT_HEAD}, {HPK_BODY,HCT_BODY}, {HPK_HANDS,HCT_HANDS},
									   {HPK_LKNEE,HCT_LKNEE}, {HPK_RKNEE,HCT_RKNEE}, {HPK_BACKPACK,HCT_BACKPACK},
									   {HPK_IMPLANTS,HCT_IMPLANTS}};

					for(int ii = 0; ii != sizeof(helper)/sizeof(helper[0]); ++ii)
					{
						thing_acceptor.SetPack(helper[ii].m_packType);
						txt->GetCell(line, m_columns[helper[ii].m_columnIndex].m_index, str);
						if(!item_set_parser.Parse(str)) PrintErr(m_xls_name, helper[ii].m_columnIndex, line, item_set_parser.GetLastPos());
					}

					if(human->Cast2Human()->GetInfo()->IsHero() || human->Cast2Human()->GetInfo()->IsAntihero())
					{
						std::string rid = human->Cast2Human()->GetInfo()->GetRID();
						if(PoL::ExperienceSaver::inst()->hasHeroInformation(rid))
						{
							HumanContext *ctx = human->Cast2Human()->GetEntityContext();
							const PoL::ExperienceInfo &info = PoL::ExperienceSaver::inst()->getExperienceInfo(rid);

							for(int ii = 0; ii != sizeof(helper)/sizeof(helper[0])-1; ++ii)
							{
								int count = 0;

								for(HumanContext::iterator it = ctx->begin(helper[ii].m_packType); it != ctx->end(); ++it)
								{
									++count;
								}

								if(count == 0)
								{
									for(int iii = 0; iii != info.m_thingTags.size(); ++iii)
									{
										if(info.m_thingTags[iii].m_index != helper[ii].m_packType) continue;
										SavSlotStr sss(info.m_thingTags[iii].m_data);
										BaseThing *thng = 0;
										DynUtils::LoadObj(sss,thng);
										thng->MakeSaveLoad(sss);

										ctx->InsertThing(helper[ii].m_packType,thng);
									}
								}
							}

							std::vector<BaseThing *> things;
							std::vector<std::string> toinsert;

							for(int i = 0; i != info.m_implants.size(); ++i)
							{
								bool needinsert = true;
								HumanContext::iterator ii = ctx->begin(HPK_IMPLANTS);
								for(; ii != ctx->end(); ++ii)
								{
									BaseThing *implant = &*ii;
									if(implant->GetInfo()->GetRID() == info.m_implants[i] &&
										std::find(things.begin(),things.end(),implant) == things.end())
									{
										things.push_back(implant);
										needinsert = false;
										break;
									}
								}

								if(needinsert)
								{
									toinsert.push_back(info.m_implants[i]);
								}
							}

							for(int i = 0; i != toinsert.size(); ++i)
							{
								BaseThing *thing = ThingFactory::GetInst()->CreateImplant(toinsert[i]);
								if(!thing_acceptor.AcceptThing(thing)) delete thing;
							}
						}
					}

					human->Cast2Human()->GetGraph()->Reset();

					HumanContext *ctx = human->Cast2Human()->GetEntityContext();
					ctx->GetTraits()->SetMovepnts(ctx->GetLimits()->GetMovepnts());
					builder->SendSpawnEvent(human);
				}                
			}
		}

	private:

		void PrintErr(const std::string& file, int col, int line, int pos)
		{
			std::ostringstream ostr;

			ostr << "HumanSpawner: ошибка описания набора предметов!!!" << std::endl;
			ostr << "файл:   " << file << std::endl;
			ostr << "строка: " << line + 1 << std::endl;
			ostr << "ячейка: " << m_columns[col].m_name << std::endl;
			ostr << "символ: " << pos << std::endl;

			throw CASUS(ostr.str().c_str());            
		}

	private:

		int         m_phase;
		const zone_map_t& m_zones;
		std::string m_xls_name;

		enum column_type
		{
			HCT_SYSNAME,
			HCT_PHASE,
			HCT_ZONE,
			HCT_DIRECTION,
			HCT_QUANTITY,
			HCT_LABEL,
			HCT_TEAM,
			HCT_HANDS,
			HCT_BACKPACK,
			HCT_BODY,
			HCT_IMPLANTS,
			HCT_HEAD,
			HCT_LKNEE,
			HCT_RKNEE,

			HCT_MAX_COLUMNS,            
		};

		static column m_columns[HCT_MAX_COLUMNS];
	};

	column HumanXlsSpawner::m_columns[HCT_MAX_COLUMNS]= 
	{      
		column(first_human_column_name, HCT_SYSNAME),
			column("phase",     HCT_PHASE),
			column("zone",      HCT_ZONE),
			column("direction", HCT_DIRECTION),
			column("quantity",  HCT_QUANTITY),
			column("label",     HCT_LABEL),
			column("team",      HCT_TEAM),
			column("hands",     HCT_HANDS),
			column("backpack",  HCT_BACKPACK),
			column("body",      HCT_BODY),
			column("implants",  HCT_IMPLANTS),
			column("head",      HCT_HEAD),
			column("knee_left", HCT_LKNEE),
			column("knee_right",HCT_RKNEE),
	};
}

//=================================================================

namespace SpawnDetail{

	//
	// прочесть таблицу расстановки для техники
	//
	class VehicleXlsSpawner
	{
	public:
		VehicleXlsSpawner(const std::string& xls, int phase, const zone_map_t& zones) :
		  m_xls_name(xls), m_phase(phase), m_zones(zones) {}

		  void Spawn(EntityBuilder* builder, Spawner::RuleSet* rules)
		  {
			  TxtFilePtr txt(m_xls_name);

			  std::string str;
			  std::istringstream istr;
			  bool fheader_found = false;
			  int dont_spawn_if_no_space = -1;

			  //читаем заголовок
			  int line = 0;
			  for(; line < txt->SizeY(); line++)
			  {
				  txt->GetCell(line, 0, str);

				  if(IsVehicleHeader(str))
				  {
					  fheader_found = true;
					  txt.ReadColumns(m_columns, m_columns + VCT_MAX_COLUMNS, line);            
					  for(int ii = 0; ii != txt->SizeX(line); ++ii)
					  {
						  std::string cell;
						  txt->GetCell(line,ii,cell);
						  if(cell == "dont.spawn.if.no_space")
						  {
							  dont_spawn_if_no_space = ii;
							  break;
						  }
					  }

					  break;
				  }
			  }

			  if(!fheader_found) throw CASUS("HumanXlsSpawner: не найдены заголовки для расст. техники!");

			  ipnt2_t   pos; 
			  int       quantity = 0;
			  DirParser dir_parser;

			  player_type team = PT_NONE;
			  SpawnTag    tag(ET_VEHICLE);

			  //читаем таблицу расстановки
			  for(line ++; line < txt->SizeY(); line++)
			  {
				  txt->GetCell(line, 0, str);

				  //если пошла другая расстановка выход
				  if(IsSpawnHeader(str)) return;

				  //читаем информацию о фазе
				  txt->GetCell(line, m_columns[VCT_PHASE].m_index, str);
				  if(!IsSuitablePhase(m_phase, str)) continue;

				  //читаем сист. имя 
				  txt->GetCell(line, m_columns[VCT_SYSNAME].m_index, str);
				  tag.SetSysName(str);

				  //читаем label
				  txt->GetCell(line, m_columns[VCT_LABEL].m_index, str);
				  tag.SetAIModel(str);

				  //читаем зону
				  txt->GetCell(line, m_columns[VCT_ZONE].m_index, str);
				  tag.SetSpawnZone(str);

				  //читаем информацию о кол-ве расстановки
				  txt->GetCell(line, m_columns[VCT_QUANTITY].m_index, str);
				  init(istr, str) >> quantity;

				  //если есть правило применим его
				  if(Spawner::Rule* rule = rules->FindRule(tag))
					  quantity = rule->CalcCount(tag, quantity);

				  //ставим столько сколько нужно
				  while(quantity --)
				  {
					  zone_map_t::const_iterator itor = m_zones.find(tag.GetSpawnZone());

					  if(itor == m_zones.end())
					  {
						  if(dont_spawn_if_no_space != -1)
						  {
							std::string dsins;
							txt->GetCellOrEmptyString(line,dont_spawn_if_no_space,dsins);
							std::transform(dsins.begin(),dsins.end(),dsins.begin(),std::tolower);
							if(dsins == "yes")
							{
								continue;
							}
						  }

						  LOGERR_LOG("Spawner: на уровне нет зоны [" << tag.GetSpawnZone() << "]\n");
						  continue;
					  }

					  vehicle_size vsize = VS_UNKNOWN;

					  {
						VehicleInfo *vinfo = static_cast<VehicleInfo*>(EntityInfoArchive::GetInst()->CreateRef(ET_VEHICLE, tag.GetSysName()));
						vsize = vinfo->GetVehicleSize();
						EntityInfoArchive::GetInst()->DeleteRef(vinfo);
					  }

					  if(!builder->GenerateSuitablePos(itor->second, &pos, vsize))
					  {
						  if(dont_spawn_if_no_space != -1)
						  {
							std::string dsins;
							txt->GetCellOrEmptyString(line,dont_spawn_if_no_space,dsins);
							std::transform(dsins.begin(),dsins.end(),dsins.begin(),std::tolower);
							if(dsins == "yes")
							{
								continue;
							}
						  }

						  LOGERR_LOG("Spawner: нет места в зоне расстановки [" + tag.GetSpawnZone() + "]!");
						  continue;
					  }


					  BaseEntity* vehicle = builder->CreateVehicle(tag.GetSysName());

					  //читаем команду
					  txt->GetCell(line, m_columns[VCT_TEAM].m_index, str);
					  init(istr, str) >> team;
					  builder->SetPlayer(vehicle, team);                    

					  //установить модель AI
					  builder->SetAIModel(vehicle, tag.GetAIModel());                     

					  //установить зону высадки
					  builder->SetSpawnZone(vehicle, tag.GetSpawnZone());

					  //читаем направление и ставим
					  txt->GetCell(line, m_columns[VCT_DIRECTION].m_index, str);
					  builder->LinkEntity(vehicle, pos, dir_parser.Parse(str));                  

					  builder->SendSpawnEvent(vehicle);
				  }                
			  }
		  }

	private:

		int         m_phase;
		const zone_map_t& m_zones;
		std::string m_xls_name;                

		enum column_type{
			VCT_SYSNAME,
			VCT_PHASE,
			VCT_ZONE,
			VCT_DIRECTION,
			VCT_QUANTITY,
			VCT_LABEL,
			VCT_TEAM,

			VCT_MAX_COLUMNS,        													
		};

		static column m_columns[VCT_MAX_COLUMNS];
	};

	column VehicleXlsSpawner::m_columns[VCT_MAX_COLUMNS] =
	{
		column(first_vehicle_column_name, VCT_SYSNAME),
			column("phase",     VCT_PHASE),
			column("zone",      VCT_ZONE),
			column("direction", VCT_DIRECTION),
			column("quantity",  VCT_QUANTITY),
			column("label",     VCT_LABEL),
			column("team",      VCT_TEAM),
	};
}

//=================================================================

namespace SpawnDetail{

	//
	// прочесть таблицу расстановки для техники
	//
	class TraderXlsSpawner{
	public:

		TraderXlsSpawner(const std::string& xls, int phase, const zone_map_t& zones) :
		  m_xls_name(xls), m_phase(phase), m_zones(zones) {}

		  void Spawn(EntityBuilder* builder, Spawner::RuleSet* rules)
		  {
			  TxtFilePtr txt(m_xls_name);

			  std::string str;
			  std::istringstream istr;
			  bool fheader_found = false;
			  int dont_spawn_if_no_space = -1;

			  //читаем заголовок
			  int line = 0;
			  for(; line < txt->SizeY(); line++)
			  {
				  txt->GetCell(line, 0, str);

				  if(IsTraderHeader(str))
				  {
					  fheader_found = true;
					  txt.ReadColumns(m_columns, m_columns + TCT_MAX_COLUMNS, line);            
					  for(int ii = 0; ii != txt->SizeX(line); ++ii)
					  {
						  std::string cell;
						  txt->GetCell(line,ii,cell);
						  if(cell == "dont.spawn.if.no_space")
						  {
							  dont_spawn_if_no_space = ii;
							  break;
						  }
					  }
					  break;
				  }
			  }

			  if(!fheader_found) throw CASUS("TraderXlsSpawner: не найдены заголовки для расст. техники!");

			  ipnt2_t   pos; 
			  int       quantity = 0;

			  DirParser          dir_parser;
			  EntityThingAcceptor thing_acceptor(builder);;
			  TraderThingFactory  thing_factory(&thing_acceptor);

			  SpawnTag tag(ET_TRADER);

			  //читаем таблицу расстановки
			  for(line ++; line < txt->SizeY(); line++){

				  txt->GetCell(line, 0, str);

				  //если пошла другая расстановка выход
				  if(IsSpawnHeader(str)) return;

				  //читаем информацию о фазе
				  txt->GetCell(line, m_columns[TCT_PHASE].m_index, str);
				  if(!IsSuitablePhase(m_phase, str)) continue;

				  //читаем сист. имя 
				  txt->GetCell(line, m_columns[TCT_SYSNAME].m_index, str);
				  tag.SetSysName(str);

				  //читаем label
				  txt->GetCell(line, m_columns[TCT_LABEL].m_index, str);
				  tag.SetAIModel(str);

				  //читаем зону
				  txt->GetCell(line, m_columns[TCT_ZONE].m_index, str);
				  tag.SetSpawnZone(str);

				  //читаем информацию о кол-ве расстановки
				  txt->GetCell(line, m_columns[TCT_QUANTITY].m_index, str);
				  init(istr, str) >> quantity;

				  //если есть правило применим его
				  if(Spawner::Rule* rule = rules->FindRule(tag))
					  quantity = rule->CalcCount(tag, quantity);

				  //ставим столько сколько нужно
				  while(quantity --)
				  {
					  zone_map_t::const_iterator itor = m_zones.find(tag.GetSpawnZone());

					  if(itor == m_zones.end())
					  {
						  if(dont_spawn_if_no_space != -1)
						  {
							std::string dsins;
							txt->GetCellOrEmptyString(line,dont_spawn_if_no_space,dsins);
							std::transform(dsins.begin(),dsins.end(),dsins.begin(),std::tolower);
							if(dsins == "yes")
							{
								continue;
							}
						  }
						  LOGERR_LOG("Spawner: на уровне нет зоны [" << tag.GetSpawnZone() << "]!\n");
						  continue;
					  }

					  if(!builder->GenerateSuitablePos(itor->second, &pos, VS_UNKNOWN))
					  {
						  if(dont_spawn_if_no_space != -1)
						  {
							std::string dsins;
							txt->GetCellOrEmptyString(line,dont_spawn_if_no_space,dsins);
							std::transform(dsins.begin(),dsins.end(),dsins.begin(),std::tolower);
							if(dsins == "yes")
							{
								continue;
							}
						  }

						  LOGERR_LOG("Spawner: нет места в зоне расстановки [" + tag.GetSpawnZone() + "]!\n");
						  continue;
					  }

					  BaseEntity* trader = builder->CreateTrader(tag.GetSysName());

					  thing_acceptor.SetEntity(trader);
					  thing_acceptor.SetPack(HPK_BACKPACK);

					  //дать торговцу предметы
					  thing_factory.GiveThings(tag.GetSysName());

					  //читаем команду
					  builder->SetPlayer(trader, PT_CIVILIAN);                    

					  //установить модель AI
					  builder->SetAIModel(trader, tag.GetAIModel());

					  //читаем зону
					  builder->SetSpawnZone(trader, tag.GetSpawnZone());

					  //читаем направление и ставим человека
					  txt->GetCell(line, m_columns[TCT_DIRECTION].m_index, str);
					  builder->LinkEntity(trader, pos, dir_parser.Parse(str));

					  builder->SendSpawnEvent(trader);
				  }                
			  }
		  }

	private:

		int         m_phase;
		const zone_map_t& m_zones;
		std::string m_xls_name;                

		enum column_type{
			TCT_SYSNAME,
			TCT_PHASE,
			TCT_ZONE,
			TCT_DIRECTION,
			TCT_QUANTITY,
			TCT_LABEL,

			TCT_MAX_COLUMNS,
		};

		static column m_columns[TCT_MAX_COLUMNS];
	};

	column TraderXlsSpawner::m_columns[TCT_MAX_COLUMNS]= 
	{
		column(first_trader_column_name, TCT_SYSNAME),
			column("phase",     TCT_PHASE),
			column("zone",      TCT_ZONE),
			column("direction", TCT_DIRECTION),
			column("quantity",  TCT_QUANTITY),
			column("label",     TCT_LABEL),
	};
}

//=================================================================

namespace SpawnDetail{

	class UsualSpawner : public Spawner{
	public:


		UsualSpawner()
		{
			m_previous = m_saved_previous_level_name;
			m_saved_previous_level_name.clear();
			//m_difficulty = D_HARD;

		}

		~UsualSpawner() { }

		void MakeSaveLoad(SavSlot& st)
		{
			if(st.IsSaving()){

				st << m_episode;
				st << m_phase;
				st << m_previous;

				for(int k = 0; k < MAX_JOINTS; st << m_exit[k++]);

				st<<(unsigned)m_difficulty;

				st<<m_lockedlevels.size();
				for(unsigned int i=0;i<m_lockedlevels.size();i++)
					st<<m_lockedlevels[i];

			} else {

				m_difficulty = (DIFFICULTY)Options::GetInt("game.type");

				st >> m_episode;
				st >> m_phase;
				st >> m_previous;

				for(int k = 0; k < MAX_JOINTS; st >> m_exit[k++]);
				try
				{
					unsigned t;
					st>>t; m_difficulty = (DIFFICULTY)t;
					st>>t; m_lockedlevels.resize(t);
					for(unsigned int i=0; i<t; ++i)
						st>>m_lockedlevels[i];
				}
				catch(CasusImprovisus &)
				{

				};
			}
		}

		int GetPhase() const
		{
			return m_phase;
		}

		int GetEpisode() const
		{
			return m_episode;
		}

		void SetPhase(int val)
		{
			m_phase = val;
			Notify(SpawnObserver::ET_PHASE_CHANGE);
		}
		//узнать/установить текущую сложность
		int GetDifficulty() const {return m_difficulty;};
		void SetDifficulty(int new_dif) {m_difficulty = new_dif;m_lockedlevels.clear();};

		void EnableExit(const std::string& level, bool fenable)
		{
			unsigned mask = 0;

			m_lockedlevels.erase(
				std::remove(m_lockedlevels.begin(),m_lockedlevels.end(),level),
				m_lockedlevels.end());

			if(!fenable) m_lockedlevels.push_back(level);
			//составим маску выходов на этот уровень
			for(int k = 0; k < MAX_JOINTS; k++){
				if(m_exit[k] == level) mask |= 1 << k;
			}

			unsigned old_mask = HexGrid::GetInst()->GetActiveJoints();

			//разрешим или запретим соотв выходы
			HexGrid::GetInst()->SetActiveJoints(fenable ? old_mask | mask : old_mask & ~mask);
		}

		bool CanExit(unsigned test, std::string* level = 0)
		{
			//по карте переходов определим уровень
			for(int k = 0; k < MAX_JOINTS; k++){

				unsigned joint = 1 << k;

				//      есть ли выход во флагах и активен ли он
				//  и   есть ли название уровня для выхода 
				if(     ((test & joint) && (HexGrid::GetInst()->GetActiveJoints() & joint))
					&&  (m_exit[k].size() && m_exit[k] != "none")){

						if(std::count(m_lockedlevels.begin(),m_lockedlevels.end(),m_exit[k]))
							return false;

						if(level) *level = m_exit[k];
						return true;
					}
			}

			return false;
		}

		void ExitLevel(const std::string& new_lvl)
		{           
			m_saved_previous_level_name = DirtyLinks::GetLevelSysName();
			Notify(SpawnObserver::ET_EXIT_LEVEL,(SpawnObserver::info_t)&new_lvl);            
			DirtyLinks::LoadLevel(new_lvl);         
		}

		void ExitLevel(const std::string& new_lvl, const std::string& old_lvl)
		{           
			m_saved_previous_level_name = old_lvl;
			DirtyLinks::LoadLevel(new_lvl);         
		}

		void Spawn(EntityBuilder* builder, RuleSet* rules)
		{
			std::string file_name = spawn_xls_dir + DirtyLinks::GetLevelRawSysName() + file_ext;

			//проверим есть ли расстановка для этого уровня на диске
			PoL::VFileWpr vfile(file_name);
			if(!vfile.get() || !vfile.get()->Size()) return;

			zone_map_t   zones;
			joint_pnts_t joints;
			entry_map_t  entry; 
			int          episode = 0;

			//прочтем зоны выхода
			bool fhave_joints = LevelsXlsReader::Read(DirtyLinks::GetLevelRawSysName(), &episode, &m_exit, &entry);

			//сканирование сетки
			ScanGrid(zones, joints);

			//покажем зоны выхода
			AIUtils::CalcAndShowActiveExits();

			//ищем вход на уровень
			SpawnObserver::entry_info_s entry_info(builder);
			if(fhave_joints && m_previous.size())
			{
				for(int k = 0; k < MAX_JOINTS; k++)
				{
					for(size_t j = 0; j < entry[k].size(); j++)
					{
						if(entry[k][j] != m_previous)
							continue;

						std::copy(joints[k].begin(), joints[k].end(), std::back_inserter(entry_info.m_entry));
						break;
					}
				}

				if(entry_info.m_entry.empty())
				{
					std::ostringstream ostr;

					ostr << "UsualSpawner: не удалось найти вход на уровень <" << DirtyLinks::GetLevelSysName();
					ostr << "> с уровня <" << m_previous << ">!";

					throw CASUS(ostr.str());
				}
			}

			entry_info.m_fnew_episode = m_episode != episode;

			if(     !fhave_joints
				||  entry_info.m_entry.empty()
				||  entry_info.m_fnew_episode)
				m_phase = 0;                

			m_episode = episode;

			EntityCountReader counter(file_name, m_phase);
			SpawnObserver::prepare_info_s prepare_info( counter.Count(IsHumanHeader),
				counter.Count(IsTraderHeader),
				counter.Count(IsVehicleHeader));
			//подготовиться  переходу
			Notify(SpawnObserver::ET_PREPARE_SPAWN, &prepare_info);
			//уведомление о входе на уровень
			Notify(SpawnObserver::ET_ENTRY_LEVEL, &entry_info);

			DoSpawn(builder,rules,zones,m_phase);

			Notify(SpawnObserver::ET_FINISH_SPAWN);
			Notify(SpawnObserver::ET_POL_ENTRY_LEVEL, &entry_info);

			GameEvMessenger::GetInst()->Notify(GameObserver::EV_UPDATE_UNLIMITED_VISIBILITY);
		}

		void PolSpawn(EntityBuilder *builder, RuleSet *rules)
		{
			std::string file_name = spawn_xls_dir + DirtyLinks::GetLevelRawSysName() + file_ext;

			//проверим есть ли расстановка для этого уровня на диске
			PoL::VFileWpr vfile(file_name);
			if(!vfile.get() || !vfile.get()->Size()) return;

			zone_map_t   zones;
			joint_pnts_t joints;
			entry_map_t  entry; 
			int          episode = 0;

			//прочтем зоны выхода
			bool fhave_joints = LevelsXlsReader::Read(DirtyLinks::GetLevelRawSysName(), &episode, &m_exit, &entry);

			//сканирование сетки
			ScanGrid(zones, joints);

			EntityCountReader counter(file_name, 0xFFFFFFFF);
			SpawnObserver::prepare_info_s prepare_info( counter.Count(IsHumanHeader),
				counter.Count(IsTraderHeader),
				counter.Count(IsVehicleHeader));

			DoSpawn(builder,rules,zones,0xFFFFFFFF);
		}

	private:
		void DoSpawn(EntityBuilder* builder, RuleSet* rules, const zone_map_t &zones, int phase)
		{
			std::string file_name = spawn_xls_dir + DirtyLinks::GetLevelRawSysName() + file_ext;

			HumanXlsSpawner humans(file_name, phase, zones);
			TraderXlsSpawner traders(file_name, phase, zones);
			VehicleXlsSpawner vehicles(file_name, phase, zones);

			vehicles.Spawn(builder, rules);
			humans.Spawn(builder, rules);
			traders.Spawn(builder, rules);
		}

		static std::vector<std::string> m_lockedlevels;
		static int m_phase;
		static int m_episode;
		static int m_difficulty;

		exit_map_t  m_exit;
		std::string m_previous;

		static std::string m_saved_previous_level_name;
	};
	std::vector<std::string> UsualSpawner::m_lockedlevels;
	int UsualSpawner::m_difficulty;
	int UsualSpawner::m_phase;
	int UsualSpawner::m_episode;
	std::string UsualSpawner::m_saved_previous_level_name;
}

//=================================================================

void Spawner::CreateUsual()
{
	m_instance = spawn_ptr_t(new UsualSpawner());
}

void Spawner::Detach(SpawnObserver* observer)
{
	m_observers.Detach(observer);
}

void Spawner::Attach(SpawnObserver* observer, SpawnObserver::event_t event)
{
	m_observers.Attach(observer, event);
}

void Spawner::Notify(SpawnObserver::event_t event, SpawnObserver::info_t info)
{
	m_observers.Notify(GetInst(), event, info);
}

#include "precomp.h"

#include "VolumeHelper.h"
#include "DataFromFile.h"
#include "ParadiseWorld.h"
#include <logic_ex/LocRes.h>

#include <common/graphpipe/graphpipe.h>
#include <common/3dEffects/EffectManager.h>
#include <common/D3DApp/GammaControl/GammaControl.h>
#include <common/D3DApp/Input/Input.h>

#include "Options/Options.h"
#include "Interface/OptionsScreen.h"
#include "Interface/WidgetSoundMgr.h"
#include "gamelevel/gamelevel.h"
#include "gamelevel/leveltologic.h"
#include "gamelevel/scattereditems.h"
#include "Skin/Shadow.h"
#include "character.h"
#include <Muffle/ISound.h>
#include "GameLevel/PoLGameLevel.h"

#include <boost/bind.hpp>
#include <boost/format.hpp>

#ifdef new
#undef new
#endif

namespace ParadiseWorldDetails
{

//=====================================================================================//
//                      class RealUpdater : public OptionsUpdater                      //
//=====================================================================================//
class RealUpdater : public OptionsUpdater
{
public:
	void SetMarksOnWalls(bool value)
	{//	отметины на стенах
		Options::Set("game.marks_on_walls",value);			
		if(IWorld::Get()->GetLevel())
			IWorld::Get()->GetLevel()->EnableMarks(GameLevel::MT_STATIC,value);
	}

	void SetDynamicLights(bool value)
	{//	динамический свет
		Options::Set("game.dynamic_lights",value);
		if(IWorld::Get()->GetLevel())
			IWorld::Get()->GetLevel()->EnableMarks(GameLevel::MT_DYNAMIC,value);
	}

	void SetEnvironment(bool value)
	{//	окружающая среда
		Options::Set("game.environment",value);
		if(IWorld::Get()->GetLevel())
			IWorld::Get()->GetLevel()->GetLongShot()->Enable(value);
	}

	void SetPath(bool value)
	{
		Options::Set("game.mark_path",value);
		LogicAPI::GetAPI()->OptionsChanged();
	}

	void SetHexGrid(bool value)
	{
		Options::Set("game.mark_wave_front",value);
		Options::Set("game.mark_exit_zone",value);
		Options::Set("game.mark_lands",value);
		LogicAPI::GetAPI()->OptionsChanged();
	}

	void SetShadowsQ(float value)
	{
		const float length = 6;
		Options::Set("system.video.shadowquality",static_cast<int>(value*length));
		Shadows::OptionsChanged();
	}

	void SetEffectsQ(float value)
	{
		IWorld::Get()->GetEffects()->SetEffectsQuality(value);
		Options::Set("system.video.effectsquality",value);
	}

	void SetGamma(float value)
	{
		const float offset = 0;
		const float length = 3;
		value = offset+value*length;
		Options::Set("system.video.gamma",value);
		GammaControl::SetGamma(value);
	}

	void SetVoicesVolume(float value)
	{
		//[0..100]
		Options::Set("game.sound.voicesvol",(int)(value*100));
		PoL::VolumeHelper::setSpeechVolume(value);
		//Muffle::ISound::instance()->asetVolume(Muffle::ISound::cSpeech,value);
	}

	void SetAmbientVolume(float value)
	{
		//[0..100]
		Options::Set("game.sound.ambientvol",(int)(value*100));
		PoL::VolumeHelper::setAmbientVolume(value);
		//Muffle::ISound::instance()->asetVolume(Muffle::ISound::cAmbient,value);
	}

	void SetEffectsVolume(float value)
	{
		//[0..100]
		Options::Set("game.sound.effectsvol",(int)(value*100));
		PoL::VolumeHelper::setEffectsVolume(value);
		//Muffle::ISound::instance()->asetVolume(Muffle::ISound::cEffects,value);
	}

	void SetMusicVolume(float value)
	{
		//[0..100]
		Options::Set("game.sound.musicvol",(int)(value*100));
		PoL::VolumeHelper::setThemesVolume(value);
		//Muffle::ISound::instance()->asetVolume(Muffle::ISound::cThemes,value);
	}

	void SetAnimaSpeed(float value)
	{
		//[0.5..5]
		extern float SpeedMul;
		value=saturate(value,0.0f,1.f);
		SpeedMul=0.5+value*(5-0.5f);
		SkAnim::SkSpeed = SpeedMul;
		Options::Set("game.anispeed",SpeedMul);
	}

	void SetCameraSpeed(float value)
	{
		//[0.5..5]
		extern float KEYSPEED;
		extern float MOUSESPD;
		value=saturate(value,0.0f,1.f);
		KEYSPEED=MOUSESPD=0.5+value*(5-0.5f);
		Options::Set("game.camera_speed",KEYSPEED);
	}

	void SetMouseSensitivity(float value)
	{
		const float offset = 0.5;
		const float length = 6;
		value = offset+value*length;
		Options::Set("system.input.sensitivity",value);
		Input::Mouse()->SetSensitivity(value);
	}

	void SetEnemyMovement(bool value)
	{
		Options::Set("game.hidden_movement.enemy_related.move",value);
	}

	void SetNeutralMovement(bool value)
	{
		Options::Set("game.hidden_movement.neutral_related.move",value);
	}

	void SetAllyMovement(bool value)
	{
		Options::Set("game.hidden_movement.friend_related.move",value);
	}

	void SetCivilianMovement(bool value)
	{
		Options::Set("game.hidden_movement.civilian_humans_traders.move",value);
	}

	void SetVehicleMovement(bool value)
	{
		Options::Set("game.hidden_movement.civilian_vehicles.move",value);
	}

	void SetEnemyFire(bool value)
	{
		Options::Set("game.hidden_movement.enemy_related.shoot",value);
	}

	void SetNeutralFire(bool value)
	{
		Options::Set("game.hidden_movement.neutral_related.shoot",value);
	}

	void SetAllyFire(bool value)
	{
		Options::Set("game.hidden_movement.friend_related.shoot",value);
	}

	void SetCivilianFire(bool value)
	{
		Options::Set("game.hidden_movement.civilian_humans_traders.shoot",value);
	}

	void SetVehicleFire(bool value)
	{
		Options::Set("game.hidden_movement.civilian_vehicles.shoot",value);
	}

public:
	bool GetPath(void)
	{
		return Options::GetInt("game.mark_path");
	}

	bool GetHexGrid(void)
	{
		return Options::GetInt("game.mark_wave_front") ||
			Options::GetInt("game.mark_exit_zone")  ||
			Options::GetInt("game.mark_lands");
	}
};

RealUpdater Updater;

}

using namespace ParadiseWorldDetails;

typedef PoLGameLevel GameLevel_t;

//=====================================================================================//
//                         static std::string InterpretExpr()                          //
//=====================================================================================//
static std::string InterpretExpr(const std::string &expr)
{
	// %d|0|100
	const size_t pos1 = expr.find('|');
	if(pos1 == std::string::npos) return expr;
	const size_t pos2 = expr.find('|',pos1+1);
	if(pos2 == std::string::npos) return expr;
	const std::string fmt = expr.substr(0,pos1);
	const std::string lowers = expr.substr(pos1+1,pos2-pos1-1);
	const std::string uppers = expr.substr(pos2+1);

	const int lower = boost::lexical_cast<int>(lowers);
	const int upper = boost::lexical_cast<int>(uppers);
	const int number = (rand() % (upper-lower+1)) + lower;

	return (boost::format(fmt) % number).str();
}

//=====================================================================================//
//                      static std::string GenerateDescription()                       //
//=====================================================================================//
static std::string GenerateDescription(const std::string &desc)
{
	std::string result;

	size_t start = 0;
	size_t end = 0;

	while((start = desc.find('$',start)) != std::string::npos)
	{
		const size_t rbrace = desc.find(')',start);
		if(start + 2 >= desc.size()) return desc;
		if(rbrace == std::string::npos) return desc;

		result.append(desc.substr(end,start-end));
		result.append(InterpretExpr(desc.substr(start+2,rbrace-start-2)));

		start = end = rbrace + 1;
	}

	result.append(desc.substr(end));

	return result;
}

//=====================================================================================//
//                           ParadiseWorld::~ParadiseWorld()                           //
//=====================================================================================//
ParadiseWorld::ParadiseWorld(GraphPipe *pipe)
:	m_aliasCounter(0),
	m_pipe(pipe),
	m_effectsMgr(pipe)
{
	LoadLevelNames();
}

//=====================================================================================//
//                           ParadiseWorld::ParadiseWorld()                            //
//=====================================================================================//
ParadiseWorld::~ParadiseWorld()
{
}

//=====================================================================================//
//                     EffectManager* ParadiseWorld::GetEffects()                      //
//=====================================================================================//
EffectManager* ParadiseWorld::GetEffects()
{
	return &m_effectsMgr;
}

//=====================================================================================//
//                             void ParadiseWorld::Tick()                              //
//=====================================================================================//
void ParadiseWorld::Tick()
{
	Timer::Update();
	float t = Timer::GetSeconds();
	GetCharacterPool()->Update(t);
}

//=====================================================================================//
//                             void ParadiseWorld::Draw()                              //
//=====================================================================================//
void ParadiseWorld::Draw()
{
	GetCharacterPool()->Draw();
	GetItemsPool()->Draw();
}

//=====================================================================================//
//                          void ParadiseWorld::ChangeLevel()                          //
//=====================================================================================//
void ParadiseWorld::ChangeLevel(const std::string &level)
{
	if(m_level.get())
	{
		m_level.reset();
	}

	if(!level.empty())
	{
		m_level.reset(new GameLevel_t(level));
	}
}

//=====================================================================================//
//                  CharacterPool* ParadiseWorld::GetCharacterPool()                   //
//=====================================================================================//
CharacterPool*  ParadiseWorld::GetCharacterPool()
{
	return &m_charpool;
}

//=====================================================================================//
//                         bool ParadiseWorld::MakeSaveLoad()                          //
//=====================================================================================//
bool ParadiseWorld::MakeSaveLoad(GenericStorage &st)
{
	if(st.IsSaving())
	{
		SaveLevelDescs(SavSlot(&st,"uc_leveldescs"));
		SaveObjectDescs(SavSlot(&st,"uc_objectdescs"));
	}
	else
	{
		if(st.GetVersion() >= 17)
		{
			LoadLevelDescs(SavSlot(&st,"uc_leveldescs"));
			LoadObjectDescs(SavSlot(&st,"uc_objectdescs"));
		}
	}

	if(!st.IsSaving())
	{
		m_level.reset();
		m_level.reset(new GameLevel_t(st));
	}
	else
	{
		m_level->Save(st);
	}

	IWorld::Get()->GetCharacterPool()->MakeSaveLoad(st);
	SavSlot itcell(&st, "items");

	GetItemsPool()->MakeSaveLoad(itcell);

	return true;
}

//=====================================================================================//
//                        void ParadiseWorld::SaveLevelDescs()                         //
//=====================================================================================//
void ParadiseWorld::SaveLevelDescs(SavSlot &slot)
{
	slot << m_aliasCounter;
	slot << m_levelsDesc.size();

	for(LevelDescMap_t::const_iterator i = m_levelsDesc.begin(); i != m_levelsDesc.end(); ++i)
	{
		slot << i->first << i->second;
	}
}

//=====================================================================================//
//                        void ParadiseWorld::LoadLevelDescs()                         //
//=====================================================================================//
void ParadiseWorld::LoadLevelDescs(SavSlot &slot)
{
	size_t count;
	slot >> m_aliasCounter;
	slot >> count;
	m_levelsDesc.clear();

	for(size_t i = 0; i != count; ++i)
	{
		std::string id, desc;
		slot >> id >> desc;
		m_levelsDesc.insert(std::make_pair(id,desc));
	}
}

//=====================================================================================//
//                        void ParadiseWorld::SaveObjectDescs()                        //
//=====================================================================================//
void ParadiseWorld::SaveObjectDescs(SavSlot &slot)
{
	slot << m_objectInfos.size();

	for(ObjectInfos_t::const_iterator i = m_objectInfos.begin(); i != m_objectInfos.end(); ++i)
	{
		slot << i->first << i->second.m_isVisible << i->second.m_isDestroyed;
	}
}

//=====================================================================================//
//                        void ParadiseWorld::LoadObjectDescs()                        //
//=====================================================================================//
void ParadiseWorld::LoadObjectDescs(SavSlot &slot)
{
	m_objectInfos.clear();

	size_t count;
	slot >> count;

	for(size_t i = 0; i != count; ++i)
	{
		StoredObjectInfo info;
		std::string name;
		slot >> name >> info.m_isVisible >> info.m_isDestroyed;
		m_objectInfos.insert(std::make_pair(name,info));
	}
}

//=====================================================================================//
//                      ItemsPool *ParadiseWorld::GetItemsPool()                       //
//=====================================================================================//
ItemsPool *ParadiseWorld::GetItemsPool()
{
	return &m_items;
}

//=====================================================================================//
//                 OptionsUpdater *ParadiseWorld::GetOptionsUpdater()                  //
//=====================================================================================//
OptionsUpdater *ParadiseWorld::GetOptionsUpdater()
{
	return &Updater;
}

//=====================================================================================//
//                     GameLevel *ParadiseWorld::GetLevel() const                      //
//=====================================================================================//
GameLevel *ParadiseWorld::GetLevel() const
{
	return m_level.get();
}

//=====================================================================================//
//                      Camera *ParadiseWorld::GetCamera() const                       //
//=====================================================================================//
Camera *ParadiseWorld::GetCamera() const
{
	return GetPipe()->GetCam();
}

//=====================================================================================//
//                        void ParadiseWorld::LoadLevelNames()                         //
//=====================================================================================//
void ParadiseWorld::LoadLevelNames()
{
	using mll::utils::table;

	std::istringstream sstr(getDataFromFile("scripts/levels.txt"),std::ios::binary);
	table tbl;

	sstr >> tbl;

	table::row_type header = tbl.row(0);
	table::iterator sysname = std::find(header.begin(),header.end(),"level");

	for(int i = 1; i < tbl.height(); ++i)
	{
		const std::string desc = PoL::getLocStr("levels." + tbl.col(sysname)[i] + ".levelname");
		m_levelsDesc.insert(std::make_pair(tbl.col(sysname)[i],desc));
	}
}

//=====================================================================================//
//                   std::string ParadiseWorld::GetLevelDesc() const                   //
//=====================================================================================//
std::string ParadiseWorld::GetLevelDesc() const
{
	return GetLevelDesc(GetLevel()->GetLevelName());
}

//=====================================================================================//
//                   std::string ParadiseWorld::GetLevelDesc() const                   //
//=====================================================================================//
std::string ParadiseWorld::GetLevelDesc(const std::string &l) const
{
	LevelDescMap_t::const_iterator i = m_levelsDesc.find(l);
	if(i == m_levelsDesc.end()) return "# " + l + " #";
	return i->second;
}

//=====================================================================================//
//                    std::string ParadiseWorld::CreateLevelAlias()                    //
//=====================================================================================//
std::string ParadiseWorld::CreateLevelAlias(const std::string &lev)
{
	std::ostringstream sstr;
	sstr << lev << ":" << m_aliasCounter++;
	const std::string desc = GenerateDescription(GetLevelDesc(lev));
	m_levelsDesc.insert(std::make_pair(sstr.str(),desc));
	return sstr.str();
}

//=====================================================================================//
//               const StoredObjectInfo *ParadiseWorld::GetObjectInfo()                //
//=====================================================================================//
const StoredObjectInfo *ParadiseWorld::GetObjectInfo(const std::string &id)
{
	ObjectInfos_t::const_iterator i = m_objectInfos.find(id);
	if(i != m_objectInfos.end()) return &i->second;
	return 0;
}

//=====================================================================================//
//                        void ParadiseWorld::SetObjectInfo();                         //
//=====================================================================================//
void ParadiseWorld::SetObjectInfo(const std::string &id, const StoredObjectInfo &info)
{
	m_objectInfos[id] = info;
}

//=====================================================================================//
//                               static bool isOnLevel()                               //
//=====================================================================================//
template<typename T>
static bool isOnLevel(const std::string &level, const T &value)
{
	return value.first.substr(0,level.size()) == level;
}

//=====================================================================================//
//                        void ParadiseWorld::ResetObjectInfo()                        //
//=====================================================================================//
void ParadiseWorld::ResetObjectInfo(const std::string &level)
{
	ObjectInfos_t result;
	std::remove_copy_if(
		m_objectInfos.begin(),m_objectInfos.end(),
		std::inserter(result,result.end()),
		boost::bind(isOnLevel<ObjectInfos_t::value_type>,level,_1));
	m_objectInfos.swap(result);
}

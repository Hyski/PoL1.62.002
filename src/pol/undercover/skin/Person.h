// Person.h: interface for the Person class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PERSON_H__C9963DA5_FFEA_4D8F_9E2F_43D7C175D6EF__INCLUDED_)
#define AFX_PERSON_H__C9963DA5_FFEA_4D8F_9E2F_43D7C175D6EF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../character.h"
#include <Muffle/ISound.h>
#include "skanim.h"
#include "BSpline.h"

class Skin;     //Punch
class Skeleton; //Punch
class SkSkin;
class SkAnim;
class SkState;
struct AnimaData;

class SoundPerson;


namespace Shadows { class BaseShadow; }
namespace PoL { class RealEntityMover; }

enum STAND{PEACESTAY=0x01,WARSTAY=0x02,SIT=0x04,SUITSTAY=0x08}; //варианты стоек
enum WEAPONS{FREEHANDS=0x01,PISTOL=0x02, RIFLE=0x04, AUTOCANNON=0x08,
ROCKET=0x10, SPECIAL=0x20, GRENADE=0x40,
CANNON=0x80};

class CharacterPool;
class Banner;
class spline;
class BoneState;

//=====================================================================================//
//                          class RealEntity : public IEntity                          //
//=====================================================================================//
class RealEntity : public IEntity
{
	friend class Shadows::BaseShadow;
	friend class CharacterPool;

	void ClearMovePoints();

	float m_skinLastUpdateTime;
	float getUpdateDelta() const;

protected://то что касается баннеров
	Banner *BillBoard;

protected:
	unsigned Effects[ET_SIZE];
	bool     EffectsState[ET_SIZE];
	float FOSrad1,FOSrad2,FOSangle; 

protected: // передвижение
	std::auto_ptr<PoL::RealEntityMover> m_mover;
	//IMovingCallBack *MoveCB;       //колбэки для движения
	//PoL::BSpline7 m_moveSpline;            //траектория движения
	//std::vector<ipnt2_t> MovePoints; //набор точек, по которым идем
	//std::vector<ipnt2_t> MovePointHex; // хексы на сплайне 
	//std::vector<float> MovePointHexPos; // координаты хексов на сплайне
	//int m_minNotified; // индекс первого непройденного хекса

	//bool IsHexNotified(int h) const { return h < m_minNotified; }
	//IMovingCallBack::COMMAND DoVisitHex(int i, IMovingCallBack::EVENT);

	//int LastPoint;                   //последняя хексовая точка, где побывали
	//float SplinePos;

protected:
	SoundPerson *SoundScheme;//набор звуков персонажа

protected://common сугубо внутренние данные
	std::string Name;//имя типа персонажа (напр. Полицейский)
	std::string SysName;//системное имя типа персонажа (напр. Полицейский)

	static const int MaxShotBones = 16;
	std::string m_shotBones[MaxShotBones];
	int m_shotBoneId[MaxShotBones];
	int m_shotBoneCount;

	unsigned Stand;//тип стояния
	unsigned CurWeapon;// текущее оружие
	unsigned CurState;//текущая анимация

	AnimaData *LastPose;   //предыдущая поза персонажа
	SkAnim *CurAnima;// та анимация, которая сейчас активна
	point3    Location;
	float  Angle;    //угол поворота персонажа
	SkSkin     *Normal;     //текущий вид
	SkSkin     *Weapons; //..
	float LastUpdate; //время, когда последний раз накладывалась кожа
	float DesAngle;//угол в сторону которого нужно будет смотреть

	bool m_firstUpdate; ///< Нужно сделать первый апдейт

protected://common персоналии
	bool DestLinked;// флаг показывает, что есть привязка к Dest
	point3 Dest;//к этой точке чувак будет стараться дойти
	D3DMATRIX World,InvWorld; //точка привязки скелета
	BBox SkinBox; //D\В мировых координатах
	BBox LocalSkinBox; // в локальных координатах

protected://общие
	//void MoveBySpline(float Time);
	//bool NotifyHex(IMovingCallBack::EVENT Ev);
	//void PlaceOnSpline(float val);

	RealEntity();
	virtual ~RealEntity();
	void UpdateSkinParms();
	void UpdateWorldMat(float Angle, const point3 &pos);
	void AnimaOver(float Time);
	void CorrectDirWalking(float Time);
	void CorrectDirTurning(float Time);
	virtual float SwitchAnimation(int Anima, float Time, float Delay)=0;
	void SetSoundScheme(SoundPerson *Scheme);
	// ~~~~~~~~~~~~~ ShadowCaster
	Shadows::BaseShadow	*shadow;	// Тень от персонажа
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~

public://common
	bool Visible; //показывает виден ли товарищ в камере
	bool Enabled; //Виден ли он логике

	bool IsReallyVisible() const { return Visible && Enabled; }

	float LastVisUpdate; //последний раз, когда обновлялась видимость
	void UpdateVisibility(float Time,GraphPipe *Pipe);//функция для обновления видимости

	void AddShotBone(const std::string &);
	const D3DMATRIX *GetWorld() const { return &World; }
	const D3DMATRIX *GetInvWorld() const { return &InvWorld; }
	const D3DMATRIX *GetBone(const std::string &);
	const SkState::BoneState *GetBoneEx(const std::string &);

public:  //common
	virtual bool NeedRotation(const point3& Dir)
	{
		point3 mydir(-(*(point3*)&World._21));
		point3 dir(Dir);
		mydir.z=dir.z=0;
		return cos(TORAD(3))>(Normalize(dir).Dot(Normalize(mydir)));
	};
	virtual void MakeSaveLoad(SavSlot &sl)=0;
	point3 GetLocation() const {return Location;}
	void SetLocation(const point3 &Pos, float Angle);
	float SetAngle(float CurTime, float Angle);
	float GetAngle();
	void SetDestination(const point3  &d);
	void Draw();
	bool TraceRay(const ray &r, float *Dist, point3 *Norm,bool AsBox);
	point3 GetShotPoint(point3 suppose_point);
	point3 GetShotPointNumber(int);
	int GetShotPointCount();
	void GetLOSPoints(points_t *points) const;
	virtual void SetOwnership(OWNERSHIP ownby);

	const std::string &GetSysName() const { return SysName; }

	bool TraceSegment(const point3 &From, const point3 &Dir, point3 *Res, point3 *Norm);

	void UpdateOnTime(float Time);
	float PoLSetAction(unsigned /*ANIMATION*/ Action,float CurTime);
	//установить текущее действие LAZY, WALK...
	float SetAction(unsigned /*ANIMATION*/ Action,float CurTime);
	unsigned int GetAction() const;
	bool IsMoving() const { return m_mover.get() != 0; }

	unsigned int GetCurState() const { return CurState; }
	float GetAnimVelocity() const
	{
		point3 p1,p2;
		p1 = CurAnima->GetOffset(LastPose,LastPose->StartTime);
		p2 = CurAnima->GetOffset(LastPose,LastPose->EndTime);
		return (p2-p1).Length()/(LastPose->EndTime - LastPose->StartTime);
	}

public:
	void PoLStartMoving(ANIMATION Type, const std::vector<ipnt2_t> &hexes, PoL::MovementController *ctrl);
	void StartMoving(ANIMATION Type, const std::vector<ipnt2_t> &hexes, PoL::MovementController *ctrl);
	void Enable(bool flag);
	void SetBanner(const std::string &banner,unsigned int Color);
	void DrawBanner();
	void DrawFOS();
	void ShowEffect(EFFECT_TYPE type, SHOW_MODE mode);
	void SetFOS(float front_radius, float back_radius, float sight_angle);

public: //касательно звуковой схемы
	void HandleSoundEvent(unsigned type);//SoundPerson::EVENTTYPE type;

public:
	float AnimaQual;//качество анимации
	bool m_actMove;

	SkState m_LastState; //как выглядел последний обновленный скелет.

};

//=====================================================================================//
//                          class Person : public RealEntity                           //
//=====================================================================================//
class Person : public RealEntity
{
protected:
	struct SkelData
	{
		enum{ANIMANAMES=17,WEAPONSNUM=8,MAXANIMANUM=10,SUITSNUM=5};
		std::string Name;
		std::string SkinNames[SUITSNUM+1];
		SkAnim *Animations[4][ANIMANAMES][WEAPONSNUM][MAXANIMANUM]; //ссылки на все анимации   [стойка][оружие][анимация][разные варианты]
		int       AnimaCount[4][ANIMANAMES][WEAPONSNUM]; //количество анимаций   [стойка][оружие][анимация]
	};
public:
	typedef std::map<std::string,SkelData> SkelCache_t;
protected://common сугубо внутренние данные
	static SkelCache_t SkelsCache;
	SkelData Skels;
	SkSkin     *Suits[SkelData::SUITSNUM+1];   //Вид в костюме+простая кожа
	std::string CurWeapName;
protected://специфичные функции
	void ParseAnimas(SkAnim *Animas[],int *Count,std::string &data);
	float SwitchAnimation(int Anima, float Time, float Delay);
public:
	enum ARMOUR_TYPE {ARM_NORM=0, ARM_SUIT1=1, ARM_SUIT2=2, ARM_SUIT3=3, ARM_SUIT4=4, ARM_SCUBA=5};
	void SetWeapon(WEAPONS Weap, const std::string &WName, float CurTime);
	//установить стойку SIT, STAND...
	void SetStanding(STAND Type, float CurTime);
	void SetArmour(ARMOUR_TYPE Arm);
	void  GetBarrel(const point3 &Offs, point3 *pos);
	const SkelData& FillSkelsCache(std::string XlsName);
	void Load(const std::string &name,const std::string skins[],const std::string &sys_name="player");  //загрузка из .xls
	void MakeSaveLoad(SavSlot &sl);

	Person();
	virtual ~Person();
public://Преобразование интерфейсов
	Person  *GetHuman() {return this;}
};

//=====================================================================================//
//                           class Vehicle:public RealEntity                           //
//=====================================================================================//
//класс техники
class Vehicle:public RealEntity  
{
public:
	struct VehSounds
	{
		std::string Shot;
		std::string Death;
		std::string Engine;
		std::string Walk;
		std::string Hurt;
		std::string Lazy;
	};
	struct SkelData
	{
		enum{ANIMANAMES=7,MAXANIMANUM=10};
		std::string Name;
		std::string SkinName;
		SkAnim *Animations[ANIMANAMES][MAXANIMANUM]; //ссылки на все анимации 
		int       AnimaCount[ANIMANAMES]; //количество анимаций
		std::string BarrelBones[2]; //кости, к которым привязаны точки вылета пули
		point3      BarrelPos[2];   //точки вылета пули в координвтах кости (см. выше)
	};
	typedef std::map<std::string,SkelData> SkelCache_t;
protected://common сугубо внутренние данные
	static SkelCache_t SkelsCache;
	SkelData Skels;

protected:
	enum{ANIMANAMES=7,MAXANIMANUM=10};
protected://персоналии
	unsigned    CurBarrel;      //текущий активный ствол вылета пули
	VehSounds   Sounds;
protected://functions
	void AnimaOver(float Time);
	void ParseAnimas(SkAnim *Animas[],int *Count,std::string &data);
	float SwitchAnimation(int Anima, float Time, float Delay);
public:
	const SkelData& FillSkelsCache(std::string XlsName);
	void MakeSaveLoad(SavSlot &sl);
	void  GetBarrel(const point3 &/*фиктивный параметр*/, point3 *pos);
	Vehicle();
	void Load(const std::string &name, VehSounds &sounds);  //загрузка из .xls
	virtual ~Vehicle();
public://Преобразование интерфейсов
	Vehicle *GetVehicle() {return this;};
};

/**********************************************************************************
классы, служащие для озвучки персонажей
***********************************************************************************/
#pragma warning (disable:4503)
class TxtFile;

//=====================================================================================//
//                                 class SoundsParser                                  //
//=====================================================================================//
class SoundsParser
{
public:
	static void SetLevel(const std::string &Name);
	static SoundPerson *ParseForPerson(const std::string &Name);
	static SoundPerson *ParseForVehicle(Vehicle::VehSounds *Sounds);
	static void Precache();
	static Clear(){EventCache.clear();LevelSteps.clear();CacheLoaded=false;};
private:
	struct wave_list_t
	{
		std::vector<std::string> names;
		float parm;
	}; 
	static void ParseString(const std::string &in, const std::string &fieldname,wave_list_t *str);
	typedef std::map<std::string,std::string>   StrMap_t;//событие->данные
	typedef std::map<std::string, StrMap_t>   SysNameMap_t;//человек->набор событий
private:
	static std::string *GetEvent(const std::string &sys_name, const std::string &event);
private:
	static bool CacheLoaded;
	static SysNameMap_t EventCache;
	static StrMap_t LevelSteps; //отображение уровень->анимации ходьбы
	static std::string CurLevel;  //текущий уровень
};

//=====================================================================================//
//                                  class SoundPerson                                  //
//=====================================================================================//
class SoundPerson
{
	friend class SoundsParser;
	friend class SoundEventsQueue;
	friend class Vehicle;
public:
	enum EVENTTYPE {ET_RANDOM2D,ET_RANDOM3D, ET_LAZY, ET_WALK, ET_RUN,
		ET_HURT, ET_CHURT, ET_LUCKYSTRIKE, ET_DEATH, ET_ENEMYSIGHTED, ET_GIRL,
		ET_SELECTION,ET_MOVE, ET_ATTACK, ET_MISS, ET_UNDERFIRE,
		ET_TREATMENT_LOW, ET_TREATMENT_MED, ET_TREATMENT_HIGH,
		ET_TEAM_COME, ET_TEAM_DISMISS, ET_TEAM_NEW, ET_TEAM_LEAVE,
		ET_ENEMYDIED, ET_ENEMY_EXPLODED, ET_OPEN_OK, ET_OPEN_FAILED, 
		ET_CAR_FAILED, ET_LEVELUP,
		ET_GOT_RLAUNCHER, ET_GOT_AUTOCANNON, ET_GOT_SHOTGUN, ET_GOT_ENERGYGUN,
		ET_USE_PLASMA_GREN, ET_SAW_BIGBANG, ET_VEH_AMBIENT,	ET_NO_LOF, ET_SCANNER,

		ET_VEH_MAXTYPES,
		ET_STOPIT=0x8000
	};
public:
	void Tick(float Time, bool Enabled);     //регулярно вызываемая функция для возможности подумать
	void Event(unsigned type); //сообщение о событии
	void UpdatePos(const point3 &pos); //обновление позиции
	void Stop();  //остановка всех проигрываемых звуков
	void SetOwnership(IEntity::OWNERSHIP ownby);

	~SoundPerson();
	SoundPerson():m_OwnBy(IEntity::OS_OTHER),m_Current(NULL){;};

	class EventHandler;

protected:
	bool CanSkipEv(EVENTTYPE ev){return ev==ET_WALK||ev==ET_RUN||ev==ET_SCANNER||ev==ET_VEH_AMBIENT;};
	void AddEvent(EVENTTYPE type, EventHandler *ev); //добавить обрабатываемое событие
private:
	typedef std::map<EVENTTYPE,EventHandler*> Events_t;
	Events_t Events;
	std::string m_SysName;
	IEntity::OWNERSHIP m_OwnBy;
	EventHandler *m_Current;//текущее играемое событие

};

//=====================================================================================//
//                               class SoundEventsQueue                                //
//=====================================================================================//
class SoundEventsQueue
{
public:
	static void SoundStarted(const std::string& sys_name, SoundPerson::EVENTTYPE Type,SoundPerson::EventHandler *Ev);
	static void Tick(float Time);     //регулярно вызываемая функция для возможности подумать
	static void Clear();
	static void PersonDied(const std::string &sys_name);
private:
	struct _TypeNameRef
	{
		std::string SysName;
		SoundPerson::EVENTTYPE Type;
		SoundPerson::EventHandler *Ev;
	};
	static std::queue<_TypeNameRef> m_TeamPhrases;
};

#endif // !defined(AFX_PERSON_H__C9963DA5_FFEA_4D8F_9E2F_43D7C175D6EF__INCLUDED_)

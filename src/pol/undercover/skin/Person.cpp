// Person.cpp: implementation of the Person class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable:4503)
#include "precomp.h"
#include <common/graphpipe/graphpipe.h>
#include "skin.h"
#include "skanim.h"
#include "../Gamelevel/Gamelevel.h"
#include "skeleton.h"
#include "AnimaLibrary.h"
#include "../gamelevel/grid.h"

#include "Person.h" 
#include "../options/options.h"
#include "Shadow.h"
#include "../iworld.h"
#include "../interface/banner.h"
#include "../interface/interface.h"
#include <common/3deffects/effectmanager.h>
#include <logic2/hexutils.h>
#include <common/utils/profiler.h>
#include <common/shell/shell.h>
#include <common/GraphPipe/statesmanager.h>
#include "shadowaccum.h"

#include <common/graphpipe/PersonBoneCameraStrategy.h>


#include "../game.h"

#include "../Interface/MenuMgr.h"
#include "../Interface/GameScreen.h"
#include "../Interface/LoadingScreen.h"

#include "RealEntityMover.h"



//#define CHECK_SOUNDS
#ifdef CHECK_SOUNDS
static char *snd_type2name[]={"RANDOM2D", "RANDOM3D", "LAZY", "WALK", "RUN",
"HURT", "CHURT", "LUCKYSTRIKE", "DEATH", "ENEMYSIGHTED", "GIRL",
"SELECTION", "MOVE", "ATTACK", "MISS", "UNDERFIRE",
"TREATMENT_LOW", "TREATMENT_MED", "TREATMENT_HIGH",
"TEAM_COME", "TEAM_DISMISS", "TEAM_NEW", "TEAM_LEAVE",
"ENEMYDIED", "ENEMY_EXPLODED", "OPEN_OK", "OPEN_FAILED",
"CAR_FAILED", "LEVELUP",
"GOT_RLAUNCHER", "GOT_AUTOCANNON", "GOT_SHOTGUN", "GOT_ENERGYGUN",
"USE_PLASMA_GREN", "SAW_BIGBANG", "VEH_AMBIENT", "NOLOF", "SCANNER"};
#endif

bool isHidMovScreen()
{
	Widget * const gms = MenuMgr::Instance()->Child(GameScreen::m_pDialogName);
	Widget * const hms = gms ? gms->Child(HiddenMovementScreen::m_pDialogName) : 0;
	const bool gmsvisible = gms && gms->IsVisible();
	return gmsvisible && hms && hms->IsVisible();
}

static const std::string PERSONSKEL("animations/anims/characters/");
static const std::string PERSONSKIN("animations/skins/characters/");
static const std::string WEAPONSKIN("animations/skins/items/weapons/");
static const std::string VEHICLESKEL("animations/anims/vehicles/");
static const std::string VEHICLESKIN("animations/skins/vehicles/");
static const std::string VEHXLS("scripts/species/vehicle.txt");

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
float SpeedMul;
const float DESTFAULT=0.1;
static const float TURNINGSPEED=TORAD(240);//поворот рад/сек на месте
static const float TURNINGFAULT=TORAD(3); //ошибка угла поворот на месте
static const float WTURNINGSPEED=TORAD(90);//поворот рад/сек при ходьбе
static const float WTURNINGFAULT=TORAD(25);//ошибка угла поворот при ходьбе

/*
//=====================================================================================//
//                                    class spline                                     //
//=====================================================================================//
class spline
{
private:
	int power;
	std::vector<point3> v;
	float Sp(float,float,float);
	float m_nm1;

public:
	void Clear()
	{
		v.clear();
	}

	point3 GetPoint(float);

	spline(float _power=7)
	{
		power=_power=7;
		m_nm1 = power-3;
	}
	void AddPoint(const point3 &p)
	{
		v.push_back(p);
		if(v.size()==1)
		{
			for(int i = 0; i <= m_nm1; ++i)
			{
				v.push_back(p);
			}
		}
	};
	void End()
	{
		if(v.empty())return;
		for(int i = 0; i <= m_nm1; ++i)
		{
			v.push_back(v.back());
		}
	};
	void ShowIt();
};

//=====================================================================================//
//                                 float spline::Sp()                                  //
//=====================================================================================//
float spline::Sp(float i,float q,float t)
{
	float retVAL;
	if (q==1)
	{
		if ((t>=i)&&(t<i+1)) retVAL=1;
		else retVAL=0;
	}
	else
	{
		retVAL= (t-i)   /(q-1) * Sp(i  ,q-1,t)
			+(i+q-t) /(q-1) * Sp(i+1,q-1,t);
	}
	return retVAL;
}

//=====================================================================================//
//                              point3 spline::GetPoint()                              //
//=====================================================================================//
point3 spline::GetPoint(float t)
{
	point3 p(NULLVEC);
	t *= v.size() - m_nm1*2.0f - 2;
	t += m_nm1 + 4;
	for(int q = 0; q < v.size(); q++)
	{
		p += Sp(q,power,t) * v[q];
	}
	return p;
}

//=====================================================================================//
//                                void spline::ShowIt()                                //
//=====================================================================================//
void spline::ShowIt()
{
	if(v.empty()) return;
	GraphPipe *Pipe=IWorld::Get()->GetPipe();
	const unsigned int num = 77;
	Primi prim(num);
	Primi::FVFVertex *vts = &prim.Verts[0];
	float i=0;
	for(int n=0;i<1;i+=1.f/(num+1),n++)
	{
		vts[n].pos = GetPoint(i);
	}
	prim.Prim=Primi::LINESSTRIP;
	prim.Contents=Primi::NEEDTRANSFORM;
#ifdef _HOME_VERSION
	Pipe->Chop("purewhite",&prim,"Spline");
#else
	Pipe->Chop("purewhite",&prim);
#endif
}

*/

//=====================================================================================//
//                              static void drawSpline()                               //
//=====================================================================================//
template<typename T>
static void drawSpline(const T &spline)
{
	if(!spline.isBaked()) return;

	GraphPipe *pipe = IWorld::Get()->GetPipe();
	const unsigned int num = 77;
	Primi prim(num);
	Primi::FVFVertex *vts = &prim.Verts[0];
	float i=0;
	for(int n = 0; i < 1; i += 1.f/(num+1.0f), n++)
	{
		vts[n].pos = spline.getPoint(i);
	}

	prim.Prim = Primi::LINESSTRIP;
	prim.Contents = Primi::NEEDTRANSFORM;
#ifdef _HOME_VERSION
	pipe->Chop("purewhite",&prim,"Spline");
#else
	pipe->Chop("purewhite",&prim);
#endif
}

/**********************************************************************************
классы, служащие для озвучки персонажей
***********************************************************************************/

std::queue<SoundEventsQueue::_TypeNameRef> SoundEventsQueue::m_TeamPhrases;

//=====================================================================================//
//                                  class SoundPlayer                                  //
//=====================================================================================//
class SoundPlayer
{
	Muffle::HEmitter m_Emitter;

public:
	SoundPlayer() : m_Emitter(0) {Registered=false;}
	SoundPlayer(const SoundPlayer &a) : m_Emitter(0) {*this=a;}
	~SoundPlayer(){Stop();}

	SoundPlayer& operator=(const SoundPlayer &a)
	{
		m_Pos=a.m_Pos;
		m_Vel=a.m_Vel;
		Registered=a.Registered;
		return *this;
	}

public: //поддержка интерфейсов sound:sdemit
	point3& GetPos(){return m_Pos;};
	point3& GetVel(){return m_Vel;};

public:
	void Play2d(const std::string &Name,bool cycled=false)
	{       
		Play_(cycled?"charactercycled2d":"character2d",Name,true);  
	}

	void Play3d(const std::string &Name,bool cycled=false)
	{       Play_(cycled?"charactercycled":"character",Name,false); 
	}

	void SetPosition(const point3 &Pos)
	{
		if(m_Emitter.get())
		{
			m_Emitter->setPosition(Pos.as_mll_vector());
		}

		m_Pos=Pos;
	}

	void Stop()
	{
		if(m_Emitter.get())
		{
			m_Emitter->stop();
			m_Emitter.reset();
			Unreg();
		}
	}

	bool IsPlaying()
	{
		bool p=m_Emitter.get()?m_Emitter->isPlaying():false;
		if (m_Emitter.get()&&!p)
		{
			Stop();
		}
		return p;
	};
private:
	void Reg(){if(!Registered){Registered=true;}};
	void Unreg(){if(Registered){Registered=false;}};
	void Play_(const std::string &script, const std::string &name,bool /*is2d*/)
	{
		Stop();
		Reg();
		Muffle::HScript snd_script = Muffle::ISound::instance()->getScript(script.c_str());
		m_Emitter = Muffle::ISound::instance()->createEmitter(snd_script,name.c_str());
		m_Emitter->setPosition(m_Pos.as_mll_vector());
		m_Emitter->play();
	}
protected:
	point3 m_Pos,m_Vel;
	bool Registered;
};  

class SoundPerson::EventHandler // класс для обработки звукового события.
{
	friend class SoundsParser;
	friend class SoundEventsQueue;
	friend class SoundPerson;
public:
	void Go();
	void Stop(){m_Player.Stop();};
	void UpdatePos(const point3 &pos){m_Player.SetPosition(pos);};
	void Tick(float Time,bool ClearWathdog=false);     //регулярно вызываемая функция для возможности подумать
	bool IsPlaying() {return m_Player.IsPlaying();};
	const SoundPlayer* GetPlayer(){return &m_Player;};
	bool is2d() const {return m_Is2d;};
	enum TYPE {T_BYPERCENT, T_BYFREQ};
private:
	TYPE m_PlayingType;
	bool m_Is2d;
	bool m_Cycled;
	float m_Percent;      //с какой вероятностью при событии играется звук. [0..1]
	float m_Frequency;    //раз в m_Frequency секунд играется звук 
	int m_LastPlayed;
	std::vector<std::string> m_SndNames;
	SoundPlayer m_Player;
	int m_Priority;
private:
	float m_LastBeat;
public:
	EventHandler():m_Cycled(false),m_LastBeat(0),m_Frequency(1),m_Percent(0),m_Is2d(true),m_PlayingType(T_BYPERCENT),m_LastPlayed(-1){};
};

void SoundEventsQueue::PersonDied(const std::string& sys_name)
{
	if(m_TeamPhrases.size())
	{
		std::queue<_TypeNameRef> p=m_TeamPhrases;
		while (m_TeamPhrases.size()) m_TeamPhrases.pop();
		while(p.size())
		{
			if(p.front().SysName!=sys_name)
			{
				m_TeamPhrases.push(p.front());
			}
			p.pop();
		}
		if(m_TeamPhrases.size() && !m_TeamPhrases.front().Ev->IsPlaying()) 
		{
			m_TeamPhrases.front().Ev->Go();
		}
	}


}

void SoundEventsQueue::SoundStarted(const std::string& sys_name, SoundPerson::EVENTTYPE Type,SoundPerson::EventHandler *Ev)
{
	//
	_TypeNameRef dt;
	dt.Ev=Ev;
	dt.Type=Type;
	dt.SysName=sys_name;
	m_TeamPhrases.push(dt);
	if(m_TeamPhrases.size()==1)
	{
		Ev->Go();
	}
}
void SoundEventsQueue::Clear()
{
	while(m_TeamPhrases.size())
		m_TeamPhrases.pop();
}
void SoundEventsQueue::Tick(float Time)
{
	if(m_TeamPhrases.size())
	{
		_TypeNameRef p=m_TeamPhrases.front();
		if(!p.Ev->m_Player.IsPlaying())
		{
			m_TeamPhrases.pop();
			if(m_TeamPhrases.size()) 	m_TeamPhrases.front().Ev->Go();
		}
	}
}


//статические переменные
bool                       SoundsParser::CacheLoaded=false;
SoundsParser::SysNameMap_t SoundsParser::EventCache;
SoundsParser::StrMap_t     SoundsParser::LevelSteps; //отображение уровень->анимации ходьбы и бега
std::string                SoundsParser::CurLevel="h_house"; 

//=====================================================================================//
//                       bool IEntity::IsCameraAttached() const                        //
//=====================================================================================//
bool IEntity::IsCameraAttached() const
{
	if(IWorld::Get()->GetPipe()->GetCam()->GetMoveType() == cmtPersonBone &&
		PersonBoneCameraStrategy::m_realEntity == this) return true;
	return false;
}

//=====================================================================================//
//                              RealEntity::RealEntity()                               //
//=====================================================================================//
RealEntity::RealEntity()
:	BillBoard(NULL),
	LastPose(NULL),
	CurAnima(NULL),
	Normal(NULL),
	Weapons(NULL),
	Angle(0),
	Location(NULLVEC),
	LastVisUpdate(0),
	SoundScheme(NULL),
	m_shotBoneCount(0),
	m_actMove(false)
{
	memset(Effects,0,sizeof(Effects));
	memset(EffectsState,0,sizeof(EffectsState));
	Enabled=true;
	Visible=true;
	SkinBox.Degenerate();
	m_firstUpdate = true;
}

//=====================================================================================//
//                              RealEntity::~RealEntity()                              //
//=====================================================================================//
RealEntity::~RealEntity()
{
	delete BillBoard;
	delete LastPose;
	if(Weapons) delete Weapons;
	if(SoundScheme) delete SoundScheme;
	EffectManager *em=IWorld::Get()->GetEffects();
	for(int type=0;type<ET_SIZE;type++)
	{
		if(Effects[type]) em->DestroyEffect(Effects[type]);
	}
}

//=====================================================================================//
//          void RealEntity::HandleSoundEvent()//SoundPerson::EVENTTYPE type;          //
//=====================================================================================//
void RealEntity::HandleSoundEvent(unsigned type)//SoundPerson::EVENTTYPE type;
{
	if(!SoundScheme) return;
	if(!Enabled && type!=SoundPerson::ET_CAR_FAILED) return;

	SoundScheme->Event(type);
}

//=====================================================================================//
//                          void RealEntity::SetSoundScheme()                          //
//=====================================================================================//
void RealEntity::SetSoundScheme(SoundPerson *Scheme)
{
	if(SoundScheme) delete SoundScheme;
	SoundScheme=Scheme;
}

//=====================================================================================//
//                              void RealEntity::Enable()                              //
//=====================================================================================//
void RealEntity::Enable(bool flag)
{
	if(Enabled != flag)
	{
		Enabled = flag;

		for(int en=0;en<ET_SIZE;en++)
		{
			if(EffectsState[en])
			{
				ShowEffect((EFFECT_TYPE)en, flag?SM_SHOW:SM_HIDE);
				if(!flag)EffectsState[en]=true;
			}
		}

		if(!Enabled)
		{
			if(SoundScheme) SoundScheme->Stop();
		}
	}
}

//=====================================================================================//
//                            void RealEntity::DrawBanner()                            //
//=====================================================================================//
void RealEntity::DrawBanner()
{
	if(BillBoard&&Enabled&&Visible&&CurState!=DEAD)
	{
		point3 pnt(GetLocation()+point3(0,0,1));
		BillBoard->SetPoint(pnt);
	}
}

//=====================================================================================//
//                            void RealEntity::SetBanner()                             //
//=====================================================================================//
void RealEntity::SetBanner(const std::string &banner,unsigned int Color)
{
	if(!BillBoard) BillBoard=new Banner;
	BillBoard->SetText(banner.c_str());
	BillBoard->SetColor(Color);
}

//=====================================================================================//
//                               void RealEntity::Draw()                               //
//=====================================================================================//
void RealEntity::Draw()
{
	if(!Visible || !Normal || !Enabled || !Game::RenderEnabled()) return;
	GraphPipe *Pipe=IWorld::Get()->GetPipe();

	if(IsCameraAttached()) return;

	StatesManager::SetTransform(D3DTRANSFORMSTATE_WORLD,&World);

#ifdef _HOME_VERSION
	Pipe->Chop(Normal->GetMesh(),"Person");
	if(Weapons) Pipe->Chop(Weapons->GetMesh(),"PersonWeapon");
	//drawSpline(m_moveSpline);
#else
	Pipe->Chop(Normal->GetMesh());
	if(Weapons) Pipe->Chop(Weapons->GetMesh());
#endif
}

//=====================================================================================//
//                        void RealEntity::CorrectDirWalking()                         //
//=====================================================================================//
void RealEntity::CorrectDirWalking(float deltaTime)
{
	//ходячий товарищ поворачивается в сторону точки,
	//которую ему укажут. Туда и идет
	Quaternion q;
	Transformation World;

	float DELTANG=WTURNINGSPEED*deltaTime*SpeedMul;
	float adelta;
	if(DestLinked)
	{
		point3 dist(Dest.x,Dest.y,0);
		dist-=Location;dist.z=0;
		if(dist.Length()<DESTFAULT) 
		{DestLinked=false;return;}

		//коррекция направления
		if(Angle>PIm2)Angle-=PIm2;
		if(Angle<0)   Angle+=PIm2;
		float DesiredAngle;

		DesiredAngle=atan2(dist.x,-dist.y);
		if(DesiredAngle<0) DesiredAngle=PIm2+DesiredAngle;
		if(!Visible||!Enabled || !Game::RenderEnabled())
		{
			Angle=DesiredAngle;
			SetLocation(Dest,Angle);
		}
		else
		{

			float delta=(DesiredAngle-Angle);
			adelta=fabs(delta);
			if(adelta>WTURNINGFAULT)
			{
				float er=1.f,ad=adelta>PI?PIm2-adelta:adelta;
				if(ad>TORAD(29)) er=7*(ad-TORAD(29))+1;
				DELTANG*=er;
				if(FastAbs(DELTANG)>adelta) 
				{
					if(DELTANG>0.f) DELTANG=adelta;
					else            DELTANG=-adelta;
				}
				int sign=delta<0?-1:1;
				//if(adelta>PI){ adelta=PIm2-adelta;Angle-=adelta*0.4*sign;}
				//else Angle+=adelta*0.4*sign;
				if(adelta>PI){ Angle-=DELTANG*sign;}
				else Angle+=DELTANG*sign;
				//FIXME:
				if(GetVehicle())
				{
					SetLocation(Dest,Angle);
				}
				SetLocation(Location,Angle);
			}
		}
	}
}

//=====================================================================================//
//                        void RealEntity::CorrectDirTurning()                         //
//=====================================================================================//
void RealEntity::CorrectDirTurning(float deltaTime)
{
	if(DestLinked)
	{

		if(!Visible||!Enabled || !Game::RenderEnabled())
		{ //если поворота не видно - пропустим
			Angle=DesAngle;
			SetLocation(Location,Angle);
			DestLinked=false;
			return;
		}

		point3 _dir = point3(cos(Angle),sin(Angle),0);
		point3 _des_dir = point3(cos(DesAngle),sin(DesAngle),0);
		float cosa=_dir.Dot(_des_dir);
		cosa=std::min(std::max(-1.f,cosa),1.f);
		float ang = acos(cosa);
		if(ang < TORAD(5))
		{
			SetLocation(Location,DesAngle);
			DestLinked=false;
			return;
		}
		point3 cross=_dir.Cross(_des_dir);

		float DELTANG=TURNINGSPEED*deltaTime*SpeedMul;
		float add=std::min(DELTANG,ang);
		if(cross.z<0) add=-add;
		Angle+=add;
		SetLocation(Location,Angle);

	}
	else
	{
		SetAction(STAY,LastUpdate+deltaTime);
	}
}




static const unsigned STICKCASE=LAZY|STAY|HURT
|DEAD
|FALL|USE|KNOCKOUT
|AIMSHOOT|SHOOT|AIMSHOOTRECOIL
|SHOOTRECOIL; //случай отсутствия смещения
static const unsigned TURNINGCASE=WALK|RUN;//случай изменения угла поворота

static const unsigned GOSTAYCASE=LAZY|STAY|HURT|FALL|  //в этой ситуации возврат к стойке
USE|KNOCKOUT|RELOAD/*|AIMSHOOTRECOIL|SHOOTRECOIL*/;
static const unsigned CYCLEDCASE=WALK|RUN|TURN;

//=====================================================================================//
//                            void RealEntity::AnimaOver()                             //
//=====================================================================================//
void RealEntity::AnimaOver(float Time)
{
	if(CurState&GOSTAYCASE)
	{
		//выберем одну из анимаций стойки
		if(CurState==STAY&&rand()<2000)
		{
			m_actMove = false;
			SetAction(LAZY,Time);
		}
		else
		{
			m_actMove = false;
			SetAction(STAY,Time);
		}
		//если уже стояли, то вероятно захотим сыграть lazy
	}
	else if(CurState&CYCLEDCASE) 
	{
		SwitchAnimation(rand(),Time,0.01f);
	}
	//else
	//{
	//	if(Normal) Normal->Update(&m_LastState);
	//	if(Weapons) Weapons->Update(&m_LastState);
	//}
}

//=====================================================================================//
//                         void RealEntity::UpdateVisibility()                         //
//=====================================================================================//
void RealEntity::UpdateVisibility(float Time,GraphPipe *Pipe)
{
	UpdateSkinParms();
	Frustum *f = &Pipe->GetCam()->Cone;
	Visible = (Frustum::NOTVISIBLE != f->TestBBox(SkinBox));
}

//=====================================================================================//
//                                    void invmat()                                    //
//=====================================================================================//
void invmat(D3DMATRIX &a, const D3DMATRIX &b)
{
	point3 Translate(b._41,b._42,b._43);
	point3 right(b._11,b._12,b._13);
	point3    up(b._21,b._22,b._23);
	point3   dir(b._31,b._32,b._33);
	D3DUtil_SetIdentityMatrix(a);
	a._41=-Translate.Dot(right);
	a._42=-Translate.Dot(up);
	a._43=-Translate.Dot(dir);
	a._11=b._11; a._12=b._21; a._13=b._31;
	a._21=b._12; a._22=b._22; a._23=b._32;
	a._31=b._13; a._32=b._23; a._33=b._33;
}

//=====================================================================================//
//                        static point3 XVectorMatrixMultiply()                        //
//=====================================================================================//
static point3 XVectorMatrixMultiply(const D3DMATRIX &mat)
{
	return point3(mat._11, mat._12, mat._13);
}

//=====================================================================================//
//                        static point3 YVectorMatrixMultiply()                        //
//=====================================================================================//
static point3 YVectorMatrixMultiply(const D3DMATRIX &mat)
{
	return point3(mat._21, mat._22, mat._23);
}

//=====================================================================================//
//                        static point3 ZVectorMatrixMultiply()                        //
//=====================================================================================//
static point3 ZVectorMatrixMultiply(const D3DMATRIX &mat)
{
	return point3(mat._31, mat._32, mat._33);
}

//=====================================================================================//
//                          static void getMinMaxComponents()                          //
//=====================================================================================//
__forceinline static void getMinMaxComponents(const point3 &locvec, const BBox &box,
								float &min, float &max)
{
	const float x = locvec.x * (box.maxx - box.minx);
	const float y = locvec.y * (box.maxy - box.miny);
	const float z = locvec.z * (box.maxz - box.minz);

	(x >= 0.0f ? max : min) += x;
	(y >= 0.0f ? max : min) += y;
	(z >= 0.0f ? max : min) += z;
}

//=====================================================================================//
//                         void RealEntity::UpdateSkinParms()                          //
//=====================================================================================//
void RealEntity::UpdateSkinParms()
{
	//Обновим матрицы
	invmat(InvWorld,World);
	//обновим BBox
	LocalSkinBox = Normal->GetBBox();

	const point3 localx = XVectorMatrixMultiply(InvWorld);
	const point3 localy = YVectorMatrixMultiply(InvWorld);
	const point3 localz = ZVectorMatrixMultiply(InvWorld);
	const point3 pos(World._41,World._42,World._43);

	const point3 locorig(LocalSkinBox.minx,LocalSkinBox.miny,LocalSkinBox.minz);
	const point3 start = point3(locorig.Dot(localx),locorig.Dot(localy),locorig.Dot(localz)) + pos;

	SkinBox.minx = start.x;
	SkinBox.miny = start.y;
	SkinBox.minz = start.z;
	SkinBox.maxx = start.x;
	SkinBox.maxy = start.y;
	SkinBox.maxz = start.z;

	getMinMaxComponents(localx,LocalSkinBox,SkinBox.minx,SkinBox.maxx);
	getMinMaxComponents(localy,LocalSkinBox,SkinBox.miny,SkinBox.maxy);
	getMinMaxComponents(localz,LocalSkinBox,SkinBox.minz,SkinBox.maxz);
}

//=====================================================================================//
//                           void RealEntity::UpdateOnTime()                           //
//=====================================================================================//
void RealEntity::UpdateOnTime(float Time)
{ 
	CodeProfiler CF("person.update");

	GraphPipe *Pipe=IWorld::Get()->GetPipe();  
	if(!CurAnima) return;

	// Проверим, не нужно ли совершить первый апдейт
	if(m_firstUpdate)
	{
		m_firstUpdate = false;
		m_LastState = CurAnima->Get(LastPose,0.0f);
		if(Normal) Normal->Update(&m_LastState);
		if(Weapons) Weapons->Update(&m_LastState);
	}

	UpdateVisibility(Time,Pipe);
	float DesUpdFPS = (Visible&&Enabled)?(1.f/(AnimaQual*31.0f)):(1.0f/3.0f);
	if(CurState==DEAD&&LastPose&&Time>LastPose->EndTime) DesUpdFPS=0.3f;
	if((!Visible || !Enabled) && (Time-LastUpdate) < getUpdateDelta()) return;
	//if(Time-LastUpdate<=DesUpdFPS) return;//ограничиваем обновление в зависимости от видимости

	if(LastPose&&Time>LastPose->EndTime) //кончилась анимация
	{
		AnimaOver(Time);
	}

	if(m_mover.get() && !m_mover->update(Time))
	{
		m_actMove = false;
		m_mover.reset();
	}

	if(CurState&TURNINGCASE)//парень поворачивается
	{
		if(!IsMoving())
		{
			CorrectDirWalking(Time-LastUpdate);
		}
	}

	if(CurState&TURN)//парень поворачивается
	{
		CorrectDirTurning(Time-LastUpdate);
	}

	if(IsMoving())
	{
		Location.z = World._43 = IWorld::Get()->GetLevel()->GetGrid()->Height(Location);
	}
	else
	{
		Location.z = World._43 = IWorld::Get()->GetLevel()->GetGrid()->SimpleHeight(Location);
	}

	if((Normal || Weapons) && (Enabled && Visible && Game::RenderEnabled()))
	{
		m_skinLastUpdateTime = Time;
		m_LastState = CurAnima->Get(LastPose,Time);

		if(m_LastState.GetBoneCount()==0)
		{
			throw CASUS(Name+" - проблемы с анимацией!");
		}

		if(Normal) Normal->Update(&m_LastState);
		if(Weapons) Weapons->Update(&m_LastState);
	}

	UpdateSkinParms();
	LastUpdate = Time+(1.f/(2*(AnimaQual*31)))*((float)rand()/32768-0.5);
	// ~~~~~~~~~~~~~ ShadowCaster
	if(shadow && shadow->PoLNeedUpdate(Time))
	{
		Shadows::ShadowAccum::inst()->addShadow(shadow);
	}
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~
	EffectManager *em=IWorld::Get()->GetEffects();
	for(int en=0;en<ET_SIZE;en++)
	{
		point3 pos=GetLocation();
		if(Effects[en])
			em->SetPosition(Effects[en],en==ET_SELECTION?pos:(pos+AXISZ));
	}
	//поддержка звуковой схемы
	if(SoundScheme)
	{
		SoundScheme->UpdatePos(GetLocation());
		if(CurState==DEAD)	SoundScheme->Tick(0,Enabled);
		else					SoundScheme->Tick(Time,Enabled);
	}
}

//=====================================================================================//
//                            float RealEntity::SetAngle()                             //
//=====================================================================================//
float RealEntity::SetAngle(float CurTime, float angle)
{
	if(!Enabled||!Visible|| !Game::RenderEnabled())
	{
		Angle=angle;
		SetLocation(Location,Angle);
		return CurTime;
	}
	else
	{
		point3 _dir = point3(cos(Angle),sin(Angle),0);
		point3 _des_dir = point3(cos(angle),sin(angle),0);
		float cosa=_dir.Dot(_des_dir);
		cosa=std::min(std::max(-1.f,cosa),1.f);
		float ang = acos(cosa);
		if(ang < TORAD(5))
		{
			Angle = angle;
			SetLocation(Location,Angle);
			DestLinked=false;
			return CurTime;
		}

		DesAngle = angle;
		DestLinked=true;
		SetAction(TURN, CurTime);

		return CurTime+ang/TURNINGSPEED/SpeedMul;
	}
}

//=====================================================================================//
//                          void RealEntity::UpdateWorldMat()                          //
//=====================================================================================//
void RealEntity::UpdateWorldMat(float Angle, const point3 &pos)
{
	D3DUtil_SetRotateZMatrix( World, Angle);
	World._41 = pos.x;
	World._42 = pos.y;
	World._43 = pos.z;
}

//=====================================================================================//
//                           void RealEntity::SetLocation()                            //
//=====================================================================================//
void RealEntity::SetLocation(const point3 &_Pos, float angle)
{
	Angle = fmodf(angle,TORAD(360.0f));
	if(!CurAnima) return;
	UpdateWorldMat(Angle,_Pos);

	if(Location!=_Pos)
	{
		Location = _Pos;
		Location.z = World._43=IWorld::Get()->GetLevel()->GetGrid()->Height(Location);
	}
}

//=====================================================================================//
//                          void RealEntity::SetDestination()                          //
//=====================================================================================//
void RealEntity::SetDestination(const point3 &d)
{
	DestLinked = true;
	Dest = d;
}

//=====================================================================================//
//                      float RealEntity::getUpdateDelta() const                       //
//=====================================================================================//
float RealEntity::getUpdateDelta() const
{
	if(IsMoving()) return 0.0f;
	return 0.333333f;
}

//=====================================================================================//
//                             bool RealEntity::TraceRay()                             //
//=====================================================================================//
bool RealEntity::TraceRay(const ray &r, float *Dist, point3 *Norm, bool AsBox)
{
	if(!Normal) return false;
	point3 p,norm;

	if(SkinBox.IntersectRay(r,&p))
	{
		ray localray;
		VectorMatrixMultiply(TODXVECTOR(localray.Direction),TODXVECTOR(r.Direction),InvWorld);
		PointMatrixMultiply(TODXVECTOR(localray.Origin),TODXVECTOR(r.Origin),InvWorld);
		localray.Update();

		if(LocalSkinBox.IntersectRay(localray,&p))
		{
			if(AsBox)
			{
				*Dist=(p-localray.Origin).Length();
				*Norm=AXISZ;
				return true;
			}

			if((Timer::GetSeconds() - m_skinLastUpdateTime) >= getUpdateDelta())
			{
				m_skinLastUpdateTime = Timer::GetSeconds();
				SkState lastState = CurAnima->Get(LastPose,Timer::GetSeconds());
				Normal->Update(&lastState);
			}

			if(Normal->GetMesh()->TraceRay(localray,Dist,&norm))
			{
				VectorMatrixMultiply(*(D3DVECTOR*)Norm,*(D3DVECTOR*)&norm,World);
				return true;
			}
		}
	}
	return false;
}

//=====================================================================================//
//                           bool RealEntity::TraceSegment()                           //
//=====================================================================================//
bool RealEntity::TraceSegment(const point3 &From, const point3 &Dir, point3 *Res, point3 *Norm)
{
	if(!Normal) return false;
	point3 p,norm;
	float dist;
	point3 start_pnt=From,end_pnt=From+Dir,mid_pnt=From+0.5*Dir;
	float Len=Dir.Length()/2; //половина длины сегмента
	if(SkinBox.DistToPoint(mid_pnt)>Len)return false;
	ray localray;
	VectorMatrixMultiply(TODXVECTOR(localray.Direction),
		TODXVECTOR(Dir),InvWorld);
	PointMatrixMultiply(TODXVECTOR(localray.Origin),
		TODXVECTOR(From),InvWorld);
	localray.Update();

	if((Timer::GetSeconds() - m_skinLastUpdateTime) >= getUpdateDelta())
	{
		m_skinLastUpdateTime = Timer::GetSeconds();
		SkState lastState = CurAnima->Get(LastPose,Timer::GetSeconds());
		Normal->Update(&lastState);
	}

	if(Normal->GetMesh()->TraceRay(localray,&dist,&norm) && dist <= 1.0f)
	{
		VectorMatrixMultiply(*(D3DVECTOR*)Norm,*(D3DVECTOR*)&norm,World);
		*Res=From+dist*Dir;
		return true;
	}
	return false;
}

//=====================================================================================//
//                          float RealEntity::PoLSetAction()                           //
//=====================================================================================//
float RealEntity::PoLSetAction(unsigned /*ANIMATION*/ Action,float CurTime)
{
	const unsigned REPENABLE=(TURN|SPECANIM|PANIC|STAY|SHOOTRECOIL|AIMSHOOTRECOIL);
	const unsigned SKIPABLE=(LAZY|HURT|SHOOT|SHOOTRECOIL|FALL|DEAD|USE|RELOAD|AIMSHOOT|AIMSHOOTRECOIL|SPECANIM);
	const unsigned SOUNDHANDLED=(TURN|WALK|RUN|LAZY|DEAD|HURT);

	bool Changed=  (CurState!=Action);

	if(Action==HURT && CurState==FALL)  return CurTime;

	float ret,delay;
	//FIXME:
	if(Action==DEAD) 
	{
		for(int en=0;en<ET_SIZE;en++)
		{
			if(Effects[en])
			{
				ShowEffect((EFFECT_TYPE)en, SM_HIDE);
			}
		}
	}

	if(!Changed && !(CurState&REPENABLE)) return CurTime;
	if(CurState==TURN && DestLinked && !Changed)
	{
		SetLocation(Location,DesAngle);
		DestLinked=false;
	}

	switch(Action)
	{
		case SHOOTRECOIL: delay=0.01f;break;
		case AIMSHOOTRECOIL: delay=0.01f;break;
		case LAZY: case STAY: delay=0.9f;break;
		//case LAZY: case STAY: delay=0.001f;break; // gvozdoder: changed to 1e-3f as requested by inok
		case WALK: case RUN: delay=0.5f;break;
		case TURN:           delay=0.5f;break;
		default: delay=1.5f;
	}

	const bool ShootAfterRecoil = (Action&(SHOOT|AIMSHOOT))&&(CurState&(SHOOTRECOIL|AIMSHOOTRECOIL));
	const bool CanSkip = ((!Enabled || !Visible || !Game::RenderEnabled())&&(Action&SKIPABLE)) || ShootAfterRecoil;

	//CanSkip = CanSkip||( (Action&(SHOOT|AIMSHOOT))&&(CurState&(SHOOTRECOIL|AIMSHOOTRECOIL)) );

	//assert( !IsMoving() || Action==WALK || Action==RUN);
	CurState = Action;

	if(CanSkip)
	{
		SwitchAnimation(rand(),0.0f,0.0f);
		ret = CurTime;
	}
	else
	{
		ret = SwitchAnimation(rand(),CurTime,delay);
		//delay = std::min(ret-CurTime,delay);
	}

	if(SoundScheme && Changed)
	{
		if(CurState&SOUNDHANDLED)
		{
			if(CurState&WALK)HandleSoundEvent(SoundPerson::ET_WALK);
			else if(CurState&TURN)HandleSoundEvent(SoundPerson::ET_WALK);
			else if(CurState&RUN)HandleSoundEvent(SoundPerson::ET_RUN);
			else if(CurState&LAZY)HandleSoundEvent(SoundPerson::ET_LAZY);
			else if(CurState&DEAD)
			{  //если умер - остановить все звуки и проиграть звук смерти
				HandleSoundEvent(SoundPerson::ET_WALK|SoundPerson::ET_STOPIT);
				HandleSoundEvent(SoundPerson::ET_RUN|SoundPerson::ET_STOPIT);
				HandleSoundEvent(SoundPerson::ET_DEATH);
			}
			else if(CurState&HURT)
			{  //если умер - остановить все звуки и проиграть звук смерти
				HandleSoundEvent(SoundPerson::ET_WALK|SoundPerson::ET_STOPIT);
				HandleSoundEvent(SoundPerson::ET_RUN|SoundPerson::ET_STOPIT);
				HandleSoundEvent(SoundPerson::ET_HURT);
			}
			if(!(CurState&LAZY)) HandleSoundEvent(SoundPerson::ET_VEH_AMBIENT|SoundPerson::ET_STOPIT);
		}
		else 
		{
			HandleSoundEvent(SoundPerson::ET_WALK|SoundPerson::ET_STOPIT);
			HandleSoundEvent(SoundPerson::ET_RUN|SoundPerson::ET_STOPIT);
			HandleSoundEvent(SoundPerson::ET_VEH_AMBIENT);
			//HandleSoundEvent(SoundPerson::ET_LAZY|SoundPerson::ET_STOPIT);
		}
	}

	//if((CurState & (AIMSHOOT|SHOOT)) && !CanSkip && CurWeapon != GetBit(GRENADE))//чтобы успел сыграть анимацию прицеливания
	//{
	//	ret += 0.5f; //0.2f;
	//}

	if(!Game::RenderEnabled()) return CurTime;

	return ret;
}

//=====================================================================================//
//                            float RealEntity::SetAction()                            //
//=====================================================================================//
float RealEntity::SetAction(unsigned /*ANIMATION*/ Action,float CurTime)
{
	if(m_actMove) return CurTime;
	return PoLSetAction(Action,CurTime);
}

//=====================================================================================//
//                          void RealEntity::PoLStartMoving()                          //
//=====================================================================================//
void RealEntity::PoLStartMoving(ANIMATION Type, const std::vector<ipnt2_t> &hexes, PoL::MovementController *ctrl)
{
	assert( !IsMoving() );
	m_mover.reset(new PoL::RealEntityMover(Type,this,hexes,ctrl));
	m_actMove = true;
}

//=====================================================================================//
//                           void RealEntity::StartMoving()                            //
//=====================================================================================//
void RealEntity::StartMoving(ANIMATION Type, const std::vector<ipnt2_t> &hexes, PoL::MovementController *ctrl)
{
	if(m_actMove) return;
	assert( !IsMoving() );
	m_mover.reset(new PoL::RealEntityMover(Type,this,hexes,ctrl));
}

//=====================================================================================//
//                            void RealEntity::ShowEffect()                            //
//=====================================================================================//
void RealEntity::ShowEffect(EFFECT_TYPE type, SHOW_MODE mode)
{
	EffectManager *em=IWorld::Get()->GetEffects();
	if(mode==SM_SHOW)
	{
		std::string eff_name;
		point3 pos;
		pos=GetLocation()+point3(0,0,1);
		float rad=0;
		switch(type)
		{
		case ET_FLAME:
			eff_name="hit_small_flame";
			break;
		case ET_SHOCK:
			eff_name="hit_small_flame";
			break;
		case ET_ELECTRIC:
			eff_name="hit_small_flame";
			break;
		case ET_SELECTION:
			pos = GetLocation();
			eff_name="small_selection";
			{
				const float dx=(SkinBox.maxx-SkinBox.minx);
				const float dy=(SkinBox.maxy-SkinBox.miny);
				rad = 0.4f*sqrtf(dx*dx+dy*dy);
				if(rad<0.5f || dx<0.0f || dy<0.0f) rad = 0.5f;
			}
			break;
		default:return;
		}
		if(Enabled)
		{
			if(Effects[type]) em->DestroyEffect(Effects[type]);
			Effects[type]=em->CreateMovableEffect(eff_name,pos,rad);
		}
		EffectsState[type]=true;
	}
	else
	{
		if(type<0||type>=ET_SIZE) return;
		EffectsState[type]=false;
		if(!Effects[type]) return;
		switch(mode)
		{
		case SM_HIDE: 
			em->DestroyEffect(Effects[type]);
			Effects[type]=0;
			break;
		case SM_SLOW_HIDE: 
			em->FinishEffect(Effects[type]);
			Effects[type]=0;
			break;
		}
	}
}

//=====================================================================================//
//                             void RealEntity::DrawFOS()                              //
//=====================================================================================//
void RealEntity::DrawFOS()
{
	if(!Visible||!Enabled||CurState==DEAD|| !Game::RenderEnabled()) return;
	GraphPipe *Pipe=IWorld::Get()->GetPipe();
	float ang1,ang2;
	float r1,r2;
	r1=FOSrad1;r2=FOSrad2;
	if(!r1&&!r2) return;
	ang1=Angle-FOSangle/2;
	ang2=Angle+FOSangle/2;
	static const int PNTNUM=17;
	Primi prim(PNTNUM*2+2);
	Primi::FVFVertex *vts = &prim.Verts[0];
//	point3 pos1[PNTNUM*2+2];
//	texcoord tuv[PNTNUM*2+2];
//	unsigned col[PNTNUM*2+2];
	point3 pnt=GetLocation()+AXISZ;
	int i=0;
	for(;i<PNTNUM;i++)
	{
		float coeff=(float)i/(PNTNUM-1);
		float phase=ang1+coeff*FOSangle;
		/* by Flif float phase2=ang2+coeff*(TORAD(360)-FOSangle);*/
		vts[i].pos = pnt+r2*(sin(phase)*AXISX-cos(phase)*AXISY);
		//pos1[i]=pnt+r2*(sin(phase)*AXISX-cos(phase)*AXISY);
	}
	
	vts[i++].pos = pnt+r1*(sin(ang2)*AXISX-cos(ang2)*AXISY);
	
	for(;i<PNTNUM*2+1;i++)
	{
		float coeff=(float)(i-PNTNUM-1)/(PNTNUM-1);
		/* by Flif float phase=ang1+coeff*FOSangle;*/
		float phase2=ang2+coeff*(TORAD(360)-FOSangle);
		vts[i].pos = pnt+r1*(sin(phase2)*AXISX-cos(phase2)*AXISY);
	}
	vts[i++].pos = pnt+r2*(sin(ang1)*AXISX-cos(ang1)*AXISY);

//	Primi prim;
//	prim.Diffuse=col;
//	prim.Pos=pos1;
//	prim.UVs[0]=tuv;
	prim.Prim=Primi::LINESSTRIP;
	prim.Contents=Primi::NEEDTRANSFORM;
//	prim.VertNum=i;
//	prim.IdxNum=0;
#ifdef _HOME_VERSION
	Pipe->Chop("purewhite",&prim,"Field of sight");
#else
	Pipe->Chop("purewhite",&prim);
#endif
}

//=====================================================================================//
//                              void RealEntity::SetFOS()                              //
//=====================================================================================//
void RealEntity::SetFOS(float front_radius, float back_radius, float sight_angle)
{
	FOSrad1=back_radius;
	FOSrad2=front_radius;
	FOSangle=sight_angle;
}

//=====================================================================================//
//                            float RealEntity::GetAngle()                             //
//=====================================================================================//
float RealEntity::GetAngle()
{
	return Angle;
};

//=====================================================================================//
//                          point3 RealEntity::GetShotPoint()                          //
//=====================================================================================//
point3 RealEntity::GetShotPoint(point3 suppose_point)
{
	point3 pnt,pnt1;
	const unsigned int index = m_shotBoneCount ? m_shotBoneId[rand()%m_shotBoneCount] : 0;

	if(m_shotBoneCount && m_LastState.GetBoneCount() > index)
	{
		pnt = m_LastState.GetBoneEx(index).Trans;
	}
	else if(suppose_point!=NULLVEC)
	{
		pnt = m_LastState.GetNearestBone(suppose_point);
	}
	else
	{
		pnt = m_LastState.GetNearestBone(SkinBox.GetCenter());
	}

	PointMatrixMultiply(TODXVECTOR(pnt1),TODXVECTOR(pnt),World);
	return pnt1;	
}

//=====================================================================================//
//                       point3 RealEntity::GetShotPointNumber()                       //
//=====================================================================================//
point3 RealEntity::GetShotPointNumber(int n)
{
	assert( n < m_shotBoneCount );
	point3 pnt1, pnt = m_LastState.GetBoneEx(m_shotBoneId[n]).Trans;
	PointMatrixMultiply(TODXVECTOR(pnt1),TODXVECTOR(pnt),World);
	return pnt1;	
}

//=====================================================================================//
//                         int RealEntity::GetShotPointCount()                         //
//=====================================================================================//
int RealEntity::GetShotPointCount()
{
	return m_shotBoneCount;
}

//=====================================================================================//
//                       const D3DXMATRIX *RealEntity::GetBone()                       //
//=====================================================================================//
const D3DMATRIX *RealEntity::GetBone(const std::string &bone)
{
	int boneid = CurAnima->GetBoneID(bone);
	if(boneid != -1) return m_LastState.GetBone(boneid);
	return 0;
}

//=====================================================================================//
//                  const SkState::BoneState *RealEntity::GetBoneEx()                  //
//=====================================================================================//
const SkState::BoneState *RealEntity::GetBoneEx(const std::string &bone)
{
	int boneid = CurAnima->GetBoneID(bone);
	if(boneid != -1) return &m_LastState.GetBoneEx(boneid);
	return 0;
}

//=====================================================================================//
//                           void RealEntity::AddShotBone()                            //
//=====================================================================================//
void RealEntity::AddShotBone(const std::string &bone)
{
	m_shotBoneId[m_shotBoneCount] = CurAnima->GetBoneID(bone);
	m_shotBones[m_shotBoneCount++] = bone;
}

//=====================================================================================//
//                        void RealEntity::GetLOSPoints() const                        //
//=====================================================================================//
void RealEntity::GetLOSPoints(points_t *points) const
{
	points->push_back(GetLocation()+1.4*AXISZ);
	points->push_back(const_cast<RealEntity*>(this)->GetShotPoint(points->back()));
	int start=points->size();

	if(LocalSkinBox.minx<LocalSkinBox.maxx)
	{
		points->push_back(point3(LocalSkinBox.minx,LocalSkinBox.miny,LocalSkinBox.minz));
		points->push_back(point3(LocalSkinBox.minx,LocalSkinBox.miny,LocalSkinBox.maxz));
		points->push_back(point3(LocalSkinBox.minx,LocalSkinBox.maxy,LocalSkinBox.minz));
		points->push_back(point3(LocalSkinBox.minx,LocalSkinBox.maxy,LocalSkinBox.maxz));
		points->push_back(point3(LocalSkinBox.maxx,LocalSkinBox.miny,LocalSkinBox.minz));
		points->push_back(point3(LocalSkinBox.maxx,LocalSkinBox.miny,LocalSkinBox.maxz));
		points->push_back(point3(LocalSkinBox.maxx,LocalSkinBox.maxy,LocalSkinBox.minz));
		points->push_back(point3(LocalSkinBox.maxx,LocalSkinBox.maxy,LocalSkinBox.maxz));

		if(LocalSkinBox.maxx-LocalSkinBox.minx>2)
		{
			points->push_back(point3(0.5*(LocalSkinBox.minx+LocalSkinBox.maxx),LocalSkinBox.miny,LocalSkinBox.minz));
			points->push_back(point3(0.5*(LocalSkinBox.minx+LocalSkinBox.maxx),LocalSkinBox.miny,LocalSkinBox.maxz));
			points->push_back(point3(0.5*(LocalSkinBox.minx+LocalSkinBox.maxx),LocalSkinBox.maxy,LocalSkinBox.minz));
			points->push_back(point3(0.5*(LocalSkinBox.minx+LocalSkinBox.maxx),LocalSkinBox.maxy,LocalSkinBox.maxz));
		}

		if(LocalSkinBox.maxy-LocalSkinBox.miny>2)
		{
			points->push_back(point3(LocalSkinBox.minx,0.5*(LocalSkinBox.miny+LocalSkinBox.maxy),LocalSkinBox.minz));
			points->push_back(point3(LocalSkinBox.minx,0.5*(LocalSkinBox.miny+LocalSkinBox.maxy),LocalSkinBox.maxz));
			points->push_back(point3(LocalSkinBox.maxx,0.5*(LocalSkinBox.miny+LocalSkinBox.maxy),LocalSkinBox.minz));
			points->push_back(point3(LocalSkinBox.maxx,0.5*(LocalSkinBox.miny+LocalSkinBox.maxy),LocalSkinBox.maxz));
		}
		if(LocalSkinBox.maxz-LocalSkinBox.minz>2)
		{
			points->push_back(point3(LocalSkinBox.minx,LocalSkinBox.miny,0.5*(LocalSkinBox.minz+LocalSkinBox.maxz)));
			points->push_back(point3(LocalSkinBox.minx,LocalSkinBox.maxy,0.5*(LocalSkinBox.minz+LocalSkinBox.maxz)));
			points->push_back(point3(LocalSkinBox.maxx,LocalSkinBox.miny,0.5*(LocalSkinBox.minz+LocalSkinBox.maxz)));
			points->push_back(point3(LocalSkinBox.maxx,LocalSkinBox.maxy,0.5*(LocalSkinBox.minz+LocalSkinBox.maxz)));
		}
	}

	for(int i = start; i != points->size(); i++)
	{
		PointMatrixMultiply(TODXVECTOR((*points)[i]),TODXVECTOR((*points)[i]),World);
	}
}

//=====================================================================================//
//                           void RealEntity::SetOwnership()                           //
//=====================================================================================//
void RealEntity::SetOwnership(OWNERSHIP ownby)
{
	if(SoundScheme) SoundScheme->SetOwnership(ownby);
}

//=====================================================================================//
//                             void Person::MakeSaveLoad()                             //
//=====================================================================================//
void Person::MakeSaveLoad(SavSlot &sl)
{
	if(sl.IsSaving())
	{ 
		int i;
		sl<<Name;
		sl<<SysName;
		for(i=0;i<SkelData::SUITSNUM+1;i++) sl<<Skels.SkinNames[i];
		sl<<GetLocation()<<Angle;
		sl<<CurState<<CurWeapon<<CurWeapName<<Stand;
		sl<<Visible<<Enabled;
		sl<<FOSrad1<<FOSrad2<<FOSangle; 
		for(i=0;i<ET_SIZE;i++) sl<<EffectsState[ET_SIZE];

		sl << m_shotBoneCount;
		for(i = 0; i < m_shotBoneCount; ++i) sl << m_shotBones[i];
	}
	else
	{
		std::string sk[SkelData::SUITSNUM+1];
		point3 Loc;float Ang;
		unsigned CS,CW,ST;
		std::string weap;
		int i;
		sl>>Name;
		sl>>SysName;
		for(i=0;i<SkelData::SUITSNUM+1;i++) sl>>sk[i];
		sl>>Loc>>Ang;
		sl>>CS>>CW>>weap>>ST;
		Load(Name,sk,SysName);

		SoundPerson *p=SoundScheme;
		SoundScheme=NULL;
		SetAction(CS,0);
		SoundScheme=p;

		SetStanding((STAND)ST,0);
		SetWeapon((WEAPONS)CW,weap,0);
		SetLocation(Loc,Ang);
		sl>>Visible>>Enabled;
		sl>>FOSrad1>>FOSrad2>>FOSangle; 
		for(i=0;i<ET_SIZE;i++) sl>>EffectsState[ET_SIZE];

		int sbc = 0;
		sl >> sbc;
		for(i = 0; i < sbc; ++i)
		{
			std::string bone;
			sl >> bone;
			AddShotBone(bone);
		}

		Enable(Enabled);
		m_firstUpdate = true;
		LastUpdate = 0.0f;
		UpdateOnTime(0.0f);
	}
}

//=====================================================================================//
//                            void Vehicle::MakeSaveLoad()                             //
//=====================================================================================//
void Vehicle::MakeSaveLoad(SavSlot &sl)
{
	if(sl.IsSaving())
	{ 
		int i;
		sl<<Name;
		sl<<GetLocation()<<Angle;
		sl<<CurState;
		sl<<Sounds.Shot<<Sounds.Death<<Sounds.Engine<<Sounds.Walk<<Sounds.Hurt<<Sounds.Lazy;
		sl<<Visible<<Enabled;
		sl<<FOSrad1<<FOSrad2<<FOSangle; 
		for(i=0;i<ET_SIZE;i++) sl<<EffectsState[ET_SIZE];

		sl << m_shotBoneCount;
		for(i = 0; i < m_shotBoneCount; ++i) sl << m_shotBones[i];
	}
	else
	{
		point3 Loc;float Ang;
		unsigned CS;
		std::string weap;
		int i;
		sl>>Name;
		sl>>Loc>>Ang;
		sl>>CS;
		sl>>Sounds.Shot>>Sounds.Death>>Sounds.Engine>>Sounds.Walk>>Sounds.Hurt>>Sounds.Lazy;
		Load(Name,Sounds);

		SoundPerson *p=SoundScheme;
		SoundScheme=NULL;

		SetAction(CS,0);
		SoundScheme=p;

		SetLocation(Loc,Ang);
		sl>>Visible>>Enabled;
		sl>>FOSrad1>>FOSrad2>>FOSangle; 
		for(i=0;i<ET_SIZE;i++) sl>>EffectsState[ET_SIZE];

		if(sl.GetStore()->GetVersion() >= 9)
		{
			int sbc = 0;
			sl >> sbc;
			for(i = 0; i < sbc; ++i)
			{
				std::string bone;
				sl >> bone;
				AddShotBone(bone);
			}
		}

		Enable(Enabled);
		m_firstUpdate = true;
		LastUpdate = 0.0f;
		UpdateOnTime(0.0f);
	}

}

//=====================================================================================//
//                                  Person::Person()                                   //
//=====================================================================================//
Person::Person()
{
	LastUpdate = 0.0f;
	Stand = GetBit(PEACESTAY);
	CurWeapon = GetBit(PISTOL);
	CurState = LAZY;
	Dest = point3(0.0f,0.0f,0.0f);
	DestLinked = false;

	SpeedMul = Options::GetFloat("game.anispeed");
	AnimaQual = Options::GetFloat("game.animaquality");

	int _size=SkelData::WEAPONSNUM*SkelData::MAXANIMANUM*SkelData::ANIMANAMES;
	/* by Flif AnimaLibrary *People=AnimaLibrary::GetInst();*/
	memset(Skels.Animations,0,4*_size*sizeof(SkAnim*));
	memset(Suits,0,SkelData::SUITSNUM*sizeof(SkSkin*));

	SkAnim::SkSpeed = SpeedMul;

	// ~~~~~~~~~~~~~~~~~~~~~ ShadowCaster
	shadow = Shadows::CreateShadow (this);
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
}

//=====================================================================================//
//                                  Person::~Person()                                  //
//=====================================================================================//
Person::~Person()
{
	for(int i=0;i<SkelData::SUITSNUM+1;i++)
		DESTROY(Suits[i]);

	// ~~~~~~~~~~~~~~~~~~~~~ ShadowCaster
	if (shadow) delete shadow;
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

}

//=====================================================================================//
//                             void Person::ParseAnimas()                              //
//=====================================================================================//
//разбор строки, включающей в себя несколько названий, разделенных ','
void Person::ParseAnimas(SkAnim *Animas[],int *Count,std::string &data)
{
	static const std::string MASK=", .;\t";
	static const std::string EXT=".skel";
	int AnimaNames=0;
	std::string::size_type t,start=0;
	static std::string Name;
	while(AnimaNames<SkelData::MAXANIMANUM-1) 
	{
		t=data.find_first_of(MASK,start);
		if(t==data.npos)   t=data.size();
		Name=std::string(data,start,t-start);
		start=t;
		start=data.find_first_not_of(MASK,start);
		if(!Name.empty() && Name[0]!='-')
		{
			Animas[AnimaNames]=AnimaLibrary::GetInst()->GetSkAnimation(PERSONSKEL+Name+EXT);

			if(Animas[AnimaNames])
			{
				AnimaNames++;
			}
			else
			{
				//        logFile["failed_skels.log"]("ошибка загрузки %s в %s\n",Name.c_str(),this->Name.c_str());
			}
		}
		if(start==data.npos) break;
	}
	*Count=AnimaNames;

}

//загрузка из .xls
Person::SkelCache_t Person::SkelsCache;
Vehicle::SkelCache_t Vehicle::SkelsCache;

//=====================================================================================//
//                  const Person::SkelData& Person::FillSkelsCache()                   //
//=====================================================================================//
const Person::SkelData& Person::FillSkelsCache(std::string XlsName)
{
	std::string inp;
	SkelCache_t::iterator it;
	it=SkelsCache.find(XlsName);
	if(it!=SkelsCache.end()) return it->second;

	TxtFile *xls=AnimaLibrary::GetInst()->GetTable(XlsName);
	DataMgr::Release(XlsName.c_str());
	if(!xls->SizeY()) throw CASUS(std::string("Отсутствует таблица описания человека:")+XlsName);
	SkelData data;
	//xls->GetCell(1,0,data.Name);
	data.Name=XlsName;
	for(int stand=0;stand<4;stand++)
		for(int weap=0;weap<SkelData::WEAPONSNUM;weap++)
			for(int anim=0;anim<SkelData::ANIMANAMES;anim++)
			{
				int y;
				int x=1+weap;
				if((1<<anim)==PANIC)//PANIC
				{   y=4; x=10; }
				else if((1<<anim)==SPECANIM)//
				{   y=3; x=10; }
				else
				{   y=4+anim+stand*16; }
				xls->GetCell(y,x,inp);
				data.AnimaCount[stand][anim][weap]=0;
				//logFile["bug.log"]("%d %d %d =>%s,[%s]\n",stand, weap, anim, inp.c_str(),XlsName.c_str());
				ParseAnimas(data.Animations[stand][anim][weap],&data.AnimaCount[stand][anim][weap],inp);
				//logFile["bug.log"]("^^^^^^^^^^^^^^^^^^^^^^^^^\n");

			}
			//logFile["bug.log"]("OK\n");
			SkelsCache[XlsName]=data;
			return SkelsCache[XlsName];
}

//=====================================================================================//
//                 const Vehicle::SkelData& Vehicle::FillSkelsCache()                  //
//=====================================================================================//
const Vehicle::SkelData& Vehicle::FillSkelsCache(std::string name)
{
	enum {BONE1=10,BONE2=12,BARREL1=9,BARREL2=11};
	std::string inp;
	SkelCache_t::iterator it;
	it=SkelsCache.find(name);
	if(it!=SkelsCache.end()) return it->second;

	SkelData data;
	//
	TxtFile *xls=AnimaLibrary::GetInst()->GetTable(VEHXLS);
	DataMgr::Release(VEHXLS.c_str());
	unsigned int y=0;
	xls->FindInCol(name.c_str(),&y,0);
	data.Name=name;
	//читаем анимации
	for(int anim=0;anim<ANIMANAMES;anim++)
	{
		int x;
		x=2+anim;
		xls->GetCell(y,x,inp);
		data.AnimaCount[anim]=0;
		ParseAnimas(data.Animations[anim],&data.AnimaCount[anim],inp);
	}
	xls->GetCell(y,BONE1,data.BarrelBones[0]);
	xls->GetCell(y,BONE2,data.BarrelBones[1]);
	xls->GetCell(y,BARREL1,inp);
	sscanf(inp.c_str(),"%f;%f;%f",&data.BarrelPos[0].x,&data.BarrelPos[0].y,&data.BarrelPos[0].z);
	xls->GetCell(y,BARREL2,inp);
	sscanf(inp.c_str(),"%f;%f;%f",&data.BarrelPos[1].x,&data.BarrelPos[1].y,&data.BarrelPos[1].z);
	//читаем скины
	xls->GetCell(y,1,inp);
	data.SkinName=std::string(VEHICLESKIN+inp+".skin");
	SkelsCache[name]=data;
	return SkelsCache[name];
}

//=====================================================================================//
//                                 void Person::Load()                                 //
//=====================================================================================//
void Person::Load(const std::string &name,const std::string skins[],const std::string &sys_name)
{
	int i;
	//читаем анимации
	Skels=FillSkelsCache(name);
	Name=Skels.Name;
	SysName=sys_name;
	//читаем скины
	for(i=0;i<SkelData::SUITSNUM+1;i++)
	{
		Suits[i]=AnimaLibrary::GetInst()->GetSkSkin(PERSONSKIN+skins[i]+".skin");
		Skels.SkinNames[i]=skins[i];
		if(Suits[i])
		{
			const TexObject *texobj = Suits[i]->GetMesh();
			for(int j = 0; j < texobj->PartNum; ++j)
			{
				VShader *shad = Shell::Instance()->GetGraphPipe()->FindShader(texobj->Parts[j]->MaterialName);
				shad->PrecacheTextures();
			}
		}
	}
	Normal=Suits[0];

	Stand = GetBit(PEACESTAY);
	assert( !IsMoving() );
	CurState = LAZY;
	CurWeapon = GetBit(FREEHANDS);
	SwitchAnimation(0,0,0);
	SkAnim *r=CurAnima;//Skels.Animations[Stand][GetBit(CurState)][CurWeapon][0];
	if(!r) throw CASUS("проблемы со скелетом:"+Name);

	//присобачим все кожи к нашему скелету...
	if(!Normal)
		throw CASUS("на диске нет такой кожи:"+skins[0]);
	try
	{
		Normal->ValidateLinks(CurAnima);
	}
	catch(mll::debug::exception &e)
	{
		std::ostringstream sstr;
		sstr << "Системное имя персонажа <" << SysName << ">: Скелет <" << Name
			<< ">: Кожа <" << skins[0] << ">: " << e.what();
		throw CASUS(sstr.str().c_str());
	}

	Weapons=NULL;

	CurAnima->Start(0);
	SetLocation(point3(17,37,1.2),PId2);

	// ~~~~~~~~~~~~~ ShadowCaster
	if (shadow) shadow->AddObject (Normal->GetMesh());
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~

	SetSoundScheme(SoundsParser::ParseForPerson(sys_name));
}

//=====================================================================================//
//                           float Person::SwitchAnimation()                           //
//=====================================================================================//
float Person::SwitchAnimation(int Anima, float Time,float Delay)
{
	if(!Skels.AnimaCount[Stand][GetBit(CurState)][CurWeapon])
	{
		struct desc {char *Name;int State;};
		desc anims[] = {"LAZY",0x01, "STAY",0x02, "HURT",0x04,"WALK",0x08, "TURN",0x10,
			"SHOOT",0x20,"SHOOTRECOIL",0x40, "DEAD",0x80, "RUN",0x100,
			"FALL",0x200, "USE",0x400, "KNOCKOUT",0x800, "RELOAD",0x1000,
			"AIMSHOOT",0x2000, "AIMSHOOTRECOIL",0x4000, "PANIC",0x8000,
			"SPECANIM",0x10000,"",0};
		desc weaps[] = {"FREEHANDS",0x01, "PISTOL",0x02, "RIFLE",0x04, "AUTOCANNON",0x08, "ROCKET",0x10, "SPECIAL",0x20, "GRENADE",0x40, "CANNON",0x80};
		desc stands[] = {"PEACESTAY",0x01, "WARSTAY",0x02, "SIT",0x04, "SUITSTAY",0x08}; //варианты стоек

#ifdef _HOME_VERSION
		if(CurState == WALK || CurState == RUN)
		{
			std::ostringstream desc;
			desc << SysName << ", " << Name << ": не удалось поставить анимацию: " << anims[GetBit(CurState)].Name
				<< ", оружие: " << weaps[CurWeapon].Name << ", стойка: " << stands[Stand].Name;
			MessageBox(GetForegroundWindow(),desc.str().c_str(),"Animation warning",MB_OK);
		}
#endif

		//		 logFile["failed_skels.log"]("%s: не удалось поставить анимацию:%s, оружие:%d\n",SysName.c_str(),anims[GetBit(CurState)].Name,CurWeapon);
		return Time;
	}

	int _Anima = Anima%Skels.AnimaCount[Stand][GetBit(CurState)][CurWeapon];
	if(Skels.Animations[Stand][GetBit(CurState)][CurWeapon][_Anima])
	{
		SkAnim *NewAnim = Skels.Animations[Stand][GetBit(CurState)][CurWeapon][_Anima];
		if(!LastPose)
		{
			LastPose = new AnimaData;
			CurAnima = NewAnim;
			*LastPose = CurAnima->Start(Time);
		}
		else
		{
			m_LastState = CurAnima->Get(LastPose,Time);
			CurAnima = NewAnim;
			*LastPose = CurAnima->Start(Time,Delay,&m_LastState);
		}
		SetLocation(Location,Angle);
		return LastPose->EndTime;
	}
	return Time;
}

//=====================================================================================//
//                             void Person::SetStanding()                              //
//=====================================================================================//
void Person::SetStanding(STAND Type,float CurTime)
{
	int i=GetBit(Type);
	if(Stand==i) return;
	Stand=i;
	SwitchAnimation(rand(),CurTime,0.5);
}

//=====================================================================================//
//                              void Person::SetWeapon()                               //
//=====================================================================================//
void Person::SetWeapon(WEAPONS Weap, const std::string &WName, float CurTime)
{
	std::string prevWeapName;
	prevWeapName.swap(CurWeapName);
	CurWeapName = WName;
	std::string n = WEAPONSKIN + WName + ".skin";
	int weap = GetBit(Weap);
	const bool weaponChanged = CurWeapon != weap;
	//if(Weapons&&CurWeapon==weap) return; для дальнейшей оптимизации нужно знать еще и имя текущего оружия

	CurWeapon = weap;

	if(weaponChanged)
	{
		if(CurState == SHOOTRECOIL || CurState == AIMSHOOTRECOIL)
		{
			assert( !IsMoving() );
			CurState = LAZY;
		}
		SwitchAnimation(rand(), CurTime, 0.2f);
	}

	DESTROY(Weapons);
	Weapons = AnimaLibrary::GetInst()->GetSkSkin(n);

	if(Weapons) Weapons->ValidateLinks(CurAnima);

	SkState st = CurAnima->Get(LastPose,CurTime);
	if(st.GetBoneCount() != 0)
	{
		if(Normal) Normal->Update(&st);
		if(Weapons) Weapons->Update(&st);
	}

	if(shadow) 
	{
		shadow->Clear();
		shadow->AddObject(Normal->GetMesh());
		if(Weapons) shadow->AddObject(Weapons->GetMesh());
	}

	UpdateSkinParms();
}

//=====================================================================================//
//                              void Person::SetArmour()                               //
//=====================================================================================//
void Person::SetArmour(ARMOUR_TYPE Arm)
{
	if(Suits[Arm])
	{
		if(Normal==Suits[Arm]) return;
		Normal=Suits[Arm];
	}
	Normal->ValidateLinks(CurAnima); 

	SkState st=CurAnima->Get(LastPose,LastUpdate);
	if(st.GetBoneCount()!=0)
	{
		if(Normal)    Normal->Update(&st);
	}


	if (shadow) 
	{
		shadow->Clear();
		shadow->AddObject (Normal->GetMesh());
		if(Weapons) shadow->AddObject (Weapons->GetMesh());
	}

}

//=====================================================================================//
//                              void Person::GetBarrel()                               //
//=====================================================================================//
void Person::GetBarrel(const point3 &Offs, point3 *pos)
{
	*pos=point3(0,0,0);

	if(!CurAnima) return;
	int ID=CurAnima->GetBoneID("Bip01 R Hand");
	const D3DMATRIX *mat=NULL;
	if(ID>=0)  mat=CurAnima->Get(LastPose,Timer::GetSeconds()).GetBone(ID);
	else return;
	point3 o1;
	PointMatrixMultiply(*(D3DVECTOR*)&o1,*(D3DVECTOR*)&Offs,*(D3DMATRIX*)mat);
	PointMatrixMultiply(*(D3DVECTOR*)pos,*(D3DVECTOR*)&o1,World);
	return;
}

//Vehicle
//=====================================================================================//
//                                 Vehicle::Vehicle()                                  //
//=====================================================================================//
Vehicle::Vehicle()
{
	LastUpdate = 0.0f;
	CurState = LAZY;
	Dest = point3(0.0f,0.0f,0.0f);
	DestLinked = false;
	AnimaQual = Options::GetFloat("game.animaquality");
	SpeedMul = Options::GetFloat("game.anispeed");

	shadow = Shadows::CreateShadow(this);
}

//=====================================================================================//
//                                 Vehicle::~Vehicle()                                 //
//=====================================================================================//
Vehicle::~Vehicle()
{
	if(Normal) delete Normal;
	// ~~~~~~~~~~~~~~~~~~~~~ ShadowCaster
	if (shadow) delete shadow;
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
}

//разбор строки, включающей в себя несколько названий, разделенных ','
//=====================================================================================//
//                             void Vehicle::ParseAnimas()                             //
//=====================================================================================//
void Vehicle::ParseAnimas(SkAnim *Animas[],int *Count,std::string &data)
{
	static const std::string MASK=", .;\t";
	static const std::string EXT=".skel";
	int AnimaNames=0;
	std::string::size_type t,start=0;
	static std::string Name;
	while(AnimaNames<MAXANIMANUM-1) 
	{
		t=data.find_first_of(MASK,start);
		if(t==data.npos)   t=data.size();
		Name=std::string(data,start,t-start);
		start=t;
		start=data.find_first_not_of(MASK,start);
		if(Name.size()&&Name[0]!='-')
		{
			Animas[AnimaNames]=AnimaLibrary::GetInst()->GetSkAnimation(VEHICLESKEL+Name+EXT);
			if(Animas[AnimaNames])
			{
				AnimaNames++;
			}
		}
		if(start==data.npos) break;
	}
	*Count=AnimaNames;
}

//загрузка из .xls

//=====================================================================================//
//                                void Vehicle::Load()                                 //
//=====================================================================================//
void Vehicle::Load(const std::string &name,Vehicle::VehSounds &sounds)
{
	Sounds=sounds;
	Skels=FillSkelsCache(name);
	CurBarrel=0;
	//читаем скины
	Name=name;
	Normal=AnimaLibrary::GetInst()->GetSkSkin(Skels.SkinName);
	if(!Normal) throw CASUS("Проблемы с загрузкой техники:\n"+Skels.SkinName);

	//создадим скелет по образу и подобию...
	/* by Flif    SkAnim *r=Skels.Animations[GetBit(CurState)][0];*/
	assert( !IsMoving() );
	CurState = LAZY;
	SwitchAnimation(0, 0,0);
	//присобачим все кожи к нашему скелету...
	Normal->ValidateLinks(CurAnima);  

	if(CurAnima)
		CurAnima->Start(0);
	else throw CASUS("Нет анимации для техники:"+name);
	SetLocation(point3(17,37,1.2),PId2);

	// ~~~~~~~~~~~~~ ShadowCaster
	if (shadow) shadow->AddObject (Normal->GetMesh());
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~
	SetSoundScheme(SoundsParser::ParseForVehicle(&Sounds));
	if(SoundScheme) SoundScheme->m_SysName=name;
}

//=====================================================================================//
//                          float Vehicle::SwitchAnimation()                           //
//=====================================================================================//
float Vehicle::SwitchAnimation(int Anima, float Time,float Delay)
{
	try
	{
		if(!Skels.AnimaCount[GetBit(CurState)])
		{
			return Time;
		}

		int _Anima=Anima%Skels.AnimaCount[GetBit(CurState)];
		if(Skels.Animations[GetBit(CurState)][_Anima])
		{
			SkAnim *NewAnim=Skels.Animations[GetBit(CurState)][_Anima];
			if(!LastPose)
			{
				LastPose = new AnimaData;
				CurAnima=NewAnim;
				*LastPose=CurAnima->Start(Time);
			}
			else
			{
				m_LastState=CurAnima->Get(LastPose,Time);
				CurAnima=NewAnim;
				*LastPose=CurAnima->Start(Time,Delay,&m_LastState);
			}
			SetLocation(Location,Angle);
			return LastPose->EndTime;
		}
		else
		{
			assert( !"Animation not found!" );
		}
	}
	catch(CasusImprovisus &a)
	{
		throw CASUS("Error Setting Skeletons! "+SysName+"["+Name+"]:\n" + a.what());
	}
	return Time;
}

//=====================================================================================//
//                              void Vehicle::GetBarrel()                              //
//=====================================================================================//
void  Vehicle::GetBarrel(const point3 &/*фиктивный параметр*/, point3 *pos)
{
	*pos=point3(0,0,0);

	if(!CurAnima) return;
	int ID=CurAnima->GetBoneID(Skels.BarrelBones[CurBarrel]);
	const D3DMATRIX *mat=NULL;
	if(ID>=0)  mat=CurAnima->Get(LastPose,Timer::GetSeconds()).GetBone(ID);
	else return;
	CurBarrel^=1;
	point3 o1;
	PointMatrixMultiply(*(D3DVECTOR*)&o1,*(D3DVECTOR*)&NULLVEC,*(D3DMATRIX*)mat);
	PointMatrixMultiply(*(D3DVECTOR*)pos,*(D3DVECTOR*)&o1,World);
	return;
}
//* место для тестирования

/**********************************************************************************/
//=====================================================================================//
//                 void SoundPerson::AddEvent() //сообщение о событии                  //
//=====================================================================================//
void SoundPerson::AddEvent(EVENTTYPE type, SoundPerson::EventHandler *ev) //сообщение о событии
{
	if(!ev) return;
	Events_t::iterator it=Events.find(type),ite=Events.end();
	if(it==ite) 
	{
		Events[type]=ev;
	}
	else
	{
		delete it->second;
		it->second=ev;
	}
}

//=====================================================================================//
//                          void SoundPerson::SetOwnership()                           //
//=====================================================================================//
void SoundPerson::SetOwnership(IEntity::OWNERSHIP ownby)
{
	m_OwnBy=ownby;

	//все ниже, для того, чтобы чуваки не говорили много сразу после загрузки.
	Events_t::iterator it=Events.begin(),ite=Events.end();
	Timer::Update();
	float Time=Timer::GetSeconds()+60*10;
	for(;it!=ite;it++)
	{
		it->second->Tick(Time,true);
	}

}

//=====================================================================================//
//                              void SoundPerson::Event()                              //
//=====================================================================================//
void SoundPerson::Event(unsigned type) //сообщение о событии
{
	SoundEventsQueue::Tick(0);
	bool Stop=type&ET_STOPIT?true:false;
	type&=~ET_STOPIT;

	Events_t::iterator it=Events.find((EVENTTYPE)type),ite=Events.end();
	if(it==ite) return;
	if(Stop)     it->second->Stop();
	else
	{
		const EVENTTYPE Type=(EVENTTYPE)type;
		bool TeamMode= (Type==ET_TEAM_COME|| Type==ET_TEAM_DISMISS || Type==ET_TEAM_NEW|| Type==ET_TEAM_LEAVE);
		if(TeamMode)
		{
			SoundEventsQueue::SoundStarted(m_SysName,Type,it->second);
		}
		else if(CanSkipEv(Type))
		{
			it->second->Go();
		}
		else
		{//простое событие
			int pr=it->second->m_Priority;
			if(!m_Current || m_Current->m_Priority < pr )
			{//только если позволяет приоритет
				it->second->Go();
				if(it->second->IsPlaying())	
				{
					if(m_Current) m_Current->Stop();
					m_Current=it->second;
				}
			}
		}
	}
}

//=====================================================================================//
//                              void SoundPerson::Tick()                               //
//=====================================================================================//
void SoundPerson::Tick(float Time, bool Enabled)
{
	bool watchdog=!Enabled;
	EVENTTYPE type;

	SoundEventsQueue::Tick(0);

	if(m_Current && !m_Current->IsPlaying()) m_Current=NULL;

	Events_t::iterator it=Events.begin(),ite=Events.end();
	for(;it!=ite;it++)
	{
		type=it->first;
		bool walking=CanSkipEv(type);
		int pr=it->second->m_Priority;
		int pr1=(m_Current?m_Current->m_Priority:0);

		if(it->second->is2d() && m_OwnBy!=IEntity::OS_PLAYER) 		watchdog=true; //2D звуки должны играть только члены команды
		if(!walking&&pr<pr1)	watchdog=true; // и только если можно

		it->second->Tick(Time,watchdog);


		if(it->second->IsPlaying()&&!walking && !watchdog && m_Current!=it->second)		//если звук начал играться - оповестим об этом очередь
		{
			if(m_Current) m_Current->Stop();
			m_Current=it->second;
		}

	}
}

//=====================================================================================//
//                 void SoundPerson::UpdatePos() //обновление позиции                  //
//=====================================================================================//
void SoundPerson::UpdatePos(const point3 &pos) //обновление позиции
{
	Events_t::iterator it=Events.begin(),ite=Events.end();
	for(;it!=ite;it++)
		it->second->UpdatePos(pos);
}

//=====================================================================================//
//                              void SoundPerson::Stop()                               //
//=====================================================================================//
void SoundPerson::Stop() 
{
	Events_t::iterator it=Events.begin(),ite=Events.end();
	for(;it!=ite;it++)
		it->second->Stop();
}

//=====================================================================================//
//                             SoundPerson::~SoundPerson()                             //
//=====================================================================================//
SoundPerson::~SoundPerson()
{
	Stop();
	//удостовериться, что в звуковой очереди не осталось ссылок на меня
	SoundEventsQueue::PersonDied(m_SysName);
	SoundEventsQueue::Tick(0);

	Events_t::iterator it=Events.begin(),ite=Events.end();
	for(;it!=ite;it++)
		delete it->second;
	Events.clear();
}

//=====================================================================================//
//                        void SoundPerson::EventHandler::Go()                         //
//=====================================================================================//
void SoundPerson::EventHandler::Go()
{
	if(m_PlayingType!=T_BYPERCENT)
	{
#ifdef CHECK_SOUNDS
		logFile["person_sounds.log"]("из-за того, что не вероятностный ");
#endif
		return;
	}
	float p=frand();
	if(p<m_Percent)
	{
		int num=rand()%m_SndNames.size();
#ifdef CHECK_SOUNDS
		logFile["person_sounds.log"](" \"%s\" ",m_SndNames[num].c_str());
#endif

		if(m_Is2d)  m_Player.Play2d(m_SndNames[num],m_Cycled);
		else        m_Player.Play3d(m_SndNames[num],m_Cycled);
	}
#ifdef CHECK_SOUNDS
	else
	{
		logFile["person_sounds.log"]("из-за вероятности ");
	}
#endif
}

//=====================================================================================//
//                       void SoundPerson::EventHandler::Tick()                        //
//=====================================================================================//
void SoundPerson::EventHandler::Tick(float Time, bool ClearWatchdog)
{
	//return ;//fixme
	if(m_PlayingType!=T_BYFREQ) return;
	if(ClearWatchdog)
	{
		m_LastBeat=Time+m_Frequency*(0.5+frand());
		return ;
	}
	if(Time<=m_LastBeat) return;
	if(frand()*m_Frequency<Time-m_LastBeat)
	{
		//Console::AddString("");
		int sz=m_SndNames.size();
		int num=rand()%sz;
		while(m_LastPlayed!=-1 && sz!=1 && num==m_LastPlayed)	num=rand()%sz;

		if(m_Is2d)  m_Player.Play2d(m_SndNames[num],m_Cycled);
		else        m_Player.Play3d(m_SndNames[num],m_Cycled);
		m_LastBeat+=m_Frequency*(0.9+0.2*frand());
		m_LastPlayed = num;
	}
}

//=====================================================================================//
//                          void SoundsParser::ParseString()                           //
//=====================================================================================//
void SoundsParser::ParseString(const std::string &in,
							   const std::string &fieldname,
							   SoundsParser::wave_list_t *str
							   )
{
	static const std::string NAME_PREFIX("sounds/units/");
	std::string::size_type it=0;
	std::string::size_type t=0;
	std::string data(in),Name;
	str->parm=0;
	str->names.clear();
	/*
	шаблон
	fieldname fname,fname,fname = number;fieldname fname,fname,fname = number%;
	*/
	//поиск в строке указанного поля
	if(int sz=fieldname.size())
	{
		t=data.find(fieldname); 
		if(t==data.npos) return;
		t+=sz;
		it=t;
	}
	//выцепление имен файлов  
	do{
		t=data.find_first_of(",=",it);
		if(t==data.npos) return;
		str->names.push_back(NAME_PREFIX + KillSpaces(std::string(data,it,t-it)) + ".wav");
		it=t+1;
		if(data[t]=='=') break;
	}while(true);
	//выцепление параметра
	t=data.find_first_of(";%",it);
	Name=std::string(data,it,t-it);
	//уберем все пробелы

	str->parm=atof(Name.c_str());
}

//=====================================================================================//
//                            void SoundsParser::SetLevel()                            //
//=====================================================================================//
void SoundsParser::SetLevel(const std::string &Name)
{
	//в зависимости от уровня меняются звуки ходьбы и бега
	CurLevel=Name;
}

//=====================================================================================//
//                            void SoundsParser::Precache()                            //
//=====================================================================================//
void SoundsParser::Precache()
{
	static const std::string XlsName="scripts/species/sounds.txt";
	static const std::string XlsLevels="scripts/levels.txt";
	TxtFile *xls=AnimaLibrary::GetInst()->GetTable(XlsName);
	if(xls)
	{
		DataMgr::Release(XlsName.c_str());
		for(int i=1;i<xls->SizeX(0);i++)
		{
			std::string sysname;
			xls->GetCell(0,i,sysname);
			for(int j=1;j<xls->SizeY();j++)
			{
				std::string event_name,data;
				xls->GetCell(j,0,event_name);
				xls->GetCell(j,i,data);
				if(data.size())
					EventCache[sysname][event_name]=data;
			}
		}
	}

	TxtFile *xlsl=AnimaLibrary::GetInst()->GetTable(XlsLevels);
	if(xlsl)
	{
		DataMgr::Release(XlsLevels.c_str());
		for(int j=1;j<xlsl->SizeY();j++)
		{
			std::string levelname,data;
			xlsl->GetCell(j,0,levelname);
			if(!levelname.size()) break;
			xlsl->GetCell(j,21,data);
			LevelSteps[levelname]=data;
		}
	}
	CacheLoaded=true;
}

//=====================================================================================//
//                        std::string *SoundsParser::GetEvent()                        //
//=====================================================================================//
std::string *SoundsParser::GetEvent(const std::string &sys_name, const std::string &event)
{
	StrMap_t::const_iterator ev;//событие->данные
	SysNameMap_t::const_iterator person;//человек->набор событий

	person=EventCache.find(sys_name);
	if(person==EventCache.end()) return NULL;
	ev=EventCache[sys_name].find(event);
	if(ev==EventCache[sys_name].end()) return NULL;
	return &EventCache[sys_name][event];
}

//=====================================================================================//
//                     SoundPerson *SoundsParser::ParseForPerson()                     //
//=====================================================================================//
SoundPerson *SoundsParser::ParseForPerson(const std::string &Name)
{
	static const SoundPerson::EventHandler::TYPE PERCENT=SoundPerson::EventHandler::T_BYPERCENT;
	static const SoundPerson::EventHandler::TYPE FREQ=SoundPerson::EventHandler::T_BYFREQ;
	static const struct{
		char sysname[30];
		char fieldname[30]; 
		SoundPerson::EventHandler::TYPE playtype; 
		SoundPerson::EVENTTYPE  ev;
		bool cycled;
		bool is2d;
	}
	FIELD_NAMES[]={
		{"random sound","2d:",FREQ,SoundPerson::ET_RANDOM2D,false,true},
		{"random sound","3d:",FREQ,SoundPerson::ET_RANDOM3D,false,false},
		{"lazy","",PERCENT,SoundPerson::ET_LAZY,false,false},
		//{"walk","",PERCENT,SoundPerson::ET_RANDOM,false,false},
		//{"run","",PERCENT,SoundPerson::ET_RANDOM,false,false},
		{"hurt","",PERCENT,SoundPerson::ET_HURT,false,false},
		{"critical hurt","",PERCENT,SoundPerson::ET_CHURT,false,false},
		{"lucky strike","",PERCENT,SoundPerson::ET_LUCKYSTRIKE,false,true},
		{"death","",PERCENT,SoundPerson::ET_DEATH,false,false},
		{"enemy sighted","",PERCENT,SoundPerson::ET_ENEMYSIGHTED,false,true},
		{"pretty girl","",PERCENT,SoundPerson::ET_GIRL,false,true},
		{"selection","",PERCENT,SoundPerson::ET_SELECTION,false,true},
		{"move command","",PERCENT,SoundPerson::ET_MOVE,false,true},
		{"attack command","",PERCENT,SoundPerson::ET_ATTACK,false,true},
		{"critical miss","",PERCENT,SoundPerson::ET_MISS,false,true},
		{"under fire","",PERCENT,SoundPerson::ET_UNDERFIRE,false,true},
		{"treatment","low:",PERCENT,SoundPerson::ET_TREATMENT_LOW,false,true},
		{"treatment","med:",PERCENT,SoundPerson::ET_TREATMENT_MED,false,true},
		{"treatment","high:",PERCENT,SoundPerson::ET_TREATMENT_HIGH,false,true},
		{"teammate","come:",PERCENT,SoundPerson::ET_TEAM_COME,false,true},
		{"teammate","dismiss:",PERCENT,SoundPerson::ET_TEAM_DISMISS,false,true},
		{"teammate","new:",PERCENT,SoundPerson::ET_TEAM_NEW,false,true},
		{"teammate","leave:",PERCENT,SoundPerson::ET_TEAM_LEAVE,false,true},
		{"enemy died","2d:",PERCENT,SoundPerson::ET_ENEMYDIED,false,true},
		{"enemy died","3d:",PERCENT,SoundPerson::ET_ENEMYDIED,false,false},
		{"enemy exploded","2d:",PERCENT,SoundPerson::ET_ENEMY_EXPLODED,false,true},
		{"enemy exploded","3d:",PERCENT,SoundPerson::ET_ENEMY_EXPLODED,false,false},
		{"open failed","",PERCENT,SoundPerson::ET_OPEN_FAILED,false,true},
		{"open succeed","",PERCENT,SoundPerson::ET_OPEN_OK,false,true},
		{"car drive failed","",PERCENT,SoundPerson::ET_CAR_FAILED,false,true},
		{"levelup","",PERCENT,SoundPerson::ET_LEVELUP,false,true},
		{"got rocket launcher","",PERCENT,SoundPerson::ET_GOT_RLAUNCHER,false,true},
		{"got autocannon","",PERCENT,SoundPerson::ET_GOT_AUTOCANNON,false,true},
		{"got shotgun","",PERCENT,SoundPerson::ET_GOT_SHOTGUN,false,true},
		{"got energy gun","",PERCENT,SoundPerson::ET_GOT_ENERGYGUN,false,true},
		{"using plasma grenade","",PERCENT,SoundPerson::ET_USE_PLASMA_GREN,false,true},      
		{"saw termoplasma exlosion","",PERCENT,SoundPerson::ET_SAW_BIGBANG,false,true},
		{"no line of fire","",PERCENT,SoundPerson::ET_NO_LOF,false,true},
		{"","",PERCENT,SoundPerson::ET_OPEN_OK,false,false},
		{"","walk:",PERCENT,SoundPerson::ET_WALK,true,false},
		{"","run:",PERCENT,SoundPerson::ET_RUN,true,false},
		{"","",PERCENT,SoundPerson::ET_SCANNER,true,false}
	};


	if(!CacheLoaded) Precache();

	SoundPerson *Scheme=new SoundPerson;
	Scheme->m_SysName=Name;
	wave_list_t waves;

	int num = 0;
	const int fncount = sizeof(FIELD_NAMES)/sizeof(FIELD_NAMES[0]);

	for(num = 0; ; ++num)
	{
		if(FIELD_NAMES[num].sysname[0]==0) break;

		std::string *str=GetEvent(Name,FIELD_NAMES[num].sysname);
		if(!str) continue;

		ParseString(*str,FIELD_NAMES[num].fieldname,&waves);
		if(waves.names.size())
		{
			SoundPerson::EventHandler *ev=new SoundPerson::EventHandler;
			ev->m_Cycled=FIELD_NAMES[num].cycled;
			ev->m_Is2d=FIELD_NAMES[num].is2d;
			ev->m_SndNames=waves.names;
			ev->m_Percent=waves.parm/100.f;
			ev->m_Frequency=waves.parm;
			ev->m_PlayingType=FIELD_NAMES[num].playtype;
			std::string *str=GetEvent("Priority",FIELD_NAMES[num].sysname);
			ev->m_Priority=atoi(str->c_str());

			Scheme->AddEvent(FIELD_NAMES[num].ev,ev);
		}
	}

	StrMap_t::const_iterator lvl = LevelSteps.find(CurLevel); //событие->данные

	++num;
	if(lvl != LevelSteps.end())
	{
		int i=num;
		for(;i<num+2;i++)
		{
			ParseString(lvl->second,FIELD_NAMES[i].fieldname,&waves);
			if(waves.names.size())
			{
				SoundPerson::EventHandler *ev=new SoundPerson::EventHandler;
				ev->m_Cycled=FIELD_NAMES[i].cycled;
				ev->m_Is2d=FIELD_NAMES[i].is2d;
				ev->m_SndNames=waves.names;
				ev->m_Percent=2;
				ev->m_Frequency=0;
				ev->m_PlayingType=FIELD_NAMES[i].playtype;
				ev->m_Priority=100;
				Scheme->AddEvent(FIELD_NAMES[i].ev,ev);
			}
		}
		num=i;
	}
	else	
	{
		num = num+2;
	}
	{//сканнер
		ParseString("termoscanner=100%",FIELD_NAMES[num].fieldname,&waves);
		SoundPerson::EventHandler *ev=new SoundPerson::EventHandler;
		ev->m_Cycled=FIELD_NAMES[num].cycled;
		ev->m_Is2d=FIELD_NAMES[num].is2d;
		ev->m_SndNames=waves.names;
		ev->m_Percent=2;
		ev->m_Frequency=0;
		ev->m_PlayingType=FIELD_NAMES[num].playtype;
		ev->m_Priority=100;
		Scheme->AddEvent(FIELD_NAMES[num].ev,ev);
	}
	return Scheme;
}

//=====================================================================================//
//                    SoundPerson *SoundsParser::ParseForVehicle()                     //
//=====================================================================================//
SoundPerson *SoundsParser::ParseForVehicle(Vehicle::VehSounds *Sounds)
{
	static const SoundPerson::EventHandler::TYPE PERCENT=SoundPerson::EventHandler::T_BYPERCENT;

	SoundPerson *Scheme=new SoundPerson;
	Scheme->m_SysName="";
	SoundPerson::EventHandler *ev=new SoundPerson::EventHandler;
	ev->m_Cycled=true; ev->m_Is2d=false;
	ev->m_SndNames.push_back(Sounds->Walk);
	ev->m_Percent=100;  ev->m_Frequency=0;  ev->m_PlayingType=PERCENT;
	Scheme->AddEvent(SoundPerson::ET_WALK,ev);

	ev=new SoundPerson::EventHandler;
	ev->m_Cycled=false;  ev->m_Is2d=false;
	ev->m_SndNames.push_back(Sounds->Hurt);
	ev->m_Percent=100;    ev->m_Frequency=0;  ev->m_PlayingType=PERCENT;
	Scheme->AddEvent(SoundPerson::ET_HURT,ev);

	ev=new SoundPerson::EventHandler;
	ev->m_Cycled=false;  ev->m_Is2d=false;
	ev->m_SndNames.push_back(Sounds->Death);
	ev->m_Percent=100;    ev->m_Frequency=0;  ev->m_PlayingType=PERCENT;
	Scheme->AddEvent(SoundPerson::ET_DEATH,ev);

	ev=new SoundPerson::EventHandler;
	ev->m_Cycled=true;  ev->m_Is2d=false;
	ev->m_SndNames.push_back(Sounds->Engine);
	ev->m_Percent=100;    ev->m_Frequency=0;  ev->m_PlayingType=PERCENT;
	Scheme->AddEvent(SoundPerson::ET_VEH_AMBIENT,ev);

	ev=new SoundPerson::EventHandler;
	ev->m_Cycled=false;  ev->m_Is2d=false;
	ev->m_SndNames.push_back(Sounds->Lazy);
	ev->m_Percent=100;    ev->m_Frequency=0;  ev->m_PlayingType=PERCENT;
	Scheme->AddEvent(SoundPerson::ET_LAZY,ev);
	return Scheme;
}

/**********************************************************************************/


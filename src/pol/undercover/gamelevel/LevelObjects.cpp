// LevelObjects.cpp: implementation of the DynObject class.
//
//////////////////////////////////////////////////////////////////////
#include "precomp.h"

#include "Grid.h"
#include "Station.h"
#include "LevelObjects.h"
#include "LevelToLogic.h"
#include <Muffle/ISound.h>
#include <Common/Utils/OptSlot.h>
#include <Common/CmdLine/CmdLine.h>
#include <Common/SaveLoad/SaveLoad.h>

bool NamedEffect::MakeSaveLoad(SavSlot &slot)
{
	if(slot.IsSaving())
	{
		slot<<Name;
		slot<<Position<<Front<<Up<<Right<<Color;
	}
	else
	{
		slot>>Name;
		slot>>Position>>Front>>Up>>Right>>Color;
	}
	return true;
}

void NamedEffect::Save(FILE *f)
{
	fprintf(f,"Name:");
	SaveString(f,Name);
	fprintf(f,"Pos:%f,%f,%f\n",Position.x,Position.y,Position.z);
	fprintf(f,"Front:%f,%f,%f\n",Front.x,Front.y,Front.z);
	fprintf(f,"Up:%f,%f,%f\n",Up.x,Up.y,Up.z);
	fprintf(f,"Right:%f,%f,%f\n",Right.x,Right.y,Right.z);
	fprintf(f,"Color:%f,%f,%f\n",Color.x,Color.y,Color.z);
}

void NamedEffect::Load(FILE *f)
{
	fscanf(f,"Name:");
	LoadString(f,&Name);
	fscanf(f,"Pos:%f,%f,%f\n",&Position.x,&Position.y,&Position.z);
	fscanf(f,"Front:%f,%f,%f\n",&Front.x,&Front.y,&Front.z);
	fscanf(f,"Up:%f,%f,%f\n",&Up.x,&Up.y,&Up.z);
	fscanf(f,"Right:%f,%f,%f\n",&Right.x,&Right.y,&Right.z);
	fscanf(f,"Color:%f,%f,%f\n",&Color.x,&Color.y,&Color.z);
}

void NamedEffect::LoadAlfa(const mll::utils::named_vars &vars,
						   const mll::algebra::matrix3 &mtr)
{
	Name = vars["Effect"].value();
	Position = mll::algebra::point3(0.5f,0.5f,0.5f)*mtr;
	Front = mll::algebra::point3(0.0f,0.0f,1.0f)*mtr;
	Up = mll::algebra::point3(0.0f,1.0f,0.0f)*mtr;
	Right = mll::algebra::point3(1.0f,0.0f,0.0f)*mtr;
	Color = vars.get_default("color",mll::algebra::vector3(255.0f,255.0f,255.0f));
}

bool NamedSound::MakeSaveLoad(SavSlot &slot)
{
	if(slot.IsSaving())
	{

		slot<<Name;
		slot<<Type<<Freq;
		slot<<Pos<<Vel;
	}
	else
	{
		slot>>Name;
		slot>>Type>>Freq;
		slot>>Pos>>Vel;

		ParseSounds();
		LastSoundNum=0;
	}
	return true;
}

void NamedSound::LoadAlfa(const mll::utils::named_vars &vars, const mll::algebra::matrix3 &mtr)
{
	using mll::utils::named_vars;

	if(vars.exists("Sounds"))
	{
		Name = vars["Sounds"].value();
	}
	else if(vars.exists("Sound"))
	{
		Name = vars["Sound"].value();
	}
	else if(vars.exists("sounds"))
	{
		Name = vars["sounds"].value();
	}
	else if(vars.exists("sound"))
	{
		Name = vars["sound"].value();
	}

	//Name = vars["Sound"].value();
	std::string type = vars["Type"].value();
#ifdef _HOME_VERSION
	if(!CmdLine::IsKey("-allcycled"))
	{
#endif
		if(!stricmp(type.c_str(),"random")) Type = NS_RANDOM;
		else if(!stricmp(type.c_str(),"random2d")) Type = NS_RANDOM|NS_STATIC;
		else if(!stricmp(type.c_str(),"random2d")) Type = NS_CYCLED|NS_STATIC;
		else if(!stricmp(type.c_str(),"cycled2d")) Type = NS_CYCLED|NS_STATIC;
		else Type = NS_STATIC;
#ifdef _HOME_VERSION
	}
	else
	{
		if(!stricmp(type.c_str(),"random")) Type = NS_CYCLED;
		else if(!stricmp(type.c_str(),"cycled")) Type = NS_CYCLED;
		else if(!stricmp(type.c_str(),"random2d")) Type = NS_CYCLED|NS_STATIC;
		else if(!stricmp(type.c_str(),"cycled2d")) Type = NS_CYCLED|NS_STATIC;
		else Type = NS_STATIC;
	}
#endif
	Freq = vars.get_default("Frequency",1.0f);
	Pos = ::point3(mll::algebra::point3(0.5f,0.5f,0.5f)*mtr);
	Vel = ::point3(0.0f,0.0f,0.0f);
	ParseSounds();
}

void NamedSound::ParseSounds()
{
	static const std::string MASK("##");
	std::string::size_type st=0,fn,e=Name.npos;
	bool ex=false;
	do
	{
		fn=Name.find(MASK,st);
		if(fn==e) 
		{
			fn=Name.size()-st;
			ex=true;
		}

		std::string hh=Name.substr(st,fn-st);
		if(hh.size())		Names.push_back(hh);

		st=fn+MASK.size();
	}
	while(!ex && st<Name.size());

}
const std::string& NamedSound::GetNextName()
{
	if(!Names.size()) return Name;

	int num=rand()%Names.size();
	while(Names.size()>1 && num==LastSoundNum)
		num=rand()%Names.size();
	LastSoundNum=num;
	return Names[num];
}

void NamedSound::Save(FILE *f)
{
	fprintf(f,"Name:");
	SaveString(f,Name);
	fprintf(f,"Props:%d %f\n",&Type,&Freq);
	fprintf(f,"Pos:%f,%f,%f\n",Pos.x,Pos.y,Pos.z);
	fprintf(f,"Vel:%f,%f,%f\n",Vel.x,Vel.y,Vel.z);
}

void NamedSound::Load(FILE *f)
{
	fscanf(f,"Name:");
	LoadString(f,&Name);
	fscanf(f,"Props:%d %f\n",Type,Freq);
	fscanf(f,"Pos:%f,%f,%f\n",&Pos.x,&Pos.y,&Pos.z);
	fscanf(f,"Vel:%f,%f,%f\n",&Vel.x,&Vel.y,&Vel.z);
}

//тестирование нового подхода к динамическим объектам
class DynamicObject
{
public:
	class LogicProps;
	class AnimProp;
	class GeomProp;
	class UseOpt;
public:
	void EndTurn(unsigned Smth);
	void SwitchState(float St);
	void Update(float Time);
	const LogicProps& GetLogicProps() const;
	const BBox& GetBBox();
	bool TraceRay(const ray &r, float *Pos, point3 *Norm);
	bool Damage(unsigned damage_type, float value);
public:
	DynamicObject():LogicProperty(NULL),AnimationProperty(NULL),Shape(NULL),SoundProperty(NULL){};
	~DynamicObject();

	DynamicObject& operator=(const DynamicObject &a);
	DynamicObject(const DynamicObject &a);
private:
	class SoundProp;
	class CrushType;
private:
	LogicProps *LogicProperty;
	AnimProp   *AnimationProperty;
	GeomProp   *Shape;
	SoundProp  *SoundProperty;

};

class DynamicObject::UseOpt
{
public:
	bool IsDataBase() const {return isDB;};
	bool IsStorage() const {return isStorage;};
	//узнать символьную метку ящика
	const std::string& GetName() const {return ObjName;};
	//узнать название набора предметов
	const std::string& GetItems() const {return SetName;};    
	//каким ключом можно открыть
	const std::string& WhichKey() const {return UnlockKey;};    

	int GetWisdom() const {return Wisdom;};
	int GetWeight() const {return Weight;};
	int GetRespawn() const {return Respawn;};
	int GetExperience() const {return EXP;};


private:
	int Wisdom,Weight,Respawn,EXP;
	std::string SetName;
	std::string ObjName;
	bool isStorage,isDB;
	std::string UnlockKey;    //каким ключом можно открыть

public:
	UseOpt():Wisdom(0),Weight(0),Respawn(0),EXP(0),isStorage(false),isDB(false){};

};

class DynamicObject::SoundProp
{
};
class DynamicObject::CrushType
{
public:
	enum DESTRTYPE{NONE=0x0000,GLASS=0x0001,BLAST=0x0002, METAL=0x0004,WOOD=0x0008,STONE=0x0010,
		CARTON=0x0020, BLACKSMOKE=0x1000, WHITESMOKE=0x2000,    SPARKLES=0x4000, FIRE=0x8000};
	unsigned Destruction;
	unsigned PrimaryType;
	unsigned SecondaryType;
	float PrimaryDamage;
	float SecondaryDamage;
	CrushType():Destruction(NONE),PrimaryDamage(0),SecondaryDamage(0){};
};

class DynamicObject::LogicProps
{
public:
	float GetState() const;
	const UseOpt* GetUseOpt() const {return m_UseOptions;};
	bool Damage(unsigned damage_type, float value);
public://оповещения
	void StateChanged(float State);
	void EndTurn();

public:
	LogicProps():m_Crush(NULL), m_Station(NULL), m_UseOptions(NULL){};
	LogicProps(const LogicProps &a);
	LogicProps& operator=(const LogicProps &a);

	~LogicProps();
protected:
	CrushType *m_Crush;
	Station   *m_Station;
	UseOpt    *m_UseOptions;
};
class DynamicObject::AnimProp
{ 
public:
	void Init();
	void Update(DynamicObject *Parent);//ссылка на объект для оповещения об изменении состояния
	float SwitchState(float cur_time,float state);//возвращает время, когда анимация закончится
	float GetState() const;
	const D3DMATRIX& GetWorld() const;
private:
	KeyAnimation Anima;
	bool Animated; //объект постоянно анимируется
	bool Animation;//флаг - в данный момент идет анимация
	float LastTime;
	//более высокий интерфейс
	float State;//коэффициент положения анимации 0 - начало 1 - конец
	float EndState; //коэффициент - к которому стремится анимация
	float StartTime;//Время, когда начался переход между состояниями
};
class DynamicObject::GeomProp
{
public:
	const TexObject* GetShape() const{return &m_Shape;};
private:
	ShapePool::Handle m_MyShape;
	TexObject m_Shape;
};




//часть реализации
DynamicObject::LogicProps::LogicProps(const LogicProps &a)
{
	LogicProps();
	*this=a;
}
DynamicObject::LogicProps& DynamicObject::LogicProps::operator=(const LogicProps &a)
{
	if(!m_Crush) m_Crush=new CrushType;
	if(!m_Station) m_Station= new Station;
	if(!m_UseOptions) m_UseOptions=new UseOpt;
	*m_Crush=*a.m_Crush;
	*m_Station=*a.m_Station;
	*m_UseOptions=*a.m_UseOptions;
	return *this;
}
DynamicObject::LogicProps::~LogicProps()
{
	delete m_Crush;
	delete m_Station;
	delete m_UseOptions;
}
DynamicObject& DynamicObject::operator=(const DynamicObject &a)
{
	if(!LogicProperty)  LogicProperty = new LogicProps;
	*LogicProperty=*a.LogicProperty;
	if(!AnimationProperty)  AnimationProperty = new AnimProp;
	*AnimationProperty=*a.AnimationProperty;
	if(!Shape)  Shape = new GeomProp;
	*Shape=*a.Shape;
	if(!SoundProperty)  SoundProperty = new SoundProp;
	*SoundProperty=*a.SoundProperty;
	return *this;
}
DynamicObject::DynamicObject(const DynamicObject &a)
{
	DynamicObject();
	*this=a;
}
DynamicObject::~DynamicObject()
{
	delete LogicProperty;
	delete AnimationProperty;
	delete Shape;
	delete SoundProperty;
}

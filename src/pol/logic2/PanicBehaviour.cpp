/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: модели поведения человека в панике
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                
#pragma warning(disable:4786)

#include "logicdefs.h"

#include "PanicBehaviour.h"
#include "entity.h"
#include "Activity.h"
#include "AIAPI.h"
#include "PathUtils.h"
#include "Graphic.h"
#include "DirtyLinks.h"

////////////////////////////////////////////////////////////////////////////////
//
// поведение в шоке
//
////////////////////////////////////////////////////////////////////////////////
class ShockBehaviour : public PanicBehaviour
{
	float m_startTime;
	bool m_isPlayer;
	bool m_firstTime;

public:
    //инициализация модели поведения
    virtual void Init(HumanEntity* human);

    // проиграть панику для человека
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // возврат:  
    //           true   -  необходим повторный вызов 
    //           false  -  модель поведения проиграна полностью
    //
    // ВНИМАНИЕ: 
    //           по окончанию паники кол-во movepoints д.б. == 0
    //
    virtual bool Panic(HumanEntity* human);
	ShockBehaviour();

};

ShockBehaviour::ShockBehaviour()
{
}

//инициализация модели поведения
void ShockBehaviour::Init(HumanEntity* human)
{
	if(human->GetPlayer() == PT_PLAYER)
	{
		Timer::Update();
		m_startTime = Timer::GetSeconds();
		m_isPlayer = true;
	}
	else
	{
		m_isPlayer = false;
	}

	m_firstTime = true;
}

// проиграть панику для человека
bool ShockBehaviour::Panic(HumanEntity* human)
{
	if(m_isPlayer && m_firstTime)
	{
		DirtyLinks::MoveCamera(human->GetGraph()->GetPos3());
		m_firstTime = false;
	}

	bool result = false;

	if(m_isPlayer)
	{
		Timer::Update();
		result = Timer::GetSeconds() - m_startTime <= 1.0f;
	}

	// обнулим человеку мувпоинты и закончим на этом
	if(human && !result)
	{
		human->GetEntityContext()->GetTraits()->SetMovepnts(0);
	}

	return result;
}

////////////////////////////////////////////////////////////////////////////////
//
// поведение в обычной панике
//
////////////////////////////////////////////////////////////////////////////////
class UsualPanicBehaviour : public PanicBehaviour
{
public:

    //инициализация модели поведения
    virtual void Init(HumanEntity* human);

    // проиграть панику для человека
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // возврат:  
    //           true   -  необходим повторный вызов 
    //           false  -  модель поведения проиграна полностью
    //
    // ВНИМАНИЕ: 
    //           по окончанию паники кол-во movepoints д.б. == 0
    //
    virtual bool Panic(HumanEntity* human);
private:
	// текущая активность
	ActivityHolder m_activity;
};

//=====================================================================================//
//                          void UsualPanicBehaviour::Init()                           //
//=====================================================================================//
//инициализация модели поведения
void UsualPanicBehaviour::Init(HumanEntity* human)
{
	m_activity.Reset();
}

//=====================================================================================//
//                          bool UsualPanicBehaviour::Panic()                          //
//=====================================================================================//
// проиграть панику для человека
bool UsualPanicBehaviour::Panic(HumanEntity* human)
{
	// если человек нулевой (может его убили) - на выход
	if(!human) return false;

    //проиграть действие
    if(m_activity.Run(AC_TICK)) return true;

	if(AIAPI::isSitPose(human->GetEID()))
	{
		AIAPI::setStandPose(human->GetEID());
	}

	// пусть человек побегает
	AIAPI::setRun(human->GetEID());

    // 1. если у человека нет шагов - на выход
	if(AIAPI::getSteps(human->GetEID()) < 1)
	{
		human->GetEntityContext()->GetTraits()->SetMovepnts(0);
		return false;
	}

	// посмотрим можно ли куда-нибудь побежать
	
	// рассчитаем поле проходимости
	PathUtils::CalcPassField(human, 0, 0, 0);
	// получим вектор хексов до которых можно будет добежать с моими шагами
	pnt_vec_t field;
	AIAPI::getBaseField(human->GetGraph()->GetPos2(),
		AIAPI::getSteps(human->GetEID()),
		&field);
	// получим вектор доступных хексов поля
	pnt_vec_t vec;
	AIAPI::getPanicReachableField(field, &vec);
	// получим случайный хекс
	ipnt2_t target = AIAPI::getRandomPoint(vec);
	// рассчитаем путь
	pnt_vec_t path;
	PathUtils::CalcPath(human, target, path);

	// 2. если бежать нельзя - на выход
	if(path.empty())
	{
		human->GetEntityContext()->GetTraits()->SetMovepnts(0);
		return false;
	}

	// 3. можно бежать - продолжение следует
	unsigned int flags = human->IsAlwaysInPanic() ? 
							ActivityFactory::CT_ENEMY_MOVE :
							ActivityFactory::CT_ENEMY_MOVEIGNORE;
	m_activity.Reset(ActivityFactory::CreateMove(human,
		path,
		flags));
	return true;
}

////////////////////////////////////////////////////////////////////////////////
//
// поведение в берсерке
//
////////////////////////////////////////////////////////////////////////////////
class BerserkBehaviour : public PanicBehaviour
{
public:

    //инициализация модели поведения
    virtual void Init(HumanEntity* human);

    // проиграть панику для человека
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // возврат:  
    //           true   -  необходим повторный вызов 
    //           false  -  модель поведения проиграна полностью
    //
    // ВНИМАНИЕ: 
    //           по окончанию паники кол-во movepoints д.б. == 0
    //
    virtual bool Panic(HumanEntity* human);

private:
	// текущая активность
	ActivityHolder m_activity;
};

//=====================================================================================//
//                            void BerserkBehaviour::Init()                            //
//=====================================================================================//
//инициализация модели поведения
void BerserkBehaviour::Init(HumanEntity* human)
{
	m_activity.Reset();
}

//=====================================================================================//
//                           bool BerserkBehaviour::Panic()                            //
//=====================================================================================//
// проиграть панику для человека
bool BerserkBehaviour::Panic(HumanEntity* human)
{
	// если человек нулевой (может его убили) - на выход
	if(!human) return false;

    //проиграть действие
    if(m_activity.Run(AC_TICK)) return true;

	// все стреляют в автоматическом режиме оружия (autoshot)
	AIAPI::setShootType(human->GetEID(), 0.0f, 0.0f);

	BaseThing *bt = human->GetEntityContext()->PoLGetThingInHands();
    if(!AIAPI::takeBestWeapon(human->GetEID(), AssaultWeaponComparator()))
    {
        if(!AIAPI::takeGrenade(human->GetEID()))
        {
            human->GetEntityContext()->GetTraits()->SetMovepnts(0);
            return false;
        }
    }

	if(bt != human->GetEntityContext()->PoLGetThingInHands())
	{
		human->DropFlags(EA_WAS_IN_INV);
	}

    // 1. если у человека нет мувпоинтов на выстрел - на выход
	if(AIAPI::getMovePoints(human->GetEID()) <
		AIAPI::getShootMovePoints(human->GetEID(), SHT_AUTOSHOT))
	{
		human->GetEntityContext()->GetTraits()->SetMovepnts(0);
		return false;
	}

	// 2. выстрелим если сможем
	if(AIAPI::shootRandom(human->GetEID(), &m_activity))
	{
		// получилось выстрелить - продолжение следует
		return true;
	}
	// выстрелить не получилось - на выход
	human->GetEntityContext()->GetTraits()->SetMovepnts(0);
	return false;
}

//////////////////////////////////////////////////////////////////////////////////
//
// фабрика моделей поведения в панике
//
//////////////////////////////////////////////////////////////////////////////////

//создать шоковое поведение
PanicBehaviour* PanicBehaviourFactory::GetShockBehaviour()
{
	static ShockBehaviour sb;
	return &sb;
}

//создать обычную панику
PanicBehaviour* PanicBehaviourFactory::GetUsualBehaviour()
{
	static UsualPanicBehaviour upb;
	return &upb;
}

//создать панику берсерка
PanicBehaviour* PanicBehaviourFactory::GetBerserkBehaviour()
{
	static BerserkBehaviour bb;
	return &bb;
}

/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: ������ ��������� �������� � ������
                                                                                
                                                                                
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
// ��������� � ����
//
////////////////////////////////////////////////////////////////////////////////
class ShockBehaviour : public PanicBehaviour
{
	float m_startTime;
	bool m_isPlayer;
	bool m_firstTime;

public:
    //������������� ������ ���������
    virtual void Init(HumanEntity* human);

    // ��������� ������ ��� ��������
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // �������:  
    //           true   -  ��������� ��������� ����� 
    //           false  -  ������ ��������� ��������� ���������
    //
    // ��������: 
    //           �� ��������� ������ ���-�� movepoints �.�. == 0
    //
    virtual bool Panic(HumanEntity* human);
	ShockBehaviour();

};

ShockBehaviour::ShockBehaviour()
{
}

//������������� ������ ���������
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

// ��������� ������ ��� ��������
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

	// ������� �������� ��������� � �������� �� ����
	if(human && !result)
	{
		human->GetEntityContext()->GetTraits()->SetMovepnts(0);
	}

	return result;
}

////////////////////////////////////////////////////////////////////////////////
//
// ��������� � ������� ������
//
////////////////////////////////////////////////////////////////////////////////
class UsualPanicBehaviour : public PanicBehaviour
{
public:

    //������������� ������ ���������
    virtual void Init(HumanEntity* human);

    // ��������� ������ ��� ��������
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // �������:  
    //           true   -  ��������� ��������� ����� 
    //           false  -  ������ ��������� ��������� ���������
    //
    // ��������: 
    //           �� ��������� ������ ���-�� movepoints �.�. == 0
    //
    virtual bool Panic(HumanEntity* human);
private:
	// ������� ����������
	ActivityHolder m_activity;
};

//=====================================================================================//
//                          void UsualPanicBehaviour::Init()                           //
//=====================================================================================//
//������������� ������ ���������
void UsualPanicBehaviour::Init(HumanEntity* human)
{
	m_activity.Reset();
}

//=====================================================================================//
//                          bool UsualPanicBehaviour::Panic()                          //
//=====================================================================================//
// ��������� ������ ��� ��������
bool UsualPanicBehaviour::Panic(HumanEntity* human)
{
	// ���� ������� ������� (����� ��� �����) - �� �����
	if(!human) return false;

    //��������� ��������
    if(m_activity.Run(AC_TICK)) return true;

	if(AIAPI::isSitPose(human->GetEID()))
	{
		AIAPI::setStandPose(human->GetEID());
	}

	// ����� ������� ��������
	AIAPI::setRun(human->GetEID());

    // 1. ���� � �������� ��� ����� - �� �����
	if(AIAPI::getSteps(human->GetEID()) < 1)
	{
		human->GetEntityContext()->GetTraits()->SetMovepnts(0);
		return false;
	}

	// ��������� ����� �� ����-������ ��������
	
	// ���������� ���� ������������
	PathUtils::CalcPassField(human, 0, 0, 0);
	// ������� ������ ������ �� ������� ����� ����� �������� � ����� ������
	pnt_vec_t field;
	AIAPI::getBaseField(human->GetGraph()->GetPos2(),
		AIAPI::getSteps(human->GetEID()),
		&field);
	// ������� ������ ��������� ������ ����
	pnt_vec_t vec;
	AIAPI::getPanicReachableField(field, &vec);
	// ������� ��������� ����
	ipnt2_t target = AIAPI::getRandomPoint(vec);
	// ���������� ����
	pnt_vec_t path;
	PathUtils::CalcPath(human, target, path);

	// 2. ���� ������ ������ - �� �����
	if(path.empty())
	{
		human->GetEntityContext()->GetTraits()->SetMovepnts(0);
		return false;
	}

	// 3. ����� ������ - ����������� �������
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
// ��������� � ��������
//
////////////////////////////////////////////////////////////////////////////////
class BerserkBehaviour : public PanicBehaviour
{
public:

    //������������� ������ ���������
    virtual void Init(HumanEntity* human);

    // ��������� ������ ��� ��������
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // �������:  
    //           true   -  ��������� ��������� ����� 
    //           false  -  ������ ��������� ��������� ���������
    //
    // ��������: 
    //           �� ��������� ������ ���-�� movepoints �.�. == 0
    //
    virtual bool Panic(HumanEntity* human);

private:
	// ������� ����������
	ActivityHolder m_activity;
};

//=====================================================================================//
//                            void BerserkBehaviour::Init()                            //
//=====================================================================================//
//������������� ������ ���������
void BerserkBehaviour::Init(HumanEntity* human)
{
	m_activity.Reset();
}

//=====================================================================================//
//                           bool BerserkBehaviour::Panic()                            //
//=====================================================================================//
// ��������� ������ ��� ��������
bool BerserkBehaviour::Panic(HumanEntity* human)
{
	// ���� ������� ������� (����� ��� �����) - �� �����
	if(!human) return false;

    //��������� ��������
    if(m_activity.Run(AC_TICK)) return true;

	// ��� �������� � �������������� ������ ������ (autoshot)
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

    // 1. ���� � �������� ��� ���������� �� ������� - �� �����
	if(AIAPI::getMovePoints(human->GetEID()) <
		AIAPI::getShootMovePoints(human->GetEID(), SHT_AUTOSHOT))
	{
		human->GetEntityContext()->GetTraits()->SetMovepnts(0);
		return false;
	}

	// 2. ��������� ���� ������
	if(AIAPI::shootRandom(human->GetEID(), &m_activity))
	{
		// ���������� ���������� - ����������� �������
		return true;
	}
	// ���������� �� ���������� - �� �����
	human->GetEntityContext()->GetTraits()->SetMovepnts(0);
	return false;
}

//////////////////////////////////////////////////////////////////////////////////
//
// ������� ������� ��������� � ������
//
//////////////////////////////////////////////////////////////////////////////////

//������� ������� ���������
PanicBehaviour* PanicBehaviourFactory::GetShockBehaviour()
{
	static ShockBehaviour sb;
	return &sb;
}

//������� ������� ������
PanicBehaviour* PanicBehaviourFactory::GetUsualBehaviour()
{
	static UsualPanicBehaviour upb;
	return &upb;
}

//������� ������ ��������
PanicBehaviour* PanicBehaviourFactory::GetBerserkBehaviour()
{
	static BerserkBehaviour bb;
	return &bb;
}

#include "logicdefs.h"

#include "Thing.h"
#include "Entity.h"
#include "Graphic.h"
#include "AIUtils.h"
#include "TraceUtils.h"
#include "CameraShieldThrowManager.h"
#include "Form.h"

//=====================================================================================//
//                CameraShieldThrowManager::CameraShieldThrowManager()                 //
//=====================================================================================//
CameraShieldThrowManager::CameraShieldThrowManager()
:	m_thing(0)
{
}

//=====================================================================================//
//                CameraShieldThrowManager::~CameraShieldThrowManager()                //
//=====================================================================================//
CameraShieldThrowManager::~CameraShieldThrowManager()
{
	delete m_thing;
}

//=====================================================================================//
//                        void CameraShieldThrowManager::Init()                        //
//=====================================================================================//
void CameraShieldThrowManager::Init(HumanEntity* human, const point3& to)
{
    m_to = to;
    m_player = human->GetPlayer();

    m_thing = human->GetEntityContext()->GetThingInHands(TT_CAMERA|TT_SHIELD);
    human->GetEntityContext()->RemoveThing(m_thing);        
    
    point3 from, dir;
    
    human->GetGraph()->GetBarrel(NULLVEC, &from);        
    dir = m_to - from;
    
    //бросить гранату
	const float range = AIUtils::CalcGrenadeRange(human,(human->GetGraph()->GetPos3()-to).Length());
	_GrenadeTracer tracer(human->GetGraph()->GetGID(), from, Normalize(dir), dir.Length(), 0, 10.0f, range);
	if(Forms::GetInst()->IsHMVisible())
	{
		m_end_time = Timer::GetSeconds();
	}
	else
	{
		m_end_time = EffectMgr::GetInst()->MakeGrenadeFlight(tracer.GetTrace(), m_thing->GetInfo()->GetItemModel(), &m_to);       
	}
}

//=====================================================================================//
//                        bool CameraShieldThrowManager::Run()                         //
//=====================================================================================//
bool CameraShieldThrowManager::Run()
{
    if(m_thing && m_end_time < Timer::GetSeconds())
	{
        if(m_thing->GetInfo()->IsShield())
            ShieldPool::GetInst()->Insert(static_cast<ShieldThing*>(m_thing), m_to);
        else if(m_thing->GetInfo()->IsCamera())
            CameraPool::GetInst()->Insert(m_player, static_cast<CameraThing*>(m_thing), m_to);

        delete m_thing;
        m_thing = 0;
    }
    
    return m_thing != 0;
}

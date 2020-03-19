#include "logicdefs.h"

#include "VisMap.h"
#include "Entity.h"
//#include "VisUtils3.h"
#include "UpdateVisMapDrv.h"

//=====================================================================================//
//                         UpdateVisMapDrv::UpdateVisMapDrv()                          //
//=====================================================================================//
UpdateVisMapDrv::UpdateVisMapDrv(BaseEntity* entity, unsigned flags, Driver* drv)
:	DecoratorDrv(drv),
	m_flags(flags),
	m_entity(entity)
{
	m_entity->Attach(this, EV_DESTROY);
}

//=====================================================================================//
//                         UpdateVisMapDrv::~UpdateVisMapDrv()                         //
//=====================================================================================//
UpdateVisMapDrv::~UpdateVisMapDrv()
{
    UpdateSpectatorAndMarker();
    if(m_entity) m_entity->Detach(this);
}

//=====================================================================================//
//                    activity_command UpdateVisMapDrv::CalcStep()                     //
//=====================================================================================//
activity_command UpdateVisMapDrv::CalcStep(Activity* activity, activity_command user_cmd)
{
    if(m_flags & (F_UPDATE_ONCE|F_UPDATE_ALWAYS))
	{
        m_flags &= ~F_UPDATE_ONCE;
        UpdateSpectatorAndMarker();
    }

    return DecoratorDrv::CalcStep(activity, user_cmd);
}

//=====================================================================================//
//                  void UpdateVisMapDrv::UpdateSpectatorAndMarker()                   //
//=====================================================================================//
void UpdateVisMapDrv::UpdateSpectatorAndMarker()
{
    if(m_entity == 0) return;

    EntityContext* context = m_entity->GetEntityContext();

    if(m_flags & F_UPDATE_MARKER && context->GetMarker())
	{
        VisMap::GetInst()->UpdateMarker(context->GetMarker(), UE_POS_CHANGE);
	}
    
    if(m_flags & F_UPDATE_SPECTATOR && context->GetSpectator())
	{
        VisMap::GetInst()->UpdateSpectator(context->GetSpectator(), UE_POS_CHANGE);
	}
}

//=====================================================================================//
//                           void UpdateVisMapDrv::Update()                            //
//=====================================================================================//
void UpdateVisMapDrv::Update(subject_t, event_t, info_t)
{
    m_entity->Detach(this);
    m_entity = 0;
}

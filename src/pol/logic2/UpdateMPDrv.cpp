#include "logicdefs.h"

#include "Entity.h"
#include "UpdateMPDrv.h"

//=====================================================================================//
//                             UpdateMPDrv::UpdateMPDrv()                              //
//=====================================================================================//
UpdateMPDrv::UpdateMPDrv(BaseEntity* entity, int moves, Driver* drv)
:	DecoratorDrv(drv),
	m_moves(moves),
	m_entity(entity)
{
}

//=====================================================================================//
//                    activity_command UpdateMPDrv::CalcMovepnts()                     //
//=====================================================================================//
activity_command UpdateMPDrv::CalcMovepnts(Activity* activity, activity_command user_cmd)
{
    m_entity->Accept(*this);
    return DecoratorDrv::CalcMovepnts(activity, user_cmd);
}

//=====================================================================================//
//                              void UpdateMPDrv::Visit()                              //
//=====================================================================================//
void UpdateMPDrv::Visit(HumanEntity* human)
{
	human->GetEntityContext()->GetTraits()->PoLAddMovepnts( - m_moves);
}

//=====================================================================================//
//                              void UpdateMPDrv::Visit()                              //
//=====================================================================================//
void UpdateMPDrv::Visit(VehicleEntity* vehicle)
{
	vehicle->GetEntityContext()->PoLSetMovepnts(vehicle->GetEntityContext()->GetMovepnts() - m_moves);
}

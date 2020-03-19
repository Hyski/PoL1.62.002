#include "precomp.h"

#include "Grid.h"
#include "DynObject.h"
#include "DoorProperty.h"
#include <Logic2/HexUtils.h>

//=====================================================================================//
//                            DoorProperty::DoorProperty()                             //
//=====================================================================================//
DoorProperty::DoorProperty(DynObjectSet *s)
:	UsableProperty(s),
	m_opened(false)
{
	UpdateHexGrid();
}

//=====================================================================================//
//                            DoorProperty::~DoorProperty()                            //
//=====================================================================================//
DoorProperty::~DoorProperty()
{
}

//=====================================================================================//
//                          float DoorProperty::SwitchState()                          //
//=====================================================================================//
float DoorProperty::SwitchState(float tau)
{
	float ttime = 0.0f;
	m_opened = !m_opened;

	for(DynObjectSet::Iterator_t it = GetObjects()->Begin(); it != GetObjects()->End(); ++it)
	{
		ttime = std::max((*it)->ChangeState(tau),ttime);
	}

	UpdateHexGrid();
	return tau + ttime + 0.2f;
}

//=====================================================================================//
//                           float DoorProperty::SetState()                            //
//=====================================================================================//
float DoorProperty::SetState(float state, float tau)
{
	float ttime = 0.0f;
	m_opened = (state > 0.5f);

	for(DynObjectSet::Iterator_t it = GetObjects()->Begin(); it != GetObjects()->End(); ++it)
	{
		ttime = std::max((*it)->ChangeState(state,tau),ttime);
	}

	UpdateHexGrid();
	return tau + ttime + 0.2f;
}

//=====================================================================================//
//                         void DoorProperty::UpdateHexGrid()                          //
//=====================================================================================//
void DoorProperty::UpdateHexGrid()
{
	GetObjects()->OpenArea(m_opened);
}

//=====================================================================================//
//                             void DoorProperty::Update()                             //
//=====================================================================================//
void DoorProperty::Update(float tau)
{
}

//=====================================================================================//
//                           void DoorProperty::OnDestruct()                           //
//=====================================================================================//
void DoorProperty::OnDestruct()
{
	m_opened = true;
	UpdateHexGrid();
}

//=====================================================================================//
//                            void DoorProperty::MakeLoad()                            //
//=====================================================================================//
void DoorProperty::MakeLoad(SavSlot &slot)
{
	slot >> m_opened;
	SetState(m_opened ? 1.0f : 0.0f, 0.0f);
}

//=====================================================================================//
//                            void DoorProperty::MakeSave()                            //
//=====================================================================================//
void DoorProperty::MakeSave(SavSlot &slot)
{
	slot << m_opened;
}
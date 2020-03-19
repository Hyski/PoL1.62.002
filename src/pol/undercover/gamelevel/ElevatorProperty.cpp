#include "precomp.h"

#include "Grid.h"
#include "DynObject.h"
#include "ElevatorProperty.h"
#include <Logic2/HexUtils.h>

//=====================================================================================//
//                        ElevatorProperty::ElevatorProperty()                         //
//=====================================================================================//
ElevatorProperty::ElevatorProperty(DynObjectSet *s)
:	UsableProperty(s)
{
	UpdateHexGrid();
}

//=====================================================================================//
//                        ElevatorProperty::~ElevatorProperty()                        //
//=====================================================================================//
ElevatorProperty::~ElevatorProperty()
{
}

//=====================================================================================//
//                        float ElevatorProperty::SwitchState()                        //
//=====================================================================================//
float ElevatorProperty::SwitchState(float tau)
{
	float ttime = 0.0f;

	for(DynObjectSet::Iterator_t it = GetObjects()->Begin(); it != GetObjects()->End(); ++it)
	{
		ttime = std::max((*it)->ChangeState(tau),ttime);
	}

	return tau + ttime + 0.2f;
}

//=====================================================================================//
//                         float ElevatorProperty::SetState()                          //
//=====================================================================================//
float ElevatorProperty::SetState(float state, float tau)
{
	float ttime = 0.0f;

	for(DynObjectSet::Iterator_t it = GetObjects()->Begin(); it != GetObjects()->End(); ++it)
	{
		ttime = std::max((*it)->ChangeState(state,tau),ttime);
	}

	return tau + ttime + 0.2f;
}

//=====================================================================================//
//                             static void UpdateHexGrid()                             //
//=====================================================================================//
static void UpdateHexGrid(Grid *grid, const HexGrid::CellVec &vec,
						  const HexGrid::HFrontVec &frontvec, float z)
{
	for(int i = 0; i < vec.size(); i++)
	{
		grid->Get(vec[i]).z=z;
	}

	HexGrid::HFrontVec::const_iterator it = frontvec.begin(), ite = frontvec.end();

	for(; it != ite; ++it)
	{
		ipnt2_t np_f(HexUtils::scr2hex(*it->from));
		ipnt2_t np_t(HexUtils::scr2hex(*it->to));
		bool dh_f, dh_t;
		dh_f = grid->GetProp(np_f).DynHeight;
		dh_t = grid->GetProp(np_t).DynHeight;
		if( (dh_f || UsableProperty::CanPass(np_f)) && (dh_t || UsableProperty::CanPass(np_t)) )
		{
			if(fabsf(it->from->z - it->to->z) > 0.6f)
			{
				it->from->SetWeight(it->dir, 100);
				it->to->SetWeight(HexUtils::GetReverseDir(it->dir), 100);
			}
			else
			{
				it->from->SetWeight(it->dir, 0);
				it->to->SetWeight(HexUtils::GetReverseDir(it->dir), 0);
			}
		}
	}
}

//=====================================================================================//
//                       void ElevatorProperty::UpdateHexGrid()                        //
//=====================================================================================//
void ElevatorProperty::UpdateHexGrid()
{
	Grid *grid = (Grid*)HexGrid::GetInst();
	::UpdateHexGrid(
		grid,
		grid->HeightDirect[GetObjects()->GetName()],
		grid->HeightFront[GetObjects()->GetName()],
		(*GetObjects()->Begin())->GetWorldMatrix()._43
	);
}

//=====================================================================================//
//                           void ElevatorProperty::Update()                           //
//=====================================================================================//
void ElevatorProperty::Update(float tau)
{
	UpdateHexGrid();
}

//=====================================================================================//
//                          void ElevatorProperty::MakeLoad()                          //
//=====================================================================================//
void ElevatorProperty::MakeLoad(SavSlot &slot)
{
	bool liftState;
	slot >> liftState;

	const float state = (liftState ? 1.0f : 0.0f);
	for(DynObjectSet::Iterator_t it = GetObjects()->Begin(); it != GetObjects()->End(); ++it)
	{
		(*it)->ChangeState(state,0.0f);
	}
}

//=====================================================================================//
//                          void ElevatorProperty::MakeSave()                          //
//=====================================================================================//
void ElevatorProperty::MakeSave(SavSlot &slot)
{
	bool liftState = (*GetObjects()->Begin())->m_animState > 0.5f;
	slot << liftState;
}

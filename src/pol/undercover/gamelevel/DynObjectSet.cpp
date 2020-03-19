#include "precomp.h"

#include "Grid.h"
#include "../IWorld.h"
#include "GameLevel.h"
#include "DoorProperty.h"
#include "DynObjectSet.h"
#include "TrainProperty.h"
#include "DynObjectPool.h"
#include "ElevatorProperty.h"
#include "DestructibleProperty.h"
#include <Logic2/HexUtils.h>

static MlVersion g_dynObjectSetVersion(0,1);

//=====================================================================================//
//                            DynObjectSet::DynObjectSet()                             //
//=====================================================================================//
// А конструктор в DynObjectSet.h исправить не забыл?
DynObjectSet::DynObjectSet(const DynObjectSetInfo &si)
:	DynObjectSetInfo(si),
	m_isDoor(si.IsDoor()),
	m_visible(si.IsInvisible()),
	m_destroyed(false)
{
	Clean();
	InitProperties();
}

//=====================================================================================//
//                          DynObjectSet::DynObjectSet<It>()                           //
//=====================================================================================//
DynObjectSet::DynObjectSet(const DynObjectSetInfo &info, HDynObject *begin, HDynObject *end)
:	DynObjectSetInfo(info),
	m_isDoor(info.IsDoor()),
	m_visible(!info.IsInvisible()),
	m_destroyed(false)
{
	Clean();
	for(; begin != end; ++begin)
	{
		if((*begin)->HasGeometry())
		{
			AddObject(*begin);
			(*begin)->SetSet(this);
		}
	}
	OpenArea(false);
	InitProperties();
}

//=====================================================================================//
//                            DynObjectSet::~DynObjectSet()                            //
//=====================================================================================//
DynObjectSet::~DynObjectSet()
{
	OpenArea(true);
}

//=====================================================================================//
//                         void DynObjectSet::UpdateHexGrid()                          //
//=====================================================================================//
static void UpdateHexGrid(Grid *grid, const HexGrid::CellVec &vec, bool cond)
{
	ipnt2_t np;

	for(int i = 0; i < vec.size(); i++)
	{
		for(int p = 0; p < 6; p++)
		{
			HexUtils::GetFrontPnts0(vec[i], p, &np);
			HexGrid::cell &cell = grid->Get(vec[i]);

			if(grid->IsOutOfRange(np)) 
			{
				cell.SetWeight(p,100);//?FIXME:
				continue;
			}

			HexGrid::cell &ncell = grid->Get(np);
			if(cond && !UsableProperty::CanPass(np))
			{
				if(cell.GetProperty() != ncell.GetProperty())
				{
					continue;
				}
			}

			ncell.SetWeight(HexUtils::GetReverseDir(p), cond ? 0 : 100);
			cell.SetWeight(p, cond ? 0 : 100);

		}
	}
}

//=====================================================================================//
//                            void DynObjectSet::OpenArea()                            //
//=====================================================================================//
void DynObjectSet::OpenArea(bool open)
{
	Grid *grid = (Grid*)HexGrid::GetInst();
	if(grid)
	{
		::UpdateHexGrid(grid,grid->ManDirect[GetName()],open);
		::UpdateHexGrid(grid,grid->ManInvert[GetName()],!open);
	}
}

//=====================================================================================//
//                         void DynObjectSet::InitProperties()                         //
//=====================================================================================//
void DynObjectSet::InitProperties()
{
	if(IsDoor())
	{
		m_usableProp.reset(new DoorProperty(this));
	}
	else if(IsElevator())
	{
		m_usableProp.reset(new ElevatorProperty(this));
	}
	else if(DynObjectSetInfo::IsUsable())
	{
		m_usableProp.reset(new UsableProperty(this));
	}

	if(IsTrain())
	{
		m_trainProp.reset(new TrainProperty(this));
	}

	if(IsDestructible())
	{
		m_destrProp.reset(new DestructibleProperty(this));
	}

	const StoredObjectInfo *info = IWorld::Get()->GetObjectInfo(GetFullName());
	if(info)
	{
		m_destroyed = info->m_isDestroyed;
		m_visible = info->m_isVisible;
	}
}

//=====================================================================================//
//                            void DynObjectSet::Destruct()                            //
//=====================================================================================//
void DynObjectSet::Destruct()
{
	if(m_destrProp.get())
	{
		m_destrProp->Destruct();
	}

	if(m_usableProp.get())
	{
		m_usableProp->OnDestruct();
	}
	else
	{
		OpenArea(true);
	}

	m_destroyed = true;
	IWorld::Get()->SetObjectInfo(GetFullName(),GetInfo());
}

//=====================================================================================//
//                         void DynObjectSet::SetVisibility()                          //
//=====================================================================================//
void DynObjectSet::SetVisibility(bool v)
{
	if(m_visible != v)
	{
		m_visible = v;
		IWorld::Get()->SetObjectInfo(GetFullName(),GetInfo());
	}
}

//=====================================================================================//
//                   StoredObjectInfo DynObjectSet::GetInfo() const                    //
//=====================================================================================//
StoredObjectInfo DynObjectSet::GetInfo() const
{
	StoredObjectInfo result;
	result.m_isDestroyed = m_destroyed;
	result.m_isVisible = m_visible;
	return result;
}

//=====================================================================================//
//                             void DynObjectSet::Clean()                              //
//=====================================================================================//
void DynObjectSet::Clean()
{
	m_usableProp.reset();
	m_objectCount = 0;
	m_updatableObjectCount = 0;
	m_center = NULLVEC;
	std::fill(m_objects,m_objects+MAX_OBJECTS,(DynObject*)0);
	std::fill(m_updatableObjects,m_updatableObjects+MAX_OBJECTS,(DynObject*)0);
}

//=====================================================================================//
//                           void DynObjectSet::AddObject()                            //
//=====================================================================================//
void DynObjectSet::AddObject(HDynObject obj)
{
	assert( m_objectCount != MAX_OBJECTS );
	m_objects[m_objectCount++] = obj;
	CalcObjCenter();

	if(DynObjectSetInfo::IsDoor() && (obj->IsAnimated() || obj->GetAnimLength() <= 1e-3f))
	{
		m_isDoor = false;
		m_usableProp.reset();
	}
}

//=====================================================================================//
//                         void DynObjectSet::CalcObjCenter()                          //
//=====================================================================================//
void DynObjectSet::CalcObjCenter()
{
	m_center = NULLVEC;

	for(Iterator_t i = Begin(); i != End(); ++i)
	{
        m_center += (*i)->GetBound().GetCenter();
    }

    //вычислим среднее арифметическое
    m_center *= 1.0f/static_cast<float>(GetObjectCount());
}

//=====================================================================================//
//                          float DynObjectSet::SwitchState()                          //
//=====================================================================================//
float DynObjectSet::SwitchState(float tau)
{
	if(m_usableProp.get())
	{
		return m_usableProp->SwitchState(tau);
	}
	else
	{
		return tau;
	}
}

//=====================================================================================//
//                          float DynObjectSet::ChangeState()                          //
//=====================================================================================//
float DynObjectSet::ChangeState(float state, float tau)
{
	if(m_usableProp.get())
	{
		return m_usableProp->SetState(state,tau);
	}
	else
	{
		return tau;
	}
}

//=====================================================================================//
//                            void DynObjectSet::EndTurn()                             //
//=====================================================================================//
void DynObjectSet::EndTurn()
{
	if(m_trainProp.get()) m_trainProp->EndTurn();
}

//=====================================================================================//
//                      bool DynObjectSet::EverNeedUpdate() const                      //
//=====================================================================================//
bool DynObjectSet::EverNeedUpdate() const
{
	return 0 != m_updatableObjectCount;
}

//=====================================================================================//
//                   bool DynObjectSet::EverNeedUpdateUsable() const                   //
//=====================================================================================//
bool DynObjectSet::EverNeedUpdateUsable() const
{
	return m_usableProp.get() != 0;
}

//=====================================================================================//
//                              void DynObjectSet::Bake()                              //
//=====================================================================================//
void DynObjectSet::Bake()
{
	m_updatableObjectCount = 0;
	for(Iterator_t it = Begin(); it != End(); ++it)
	{
		if(it->get()->GetAnimLength() > 0.0f)
		{
            m_updatableObjects[m_updatableObjectCount++] = *it;
		}
	}
}

//=====================================================================================//
//                             void DynObjectSet::Update()                             //
//=====================================================================================//
void DynObjectSet::Update(float tau)
{
	assert( EverNeedUpdate() );
	if(m_destroyed) return;

	for(unsigned int i = 0; i < m_updatableObjectCount; ++i)
	{
		DynObject *obj = m_updatableObjects[i].get();
		if(obj->IsInMotion())
		{
			obj->Update(tau);
		}
	}
}

//=====================================================================================//
//                          void DynObjectSet::UpdateUsable()                          //
//=====================================================================================//
void DynObjectSet::UpdateUsable(float tau)
{
	assert( EverNeedUpdateUsable() );
	if(m_destroyed) return;

	m_usableProp->Update(tau);
}

//=====================================================================================//
//                         bool DynObjectSet::IsUsable() const                         //
//=====================================================================================//
bool DynObjectSet::IsUsable() const
{
	return DynObjectSetInfo::IsUsable() && m_usableProp.get() && m_usableProp->CanSwitchState();
}

//=====================================================================================//
//                            void DynObjectSet::MakeLoad()                            //
//=====================================================================================//
void DynObjectSet::MakeLoad(SavSlot &slot)
{
	MlVersion ver;
	ver.Restore(slot);

	if(!ver.IsCompatible(g_dynObjectSetVersion))
	{
		throw CASUS("Wrong save version");
	}

	OpenArea(false);
	bool storeDoor, storeElevator, storeDestr, storeTrain;
	slot >> storeDoor >> storeElevator >> storeDestr >> storeTrain;

	if(storeDoor)
	{
		m_usableProp.reset(new DoorProperty(this));
		m_usableProp->MakeLoad(slot);
	}
	else if(storeElevator)
	{
		m_usableProp.reset(new ElevatorProperty(this));
		m_usableProp->MakeLoad(slot);
	}
	else if(DynObjectSetInfo::IsUsable())
	{
		m_usableProp.reset(new UsableProperty(this));
	}

	if(storeDestr)
	{
		m_destrProp.reset(new DestructibleProperty(this));
		m_destrProp->MakeLoad(slot);
	}

	if(storeTrain)
	{
		m_trainProp.reset(new TrainProperty(this));
		m_trainProp->MakeLoad(slot);
	}
	
	m_destroyed = false;

	if(ver.GetMinor() == 1)
	{
		slot >> m_destroyed;
	}

	if(m_destroyed)
	{
		OpenArea(true);
	}
}

//=====================================================================================//
//                            void DynObjectSet::MakeSave()                            //
//=====================================================================================//
void DynObjectSet::MakeSave(SavSlot &slot)
{
	g_dynObjectSetVersion.Store(slot);
	const bool storeDoor = IsDoor() && m_usableProp.get();
	const bool storeElevator = IsElevator() && m_usableProp.get();
	const bool storeDestr = IsDestructible() && m_destrProp.get();
	const bool storeTrain = IsTrain() && m_trainProp.get();
	slot << storeDoor << storeElevator << storeDestr << storeTrain;

	if(storeDoor || storeElevator)
	{
		m_usableProp->MakeSave(slot);
	}

	if(storeDestr)
	{
		m_destrProp->MakeSave(slot);
	}

	if(storeTrain)
	{
		m_trainProp->MakeSave(slot);
	}

	slot << m_destroyed;
}

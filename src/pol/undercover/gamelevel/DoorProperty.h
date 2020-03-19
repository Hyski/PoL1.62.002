#pragma once

#include "UsableProperty.h"

//=====================================================================================//
//                     class DoorProperty : public UsableProperty                      //
//=====================================================================================//
class DoorProperty : public UsableProperty
{
	bool m_opened;

public:
	DoorProperty(DynObjectSet *s);
	~DoorProperty();

	virtual float SwitchState(float tau);
	virtual float SetState(float state, float tau);
	virtual bool CanSwitchState() const { return !m_opened; }
	virtual void Update(float tau);

	virtual void MakeSave(SavSlot &slot);
	virtual void MakeLoad(SavSlot &slot);

	virtual void OnDestruct();

private:
	void UpdateHexGrid();
};
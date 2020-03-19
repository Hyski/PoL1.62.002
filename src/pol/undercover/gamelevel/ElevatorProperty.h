#pragma once

#include "UsableProperty.h"

//=====================================================================================//
//                   class ElevatorProperty : public ObjectProperty                    //
//=====================================================================================//
class ElevatorProperty : public UsableProperty
{
public:
	ElevatorProperty(DynObjectSet *s);
	~ElevatorProperty();

	virtual float SwitchState(float tau);
	virtual float SetState(float state, float tau);
	virtual bool CanSwitchState() const { return true; }
	virtual void Update(float tau);

	virtual void MakeSave(SavSlot &slot);
	virtual void MakeLoad(SavSlot &slot);

private:
	void UpdateHexGrid();
};
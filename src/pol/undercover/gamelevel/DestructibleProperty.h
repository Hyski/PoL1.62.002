#pragma once

#include "ObjectProperty.h"


//=====================================================================================//
//                 class DestructibleProperty : public ObjectProperty                  //
//=====================================================================================//
class DestructibleProperty : public ObjectProperty
{
public:
	DestructibleProperty(DynObjectSet *);

	void Destruct();

	virtual void MakeSave(SavSlot &slot) {}
	virtual void MakeLoad(SavSlot &slot) {}
};
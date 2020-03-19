#pragma once

#include "ObjectProperty.h"

//=====================================================================================//
//                     class TrainProperty : public ObjectProperty                     //
//=====================================================================================//
class TrainProperty : public ObjectProperty
{
	unsigned int m_turnsToGo;
	bool m_atStation;

public:
	TrainProperty(DynObjectSet *);

	/// Обработать окончание хода
	void EndTurn();

	virtual void MakeSave(SavSlot &slot);
	virtual void MakeLoad(SavSlot &slot);
};
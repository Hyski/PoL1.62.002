#include "precomp.h"
#include "LevelToLogic.h"
#include "TrainProperty.h"

//=====================================================================================//
//                           TrainProperty::TrainProperty()                            //
//=====================================================================================//
TrainProperty::TrainProperty(DynObjectSet *dset)
:	ObjectProperty(dset),
	m_atStation(false)
{
	const TrainInfo &ti = dset->GetTrainInfo();
	m_turnsToGo = (rand()%ti.m_turnsOutOfLevel)+1;
	LevelAPI::GetAPI()->EnableJoint(ti.m_jointToSwitch, m_atStation);
}

//=====================================================================================//
//                            void TrainProperty::EndTurn()                            //
//=====================================================================================//
void TrainProperty::EndTurn()
{
	const TrainInfo &ti = GetObjects()->GetTrainInfo();

	//если еще не пришло время, то ждем
	if(--m_turnsToGo > 0) 
	{
		LevelAPI::GetAPI()->EnableJoint(ti.m_jointToSwitch,m_atStation);
		return;
	}

	float targetState;

	if(m_atStation)
	{
		m_atStation = false;
		m_turnsToGo = std::max(ti.m_turnsOutOfLevel,1);
		targetState = 1.0f;
	}
	else
	{
		m_atStation = true;
		m_turnsToGo = std::max(ti.m_turnsAtStation,1);
		targetState = ti.m_stopFrame / (*GetObjects()->Begin())->GetAnimLength();
	}

	LevelAPI::GetAPI()->EnableJoint(ti.m_jointToSwitch,m_atStation);

	const float tau = Timer::GetSeconds();
	for(DynObjectSet::Iterator_t i = GetObjects()->Begin(); i != GetObjects()->End(); ++i)
	{
		(*i)->ChangeState(targetState,tau);
	}
}

//=====================================================================================//
//                           void TrainProperty::MakeLoad()                            //
//=====================================================================================//
void TrainProperty::MakeLoad(SavSlot &slot)
{
	slot >> m_atStation;
	slot >> m_turnsToGo;

	const TrainInfo &ti = GetObjects()->GetTrainInfo();
	LevelAPI::GetAPI()->EnableJoint(ti.m_jointToSwitch,m_atStation);

	float targetState = 0.0f;
	if(m_atStation)
	{
		targetState = ti.m_stopFrame / (*GetObjects()->Begin())->GetAnimLength();
	}

	for(DynObjectSet::Iterator_t i = GetObjects()->Begin(); i != GetObjects()->End(); ++i)
	{
		(*i)->ChangeState(targetState,0.0f);
	}
}

//=====================================================================================//
//                           void TrainProperty::MakeSave()                            //
//=====================================================================================//
void TrainProperty::MakeSave(SavSlot &slot)
{
	slot << m_atStation;
	slot << m_turnsToGo;
}
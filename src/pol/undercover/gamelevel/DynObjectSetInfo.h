#pragma once

#include "UseInfo.h"
#include "TrainInfo.h"
#include "LevelCaseInfo.h"

class DynObjectOldData;

//=====================================================================================//
//                               class DynObjectSetInfo                                //
//=====================================================================================//
class DynObjectSetInfo
{
	enum ObjectType
	{
		otDoor			= 1<<0,
		otTrain			= 1<<1,
		otUsable		= 1<<2,
		otStorage		= 1<<3,
		otDatabase		= 1<<4,
		otElevator		= 1<<5,
		otDestructible	= 1<<6,
		otInvisible		= 1<<7
	};

	/// Информация о наборе
	std::string m_name;			///< Название объекта
	std::string m_fullName;		///< Полное имя объекта (включает название уровня)
	unsigned int m_type;		///< Тип объекта (типы могут быть комбинированы)
	UseInfo m_useInfo;
	TrainInfo m_trainInfo;
	LevelCaseInfo m_caseInfo;

public:
	DynObjectSetInfo();
	//DynObjectSetInfo(SavSlot &);
	DynObjectSetInfo(const DynObjectOldData &, const std::string &levname);

	const std::string &GetName() const { return m_name; }
	const std::string &GetFullName() const { return m_fullName; }

	bool IsDoor() const { return (m_type & otDoor) != 0; }
	bool IsTrain() const { return (m_type & otTrain) != 0; }
	bool IsUsable() const { return (m_type & otUsable) != 0; }
	bool IsStorage() const { return (m_type & otStorage) != 0; }
	bool IsDatabase() const { return (m_type & otDatabase) != 0; }
	bool IsElevator() const { return (m_type & otElevator) != 0; }
	bool IsInvisible() const { return (m_type & otInvisible) != 0; }
	bool IsDestructible() const { return (m_type & otDestructible) != 0; }

	const UseInfo &GetUseInfo() const { return m_useInfo; }
	const TrainInfo &GetTrainInfo() const { return m_trainInfo; }
	const LevelCaseInfo &GetStorageInfo() const { return m_caseInfo; }

	//void Read(SavSlot &slot);
	//void Write(SavSlot &slot);

private:
	//void Read4(SavSlot &slot);
	//void Read5(SavSlot &slot);

	//void ReadStation(const std::string &);
	//void ReadDatabase(const std::string &);
	//void ReadLevelCase(const std::string &);
};
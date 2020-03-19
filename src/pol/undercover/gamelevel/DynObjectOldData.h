#pragma once

#include "UseInfo.h"
#include "TrainInfo.h"
#include "SoundInfo.h"
#include "LevelCaseInfo.h"
#include "DestructibleInfo.h"

namespace mll { namespace utils { class named_vars; } }

//=====================================================================================//
//                               class DynObjectOldData                                //
//=====================================================================================//
class DynObjectOldData
{
	enum ObjectType
	{
		otDoor			= 1<<0,		///< ������ �������� ������
		otTrain			= 1<<1,		///< ������ �������� �������
		otUsable		= 1<<2,		///< ������ ����� ������������
		otStorage		= 1<<3,		///< ������ �������� ����������
		otDatabase		= 1<<4,		///< ������ �������� ����� ������
		otElevator		= 1<<5,		///< ������ �������� ������
		otAnimated		= 1<<6,		///< ������ ��������� �����������
		otSoundable		= 1<<7,		///< ������ �������� ������������ ��������
		otDestructible	= 1<<8,		///< ������ ����������
		otInvisible		= 1<<9		///< ������ ���������� �������
	};

	enum PhysicsType
	{
		ptGhost			= 1<<0,		///< ������ ���������
		ptTransparent	= 1<<1		///< ������ ���������
	};

	std::string m_name;			///< �������� �������
	SoundInfo m_soundInfo;
	unsigned int m_type;
	unsigned int m_physType;	///< ���������� �������� ������� (�����������, ����������)
	DestructibleInfo m_destructibleInfo;
	UseInfo m_useInfo;
	TrainInfo m_trainInfo;
	LevelCaseInfo m_caseInfo;

	float m_animState;		///< ������� ��������� ��������
	bool m_doorLocked;		///< �������� �� ����� ���������������
	bool m_isRoof;			///< ��������� �� � ������

public:
	DynObjectOldData();
	DynObjectOldData(SavSlot &slot);

	void ReadVars(const mll::utils::named_vars &vars, int floor);

	const std::string &GetName() const { return m_name; }

	bool IsRoof() const { return m_isRoof; }

	bool IsGhost() const { return (m_physType & ptGhost) != 0; }
	bool IsTransparent() const { return (m_physType & ptTransparent) != 0; }

	bool IsAnimated() const { return (m_type & otAnimated) != 0; }
	bool IsSoundable() const { return (m_type & otSoundable) != 0; }
	bool IsDestructible() const { return (m_type & otDestructible) != 0; }

	const SoundInfo &GetSoundInfo() const { return m_soundInfo; }
	const DestructibleInfo &GetDestructibleInfo() const { return m_destructibleInfo; }

	bool IsDoor() const { return (m_type & otDoor) != 0; }
	bool IsTrain() const { return (m_type & otTrain) != 0; }
	bool IsUsable() const { return (m_type & otUsable) != 0; }
	bool IsStorage() const { return (m_type & otStorage) != 0; }
	bool IsDatabase() const { return (m_type & otDatabase) != 0; }
	bool IsElevator() const { return (m_type & otElevator) != 0; }
	bool IsInvisible() const { return (m_type & otInvisible) != 0; }

	const UseInfo &GetUseInfo() const { return m_useInfo; }
	const TrainInfo &GetTrainInfo() const { return m_trainInfo; }
	const LevelCaseInfo &GetStorageInfo() const { return m_caseInfo; }

	float GetAnimState() const { return m_animState; }
	bool IsDoorLocked() const { return m_doorLocked; }

	bool operator<(const DynObjectOldData &r) const { return m_name < r.m_name; }
	friend bool operator<(const DynObjectOldData &l, const std::string &r)
	{
		return l.m_name < r;
	}
	friend bool operator<(const std::string &l, const DynObjectOldData &r)
	{
		return l < r.m_name;
	}

	void Read(SavSlot &slot);

private:
	void Read4(SavSlot &slot);

	void ReadSounds(const std::string &);
	void ReadStation(const std::string &);
	void ReadDatabase(const std::string &);
	void ReadLevelCase(const std::string &);
	void ReadDestructible(const std::string &);
};
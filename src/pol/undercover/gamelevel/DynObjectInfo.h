#pragma once

#include "SoundInfo.h"
#include "DestructibleInfo.h"

class DynObjectOldData;

//=====================================================================================//
//                                 class DynObjectInfo                                 //
//=====================================================================================//
class DynObjectInfo
{
public:
	enum ObjectType
	{
		otAnimated		= 1<<0,
		otSoundable		= 1<<1,
		otDestructible	= 1<<2
	};

	enum PhysicsType
	{
		ptGhost			= 1<<0,
		ptTransparent	= 1<<1
	};

protected:
	std::string m_name;			///< Название объекта
	SoundInfo m_soundInfo;
	unsigned int m_type;
	unsigned int m_physType;	///< Физические свойства объекта (неосязаемый, прозрачный)
	DestructibleInfo m_destructibleInfo;

public:
	DynObjectInfo();
	DynObjectInfo(SavSlot &);
	DynObjectInfo(const DynObjectOldData &);

	const std::string &GetName() const { return m_name; }

	bool IsGhost() const { return (m_physType & ptGhost) != 0; }
	bool IsTransparent() const { return (m_physType & ptTransparent) != 0; }
	bool IsDestructible() const { return (m_type & otDestructible) != 0; }

	bool IsAnimated() const { return (m_type & otAnimated) != 0; }
	bool IsSoundable() const { return (m_type & otSoundable) != 0; }

	const SoundInfo &GetSoundInfo() const { return m_soundInfo; }
	const DestructibleInfo &GetDestructibleInfo() const { return m_destructibleInfo; }

	void Read(SavSlot &slot);
	void Write(SavSlot &slot);

private:
	void Read5(SavSlot &slot);

	void ReadSounds(const std::string &);
	void ReadDestructible(const std::string &);
};

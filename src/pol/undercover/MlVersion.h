#pragma once

//=====================================================================================//
//                                   class MlVersion                                   //
//=====================================================================================//
class MlVersion
{
	unsigned int m_major;
	unsigned int m_minor;

public:
	MlVersion() : m_major(0xFFFFFFFF), m_minor(0xFFFFFFFF) {}
	MlVersion(unsigned int major, unsigned int minor) : m_major(major), m_minor(minor) {}

	unsigned int GetMajor() const { return m_major; }
	unsigned int GetMinor() const { return m_minor; }

	bool IsCompatible(const MlVersion &v) const
	{
		if(v.GetMajor() != GetMajor()) return false;
		return GetMinor() <= v.GetMinor();
	}

	template<typename T> void Store(T &stream) const
	{
		stream << m_major << m_minor;
	}

	template<typename T> void Restore(T &stream)
	{
		stream >> m_major >> m_minor;
	}
};
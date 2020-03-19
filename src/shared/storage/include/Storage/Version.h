#pragma once

#include "Storage.h"

namespace Storage
{

//=====================================================================================//
//                                    class version                                    //
//=====================================================================================//
class Version
{
	unsigned int m_major;
	unsigned int m_minor;

	bool _valid() const { return (m_major&m_minor) != 0xFFFFFFFF; }

public:
	Version() : m_major(0xFFFFFFFF), m_minor(0xFFFFFFFF) {}
	Version(unsigned int mj, unsigned int mn) : m_major(mj), m_minor(mn) {}
	bool isCompatible(const Version &v) const;

	unsigned int getMajor() const { return m_major; }
	unsigned int getMinor() const { return m_minor; }

	friend bool operator==(const Version &, const Version &);
	friend bool operator<(const Version &, const Version &);
};

template struct boost::equality_comparable<Version>;
template struct boost::less_than_comparable<Version>;

//=====================================================================================//
//                      inline bool Version::isCompatible() const                      //
//=====================================================================================//
inline bool Version::isCompatible(const Version &v) const
{
	if(!_valid() || !v._valid()) return false;
	if(m_major != v.m_major || m_minor > v.m_minor) return false;
	return true;
}

//=====================================================================================//
//                           inline obinstream &operator<<()                           //
//=====================================================================================//
inline mll::io::obinstream &operator<<(mll::io::obinstream &out, const Version &v)
{
	return out << v.getMajor() << v.getMinor();
}

//=====================================================================================//
//                           inline ibinstream &operator>>()                           //
//=====================================================================================//
inline mll::io::ibinstream &operator>>(mll::io::ibinstream &in, Version &v)
{
	unsigned int major, minor;
	in >> major >> minor;
	v = Version(major,minor);
	return in;
}

//=====================================================================================//
//                                  bool operator==()                                  //
//=====================================================================================//
inline bool operator==(const Version &lhs, const Version &rhs)
{
	if(!lhs._valid() || !rhs._valid()) return false;
	return lhs.getMajor() == rhs.getMajor() && lhs.getMinor() == rhs.getMinor();
}

//=====================================================================================//
//                               inline bool operator<()                               //
//=====================================================================================//
inline bool operator<(const Version &lhs, const Version &rhs)
{
	if(!lhs._valid() || !rhs._valid()) return false;
	if(lhs.getMajor() == rhs.getMajor()) return lhs.getMinor() < rhs.getMinor();
	return lhs.getMajor() < rhs.getMajor();
}

}
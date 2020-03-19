#pragma once

#include "VFile.h"
#include "../DataMgr/DataMgr.h"

namespace PoL
{

//=====================================================================================//
//                       class VFileBuf : public std::streambuf                        //
//=====================================================================================//
class VFileBuf : public std::streambuf
{
public:
	VFileBuf(VFile *file)
	{
		char_type * const ptr = const_cast<char_type *>(reinterpret_cast<const char_type *>(file->Data()));
		setg(ptr,ptr,ptr + file->Size());
	}

	int_type underflow()
	{
		return traits_type::eof();
	}
};

//=====================================================================================//
//                                   class VFileWpr                                    //
//=====================================================================================//
class VFileWpr
{
	std::string m_name;
	VFile *m_vfile;

public:
	VFileWpr(const std::string &name) : m_name(name), m_vfile(DataMgr::Load(name.c_str()))
	{
	}

	~VFileWpr()
	{
		DataMgr::Release(m_name.c_str());
	}

	VFile *operator->() const { return m_vfile; }

	VFile *get() const { return m_vfile; }
};

}

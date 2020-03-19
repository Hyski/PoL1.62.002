#pragma once

#include "Sizer.h"

namespace PoL
{

//=====================================================================================//
//                           class DullSizer : public Sizer                            //
//=====================================================================================//
class DullSizer : public Sizer
{
	typedef std::hash_map<std::string,SizerTypes> SizeMap_t;
	typedef std::hash_map<std::string,int> CountMap_t;

	SizeMap_t m_sizeMap;
	CountMap_t m_taskListMap;

public:
	DullSizer(unsigned int version);

	/// Возвращает тип записи
	virtual SizerTypes getEntryType(mll::io::ibinstream &, const std::string &name)
	{
		SizeMap_t::iterator i = m_sizeMap.find(name);
		assert( i != m_sizeMap.end() );
		return i->second;
	}

	/// Пропускает запись
	virtual void skipEntry(mll::io::ibinstream &in, const std::string &);

private:
	int getTaskListCount(const std::string &name)
	{
		CountMap_t::iterator i = m_taskListMap.find(name);
		assert( i != m_taskListMap.end() );
		return i->second;
	}
};

}

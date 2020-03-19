
#include "precomp.h"
#include <mll/fs/cache.h>
#include "system_hidden.h"

#include <mll/fs/entry.h>

namespace mll
{

namespace fs
{

namespace
{
	const unsigned int single_query_cache_max_size = 5;
}

static critical_section *getCacheCS()
{
	static critical_section cs;
	return &cs;
}

//=====================================================================================//
//                           void cache::add_single_query()                            //
//=====================================================================================//
void cache::add_single_query(file_id id)
{
	entry guard(getCacheCS());

	if (m_single_query_data.size() >= single_query_cache_max_size)
		m_single_query_data.pop_front();
	m_single_query_data.push_back(id);
}

//=====================================================================================//
//                               void cache::drop_file()                               //
//=====================================================================================//
void cache::drop_file(file_id id)
{
	entry guard(getCacheCS());

	//	найдем и грохнем из кэша сингл-запросов
	for (single_query_cache_t::iterator i = m_single_query_data.begin(); i != m_single_query_data.end();)
	{
		if (*i == id)
		{
			m_single_query_data.erase(i++);
			continue;
		}
		++i;
	}
}

//=====================================================================================//
//                               void cache::clear_all()                               //
//=====================================================================================//
void cache::clear_all()
{
	entry guard(getCacheCS());

	m_single_query_data.clear();
}

//=====================================================================================//
//                        file_id cache::single_query_stored()                         //
//=====================================================================================//
file_id cache::single_query_stored(const std::string& path)
{
	entry guard(getCacheCS());

	unsigned int size = m_single_query_data.size();
	file_id temp;
	for (unsigned int i = 0; i < size; ++i)
	{
		temp = m_single_query_data[i];
		if (temp.path() == path)
			return temp;
	}
	return file_id();
}

}

}


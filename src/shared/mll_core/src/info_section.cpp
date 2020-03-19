#include "precomp.h"
#include <mll/debug/callstack.h>

using namespace mll::debug;

namespace
{
typedef std::map<std::string,std::ostringstream *> info_map_t;

//=====================================================================================//
//                                    class deleter                                    //
//=====================================================================================//
class deleter
{
	info_map_t &m_map;

public:
	deleter(info_map_t &nfos) : m_map(nfos) {}

	~deleter()
	{
		for(info_map_t::iterator i = m_map.begin(); i != m_map.end(); ++i)
		{
			delete i->second;
		}
	}
};

info_map_t &get_infos();

}

//=====================================================================================//
//                       std::ostream &crash_info::get_section()                       //
//=====================================================================================//
std::ostream &crash_info::get_section(const std::string &name)
{
	info_map_t::iterator i = get_infos().find(name);
	if(i == get_infos().end())
	{
		i = get_infos().insert(info_map_t::value_type(name,new std::ostringstream)).first;
	}

	return *i->second;
}

void crash_info::remove_section(const std::string &name)
{
	get_infos().erase(name);
}

//=====================================================================================//
//                           void crash_info::flush_infos()                            //
//=====================================================================================//
void crash_info::flush_infos(std::ostream &out)
{
	for(info_map_t::iterator i = get_infos().begin(); i != get_infos().end(); ++i)
	{
		out << i->first << ":\n";
		out << i->second->str() << "\n\n";
	}
}

namespace
{
//=====================================================================================//
//                               info_map_t &get_infos()                               //
//=====================================================================================//
info_map_t &get_infos()
{
	static info_map_t infos;
	static deleter dltr(infos);
	return infos;
}
}
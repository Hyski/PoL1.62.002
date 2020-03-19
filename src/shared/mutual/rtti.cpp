#define __BUILDING_MUTUAL_LIBRARY__

#include "rtti.h"
#include <assert.h>
#include <sstream>

#pragma warning(disable:4786)

namespace
{

static bool g_indexTableValid = false;

//=====================================================================================//
//                                 struct by_type_info                                 //
//=====================================================================================//
struct by_type_info
{
	const std::type_info &m_ti;
	by_type_info(const std::type_info &ti) : m_ti(ti) {}

	bool operator()(const class_info_holder &cih) const { return (cih.entry().tinfo() == m_ti) != 0; }
};

//=====================================================================================//
//                              class_info getNthParent()                              //
//=====================================================================================//
class_info getNthParent(class_info i, unsigned n)
{
	assert( i.level() >= n );
	n = i.level() - n;
	for(unsigned int j=0;j<n;++j) i = i.parent();
	return i;
}

//=====================================================================================//
//     struct type_sort : public std::binary_function<class_info,class_info,bool>      //
//=====================================================================================//
struct type_sort
{
	bool operator()(const class_info_holder &a, const class_info_holder &b) const
	{
		const class_info ci_a = a.info();
		const class_info ci_b = b.info();
		const unsigned int max_level = std::min(ci_a.level(),ci_b.level());
		for(unsigned int i=0;i<=max_level;++i)
		{
			const std::type_info &ta = getNthParent(ci_a,i).std_type_info();
			const std::type_info &tb = getNthParent(ci_b,i).std_type_info();
			if(ta.before(tb)) return true;
			if(tb.before(ta)) return false;
		}

		return ci_a.level() < ci_b.level();
	}
};

//=====================================================================================//
//           rtti_service_implementation *get_rtti_service_implementation()            //
//=====================================================================================//
rtti_service_implementation *get_rtti_service_implementation()
{
	static rtti_service_implementation rtti;
	return &rtti;
}

}

//=====================================================================================//
//                     const std::string &class_info::name() const                     //
//=====================================================================================//
const std::string &class_info::name() const
{
	return m_entry->name();
}

//=====================================================================================//
//                        class_info class_info::parent() const                        //
//=====================================================================================//
class_info class_info::parent() const
{
	return class_info(m_entry->parent());
}

//=====================================================================================//
//                      const unsigned class_info::level() const                       //
//=====================================================================================//
unsigned class_info::level() const
{
	return m_entry->level();
}

//=====================================================================================//
//                 const std::type_info &class_info::type_info() const                 //
//=====================================================================================//
const std::type_info &class_info::std_type_info() const
{
	return m_entry->tinfo();
}

//=====================================================================================//
//                    const caster *class_info::get_caster() const                     //
//=====================================================================================//
const caster *class_info::get_caster() const
{
	return m_entry->get_caster();
}

//=====================================================================================//
//                         bool class_info::operator<() const                          //
//=====================================================================================//
bool class_info::operator<(const class_info &rhs) const
{
	if(!g_indexTableValid) get_rtti_service_implementation()->validate_indexes();
	return m_entry->index() < rhs.m_entry->index();
}

//=====================================================================================//
//                         bool class_info::operator>() const                          //
//=====================================================================================//
bool class_info::operator>(const class_info &rhs) const
{
	if(!g_indexTableValid) get_rtti_service_implementation()->validate_indexes();
	return m_entry->index() > rhs.m_entry->index();
}

//=====================================================================================//
//             rtti_service_implementation::rtti_service_implementation()              //
//=====================================================================================//
rtti_service_implementation::rtti_service_implementation()
{
}

//=====================================================================================//
//                  rtti_service_implementation::cut_unneeded_path()                   //
//=====================================================================================//
inline std::string rtti_service_implementation::cut_unneeded_path(const char *str)
{
	std::string result(str);
	result.erase(0,result.rfind('\\',result.rfind('\\')-1));
	return result;
}

//=====================================================================================//
//                 rtti_service_implementation::create_class_object()                  //
//=====================================================================================//
rtti_service_implementation::class_map::iterator
	rtti_service_implementation::create_class_object(const std::string &name,
													 class_info parent,
													 const std::type_info &info,
													 const caster *c)
{
	if(parent)
	{
		RTTI_LOG("    <create>        " << info.name() << " - parent(" << parent.name() << ")\n");
	}
	else
	{
		RTTI_LOG("    <create>        " << info.name() << "\n");
	}

	g_indexTableValid = false;
	class_map::iterator i;
	{
		class_info_holder tmp(name.c_str(),parent.entry(),c);
		tmp.addref(info);
		i = std::lower_bound(m_classes.begin(),m_classes.end(),tmp,type_sort());
		tmp.release(info);
		i = m_classes.insert(i,tmp);
	}

	if(!name.empty())
	{
		m_named_classes.insert(std::make_pair(name,&*i));
	}
	return i;
}

//=====================================================================================//
//                rtti_service_implementation::register_class_object()                 //
//=====================================================================================//
class_info rtti_service_implementation::register_class_object(const char *name,
															  const char *file,
															  class_info parent,
															  const std::type_info &info,
															  const caster *c)
{
	RTTI_LOG("<register>          " << info.name() << "\n");
	class_map::iterator i = std::find_if(m_classes.begin(),m_classes.end(),by_type_info(info));

	if(i == m_classes.end())
	{
		i = create_class_object(name,parent,info,c);
	}

	i->addref(info);
	return i->info();
}

//=====================================================================================//
//               rtti_service_implementation::unregister_class_object()                //
//=====================================================================================//
void rtti_service_implementation::unregister_class_object(const std::type_info &info)
{
	RTTI_LOG("<unregister>        " << info.name() << "\n");
	class_map::iterator i = std::find_if(m_classes.begin(),m_classes.end(),by_type_info(info));
	assert( i != m_classes.end() );

	if(!i->release(info))
	{
		RTTI_LOG("    <destroy>       " << info.name() << "\n");
		if(!i->entry().name().empty())
		{
			m_named_classes.erase(i->entry().name());
		}
		m_classes.erase(i);
	}
}

//=====================================================================================//
//                   rtti_service_implementation::dump_class_list()                    //
//=====================================================================================//
void rtti_service_implementation::dump_class_list()
{
	RTTI_LOG("@@@@ dumping active class infos\n");

	for(class_map::const_iterator i = m_classes.begin(); i != m_classes.end(); ++i)
	{
		RTTI_LOG("- name{" << i->entry().name() << "} parent{"
			<< (i->entry().parent()?i->entry().parent()->name():"")
			<< "}\n");
	}

	RTTI_LOG("@@@@ end of dump\n");
}

//=====================================================================================//
//                   rtti_service_implementation::get_class_object()                   //
//=====================================================================================//
class_info rtti_service_implementation::get_class_object(const char *name)
{
	named_class_map::const_iterator i = m_named_classes.find(name);
	if( i == m_named_classes.end() ) return class_info(0);
	return class_info(&i->second->entry());
}

//=====================================================================================//
//                void rtti_service_implementation::validate_indexes()                 //
//=====================================================================================//
void rtti_service_implementation::validate_indexes()
{
	unsigned index = 0;
	for(class_map::iterator i = m_classes.begin(); i != m_classes.end(); ++i)
	{
		i->set_index(index++);
	}
	g_indexTableValid = true;
}

//=====================================================================================//
//                                 get_rtti_service()                                  //
//=====================================================================================//
rtti_service *get_rtti_service()
{
	return get_rtti_service_implementation();
}

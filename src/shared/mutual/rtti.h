#if !defined(__RTTI_H_INCLUDED_2023399421164473__)
#define __RTTI_H_INCLUDED_2023399421164473__

#ifndef __BUILDING_MUTUAL_LIBRARY__
#error Inclusion of internal header from mutual library. May be wrong additional include directories, order ../mutual/include before ..
#endif

#if !defined(MUTUAL_API) && !defined(MUTUAL_STATIC)
#	define MUTUAL_API	__declspec(dllexport)
#endif

#include <cassert>
#include <map>
#include <hash_map>
#include <vector>
#include <list>
#include <algorithm>
#include <mutual/rtti.h>

#if defined(ML_MUTUAL_LOG_RTTI_EVENTS) || defined(_DEBUG)
#include <mll/debug/log.h>
#define RTTI_LOG(List)		MLL_MAKE_LOG("rtti/rtti.log",List)
#else
#define	RTTI_LOG(List)
#endif

//=====================================================================================//
//                               class type_info_counted                               //
//=====================================================================================//
class type_info_counted
{
	const std::type_info *m_ti;
	unsigned m_count;

public:
	type_info_counted(const std::type_info *ti) : m_ti(ti), m_count(0) {}
	~type_info_counted() {}

	unsigned addref() { return ++m_count; }
	unsigned release() { return --m_count; }

	friend bool operator<(const type_info_counted &lhs, const type_info_counted &rhs);
	friend bool operator==(const type_info_counted &lhs, const type_info_counted &rhs);

	const std::type_info &tinfo() const { return *m_ti; }
};

//=====================================================================================//
//                               friend bool operator<()                               //
//=====================================================================================//
bool operator<(const type_info_counted &lhs, const type_info_counted &rhs)
{
	return lhs.m_ti < rhs.m_ti;
}

//=====================================================================================//
//                                  bool operator==()                                  //
//=====================================================================================//
bool operator==(const type_info_counted &lhs, const type_info_counted &rhs)
{
	return lhs.m_ti == rhs.m_ti;
}

//=====================================================================================//
//                                  bool operator!=()                                  //
//=====================================================================================//
bool operator!=(const type_info_counted &lhs, const type_info_counted &rhs)
{
	return !(lhs == rhs);
}

//=====================================================================================//
//                                  class rtti_entry                                   //
//=====================================================================================//
class rtti_entry
{
	typedef std::vector<type_info_counted> typeinfos_t;

	const caster *m_caster;
	std::string m_name;
	const rtti_entry *m_parent;
	unsigned m_level;
	typeinfos_t m_tis;
	unsigned m_index;

	friend class rtti_service_implementation;

public:
	rtti_entry(const std::string &name, const rtti_entry *parent, const caster *caster)
	:	m_caster(caster),
		m_name(name),
		m_parent(parent),
		m_level(m_parent?m_parent->m_level+1:0)
	{
	}

	void addref(const type_info &ti)
	{
		typeinfos_t::iterator i = std::lower_bound(m_tis.begin(),m_tis.end(),type_info_counted(&ti));
		if(i != m_tis.end() && *i == &ti)
		{
			i->addref();
		}
		else
		{
			m_tis.insert(i,&ti)->addref();
		}
	}

	void release(const type_info &ti)
	{
		typeinfos_t::iterator i = std::lower_bound(m_tis.begin(),m_tis.end(),type_info_counted(&ti));
		assert( *i == &ti );
		if(!i->release())
		{
			m_tis.erase(i);
		}
	}

	const caster *get_caster() const { return m_caster; }
	const std::string &name() const { return m_name; }
	const rtti_entry *parent() const { return m_parent; }
	const unsigned level() const { return m_level; }
	const std::type_info &tinfo() const { return m_tis.front().tinfo(); }
	unsigned index() const { return m_index; }
	void set_index(unsigned i) { m_index = i; }
};

//=====================================================================================//
//                               class class_info_holder                               //
//=====================================================================================//
class class_info_holder
{
	rtti_entry m_entry;
	mutable unsigned m_refctr;

public:
	class_info_holder(const char *name, const rtti_entry *parent, const caster *c)
		: m_entry(name,parent,c), m_refctr(0) {}
	class_info info() const { return class_info(&m_entry); }

	unsigned addref(const type_info &ti)
	{
		m_entry.addref(ti);
		RTTI_LOG("    <addref>        " << m_entry.tinfo().name() << " "
			<< m_refctr << "->" << m_refctr+1 << "\n");
		return ++m_refctr;
	}

	unsigned release(const type_info &ti)
	{
		RTTI_LOG("    <release>       " << m_entry.tinfo().name() << " "
			<< m_refctr << "->" << m_refctr-1 << "\n");
		m_entry.release(ti);
		return --m_refctr;
	}

	const rtti_entry &entry() const { return m_entry; }
	void set_index(unsigned i) { m_entry.set_index(i); }
};

//=====================================================================================//
//                          class rtti_service_implementation                          //
//=====================================================================================//
class rtti_service_implementation : public rtti_service
{
	typedef std::list<class_info_holder> class_map;
	typedef std::hash_map<std::string,class_info_holder *> named_class_map;
	class_map m_classes;
	named_class_map m_named_classes;

	std::string cut_unneeded_path(const char *);
	class_map::iterator create_class_object(const std::string &, class_info,
		const std::type_info &, const caster *);

public:
	rtti_service_implementation();

	virtual class_info register_class_object(const char *, const char *,
													class_info,
													const std::type_info &,
													const caster *);
	virtual class_info get_class_object(const char *name);
	virtual void unregister_class_object(const std::type_info &);
	virtual void dump_class_list();

	void validate_indexes();
};

#endif // !defined(__RTTI_H_INCLUDED_2023399421164473__)
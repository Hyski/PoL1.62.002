#include "precomp.h"

#include <mll/utils/oneobj.h>

namespace Storage
{

//=====================================================================================//
//                                class Registry::Impl                                 //
//=====================================================================================//
class RegistryImpl : public Registry
{
	typedef std::hash_map<std::string,PersistentClass *> Classes_t;
	Classes_t m_classes;

public:
	virtual void registerClass(PersistentClass *p)
	{
		if(m_classes.find(p->getClassInfo().name()) == m_classes.end())
		{
			m_classes.insert( std::make_pair(p->getClassInfo().name(), p) );
		}
	}

	virtual PersistentClass *getClass(const std::string &n) const
	{
		Classes_t::const_iterator persistentClassIterator = m_classes.find(n);
		assert( persistentClassIterator != m_classes.end() );
		if( persistentClassIterator == m_classes.end() ) return 0;
		return persistentClassIterator->second;
	}
};

//=====================================================================================//
//                           Registry *Registry::instance()                            //
//=====================================================================================//
Registry *Registry::instance()
{
	static mll::utils::oneobj<RegistryImpl> registry;
	return registry.get();
}

}
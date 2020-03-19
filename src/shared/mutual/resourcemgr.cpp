#define __BUILDING_MUTUAL_LIBRARY__

#ifndef MUTUAL_API
#	define MUTUAL_API	__declspec(dllexport)
#endif

#include <mutual/resourcemgr.h>
#include <mutual/resource.h>
#include <algorithm>

#include <boost/format.hpp>
#include <mll/debug/log.h>

#if defined(ML_MUTUAL_RESOURCEMGR_LOG)
#define rcmgr(List)	MLL_MAKE_LOG("rcmgr.log",List)
#else
#define rcmgr(List)
#endif

namespace
{

//=====================================================================================//
//                            class MlResourceSortPredicate                            //
//=====================================================================================//
struct MlResourceSortPredicate : public std::binary_function<MlResource*,MlResource*,bool>
{
	bool operator()(class_info ci_a,class_info ci_b) const
	{
		return ci_a < ci_b;
	}

	bool operator()(MlResource* a,MlResource* b) const
	{
		class_info ci_a = a->rtti_dynamic();
		class_info ci_b = b->rtti_dynamic();
		if(ci_a == ci_b) return a->name() < b->name();
		return ci_a < ci_b;
	}
};

//=====================================================================================//
//                          struct MlResourceSearchPredicate                           //
//=====================================================================================//
struct MlResourceSearchPredicate
{
	class_info m_x;

	MlResourceSearchPredicate(class_info ci) : m_x(ci) {}

	bool compare(class_info a,class_info b) const
	{
		if(a>=b) return false;
		else return !(can(a).downcast_to(m_x) && can(b).downcast_to(m_x));
	}

	bool operator()(class_info a,MlResource* b) const
	{
		return compare(a,b->rtti_dynamic());
	}

	bool operator()(MlResource* a,class_info b) const
	{
		return compare(a->rtti_dynamic(),b);
	}
};

}

class MlResourceMgr::Impl
{
public:
	MlResourceMgr::container_t m_container;
};

//=====================================================================================//
//                                 class MlResourceMgr                                 //
//=====================================================================================//
MlResourceMgr::MlResourceMgr()
:	m_pimpl(new Impl)
{
	assert( !m_instance );
	m_instance = this;
}

MlResourceMgr::~MlResourceMgr()
{
#if defined(ML_MUTUAL_RESOURCEMGR_LOG)
	rcmgr("\n...container content before destroying:\n");
	for(container_t::iterator it=m_pimpl->m_container.begin();it!=m_pimpl->m_container.end();++it)
	{
		rcmgr(boost::format("class [%s], name [%s]\n") % (*it)->rtti_dynamic().std_type_info().name() % (*it)->name());
	}
	rcmgr("\n");
#endif
	assert( !m_pimpl->m_container.size() );
	m_instance = 0;

	delete m_pimpl;
}
//	поместить ресурс в менеджер (доступно только для класса MlResource)
MlResourceMgr::container_t::iterator MlResourceMgr::insertResource(MlResource* value)
{
	assert( value );

	//	добавляем ресурс в контейнер 
	//	сортировка в контейнере идет по типам
	//	внутри типа сортировка идет по строковому идентификатору
	container_t::iterator before = std::lower_bound(m_pimpl->m_container.begin(),m_pimpl->m_container.end(),value,MlResourceSortPredicate());
#if defined(ML_MUTUAL_RESOURCEMGR_LOG)
	container_t::iterator result = m_pimpl->m_container.insert(before,value);
	rcmgr(boost::format("Container content after inserting resourse [%s]:[%s]:\n") % value->rtti_dynamic().std_type_info().name() % value->name());
	for(container_t::iterator it=m_pimpl->m_container.begin();it!=m_pimpl->m_container.end();++it)
	{
		rcmgr(boost::format("class [%s], name [%s]\n") % (*it)->rtti_dynamic().std_type_info().name() % (*it)->name());
	}
	rcmgr("\n");
	return result;
#else
	return m_pimpl->m_container.insert(before,value);
#endif
}
//	удалить ресурс из менеджера (доступно только для класса MlResource)
void MlResourceMgr::removeResource(container_t::iterator handle)
{
	rcmgr(boost::format("...removing resource [%s]:[%s];\n") % (*handle)->rtti_dynamic().std_type_info().name() % (*handle)->name());
	m_pimpl->m_container.erase(handle);
}
//	получить срез ресурсов по типу ресурса
MlResourceMgr::slice_t MlResourceMgr::slice(const class_info& ci)
{
#if defined(ML_MUTUAL_RESOURCEMGR_LOG)
	MlResourceMgr::slice_t _slice = std::equal_range(m_pimpl->m_container.begin(),m_pimpl->m_container.end(),ci,MlResourceSearchPredicate(ci));
	rcmgr(boost::format("...content of slice [%s]:\n") % ci.std_type_info().name());
	for(container_t::iterator it=_slice.first;it!=_slice.second;++it)
	{
		rcmgr(boost::format("class [%s], name [%s]\n") % (*it)->rtti_dynamic().std_type_info().name() % (*it)->name());
	}
	rcmgr("\n");
	return _slice;
#else
	return std::equal_range(m_pimpl->m_container.begin(),m_pimpl->m_container.end(),ci,MlResourceSearchPredicate(ci));
#endif
}

//=====================================================================================//
//                      MlResourceMgr* MlResourceMgr::m_instance                       //
//=====================================================================================//
MlResourceMgr* MlResourceMgr::m_instance = 0;
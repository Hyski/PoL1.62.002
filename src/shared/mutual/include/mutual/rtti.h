#if !defined(__RTTI_H_INCLUDED_8093570542662288__)
#define __RTTI_H_INCLUDED_8093570542662288__

#include <string>
#include <string.h>
#include <typeinfo>

#include <mutual/_export_rules.h>

#include <boost/type_traits.hpp>

class caster;
class class_info;

//=====================================================================================//
//                                 class rtti_service                                  //
//=====================================================================================//
class rtti_service
{
public:
	virtual class_info register_class_object(const char *, const char *, class_info,
													const std::type_info &,
													const caster *) = 0;
	virtual class_info get_class_object(const char *) = 0;
	virtual void unregister_class_object(const std::type_info &) = 0;

	virtual void dump_class_list() = 0;
};

MUTUAL_API rtti_service *get_rtti_service();

class rtti_entry;

//=====================================================================================//
//                                  class class_info                                   //
//=====================================================================================//
class MUTUAL_API class_info
{
	const rtti_entry *m_entry;

public:
	explicit class_info(const rtti_entry *e) : m_entry(e) {}

	const std::string &name() const;
	class_info parent() const;
	unsigned level() const;
	const std::type_info &std_type_info() const;

	const caster *get_caster() const;
	const rtti_entry *entry() const { return m_entry; }
	bool operator==(const class_info &rhs) const { return m_entry == rhs.m_entry; }
	bool operator!=(const class_info &rhs) const { return m_entry != rhs.m_entry; }
	operator const void * const() const { return m_entry; }
	bool operator<(const class_info &rhs) const;
	bool operator>(const class_info &rhs) const;
	bool operator<=(const class_info &rhs) const { return !(*this > rhs); }
	bool operator>=(const class_info &rhs) const { return !(*this < rhs); }
};

//=====================================================================================//
//                                    class caster                                     //
//=====================================================================================//
class caster
{
protected:
	caster() {}
	~caster() {}

public:
	virtual void *rtti_cast(void *, const class_info &) const = 0;
};

//=====================================================================================//
//                                  class caster_impl                                  //
//=====================================================================================//
template<typename T>
class caster_impl : public caster
{
public:
	virtual void *rtti_cast(void *obj, const class_info &type) const
	{
		return reinterpret_cast<T*>(obj)->rtti_cast_self(type);
	}
};

//=====================================================================================//
//                              class concrete_class_info                              //
//=====================================================================================//
template<typename T>
class concrete_class_info
{
	caster_impl<T> m_caster;
	class_info m_info;

public:
	concrete_class_info(const std::string &name, const std::string &file, class_info parent)
	:	m_info(get_rtti_service()->register_class_object
			(
				name.c_str(),
				file.c_str(),
				parent,
				typeid(T),
				&m_caster
			))
	{
	}
	~concrete_class_info() { get_rtti_service()->unregister_class_object(typeid(T)); }

	const class_info &info() const { return m_info; }
};

#define _ML_RTTI_METHOD_VIRTUAL			virtual
#define _ML_RTTI_METHOD_NON_VIRTUAL		

#define _ML_RTTI_SPEC_TEMPLATE			template<>
#define _ML_RTTI_SPEC_NON_TEMPLATE		

#define	_ML_RTTI_PARENT1_EXISTS(P)				P::rtti_static()
#define	_ML_RTTI_PARENT1_DOESNT_EXIST(P)		class_info(0)
#define	_ML_RTTI_PARENT2_EXISTS(P)				else { return P::rtti_cast_self(ci); }
#define	_ML_RTTI_PARENT2_DOESNT_EXIST(P)		return 0;

#define _ML_RTTI_TEMPLATE(C,N,P,PE,V)														\
	_ML_RTTI_METHOD_##V void *rtti_cast_self() { return reinterpret_cast<void*>(this); }	\
	_ML_RTTI_METHOD_##V void *rtti_cast_self(const class_info &ci)							\
	{																						\
		if(ci == rtti_static())																\
		{																					\
			return rtti_cast_self();														\
		}																					\
		_ML_RTTI_PARENT2_##PE(P);															\
	}																						\
	static const concrete_class_info<C> &rtti_get_cci()										\
	{																						\
		static concrete_class_info<C> theInfo(N,__FILE__,_ML_RTTI_PARENT1_##PE(P));			\
		return theInfo;																		\
	}																						\
	static class_info rtti_get_parent() { return _ML_RTTI_PARENT1_##PE(P); }				\
	static class_info rtti_static() { return rtti_get_cci().info(); }						\
	_ML_RTTI_METHOD_##V class_info rtti_dynamic() const { return rtti_static(); }

#define _ML_RTTI_TEMPLATE_IFACE(C,N,P,PE,V)													\
	_ML_RTTI_METHOD_##V void *rtti_cast_self() { return reinterpret_cast<void*>(this); }	\
	_ML_RTTI_METHOD_##V void *rtti_cast_self(const class_info &ci)							\
	{																						\
		if(ci == rtti_static())																\
		{																					\
			return rtti_cast_self();														\
		}																					\
		_ML_RTTI_PARENT2_##PE(P);															\
	}																						\
	static const concrete_class_info<C> &rtti_get_cci();									\
	static class_info rtti_get_parent() { return _ML_RTTI_PARENT1_##PE(P); }				\
	static class_info rtti_static() { return rtti_get_cci().info(); }						\
	_ML_RTTI_METHOD_##V class_info rtti_dynamic() const { return rtti_static(); }


#define _ML_RTTI_TEMPLATE_IMPL(C,N,P,PE,V,T)												\
	_ML_RTTI_SPEC_##T const concrete_class_info< C > & C ::rtti_get_cci()					\
	{																						\
		static concrete_class_info< C > theInfo(N,__FILE__,_ML_RTTI_PARENT1_##PE(P));		\
		return theInfo;																		\
	}

//=====================================================================================//
//                                      ML_RTTI_*                                      //
//=====================================================================================//
#define ML_RTTI_SINGLE(C)				_ML_RTTI_TEMPLATE(C,"",void,DOESNT_EXIST,NON_VIRTUAL)
#define ML_RTTI_CUSTOM_SINGLE(C,N)		_ML_RTTI_TEMPLATE(C,N,void,DOESNT_EXIST,NON_VIRTUAL)
#define ML_RTTI_ROOT(C)					_ML_RTTI_TEMPLATE(C,"",void,DOESNT_EXIST,VIRTUAL)
#define ML_RTTI_CUSTOM_ROOT(C,N)		_ML_RTTI_TEMPLATE(C,N,void,DOESNT_EXIST,VIRTUAL)
#define ML_RTTI(C,P)					_ML_RTTI_TEMPLATE(C,"",P,EXISTS,VIRTUAL)
#define ML_RTTI_CUSTOM(C,N,P)			_ML_RTTI_TEMPLATE(C,N,P,EXISTS,VIRTUAL)

#define ML_RTTI_SINGLE_IFACE(C)				_ML_RTTI_TEMPLATE_IFACE(C,"",void,DOESNT_EXIST,NON_VIRTUAL)
#define ML_RTTI_CUSTOM_SINGLE_IFACE(C,N)	_ML_RTTI_TEMPLATE_IFACE(C,N,void,DOESNT_EXIST,NON_VIRTUAL)
#define ML_RTTI_ROOT_IFACE(C)				_ML_RTTI_TEMPLATE_IFACE(C,"",void,DOESNT_EXIST,VIRTUAL)
#define ML_RTTI_CUSTOM_ROOT_IFACE(C,N)		_ML_RTTI_TEMPLATE_IFACE(C,N,void,DOESNT_EXIST,VIRTUAL)
#define ML_RTTI_IFACE(C,P)					_ML_RTTI_TEMPLATE_IFACE(C,"",P,EXISTS,VIRTUAL)
#define ML_RTTI_CUSTOM_IFACE(C,N,P)			_ML_RTTI_TEMPLATE_IFACE(C,N,P,EXISTS,VIRTUAL)

#define ML_RTTI_SINGLE_IMPL(C)				_ML_RTTI_TEMPLATE_IMPL(C,"",void,DOESNT_EXIST,NON_VIRTUAL,NON_TEMPLATE)
#define ML_RTTI_CUSTOM_SINGLE_IMPL(C,N)		_ML_RTTI_TEMPLATE_IMPL(C,N,void,DOESNT_EXIST,NON_VIRTUAL,NON_TEMPLATE)
#define ML_RTTI_ROOT_IMPL(C)				_ML_RTTI_TEMPLATE_IMPL(C,"",void,DOESNT_EXIST,VIRTUAL,NON_TEMPLATE)
#define ML_RTTI_CUSTOM_ROOT_IMPL(C,N)		_ML_RTTI_TEMPLATE_IMPL(C,N,void,DOESNT_EXIST,VIRTUAL,NON_TEMPLATE)
#define ML_RTTI_IMPL(C,P)					_ML_RTTI_TEMPLATE_IMPL(C,"",P,EXISTS,VIRTUAL,NON_TEMPLATE)
#define ML_RTTI_CUSTOM_IMPL(C,N,P)			_ML_RTTI_TEMPLATE_IMPL(C,N,P,EXISTS,VIRTUAL,NON_TEMPLATE)

#define ML_RTTI_SINGLE_TEMPLATE_IMPL(C)				_ML_RTTI_TEMPLATE_IMPL(C,"",void,DOESNT_EXIST,NON_VIRTUAL,TEMPLATE)
#define ML_RTTI_CUSTOM_SINGLE_TEMPLATE_IMPL(C,N)	_ML_RTTI_TEMPLATE_IMPL(C,N,void,DOESNT_EXIST,NON_VIRTUAL,TEMPLATE)
#define ML_RTTI_ROOT_TEMPLATE_IMPL(C)				_ML_RTTI_TEMPLATE_IMPL(C,"",void,DOESNT_EXIST,VIRTUAL,TEMPLATE)
#define ML_RTTI_CUSTOM_ROOT_TEMPLATE_IMPL(C,N)		_ML_RTTI_TEMPLATE_IMPL(C,N,void,DOESNT_EXIST,VIRTUAL,TEMPLATE)
#define ML_RTTI_TEMPLATE_IMPL(C,P)					_ML_RTTI_TEMPLATE_IMPL(C,"",P,EXISTS,VIRTUAL,TEMPLATE)
#define ML_RTTI_CUSTOM_TEMPLATE_IMPL(C,N,P)			_ML_RTTI_TEMPLATE_IMPL(C,N,P,EXISTS,VIRTUAL,TEMPLATE)

//=====================================================================================//
//                                  rtti_class_info()                                  //
//=====================================================================================//
template<typename T>
inline class_info rtti_object_info(const T *t)
{
	return t->rtti_dynamic();
}

//=====================================================================================//
//                                 rtti_parent_info()                                  //
//=====================================================================================//
template<typename T>
inline class_info rtti_object_parent_info(const T *t)
{
	return t->rtti_dynamic()->parent();
}

//=====================================================================================//
//                               struct rtti_class_info                                //
//=====================================================================================//
//template<typename T>
//struct rtti_class_info
//{
//public:
//	rtti_class_info() {}
//	operator const class_info *() const { return T::rtti_static(); }
//	const class_info *operator->() const { return T::rtti_static(); }
//	const class_info &operator*() const { return *T::rtti_static(); }
//};

//=====================================================================================//
//                               struct rtti_parent_info                               //
//=====================================================================================//
//template<typename T>
//struct rtti_class_parent_info
//{
//public:
//	rtti_class_parent_info() {}
//	operator const class_info *() const { return T::rtti_static()->parent(); }
//	const class_info *operator->() const { return T::rtti_static()->parent(); }
//	const class_info &operator*() const { return *T::rtti_static()->parent(); }
//};

//=====================================================================================//
//                                      class can                                      //
//=====================================================================================//
class can
{
	class_info m_info;

public:
	can(class_info info) : m_info(info) {}
	bool downcast_to(class_info info) const;
};

//=====================================================================================//
//                        inline bool can::downcast_to() const                         //
//=====================================================================================//
inline bool can::downcast_to(class_info to) const
{
	class_info from = m_info;

	if(from < to) return false;
	while(from && from != to) from = from.parent();
	return from && from == to;
}

//=====================================================================================//
//                              class rtti_slice_compare                               //
//=====================================================================================//
class rtti_slice_compare
{
	class_info m_x;

public:
	rtti_slice_compare(const class_info &ci) : m_x(ci) {}

	bool operator()(const class_info &a, const class_info &b) const
	{
		if(a>=b) return false;
		else return !(can(a).downcast_to(m_x) && can(b).downcast_to(m_x));
	}
};

//=====================================================================================//
//                                    T rtti_cast()                                    //
//=====================================================================================//
template<typename T, typename R>
T rtti_cast(R *obj)
{
	typedef typename boost::remove_pointer<T>::type obj_t;
	if(can(obj->rtti_dynamic()).downcast_to(obj_t::rtti_static()))
	{
		return static_cast<T>(obj);
	}
	return 0;
}

//=====================================================================================//
//                                    T rtti_cast()                                    //
//=====================================================================================//
template<typename T, typename R>
T rtti_cast(R &obj)
{
	typedef typename boost::remove_reference<T>::type obj_t;
	if(can(obj->rtti_dynamic()).downcast_to(obj_t::rtti_static()))
	{
		return static_cast<T>(obj);
	}
	throw std::bad_cast();
}

//=====================================================================================//
//                                    T rtti_cast()                                    //
//=====================================================================================//
template<typename T, typename R>
T rtti_cast(const R *obj)
{
	typedef typename boost::remove_pointer<T>::type obj_t;
	if(can(obj->rtti_dynamic()).downcast_to(obj_t::rtti_static()))
	{
		return static_cast<T>(obj);
	}
	return 0;
}

//=====================================================================================//
//                                    T rtti_cast()                                    //
//=====================================================================================//
template<typename T, typename R>
T rtti_cast(const R &obj)
{
	typedef typename boost::remove_reference<T>::type obj_t;
	if(can(obj->rtti_dynamic()).downcast_to(obj_t::rtti_static()))
	{
		return static_cast<T>(obj);
	}
	throw std::bad_cast();
}

struct null_type { ML_RTTI_SINGLE(null_type); };

#endif // !defined(__RTTI_H_INCLUDED_8093570542662288__)
#pragma once

#include "Storage.h"

namespace Storage
{

//=====================================================================================//
//                                  class Registrator                                  //
//=====================================================================================//
template<typename T>
class Registrator
{
	T m_obj;
public:
	Registrator() { Registry::instance()->registerClass(&m_obj); }
	T *get() { return &m_obj; }
};

//=====================================================================================//
//                                 class AnyReferencer                                 //
//=====================================================================================//
class AnyReferencer
{
	const void *const m_ptr;
public:
	template<typename T> AnyReferencer(const T &ref) : m_ptr(&ref) {}
	template<typename T> AnyReferencer(T (*ref)()) : m_ptr((*ref)()) { }
};

#define __ML_PC_CAT2(A,B,C)	A ## B ## C
#define __ML_PC_CAT(A,B,C)	__ML_PC_CAT2(A,B,C)

#define ML_PERSISTENT_STATIC(T)	\
	virtual Storage::PersistentClass *getClass() const { return getClassStatic(); }	\
	static Storage::PersistentClass *getClassStatic() { static ::Storage::Registrator< ::Storage::PersistentClassImpl< T > > a; return a.get(); }

#define ML_PERSISTENT_HIMPL_STATIC(T)	\
	static ::Storage::AnyReferencer __ML_PC_CAT(__storage_g_referencer,__LINE__,T)(T::getClassStatic);

#define ML_PERSISTENT_TEMPLATE_HIMPL_STATIC(T)	\
	static ::Storage::AnyReferencer __ML_PC_CAT(__storage_g_referencer,__LINE__,T)(T::getClassStatic);

#define ML_PERSISTENT_IMPL_STATIC(T)
#define ML_PERSISTENT_TEMPLATE_IMPL_STATIC(T)

#define ML_PERSISTENT(T)	\
	virtual Storage::PersistentClass *getClass() const { return getClassStatic(); }	\
	static Storage::PersistentClass *getClassStatic()

#define ML_PERSISTENT_HIMPL(T)
#define ML_PERSISTENT_TEMPLATE_HIMPL(T)

#define ML_PERSISTENT_IMPL(T)	\
	static Storage::Registrator< Storage::PersistentClassImpl< T > > __ML_PC_CAT(g_,T,storageClassObject);	\
	Storage::PersistentClass * T :: getClassStatic() { return __ML_PC_CAT(g_,T,storageClassObject) .get(); }

#define ML_PERSISTENT_TEMPLATE_IMPL(T)	\
	static Storage::Registrator< Storage::PersistentClassImpl< T > > __ML_PC_CAT(g_,T,storageClassObject);	\
	template<> Storage::PersistentClass * T :: getClassStatic() { return __ML_PC_CAT(g_,T,storageClassObject) .get(); }

#define ML_PERSISTENT_HELPER0(R,T)		R(T)
#define ML_PERSISTENT_HELPER1(R,T)		R##_STATIC(T)
#define ML_PERSISTENT_APPLY(R,S,T)		ML_PERSISTENT_HELPER##S(R,T)

#define ML_PERSISTENT2(T,S)						ML_PERSISTENT_APPLY(ML_PERSISTENT,S,T)
#define ML_PERSISTENT_IMPL2(T,S)				ML_PERSISTENT_APPLY(ML_PERSISTENT_IMPL,S,T)
#define ML_PERSISTENT_TEMPLATE_IMPL2(T,S)		ML_PERSISTENT_APPLY(ML_PERSISTENT_TEMPLATE_IMPL,S,T)
#define ML_PERSISTENT_HIMPL2(T,S)				ML_PERSISTENT_APPLY(ML_PERSISTENT_HIMPL,S,T)
#define ML_PERSISTENT_TEMPLATE_HIMPL2(T,S)		ML_PERSISTENT_APPLY(ML_PERSISTENT_TEMPLATE_HIMPL,S,T)

}
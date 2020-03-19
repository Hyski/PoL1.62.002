#pragma once

#include "Storage.h"

//=====================================================================================//
//                                  namespace Storage                                  //
//=====================================================================================//
namespace Storage
{

class ReadSlot;

//=====================================================================================//
//                                class PersistentClass                                //
//=====================================================================================//
class PersistentClass
{
public:
	virtual class_info getClassInfo() const = 0;
//	virtual PersistentBase *construct(Repository *) = 0;
	virtual PersistentBase *restore(Repository *, ReadSlot &) = 0;
};

//=====================================================================================//
//                 class PersistentClassImpl : public PersistentClass                  //
//=====================================================================================//
template<typename T>
class PersistentClassImpl : public PersistentClass
{
public:
	virtual class_info getClassInfo() const { return T::rtti_static(); }
//	virtual PersistentBase *construct(Repository *r) { return r->construct<T>(); }
	virtual PersistentBase *restore(Repository *r, ReadSlot &s)
	{
		return r->construct<T,ReadSlot&>(s);
	}
};

}
#pragma once

#include "Storage.h"

namespace Storage
{

class WriteSlot;
class ReadSlot;

//=====================================================================================//
//                                class PersistentBase                                 //
//=====================================================================================//
class STORAGE_API PersistentBase
{
	HPtrJunction m_junction;

public:
	PersistentBase();
	PersistentBase(ReadSlot &);
	virtual ~PersistentBase();

	void *operator new(size_t);
	void operator delete(void *);

	virtual void store(WriteSlot &) const { assert( !"PersistentBase::store() is empty!" ); }
//	virtual void restore(ReadSlot &) { assert( !"PersistentBase::restore() is empty!" ); }

	virtual PersistentClass *getClass() const = 0;
	static PersistentClass *getClassStatic() { return 0; }

private:
	friend class Repository;
	friend class PtrBase;

	void *operator new(size_t, Repository *);
	void operator delete(void *, Repository *);

	static void *doAlloc(size_t);
	static void *doAlloc(size_t, Repository *);
	static void doFree(void *);

	HPtrJunction getPtrJunction() const { return m_junction; }
	void getPtrJunction(HPtrJunction p) { m_junction = p; }
};

//=====================================================================================//
//                        void *PersistentBase::operator new()                         //
//=====================================================================================//
inline void *PersistentBase::operator new(size_t s, Repository *st)
{
	return doAlloc(s,st);
}

//=====================================================================================//
//                        void *PersistentBase::operator new()                         //
//=====================================================================================//
inline void *PersistentBase::operator new(size_t s)
{
	return doAlloc(s);
}

//=====================================================================================//
//                       void PersistentBase::operator delete()                        //
//=====================================================================================//
inline void PersistentBase::operator delete(void *ptr)
{
	doFree(ptr);
}

//=====================================================================================//
//                       void PersistentBase::operator delete()                        //
//=====================================================================================//
inline void PersistentBase::operator delete(void *ptr, Repository *)
{
	doFree(ptr);
}

}
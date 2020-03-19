#include "precomp.h"

namespace Storage
{

//=====================================================================================//
//                           void *PersistentBase::doAlloc()                           //
//=====================================================================================//
void *PersistentBase::doAlloc(size_t s)
{
	return Repository::allocateStatic(s);
}

//=====================================================================================//
//                           void *PersistentBase::doAlloc()                           //
//=====================================================================================//
void *PersistentBase::doAlloc(size_t s, Repository *storage)
{
	return storage->allocate(s);
}

//=====================================================================================//
//                            void PersistentBase::doFree()                            //
//=====================================================================================//
void PersistentBase::doFree(void *a)
{
	Repository::deallocateStatic(a);
}

//=====================================================================================//
//                          PersistentBase::PersistentBase()                           //
//=====================================================================================//
PersistentBase::PersistentBase()
{
	m_junction.reset(new PtrJunction(this));
}

//=====================================================================================//
//                          PersistentBase::PersistentBase()                           //
//=====================================================================================//
PersistentBase::PersistentBase(ReadSlot &s)
:	m_junction(s.getPtrJunction())
{
	m_junction->reset(this);
}

//=====================================================================================//
//                          PersistentBase::~PersistentBase()                          //
//=====================================================================================//
PersistentBase::~PersistentBase()
{
	if(m_junction) m_junction->reset();
}

}
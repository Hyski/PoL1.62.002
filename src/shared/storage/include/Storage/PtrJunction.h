#pragma once

#include "Storage.h"

namespace Storage
{

//=====================================================================================//
//                          class PtrJunction : MlRefCounted                           //
//=====================================================================================//
class PtrJunction : MlRefCounted
{
	PersistentBase *m_pointer;

public:
	PtrJunction() : m_pointer(0) {}
	PtrJunction(PersistentBase *base) : m_pointer(base) {}

	PersistentBase *getObject() const { return m_pointer; }
	bool expired() const { return m_pointer == 0; }

private:
	friend class PersistentBase;
	friend class MlHandle<PtrJunction>;

	void reset(PersistentBase *p = 0) { m_pointer = p; }
};

typedef MlHandle<PtrJunction> HPtrJunction;

}
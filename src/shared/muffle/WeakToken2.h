#pragma once

#include "Lock2.h"

namespace Muffle2
{

class Synchronized;

//=====================================================================================//
//                                  class WeakToken2                                   //
//=====================================================================================//
class WeakToken
{
	CriticalSection m_cssys;
	CriticalSection m_cs;
	Synchronized *m_object;
	long m_weakRefCount;
	long m_refCount;

public:
	WeakToken(Synchronized *obj);
	~WeakToken();

	Lock lock() { return Lock(&m_cs); }
	Lock lockSys() { return Lock(&m_cssys); }

	void addRef();
	void release();

	void addWeakRef();
	void releaseWeak();

	WeakToken *acquirePtrFromWeak();

	Synchronized *getObject() const { return m_object; }
	int getWeakRefCount() const { return m_weakRefCount; }
	int getRefCount() const { return m_refCount; }

private:
	void dispose();
};


}

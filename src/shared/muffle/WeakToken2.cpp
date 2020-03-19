#include "precomp.h"
#include "WeakToken2.h"
#include "Synchronized2.h"

namespace Muffle2
{

//=====================================================================================//
//                               WeakToken::WeakToken()                                //
//=====================================================================================//
WeakToken::WeakToken(Synchronized *obj)
:	m_object(obj),
	m_weakRefCount(1),
	m_refCount(1)
{
}

//=====================================================================================//
//                               WeakToken::~WeakToken()                               //
//=====================================================================================//
WeakToken::~WeakToken()
{
}

//=====================================================================================//
//                              void WeakToken2::addRef()                              //
//=====================================================================================//
void WeakToken::addRef()
{
	Lock guard = lockSys();

	m_refCount++;
	m_weakRefCount++;
}

//=====================================================================================//
//                            void WeakToken2::addWeakRef()                            //
//=====================================================================================//
void WeakToken::addWeakRef()
{
	Lock guard = lockSys();

	m_weakRefCount++;
}

//=====================================================================================//
//                             void WeakToken2::release()                              //
//=====================================================================================//
void WeakToken::release()
{
	Lock guard = lockSys();

	bool need_dispose = false;
	bool need_self_destruct = false;

	if(!--m_refCount)
	{
		need_dispose = true;
	}

	if(!--m_weakRefCount)
	{
		need_self_destruct = true;
	}


	if(need_dispose)
	{
		Synchronized *myobj = m_object;
		m_object = 0;

		guard.reset();

		delete myobj;
		if(need_self_destruct)
		{
			delete this;
		}
	}
}

//=====================================================================================//
//                           void WeakToken2::releaseWeak()                            //
//=====================================================================================//
void WeakToken::releaseWeak()
{
	Lock guard = lockSys();

	if(!--m_weakRefCount)
	{
		guard.reset();
		delete this;
	}
}

//=====================================================================================//
//                             void WeakToken2::dispose()                              //
//=====================================================================================//
void WeakToken::dispose()
{
	delete m_object;
	m_object = 0;
}

//=====================================================================================//
//                    WeakToken2 *WeakToken2::acquirePtrFromWeak()                     //
//=====================================================================================//
WeakToken *WeakToken::acquirePtrFromWeak()
{
	Lock guard = lockSys();

	if(!m_refCount) return 0;
	++m_refCount;
	++m_weakRefCount;
	return this;
}


}

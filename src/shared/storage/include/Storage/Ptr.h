#pragma once

#include "PtrJunction.h"
#include "PersistentBase.h"

//=====================================================================================//
//                                  namespace Storage                                  //
//=====================================================================================//
namespace Storage
{

//=====================================================================================//
//                                    class PtrBase                                    //
//=====================================================================================//
class PtrBase
{
	HPtrJunction m_junct;

public:
	PtrBase() {}
	PtrBase(PersistentBase *pb) : m_junct(pb?pb->getPtrJunction():0) {}
	PtrBase(HPtrJunction j) : m_junct(j) {}

	bool expired() const { return !m_junct || m_junct->expired(); }
	const HPtrJunction &getJunction() const { return m_junct; }

	void reset(HPtrJunction j)
	{
		m_junct = j;
	}

	void reset(PersistentBase *pb)
	{
		m_junct = pb?pb->getPtrJunction():0;
	}

	PersistentBase *get() const
	{
		return m_junct->getObject();
	}
};

//=====================================================================================//
//                                   class PtrHelper                                   //
//=====================================================================================//
template<typename P, typename T>
struct PtrHelper
{
	friend bool operator==(const P &l, const P &r)
	{
		return l.get() == r.get();
	}

	friend bool operator==(const P &l, const T *r)
	{
		return l.get() == r;
	}

	friend bool operator<(const P &l, const P &r)
	{
		return l.get() < r.get();
	}

	friend bool operator<(const P &l, const T *r)
	{
		return l.get() < r;
	}

	friend bool operator>(const P &l, const T *r)
	{
		return l.get() > r;
	}
};

//=====================================================================================//
//                                      class Ptr                                      //
//=====================================================================================//
template<typename T>
class Ptr : PtrBase,
	public PtrHelper<Ptr,T>,
	public boost::equality_comparable<Ptr>,
	public boost::equality_comparable<Ptr,const T *>,
	public boost::partially_ordered<Ptr>,
	public boost::partially_ordered<Ptr,const T *>
{
public:
	Ptr() {}
	Ptr(T *ptr) : PtrBase(ptr) {}
	explicit Ptr(HPtrJunction j) : PtrBase(j) {}

	T *operator->() const
	{
		return get();
	}

	bool expired() const { return PtrBase::expired(); }

	T *fastGet() const 
	{
		return static_cast<T*>(PtrBase::get());
	}

	T *get() const 
	{
		return expired() ? 0 : static_cast<T*>(PtrBase::get());
	}

	Ptr &operator=(T *p) { PtrBase::reset(p); return *this; }

	const HPtrJunction &getPtrJunction() const { return PtrBase::getJunction(); }
};

//=====================================================================================//
//                              class AutoPtr : PtrBase,                               //
//=====================================================================================//
template<typename T>
class AutoPtr : PtrBase,
	public PtrHelper<AutoPtr,T>,
	public boost::equality_comparable<AutoPtr>,
	public boost::equality_comparable<AutoPtr,const T *>,
	public boost::partially_ordered<AutoPtr>,
	public boost::partially_ordered<AutoPtr,const T *>
{
public:
	AutoPtr() {}
	AutoPtr(AutoPtr &p) : PtrBase(p) { p.release(); }
	explicit AutoPtr(T *ptr) : PtrBase(ptr) {}
	explicit AutoPtr(Ptr<T> ptr) : PtrBase(ptr) {}
	explicit AutoPtr(HPtrJunction j) : PtrBase(j) {}
	~AutoPtr() { if(!expired()) delete get(); }

	T *operator->() const
	{
		return get();
	}

	bool expired() const { return PtrBase::expired(); }

	T *get() const 
	{
		return expired() ? 0 : static_cast<T*>(PtrBase::get());
	}

	void reset(T *p = 0)
	{
		if(!expired() && get() != p)
		{
			delete get();
		}
		PtrBase::reset(p);
	}

	void reset(Ptr<T> p)
	{
		if(!expired() && !p.expired() && get() != p)
		{
			delete get();
		}
		PtrBase::reset(p.getPtrJunction());
	}

	void release()
	{
		PtrBase::reset(0);
	}

	const HPtrJunction &getPtrJunction() const { return PtrBase::getJunction(); }
};

}
#pragma once

#include "WeakToken2.h"

namespace Muffle2
{

//=====================================================================================//
//                                 struct PtrStrategy                                  //
//=====================================================================================//
struct PtrStrategy
{
	static void addref(WeakToken *token) { token->addRef(); }
	static void release(WeakToken *token) {token->release(); }
};

//=====================================================================================//
//                               struct WeakPtrStrategy                                //
//=====================================================================================//
struct WeakPtrStrategy
{
	static void addref(WeakToken *token) { token->addWeakRef(); }
	static void release(WeakToken *token) { token->releaseWeak(); }
};

//=====================================================================================//
//                                 class CommonPtrBase                                 //
//=====================================================================================//
class CommonPtrBase
{
protected:
	struct AddRefedToken
	{
		WeakToken *token;
		AddRefedToken(WeakToken *tok) : token(tok) {}
		template<typename T> AddRefedToken(T *obj) : token(obj->getWeakToken()) {}
	};
};

//=====================================================================================//
//                                   class CommonPtr                                   //
//=====================================================================================//
template<typename St>
class CommonPtr : public CommonPtrBase
{
	WeakToken *m_token;

protected:
	CommonPtr() : m_token(0) { }
	CommonPtr(const AddRefedToken &tok) : m_token(tok.token) { }
	CommonPtr(WeakToken *token) : m_token(token) { addref(); }
	CommonPtr(const CommonPtr &s) : m_token(s.getToken()) { addref(); }
	template<typename T> CommonPtr(T *obj) : m_token(obj ? obj->getWeakToken() : 0) { addref(); }
	template<typename St2> CommonPtr(const CommonPtr<St2> &s) : m_token(s.getToken()) { addref(); }
	~CommonPtr() { release(); }

protected:
	template<typename T>
	void assign(T *obj)
	{
		WeakToken *newtoken = obj ? obj->getWeakToken() : 0;
		reset(newtoken);
	}

	template<typename St2>
	void assign(const CommonPtr<St2> &s)
	{
		reset(s.getToken());
	}

public:
	/// Возвращает токен
	WeakToken *getToken() const { return m_token; }
	/// Возвращает объект
	Synchronized *getObject() const { return m_token->getObject(); }

	typedef WeakToken * CommonPtr::*unspecified_bool_type;

    operator unspecified_bool_type() const
    {
        return m_token == 0 ? 0: &CommonPtr::m_token;
    }

	bool operator!() const
	{
		return m_token == 0;
	}

	void swap(CommonPtr &p)
	{
		std::swap(m_token,p.m_token);
	}

private:
	void reset(WeakToken *token)
	{
		if(!m_token || m_token != token)
		{
			release();
			m_token = token;
			addref();
		}
	}

	void addref() { if(m_token) St::addref(m_token); }
	void release() { if(m_token) { St::release(m_token); m_token = 0; } }
};

//=====================================================================================//
//                      class Ptr : public CommonPtr<PtrStrategy>                      //
//=====================================================================================//
template<typename T>
class Ptr : public CommonPtr<PtrStrategy>
{
	typedef CommonPtr<PtrStrategy> Base_t;

public:
	Ptr() {}
	Ptr(T *obj) : Base_t(obj) {}
	Ptr(const Ptr &p) : Base_t(p) {}
	Ptr(const AddRefedToken &tok) : Base_t(tok) {}
	template<typename T2> Ptr(const Ptr<T2> &p) : Base_t(p) { T *typeCheck = p.get(); }

	Ptr &operator=(const Ptr &p)
	{
		assign(p);
		return *this;
	}

	Ptr &operator=(T *obj)
	{
		assign(obj);
		return *this;
	}

	T *operator->() const
	{
		assert( get() != 0 );
		return get();
	}

	/// Возвращает подконтрольный объект
	T *get() const
	{
		return getToken() ? static_cast<T*>(getObject()) : 0;
	}

	bool operator==(const T *obj) const
	{
		return getToken() == obj->getWeakToken();
	}

	bool operator!=(const T *obj) const
	{
		return getToken() != obj->getWeakToken();
	}

	void reset(T *obj = 0)
	{
		assign(obj);
	}

	using Base_t::swap;

	static Ptr<T> create()
	{
		AddRefedToken tok(new T);
		return Ptr<T>(tok);
	}

	template<typename T1>
	static Ptr<T> create(T1 t1)
	{
		AddRefedToken tok(new T(t1));
		return Ptr<T>(tok);
	}

	template<typename T1, typename T2>
	static Ptr<T> create(T1 t1, T2 t2)
	{
		AddRefedToken tok(new T(t1,t2));
		return Ptr<T>(tok);
	}

	template<typename T1, typename T2, typename T3>
	static Ptr<T> create(T1 t1, T2 t2, T3 t3)
	{
		AddRefedToken tok(new T(t1,t2,t3));
		return Ptr<T>(tok);
	}

	template<typename T1, typename T2, typename T3, typename T4>
	static Ptr<T> create(T1 t1, T2 t2, T3 t3, T4 t4)
	{
		AddRefedToken tok(new T(t1,t2,t3,t4));
		return Ptr<T>(tok);
	}

	template<typename T1, typename T2, typename T3, typename T4, typename T5>
	static Ptr<T> create(T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
	{
		AddRefedToken tok(new T(t1,t2,t3,t4,t5));
		return Ptr<T>(tok);
	}
};

//=====================================================================================//
//                  class WeakPtr : public CommonPtr<WeakPtrStrategy>                  //
//=====================================================================================//
template<typename T>
class WeakPtr : public CommonPtr<WeakPtrStrategy>
{
	typedef CommonPtr<WeakPtrStrategy> Base_t;

public:
	typedef Ptr<T> Ptr_t;

public:
	WeakPtr() {}
	WeakPtr(T *obj) : Base_t(obj) {}
	WeakPtr(const Ptr<T> &p) : Base_t(p) {}
	WeakPtr(const WeakPtr &p) : Base_t(p) {}

	WeakPtr &operator=(const WeakPtr &p)
	{
		assign(p);
		return *this;
	}

	WeakPtr &operator=(T *obj)
	{
		assign(obj);
		return *this;
	}

	bool operator==(const T *obj) const
	{
		return getToken() == obj->getWeakToken();
	}

	bool operator!=(const T *obj) const
	{
		return getToken() != obj->getWeakToken();
	}

	bool expired() const
	{
		return !getToken() || getToken()->getRefCount() == 0;
	}

	Ptr<T> acquire() const
	{
		if(getToken())
		{
			if(WeakToken *token = getToken()->acquirePtrFromWeak())
			{
				return Ptr<T>(AddRefedToken(token));
			}
		}

		return Ptr<T>();
	}

	void reset(T *obj = 0)
	{
		assign(obj);
	}

	using Base_t::swap;
};

}

#pragma once

namespace PoL
{

//=====================================================================================//
//                                   class NotifyFn                                    //
//=====================================================================================//
template<typename Fn>
class NotifyFn
{
public:
	class Connection;
	typedef Fn Fn_t;
	friend class Connection;

private:
	typedef std::list<Fn_t> Fns_t;
	Fns_t m_fns;

public:
	/// Добавить коллбэк
	Connection addFn(Fn_t fn)
	{
		return Connection(this,m_fns.insert(m_fns.end(),fn));
	}

	void notify()
	{
		for(Fns_t::iterator i = m_fns.begin(); i != m_fns.end(); ++i)
		{
			(*i)();
		}
	}

	template<typename T>
	void notify(T t)
	{
		for(Fns_t::iterator i = m_fns.begin(); i != m_fns.end(); ++i)
		{
			(*i)(t);
		}
	}
};

//=====================================================================================//
//                           class NotifyFn<Fn>::Connection                            //
//=====================================================================================//
template<typename Fn>
class NotifyFn<Fn>::Connection
{
	friend class NotifyFn;
	Connection(NotifyFn *nfn, typename Fns_t::iterator i) : m_nfn(nfn), m_itor(i) {}

	NotifyFn *m_nfn;
	typename Fns_t::iterator m_itor;

public:
	Connection() : m_nfn(0) {}
	Connection(Connection &n) : m_nfn(n.m_nfn), m_itor(n.m_itor) { n.m_nfn = 0; }

	Connection &operator=(Connection &n)
	{
		if(this != &n)
		{
			Connection copy(n);
			swap(copy);
		}
		return *this;
	}

	~Connection()
	{
		disconnect();
	}

	void swap(Connection &c)
	{
		std::swap(m_nfn,c.m_nfn);
		std::swap(m_itor,c.m_itor);
	}

	void reset()
	{
		disconnect();
	}

private:
	void disconnect()
	{
		if(m_nfn)
		{
			m_nfn->m_fns.erase(m_itor);
			m_nfn = 0;
		}
	}
};

}
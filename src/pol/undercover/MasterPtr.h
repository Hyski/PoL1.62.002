#pragma once

//=====================================================================================//
//                                   class MasterPtr                                   //
//=====================================================================================//
/// Ведущий указатель
template<typename T>
class MasterPtr
{
	T *m_ptr;

public:
	MasterPtr() : m_ptr(new T) {}
	explicit MasterPtr(T *p) : m_ptr(p) {}
	MasterPtr(const MasterPtr &ptr) : m_ptr(new T(*ptr.m_ptr)) {}

	template<typename T1>
	MasterPtr(T1 p1) : T(new T(p1)) {}
	template<typename T1, typename T2>
	MasterPtr(T1 p1, T2 p2) : T(new T(p1,p2)) {}
	template<typename T1, typename T2, typename T3>
	MasterPtr(T1 p1, T2 p2, T3 p3) : T(new T(p1,p2,p3)) {}

	~MasterPtr() { delete m_ptr; }

	T *operator->() { return m_ptr; }
	const T *operator->() const { return m_ptr; }
	T &operator*() { return *m_ptr; }
	const T &operator*() const { return *m_ptr; }

	MasterPtr &swap(MasterPtr &p)
	{
		std::swap(m_ptr,p.m_ptr);
		return *this;
	}

	MasterPtr &operator=(const MasterPtr &p)
	{
		return swap(MasterPtr(p));
	}
};
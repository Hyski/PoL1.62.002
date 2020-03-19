#pragma once

#include "Storage.h"

namespace Storage
{

class RepositoryIteratorBase;
class RepositoryConstIteratorBase;

//=====================================================================================//
//                          class RepositoryConstIteratorBase                          //
//=====================================================================================//
class STORAGE_API RepositoryConstIteratorBase
{
	friend class Repository;

	class Impl;
	Impl *m_pimpl;

public:
	RepositoryConstIteratorBase();
	RepositoryConstIteratorBase(const RepositoryIteratorBase &);
	RepositoryConstIteratorBase(const RepositoryConstIteratorBase &);
	~RepositoryConstIteratorBase();

protected:
	void next();
	bool equals(const RepositoryConstIteratorBase &) const;
	const PersistentBase *deref() const;
};

//=====================================================================================//
//                            class RepositoryIteratorBase                             //
//=====================================================================================//
class STORAGE_API RepositoryIteratorBase
{
	friend class Repository;
	friend class RepositoryConstIteratorBase;

	class Impl;
	Impl *m_pimpl;

public:
	RepositoryIteratorBase();
	RepositoryIteratorBase(const RepositoryIteratorBase &);
	~RepositoryIteratorBase();

protected:
	void next();
	bool equals(const RepositoryIteratorBase &) const;
	PersistentBase *deref() const;
};

template<typename T> struct RepItHelper { static class_info getClassInfo() { return T::rtti_static(); } };
template<> struct RepItHelper<PersistentBase> { static class_info getClassInfo() { return class_info(0); } };

//=====================================================================================//
//              class RepositoryIterator : public RepositoryIteratorBase               //
//=====================================================================================//
template<typename T>
class RepositoryIterator : public RepositoryIteratorBase,
	public boost::incrementable<RepositoryIterator>,
	public boost::equality_comparable<RepositoryIterator>,
	public std::iterator<std::forward_iterator_tag, T *>
{
	T mutable * m_ptr;

public:
	RepositoryIterator() : RepositoryIteratorBase(RepItHelper<T>::getClassInfo()) {}
	RepositoryIterator(const RepositoryIteratorBase &a) : RepositoryIteratorBase(a) {}

	RepositoryIterator &operator++()
	{
		next();
		return *this;
	}

	bool operator==(const RepositoryIterator &b) const
	{
		return equals(b);
	}

	T * * const operator->() const { m_ptr = static_cast<T *>(deref()); return &m_ptr; }
	T * const & operator*() const { m_ptr = static_cast<T *>(deref()); return m_ptr; }
};

//=====================================================================================//
//              class RepositoryIterator : public RepositoryIteratorBase               //
//=====================================================================================//
template<typename T>
class RepositoryConstIterator : public RepositoryConstIteratorBase,
	public boost::incrementable<RepositoryConstIterator>,
	public boost::equality_comparable<RepositoryConstIterator>,
	public std::iterator<std::forward_iterator_tag, const T *>
{
	const T mutable * m_ptr;

public:
	RepositoryConstIterator() : RepositoryConstIteratorBase(RepItHelper<T>::getClassInfo()) {}
	RepositoryConstIterator(const RepositoryConstIteratorBase &a) : RepositoryConstIteratorBase(a) {}

	RepositoryConstIterator &operator++()
	{
		next();
		return *this;
	}

	bool operator==(const RepositoryConstIterator &b) const
	{
		return equals(b);
	}

	const T * * const operator->() const { m_ptr = static_cast<const T *>(deref()); return &m_ptr; }
	const T * const & operator*() const { m_ptr = static_cast<const T *>(deref()); return m_ptr; }
};

}
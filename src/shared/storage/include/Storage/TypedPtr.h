#pragma once

#include "Storage.h"

namespace Storage
{

//=====================================================================================//
//                                   class TypedPtr                                    //
//=====================================================================================//
class TypedPtr
{
	friend class Repository;

	PersistentBase *m_ptr;
	class_info m_type;

public:
	TypedPtr() : m_ptr(0), m_type(0) {}
	TypedPtr(const TypedPtr &tp) : m_ptr(tp.m_ptr), m_type(tp.m_type) {}
	template<typename T> TypedPtr(T *obj) : m_ptr(obj), m_type(T::rtti_static()) {}

	TypedPtr &operator=(const TypedPtr &tp)
	{
		if(this != &tp)
		{
			m_ptr = tp.m_ptr;
			m_type = tp.m_type;
		}
		return *this;
	}

	PersistentBase *getObject() const { return m_ptr; }
	class_info getType() const { return m_type; }

	bool isReferenceValue() const { return m_ptr == 0; }

public:
	static TypedPtr referenceValue(class_info ci)
	{
		TypedPtr result;
		result.m_type = ci;
		return result;
	}
};

//=====================================================================================//
//                               inline bool operator<()                               //
//=====================================================================================//
inline bool operator<(const TypedPtr &l, const TypedPtr &r)
{
	return l.getObject() < r.getObject();
}

//=====================================================================================//
//                               inline bool operator<()                               //
//=====================================================================================//
inline bool operator<(const TypedPtr &l, void *r)
{
	return l.getObject() < r;
}

//=====================================================================================//
//                               inline bool operator<()                               //
//=====================================================================================//
inline bool operator<(void *l, const TypedPtr &r)
{
	return l < r.getObject();
}

}
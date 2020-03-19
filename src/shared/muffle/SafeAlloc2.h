#pragma once

namespace Muffle2
{

template<typename T> class SafeAlloc;

//=====================================================================================//
//                                class SafeAlloc<void>                                //
//=====================================================================================//
template<>
class SafeAlloc<void>
{
public:
	typedef void *pointer;
	typedef const void *const_pointer;

	typedef void value_type;

	template<typename U> struct rebind { typedef SafeAlloc<U> other; };
};

//=====================================================================================//
//                                  class SafeAlloc2                                   //
//=====================================================================================//
template<typename T>
class SafeAlloc
{
public:
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef T * pointer;
	typedef const T * const_pointer;
	typedef T & reference;
	typedef const T & const_reference;
	typedef T value_type;

	template<typename U> struct rebind { typedef SafeAlloc<U> other; };

	SafeAlloc() throw() {}
	SafeAlloc(const SafeAlloc &) throw() {}
	template<typename U> SafeAlloc(const SafeAlloc<U> &) throw() {}
	~SafeAlloc() throw() {}

	pointer address(reference x) const { return &x; }
	const_pointer address(const_reference x) const { return &x; }

	pointer allocate(size_type s, SafeAlloc<void>::const_pointer hint = 0)
	{
		return (pointer)::operator new(s*sizeof(value_type));
	}

	void deallocate(pointer p, size_type s)
	{
		::operator delete(p);
	}

	size_type max_size() const throw() { return 0x7FFFFFFF; }

	void construct(pointer p, const_reference val)
	{
		new((void*)p) value_type(val);
	}

	void destroy(pointer p)
	{
		((value_type*)p)->~value_type();
	}
};

}

#pragma once

#include "Vis.h"
#include "VisRangeBase.h"

//=====================================================================================//
//                                    namespace Vis                                    //
//=====================================================================================//
namespace Vis
{

//=====================================================================================//
//                      class IndexRange : public IndexRangeBase                       //
//=====================================================================================//
template<typename T>
class IndexRange : public IndexRangeBase
{
public:
	typedef T *iterator;
	typedef std::pair<iterator,iterator> LockRange_t;

public:
	IndexRange(IndexRangeData *data) : IndexRangeBase(data) {}
	virtual ~IndexRange() {}

	LockRange_t lock(unsigned int start, unsigned int count);
	LockRange_t lock(unsigned int count);
	LockRange_t lock();

	void select();
};

//=====================================================================================//
//               inline IndexRange<T>::LockRange_t IndexRange<T>::lock()               //
//=====================================================================================//
template<typename T>
inline typename IndexRange<T>::LockRange_t IndexRange<T>::lock(unsigned int start, unsigned int count)
{
	LockRange_t result;
	result.first = reinterpret_cast<T *>(IndexRangeBase::lock(start,count));
	result.second = result.first + count;
	return result;
}

//=====================================================================================//
//               inline IndexRange<T>::LockRange_t IndexRange<T>::lock()               //
//=====================================================================================//
template<typename T>
inline typename IndexRange<T>::LockRange_t IndexRange<T>::lock(unsigned int count)
{
	LockRange_t result;
	result.first = reinterpret_cast<T *>(IndexRangeBase::lock(count));
	result.second = result.first + count;
	return result;
}

//=====================================================================================//
//               inline IndexRange<T>::LockRange_t IndexRange<T>::lock()               //
//=====================================================================================//
template<typename T>
inline typename IndexRange<T>::LockRange_t IndexRange<T>::lock()
{
	LockRange_t result;
	result.first = reinterpret_cast<T *>(IndexRangeBase::lock());
	result.second = result.first + getData()->getCount();
	return result;
}

//=====================================================================================//
//                         inline void IndexRange<T>::select()                         //
//=====================================================================================//
template<typename T>
inline void IndexRange<T>::select()
{
	getData()->select();
}

}

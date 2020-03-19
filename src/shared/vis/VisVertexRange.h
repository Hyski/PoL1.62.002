#pragma once

#include "Vis.h"
#include "VisRangeBase.h"

//=====================================================================================//
//                                    namespace Vis                                    //
//=====================================================================================//
namespace Vis
{

//=====================================================================================//
//                                  class VertexRange                                  //
//=====================================================================================//
template<typename T>
class VertexRange : public VertexRangeBase
{
public:
	typedef T *iterator;
	typedef std::pair<iterator,iterator> LockRange_t;

public:
	VertexRange(VertexRangeData *data) : VertexRangeBase(data) {}
	virtual ~VertexRange() {}

	LockRange_t lock(unsigned int start, unsigned int count);
	LockRange_t lock(unsigned int count);
	LockRange_t lock();

	void select(unsigned int stream = 0);
};

//=====================================================================================//
//                 inline VertexRange::LockRange_t VertexRange::lock()                 //
//=====================================================================================//
template<typename T>
inline typename VertexRange<T>::LockRange_t VertexRange<T>::lock(unsigned int start, unsigned int count)
{
	LockRange_t result;
	result.first = reinterpret_cast<T *>(VertexRangeBase::lock(start,count));
	result.second = result.first + count;
	return result;
}

//=====================================================================================//
//              inline VertexRange<T>::LockRange_t VertexRange<T>::lock()              //
//=====================================================================================//
template<typename T>
inline typename VertexRange<T>::LockRange_t VertexRange<T>::lock(unsigned int count)
{
	LockRange_t result;
	result.first = reinterpret_cast<T *>(VertexRangeBase::lock(count));
	result.second = result.first + count;
	return result;
}

//=====================================================================================//
//              inline VertexRange<T>::LockRange_t VertexRange<T>::lock()              //
//=====================================================================================//
template<typename T>
inline typename VertexRange<T>::LockRange_t VertexRange<T>::lock()
{
	LockRange_t result;
	result.first = reinterpret_cast<T *>(VertexRangeBase::lock());
	result.second = result.first + getData()->getCount();
	return result;
}

//=====================================================================================//
//                        inline void VertexRange<T>::select()                         //
//=====================================================================================//
template<typename T>
inline void VertexRange<T>::select(unsigned int stream)
{
	getData()->select(stream);
}

}

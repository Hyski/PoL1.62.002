#pragma once

#include "Vis.h"
#include "VisIndexRangeData.h"
#include "VisVertexRangeData.h"

//=====================================================================================//
//                                    namespace Vis                                    //
//=====================================================================================//
namespace Vis
{

//=====================================================================================//
//                                class VertexRangeBase                                //
//=====================================================================================//
template<typename T>
class RangeBase
{
	T *m_data;

public:
	RangeBase(T *data) : m_data(data) { m_data->acquire(); }
	virtual ~RangeBase() { m_data->free(); }

	T *getData() const { return m_data; }

	/// Залочивание всего буфера
	void *lock();
	/// Залочивание куска необходимого размера (нужно для динамических буферов)
	void *lock(unsigned int count);
	/// Залочивание определенного куска
	void *lock(unsigned int start, unsigned int count);

	/// Получить текущую позицию в динамическом буфере
	unsigned int getPosition() const { return m_data->getPosition(); }
	unsigned int getLockDistance() const { return m_data->getLockDistance(); }

	/// Разлочивание
	void unlock();

	//void reset() { m_position = m_lastPosition = 0; }

	void registerCallback(typename T::ReloadCallback_t cb) { m_data->registerCallback(cb); }
	void unregisterCallback() { m_data->unregisterCallback(); }
};

//=====================================================================================//
//              class VertexRangeBase : public RangeBase<VertexRangeData>              //
//=====================================================================================//
class VertexRangeBase : public RangeBase<VertexRangeData>
{
public:
	VertexRangeBase(VertexRangeData *data) : RangeBase<VertexRangeData>(data) {}

	/// Выбор текущего вертекс-буфера в Direct3DDevice
	void select(unsigned int stream = 0) { getData()->select(stream); }
};

//=====================================================================================//
//               class IndexRangeBase : public RangeBase<IndexRangeData>               //
//=====================================================================================//
class IndexRangeBase : public RangeBase<IndexRangeData>
{
public:
	IndexRangeBase(IndexRangeData *data) : RangeBase<IndexRangeData>(data) {}

	/// Выбор текущего индекс-буфера в Direct3DDevice
	void select() { getData()->select(); }
};

//=====================================================================================//
//                              void RangeBase<T>::lock()                              //
//=====================================================================================//
template<typename T>
inline void *RangeBase<T>::lock(unsigned int start, unsigned int count)
{
	//return m_data->lock(m_data->getStart()+start,count,ltInsert);
	return m_data->lock(start,count);
}

//=====================================================================================//
//                          inline void *RangeBase<T>::lock()                          //
//=====================================================================================//
template<typename T>
inline void *RangeBase<T>::lock(unsigned int count)
{
	return m_data->lock(count);
	//assert( m_data->getCount() >= count );
	//LockType lt = ltAppend;

	//if(m_position+count > m_data->getCount())
	//{
	//	lt = ltFlush;
	//	m_position = 0;
	//}

	//void *result = m_data->lock(m_data->getStart()+m_position,count,lt);
	//m_lastPosition = m_position;
	//m_position += count;
	//m_lockDistance = count;

	//return result;
}

//=====================================================================================//
//                          inline void *RangeBase<T>::lock()                          //
//=====================================================================================//
template<typename T>
inline void *RangeBase<T>::lock()
{
	//return m_data->lock(m_data->getStart(),m_data->getCount(),ltInsert);
	return m_data->lock();
}

//=====================================================================================//
//                         inline void RangeBase<T>::unlock()                          //
//=====================================================================================//
template<typename T>
inline void RangeBase<T>::unlock()
{
	m_data->unlock();
}

}

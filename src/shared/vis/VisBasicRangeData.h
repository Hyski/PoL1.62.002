#if !defined(__VIS_BASIC_RANGE_DATA_H_INCLUDED_2627849057366726__)
#define __VIS_BASIC_RANGE_DATA_H_INCLUDED_2627849057366726__

#include "Vis.h"
#include <d3d9.h>
#include <boost/function.hpp>

//=====================================================================================//
//                                    namespace Vis                                    //
//=====================================================================================//
namespace Vis
{

enum LockType
{
	ltInsert		=	D3DLOCK_NOSYSLOCK,
	ltAppend		=	D3DLOCK_NOSYSLOCK|D3DLOCK_NOOVERWRITE,
	ltFlush			=	D3DLOCK_NOSYSLOCK|D3DLOCK_DISCARD
};

#pragma warning(push)
#pragma warning(disable: 4251)

//=====================================================================================//
//                                class BasicRangeData                                 //
//=====================================================================================//
class VIS_BUFFER_IMPORT BasicRangeData
{
	unsigned int m_vertexSize;
	unsigned int m_begin;
	unsigned int m_count;
	unsigned int m_useCount;

protected:
	unsigned int m_position;
	unsigned int m_lastPosition;
	unsigned int m_lockDistance;

protected:
	BasicRangeData cutForUse(unsigned int count);
	BasicRangeData join(const BasicRangeData &);

	void acquire();
	void free();

public:
	typedef boost::function<void ()> ReloadCallback_t;

public:
	BasicRangeData() : m_vertexSize(0), m_begin(0), m_count(0), m_useCount(0),
		m_position(0), m_lastPosition(0), m_lockDistance(0) {}
	BasicRangeData(unsigned int vsize, unsigned int begin, 
		unsigned int count) : m_vertexSize(vsize), m_begin(begin), m_count(count),
							  m_useCount(0), m_position(0), m_lastPosition(0), m_lockDistance(0) {}
	BasicRangeData(const BasicRangeData &d) :
		m_vertexSize(d.m_vertexSize),
		m_begin(d.m_begin),
		m_count(d.m_count),
		m_useCount(d.m_useCount),
		m_position(0), m_lastPosition(0), m_lockDistance(0)
		{ assert( m_useCount == 0 ); }

	virtual ~BasicRangeData() {}

	bool isInUse() const { return m_useCount != 0; }
	unsigned int getStart() const { return m_begin; }
	unsigned int getCount() const { return m_count; }
	unsigned int getVertexSize() const { return m_vertexSize; }
	unsigned int getSizeInBytes() const { return m_count * m_vertexSize; }
	unsigned int getPosition() const { return m_lastPosition; }
	unsigned int getLockDistance() const { return m_lockDistance; }

	operator void *() const { return (void *)(m_count != 0); }
	bool operator!() const { return m_count == 0; }

	void registerCallback(ReloadCallback_t cb) { m_rcb = cb; }
	void unregisterCallback() { m_rcb = ReloadCallback_t(); }

	void *lock(unsigned int, unsigned int);
	void *lock(unsigned int);
	void *lock();

private:
	friend class Blocker;

	virtual void *doLock(unsigned int, unsigned int, LockType) { return 0; }
	virtual bool isBlocked() const { return false; }
	virtual void setBlock() const {}
	virtual void releaseBlock() const {}

	friend class BufferMgr;

	ReloadCallback_t m_rcb;
	void notify() { if(m_rcb) m_rcb(); }
};

#pragma warning(pop)

//=====================================================================================//
//                  inline BasicRangeData BasicRangeData::cutForUse()                  //
//=====================================================================================//
inline BasicRangeData BasicRangeData::cutForUse(unsigned int count)
{
	if(m_count < count) return BasicRangeData();
	BasicRangeData result(m_vertexSize,m_begin,count);
	m_begin += count;
	m_count -= count;
	return result;
}

inline void *BasicRangeData::lock(unsigned int start, unsigned int count)
{
	return doLock((start + m_begin)*m_vertexSize,count*m_vertexSize, ltInsert);
}

inline void *BasicRangeData::lock(unsigned int count)
{
	assert( getCount() >= count );
	LockType lt = ltAppend;

	if(m_position+count > getCount())
	{
		lt = ltFlush;
		m_position = 0;
	}

	void *result = doLock((m_begin+m_position)*m_vertexSize,count*m_vertexSize,lt);
	m_lastPosition = m_position;
	m_position += count;
	m_lockDistance = count;
	return result;
}

inline void *BasicRangeData::lock()
{
	return doLock(m_begin*m_vertexSize,m_count*m_vertexSize, ltInsert);
}

//=====================================================================================//
//                             struct RangeDataBySizePred                              //
//=====================================================================================//
struct RangeDataBySizePred
{
	unsigned int m_size;
	RangeDataBySizePred(unsigned int s) : m_size(s) {}

	bool operator()(const BasicRangeData &v) const
	{
		return !v.isInUse() && v.getSizeInBytes() >= m_size;
	}
};
}

#endif // !defined(__VIS_BASIC_RANGE_DATA_H_INCLUDED_2627849057366726__)
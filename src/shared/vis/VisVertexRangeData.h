#pragma once

#include "Vis.h"
#include <atlcomcli.h>
#include "VisVertexBuffer.h"
#include "VisBasicRangeData.h"

//=====================================================================================//
//                                    namespace Vis                                    //
//=====================================================================================//
namespace Vis
{

#pragma warning(push)
#pragma warning(disable: 4251)

//=====================================================================================//
//                                class VertexRangeData                                //
//=====================================================================================//
class VIS_BUFFER_IMPORT VertexRangeData : public BasicRangeData
{
	HVertexBuffer m_buffer;

public:
	VertexRangeData() {}
	VertexRangeData(HVertexBuffer buf, unsigned int vsize, 
		unsigned int begin, unsigned int count)
		: BasicRangeData(vsize,begin,count), m_buffer(buf) {}
	VertexRangeData(HVertexBuffer buf, const BasicRangeData &d)
		: BasicRangeData(d), m_buffer(buf) {}

	void acquire();
	void free();

	VertexRangeData cutForUse(unsigned int count);
	VertexRangeData join(const VertexRangeData &);

	void unlock();

	void select(unsigned int stream = 0);
	HVertexBuffer getBuffer() const { return m_buffer; }

private:
	virtual bool isBlocked() const { return m_buffer->isBlocked(); }
	virtual void setBlock() const { m_buffer->setBlock(); }
	virtual void releaseBlock() const { m_buffer->releaseBlock(); }

	friend class BufferMgr;

	void *doLock(unsigned int start, unsigned int count, LockType);

	bool isBufferEmptyAndFullSize() const
	{
		return m_buffer && m_buffer->getLength() == getSizeInBytes();
	}
};

#pragma warning(pop)

//=====================================================================================//
//                 inline VertexRangeData VertexRangeData::cutForUse()                 //
//=====================================================================================//
inline VertexRangeData VertexRangeData::cutForUse(unsigned int count)
{
	BasicRangeData result = BasicRangeData::cutForUse(count);
	if(result) return VertexRangeData(m_buffer,result);
	return VertexRangeData();
}

//=====================================================================================//
//                        inline void *VertexRangeData::lock()                         //
//=====================================================================================//
inline void *VertexRangeData::doLock(unsigned int start, unsigned int count, LockType lt)
{
	return m_buffer->lock(start,count,lt);
}

//=====================================================================================//
//                        inline void VertexRangeData::unlock()                        //
//=====================================================================================//
inline void VertexRangeData::unlock()
{
	m_buffer->unlock();
}

}

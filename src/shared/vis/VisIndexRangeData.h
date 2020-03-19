#pragma once

#include "Vis.h"
#include <atlcomcli.h>
#include "VisIndexBuffer.h"
#include "VisBasicRangeData.h"

//=====================================================================================//
//                                    namespace Vis                                    //
//=====================================================================================//
namespace Vis
{

#pragma warning(push)
#pragma warning(disable: 4251)

//=====================================================================================//
//                    class IndexRangeData : public BasicRangeData                     //
//=====================================================================================//
class VIS_BUFFER_IMPORT IndexRangeData : public BasicRangeData
{
	void *m_startOfBuffer;
	HIndexBuffer m_buffer;
	//CComPtr<IDirect3DIndexBuffer9> m_buffer;

public:
	IndexRangeData() : m_startOfBuffer(0) {}
	IndexRangeData(HIndexBuffer buf, unsigned int vsize, unsigned int begin, unsigned int count)
		: BasicRangeData(vsize,begin,count), m_startOfBuffer(0), m_buffer(buf) {}
	IndexRangeData(HIndexBuffer buf, const BasicRangeData &d)
		: BasicRangeData(d), m_startOfBuffer(0), m_buffer(buf) {}

	void acquire();
	void free();

	IndexRangeData cutForUse(unsigned int count);
	IndexRangeData join(const IndexRangeData &);

	void unlock();

	void select();
	HIndexBuffer getBuffer() const { return m_buffer; }

	void setStartOfBuffer(void *ptr) { m_startOfBuffer = ptr; }

private:
	virtual bool isBlocked() const { return m_buffer->isBlocked(); }
	virtual void setBlock() const { m_buffer->setBlock(); }
	virtual void releaseBlock() const { m_buffer->releaseBlock(); }

	friend class BufferMgr;

	void *doLock(unsigned int start, unsigned int count, LockType lt);

	bool isBufferEmptyAndFullSize() const
	{
		return m_buffer && m_buffer->getLength() == getSizeInBytes();
	}
};

#pragma warning(pop)

//=====================================================================================//
//                     IndexRangeData IndexRangeData::cutForUse()                      //
//=====================================================================================//
inline IndexRangeData IndexRangeData::cutForUse(unsigned int count)
{
	BasicRangeData result = BasicRangeData::cutForUse(count);
	if(result) return IndexRangeData(m_buffer,result);
	return IndexRangeData();
}

//=====================================================================================//
//                            void *IndexRangeData::lock()                             //
//=====================================================================================//
inline void *IndexRangeData::doLock(unsigned int start, unsigned int count, LockType lt)
{
	return m_buffer->lock(start,count,lt);
}

//=====================================================================================//
//                            void IndexRangeData::unlock()                            //
//=====================================================================================//
inline void IndexRangeData::unlock()
{
	m_buffer->unlock();
}

}

#include "Vis.h"
#include "VisBufferMgr.h"
#include "VisVertexRangeData.h"

namespace Vis
{

//=====================================================================================//
//                           void VertexRangeData::select()                            //
//=====================================================================================//
void VertexRangeData::select(unsigned int stream)
{
	m_buffer->select(stream);
}

//=====================================================================================//
//                       VertexRangeData VertexRangeData::join()                       //
//=====================================================================================//
VertexRangeData VertexRangeData::join(const VertexRangeData &data)
{
	if(m_buffer != data.m_buffer) return VertexRangeData();
	return VertexRangeData(m_buffer,BasicRangeData::join(data));
}

//=====================================================================================//
//                           void VertexRangeData::acquire()                           //
//=====================================================================================//
void VertexRangeData::acquire()
{
	BasicRangeData::acquire();
}

//=====================================================================================//
//                            void VertexRangeData::free()                             //
//=====================================================================================//
void VertexRangeData::free()
{
	BasicRangeData::free();
	if(!BasicRangeData::isInUse()) BufferMgr::instance()->onBufferFreed(this);
}

}
#include "Vis.h"
#include "VisBufferMgr.h"
#include "VisIndexRangeData.h"

namespace Vis
{

//=====================================================================================//
//                            void IndexRangeData::select()                            //
//=====================================================================================//
void IndexRangeData::select()
{
	m_buffer->select();
}

//=====================================================================================//
//                        IndexRangeData IndexRangeData::join()                        //
//=====================================================================================//
IndexRangeData IndexRangeData::join(const IndexRangeData &data)
{
	if(m_buffer != data.m_buffer) return IndexRangeData();
	return IndexRangeData(m_buffer,BasicRangeData::join(data));
}

//=====================================================================================//
//                           void IndexRangeData::acquire()                            //
//=====================================================================================//
void IndexRangeData::acquire()
{
	BasicRangeData::acquire();
}

//=====================================================================================//
//                             void IndexRangeData::free()                             //
//=====================================================================================//
void IndexRangeData::free()
{
	BasicRangeData::free();
	if(!BasicRangeData::isInUse()) BufferMgr::instance()->onBufferFreed(this);
}

}
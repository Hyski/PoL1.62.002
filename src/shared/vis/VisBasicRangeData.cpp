#include "Vis.h"
#include "VisBasicRangeData.h"

//#ifdef min
//#undef min
//#endif

namespace Vis
{

//=====================================================================================//
//                           void BasicRangeData::acquire()                            //
//=====================================================================================//
void BasicRangeData::acquire()
{
	//assert( m_used == false );
	++m_useCount;
}

//=====================================================================================//
//                             void BasicRangeData::free()                             //
//=====================================================================================//
void BasicRangeData::free()
{
	//assert( m_used == true );
	--m_useCount;
}

//=====================================================================================//
//                             bool BasicRangeData::join()                             //
//=====================================================================================//
BasicRangeData BasicRangeData::join(const BasicRangeData &data)
{
	assert( m_vertexSize == data.m_vertexSize );
	if(isInUse() || data.isInUse()) return BasicRangeData();

	const bool meIsPrevToData = m_begin + m_count != data.m_begin;
	const bool meIsNextToData = data.m_begin + data.m_count != m_begin;

	if(meIsPrevToData && meIsNextToData) return BasicRangeData();

	return BasicRangeData(m_vertexSize, std::min(m_begin,data.m_begin), m_count + data.m_count);
}

}
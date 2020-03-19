#include "Vis.h"
#include <algorithm>
#include "VisBufferMgr.h"
#include <mll/debug/log.h>

#include <d3dx9core.h>

namespace Vis
{

#ifndef ML_VIS_DISABLE_LOGS
#define BUFMGR_LOG(L)
#else
#define BUFMGR_LOG(L)	MLL_MAKE_DEBUG_LOG("vis/bufmgr.log",L)
#endif

static BufferMgr *g_instance = 0;
static unsigned int g_defaultVertexCount = 32768;
static unsigned int g_defaultIndexCount = 65536*2;

//=====================================================================================//
//                               BufferMgr::BufferMgr()                                //
//=====================================================================================//
BufferMgr::BufferMgr(IDirect3DDevice9 *dev)
:	m_device(dev),
	m_currentVBuffers(8,(IDirect3DVertexBuffer9*)0),
	m_currentIBuffer(0)
{
	assert( g_instance == 0 );
	g_instance = this;
}

//=====================================================================================//
//                               BufferMgr::~BufferMgr()                               //
//=====================================================================================//
BufferMgr::~BufferMgr()
{
	assert( bytesOccupiedInVertexBuffers() == 0 );
	assert( bytesOccupiedInIndexBuffers() == 0 );

	m_iranges.clear();
	m_vranges.clear();

	assert( m_ibuffers.empty() );
	assert( m_vbuffers.empty() );

	assert( g_instance == this );
	g_instance = 0;
}

//=====================================================================================//
//                          BufferMgr *BufferMgr::instance()                           //
//=====================================================================================//
BufferMgr *BufferMgr::instance()
{
	return g_instance;
}

//=====================================================================================//
//                      unsigned int BufferMgr::countSize() const                      //
//=====================================================================================//
template<typename T>
unsigned int BufferMgr::countSize(const T &map, bool compare) const
{
	unsigned int result = 0;

	typename T::const_iterator i;
	for(i = map.begin(); i != map.end(); ++i)
	{
		typename T::mapped_type::const_iterator j;
		for(j = i->second.begin(); j != i->second.end(); ++j)
		{
			if(j->isInUse() == compare)
			{
				result += j->getSizeInBytes();
			}
		}
	}

	return result;
}

//=====================================================================================//
//            unsigned int BufferMgr::bytesOccupiedInVertexBuffers() const             //
//=====================================================================================//
unsigned int BufferMgr::bytesOccupiedInVertexBuffers() const
{
	return countSize(m_vranges,true);
}

//=====================================================================================//
//             unsigned int BufferMgr::bytesUnusedInVertexBuffers() const              //
//=====================================================================================//
unsigned int BufferMgr::bytesUnusedInVertexBuffers() const
{
	return countSize(m_vranges,false);
}

//=====================================================================================//
//             unsigned int BufferMgr::bytesOccupiedInIndexBuffers() const             //
//=====================================================================================//
unsigned int BufferMgr::bytesOccupiedInIndexBuffers() const
{
	return countSize(m_iranges,true);
}

//=====================================================================================//
//              unsigned int BufferMgr::bytesUnusedInIndexBuffers() const              //
//=====================================================================================//
unsigned int BufferMgr::bytesUnusedInIndexBuffers() const
{
	return countSize(m_iranges,false);
}

//=====================================================================================//
//                              I BufferMgr::cutAPiece()                               //
//=====================================================================================//
template<typename C, typename I>
I BufferMgr::cutAPiece(C &c, I it, unsigned int size)
{
	I result = c.insert(it,it->cutForUse(size));
	if(!*it) c.erase(it);
	return result;
}

//=====================================================================================//
//                CComPtr<IDirect3DVertexBuffer9> BufferMgr::createVB()                //
//=====================================================================================//
HVertexBuffer BufferMgr::createVB(unsigned int fvf, BufferPoolType flags,
													unsigned int count)
{
	HVertexBuffer result = new VertexBuffer(m_device,count*fvf_size(fvf),fvf,flags);
	m_vbuffers.push_back(result.get());
	return result;
}

//=====================================================================================//
//                CComPtr<IDirect3DIndexBuffer9> BufferMgr::createIB()                 //
//=====================================================================================//
HIndexBuffer BufferMgr::createIB(unsigned int fvf, BufferPoolType flags,
												   unsigned int count)
{
	D3DFORMAT fmt = (fvf==2)?D3DFMT_INDEX16:D3DFMT_INDEX32;
	HIndexBuffer result = new IndexBuffer(m_device,count*fvf,fmt,flags);
	m_ibuffers.push_back(result.get());
	return result;
}

//=====================================================================================//
//                            bool BufferMgr::isInBuffer()                             //
//=====================================================================================//
template<typename C, typename V>
bool BufferMgr::isInBuffer(C &c, V *v)
{
	for(typename C::iterator i = c.begin(); i != c.end(); ++i)
	{
		for(typename C::mapped_type::iterator j = i->second.begin(); j != i->second.end(); ++j)
		{
			if(&*j == v) return true;
		}
	}
	return false;
}

//=====================================================================================//
//           std::auto_ptr<VertexRangeData> BufferMgr::getVertexRangeData()            //
//=====================================================================================//
VertexRangeData *BufferMgr::getVertexRangeData(unsigned int fvf,
											   size_t size,
											   BufferPoolType flags)
{
	unsigned int vtx_size = D3DXGetFVFVertexSize(fvf);
	unsigned int total_size = vtx_size*size;
	Id index(fvf,flags);

	VertexRangeData *result = 0;
	VertexRanges_t &ranges = m_vranges[index];

	if(flags != bptDynamicData)
	{
		VertexRanges_t::iterator i = std::find_if(ranges.begin(),ranges.end(),
											RangeDataBySizePred(total_size));

		if(i == ranges.end())
		{
			unsigned int real_size = std::max(size,g_defaultVertexCount);
			HVertexBuffer buf = createVB(fvf,flags,real_size);
			i = ranges.insert(ranges.end(), VertexRangeData(buf,vtx_size,0,real_size));
		}

		result = &*cutAPiece(ranges,i,size);
		assert( isInBuffer(m_vranges,result) );
	}
	else
	{
		VertexRanges_t::iterator i = ranges.begin();

		if(i == ranges.end())
		{
			unsigned int real_size = std::max(size,g_defaultVertexCount);
			HVertexBuffer buf = createVB(fvf,flags,real_size);
			i = ranges.insert(ranges.end(),VertexRangeData(buf,vtx_size,0,real_size));
		}

		result = &*i;
		assert( isInBuffer(m_vranges,result) );
	}

	return result;
}

//=====================================================================================//
//                   IndexRangeData *BufferMgr::getIndexRangeData()                    //
//=====================================================================================//
IndexRangeData *BufferMgr::getIndexRangeData(unsigned int fvf,
											 size_t size,
											 BufferPoolType flags)
{
	unsigned int vtx_size = fvf;
	unsigned int total_size = vtx_size*size;
	Id index(fvf,flags);

	IndexRangeData *result = 0;
	IndexRanges_t &ranges = m_iranges[index];

	if(flags != bptDynamicData)
	{
		IndexRanges_t::iterator i = std::find_if(ranges.begin(),ranges.end(),
											RangeDataBySizePred(total_size));

		if(i == ranges.end())
		{
			unsigned int real_size = std::max(size,g_defaultIndexCount);
			HIndexBuffer buf = createIB(fvf,flags,real_size);
			i = ranges.insert(ranges.end(), IndexRangeData(buf,vtx_size,0,real_size));
		}

		result = &*cutAPiece(ranges,i,size);
		assert( isInBuffer(m_iranges,result) );
	}
	else
	{
		IndexRanges_t::iterator i = ranges.begin();

		if(i == ranges.end())
		{
			unsigned int real_size = std::max(size,g_defaultIndexCount);
			HIndexBuffer buf = createIB(fvf,flags,real_size);
			i = ranges.insert(ranges.end(),IndexRangeData(buf,vtx_size,0,real_size));
		}

		result = &*i;
		assert( isInBuffer(m_iranges,result) );
	}

	return result;
}

//=====================================================================================//
//                         void BufferMgr::setCurrentVBuffer()                         //
//=====================================================================================//
void BufferMgr::setCurrentVBuffer(unsigned int stream, IDirect3DVertexBuffer9 *buf,
								  unsigned int size)
{
	if(m_currentVBuffers[stream] != buf)
	{
		HRESULT hres = m_device->SetStreamSource(stream,buf,0,size);
		assert( SUCCEEDED(hres) );
		D3DVERTEXBUFFER_DESC vbd;
		buf->GetDesc(&vbd);
		hres = m_device->SetFVF(vbd.FVF);
		assert( SUCCEEDED(hres) );
		m_currentVBuffers[stream] = buf;
	}
}

//=====================================================================================//
//                         void BufferMgr::setCurrentIBuffer()                         //
//=====================================================================================//
void BufferMgr::setCurrentIBuffer(IDirect3DIndexBuffer9 *buf)
{
	if(m_currentIBuffer != buf)
	{
		HRESULT hres = m_device->SetIndices(buf);
		assert( SUCCEEDED(hres) );
		m_currentIBuffer = buf;
	}
}

//=====================================================================================//
//                        void BufferMgr::resetCurrentBuffers()                        //
//=====================================================================================//
void BufferMgr::resetCurrentBuffers()
{
	std::fill(m_currentVBuffers.begin(),m_currentVBuffers.end(),(IDirect3DVertexBuffer9*)0);
	m_currentIBuffer = 0;
}

//=====================================================================================//
//                           void BufferMgr::onBufferFreed()                           //
//=====================================================================================//
void BufferMgr::onBufferFreed(IndexRangeData *data)
{
	IndexRanges_t *ranges;
	IndexRanges_t::iterator i;

	boost::tie(i,ranges) = findData(data);
	assert( ranges != 0 );

	if(i != ranges->begin())
	{
		IndexRangeData newData = i->join(*boost::prior(i));
		if(newData)
		{
			ranges->erase(boost::prior(i));
			i = ranges->insert(i,newData);
			ranges->erase(boost::next(i));
		}
	}

	if(boost::next(i) != ranges->end())
	{
		IndexRangeData newData = i->join(*boost::next(i));
		if(newData)
		{
			ranges->erase(boost::next(i));
			i = ranges->insert(i,newData);
			ranges->erase(boost::next(i));
		}
	}

	if(i->isBufferEmptyAndFullSize() && !i->isBlocked())
	{
		ranges->erase(i);
	}
}

//=====================================================================================//
//                         void BufferMgr::removeIndexBuffer()                         //
//=====================================================================================//
void BufferMgr::removeIndexBuffer(IndexBuffer *buf)
{
	IndexBuffers_t::iterator i = std::find(m_ibuffers.begin(),m_ibuffers.end(),buf);
	assert( i != m_ibuffers.end() );
	m_ibuffers.erase(i);
}

//=====================================================================================//
//                        void BufferMgr::removeVertexBuffer()                         //
//=====================================================================================//
void BufferMgr::removeVertexBuffer(VertexBuffer *buf)
{
	VertexBuffers_t::iterator i = std::find(m_vbuffers.begin(),m_vbuffers.end(),buf);
	assert( i != m_vbuffers.end() );
	m_vbuffers.erase(i);
}

//=====================================================================================//
//                           void BufferMgr::onBufferFreed()                           //
//=====================================================================================//
void BufferMgr::onBufferFreed(VertexRangeData *data)
{
	VertexRanges_t *ranges;
	VertexRanges_t::iterator i;

	boost::tie(i,ranges) = findData(data);
	assert( ranges != 0 );

	if(i != ranges->begin())
	{
		VertexRangeData newData = i->join(*boost::prior(i));
		if(newData)
		{
			ranges->erase(boost::prior(i));
			i = ranges->insert(i,newData);
			ranges->erase(boost::next(i));
		}
	}

	if(boost::next(i) != ranges->end())
	{
		VertexRangeData newData = i->join(*boost::next(i));
		if(newData)
		{
			ranges->erase(boost::next(i));
			i = ranges->insert(i,newData);
			ranges->erase(boost::next(i));
		}
	}

	if(i->isBufferEmptyAndFullSize() && !i->isBlocked())
	{
		ranges->erase(i);
	}
}

//=====================================================================================//
//              BufferMgr::IndexRanges_t::iterator BufferMgr::findData()               //
//=====================================================================================//
std::pair<BufferMgr::IndexRanges_t::iterator,BufferMgr::IndexRanges_t *>
BufferMgr::findData(IndexRangeData *data)
{
	for(IndexCluster_t::iterator i = m_iranges.begin(); i != m_iranges.end(); ++i)
	{
		for(IndexRanges_t::iterator j = i->second.begin(); j != i->second.end(); ++j)
		{
			if(&*j == data) return std::make_pair(j,&i->second);
		}
	}

	return std::make_pair(IndexRanges_t::iterator(),(IndexRanges_t *)0);
}

//=====================================================================================//
//              BufferMgr::VertexRanges_t::iterator BufferMgr::findData()              //
//=====================================================================================//
std::pair<BufferMgr::VertexRanges_t::iterator,BufferMgr::VertexRanges_t *>
BufferMgr::findData(VertexRangeData *data)
{
	for(VertexCluster_t::iterator i = m_vranges.begin(); i != m_vranges.end(); ++i)
	{
		for(VertexRanges_t::iterator j = i->second.begin(); j != i->second.end(); ++j)
		{
			if(&*j == data) return std::make_pair(j,&i->second);
		}
	}

	return std::make_pair(VertexRanges_t::iterator(),(VertexRanges_t *)0);
}

//=====================================================================================//
//                        void BufferMgr::debugOutputContent()                         //
//=====================================================================================//
void BufferMgr::debugOutputContent()
{
	BUFMGR_LOG("Buffer content:\n");

	BUFMGR_LOG("\tVertices:\n");
	for(VertexCluster_t::iterator i = m_vranges.begin(); i != m_vranges.end(); ++i)
	{
		BUFMGR_LOG("\t\tFVF: " << i->first.fvf << ", flags: " << i->first.flags << "\n");
		for(VertexRanges_t::iterator j = i->second.begin(); j != i->second.end(); ++j)
		{
			BUFMGR_LOG("\t\t\t(" << j->getStart() << "," << j->getCount()+j->getStart() << ") "
				<< (j->isInUse()?"used":"free") << "\n");
		}
	}

	BUFMGR_LOG("\tIndices:\n");
	for(IndexCluster_t::iterator i = m_iranges.begin(); i != m_iranges.end(); ++i)
	{
		BUFMGR_LOG("\t\tFVF: " << i->first.fvf << ", flags: " << i->first.flags << "\n");
		for(IndexRanges_t::iterator j = i->second.begin(); j != i->second.end(); ++j)
		{
			BUFMGR_LOG("\t\t\t(" << j->getStart() << "," << j->getCount()+j->getStart() << ") "
				<< (j->isInUse()?"used":"free") << "\n");
		}
	}
}

//=====================================================================================//
//                  unsigned int BufferMgr::vertexBufferCount() const                  //
//=====================================================================================//
unsigned int BufferMgr::vertexBufferCount() const
{
	std::set<HVertexBuffer> buffers;
	for(VertexCluster_t::const_iterator i = m_vranges.begin(); i != m_vranges.end(); ++i)
	{
		for(VertexRanges_t::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
		{
			buffers.insert(j->getBuffer());
		}
	}
	return buffers.size();
}

//=====================================================================================//
//                  unsigned int BufferMgr::indexBufferCount() const                   //
//=====================================================================================//
unsigned int BufferMgr::indexBufferCount() const
{
	std::set<HIndexBuffer> buffers;
	for(IndexCluster_t::const_iterator i = m_iranges.begin(); i != m_iranges.end(); ++i)
	{
		for(IndexRanges_t::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
		{
			buffers.insert(j->getBuffer());
		}
	}
	return buffers.size();
}

//=====================================================================================//
//                           void BufferMgr::onDeviceLost()                            //
//=====================================================================================//
void BufferMgr::onDeviceLost()
{
	resetCurrentBuffers();

	for(IndexBuffers_t::iterator i = m_ibuffers.begin(); i != m_ibuffers.end(); ++i)
	{
		(*i)->onDeviceLost();
	}

	for(VertexBuffers_t::iterator i = m_vbuffers.begin(); i != m_vbuffers.end(); ++i)
	{
		(*i)->onDeviceLost();
	}
}

//=====================================================================================//
//                           void BufferMgr::onDeviceReset()                           //
//=====================================================================================//
void BufferMgr::onDeviceReset()
{
	for(IndexBuffers_t::iterator i = m_ibuffers.begin(); i != m_ibuffers.end(); ++i)
	{
		(*i)->onDeviceReset();
		if((*i)->getPoolType() == bptManualReload)
		{
			(*i)->lockWhole();
		}
	}

	for(VertexBuffers_t::iterator i = m_vbuffers.begin(); i != m_vbuffers.end(); ++i)
	{
		(*i)->onDeviceReset();
		if((*i)->getPoolType() == bptManualReload)
		{
			(*i)->lockWhole();
		}
	}

	for(IndexCluster_t::iterator i = m_iranges.begin(); i != m_iranges.end(); ++i)
	{
		if(i->first.flags == bptAutomaticReload) continue;
		if(i->second.empty()) continue;

		if(i->first.flags == bptManualReload)
		{
			for(IndexRanges_t::iterator j = i->second.begin(); j != i->second.end(); ++j)
			{
				if(i->first.flags == bptManualReload && j->isInUse())
				{
					j->notify();
				}
			}
		}
	}

	for(VertexCluster_t::iterator i = m_vranges.begin(); i != m_vranges.end(); ++i)
	{
		if(i->first.flags == bptAutomaticReload) continue;
		if(i->second.empty()) continue;

		for(VertexRanges_t::iterator j = i->second.begin(); j != i->second.end(); ++j)
		{
			if(i->first.flags == bptManualReload && j->isInUse())
			{
				j->notify();
			}
		}
	}

	for(IndexBuffers_t::iterator i = m_ibuffers.begin(); i != m_ibuffers.end(); ++i)
	{
		if((*i)->getPoolType() == bptManualReload)
		{
			(*i)->unlockWhole();
		}
	}

	for(VertexBuffers_t::iterator i = m_vbuffers.begin(); i != m_vbuffers.end(); ++i)
	{
		if((*i)->getPoolType() == bptManualReload)
		{
			(*i)->unlockWhole();
		}
	}
}

}
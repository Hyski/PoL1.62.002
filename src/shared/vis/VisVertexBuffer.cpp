#include "Vis.h"
#include "VisBufferMgr.h"
#include "VisVertexBuffer.h"

namespace Vis
{

//=====================================================================================//
//                            VertexBuffer::VertexBuffer()                             //
//=====================================================================================//
VertexBuffer::VertexBuffer(IDirect3DDevice9 *dev, unsigned int length,
						   unsigned int fmt, BufferPoolType pool)
:	m_length(length),
	m_bpt(pool),
	m_format(fmt),
	m_device(dev),
	m_wholeLockData(0),
	m_block(0)
{
	createBuffer();
}


//=====================================================================================//
//                            VertexBuffer::~VertexBuffer()                            //
//=====================================================================================//
VertexBuffer::~VertexBuffer()
{
	BufferMgr::instance()->removeVertexBuffer(this);
}

//=====================================================================================//
//                             void VertexBuffer::select()                             //
//=====================================================================================//
void VertexBuffer::select(unsigned int stream)
{
	BufferMgr::instance()->setCurrentVBuffer(stream,m_buffer,D3DXGetFVFVertexSize(m_format));
}

//=====================================================================================//
//                             void *VertexBuffer::lock()                              //
//=====================================================================================//
void *VertexBuffer::lock(unsigned int start, unsigned int count, unsigned int lockType)
{
	void *data = 0;
	if(m_wholeLockData)
	{
		data = reinterpret_cast<char *>(m_wholeLockData) + start;
	}
	else
	{
		HRESULT hres = m_buffer->Lock(start,count,&data,lockType);
		assert( SUCCEEDED(hres) );
	}
	return data;
}

//=====================================================================================//
//                             void VertexBuffer::unlock()                             //
//=====================================================================================//
void VertexBuffer::unlock()
{
	if (!m_wholeLockData)
		m_buffer->Unlock();
}

//=====================================================================================//
//                           void *VertexBuffer::lockWhole()                           //
//=====================================================================================//
void *VertexBuffer::lockWhole()
{
	assert( m_wholeLockData == 0 );
	HRESULT hres = m_buffer->Lock(0,0,&m_wholeLockData,ltInsert);
	assert( SUCCEEDED(hres) );
	return m_wholeLockData;
}

//=====================================================================================//
//                          void VertexBuffer::unlockWhole()                           //
//=====================================================================================//
void VertexBuffer::unlockWhole()
{
	assert( m_wholeLockData != 0 );
	HRESULT hres = m_buffer->Unlock();
	assert( SUCCEEDED(hres) );
	m_wholeLockData = 0;
}

//=====================================================================================//
//                          void VertexBuffer::onDeviceLost()                          //
//=====================================================================================//
void VertexBuffer::onDeviceLost()
{
	if(m_bpt == bptManualReload || m_bpt == bptDynamicData)
	{
		m_buffer.Release();
	}
}

//=====================================================================================//
//                          void VertexBuffer::createBuffer()                          //
//=====================================================================================//
void VertexBuffer::createBuffer()
{
	unsigned int usage = 0;
	D3DPOOL pool = D3DPOOL_DEFAULT;

	switch(m_bpt)
	{
	case bptAutomaticReload:
		usage = 0;
		pool = D3DPOOL_MANAGED;
		break;
	case bptManualReload:
		usage = D3DUSAGE_WRITEONLY;
		pool = D3DPOOL_DEFAULT;
		break;
	case bptDynamicData:
		usage = D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY;
		pool = D3DPOOL_DEFAULT;
		break;
	}

	HRESULT hres = m_device->CreateVertexBuffer
		(
			m_length,
			usage,
			m_format,
			pool,
			&m_buffer,
			NULL
		);

	assert( SUCCEEDED(hres) );
}

//=====================================================================================//
//                         void VertexBuffer::onDeviceReset()                          //
//=====================================================================================//
void VertexBuffer::onDeviceReset()
{
	if(m_bpt == bptManualReload || m_bpt == bptDynamicData)
	{
		createBuffer();
	}
}
}
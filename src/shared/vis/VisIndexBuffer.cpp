#include "Vis.h"
#include "VisBufferMgr.h"
#include "VisIndexBuffer.h"

namespace Vis
{

//=====================================================================================//
//                             IndexBuffer::IndexBuffer()                              //
//=====================================================================================//
IndexBuffer::IndexBuffer(IDirect3DDevice9 *dev, unsigned int length,
						 D3DFORMAT fmt, BufferPoolType pool)
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
//                             IndexBuffer::~IndexBuffer()                             //
//=====================================================================================//
IndexBuffer::~IndexBuffer()
{
	BufferMgr::instance()->removeIndexBuffer(this);
}

//=====================================================================================//
//                             void IndexBuffer::select()                              //
//=====================================================================================//
void IndexBuffer::select()
{
	BufferMgr::instance()->setCurrentIBuffer(m_buffer);
}

//=====================================================================================//
//                              void *IndexBuffer::lock()                              //
//=====================================================================================//
void *IndexBuffer::lock(unsigned int start, unsigned int count, unsigned int lockType)
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
//                             void IndexBuffer::unlock()                              //
//=====================================================================================//
void IndexBuffer::unlock()
{
	if (!m_wholeLockData)
		m_buffer->Unlock();
}

//=====================================================================================//
//                           void *IndexBuffer::lockWhole()                            //
//=====================================================================================//
void *IndexBuffer::lockWhole()
{
	assert( m_wholeLockData == 0 );
	HRESULT hres = m_buffer->Lock(0,0,&m_wholeLockData,ltInsert);
	assert( SUCCEEDED(hres) );
	return m_wholeLockData;
}

//=====================================================================================//
//                           void IndexBuffer::unlockWhole()                           //
//=====================================================================================//
void IndexBuffer::unlockWhole()
{
	assert( m_wholeLockData != 0 );
	HRESULT hres = m_buffer->Unlock();
	assert( SUCCEEDED(hres) );
	m_wholeLockData = 0;
}

//=====================================================================================//
//                          void IndexBuffer::onDeviceLost()                           //
//=====================================================================================//
void IndexBuffer::onDeviceLost()
{
	if(m_bpt == bptManualReload || m_bpt == bptDynamicData)
	{
		m_buffer.Release();
	}
}

//=====================================================================================//
//                          void IndexBuffer::createBuffer()                           //
//=====================================================================================//
void IndexBuffer::createBuffer()
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

	HRESULT hres = m_device->CreateIndexBuffer
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
//                          void IndexBuffer::onDeviceReset()                          //
//=====================================================================================//
void IndexBuffer::onDeviceReset()
{
	if(m_bpt == bptManualReload || m_bpt == bptDynamicData)
	{
		createBuffer();
	}
}

}
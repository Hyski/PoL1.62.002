#pragma once

#include "Vis.h"
#include <d3d9.h>
#include <atlcomcli.h>
#include <mutual/object.h>
#include "VisBufferPoolType.h"

namespace Vis
{

#pragma warning(push)
#pragma warning(disable:4275 4251)
//=====================================================================================//
//                       class IndexBuffer : public MlRefCounted                       //
//=====================================================================================//
class VIS_BUFFER_IMPORT IndexBuffer : public MlRefCounted, boost::noncopyable
{
	BufferPoolType m_bpt;
	unsigned int m_length;
	D3DFORMAT m_format;
	int m_block;

	ATL::CComPtr<IDirect3DDevice9> m_device;
	ATL::CComPtr<IDirect3DIndexBuffer9> m_buffer;

	void *m_wholeLockData;

public:
	IndexBuffer(IDirect3DDevice9 *device, unsigned int length, D3DFORMAT fmt, BufferPoolType bpt);
	~IndexBuffer();

    void select();
	void *lock(unsigned int start, unsigned int count, unsigned int lockType);
	void unlock();

	void *getWholeLockPtr() { return m_wholeLockData; }

	unsigned int getLength() const { return m_length; }
	BufferPoolType getPoolType() const { return m_bpt; }

	void *lockWhole();
	void unlockWhole();

	void onDeviceLost();
	void onDeviceReset();

private:
	friend class IndexRangeData;

	bool isBlocked() { return m_block != 0; }
	void setBlock() { ++m_block; }
	void releaseBlock() { --m_block; }

private:
	void createBuffer();
};
#pragma warning(pop)

typedef MlHandle<IndexBuffer> HIndexBuffer;

}
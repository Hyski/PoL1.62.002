#if !defined(__VIS_BUFFER_MGR_H_INCLUDED_3626475985387791__)
#define __VIS_BUFFER_MGR_H_INCLUDED_3626475985387791__

#include "Vis.h"

#include <map>
#include <d3d9.h>
#include "fvf_vertex.h"
#include "VisIndexBuffer.h"
#include "VisBufferPoolType.h"
#include "VisIndexRangeData.h"
#include "VisVertexRangeData.h"

//=====================================================================================//
//                                    namespace Vis                                    //
//=====================================================================================//
namespace Vis
{

template<typename I> class IndexRange;
template<typename V> class VertexRange;

#pragma warning(push)
#pragma warning(disable: 4251)

//=====================================================================================//
//                                   class BufferMgr                                   //
//=====================================================================================//
class VIS_BUFFER_IMPORT BufferMgr
{
	struct Id;

	typedef std::list<IndexBuffer *> IndexBuffers_t;
	typedef std::list<VertexBuffer *> VertexBuffers_t;

	typedef std::list<IndexRangeData> IndexRanges_t;
	typedef std::list<VertexRangeData> VertexRanges_t;
	typedef std::map<Id,IndexRanges_t> IndexCluster_t;
	typedef std::map<Id,VertexRanges_t> VertexCluster_t;

	ATL::CComPtr<IDirect3DDevice9> m_device;

	IndexCluster_t m_iranges;	///< Индекс буферы
	VertexCluster_t m_vranges;	///< Вертекс буферы

	IndexBuffers_t m_ibuffers;
	VertexBuffers_t m_vbuffers;

	std::vector<IDirect3DVertexBuffer9 *> m_currentVBuffers;
	IDirect3DIndexBuffer9 *m_currentIBuffer;

public:
	BufferMgr(IDirect3DDevice9 *);
	~BufferMgr();

	template<typename V>
	std::auto_ptr< VertexRange<V> > getVertexRange(size_t size, BufferPoolType flags);

	template<typename I>
	std::auto_ptr< IndexRange<I> > getIndexRange(size_t size, BufferPoolType flags);

	void squeezeStatic();

	unsigned int vertexBufferCount() const;
	unsigned int indexBufferCount() const;
	unsigned int bytesOccupiedInVertexBuffers() const;
	unsigned int bytesUnusedInVertexBuffers() const;
	unsigned int bytesOccupiedInIndexBuffers() const;
	unsigned int bytesUnusedInIndexBuffers() const;

	static BufferMgr *instance();

	void setCurrentVBuffer(unsigned int stream, IDirect3DVertexBuffer9 *, unsigned int size);
	void setCurrentIBuffer(IDirect3DIndexBuffer9 *);
	void resetCurrentBuffers();

	void onBufferFreed(IndexRangeData *);
	void onBufferFreed(VertexRangeData *);

	void onDeviceLost();
	void onDeviceReset();

	void removeIndexBuffer(IndexBuffer *);
	void removeVertexBuffer(VertexBuffer *);

private:
	VertexRangeData *getVertexRangeData(unsigned int fvf, size_t count, BufferPoolType flags);
	IndexRangeData *getIndexRangeData(unsigned int idxSize, size_t count, BufferPoolType flags);

	template<typename T>
	unsigned int countSize(const T &, bool) const;

	template<typename C, typename I>
	I cutAPiece(C &, I, unsigned int size);

	template<typename C, typename V>
	bool isInBuffer(C &, V *);

	HVertexBuffer createVB(unsigned int fvf, BufferPoolType flags, unsigned int count);
	HIndexBuffer createIB(unsigned int fvf, BufferPoolType flags, unsigned int count);

	void debugOutputContent();

	std::pair<IndexRanges_t::iterator,IndexRanges_t *> findData(IndexRangeData *);
	std::pair<VertexRanges_t::iterator,VertexRanges_t *> findData(VertexRangeData *);

private:
	friend struct Utility;
	IDirect3DDevice9 *getDevice() { return m_device.p; }
};

#pragma warning(pop)

//=====================================================================================//
//                                struct BufferMgr::Id                                 //
//=====================================================================================//
struct BufferMgr::Id
{
	unsigned int fvf;
	BufferPoolType flags;

	Id() : fvf(0), flags(bptAutomaticReload) {}
	Id(unsigned int v, BufferPoolType l) : fvf(v), flags(l) {}

	bool operator<(const Id &r) const
	{
		if(fvf == r.fvf) return flags < r.flags;
		return fvf < r.fvf;
	}
};

class Blocker
{
	const BasicRangeData *m_data;

public:
	Blocker() : m_data(0) {}
	explicit Blocker(const BasicRangeData *r) : m_data(r) { doSetBlock(); }
	Blocker(const Blocker &b) : m_data(b.m_data) { doSetBlock(); }
	~Blocker() { doReleaseBlock(); }

	Blocker &operator=(const Blocker &b)
	{
		reset(b.m_data);
		return *this;
	}

	void reset(const BasicRangeData *r = 0)
	{
		if(m_data != r)
		{
			doReleaseBlock();
			m_data = r;
			doSetBlock();
		}
	}

private:
	void doSetBlock() const { if(m_data) m_data->setBlock(); }
	void doReleaseBlock() const { if(m_data) m_data->releaseBlock(); }
};

//=====================================================================================//
//             std::auto_ptr< VertexRange<V> > BufferMgr::getVertexRange()             //
//=====================================================================================//
template<typename V>
std::auto_ptr< VertexRange<V> > BufferMgr::getVertexRange(size_t size, BufferPoolType flags)
{
	std::auto_ptr< VertexRange<V> > result;
	result.reset(new VertexRange<V>(getVertexRangeData(V::fvf,size,flags)));
	return result;
}

//=====================================================================================//
//              std::auto_ptr< IndexRange<I> > BufferMgr::getIndexRange()              //
//=====================================================================================//
template<typename I>
std::auto_ptr< IndexRange<I> > BufferMgr::getIndexRange(size_t size, BufferPoolType flags)
{
	std::auto_ptr< IndexRange<I> > result;
	result.reset(new IndexRange<I>(getIndexRangeData(sizeof(I),size,flags)));
	return result;
}

}

#endif // !defined(__VIS_BUFFER_MGR_H_INCLUDED_3626475985387791__)
#pragma once

#include "Vis.h"
#include "VisVertexRange.h"
#include "VisMeshShapeBuffer.h"

namespace Vis
{

#pragma warning(push)
#pragma warning(disable: 4275)

//=====================================================================================//
//               class WaterStrategy : public MeshShapeBuffer::Strategy                //
//=====================================================================================//
class VIS_IMPORT WaterStrategy : public MeshShapeBuffer::Strategy
{
public:
	enum { fvf = D3DFVF_XYZ|D3DFVF_NORMAL };
	typedef fvf_vertex<fvf> Vertex_t;
	typedef VertexRange<Vertex_t> Range_t;

public:
	virtual void initRange(MeshPiece *piece)
	{
		ChannelPosition3 *xyz = piece->getChannel(channelIdPos);
		m_range = BufferMgr::instance()->getVertexRange<Vertex_t>(
			xyz->getSize(),bptAutomaticReload);
	}

	virtual void fillRange(MeshPiece *piece);

private:
	Range_t *getMyRange() const { return static_cast<Range_t *>(getRange()); }
};

#pragma warning(pop)

}

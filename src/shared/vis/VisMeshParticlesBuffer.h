#pragma once

#include "Vis.h"
#include <d3d9.h>

#include "VisBufferMgr.h"
#include "VisRangeBase.h"
#include "VisIndexRange.h"
#include "VisVertexRange.h"
#include "VisMeshPieceBuffer.h"
#include "ShaderFXShaderStateLibrary.h"

//=====================================================================================//
//                                    namespace Vis                                    //
//=====================================================================================//
namespace Vis
{

//=====================================================================================//
//                                class MeshShapeBuffer                                //
//=====================================================================================//
class VIS_IMPORT MeshParticlesBuffer : public MeshPieceBuffer
{
	typedef fvf_vertex<D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX2> Vertex_t;
	typedef VertexRange<Vertex_t> Vertices_t;
	typedef IndexRange<unsigned short> Indices_t;

	std::auto_ptr< Vertices_t > m_vertices;
	std::auto_ptr< Indices_t > m_indices;

public:
	ML_RTTI(MeshParticlesBuffer,MeshPieceBuffer);

	MeshParticlesBuffer(MeshPiece *piece, ShaderFX::HShaderStateLibrary);
	~MeshParticlesBuffer();

	Shape *getShape() const { return static_cast<Shape *>(getPiece()); }

	IndexRangeBase *getIndices() { return m_indices.get(); }
	IndexRangeBase *getIndicesOfGroup(const MeshPolyGroup *) { return 0; }
	VertexRangeBase *getVertices() { return m_vertices.get(); }
	D3DPRIMITIVETYPE getPrimitiveType() { return D3DPT_TRIANGLELIST; }
	void prepareForDrawing(IDirect3DDevice9 *, const MeshState::NodeData &);
	void prepareForDrawing(IDirect3DDevice9 *, const MeshState::NodeData &, const MeshPolyGroup *,
		ShaderFX::HShaderState);
	void drawPrimitivesWithoutShaderApply(IDirect3DDevice9 *, const MeshState::NodeData &);
	void drawPrimitivesWithoutShaderApply(IDirect3DDevice9 *, const MeshState::NodeData &, const MeshPolyGroup *);
	bool hasPolyGroup(const MeshPolyGroup *g) { return false; }

public:
	void initBuffers();
};

}

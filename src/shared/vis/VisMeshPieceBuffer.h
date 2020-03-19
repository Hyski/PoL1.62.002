#pragma once

#include "Vis.h"
#include <d3d9.h>
#include "VisAppFront.h"
#include "VisMeshState.h"
#include "ShaderFXShaderState.h"

//=====================================================================================//
//                                    namespace Vis                                    //
//=====================================================================================//
namespace Vis
{

class IndexRangeBase;
class VertexRangeBase;

#pragma warning(push)
#pragma warning(disable: 4275)

//=====================================================================================//
//                     class MeshPieceBuffer : public PieceBuffer                      //
//=====================================================================================//
class VIS_IMPORT MeshPieceBuffer : public PieceBuffer
{
	MeshPiece *m_meshPiece;
	Piece *m_piece;
	ShaderFX::HShaderState m_shader;

public:
	ML_RTTI(MeshPieceBuffer,PieceBuffer);

	MeshPieceBuffer(MeshPiece *mpiece) : m_meshPiece(mpiece), m_piece(m_meshPiece->getPiece()) {}
	~MeshPieceBuffer() {}
	
	void drawPrimitives(IDirect3DDevice9 *, const MeshState::NodeData &);
	void drawPrimitives(IDirect3DDevice9 *, const MeshState::NodeData &, ShaderFX::HShaderState);
	void drawPrimitives(IDirect3DDevice9 *, const MeshState::NodeData &, const MeshPolyGroup *,
									ShaderFX::HShaderState = 0);

	virtual IndexRangeBase *getIndices() = 0;
	virtual IndexRangeBase *getIndicesOfGroup(const MeshPolyGroup *) = 0;
	virtual VertexRangeBase *getVertices() = 0;
	virtual D3DPRIMITIVETYPE getPrimitiveType() = 0;
	virtual void prepareForDrawing(IDirect3DDevice9 *, const MeshState::NodeData &) = 0;
	virtual void prepareForDrawing(IDirect3DDevice9 *, const MeshState::NodeData &,
									const MeshPolyGroup *, ShaderFX::HShaderState) = 0;
	virtual void drawPrimitivesWithoutShaderApply(IDirect3DDevice9 *, const MeshState::NodeData &) = 0;
	virtual void drawPrimitivesWithoutShaderApply(IDirect3DDevice9 *, const MeshState::NodeData &,
									const MeshPolyGroup *) = 0;

	virtual bool hasPolyGroup(const MeshPolyGroup *) = 0;

	Piece *getPiece() const { return m_piece; }
	MeshPiece *getMeshPiece() const { return m_meshPiece; }
	ShaderFX::HShaderState getShaderState() const { return m_shader; }
	void setShaderState(ShaderFX::HShaderState s) { m_shader = s; }
};

#pragma warning(pop)

//=====================================================================================//
//                    inline void MeshPieceBuffer::drawPrimitives()                    //
//=====================================================================================//
inline void MeshPieceBuffer::drawPrimitives(IDirect3DDevice9 *dev,
											const MeshState::NodeData &node_data)
{
	if(m_shader.get())
	{
		prepareForDrawing(dev,node_data);
		const unsigned int passes = m_shader->begin();
		for(unsigned int i = 0; i < passes; ++i)
		{
			m_shader->pass(i);
			drawPrimitivesWithoutShaderApply(dev,node_data);
		}
		m_shader->end();
	}
}

//=====================================================================================//
//                    inline void MeshPieceBuffer::drawPrimitives()                    //
//=====================================================================================//
inline void MeshPieceBuffer::drawPrimitives(IDirect3DDevice9 *dev,
											const MeshState::NodeData &node_data,
											ShaderFX::HShaderState shd)
{
	if(shd.get())
	{
		prepareForDrawing(dev,node_data);
		const unsigned int passes = shd->begin();
		for(unsigned int i = 0; i < passes; ++i)
		{
			shd->pass(i);
			drawPrimitivesWithoutShaderApply(dev,node_data);
		}
		shd->end();
	}
}

//=====================================================================================//
//                    inline void MeshPieceBuffer::drawPrimitives()                    //
//=====================================================================================//
inline void MeshPieceBuffer::drawPrimitives(IDirect3DDevice9 *dev,
											const MeshState::NodeData &node_data,
											const MeshPolyGroup *group,
											ShaderFX::HShaderState shader)
{
	if(!group) return;
	ShaderFX::HShaderState shd = shader;
	if(!shd.get()) shd = m_shader;
	if(!hasPolyGroup(group)) return;

	if(shd.get())
	{
		prepareForDrawing(dev,node_data,group,shd);
		const unsigned int passes = shd->begin();
		for(unsigned int i = 0; i < passes; ++i)
		{
			shd->pass(i);
			drawPrimitivesWithoutShaderApply(dev,node_data,group);
		}
		shd->end();
	}
}

}

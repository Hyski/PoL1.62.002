#pragma once

#include "Vis.h"
#include <d3d9.h>

#include "VisBufferMgr.h"
#include "VisRangeBase.h"
#include "VisIndexRange.h"
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
class VIS_IMPORT MeshShapeBuffer : public MeshPieceBuffer
{
public:
	class Strategy;

private:
	typedef std::map<const MeshPolyGroup *, IndexRange<unsigned short> *> GroupIndices_t;

	std::auto_ptr<Strategy> m_strategy;
	std::auto_ptr< IndexRange<unsigned short> > m_indices;
	GroupIndices_t m_groupIndices;

public:
	ML_RTTI(MeshShapeBuffer,MeshPieceBuffer);

	MeshShapeBuffer(MeshPiece *piece, ShaderFX::HShaderStateLibrary);
	~MeshShapeBuffer();

	Shape *getShape() const { return static_cast<Shape *>(getPiece()); }

	IndexRangeBase *getIndices() { return m_indices.get(); }
	IndexRangeBase *getIndicesOfGroup(const MeshPolyGroup *);
	VertexRangeBase *getVertices();
	D3DPRIMITIVETYPE getPrimitiveType() { return D3DPT_TRIANGLELIST; }
	void prepareForDrawing(IDirect3DDevice9 *, const MeshState::NodeData &);
	void prepareForDrawing(IDirect3DDevice9 *, const MeshState::NodeData &, const MeshPolyGroup *,
		ShaderFX::HShaderState);
	void drawPrimitivesWithoutShaderApply(IDirect3DDevice9 *, const MeshState::NodeData &);
	void drawPrimitivesWithoutShaderApply(IDirect3DDevice9 *, const MeshState::NodeData &, const MeshPolyGroup *);
	bool hasPolyGroup(const MeshPolyGroup *g) { return m_groupIndices.find(g) != m_groupIndices.end(); }

private:
	void initStrategy();
	void fillRange();

	void initIndices();
	void fillIndices();
	void generateTextureSpace();

	void makeGroupBuffers();

	void onVerticesReloaded();
	void onIndicesReloaded();
	void fillGroupBuffer(IndexRange<unsigned short> *irange,
						 std::pair<PolyGroup::iterator,PolyGroup::iterator> polys);
};

//=====================================================================================//
//                           class MeshShapeBuffer::Strategy                           //
//=====================================================================================//
class MeshShapeBuffer::Strategy
{
protected:
	std::auto_ptr<VertexRangeBase> m_range;

public:
	virtual ~Strategy() {}
	VertexRangeBase *getRange() const { return m_range.get(); }
	virtual void initRange(MeshPiece *) = 0;
	virtual void fillRange(MeshPiece *) = 0;
};


}

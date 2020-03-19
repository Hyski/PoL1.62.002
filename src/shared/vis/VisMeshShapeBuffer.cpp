#include "Vis.h"
#include "VisUtility.h"
#include "VisMeshPiece.h"
#include "ShaderFXFront.h"
#include "VisVertexRange.h"
#include "VisWaterStrategy.h"
#include "ShaderFXParameter.h"
#include "VisMeshShapeBuffer.h"
#include "ShaderFXShaderState.h"
#include "ShaderFXShaderStateLibrary.h"

#include <boost/bind.hpp>

namespace Vis
{

//=====================================================================================//
//                                   enum VertexType                                   //
//=====================================================================================//
enum VertexType
{
	vtPos					=	D3DFVF_XYZ,
	vtPosDif				=	D3DFVF_XYZ|D3DFVF_DIFFUSE,
	vtPosNormal				=	D3DFVF_XYZ|D3DFVF_NORMAL,
	vtPosNormalUV			=	D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1,
	vtPosNormalDifUV		=	D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX1,
	vtPosNormalUVTangent	=	D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX2|D3DFVF_TEXCOORDSIZE3(1)
};

namespace MeshShapeBufferDetail
{
//=====================================================================================//
//                class StrategyImpl : public MeshShapeBuffer::Strategy                //
//=====================================================================================//
template<VertexType T>
class StrategyImpl : public MeshShapeBuffer::Strategy
{
	typedef fvf_vertex<T> Vertex_t;
	typedef VertexRange<Vertex_t> Range_t;

public:
	~StrategyImpl()
	{
		getMyRange()->unregisterCallback();
	}

	virtual void initRange(MeshPiece *piece)
	{
		ChannelPosition3 *xyz = piece->getChannel(channelIdPos);
		m_range = BufferMgr::instance()->getVertexRange<Vertex_t>(
			xyz->getSize(),bptManualReload);
		getMyRange()->registerCallback(boost::bind(fillRange,this,piece));
	}

	virtual void fillRange(MeshPiece *piece);

private:
	Range_t *getMyRange() const { return static_cast<Range_t *>(getRange()); }
};

//=====================================================================================//
//     void StrategyImpl<MeshShapeBuffer::Strategy::vtPosNormalDifUV>::fillRange()     //
//=====================================================================================//
template<>
void StrategyImpl<vtPosNormalDifUV>::fillRange(MeshPiece *piece)
{
	ChannelPosition3 *xyz = piece->getChannel(channelIdPos);
	ChannelNormal3 *normal = piece->getChannel(channelIdNormal);
	ChannelTexCoord2 *uv = piece->getChannel(channelIdTexCoord);
	ChannelUInt *diffuse = piece->getChannel(channelIdDiffuse);

	Range_t::LockRange_t lock = getMyRange()->lock();
	unsigned int index = 0;

	for(Range_t::iterator i = lock.first; i != lock.second; ++i, ++index)
	{
		i->m_pos = xyz->getData()[index];
		i->m_normal = normal->getData()[index];
		i->m_uv = uv->getData()[index];
		i->m_diffuse = diffuse->getData()[index];
	}

	getMyRange()->unlock();
}

template<>
void StrategyImpl<vtPos>::fillRange(MeshPiece *piece)
{
	ChannelPosition3 *xyz = piece->getChannel(channelIdPos);

	Range_t::LockRange_t lock = getMyRange()->lock();
	unsigned int index = 0;

	for(Range_t::iterator i = lock.first; i != lock.second; ++i, ++index)
	{
		i->m_pos = xyz->getData()[index];
	}

	getMyRange()->unlock();
}

//=====================================================================================//
//      void StrategyImpl<MeshShapeBuffer::Strategy::vtPosNormalUV>::fillRange()       //
//=====================================================================================//
template<>
void StrategyImpl<vtPosNormalUV>::fillRange(MeshPiece *piece)
{
	ChannelPosition3 *xyz = piece->getChannel(channelIdPos);
	ChannelNormal3 *normal = piece->getChannel(channelIdNormal);
	ChannelTexCoord2 *uv = piece->getChannel(channelIdTexCoord);

	Range_t::LockRange_t lock = getMyRange()->lock();
	unsigned int index = 0;

	for(Range_t::iterator i = lock.first; i != lock.second; ++i, ++index)
	{
		i->m_pos = xyz->getData()[index];
		i->m_normal = normal->getData()[index];
		i->m_uv = uv->getData()[index];
	}

	getMyRange()->unlock();
}

//=====================================================================================//
//                      void StrategyImpl<vtPosDif>::fillRange()                       //
//=====================================================================================//
template<>
void StrategyImpl<vtPosDif>::fillRange(MeshPiece *piece)
{
	ChannelPosition3 *xyz = piece->getChannel(channelIdPos);
	ChannelUInt *diffuse = piece->getChannel(channelIdDiffuse);

	Range_t::LockRange_t lock = getMyRange()->lock();
	unsigned int index = 0;

	for(Range_t::iterator i = lock.first; i != lock.second; ++i, ++index)
	{
		i->m_pos = xyz->getData()[index];
		i->m_diffuse = diffuse->getData()[index];
	}

	getMyRange()->unlock();
}

//=====================================================================================//
//       void StrategyImpl<MeshShapeBuffer::Strategy::vtPosNormal>::fillRange()        //
//=====================================================================================//
template<>
void StrategyImpl<vtPosNormal>::fillRange(MeshPiece *piece)
{
	ChannelPosition3 *xyz = piece->getChannel(channelIdPos);
	ChannelNormal3 *normal = piece->getChannel(channelIdNormal);
	//
	Range_t::LockRange_t lock = getMyRange()->lock();
	unsigned int index = 0;
	for(Range_t::iterator i = lock.first; i != lock.second; ++i, ++index)
	{
		i->m_pos = xyz->getData()[index];
		i->m_normal = normal->getData()[index];
	}
	getMyRange()->unlock();
}

//=====================================================================================//
//      void StrategyImpl<MeshShapeBuffer::Strategy::vtPosNormalUV>::fillRange()       //
//=====================================================================================//
template<>
void StrategyImpl<vtPosNormalUVTangent>::fillRange(MeshPiece *piece)
{
	ChannelPosition3 *xyz = piece->getChannel(channelIdPos);
	ChannelNormal3 *normal = piece->getChannel(channelIdNormal);
	ChannelTexCoord2 *uv = piece->getChannel(channelIdTexCoord);
	ChannelNormal3 *tangent = piece->getChannel(channelIdTangent);

	Range_t::LockRange_t lock = getMyRange()->lock();
	unsigned int index = 0;

	for(Range_t::iterator i = lock.first; i != lock.second; ++i, ++index)
	{
		i->m_pos = xyz->getData()[index];
		i->m_normal = normal->getData()[index];
		i->m_uv = uv->getData()[index];
		i->m_uvw2 = tangent->getData()[index] + mll::algebra::origin;
	}

	getMyRange()->unlock();
}

}

using namespace MeshShapeBufferDetail;

//=====================================================================================//
//                         MeshShapeBuffer::MeshShapeBuffer()                          //
//=====================================================================================//
MeshShapeBuffer::MeshShapeBuffer(MeshPiece *piece, ShaderFX::HShaderStateLibrary ssl)
:	MeshPieceBuffer(piece)
{
	initStrategy();
	initIndices();
	fillIndices();
	Material *mat = piece->getPiece()->getMaterial();
	ShaderFX::HShaderState shader;

	if(mat->getBody().empty())
	{
		if(!ssl.get())
		{
			shader = ShaderFX::Front::instance()->getShaderState(mat->getName());
		}
		else
		{
			shader = ssl->getShaderState(mat->getName());
		}
	}
	else
	{
		shader = ShaderFX::Front::instance()->getEmbeddedShaderState(mat->getBody());
	}

	makeGroupBuffers();

	setShaderState(shader);
}

//=====================================================================================//
//                         MeshShapeBuffer::~MeshShapeBuffer()                         //
//=====================================================================================//
MeshShapeBuffer::~MeshShapeBuffer()
{
	for(GroupIndices_t::iterator i = m_groupIndices.begin(); i != m_groupIndices.end(); ++i)
	{
		i->second->unregisterCallback();
		delete i->second;
		i->second = 0;
	}

	m_indices->unregisterCallback();
}

//=====================================================================================//
//                      void MeshShapeBuffer::prepareForDrawing()                      //
//=====================================================================================//
void MeshShapeBuffer::prepareForDrawing(IDirect3DDevice9 *dev,
										const MeshState::NodeData &node_data)
{
	m_strategy->getRange()->select();
	m_indices->select();
	
	getShaderState()->getParameter("World")->setMatrix(node_data.getMatrix());
}

//=====================================================================================//
//                      void MeshShapeBuffer::prepareForDrawing()                      //
//=====================================================================================//
void MeshShapeBuffer::prepareForDrawing(IDirect3DDevice9 *dev,
										const MeshState::NodeData &node_data,
										const MeshPolyGroup *pg,
										ShaderFX::HShaderState state)
{
	m_strategy->getRange()->select();
	getIndicesOfGroup(pg)->select();
	
	state->getParameter("World")->setMatrix(node_data.getMatrix());
}


//=====================================================================================//
//              void MeshShapeBuffer::drawPrimitivesWithoutShaderApply()               //
//=====================================================================================//
void MeshShapeBuffer::drawPrimitivesWithoutShaderApply(IDirect3DDevice9 *dev,
													   const MeshState::NodeData &)
{
	VertexRangeData * const vrd = m_strategy->getRange()->getData();
	IndexRangeData * const ird = m_indices->getData();

	HRESULT hres = dev->DrawIndexedPrimitive
		(
		D3DPT_TRIANGLELIST,
		vrd->getStart(),
		0,
		vrd->getCount(),
		ird->getStart(),
		ird->getCount()/3
		);
	assert( SUCCEEDED(hres) );
}

//=====================================================================================//
//              void MeshShapeBuffer::drawPrimitivesWithoutShaderApply()               //
//=====================================================================================//
void MeshShapeBuffer::drawPrimitivesWithoutShaderApply(IDirect3DDevice9 *dev,
													   const MeshState::NodeData &,
													   const MeshPolyGroup *pg)
{
	assert( pg != 0 );
	VertexRangeData * const vrd = m_strategy->getRange()->getData();
	IndexRangeData * const ird = getIndicesOfGroup(pg)->getData();

	HRESULT hres = dev->DrawIndexedPrimitive
		(
		D3DPT_TRIANGLELIST,
		vrd->getStart(),
		0,
		vrd->getCount(),
		ird->getStart(),
		ird->getCount()/3
		);
	assert( SUCCEEDED(hres) );
}

//=====================================================================================//
//                     void MeshShapeBuffer::determineVertexType()                     //
//=====================================================================================//
void MeshShapeBuffer::initStrategy()
{
	if(getMeshPiece()->getPiece()->getOptions().optionExists("water"))
	{
		m_strategy.reset(new WaterStrategy);
	}
	else if(getMeshPiece()->getChannel(channelIdPos)
		&& getMeshPiece()->getChannel(channelIdNormal)
		&& getMeshPiece()->getChannel(channelIdTexCoord)
		&& getMeshPiece()->getChannel(channelIdDiffuse))
	{
		m_strategy.reset(new StrategyImpl<vtPosNormalDifUV>);
	}
	else if(getMeshPiece()->getChannel(channelIdPos)
		&& getMeshPiece()->getChannel(channelIdNormal)
		&& getMeshPiece()->getChannel(channelIdTexCoord))
	{
		if(getMeshPiece()->getPiece()->getMaterial()->getOptions().optionExists("bump") &&
			getMeshPiece()->getPiece()->getMaterial()->getOptions().getBoolean("bump"))
		{
			generateTextureSpace();
			m_strategy.reset(new StrategyImpl<vtPosNormalUVTangent>);
		}
		else
		{
			m_strategy.reset(new StrategyImpl<vtPosNormalUV>);
		}
	}
	else if(getMeshPiece()->getChannel(channelIdPos)
		&& getMeshPiece()->getChannel(channelIdNormal))
	{
		m_strategy.reset(new StrategyImpl<vtPosNormal>);
	}
	else if(getMeshPiece()->getChannel(channelIdPos)
		&& getMeshPiece()->getChannel(channelIdDiffuse))
	{
		m_strategy.reset(new StrategyImpl<vtPosDif>);
	}
	else if(getMeshPiece()->getChannel(channelIdPos))
	{
		m_strategy.reset(new StrategyImpl<vtPos>);
	}

	if(m_strategy.get())
	{
		m_strategy->initRange(getMeshPiece());
		m_strategy->fillRange(getMeshPiece());
	}
}

//=====================================================================================//
//                    void MeshShapeBuffer::generateTextureSpace()                     //
//=====================================================================================//
void MeshShapeBuffer::generateTextureSpace()
{
	const float SMALL_FLOAT = 1e-12f;

	struct vertex
	{
		vector3 pos;
		vector2 tex;
		vector3 surf;
		vector3 tan;
	};

	unsigned int i, index = 0;

// vertex and index counts
	unsigned int v_count = getMeshPiece()->getChannel(channelIdPos)->getSize();
	unsigned int i_count = getMeshPiece()->getChannel(channelIdIndex)->getSize();

// s, t, (s x t) vectors - local basis in texture space for each vertex
	ChannelNormal3::Data_t t_vect(v_count);
	ChannelNormal3::Data_t s_vect(v_count);
	ChannelNormal3::Data_t st_vect(v_count);

// create new tangent channel
	ChannelNormal3 *tangent = getMeshPiece()->addChannel(channelIdTangent);

// clear the basis vectors
	for (i = 0; i < v_count; i++)
	{
		t_vect[i] = vector3(0.0f , 0.0f, 0.0f);
		s_vect[i] = vector3(0.0f, 0.0f, 0.0f);
	}

// get existed channels
	ChannelPosition3 *xyz = getMeshPiece()->getChannel(channelIdPos);
	ChannelTexCoord2 *uv = getMeshPiece()->getChannel(channelIdTexCoord);
	ChannelIndex16 *idxs = getMeshPiece()->getChannel(channelIdIndex);

	for(i = 0; i < i_count; i += 3 )
	{
		unsigned int tri_index[3];
		vertex	v0, v1, v2;	
		vector2 du, dv;
		vector3 cp, edge01, edge02;

// clear vertex local basis
		v0.surf  = vector3(0.0f, 0.0f, 0.0f);
		v0.tan  = vector3(0.0f, 0.0f, 0.0f);

		v1.surf  = vector3(0.0f, 0.0f, 0.0f);
		v1.tan  = vector3(0.0f, 0.0f, 0.0f);

		v2.surf  = vector3(0.0f, 0.0f, 0.0f);
		v2.tan  = vector3(0.0f, 0.0f, 0.0f);

// get triangle
		tri_index[0] = idxs->getData()[i];
		tri_index[1] = idxs->getData()[i+1];
		tri_index[2] = idxs->getData()[i+2];

// fill info into vertex
		v0.pos = xyz->getData()[tri_index[0]] - mll::algebra::origin;
		v0.tex = uv->getData()[tri_index[0]] - mll::algebra::origin;

		v1.pos = xyz->getData()[tri_index[1]] - mll::algebra::origin;
		v1.tex = uv->getData()[tri_index[1]] - mll::algebra::origin;

		v2.pos = xyz->getData()[tri_index[2]] - mll::algebra::origin;
		v2.tex = uv->getData()[tri_index[2]] - mll::algebra::origin;

// x, s, t
		edge01.x = v1.pos.x - v0.pos.x;
		edge01.y = v1.tex.x - v0.tex.x;
		edge01.z = v1.tex.y - v0.tex.y;

		edge02.x = v2.pos.x - v0.pos.x;
		edge02.y = v2.tex.x - v0.tex.x;
		edge02.z = v2.tex.y - v0.tex.y;

		cp = edge01.cross(edge02);
		if ( fabsf(cp.x) > SMALL_FLOAT )
		{
			v0.surf.x += -cp.y / cp.x;
			v0.tan.x += -cp.z / cp.x;

			v1.surf.x += -cp.y / cp.x;
			v1.tan.x += -cp.z / cp.x;
			
			v2.surf.x += -cp.y / cp.x;
			v2.tan.x += -cp.z / cp.x;
		}

// y, s, t
		edge01.x = v1.pos.y - v0.pos.y;
		edge01.y = v1.tex.x - v0.tex.x;
		edge01.z = v1.tex.y - v0.tex.y;

		edge02.x = v2.pos.y - v0.pos.y;
		edge02.y = v2.tex.x - v0.tex.x;
		edge02.z = v2.tex.y - v0.tex.y;

		cp = edge01.cross(edge02);
		if ( fabs(cp.x) > SMALL_FLOAT )
		{
			v0.surf.y += -cp.y / cp.x;
			v0.tan.y += -cp.z / cp.x;

			v1.surf.y += -cp.y / cp.x;
			v1.tan.y += -cp.z / cp.x;
			
			v2.surf.y += -cp.y / cp.x;
			v2.tan.y += -cp.z / cp.x;
		}

// z, s, t
		edge01.x = v1.pos.z - v0.pos.z;
		edge01.y = v1.tex.x - v0.tex.x;
		edge01.z = v1.tex.y - v0.tex.y;

		edge02.x = v2.pos.z - v0.pos.z;
		edge02.y = v2.tex.x - v0.tex.x;
		edge02.z = v2.tex.y - v0.tex.y;

		cp = edge01.cross(edge02);
		if ( fabsf(cp.x) > SMALL_FLOAT )
		{
			v0.surf.z += -cp.y / cp.x;
			v0.tan.z += -cp.z / cp.x;

			v1.surf.z += -cp.y / cp.x;
			v1.tan.z += -cp.z / cp.x;
			
			v2.surf.z += -cp.y / cp.x;
			v2.tan.z += -cp.z / cp.x;
		}

		s_vect[tri_index[0]] +=  v0.surf;
		s_vect[tri_index[1]] +=  v1.surf;
		s_vect[tri_index[2]] +=  v2.surf;

		t_vect[tri_index[0]] +=  v0.tan;
		t_vect[tri_index[1]] +=  v1.tan;
		t_vect[tri_index[2]] +=  v2.tan;
	}

// normalize tangent and copy
	for(i = 0; i < v_count; i ++ )
	{
		t_vect[i].normalize();
		s_vect[i].normalize();
	}

	tangent->swapData(t_vect);
}

//=====================================================================================//
//                         void MeshShapeBuffer::initIndices()                         //
//=====================================================================================//
void MeshShapeBuffer::initIndices()
{
	unsigned int count = getMeshPiece()->getChannel(channelIdIndex)->getSize();
	m_indices = BufferMgr::instance()->getIndexRange<unsigned short>(count,bptManualReload);
	m_indices->registerCallback(boost::bind(onIndicesReloaded,this));
}

//=====================================================================================//
//                         void MeshShapeBuffer::fillIndices()                         //
//=====================================================================================//
void MeshShapeBuffer::fillIndices()
{
	ChannelIndex16 *idxs = getMeshPiece()->getChannel(channelIdIndex);
	unsigned int index = 0;
	IndexRange<unsigned short>::LockRange_t lock = m_indices->lock();

	for(IndexRange<unsigned short>::iterator i = lock.first; i != lock.second; ++i, ++index)
	{
		*i = idxs->getData()[index];
	}

	m_indices->unlock();
}

//=====================================================================================//
//                   VertexRangeBase *MeshShapeBuffer::getVertices()                   //
//=====================================================================================//
VertexRangeBase *MeshShapeBuffer::getVertices()
{
	return m_strategy->getRange();
}

//=====================================================================================//
//                      void MeshShapeBuffer::makeGroupBuffers()                       //
//=====================================================================================//
void MeshShapeBuffer::makeGroupBuffers()
{
	Mesh *mesh = getMeshPiece()->getMesh();
	Mesh::PolyGroups_t::const_iterator i = mesh->getPolyGroups().begin();
	ChannelIndex16 *indices = getMeshPiece()->getChannel(channelIdIndex);

	for(; i != mesh->getPolyGroups().end(); ++i)
	{
		typedef std::pair<PolyGroup::iterator,PolyGroup::iterator> PolyRange_t;
		PolyRange_t polys = i->second.getPolyIdsOfPiece(getMeshPiece());
		const int count = std::distance(polys.first,polys.second);
		if(count == 0) continue;

		std::auto_ptr< IndexRange<unsigned short> > irange;
		irange = BufferMgr::instance()->getIndexRange<unsigned short>(count*3,bptManualReload);
		irange->registerCallback(boost::bind(fillGroupBuffer,this,irange.get(),polys));

		fillGroupBuffer(irange.get(),polys);

		m_groupIndices[&i->second] = irange.release();
	}
}

//=====================================================================================//
//                       void MeshShapeBuffer::fillGroupBuffer()                       //
//=====================================================================================//
void MeshShapeBuffer::fillGroupBuffer(IndexRange<unsigned short> *irange,
									  std::pair<PolyGroup::iterator,PolyGroup::iterator> polys)
{
	typedef std::pair<PolyGroup::iterator,PolyGroup::iterator> PolyRange_t;
	ChannelIndex16 *indices = getMeshPiece()->getChannel(channelIdIndex);

	IndexRange<unsigned short>::LockRange_t lock = irange->lock();
	unsigned int idx = 0;
	for(PolyGroup::iterator j = polys.first; j != polys.second; ++j, idx += 3)
	{
		lock.first[idx] = indices->getData()[j->m_index*3];
		lock.first[idx+1] = indices->getData()[j->m_index*3+1];
		lock.first[idx+2] = indices->getData()[j->m_index*3+2];
	}
	irange->unlock();
}

//=====================================================================================//
//                IndexRangeBase *MeshShapeBuffer::getIndicesOfGroup()                 //
//=====================================================================================//
IndexRangeBase *MeshShapeBuffer::getIndicesOfGroup(const MeshPolyGroup *g)
{
	GroupIndices_t::iterator i = m_groupIndices.find(g);
	if(i == m_groupIndices.end()) return 0;
	return i->second;
}

//=====================================================================================//
//                      void MeshShapeBuffer::onIndicesReloaded()                      //
//=====================================================================================//
void MeshShapeBuffer::onIndicesReloaded()
{
	fillIndices();
}

}
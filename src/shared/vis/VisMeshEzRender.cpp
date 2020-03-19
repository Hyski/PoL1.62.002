#include "Vis.h"
#include <d3d9.h>
#include "VisMeshState.h"
#include "VisMeshEzRender.h"
#include "VisMeshPieceBuffer.h"
#include "ShaderFXShaderState.h"

namespace Vis
{

namespace MeshEzRenderDetail
{
struct ApplyDirLight
{
	ShaderFX::DirLight m_dl;

	ApplyDirLight(const ShaderFX::DirLight &dl) : m_dl(dl) {}
	void operator()(MeshPieceBuffer *pb) const
	{
		if(pb->getShaderState()->getDirLightCount() >= 1)
		{
			pb->getShaderState()->setDirLight(0,m_dl);
		}
	}
};
}

using namespace MeshEzRenderDetail;

//=====================================================================================//
//                             void MeshEzRender::apply()                              //
//=====================================================================================//
template<typename Fn>
void MeshEzRender::apply(Fn fn)
{
	assert( m_state.get() );

	const Vis::MeshState::NodeTree_t &nodes = m_state->getNodeTree();
	for(Vis::MeshState::iterator i = nodes.begin(); i != nodes.end(); ++i)
	{
		const Vis::Mesh::NodeData &node_data = *i->m_node;
		for(unsigned int piece = 0; piece != node_data.m_pieces.size(); ++piece)
		{
			Vis::PieceBuffer *buf = node_data.m_pieces[piece]->getBuffer();
			assert( can(buf->rtti_dynamic()).downcast_to(Vis::MeshPieceBuffer::rtti_static()) );

			Vis::MeshPieceBuffer *pieceBuf = static_cast<Vis::MeshPieceBuffer *>(buf);
			if(!pieceBuf) continue;
			fn(pieceBuf);
		}
	}
}

//=====================================================================================//
//                            MeshEzRender::MeshEzRender()                             //
//=====================================================================================//
MeshEzRender::MeshEzRender()
{
}

//=====================================================================================//
//                            MeshEzRender::MeshEzRender()                             //
//=====================================================================================//
MeshEzRender::MeshEzRender(HMeshState state)
:	m_state(state)
{
}

//=====================================================================================//
//                            MeshEzRender::~MeshEzRender()                            //
//=====================================================================================//
MeshEzRender::~MeshEzRender()
{
}

//=====================================================================================//
//                             void MeshEzRender::reset()                              //
//=====================================================================================//
void MeshEzRender::reset(HMeshState state)
{
	m_state = state;
}

//=====================================================================================//
//                             void MeshEzRender::render()                             //
//=====================================================================================//
void MeshEzRender::render(IDirect3DDevice9 *dev)
{
	assert( m_state.get() );

	const Vis::MeshState::NodeTree_t &nodes = m_state->getNodeTree();
	for(Vis::MeshState::iterator i = nodes.begin(); i != nodes.end(); ++i)
	{
		const Vis::Mesh::NodeData &node_data = *i->m_node;
		for(unsigned int piece = 0; piece != node_data.m_pieces.size(); ++piece)
		{
			Vis::PieceBuffer *buf = node_data.m_pieces[piece]->getBuffer();
			assert( can(buf->rtti_dynamic()).downcast_to(Vis::MeshPieceBuffer::rtti_static()) );

			Vis::MeshPieceBuffer *pieceBuf = static_cast<Vis::MeshPieceBuffer *>(buf);
			if(!pieceBuf) continue;
			pieceBuf->drawPrimitives(dev,*i);
		}
	}
}

//=====================================================================================//
//                             void MeshEzRender::render()                             //
//=====================================================================================//
void MeshEzRender::render(IDirect3DDevice9 *dev, ShaderFX::HShaderState shd)
{
	assert( m_state.get() );

	const Vis::MeshState::NodeTree_t &nodes = m_state->getNodeTree();
	for(Vis::MeshState::iterator i = nodes.begin(); i != nodes.end(); ++i)
	{
		const Vis::Mesh::NodeData &node_data = *i->m_node;
		for(unsigned int piece = 0; piece != node_data.m_pieces.size(); ++piece)
		{
			Vis::PieceBuffer *buf = node_data.m_pieces[piece]->getBuffer();
			assert( can(buf->rtti_dynamic()).downcast_to(Vis::MeshPieceBuffer::rtti_static()) );

			Vis::MeshPieceBuffer *pieceBuf = static_cast<Vis::MeshPieceBuffer *>(buf);
			if(!pieceBuf) continue;
			pieceBuf->drawPrimitives(dev,*i,shd);
		}
	}
}

//=====================================================================================//
//                             void MeshEzRender::render()                             //
//=====================================================================================//
void MeshEzRender::render(IDirect3DDevice9 *dev, const MeshPolyGroup *pg,
						  ShaderFX::HShaderState shd)
{
	assert( m_state.get() );

	const Vis::MeshState::NodeTree_t &nodes = m_state->getNodeTree();
	for(Vis::MeshState::iterator i = nodes.begin(); i != nodes.end(); ++i)
	{
		const Vis::Mesh::NodeData &node_data = *i->m_node;
		for(unsigned int piece = 0; piece != node_data.m_pieces.size(); ++piece)
		{
			Vis::PieceBuffer *buf = node_data.m_pieces[piece]->getBuffer();
			assert( can(buf->rtti_dynamic()).downcast_to(Vis::MeshPieceBuffer::rtti_static()) );

			Vis::MeshPieceBuffer *pieceBuf = static_cast<Vis::MeshPieceBuffer *>(buf);
			if(!pieceBuf) continue;
			pieceBuf->drawPrimitives(dev,*i,pg,shd);
		}
	}
}

//=====================================================================================//
//                     void MeshEzRender::applyDirectionalLight()                      //
//=====================================================================================//
void MeshEzRender::applyDirectionalLight(const ShaderFX::DirLight &lt)
{
	apply(ApplyDirLight(lt));
}

//=====================================================================================//
//                             void MeshEzRender::render()                             //
//=====================================================================================//
void MeshEzRender::render(HMeshState s, IDirect3DDevice9 *d)
{
	reset(s);
	render(d);
}

//=====================================================================================//
//                             void MeshEzRender::render()                             //
//=====================================================================================//
void MeshEzRender::render(HMeshState s, IDirect3DDevice9 *d, ShaderFX::HShaderState ss)
{
	reset(s);
	render(d,ss);
}

//=====================================================================================//
//                             void MeshEzRender::render()                             //
//=====================================================================================//
void MeshEzRender::render(HMeshState s, IDirect3DDevice9 *d, const MeshPolyGroup *pg,
						  ShaderFX::HShaderState ss)
{
	reset(s);
	render(d,pg,ss);
}

//=====================================================================================//
//                     void MeshEzRender::applyDirectionalLight()                      //
//=====================================================================================//
void MeshEzRender::applyDirectionalLight(HMeshState s, const ShaderFX::DirLight &dl)
{
	reset(s);
	applyDirectionalLight(dl);
}


}
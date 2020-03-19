#include "Vis.h"
#include "VisMeshPiece.h"
#include "ShaderFXFront.h"
#include "VisVertexRange.h"
#include "VisWaterStrategy.h"
#include "ShaderFXParameter.h"
#include "ShaderFXShaderState.h"
#include "VisMeshParticlesBuffer.h"
#include "ShaderFXShaderStateLibrary.h"

namespace Vis
{

//=====================================================================================//
//                     MeshParticlesBuffer::MeshParticlesBuffer()                      //
//=====================================================================================//
MeshParticlesBuffer::MeshParticlesBuffer(MeshPiece *piece, ShaderFX::HShaderStateLibrary ssl)
:	MeshPieceBuffer(piece)
{
	Material *mat = piece->getPiece()->getMaterial();
	ShaderFX::HShaderState shader;

	initBuffers();

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

	setShaderState(shader);
}

//=====================================================================================//
//                     MeshParticlesBuffer::~MeshParticlesBuffer()                     //
//=====================================================================================//
MeshParticlesBuffer::~MeshParticlesBuffer()
{
}

//=====================================================================================//
//                    void MeshParticlesBuffer::prepareForDrawing()                    //
//=====================================================================================//
void MeshParticlesBuffer::prepareForDrawing(IDirect3DDevice9 *dev,
										const MeshState::NodeData &node_data)
{
	m_vertices->select();
	m_indices->select();
	
	getShaderState()->getParameter("World")->setMatrix(node_data.getMatrix());
}

//=====================================================================================//
//                    void MeshParticlesBuffer::prepareForDrawing()                    //
//=====================================================================================//
void MeshParticlesBuffer::prepareForDrawing(IDirect3DDevice9 *dev,
										const MeshState::NodeData &node_data,
										const MeshPolyGroup *pg,
										ShaderFX::HShaderState state)
{
}

//=====================================================================================//
//            void MeshParticlesBuffer::drawPrimitivesWithoutShaderApply()             //
//=====================================================================================//
void MeshParticlesBuffer::drawPrimitivesWithoutShaderApply(IDirect3DDevice9 *dev,
													   const MeshState::NodeData &)
{
	VertexRangeData * const vrd = m_vertices->getData();
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
//            void MeshParticlesBuffer::drawPrimitivesWithoutShaderApply()             //
//=====================================================================================//
void MeshParticlesBuffer::drawPrimitivesWithoutShaderApply(IDirect3DDevice9 *dev,
													   const MeshState::NodeData &,
													   const MeshPolyGroup *pg)
{
}

//=====================================================================================//
//                       void MeshParticlesBuffer::initBuffers()                       //
//=====================================================================================//
void MeshParticlesBuffer::initBuffers()
{
	ChannelPosition3 *pos = getMeshPiece()->getChannel(channelIdPos);
	ChannelFloat *size = static_cast<ChannelFloat*>(*getMeshPiece()->getChannelByName("size"));
	m_vertices = BufferMgr::instance()->getVertexRange<Vertex_t>(pos->getSize()*4,bptAutomaticReload);
	m_indices = BufferMgr::instance()->getIndexRange<unsigned short>(pos->getSize()*6,bptAutomaticReload);

	point3 center = (getMeshPiece()->getObb()[obb3::pt_xyz] - getMeshPiece()->getObb()[obb3::pt_origin])*0.5f
		+ getMeshPiece()->getObb()[obb3::pt_origin];

	Vertices_t::LockRange_t vlock = m_vertices->lock();
	Indices_t::LockRange_t ilock = m_indices->lock();

	point2 uv[4] = {	point2(0.0f,0.0f),
						point2(1.0f,0.0f),
						point2(1.0f,1.0f),
						point2(0.0f,1.0f) };

	for(unsigned int i = 0; i < pos->getSize(); ++i)
	{
		float psize = (((float)rand()/32767)*0.2f+0.9f) * size->getData()[i];

		for(unsigned int j = 0; j < 4; ++j)
		{
			vlock.first[i*4 + j].m_pos = pos->getData()[i];
			vlock.first[i*4 + j].m_uv = uv[j];
			vlock.first[i*4 + j].m_uv2 = point2(psize,0.0f);
			vlock.first[i*4 + j].m_diffuse = 0xFFFFFFFF;
			vlock.first[i*4 + j].m_normal = vector3(pos->getData()[i]-center).normalize();
		}

		ilock.first[i*6 + 0] = i*4 + 0;
		ilock.first[i*6 + 1] = i*4 + 1;
		ilock.first[i*6 + 2] = i*4 + 2;
		ilock.first[i*6 + 3] = i*4 + 0;
		ilock.first[i*6 + 4] = i*4 + 3;
		ilock.first[i*6 + 5] = i*4 + 2;
	}

	m_vertices->unlock();
	m_indices->unlock();
}

}
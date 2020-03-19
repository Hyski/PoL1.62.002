#include "Vis.h"
#include "VisMeshPiece.h"
#include "HashTree.h"

namespace Vis
{

using mll::geometry::aabb3;
using mll::geometry::plane;

namespace{
//=====================================================================================//
//                                   class Triangle                                    //
//=====================================================================================//
class Triangle
{
	ChannelPosition3 *m_pos;
	ChannelIndex16 *m_indices;
	int m_index;

public:
	Triangle() : m_pos(0), m_indices(0), m_index(0) {}
	Triangle(ChannelPosition3 *pos, ChannelIndex16 *idx, int index)
	:	m_pos(pos), m_indices(idx), m_index(index) {}

	int getIndex() const { return m_index; }
	ChannelIndex16 *getIndices() const { return m_indices; }
	float intersect(const mll::geometry::ray3 &r)
	{
		return r.intersect_triangle(getPoint(0),getPoint(1),getPoint(2));
	}

	const point3 &getPoint(int idx) const
	{
		return m_pos->getData()[m_indices->getData()[m_index*3+idx]];
	}
};

//=====================================================================================//
//                                class TriangeFunctor                                 //
//=====================================================================================//
class TriangleFunctor
{
public:
	aabb3 getGrowing(Triangle *tri)
	{
		aabb3 box;
		box.grow(tri->getPoint(0));
		box.grow(tri->getPoint(1));
		box.grow(tri->getPoint(2));
		return box;
	}

	float getDistance(Triangle *tri, const plane &pl)
	{
		float d1 = pl.classify(tri->getPoint(0));
		float d2 = pl.classify(tri->getPoint(1));
		float d3 = pl.classify(tri->getPoint(2));

		if(d1*d2 < 0.0f || d2*d3 < 0.0f || d1*d3 < 0.0f) return 0.0f;
		if(d1 <= 0.0f && d2 <= 0.0f && d3 <= 0.0f) return std::max(std::max(d1,d2),d3);
		return std::min(std::min(d1,d2),d3);
	}
};
}

//=====================================================================================//
//                                class MeshPiece::Impl                                //
//=====================================================================================//
class MeshPiece::Impl
{
public:
	typedef std::vector<Triangle> Triangles_t;
	typedef ClassifiedHashTree<Triangle,TriangleFunctor> Tree_t;

	Triangles_t m_triangles;
	Tree_t m_tree;

	Impl() : m_tree(TriangleFunctor(),4.0f,200) {}
};

//=====================================================================================//
//                               MeshPiece::MeshPiece()                                //
//=====================================================================================//
MeshPiece::MeshPiece(Mesh *mesh, Piece *piece, 
					 AppFront *front, std::list<ChannelBase *> &chnls,
					 HObject userdata)
:	m_mesh(mesh),
	m_piece(piece)
{
	typedef std::list<ChannelBase*>::iterator ChanListItor_t;
	typedef std::pair<ChanListItor_t,ChanListItor_t> Range_t;
	Range_t range = std::equal_range(chnls.begin(),chnls.end(),getPiece(),ChannelsByPiecePred());
	m_channels.insert(m_channels.end(),range.first,range.second);
	chnls.erase(range.first,range.second);

	calcObb();

	if(front)
	{
		m_buffer = front->createPieceBuffer(this,userdata);
	}
}

//=====================================================================================//
//                               MeshPiece::~MeshPiece()                               //
//=====================================================================================//
MeshPiece::~MeshPiece()
{
}

//=====================================================================================//
//                  MeshPiece::iterator MeshPiece::getChannelByName()                  //
//=====================================================================================//
MeshPiece::iterator MeshPiece::getChannelByName(const std::string &name) const
{
	for(iterator i = begin(); i != end(); ++i)
	{
		if((*i)->getName() == name) return i;
	}
	return end();
}

//=====================================================================================//
//                    IntermediateTraceResult MeshPiece::traceRay()                    //
//=====================================================================================//
IntermediateTraceResult MeshPiece::traceRay(const ray3 &r)
{
	if(getPiece()->rtti_dynamic() == Particles::rtti_static())
	{
		IntermediateTraceResult tr;
		tr.m_pos = -1.0f;
		return tr;
	}

	if(!m_pimpl.get()) makeOctTree();

	IntermediateTraceResult tr;
	tr.m_pos = -1.0f;

	Impl::Tree_t::iterator it = m_pimpl->m_tree.traceRay(r);
	while(!it.isDone())
	{
		if((it.distance() >= 0.0f) && (it.distance() < tr.m_pos || tr.m_pos < 0.0f))
		{
			tr.m_pos = it.distance();
			tr.m_polyId = PolyId(it->getIndices(),it->getIndex());
		}
		it.next();
	}

/*	ChannelPosition3 *pos = getChannel(channelIdPos);
	const ChannelPosition3::Data_t &vtx = pos->getData();
	ChannelIndex16 *indices = getChannel(channelIdIndex);
	const ChannelIndex16::Data_t &idx = indices->getData();

	for(unsigned int k = 0; k < indices->getSize(); k += 3)
	{
		float t = r.intersect_triangle(vtx[idx[k]],vtx[idx[k+1]],vtx[idx[k+2]]);
		if((t >= 0.0f) && (t < tr.m_pos || tr.m_pos < 0.0f))
		{
			tr.m_pos = t;
			tr.m_polyId = PolyId(indices,k/3);
		}
	}*/

	return tr;
}

//=====================================================================================//
//                            void MeshPiece::makeOctTree()                            //
//=====================================================================================//
void MeshPiece::makeOctTree()
{
	ChannelPosition3 *pos = getChannel(channelIdPos);
	ChannelIndex16 *indices = getChannel(channelIdIndex);
	
	m_pimpl.reset(new Impl);
	m_pimpl->m_triangles.reserve(indices->getSize()/3);

	for(unsigned int i = 0; i < indices->getSize()/3; ++i)
	{
		m_pimpl->m_triangles.push_back(Triangle(pos,indices,i));
	}

	m_pimpl->m_tree.construct(m_pimpl->m_triangles.begin(),m_pimpl->m_triangles.end());
	HashTreeStat t = m_pimpl->m_tree.getStat();
}

//=====================================================================================//
//                    IntermediateTraceResult MeshPiece::traceRay()                    //
//=====================================================================================//
IntermediateTraceResult MeshPiece::traceRay(const ray3 &r, TraceType tt)
{
	if(getPiece()->rtti_dynamic() == Particles::rtti_static())
	{
		IntermediateTraceResult tr;
		tr.m_pos = -1.0f;
		return tr;
	}
	if(!m_pimpl.get()) makeOctTree();

	IntermediateTraceResult tr;
	tr.m_pos = -1.0f;

	std::string traceType = "No_Such_An_Option";
	switch(tt)
	{
		case ttCursor: traceType = "cursoropaque"; break;
		case ttPhysics: traceType = "physicsopaque"; break;
		case ttVisibility: traceType = "visibilityopaque"; break;
	}

	const OptionSet &matopts = getPiece()->getMaterial()->getOptions();
	if(matopts.optionExists(traceType) && !matopts.getBoolean(traceType))
	{
		return tr;
	}

	Impl::Tree_t::iterator it = m_pimpl->m_tree.traceRay(r);
	if(!it.isDone())
	{
		Triangle *tt = &*it;
		tr.m_pos = it.distance();
		tr.m_polyId = PolyId(tt->getIndices(),tt->getIndex());
	}

/*	ChannelPosition3 *pos = getChannel(channelIdPos);
	const ChannelPosition3::Data_t &vtx = pos->getData();
	ChannelIndex16 *indices = getChannel(channelIdIndex);
	const ChannelIndex16::Data_t &idx = indices->getData();

	for(unsigned int k = 0; k < indices->getSize(); k += 3)
	{
		float t = r.intersect_triangle(vtx[idx[k]],vtx[idx[k+1]],vtx[idx[k+2]]);
		if((t >= 0.0f) && (t < tr.m_pos || tr.m_pos < 0.0f))
		{
			tr.m_pos = t;
			tr.m_polyId = PolyId(indices,k/3);
		}
	}*/

	return tr;
}

void MeshPiece::calcObb()
{
	m_obb.degenerate();
	ChannelPosition3 *pos = getChannel(channelIdPos);
	const ChannelPosition3::Data_t &vtx = pos->getData();

	for(unsigned int i = 0; i != pos->getSize(); ++i)
	{
		m_obb.grow(vtx[i]);
	}
}

}
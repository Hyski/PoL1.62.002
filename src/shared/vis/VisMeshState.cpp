#include "Vis.h"
#include "VisMeshState.h"

namespace Vis
{

ML_RTTI_IMPL(MeshState,MlObject);

//=====================================================================================//
//                               MeshState::MeshState()                                //
//=====================================================================================//
MeshState::MeshState()
:	m_world(matrix3::identity())
{
}

//=====================================================================================//
//                               MeshState::MeshState()                                //
//=====================================================================================//
MeshState::MeshState(HMesh mesh)
:	m_mesh(mesh),
	m_world(matrix3::identity())
{
	if(m_mesh)
	{
		constructTreeFromMesh();
		initMatrices();
		calcObb();
	}
}

//=====================================================================================//
//                               MeshState::MeshState()                                //
//=====================================================================================//
MeshState::MeshState(const MeshState &s)
:	m_mesh(s.m_mesh),
	m_world(s.m_world),
	m_tree(s.m_tree),
	m_obb(s.m_obb)
{
}

//=====================================================================================//
//                       void MeshState::constructTreeFromMesh()                       //
//=====================================================================================//
void MeshState::constructTreeFromMesh()
{
	assert( m_mesh );

	for(Mesh::iterator i = m_mesh->begin(); i != m_mesh->end(); i.brother())
	{
		constructTreeBranch(m_tree.insert_son(m_tree.end(),NodeData(i)));
	}
}

//=====================================================================================//
//                        void MeshState::constructTreeBranch()                        //
//=====================================================================================//
void MeshState::constructTreeBranch(NodeTree_t::iterator parent)
{
	assert( m_mesh );

	for(Mesh::iterator i = parent->m_node.son_begin(); i != parent->m_node.son_end(); i.brother())
	{
		constructTreeBranch(m_tree.insert_son(parent,NodeData(i)));
	}
}

//=====================================================================================//
//                           void MeshState::initMatrices()                            //
//=====================================================================================//
void MeshState::initMatrices()
{
	assert( m_mesh );

	for(NodeTree_t::iterator i = m_tree.begin(); i != m_tree.end(); i.brother())
	{
		i->update(m_world);
		initMatrix(i);
	}
}

//=====================================================================================//
//                            void MeshState::initMatrix()                             //
//=====================================================================================//
void MeshState::initMatrix(NodeTree_t::iterator parent)
{
	for(NodeTree_t::iterator i = parent.son_begin(); i != parent.son_end(); i.brother())
	{
		i->update(parent->getMatrix());
		initMatrix(i);
	}
}

//=====================================================================================//
//                             void MeshState::setWorld()                              //
//=====================================================================================//
void MeshState::setWorld(const matrix3 &world)
{
	m_world = world;
	initMatrices();
	calcObb();
}

//=====================================================================================//
//                   MeshState::NodeHandler MeshState::getHandler()                    //
//=====================================================================================//
MeshState::NodeHandler MeshState::getHandler(const std::string &name)
{
	for(NodeTree_t::iterator i = m_tree.begin(); i != m_tree.end(); ++i)
	{
		if(i->m_node->m_node->getName() == name) return NodeHandler(this,i);
	}
	return NodeHandler(0,NodeTree_t::iterator());
}

//=====================================================================================//
//                          void MeshState::updateFromNode()                           //
//=====================================================================================//
void MeshState::updateFromNode(NodeTree_t::iterator what)
{
	matrix3 base = m_world;
	if(what.has_parent())
	{
		base = what.get_parent()->getMatrix();
	}

	what->update(base);
	initMatrix(what);
}

//=====================================================================================//
//                              void MeshState::calcObb()                              //
//=====================================================================================//
void MeshState::calcObb()
{
	m_obb.degenerate();

	for(iterator i = getNodeTree().begin(); i != getNodeTree().end(); ++i)
	{
		const matrix3 mat = i->getMatrix();

		typedef Mesh::NodeData::MeshPieces_t::const_iterator MPItor_t;
		for(MPItor_t j = i->m_node->m_pieces.begin(); j != i->m_node->m_pieces.end(); ++j)
		{
			m_obb.grow((*j)->getObb() * mat);
		}
	}
}

//=====================================================================================//
//                          float MeshState::traceRay() const                          //
//=====================================================================================//
float MeshState::traceRay(const ray3 &ray) const
{
	float result = -1.0f;
	if (m_obb.intersect(ray)==result) return result;

	for(iterator i = getNodeTree().begin(); i != getNodeTree().end(); ++i)
	{
		const matrix3 inv_mat = matrix3(i->getMatrix()).invert();
		const ray3 local_ray = ray*inv_mat;

		typedef Mesh::NodeData::MeshPieces_t::const_iterator MPItor_t;
		for(MPItor_t j = i->m_node->m_pieces.begin(); j != i->m_node->m_pieces.end(); ++j)
		{
			if ( -1.0f != (*j)->getObb().intersect(local_ray))
			{
				IntermediateTraceResult tr = (*j)->traceRay(local_ray);

				if((tr.m_pos >= 0.0f) && (tr.m_pos < result || result < 0.0f))
				{
					result = tr.m_pos;
				}
			}
		}
	}
	return result;
}

//=====================================================================================//
//                          float MeshState::traceRay() const                          //
//=====================================================================================//
float MeshState::traceRay(const ray3 &ray, TraceType tt) const
{
	float result = -1.0f;
	if (m_obb.intersect(ray)==result) return result;

	for(iterator i = getNodeTree().begin(); i != getNodeTree().end(); ++i)
	{
		const matrix3 inv_mat = matrix3(i->getMatrix()).invert();
		const ray3 local_ray = ray*inv_mat;

		typedef Mesh::NodeData::MeshPieces_t::const_iterator MPItor_t;
		for(MPItor_t j = i->m_node->m_pieces.begin(); j != i->m_node->m_pieces.end(); ++j)
		{
			if ( -1.0f != (*j)->getObb().intersect(local_ray))
			{
				IntermediateTraceResult tr = (*j)->traceRay(local_ray,tt);

				if((tr.m_pos >= 0.0f) && (tr.m_pos < result || result < 0.0f))
				{
					result = tr.m_pos;
				}
			}
		}
	}
	return result;
}

//=====================================================================================//
//                       TraceResult MeshState::traceRay() const                       //
//=====================================================================================//
TraceResult MeshState::traceRayWithGroup(const ray3 &ray) const
{
	float result = -1.0f;
	const MeshPolyGroup *polygrp = 0;
	if (-1.0f!=m_obb.intersect(ray))
	{
		for(iterator i = getNodeTree().begin(); i != getNodeTree().end(); ++i)
		{
			const matrix3 inv_mat = matrix3(i->getMatrix()).invert();
			const ray3 local_ray = ray*inv_mat;

			typedef Mesh::NodeData::MeshPieces_t::const_iterator MPItor_t;
			for(MPItor_t j = i->m_node->m_pieces.begin(); j != i->m_node->m_pieces.end(); ++j)
			{
				if ( -1.0f != (*j)->getObb().intersect(local_ray))
				{
					IntermediateTraceResult tr = (*j)->traceRay(local_ray);

					if((tr.m_pos >= 0.0f) && (tr.m_pos < result || result < 0.0f))
					{
						result = tr.m_pos;

						for(Mesh::PolyGroups_t::const_iterator m = getMesh()->getPolyGroups().begin();
							m != getMesh()->getPolyGroups().end(); ++m)
						{
							if(m->second.getPolyGroup()->isYourPoly(tr.m_polyId))
							{
								polygrp = &m->second;
							}
						}
					}
				}
			}
		}
	}
	TraceResult r = {result,polygrp};
	return r;
}

//=====================================================================================//
//                  TraceResult MeshState::traceRayWithGroup() const                   //
//=====================================================================================//
TraceResult MeshState::traceRayWithGroup(const ray3 &ray, TraceType tt) const
{
	float result = -1.0f;
	const MeshPolyGroup *polygrp = 0;

	if (-1.0f!=m_obb.intersect(ray))
	{
		for(iterator i = getNodeTree().begin(); i != getNodeTree().end(); ++i)
		{
			const matrix3 inv_mat = matrix3(i->getMatrix()).invert();
			const ray3 local_ray = ray*inv_mat;

			typedef Mesh::NodeData::MeshPieces_t::const_iterator MPItor_t;
			for(MPItor_t j = i->m_node->m_pieces.begin(); j != i->m_node->m_pieces.end(); ++j)
			{
				if ( -1.0f != (*j)->getObb().intersect(local_ray))
				{
					IntermediateTraceResult tr = (*j)->traceRay(local_ray,tt);

					if((tr.m_pos >= 0.0f) && (tr.m_pos < result || result < 0.0f))
					{
						result = tr.m_pos;

						for(Mesh::PolyGroups_t::const_iterator m = getMesh()->getPolyGroups().begin();
							m != getMesh()->getPolyGroups().end(); ++m)
						{
							if(m->second.getPolyGroup()->isYourPoly(tr.m_polyId))
							{
								polygrp = &m->second;
							}
						}
					}
				}
			}
		}
	}
	TraceResult r = {result,polygrp};
	return r;
}

//=====================================================================================//
//                            MeshState &MeshState::swap()                             //
//=====================================================================================//
MeshState &MeshState::swap(MeshState &state)
{
	m_mesh.swap(state.m_mesh);
	m_tree.swap(state.m_tree);
	std::swap(m_world,state.m_world);
	std::swap(m_obb,state.m_obb);

	return *this;
}

//=====================================================================================//
//                         MeshState &MeshState::operator =()                          //
//=====================================================================================//
MeshState &MeshState::operator =(const MeshState &state)
{
	return swap(MeshState(state));
}

}
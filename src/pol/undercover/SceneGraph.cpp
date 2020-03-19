#include "precomp.h"
#include "SceneGraph.h"

namespace PoL
{

static SceneGraph g_sceneGraph(2.0f);

//=====================================================================================//
//                              static int getTreeDepth()                              //
//=====================================================================================//
static int getTreeDepth(float ss, float ms)
{
	return floorf(logf(ss/ms)/logf(2.0f));
}

//=====================================================================================//
//                         static unsigned int getNodeCount()                          //
//=====================================================================================//
static unsigned int getNodeCount(int depth)
{
	return depth ? getNodeCount(depth-1)*8 + 1 : 1;
}

//=====================================================================================//
//                              SceneGraph::SceneGraph()                               //
//=====================================================================================//
SceneGraph::SceneGraph(float minsize)
:	m_minNodeSize(minsize),
	m_depth(0)
	//m_depth(floorf(logf(scenesize/minsize)/logf(2.0f)))
	//m_nodes(getNodeCount(m_depth))
{
	aabb3 t;
	t.grow(mll::algebra::point3(0.0f,0.0f,-20.0f));
	t.grow(mll::algebra::point3(155.0f,155.0f,40.0f));
	reset(t);
}

//=====================================================================================//
//                              void SceneGraph::reset()                               //
//=====================================================================================//
void SceneGraph::reset(const aabb3 &bound)
{
	float size = std::max(std::max(bound.diag().x,bound.diag().y),bound.diag().z);
	m_depth = getTreeDepth(size,m_minNodeSize);
	int nodecount = getNodeCount(m_depth);
}

}

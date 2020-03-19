#include "precomp.h"

#include "oct_tree_idx.h"
#include "IntersectionCache.h"

namespace PoL
{

//=====================================================================================//
//                               static void construct()                               //
//=====================================================================================//
static void construct(oct_tree::node *node, oct_tree *tree, const char *triDesc,
					  IntersectionNode &inode,
					  std::vector<IntersectionNode> &nodes,
					  std::vector<IntersectionTriangle> &triangles)
{
	inode.m_triangleCount = 0;
	inode.m_firstTriangle = 0;
	inode.m_childrenMask = 0;

	if(node->has_children())
	{
		for(int i = 0; i < 8; ++i)
		{
			if(node->m_child[i].get())
			{
				inode.m_childrenMask |= (1 << i);
			}
		}
	}

	if(inode.getChildrenCount())
	{
		inode.m_children = &*nodes.end();
		nodes.insert(nodes.end(),inode.getChildrenCount(),IntersectionNode());

		for(int i = 0; i < inode.getChildrenCount(); ++i)
		{
			unsigned char pidx = inode.getIndexFromChild(i);

			assert( node->m_child[pidx].get() != 0 );

			float x = pidx&1 ? inode.m_box.min().x : inode.m_box.max().x;
			float y = pidx&2 ? inode.m_box.min().y : inode.m_box.max().y;
			float z = pidx&4 ? inode.m_box.min().z : inode.m_box.max().z;

			const mll::geometry::aabb3 cbox(inode.m_box.center(),mll::algebra::point3(x,y,z));

			inode.m_children[i].m_parentIndex = pidx;
			inode.m_children[i].m_box = cbox;

			construct(node->m_child[pidx].get(),tree,triDesc,inode.m_children[i],nodes,triangles);
		}
	}
	else
	{
		inode.m_children = 0;

		inode.m_triangleCount = node->m_tris.size();
		inode.m_firstTriangle = triangles.size();
		triangles.resize(triangles.size() + inode.m_triangleCount);

		for(int i = 0; i < inode.m_triangleCount; ++i)
		{
			IntersectionTriangle tri;
			tri.m_v1 = tree->get_storage().m_verts[node->m_tris[i]->v[0]].pos;
			tri.m_edge1 = tree->get_storage().m_verts[node->m_tris[i]->v[1]].pos - tri.m_v1;
			tri.m_edge2 = tree->get_storage().m_verts[node->m_tris[i]->v[2]].pos - tri.m_v1;
			tri.m_transMask = triDesc[node->m_tris[i] - &tree->get_triangles()[0]];
			triangles[inode.m_firstTriangle + i] = tri;
		}
	}
}

//=====================================================================================//
//                               static void construct()                               //
//=====================================================================================//
static void construct(oct_tree *tree, const char *triDesc,
					  std::vector<IntersectionNode> &nodes,
					  std::vector<IntersectionTriangle> &triangles)
{
	IntersectionNode inode;
	inode.m_parentIndex = 0;
	inode.m_box = tree->get_root()->bound();
	nodes.push_back(inode);

	construct(tree->get_root(),tree,triDesc,nodes.back(),nodes,triangles);
}

//=====================================================================================//
//                       IntersectionCache::IntersectionCache()                        //
//=====================================================================================//
IntersectionCache::IntersectionCache(oct_tree *t, const char *triDesc)
{
	m_nodes.reserve(t->NodeCount());
	construct(t,triDesc,m_nodes,m_trias);
}

}
#pragma once

namespace PoL
{

//=====================================================================================//
//                                class SceneGraphNode                                 //
//=====================================================================================//
struct SceneGraphNode
{
	typedef mll::geometry::aabb3 aabb3;
	typedef std::vector<long> Objects_t;

	aabb3 m_rawBbox, m_bbox;
	Objects_t m_objects;

	enum ChildIndex
	{
		ciPosX		= 1<<0,
		ciNegX		= 0<<0,
		ciPosY		= 1<<1,
		ciNegY		= 0<<1,
		ciPosZ		= 1<<2,
		ciNegZ		= 0<<2
	};

	SceneGraphNode *m_children;		///< ”казатель на 8 детей
};

//=====================================================================================//
//                                  class SceneGraph                                   //
//=====================================================================================//
class SceneGraph
{
	typedef mll::geometry::aabb3 aabb3;

	typedef SceneGraphNode Node;
	typedef std::vector<Node> Nodes_t;

	float m_minNodeSize;
	int m_depth;
	Nodes_t m_nodes;

public:
	SceneGraph(float minsize);

	void reset(const aabb3 &);
};

}

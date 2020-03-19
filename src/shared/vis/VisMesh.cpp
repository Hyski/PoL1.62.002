#include "Vis.h"
#include "VisMesh.h"
#include "VisMeshMgr.h"
#include "VisAppFront.h"

namespace Vis
{

ML_RTTI_IMPL(Mesh,MlResource);

//=====================================================================================//
//                            struct FindNodeDataByNodePtr                             //
//=====================================================================================//
struct FindNodeDataByNodePtr
{
	Node *m_nodePtr;
	FindNodeDataByNodePtr(Node *ptr) : m_nodePtr(ptr) {}
	bool operator()(const Mesh::NodeData &data) const
	{
		return data.m_node == m_nodePtr;
	}
};

//=====================================================================================//
//                                    Mesh::Mesh()                                     //
//=====================================================================================//
Mesh::Mesh(const std::string &name, AppFront *front, HObject userdata)
:	MlResource(name),
	m_storage("MASCEV04")
{
	const std::string real_name = "/meshes/" + name + ".masce";
	m_storage.restore(mll::io::ibinstream(front->openFile(real_name)->stream()));
	initGroups();
	buildTree();
	initPieces(front,userdata);
}

//=====================================================================================//
//                                    Mesh::Mesh()                                     //
//=====================================================================================//
Mesh::Mesh(const std::string &name, std::istream &in, AppFront *front, HObject userdata)
:	MlResource(name),
	m_storage("MASCEV04")
{
	m_storage.restore(mll::io::ibinstream(in));
	initGroups();
	buildTree();
	initPieces(front,userdata);
}

//=====================================================================================//
//                                    Mesh::~Mesh()                                    //
//=====================================================================================//
Mesh::~Mesh()
{
	if(MeshMgr::instance()) MeshMgr::instance()->removeMesh(name());
	for(NodeTree_t::iterator i = m_tree.begin(); i != m_tree.end(); ++i)
	{
		for(unsigned int j = 0; j != i->m_pieces.size(); ++j)
		{
			delete i->m_pieces[j];
			i->m_pieces[j] = 0;
		}
	}
}

//=====================================================================================//
//                               void Mesh::buildTree()                                //
//=====================================================================================//
void Mesh::buildTree()
{
	typedef std::list<Node *> Nodes_t;

	Nodes_t nodes;
	std::copy(m_storage.begin<Node>(),m_storage.end<Node>(),std::back_inserter(nodes));

	for(Nodes_t::iterator i = nodes.begin(); !nodes.empty(); )
	{
		if((*i)->hasParent())
		{
			NodeTree_t::iterator where = std::find_if(m_tree.begin(),m_tree.end(),
															FindNodeDataByNodePtr((*i)->getParent()));
			if(where == m_tree.end())
			{
				++i;
			}
			else
			{
				m_tree.insert_son(where,NodeData(*i));
				i = nodes.erase(i);
			}
		}
		else
		{
			m_tree.insert_son(m_tree.end(),NodeData(*i));
			i = nodes.erase(i);
		}

		if(i == nodes.end()) i = nodes.begin();
	}
}

//=====================================================================================//
//                               void Mesh::initPieces()                               //
//=====================================================================================//
void Mesh::initPieces(AppFront *front, HObject userdata)
{
	typedef std::list<Piece *> Pieces_t;
	typedef std::list<ChannelBase *> Channels_t;

	Pieces_t pieces;
	Channels_t channels;

	std::copy(m_storage.begin<Piece>(),m_storage.end<Piece>(),std::back_inserter(pieces));
	std::copy(m_storage.begin<ChannelBase>(),m_storage.end<ChannelBase>(),std::back_inserter(channels));
	pieces.sort(PiecesByNodePred());
	channels.sort(ChannelsByPiecePred());

	for(NodeTree_t::iterator i = m_tree.begin(); i != m_tree.end(); ++i)
	{
		typedef std::pair<Pieces_t::iterator,Pieces_t::iterator> Range_t;
		Range_t range = std::equal_range(pieces.begin(),pieces.end(),i->m_node,PiecesByNodePred());
		i->m_pieces.reserve(std::distance(range.first,range.second));

		for(Pieces_t::iterator j = range.first; j != range.second; ++j)
		{
			typedef std::list<ChannelBase*>::iterator ChanListItor_t;
			typedef std::pair<ChanListItor_t,ChanListItor_t> CRange_t;
			CRange_t range = std::equal_range(channels.begin(),channels.end(),*j,ChannelsByPiecePred());
			if((*range.first)->getSize())
			{
				i->m_pieces.push_back(new MeshPiece(this,*j,front,channels,userdata));
			}
		}

		pieces.erase(range.first,range.second);
	}
}

//=====================================================================================//
//                               void Mesh::initGroups()                               //
//=====================================================================================//
void Mesh::initGroups()
{
	typedef std::list<PolyGroup *> PolyGroups_t;

	PolyGroups_t pgs;
	std::copy(m_storage.begin<PolyGroup>(),m_storage.end<PolyGroup>(),std::back_inserter(pgs));

	for(PolyGroups_t::iterator i = pgs.begin(); i != pgs.end(); ++i)
	{
		m_polyGroups.insert(std::make_pair((*i)->getName(),*i));
	}
}

}
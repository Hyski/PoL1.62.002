#if !defined(__VIS_PIECE_H_INCLUDED_7507999770485014__)
#define __VIS_PIECE_H_INCLUDED_7507999770485014__

#include "VisOptionSet.h"

//=====================================================================================//
//                                    namespace Vis                                    //
//=====================================================================================//
namespace Vis
{

class Node;
class Material;

#pragma warning(push)
#pragma warning(disable: 4251 4275)

//=====================================================================================//
//                                     class Piece                                     //
//=====================================================================================//
class VIS_IMPORT Piece : public Storage::PersistentBase
{
	std::string m_name;
	Storage::Ptr<Node> m_node;
	Storage::Ptr<Material> m_material;

	OptionSet m_options;

public:
	ML_RTTI_CUSTOM_ROOT(Piece,"[Vis.Piece]");

	Piece();
	Piece(const Piece &);
	Piece(Storage::ReadSlot &);
	Piece(const std::string &, Node *, const OptionSet & = OptionSet());
	virtual ~Piece();

	void setName(const std::string &name) { m_name = name; }
	void setMaterial(Material *mat) { m_material = mat; }

	Node *getNode() const { return m_node.get(); }
	const std::string &getName() const { return m_name; }
	const OptionSet &getOptions() const { return m_options; }
	Material *getMaterial() const { return m_material.get(); }

	virtual bool isIdentical(const Piece *) const = 0;
	virtual Piece *clone(Node *newParent, Storage::Repository *) const = 0;

	void store(Storage::WriteSlot &) const;
};

//=====================================================================================//
//                           static bool piecesByNodePred()                            //
//=====================================================================================//
struct PiecesByNodePred
{
	bool operator()(const Piece *l, const Piece *r) const
	{
		return l->getNode() < r->getNode();
	}
	bool operator()(const Node *l, const Piece *r) const
	{
		return l < r->getNode();
	}
	bool operator()(const Piece *l, const Node *r) const
	{
		return l->getNode() < r;
	}
};

#pragma warning(pop)

}

#endif // !defined(__VIS_PIECE_H_INCLUDED_7507999770485014__)
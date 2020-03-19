#include "Vis.h"

namespace Vis
{

//=====================================================================================//
//                                   Piece::Piece()                                    //
//=====================================================================================//
Piece::Piece()
:	m_name("**wrong_piece_name**"),
	m_node(0),
	m_material(0)
{
}

//=====================================================================================//
//                                   Piece::Piece()                                    //
//=====================================================================================//
Piece::Piece(const Piece &p)
:	m_name(p.getName()),
	m_node(p.getNode()),
	m_options(p.getOptions()),
	m_material(p.getMaterial())
{
}

//=====================================================================================//
//                                   Piece::Piece()                                    //
//=====================================================================================//
Piece::Piece(Storage::ReadSlot &slot)
:	Storage::PersistentBase(slot)
{
	slot.getStream() >> m_name;
	m_node = slot.restoreWeakReference<Node>();
	m_material = slot.restoreWeakReference<Material>();
	m_options.restore(slot.getStream());
}

//=====================================================================================//
//                                   Piece::Piece()                                    //
//=====================================================================================//
Piece::Piece(const std::string &name, Node *node, const OptionSet &options)
:	m_name(name),
	m_node(node),
	m_options(options),
	m_material(0)
{
}

//=====================================================================================//
//                                   Piece::~Piece()                                   //
//=====================================================================================//
Piece::~Piece()
{
}

//=====================================================================================//
//                                 void Piece::store()                                 //
//=====================================================================================//
void Piece::store(Storage::WriteSlot &slot) const
{
	slot.getStream() << m_name;
	slot.storeReference(m_node);
	slot.storeReference(m_material);
	m_options.store(slot.getStream());
}

}
#include "Vis.h"
#include "VisMeshPiece.h"
#include "VisMeshPolyGroup.h"

namespace Vis
{

//=====================================================================================//
//                           MeshPolyGroup::MeshPolyGroup()                            //
//=====================================================================================//
MeshPolyGroup::MeshPolyGroup()
:	m_polyGroup(0)
{
}

//=====================================================================================//
//                           MeshPolyGroup::MeshPolyGroup()                            //
//=====================================================================================//
MeshPolyGroup::MeshPolyGroup(PolyGroup *p)
:	m_polyGroup(p)
{
}

//=====================================================================================//
//            PolyGroup::PolyRange_t MeshPolyGroup::getPolysOfPiece() const            //
//=====================================================================================//
PolyGroup::PolyRange_t MeshPolyGroup::getPolyIdsOfPiece(MeshPiece *piece) const
{
	return getPolyGroup()->getPolyIdsOfPiece(piece->getChannel(channelIdIndex));
}

}
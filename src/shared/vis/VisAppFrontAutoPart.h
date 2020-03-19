#pragma once

#include "Vis.h"
#include "VisAppFront.h"

//=====================================================================================//
//                                    namespace Vis                                    //
//=====================================================================================//
namespace Vis
{

#pragma warning(push)
#pragma warning(disable: 4275)

//=====================================================================================//
//                      class AppFrontAutoPart : public AppFront                       //
//=====================================================================================//
class VIS_IMPORT AppFrontAutoPart : public AppFront
{
public:
	std::auto_ptr<PieceBuffer> createPieceBuffer(MeshPiece *, HObject userdata);
};

#pragma warning(pop)

}

#include "Vis.h"
#include "VisMeshPiece.h"
#include "VisMeshShapeBuffer.h"
#include "VisMeshParticlesBuffer.h"
#include "VisAppFrontAutoPart.h"

namespace Vis
{

//=====================================================================================//
//          std::auto_ptr<PieceBuffer> AppFrontAutoPart::createPieceBuffer()           //
//=====================================================================================//
std::auto_ptr<PieceBuffer> AppFrontAutoPart::createPieceBuffer(MeshPiece *piece,
															   HObject userdata)
{
	std::auto_ptr<PieceBuffer> result;

	if(piece->getPiece()->rtti_dynamic() == Shape::rtti_static())
	{
		result.reset(new MeshShapeBuffer(piece,userdata.get()));
	}
	else if(piece->getPiece()->rtti_dynamic() == Particles::rtti_static())
	{
		result.reset(new MeshParticlesBuffer(piece,userdata.get()));
	}

	return result;
}

}
#include "Vis.h"
#include "VisWaterStrategy.h"

namespace Vis
{

//=====================================================================================//
//                           void WaterStrategy::fillRange()                           //
//=====================================================================================//
void WaterStrategy::fillRange(MeshPiece *piece)
{
	Range_t::LockRange_t lock = getMyRange()->lock();
	getMyRange()->unlock();
}

}
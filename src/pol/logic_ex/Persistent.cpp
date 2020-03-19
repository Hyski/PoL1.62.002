#include "precomp.h"
#include "ActFactory.h"
#include "Persistent.h"

namespace PoL
{

//=====================================================================================//
//                   const std::string &Persistent::getName() const                    //
//=====================================================================================//
const std::string &Persistent::getName() const
{
	return ActFactory::instance()->getNameFromId(getId());
}

}
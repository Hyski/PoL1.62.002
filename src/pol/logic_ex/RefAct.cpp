#include "precomp.h"

#include "RefAct.h"
#include "ActFactory.h"

namespace PoL
{

//=====================================================================================//
//                                  RefAct::RefAct()                                   //
//=====================================================================================//
RefAct::RefAct(const std::string &act)
:	m_act(ActFactory::instance()->create(act))
{
}

}
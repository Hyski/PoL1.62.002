#include "precomp.h"
#include "Synchronized2.h"

namespace Muffle2
{

static volatile long g_objectCount = 0;

//=====================================================================================//
//                           Synchronized2::Synchronized2()                            //
//=====================================================================================//
Synchronized::Synchronized()
:	m_weakToken(new WeakToken(this))
{
	InterlockedIncrement(&g_objectCount);
}

//=====================================================================================//
//                            Synchronized::~Synchronized()                            //
//=====================================================================================//
Synchronized::~Synchronized()
{
	InterlockedDecrement(&g_objectCount);
}

//=====================================================================================//
//                      long Synchronized::getSynchronizedCount()                      //
//=====================================================================================//
long Synchronized::getInstanceCount()
{
	return g_objectCount;
}

}

#include "precomp.h"
#include "Decoder2.h"

namespace Muffle2
{

static volatile long g_instanceCount = 0;

//=====================================================================================//
//                                 Decoder::Decoder()                                  //
//=====================================================================================//
Decoder::Decoder()
{
	InterlockedIncrement(&g_instanceCount);
}

//=====================================================================================//
//                                 Decoder::~Decoder()                                 //
//=====================================================================================//
Decoder::~Decoder()
{
	InterlockedDecrement(&g_instanceCount);
}

//=====================================================================================//
//                          long Decoder::getInstanceCount()                           //
//=====================================================================================//
long Decoder::getInstanceCount()
{
	return g_instanceCount;
}

}


#include "precomp.h"
#include "BufferCtrl2.h"

namespace Muffle2
{

static volatile long g_instanceCount = 0;

//=====================================================================================//
//                              BufferCtrl::BufferCtrl()                               //
//=====================================================================================//
BufferCtrl::BufferCtrl()
{
	InterlockedIncrement(&g_instanceCount);
}

//=====================================================================================//
//                              BufferCtrl::~BufferCtrl()                              //
//=====================================================================================//
BufferCtrl::~BufferCtrl()
{
	InterlockedDecrement(&g_instanceCount);
}

//=====================================================================================//
//                         long BufferCtrl::getInstanceCount()                         //
//=====================================================================================//
long BufferCtrl::getInstanceCount()
{
	return g_instanceCount;
}

}

#include "precomp.h"
#include "Locker2.h"
#include "SoundBuffer2.h"

namespace Muffle2
{

//=====================================================================================//
//                                  Locker::~Locker()                                  //
//=====================================================================================//
Locker::~Locker()
{
	if(buffer)
	{
		buffer->unlockBuffer(*this);
	}
}

}

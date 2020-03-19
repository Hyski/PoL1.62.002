#include "precomp.h"
#include "MuffleImpl2.h"

namespace Muffle
{

//=====================================================================================//
//                             ISound *ISound::instance()                              //
//=====================================================================================//
ISound *ISound::instance()
{
	assert( Muffle2::MuffleImpl::inst() != 0 );
	return Muffle2::MuffleImpl::inst();
}

//=====================================================================================//
//                              static void killMuffle()                               //
//=====================================================================================//
static void killMuffle()
{
	if(ISound::isInitialized())
	{
		ISound::shutInstance();
	}
}

//=====================================================================================//
//                             void ISound::initInstance()                             //
//=====================================================================================//
void ISound::initInstance(SndServices *ss)
{
	new Muffle2::MuffleImpl(ss);
	atexit(killMuffle);
}

//=====================================================================================//
//                             void ISound::shutInstance()                             //
//=====================================================================================//
void ISound::shutInstance()
{
	delete Muffle2::MuffleImpl::inst();
}

//=====================================================================================//
//                            bool ISound::isInitialized()                             //
//=====================================================================================//
bool ISound::isInitialized()
{
	return Muffle2::MuffleImpl::inst() != 0;
}

}
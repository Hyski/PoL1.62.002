#include "precomp.h"
#include "Emitter2.h"
#include <boost/bind.hpp>

namespace Muffle2
{

static volatile long g_instanceCount = 0;

//=====================================================================================//
//                                static void forEach()                                //
//=====================================================================================//
template<typename C, typename Op>
static void forEach(C c, Op op)
{
	typedef C::iterator It_t;

	for(It_t i = c.begin(); i != c.end(); ++i)
	{
		if(HPlayingSound locked = i->sound.acquire())
		{
			PlayingSound *snd = locked.get();
			op(snd);
		}
	}
}

//=====================================================================================//
//                                 Emitter::Emitter()                                  //
//=====================================================================================//
Emitter::Emitter(Muffle::ISound::Channel chan)
:	m_uniqueCtr(1),
	m_channel(chan)
{
	InterlockedIncrement(&g_instanceCount);
	m_info.ctrl = BufferCtrlMgr::inst()->createBufferCtrl(BufferCtrlMgr::inst()->getBufferCtrl(m_channel));
}

//=====================================================================================//
//                                 Emitter::~Emitter()                                 //
//=====================================================================================//
Emitter::~Emitter()
{
	InterlockedDecrement(&g_instanceCount);
}

//=====================================================================================//
//                                void Emitter::stop()                                 //
//=====================================================================================//
void Emitter::stop()
{
	MUFFLE_SYNCHRONIZED_THIS;

	forEach(m_sounds, boost::bind(&PlayingSound::stop,_1));
}

//=====================================================================================//
//                                 int Emitter::play()                                 //
//=====================================================================================//
int Emitter::play(HScript script, const std::string &sound, size_t startpos)
{
	MUFFLE_SYNCHRONIZED_THIS;

	if(HPlayingSound snd = PlayingSound::createPlayingSound(script,sound,startpos,m_info))
	{
		SoundInfo sinfo = {m_uniqueCtr++, snd};
		m_sounds.push_back(sinfo);
		return sinfo.uniqueId;
	}
	else
	{
		return 0;
	}
}

//=====================================================================================//
//                              bool Emitter::isPlaying()                              //
//=====================================================================================//
bool Emitter::isPlaying()
{
	MUFFLE_SYNCHRONIZED_THIS;

	for(Sounds_t::iterator i = m_sounds.begin(); i != m_sounds.end(); ++i)
	{
		if(HPlayingSound lockedSnd = i->sound.acquire())
		{
			if(lockedSnd->isPlaying()) return true;
		}
	}

	return false;
}

//=====================================================================================//
//                              void Emitter::setVolume()                              //
//=====================================================================================//
void Emitter::setVolume(float vol)
{
	MUFFLE_SYNCHRONIZED_THIS;

	m_info.ctrl->setVolume(vol);
}

//=====================================================================================//
//                             void Emitter::setPosition()                             //
//=====================================================================================//
void Emitter::setPosition(const vec &p)
{
	MUFFLE_SYNCHRONIZED_THIS;

	if(p != m_info.position)
	{
		m_info.position = p;
		forEach(m_sounds, boost::bind(PlayingSound::setPosition,_1,p));
	}
}

//=====================================================================================//
//                             void Emitter::setVelocity()                             //
//=====================================================================================//
void Emitter::setVelocity(const vec &p)
{
	MUFFLE_SYNCHRONIZED_THIS;

	if(p != m_info.velocity)
	{
		m_info.velocity = p;
		forEach(m_sounds, boost::bind(PlayingSound::setVelocity,_1,p));
	}
}

//=====================================================================================//
//                         void Emitter::setConeOrientation()                          //
//=====================================================================================//
void Emitter::setConeOrientation(const vec &p)
{
	MUFFLE_SYNCHRONIZED_THIS;

	if(p != m_info.coneOrientation)
	{
		m_info.coneOrientation = p;
		forEach(m_sounds, boost::bind(PlayingSound::setConeOrientation,_1,p));
	}
}

//=====================================================================================//
//                          long Emitter::getInstanceCount()                           //
//=====================================================================================//
long Emitter::getInstanceCount()
{
	return g_instanceCount;
}

}

#include "precomp.h"
#include "Fader2.h"
#include "Worker2.h"
#include "PlayingSound2.h"

namespace Muffle2
{

//=====================================================================================//
//                                   Fader::Fader()                                    //
//=====================================================================================//
Fader::Fader(WHPlayingSound snd, long time, bool out)
:	m_psound(snd),
	m_startTime(timeGetTime()),
	m_time(time),
	m_out(out),
	m_lastVolume(m_out ? 1.0f : 0.0f),
	m_volFactor(1.0f)
{
	if(HPlayingSound lockedSnd = m_psound.acquire())
	{
		lockedSnd->setFader(this);
		if(m_out)
		{
			m_volFactor = lockedSnd->getVolume();
		}
	}
}

//=====================================================================================//
//                                   Fader::~Fader()                                   //
//=====================================================================================//
Fader::~Fader()
{
	if(HPlayingSound lockedSnd = m_psound.acquire())
	{
		if(m_out)
		{
			lockedSnd->badStop();
		}
		else
		{
			lockedSnd->setVolume(1.0f);
		}
	}
}

//=====================================================================================//
//                             bool Fader::expired() const                             //
//=====================================================================================//
bool Fader::expired() const
{
	bool result = true;

	if(HPlayingSound lockedSnd = m_psound.acquire())
	{
		if(lockedSnd->getFader() == this)
		{
			result = (m_lastVolume == (m_out ? 0.0f : 1.0f));
		}
	}

	return result;
}

//=====================================================================================//
//                                void Fader::execute()                                //
//=====================================================================================//
void Fader::execute()
{
	MUFFLE_SYNCHRONIZED_THIS;

	if(HPlayingSound lockedSnd = m_psound.acquire())
	{
		if(lockedSnd->getFader() == this)
		{
			long curtime = timeGetTime();
			long dt = curtime - m_startTime;

			float vol = static_cast<float>(dt) / static_cast<float>(m_time);
			if(m_out) vol = 1.0f - vol;

			vol = std::min(1.0f,vol);
			vol = std::max(0.0f,vol);
			m_lastVolume = vol;

			lockedSnd->setVolume(vol * m_volFactor);
		}
	}
}

//=====================================================================================//
//                             void Fader::createFadeOut()                             //
//=====================================================================================//
void Fader::createFadeOut(WHPlayingSound snd, long time)
{
	HFader fader = HFader::create(snd,time,true);
	Worker::inst()->addWork(fader,20,true);
}

//=====================================================================================//
//                             void Fader::createFadeIn()                              //
//=====================================================================================//
void Fader::createFadeIn(WHPlayingSound snd, long time)
{
	HFader fader = HFader::create(snd,time,false);
	Worker::inst()->addWork(fader,20,true);
}

}


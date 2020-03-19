#include "precomp.h"
#include "Fader2.h"
#include "Worker2.h"
#include "Settings2.h"
#include "DecoderMgr2.h"
#include "ThemeManager2.h"

namespace Muffle2
{

static ThemeManager *g_inst = 0;

//=====================================================================================//
//                            ThemeManager::ThemeManager()                             //
//=====================================================================================//
ThemeManager::ThemeManager()
:	m_idCounter(0),
	m_lastScript(0),
	m_lastMuted(true),
	m_lastStartTime(0.0f)
{
	assert( g_inst == 0 );
	g_inst = this;
}

//=====================================================================================//
//                            ThemeManager::~ThemeManager()                            //
//=====================================================================================//
ThemeManager::~ThemeManager()
{
	assert( g_inst == this );
	g_inst = 0;
}

//=====================================================================================//
//                     HThemeSession ThemeManager::createSession()                     //
//=====================================================================================//
HThemeSession ThemeManager::createSession()
{
	MUFFLE_SYNCHRONIZED_THIS;

	const int id = m_idCounter++;
	HThemeSession sess = HThemeSession::create(id);
	SessInfo info = {id,sess};

	m_sessions.push_back(info);
	update();

	return sess;
}

//=====================================================================================//
//                         void ThemeManager::removeSession()                          //
//=====================================================================================//
void ThemeManager::removeSession(int id)
{
	MUFFLE_SYNCHRONIZED_THIS;

	for(Sessions_t::iterator i = m_sessions.begin(); i != m_sessions.end(); ++i)
	{
		if(i->id == id)
		{
			m_sessions.erase(i);
			update();
			break;
		}
	}
}

//=====================================================================================//
//                             void ThemeManager::update()                             //
//=====================================================================================//
void ThemeManager::update()
{
	MUFFLE_SYNCHRONIZED_THIS;

	HScript script = 0;
	std::string sound;
	bool muted = true;
	float startTime = 0.0f;

	if(!m_sessions.empty())
	{
		if(HThemeSession lockedSess = m_sessions.back().sess.acquire())
		{
			script = lockedSess->getScript();
			sound = lockedSess->getSound();
			muted = lockedSess->isMuted();
			startTime = lockedSess->getStartTime();
		}
	}

	if(m_lastScript != script || m_lastSound != sound || m_lastMuted != muted)
	{
		startFadeOut(m_currentTheme);
		m_lastScript = script;
		m_lastSound = sound;
		m_lastMuted = muted;
		m_lastStartTime = startTime;
		m_currentTheme = startFadeIn();
	}
}

//=====================================================================================//
//                          void ThemeManager::startFadeOut()                          //
//=====================================================================================//
void ThemeManager::startFadeOut(WHPlayingSound snd)
{
	if(HPlayingSound lockedSnd = snd.acquire())
	{
		lockedSnd->stop();
	}
}

//=====================================================================================//
//                     WHPlayingSound ThemeManager::startFadeIn()                      //
//=====================================================================================//
WHPlayingSound ThemeManager::startFadeIn()
{
	HPlayingSound result;

	if(m_lastScript && !m_lastSound.empty() && !m_lastMuted)
	{
		EmitterInfo info;
		info.ctrl = BufferCtrlMgr::inst()->getBufferCtrl(Muffle::ISound::cThemes);
		HDecoder decoder = DecoderMgr::createDecoder(m_lastSound);
		if(decoder)
		{
			size_t start = static_cast<size_t>(m_lastStartTime * decoder->getFormat().rate);
			start %= decoder->getSampleCount();
			result = PlayingSound::createPlayingSound(m_lastScript,m_lastSound,start,info,true);
		}
	}

	return result;
}

//=====================================================================================//
//                         ThemeManager *ThemeManager::inst()                          //
//=====================================================================================//
ThemeManager *ThemeManager::inst()
{
	return g_inst;
}

}

#include "precomp.h"
#include "ThemeSession2.h"
#include "ThemeManager2.h"

namespace Muffle2
{

//=====================================================================================//
//                            ThemeSession::ThemeSession()                             //
//=====================================================================================//
ThemeSession::ThemeSession(int id)
:	m_id(id),
	m_script(0),
	m_muted(false),
	m_startTime(0.0f),
	m_paused(false),
	m_startPlaybackTime(0)
{
}

//=====================================================================================//
//                            ThemeSession::~ThemeSession()                            //
//=====================================================================================//
ThemeSession::~ThemeSession()
{
	ThemeManager::inst()->removeSession(m_id);
}

//=====================================================================================//
//                          void ThemeSession::changeTheme()                           //
//=====================================================================================//
void ThemeSession::changeTheme(HScript script, const std::string &theme_)
{
	bool needupdate = false;
	{
		MUFFLE_SYNCHRONIZED_THIS;

		std::string theme = theme_;
		if(theme.empty()) theme = script->getInfo().sound;

		needupdate = (m_script != script || m_sound != theme);

		if(needupdate)
		{
			m_script = script;
			m_sound = theme;
			m_startTime = 0.0f;
		}
	}

	if(needupdate)
	{
		m_startPlaybackTime = timeGetTime();
		ThemeManager::inst()->update();
	}
}

//=====================================================================================//
//                             void ThemeSession::pause()                              //
//=====================================================================================//
void ThemeSession::pause()
{
	bool needupdate = false;

	{
		MUFFLE_SYNCHRONIZED_THIS;

		needupdate = !m_paused;
		if(needupdate)
		{
			m_paused = true;

			if(m_script && !m_sound.empty())
			{
				long deltat = timeGetTime() - m_startPlaybackTime;
				m_startTime = static_cast<float>(deltat) * 1e-3f;
			}
		}
	}

	if(needupdate)
	{
		ThemeManager::inst()->update();
	}
}

//=====================================================================================//
//                             void ThemeSession::resume()                             //
//=====================================================================================//
void ThemeSession::resume()
{
	bool needupdate = false;

	{
		MUFFLE_SYNCHRONIZED_THIS;

		needupdate = m_paused;
		if(needupdate)
		{
			m_paused = false;
		}
	}

	if(needupdate)
	{
		ThemeManager::inst()->update();
	}
}

//=====================================================================================//
//                              void ThemeSession::mute()                              //
//=====================================================================================//
void ThemeSession::mute(bool m)
{
	bool needupdate = false;

	{
		MUFFLE_SYNCHRONIZED_THIS;
		needupdate = (m_muted != m);

		if(needupdate)
		{
			m_muted = m;
			m_startTime = 0.0f;
		}
	}

	ThemeManager::inst()->update();
}

//=====================================================================================//
//                       HScript ThemeSession::getScript() const                       //
//=====================================================================================//
HScript ThemeSession::getScript() const
{
	MUFFLE_SYNCHRONIZED_THIS;

	return m_script;
}

//=====================================================================================//
//                  const std::string &ThemeSession::getSound() const                  //
//=====================================================================================//
const std::string &ThemeSession::getSound() const
{
	MUFFLE_SYNCHRONIZED_THIS;

	return m_sound;
}


}

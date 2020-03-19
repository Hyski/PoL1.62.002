#pragma once

#include "SafeAlloc2.h"
#include "Synchronized2.h"
#include "ThemeSession2.h"
#include "PlayingSound2.h"

namespace Muffle2
{

//=====================================================================================//
//                      class ThemeManager : public Synchronized                       //
//=====================================================================================//
class ThemeManager : public Synchronized
{
	struct SessInfo
	{
		int id;
		WHThemeSession sess;
	};

	typedef std::list< SessInfo, SafeAlloc<SessInfo> > Sessions_t;
	Sessions_t m_sessions;

	int m_idCounter;
	HScript m_lastScript;
	std::string m_lastSound;
	bool m_lastMuted;
	float m_lastStartTime;

	WHPlayingSound m_currentTheme;

public:
	ThemeManager();
	~ThemeManager();

	/// Создать сессию
	HThemeSession createSession();
	/// Убрать сессию из менеджера
	void removeSession(int);

	/// Вызвать при изменении состояния сессий
	void update();

	/// Возвращает экземпляр ThemeManager
	static ThemeManager *inst();

private:
	/// Начать затухание звука
	void startFadeOut(WHPlayingSound snd);
	/// Начать плавное проигрывание звука
	WHPlayingSound startFadeIn();
};

}


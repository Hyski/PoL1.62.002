#pragma once

#include "Ptr2.h"
#include "Synchronized2.h"

namespace Muffle2
{

//=====================================================================================//
//                      class ThemeSession : public Synchronized                       //
//=====================================================================================//
class ThemeSession : public Synchronized
{
	int m_id;
	HScript m_script;
	std::string m_sound;
	bool m_muted;
	float m_startTime;
	long m_startPlaybackTime;

	bool m_paused;

public:
	ThemeSession(int id);
	~ThemeSession();

	/// Сменить тему
	void changeTheme(HScript, const std::string &);
	/// Заглушить тему
	void mute(bool m);

	/// Запаузить
	void pause();
	/// Возобновить проигрывание
	void resume();

	/// Возвращает скрипт
	HScript getScript() const;
	/// Возвращает имя звука
	const std::string &getSound() const;
	/// Возвращает позицию начала проигрывания
	float getStartTime() const { return m_startTime; }

	/// Заглушена ли тема
	bool isMuted() const { return m_muted || m_paused; }
};

typedef Ptr<ThemeSession> HThemeSession;
typedef WeakPtr<ThemeSession> WHThemeSession;

}


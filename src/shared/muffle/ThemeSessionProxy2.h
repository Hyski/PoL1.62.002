#pragma once

#include "ThemeSession2.h"

namespace Muffle2
{

//=====================================================================================//
//               class ThemeSessionProxy : public Muffle::IThemeSession                //
//=====================================================================================//
class ThemeSessionProxy : public Muffle::IThemeSession
{
	HThemeSession m_sess;

public:
	ThemeSessionProxy(HThemeSession sess) : m_sess(sess) {}

	/// Сменить тему
	virtual void changeTheme(HScript script, const std::string &theme) { m_sess->changeTheme(script,theme); }
	virtual void changeTheme(HScript script) { m_sess->changeTheme(script,script->getInfo().sound); }

	/// Запаузить тему
	virtual void pause() { m_sess->pause(); }
	/// Возобновить проигрывание темы
	virtual void resume() { m_sess->resume(); }

	/// Включить/выключить музыкальную тему
	virtual void mute(bool m) { m_sess->mute(m); }

};

}

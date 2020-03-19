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

	/// ������� ����
	virtual void changeTheme(HScript script, const std::string &theme) { m_sess->changeTheme(script,theme); }
	virtual void changeTheme(HScript script) { m_sess->changeTheme(script,script->getInfo().sound); }

	/// ��������� ����
	virtual void pause() { m_sess->pause(); }
	/// ����������� ������������ ����
	virtual void resume() { m_sess->resume(); }

	/// ��������/��������� ����������� ����
	virtual void mute(bool m) { m_sess->mute(m); }

};

}

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

	/// ������� ������
	HThemeSession createSession();
	/// ������ ������ �� ���������
	void removeSession(int);

	/// ������� ��� ��������� ��������� ������
	void update();

	/// ���������� ��������� ThemeManager
	static ThemeManager *inst();

private:
	/// ������ ��������� �����
	void startFadeOut(WHPlayingSound snd);
	/// ������ ������� ������������ �����
	WHPlayingSound startFadeIn();
};

}


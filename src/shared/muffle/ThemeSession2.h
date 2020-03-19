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

	/// ������� ����
	void changeTheme(HScript, const std::string &);
	/// ��������� ����
	void mute(bool m);

	/// ���������
	void pause();
	/// ����������� ������������
	void resume();

	/// ���������� ������
	HScript getScript() const;
	/// ���������� ��� �����
	const std::string &getSound() const;
	/// ���������� ������� ������ ������������
	float getStartTime() const { return m_startTime; }

	/// ��������� �� ����
	bool isMuted() const { return m_muted || m_paused; }
};

typedef Ptr<ThemeSession> HThemeSession;
typedef WeakPtr<ThemeSession> WHThemeSession;

}


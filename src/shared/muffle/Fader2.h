#pragma once

#include "Work2.h"

namespace Muffle2
{

class PlayingSound;

typedef Ptr<PlayingSound> HPlayingSound;
typedef WeakPtr<PlayingSound> WHPlayingSound;

//=====================================================================================//
//                             class Fader2 : public Work                              //
//=====================================================================================//
class Fader : public Work
{
	long m_startTime;
	long m_time;
	bool m_out;
	float m_lastVolume;
	float m_volFactor;

	WHPlayingSound m_psound;


public:
	Fader(WHPlayingSound snd, long time, bool out);
	~Fader();

	/// ���������� true, ���� ������ ��� �� ��������
	virtual bool expired() const;
	/// ��������� ��������
	virtual void execute();

	/// ������� ������� ��������� ���������
	static void createFadeOut(WHPlayingSound snd, long time);
	/// ������� ������� ���������� ���������
	static void createFadeIn(WHPlayingSound snd, long time);
};

}

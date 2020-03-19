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

	/// Возвращает true, если объект уже не актуален
	virtual bool expired() const;
	/// Выполнить действие
	virtual void execute();

	/// Создать плавное затухание громкости
	static void createFadeOut(WHPlayingSound snd, long time);
	/// Создать плавное нарастание громкости
	static void createFadeIn(WHPlayingSound snd, long time);
};

}

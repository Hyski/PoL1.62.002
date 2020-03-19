#pragma once

#include "SafeAlloc2.h"
#include "BufferCtrl2.h"
#include "Synchronized2.h"
#include "PlayingSound2.h"

namespace Muffle2
{

//=====================================================================================//
//                           class Emitter : public IEmitter                           //
//=====================================================================================//
class Emitter : public Synchronized
{
	struct SoundInfo
	{
		int uniqueId;
		WHPlayingSound sound;
	};

	typedef std::vector< SoundInfo, SafeAlloc<SoundInfo> > Sounds_t;

	int m_uniqueCtr;
	Sounds_t m_sounds;
	EmitterInfo m_info;
	Muffle::ISound::Channel m_channel;

public:
	Emitter(Muffle::ISound::Channel chan);
	~Emitter();

	/// Проиграть звук
	int play(HScript script, const std::string &sound, size_t startpos);
	/// Остановить все играющие звука
	void stop();

	/// Играется ли что-нибудь в эмиттере
	bool isPlaying();

	/// Установить громкость эмиттера
	void setVolume(float vol);

	/// Установить позицию звука
	void setPosition(const vec &p);
	/// Установить позицию звука
	void setVelocity(const vec &p);
	/// Установить ориентацию конуса
	void setConeOrientation(const vec &p);

	/// Возвращает количество объектов Emitter
	static long getInstanceCount();
};

typedef Ptr<Emitter> HEmitterImpl;
typedef WeakPtr<Emitter> WHEmitterImpl;

}


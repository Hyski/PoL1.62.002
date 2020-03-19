#pragma once

#include "Ptr2.h"
#include "Work2.h"
#include "SafeAlloc2.h"
#include "Synchronized2.h"
#include "PlayingSound2.h"

namespace Muffle2
{

class PlayingSoundMgr;

typedef Ptr<PlayingSoundMgr> HPlayingSoundMgr;
typedef WeakPtr<PlayingSoundMgr> WHPlayingSoundMgr;

//=====================================================================================//
//                     class PlayingSoundMgr : public Synchronized                     //
//=====================================================================================//
class PlayingSoundMgr : public Synchronized
{
	typedef std::list< HPlayingSound, SafeAlloc<HPlayingSound> > Sounds_t;
	Sounds_t m_sounds;

	HWork m_soundStateSeeker;

public:
	PlayingSoundMgr();
	~PlayingSoundMgr();

	/// Завершить работу, остановить все звуки
	void finish() { stopAllSounds(); }

	/// Добавить звук в менеджер
	void addSound(HPlayingSound snd);
	/// Проверить состояние звуков в менеджере
	void checkSoundStates();

	/// Возвращает количество проигрывающихся звуков
	size_t getPlayingSoundCount() const;

	/// Возвращает экземпляр класса
	static HPlayingSoundMgr inst();

private:
	/// Остановить проигрывание всех звуков
	void stopAllSounds();
	/// Возвращает true, если звуков нет
	bool isEmpty() const;
};


}

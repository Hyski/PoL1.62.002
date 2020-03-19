#pragma once

#include "Ptr2.h"
#include "Work2.h"
#include "Decoder2.h"
#include "BufferCtrl2.h"
#include "EmitterInfo2.h"
#include "SoundBuffer2.h"
#include "Synchronized2.h"

namespace Muffle2
{

class Fader;
class PlayingSound;

typedef Ptr<Fader> HFader;
typedef WeakPtr<Fader> WHFader;
typedef Ptr<PlayingSound> HPlayingSound;
typedef WeakPtr<PlayingSound> WHPlayingSound;

//=====================================================================================//
//                      class PlayingSound : public Synchronized                       //
//=====================================================================================//
class PlayingSound : public Synchronized
{
	friend HPlayingSound;

	HScript m_script;					/// Звуковой скрипт
	bool m_wasPlaying;					/// Проигрывался ли звук в предыдущий момент времени
	size_t m_decodedSamples;			/// Количество сэмплов, которые уже декодированы
	size_t m_playedSamples;				/// Количество проигранных сэмплов
	size_t m_oldPlayPos;				/// Предыдущая позиция проигрывания (для подсчета количества проигранных сэмплов)
	size_t m_decoderDataSampleCount;	/// Количество сэмплов в декодере
	bool m_dynamic;						/// Динамический ли буфер

	HDecoder m_decoder;
	HSoundBuffer m_buffer;
	HBufferCtrl m_ctrl;

	WHFader m_fader;

	PlayingSound(HDecoder decoder, HScript, size_t, const EmitterInfo &, bool muted);

public:
	~PlayingSound();

	/// Проверить, играет ли звук. Возвращает true, если играет
	bool isPlaying();
	/// Остановить проигрывание звука
	void stop();
	/// Остановить проигрывание звука без фэйда
	void badStop();

	/// Осуществить декодирование (если это необходимо)
	void decode(bool force = false);

	/// Установить фэйдер
	void setFader(HFader fader);
	/// Возвращает фэйдер
	WHFader getFader() const;

	/// Установить позицию звука
	void setPosition(const vec &p) { m_buffer->setPosition(p); }
	/// Установить позицию звука
	void setVelocity(const vec &p) { m_buffer->setVelocity(p); }
	/// Установить ориентацию конуса
	void setConeOrientation(const vec &p) { m_buffer->setConeOrientation(p); }

	/// Установить громкость
	void setVolume(float vol) { m_ctrl->setVolume(vol); }
	/// Возвращает громкость
	float getVolume() const { return m_ctrl->getVolume(); }

	/// Проиграть звук
	static HPlayingSound createPlayingSound(HScript, const std::string &sound,
											size_t startpos, const EmitterInfo &,
											bool muted = false);

		/// Проиграть звук
	static HPlayingSound createPlayingSound(HDecoder, HScript, const std::string &,
											size_t startpos, const EmitterInfo &,
											bool muted = false);

private:
	/// Проверить, остановилось ли проигрывание буфера
	bool isPlaybackStopped();
	/// Метод вызывается, когда звук прекращает проигрываться
	void onStop();
};

}

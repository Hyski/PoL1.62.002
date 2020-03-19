#pragma once

#include "format.h"

#include "Ptr2.h"
#include "Locker2.h"
#include "EmitterInfo2.h"
#include "Synchronized2.h"

namespace Muffle2
{

//=====================================================================================//
//                       class SoundBuffer : public Synchronized                       //
//=====================================================================================//
class SoundBuffer : public Synchronized
{
	Format m_format;
	ATL::CComPtr<IDirectSoundBuffer8> m_buffer;
	ATL::CComPtr<IDirectSound3DBuffer8> m_buffer3d;
	size_t m_bufferSize;
	size_t m_lastWritePos;

	bool m_is3d;
	bool m_dynamic;
	size_t m_currentPlayPosition;
	size_t m_currentWritePosition;

public:
	static const float MaxStaticBufferLengthInSeconds; // = 2.0f;

	SoundBuffer(size_t, const Format &, bool, bool, const EmitterInfo &);
	~SoundBuffer();

	/// Возвращает динамический ли это буфер
	bool isDynamic() const { return m_dynamic; }
	/// Трехмерный ли источник
	bool is3d() const { return m_is3d; }
	/// Играется ли на данный момент буфер
	bool isPlaying();

	/// Начать проигрывание
	void play(bool loop);
	/// Остановить проигрывание
	void stop();

	/// Обновить позиции чтения/записи
	void updateCurrentPosition();

	/// Возвращает текущую позицию проигрывания
	size_t getCurrentPlayPosition() const { return m_currentPlayPosition; }
	/// Возвращает текущую позицию записи
	size_t getCurrentWritePosition() const { return m_currentWritePosition; }
	/// Возвращает размер буфера в сэмплах
	size_t getSize() const { return m_bufferSize; }

	/// Залочить доступный кусок буфера
	Locker lockBuffer();
	/// Разлочить кусок буфера
	void unlockBuffer(const LockData &);

	/// Возвращает формат буфера
	const Format &getFormat() const { return m_format; }
	/// Установить громкость
	void setVolume(long decibel);
	/// Возвращает частоту дискретизации
	long getFrequency() const;
	/// Установить частоту дискретизации
	void setFrequency(long freq);

	/// Установить позицию
	void setPosition(const vec &p);
	/// Установить скорость
	void setVelocity(const vec &p);
	/// Установить дистанции
	void setDistances(float n, float f);
	/// Установить параметры конуса
	void setConeParams(float min, float max, float outvol);
	/// Установить направление конуса
	void setConeOrientation(const vec &p);

	/// Создать "правильный" буфер
	static HSoundBuffer createBuffer(size_t size, const Format &fmt, bool, const EmitterInfo &);

	/// Возвращает количество объектов SoundBuffer
	static long getInstanceCount();

private:
	bool doIsPlaying();
	void doUpdateCurrentPosition();
};

typedef Ptr<SoundBuffer> HSoundBuffer;
typedef WeakPtr<SoundBuffer> WHSoundBuffer;

}

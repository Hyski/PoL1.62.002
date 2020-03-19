#pragma once

#include "BufferCtrl2.h"
#include "SoundBuffer2.h"

namespace Muffle2
{

//=====================================================================================//
//                      class BufferCtrlImpl : public BufferCtrl                       //
//=====================================================================================//
class BufferCtrlImpl : public BufferCtrl
{
	BufferCtrlMgr::Iterator_t m_self;	///< Итератор на себя любимого

	float m_vol;				///< Собственный уровень громкости
	float m_muted;				///< Заглушен ли звук
	float m_actualVol;			///< Текущий уровень громкости

	long m_frequency;			///< Изначальная частота дискретизации звука
	float m_freqFactor;			///< Коэффициент к частоте дискретизации
	float m_actualFreqFactor;	///< Текущий коэффициент к частоте дискретизации

	WHSoundBuffer m_buffer;		///< WeakPtr на звуковой буфер

public:
	BufferCtrlImpl(BufferCtrlMgr::Iterator_t it);
	BufferCtrlImpl(BufferCtrlMgr::Iterator_t it, HSoundBuffer buf);
	~BufferCtrlImpl();

	/// Установить громкость
	virtual void setVolume(float vol);
	/// Установить множитель к частоте дискретизации
	virtual void setFrequencyFactor(float val);
	/// Заглушить/включить звук
	virtual void mute(bool);

	/// Возвращает громкость
	virtual float getVolume() const { return m_muted * m_vol * getParentVolume(); }
	/// Возвращает коэффициент к частоте дискретизации
	virtual float getFrequencyFactor() const { return m_freqFactor * getParentFrequencyFactor(); }

	virtual void onParentChanged();

	/// Возвращает итератор на себя
	virtual BufferCtrlMgr::Iterator_t getSelfIterator() const { return m_self; }

private:
	/// Возвращает громкость родителя
	float getParentVolume() const;
	/// Возвращает коэффициент к частоте дискретизации родителя
	float getParentFrequencyFactor() const;

	void adjust();
	/// Возвращает true, если состояние BufferCtrlImpl изменилось
	bool adjustSelf();
	void adjustDependents();

	long getVolumeInDecibels() const;
	long getFrequencyInSamplesPerSecond() const;
};

}
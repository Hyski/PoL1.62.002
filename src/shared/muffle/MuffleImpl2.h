#pragma once

#include "Kernel2.h"

namespace Muffle2
{

//=====================================================================================//
//                          class MuffleImpl : public ISound                           //
//=====================================================================================//
class MuffleImpl : public Muffle::ISound
{
	Muffle::SndServices *m_svc;
	Kernel m_kernel;

public:
	MuffleImpl(Muffle::SndServices *);
	~MuffleImpl();

	virtual void manage();

	// Начать музыкальную сессию
	virtual Muffle::HThemeSession beginSession() { return m_kernel.beginSession(); }

	// Создать источник звука
	virtual Muffle::HEmitter createEmitter(Channel);
	virtual Muffle::HEmitter createEmitter(Muffle::HScript script);
	virtual Muffle::HEmitter createEmitter(Muffle::HScript script, const std::string& waveFile);

	// Проиграть звук. После запуска звука с помощью этих функций 
	// его никак проконтролировать нельзя.
	virtual void emitSound(Muffle::HScript script, const std::string& waveFile, float startSec);
	virtual void emitSound(Muffle::HScript script, float startSec);

	/// Возвращает длительность звука в секундах
	virtual float getSoundDuration(const std::string &sound);

	/// Возвращает указатель на скрипт
	virtual Muffle::HScript getScript(const std::string& scriptName);
	/// Внедрить скрипт в Muffle
	virtual HScript embedScript(const Muffle::ScriptInfo &);

	/// Установить громкость звука для определенного канала
	virtual void setVolume(Channel, float volume);
	/// Установить коэффициент частоты дискретизации на канал
	virtual void setFrequency(Channel, float factor);
	/// Включить/выключить определенный канал
	virtual void muteChannel(Channel, bool);

	/// Получить громкость звука для определенного канала
	virtual float getVolume(Channel) const;


	// Изменить позицию камеры
	virtual void setCamera(const vec &dir, const vec &up, const vec &right, const vec &origin);

	static MuffleImpl *inst();

};

}

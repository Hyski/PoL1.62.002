#pragma once

#include "Emitter2.h"

namespace Muffle2
{

//=====================================================================================//
//                        class EmitterProxy : public IEmitter                         //
//=====================================================================================//
class EmitterProxy : public Muffle::IEmitter
{
	HScript m_defaultScript;
	std::string m_defaultSound;
	HEmitterImpl m_emitter;

public:
	EmitterProxy(HScript script, const std::string &sound, HEmitterImpl em);

	virtual int play();
	virtual int play(HScript script, const std::string &sound, float startpos);

	/// Остановить все звуки в эмиттере
	virtual void stop() { m_emitter->stop(); }

	/// Установить позицию звука
	virtual void setPosition(const vec &p) { m_emitter->setPosition(p); }
	/// Установить позицию звука
	virtual void setVelocity(const vec &p) { m_emitter->setVelocity(p); }
	/// Установить ориентацию конуса
	virtual void setConeOrientation(const vec &p) { m_emitter->setConeOrientation(p); }

	virtual void setVolume(float vol) { m_emitter->setVolume(vol); }

	/// Возвращает состояние источника, проигрывает ли он на данный момент звук
	virtual bool isPlaying() { return m_emitter->isPlaying(); }

	/// Возвращает "валидность" эмиттера, т. е. правильный ли в нем скрипт и вавчик
	virtual bool isValid() const { return true; }
};

}

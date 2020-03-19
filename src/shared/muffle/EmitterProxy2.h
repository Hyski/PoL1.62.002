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

	/// ���������� ��� ����� � ��������
	virtual void stop() { m_emitter->stop(); }

	/// ���������� ������� �����
	virtual void setPosition(const vec &p) { m_emitter->setPosition(p); }
	/// ���������� ������� �����
	virtual void setVelocity(const vec &p) { m_emitter->setVelocity(p); }
	/// ���������� ���������� ������
	virtual void setConeOrientation(const vec &p) { m_emitter->setConeOrientation(p); }

	virtual void setVolume(float vol) { m_emitter->setVolume(vol); }

	/// ���������� ��������� ���������, ����������� �� �� �� ������ ������ ����
	virtual bool isPlaying() { return m_emitter->isPlaying(); }

	/// ���������� "����������" ��������, �. �. ���������� �� � ��� ������ � ������
	virtual bool isValid() const { return true; }
};

}

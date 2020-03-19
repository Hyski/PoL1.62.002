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

	// ������ ����������� ������
	virtual Muffle::HThemeSession beginSession() { return m_kernel.beginSession(); }

	// ������� �������� �����
	virtual Muffle::HEmitter createEmitter(Channel);
	virtual Muffle::HEmitter createEmitter(Muffle::HScript script);
	virtual Muffle::HEmitter createEmitter(Muffle::HScript script, const std::string& waveFile);

	// ��������� ����. ����� ������� ����� � ������� ���� ������� 
	// ��� ����� ����������������� ������.
	virtual void emitSound(Muffle::HScript script, const std::string& waveFile, float startSec);
	virtual void emitSound(Muffle::HScript script, float startSec);

	/// ���������� ������������ ����� � ��������
	virtual float getSoundDuration(const std::string &sound);

	/// ���������� ��������� �� ������
	virtual Muffle::HScript getScript(const std::string& scriptName);
	/// �������� ������ � Muffle
	virtual HScript embedScript(const Muffle::ScriptInfo &);

	/// ���������� ��������� ����� ��� ������������� ������
	virtual void setVolume(Channel, float volume);
	/// ���������� ����������� ������� ������������� �� �����
	virtual void setFrequency(Channel, float factor);
	/// ��������/��������� ������������ �����
	virtual void muteChannel(Channel, bool);

	/// �������� ��������� ����� ��� ������������� ������
	virtual float getVolume(Channel) const;


	// �������� ������� ������
	virtual void setCamera(const vec &dir, const vec &up, const vec &right, const vec &origin);

	static MuffleImpl *inst();

};

}

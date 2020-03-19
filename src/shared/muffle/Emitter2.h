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

	/// ��������� ����
	int play(HScript script, const std::string &sound, size_t startpos);
	/// ���������� ��� �������� �����
	void stop();

	/// �������� �� ���-������ � ��������
	bool isPlaying();

	/// ���������� ��������� ��������
	void setVolume(float vol);

	/// ���������� ������� �����
	void setPosition(const vec &p);
	/// ���������� ������� �����
	void setVelocity(const vec &p);
	/// ���������� ���������� ������
	void setConeOrientation(const vec &p);

	/// ���������� ���������� �������� Emitter
	static long getInstanceCount();
};

typedef Ptr<Emitter> HEmitterImpl;
typedef WeakPtr<Emitter> WHEmitterImpl;

}


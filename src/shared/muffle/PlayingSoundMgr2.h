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

	/// ��������� ������, ���������� ��� �����
	void finish() { stopAllSounds(); }

	/// �������� ���� � ��������
	void addSound(HPlayingSound snd);
	/// ��������� ��������� ������ � ���������
	void checkSoundStates();

	/// ���������� ���������� ��������������� ������
	size_t getPlayingSoundCount() const;

	/// ���������� ��������� ������
	static HPlayingSoundMgr inst();

private:
	/// ���������� ������������ ���� ������
	void stopAllSounds();
	/// ���������� true, ���� ������ ���
	bool isEmpty() const;
};


}

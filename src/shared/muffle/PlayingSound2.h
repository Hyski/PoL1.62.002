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

	HScript m_script;					/// �������� ������
	bool m_wasPlaying;					/// ������������ �� ���� � ���������� ������ �������
	size_t m_decodedSamples;			/// ���������� �������, ������� ��� ������������
	size_t m_playedSamples;				/// ���������� ����������� �������
	size_t m_oldPlayPos;				/// ���������� ������� ������������ (��� �������� ���������� ����������� �������)
	size_t m_decoderDataSampleCount;	/// ���������� ������� � ��������
	bool m_dynamic;						/// ������������ �� �����

	HDecoder m_decoder;
	HSoundBuffer m_buffer;
	HBufferCtrl m_ctrl;

	WHFader m_fader;

	PlayingSound(HDecoder decoder, HScript, size_t, const EmitterInfo &, bool muted);

public:
	~PlayingSound();

	/// ���������, ������ �� ����. ���������� true, ���� ������
	bool isPlaying();
	/// ���������� ������������ �����
	void stop();
	/// ���������� ������������ ����� ��� �����
	void badStop();

	/// ����������� ������������� (���� ��� ����������)
	void decode(bool force = false);

	/// ���������� ������
	void setFader(HFader fader);
	/// ���������� ������
	WHFader getFader() const;

	/// ���������� ������� �����
	void setPosition(const vec &p) { m_buffer->setPosition(p); }
	/// ���������� ������� �����
	void setVelocity(const vec &p) { m_buffer->setVelocity(p); }
	/// ���������� ���������� ������
	void setConeOrientation(const vec &p) { m_buffer->setConeOrientation(p); }

	/// ���������� ���������
	void setVolume(float vol) { m_ctrl->setVolume(vol); }
	/// ���������� ���������
	float getVolume() const { return m_ctrl->getVolume(); }

	/// ��������� ����
	static HPlayingSound createPlayingSound(HScript, const std::string &sound,
											size_t startpos, const EmitterInfo &,
											bool muted = false);

		/// ��������� ����
	static HPlayingSound createPlayingSound(HDecoder, HScript, const std::string &,
											size_t startpos, const EmitterInfo &,
											bool muted = false);

private:
	/// ���������, ������������ �� ������������ ������
	bool isPlaybackStopped();
	/// ����� ����������, ����� ���� ���������� �������������
	void onStop();
};

}

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

	/// ���������� ������������ �� ��� �����
	bool isDynamic() const { return m_dynamic; }
	/// ���������� �� ��������
	bool is3d() const { return m_is3d; }
	/// �������� �� �� ������ ������ �����
	bool isPlaying();

	/// ������ ������������
	void play(bool loop);
	/// ���������� ������������
	void stop();

	/// �������� ������� ������/������
	void updateCurrentPosition();

	/// ���������� ������� ������� ������������
	size_t getCurrentPlayPosition() const { return m_currentPlayPosition; }
	/// ���������� ������� ������� ������
	size_t getCurrentWritePosition() const { return m_currentWritePosition; }
	/// ���������� ������ ������ � �������
	size_t getSize() const { return m_bufferSize; }

	/// �������� ��������� ����� ������
	Locker lockBuffer();
	/// ��������� ����� ������
	void unlockBuffer(const LockData &);

	/// ���������� ������ ������
	const Format &getFormat() const { return m_format; }
	/// ���������� ���������
	void setVolume(long decibel);
	/// ���������� ������� �������������
	long getFrequency() const;
	/// ���������� ������� �������������
	void setFrequency(long freq);

	/// ���������� �������
	void setPosition(const vec &p);
	/// ���������� ��������
	void setVelocity(const vec &p);
	/// ���������� ���������
	void setDistances(float n, float f);
	/// ���������� ��������� ������
	void setConeParams(float min, float max, float outvol);
	/// ���������� ����������� ������
	void setConeOrientation(const vec &p);

	/// ������� "����������" �����
	static HSoundBuffer createBuffer(size_t size, const Format &fmt, bool, const EmitterInfo &);

	/// ���������� ���������� �������� SoundBuffer
	static long getInstanceCount();

private:
	bool doIsPlaying();
	void doUpdateCurrentPosition();
};

typedef Ptr<SoundBuffer> HSoundBuffer;
typedef WeakPtr<SoundBuffer> WHSoundBuffer;

}

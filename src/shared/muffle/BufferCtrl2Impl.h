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
	BufferCtrlMgr::Iterator_t m_self;	///< �������� �� ���� ��������

	float m_vol;				///< ����������� ������� ���������
	float m_muted;				///< �������� �� ����
	float m_actualVol;			///< ������� ������� ���������

	long m_frequency;			///< ����������� ������� ������������� �����
	float m_freqFactor;			///< ����������� � ������� �������������
	float m_actualFreqFactor;	///< ������� ����������� � ������� �������������

	WHSoundBuffer m_buffer;		///< WeakPtr �� �������� �����

public:
	BufferCtrlImpl(BufferCtrlMgr::Iterator_t it);
	BufferCtrlImpl(BufferCtrlMgr::Iterator_t it, HSoundBuffer buf);
	~BufferCtrlImpl();

	/// ���������� ���������
	virtual void setVolume(float vol);
	/// ���������� ��������� � ������� �������������
	virtual void setFrequencyFactor(float val);
	/// ���������/�������� ����
	virtual void mute(bool);

	/// ���������� ���������
	virtual float getVolume() const { return m_muted * m_vol * getParentVolume(); }
	/// ���������� ����������� � ������� �������������
	virtual float getFrequencyFactor() const { return m_freqFactor * getParentFrequencyFactor(); }

	virtual void onParentChanged();

	/// ���������� �������� �� ����
	virtual BufferCtrlMgr::Iterator_t getSelfIterator() const { return m_self; }

private:
	/// ���������� ��������� ��������
	float getParentVolume() const;
	/// ���������� ����������� � ������� ������������� ��������
	float getParentFrequencyFactor() const;

	void adjust();
	/// ���������� true, ���� ��������� BufferCtrlImpl ����������
	bool adjustSelf();
	void adjustDependents();

	long getVolumeInDecibels() const;
	long getFrequencyInSamplesPerSecond() const;
};

}
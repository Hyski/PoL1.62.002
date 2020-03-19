#pragma once

#include "Ptr2.h"
#include "Synchronized2.h"

#include <mll/utils/lcrn_tree.h>

namespace Muffle2
{

class BufferCtrl;
class SoundBuffer;

typedef Ptr<BufferCtrl> HBufferCtrl;
typedef WeakPtr<BufferCtrl> WHBufferCtrl;
typedef Ptr<SoundBuffer> HSoundBuffer;

//=====================================================================================//
//                      class BufferCtrlMgr : public Synchronized                      //
//=====================================================================================//
class BufferCtrlMgr : public Synchronized
{
	typedef mll::utils::lcrn_tree<WHBufferCtrl> Ctrls_t;
	Ctrls_t m_ctrls;

	/// ��������� �������
	HBufferCtrl m_globalCtrls[Muffle::ISound::cChannelCount];

public:
	typedef Ctrls_t::iterator Iterator_t;

public:
	BufferCtrlMgr();
	~BufferCtrlMgr();

	/// ������� ����� BufferCtrl
	HBufferCtrl createBufferCtrl();
	HBufferCtrl createBufferCtrl(HBufferCtrl parent);
	HBufferCtrl createBufferCtrl(HBufferCtrl parent, HSoundBuffer buf);

	/// ���������� ���������� ������
	HBufferCtrl getBufferCtrl(Muffle::ISound::Channel);

	/// ���������� ��������� ������������� �����������
	float getParentVolume(Iterator_t);
	/// ���������� ����������� � ������� ������������� ������������� �����������
	float getParentFrequencyFactor(Iterator_t);

	/// ������� onParentChanged � �������� ������������
	void adjustChildren(Iterator_t);

	/// ������ ��������� BufferCtrl �� ������
	void removeBufferCtrl(Iterator_t itor);

	/// ���������� ��������� BufferCtrlMgr
	static BufferCtrlMgr *inst();
};

}


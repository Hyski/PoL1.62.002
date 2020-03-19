#pragma once

#include "Ptr2.h"
#include "BufferCtrlMgr2.h"

namespace Muffle2
{

//=====================================================================================//
//                         class Volume : public Synchronized                          //
//=====================================================================================//
class BufferCtrl : public Synchronized
{
public:
	BufferCtrl();
	~BufferCtrl();

	/// ���������� ���������
	virtual void setVolume(float vol) = 0;
	/// ���������� ��������� � ������� �������������
	virtual void setFrequencyFactor(float val) = 0;
	/// ���������/�������� ����
	virtual void mute(bool m) = 0;

	/// ���������� ���������
	virtual float getVolume() const = 0;
	/// ���������� ����������� � ������� �������������
	virtual float getFrequencyFactor() const = 0;

	/// ������������ ��� ��������� ���������� ��������
	virtual void onParentChanged() = 0;

	/// ���������� �������� �� ����
	virtual BufferCtrlMgr::Iterator_t getSelfIterator() const = 0;

	/// ���������� ���������� �������� BufferCtrl
	static long getInstanceCount();
};

typedef Ptr<BufferCtrl> HBufferCtrl;
typedef WeakPtr<BufferCtrl> WHBufferCtrl;

}

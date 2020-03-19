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

	/// Установить громкость
	virtual void setVolume(float vol) = 0;
	/// Установить множитель к частоте дискретизации
	virtual void setFrequencyFactor(float val) = 0;
	/// Заглушить/включить звук
	virtual void mute(bool m) = 0;

	/// Возвращает громкость
	virtual float getVolume() const = 0;
	/// Возвращает коэффициент к частоте дискретизации
	virtual float getFrequencyFactor() const = 0;

	/// Вызывающийся при изменении параметров родителя
	virtual void onParentChanged() = 0;

	/// Возвращает итератор на себя
	virtual BufferCtrlMgr::Iterator_t getSelfIterator() const = 0;

	/// Возвращает количество объектов BufferCtrl
	static long getInstanceCount();
};

typedef Ptr<BufferCtrl> HBufferCtrl;
typedef WeakPtr<BufferCtrl> WHBufferCtrl;

}

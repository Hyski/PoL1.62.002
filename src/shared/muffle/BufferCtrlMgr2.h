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

	/// Громкости каналов
	HBufferCtrl m_globalCtrls[Muffle::ISound::cChannelCount];

public:
	typedef Ctrls_t::iterator Iterator_t;

public:
	BufferCtrlMgr();
	~BufferCtrlMgr();

	/// Создать новый BufferCtrl
	HBufferCtrl createBufferCtrl();
	HBufferCtrl createBufferCtrl(HBufferCtrl parent);
	HBufferCtrl createBufferCtrl(HBufferCtrl parent, HSoundBuffer buf);

	/// Возвращает контроллер канала
	HBufferCtrl getBufferCtrl(Muffle::ISound::Channel);

	/// Возвращает громкость родительского контроллера
	float getParentVolume(Iterator_t);
	/// Возвращает коэффициент к частоте дискретизации родительского контроллера
	float getParentFrequencyFactor(Iterator_t);

	/// Вызвать onParentChanged у дочерних контроллеров
	void adjustChildren(Iterator_t);

	/// Убрать указанный BufferCtrl из дерева
	void removeBufferCtrl(Iterator_t itor);

	/// Возвращает экземпляр BufferCtrlMgr
	static BufferCtrlMgr *inst();
};

}


#include "precomp.h"
#include "SafeAlloc2.h"
#include "BufferCtrlMgr2.h"
#include "BufferCtrl2Impl.h"

namespace Muffle2
{

static BufferCtrlMgr *g_inst = 0;

//=====================================================================================//
//                           BufferCtrlMgr::BufferCtrlMgr()                            //
//=====================================================================================//
BufferCtrlMgr::BufferCtrlMgr()
{
	assert( g_inst == 0 );
	g_inst = this;

	m_globalCtrls[Muffle::ISound::cMaster] = createBufferCtrl();

	for(int i = Muffle::ISound::cThemes; i != Muffle::ISound::cChannelCount; ++i)
	{
		m_globalCtrls[i] = createBufferCtrl(m_globalCtrls[Muffle::ISound::cMaster]);
	}
}

//=====================================================================================//
//                           BufferCtrlMgr::~BufferCtrlMgr()                           //
//=====================================================================================//
BufferCtrlMgr::~BufferCtrlMgr()
{
	assert( g_inst == this );
	g_inst = 0;
}

//=====================================================================================//
//                       float BufferCtrlMgr::getParentVolume()                        //
//=====================================================================================//
float BufferCtrlMgr::getParentVolume(Iterator_t it)
{
	MUFFLE_SYNCHRONIZED_THIS;

	if(it.has_parent())
	{
		if(HBufferCtrl lockedParent = it.get_parent()->acquire())
		{
			return lockedParent->getVolume();
		}
	}

	return 1.0f;
}

//=====================================================================================//
//                   float BufferCtrlMgr::getParentFrequencyFactor()                   //
//=====================================================================================//
float BufferCtrlMgr::getParentFrequencyFactor(Iterator_t it)
{
	MUFFLE_SYNCHRONIZED_THIS;

	if(it.has_parent())
	{
		if(HBufferCtrl lockedParent = it.get_parent()->acquire())
		{
			return lockedParent->getFrequencyFactor();
		}
	}

	return 1.0f;
}

//=====================================================================================//
//                        void BufferCtrlMgr::adjustChildren()                         //
//=====================================================================================//
void BufferCtrlMgr::adjustChildren(Iterator_t i)
{
	typedef std::list< HBufferCtrl, SafeAlloc<HBufferCtrl> > BufferCtrls_t;
	BufferCtrls_t toupdate;

	{
		MUFFLE_SYNCHRONIZED_THIS;
		for(BufferCtrlMgr::Iterator_t it = i.get_son(); it != i.son_end(); it.brother())
		{
			if(HBufferCtrl lockedChild = it->acquire())
			{
				toupdate.push_back(lockedChild);
			}
		}
	}

	for(BufferCtrls_t::iterator i = toupdate.begin(); i != toupdate.end(); ++i)
	{
		(*i)->onParentChanged();
	}
}

//=====================================================================================//
//                     HBufferCtrl BufferCtrlMgr::getBufferCtrl()                      //
//=====================================================================================//
HBufferCtrl BufferCtrlMgr::getBufferCtrl(Muffle::ISound::Channel chan)
{
	MUFFLE_SYNCHRONIZED_THIS;

	return m_globalCtrls[chan];
}

//=====================================================================================//
//                    HBufferCtrl BufferCtrlMgr::createBufferCtrl()                    //
//=====================================================================================//
HBufferCtrl BufferCtrlMgr::createBufferCtrl()
{
	MUFFLE_SYNCHRONIZED_THIS;

	Iterator_t ctrlIt = m_ctrls.insert_son(m_ctrls.end(),WHBufferCtrl());
	HBufferCtrl ctrl = Ptr<BufferCtrlImpl>::create(ctrlIt);
	*ctrlIt = ctrl;
	return ctrl;
}

//=====================================================================================//
//                    HBufferCtrl BufferCtrlMgr::createBufferCtrl()                    //
//=====================================================================================//
HBufferCtrl BufferCtrlMgr::createBufferCtrl(HBufferCtrl parent)
{
	MUFFLE_SYNCHRONIZED_THIS;

	Iterator_t ctrlIt = m_ctrls.insert_son(parent->getSelfIterator(),WHBufferCtrl());
	HBufferCtrl ctrl = Ptr<BufferCtrlImpl>::create(ctrlIt);
	*ctrlIt = ctrl;
	return ctrl;
}

//=====================================================================================//
//                    HBufferCtrl BufferCtrlMgr::createBufferCtrl()                    //
//=====================================================================================//
HBufferCtrl BufferCtrlMgr::createBufferCtrl(HBufferCtrl parent, HSoundBuffer buf)
{
	MUFFLE_SYNCHRONIZED_THIS;

	Iterator_t ctrlIt = m_ctrls.insert_son(parent->getSelfIterator(),WHBufferCtrl());
	HBufferCtrl ctrl = Ptr<BufferCtrlImpl>::create(ctrlIt,buf);
	*ctrlIt = ctrl;
	return ctrl;
}

//=====================================================================================//
//                       void BufferCtrlMgr::removeBufferCtrl()                        //
//=====================================================================================//
void BufferCtrlMgr::removeBufferCtrl(Iterator_t itor)
{
	MUFFLE_SYNCHRONIZED_THIS;

	assert( !itor.has_son() );
	m_ctrls.erase(itor);
}

//=====================================================================================//
//                        BufferCtrlMgr *BufferCtrlMgr::inst()                         //
//=====================================================================================//
BufferCtrlMgr *BufferCtrlMgr::inst()
{
	return g_inst;
}

}

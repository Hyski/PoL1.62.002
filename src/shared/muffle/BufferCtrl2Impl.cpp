#include "precomp.h"
#include "BufferCtrl2Impl.h"

namespace Muffle2
{

//=====================================================================================//
//                          BufferCtrlImpl::BufferCtrlImpl()                           //
//=====================================================================================//
BufferCtrlImpl::BufferCtrlImpl(BufferCtrlMgr::Iterator_t it)
:	m_self(it),
	m_vol(1.0f),
	m_muted(1.0f),
	m_actualVol(-1.0f),
	m_frequency(0L),
	m_freqFactor(1.0f),
	m_actualFreqFactor(1.0f)
{
	adjustSelf();
}

//=====================================================================================//
//                          BufferCtrlImpl::BufferCtrlImpl()                           //
//=====================================================================================//
BufferCtrlImpl::BufferCtrlImpl(BufferCtrlMgr::Iterator_t it, HSoundBuffer buf)
:	m_self(it),
	m_buffer(buf),
	m_vol(1.0f),
	m_muted(1.0f),
	m_actualVol(-1.0f),
	m_frequency(buf->getFrequency()),
	m_freqFactor(1.0f),
	m_actualFreqFactor(1.0f)
{
	adjustSelf();
}

//=====================================================================================//
//                          BufferCtrlImpl::~BufferCtrlImpl()                          //
//=====================================================================================//
BufferCtrlImpl::~BufferCtrlImpl()
{
}

//=====================================================================================//
//                    float BufferCtrlImpl::getParentVolume() const                    //
//=====================================================================================//
float BufferCtrlImpl::getParentVolume() const
{
	return BufferCtrlMgr::inst()->getParentVolume(m_self);
}

//=====================================================================================//
//               float BufferCtrlImpl::getParentFrequencyFactor() const                //
//=====================================================================================//
float BufferCtrlImpl::getParentFrequencyFactor() const
{
	return BufferCtrlMgr::inst()->getParentFrequencyFactor(m_self);
}

//=====================================================================================//
//                          bool BufferCtrlImpl::adjustSelf()                          //
//=====================================================================================//
bool BufferCtrlImpl::adjustSelf()
{
	const float curvol = getVolume();
	const float curfreq = getFrequencyFactor();
	const bool result = (curvol != m_actualVol || curfreq != m_actualFreqFactor);

	if(HSoundBuffer lockedBuffer = m_buffer.acquire())
	{
		if(curvol != m_actualVol)
		{
			lockedBuffer->setVolume(getVolumeInDecibels());
		}

		if(curfreq != m_actualFreqFactor)
		{
			lockedBuffer->setFrequency(getFrequencyInSamplesPerSecond());
		}
	}

	m_actualVol = curvol;
	m_actualFreqFactor = curfreq;

	return result;
}

//=====================================================================================//
//                     long BufferCtrlImpl::getVolumeInDecibels()                      //
//=====================================================================================//
long BufferCtrlImpl::getVolumeInDecibels() const
{
	return normToDecibel(getVolume());
}

//=====================================================================================//
//                long BufferCtrlImpl::getFrequencyInSamplesPerSecond()                //
//=====================================================================================//
long BufferCtrlImpl::getFrequencyInSamplesPerSecond() const
{
	return static_cast<long>(static_cast<float>(m_frequency)*getFrequencyFactor());
}

//=====================================================================================//
//                            void BufferCtrlImpl::adjust()                            //
//=====================================================================================//
void BufferCtrlImpl::adjust()
{
	bool changed;

	{
		MUFFLE_SYNCHRONIZED_THIS;
		changed = adjustSelf();
	}

	if(changed)
	{
		adjustDependents();
	}
}

//=====================================================================================//
//                       void BufferCtrlImpl::onParentChanged()                        //
//=====================================================================================//
void BufferCtrlImpl::onParentChanged()
{
	adjust();
}

//=====================================================================================//
//                       void BufferCtrlImpl::adjustDependents()                       //
//=====================================================================================//
void BufferCtrlImpl::adjustDependents()
{
	BufferCtrlMgr::inst()->adjustChildren(m_self);
}

//=====================================================================================//
//                          void BufferCtrlImpl::setVolume()                           //
//=====================================================================================//
void BufferCtrlImpl::setVolume(float vol)
{
	{
		MUFFLE_SYNCHRONIZED_THIS;
		m_vol = vol;
	}

	adjust();
}

//=====================================================================================//
//                      void BufferCtrlImpl::setFrequencyFactor()                      //
//=====================================================================================//
void BufferCtrlImpl::setFrequencyFactor(float val)
{
	{
		MUFFLE_SYNCHRONIZED_THIS;
		m_freqFactor = val;
	}

	adjust();
}

//=====================================================================================//
//                             void BufferCtrlImpl::mute()                             //
//=====================================================================================//
void BufferCtrlImpl::mute(bool m)
{
	{
		MUFFLE_SYNCHRONIZED_THIS;
		m_muted = m ? 0.0f : 1.0f;
	}

	adjust();
}


}

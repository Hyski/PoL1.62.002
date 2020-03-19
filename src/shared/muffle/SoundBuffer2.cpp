#include "precomp.h"

#include "Kernel2.h"
#include "SafeAlloc2.h"
#include "SoundBuffer2.h"

namespace Muffle2
{

static volatile long g_instanceCount = 0;
const float SoundBuffer::MaxStaticBufferLengthInSeconds = 2.0f;

//=====================================================================================//
//               static ATL::CComPtr<IDirectSoundBuffer8> createBuffer()               //
//=====================================================================================//
static ATL::CComPtr<IDirectSoundBuffer8> doCreateBuffer(IDirectSound8 *dsound, size_t size,
													  const Format &fmt, bool is3d, bool dynamic)
{
	WAVEFORMATEX wfmt;
	DSBUFFERDESC desc;

	DWORD initialFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY;
	initialFlags |= is3d ? DSBCAPS_CTRL3D : 0;
	initialFlags |= dynamic ? 0 : DSBCAPS_STATIC;

	desc.dwSize = sizeof(desc);
	desc.dwFlags = initialFlags|DSBCAPS_LOCHARDWARE;
	desc.dwBufferBytes = size * sizeof(short) * fmt.channels;
	desc.dwReserved = 0;
	desc.lpwfxFormat = &wfmt;
	desc.guid3DAlgorithm = (is3d ? DS3DALG_DEFAULT : GUID_NULL);

	wfmt.cbSize = sizeof(wfmt);
	wfmt.nChannels = fmt.channels;
	wfmt.nSamplesPerSec = fmt.rate;
	wfmt.wBitsPerSample = sizeof(short)*8;
	wfmt.nBlockAlign = sizeof(short)*fmt.channels;
	wfmt.wFormatTag = WAVE_FORMAT_PCM;
	wfmt.nAvgBytesPerSec = fmt.rate * wfmt.nBlockAlign;

	ATL::CComPtr<IDirectSoundBuffer> buf;

//#if !defined(__MUFFLE_NO_HARDWARE_BUFFERS__)
	dsound->CreateSoundBuffer(&desc,&buf,NULL);
//#endif

	if(!buf)
	{
		desc.dwFlags = initialFlags|DSBCAPS_LOCSOFTWARE;
		dsound->CreateSoundBuffer(&desc,&buf,NULL);
	}

	ATL::CComPtr<IDirectSoundBuffer8> buf8;
	if(buf)
	{
		SAFE_CALL(buf->QueryInterface(IID_IDirectSoundBuffer8,(void**)&buf8));
	}

	return buf8;
}

//=====================================================================================//
//                             SoundBuffer::SoundBuffer()                              //
//=====================================================================================//
SoundBuffer::SoundBuffer(size_t size, const Format &fmt, bool is3d, bool dynamic,
						 const EmitterInfo &einfo)
:	m_format(fmt),
	m_bufferSize(size),
	m_lastWritePos(0),
	m_currentPlayPosition(0),
	m_currentWritePosition(0),
	m_dynamic(dynamic),
	m_is3d(is3d)
{
	InterlockedIncrement(&g_instanceCount);

	if(size)
	{
		m_buffer = doCreateBuffer(Kernel::inst()->getDirectSound(), size, fmt, is3d, dynamic);
		if(m_is3d)
		{
			SAFE_CALL( m_buffer->QueryInterface(IID_IDirectSound3DBuffer8,(void**)&m_buffer3d) );
			setPosition(einfo.position);
			setVelocity(einfo.velocity);
			setConeOrientation(einfo.coneOrientation);
		}
	}
}

//=====================================================================================//
//                             SoundBuffer::~SoundBuffer()                             //
//=====================================================================================//
SoundBuffer::~SoundBuffer()
{
	InterlockedDecrement(&g_instanceCount);
}

//=====================================================================================//
//                         LockData SoundBuffer::lockBuffer()                          //
//=====================================================================================//
Locker SoundBuffer::lockBuffer()
{
	MUFFLE_SYNCHRONIZED_THIS;

	if(m_buffer)
	{
		LPVOID data1 = 0, data2 = 0;
		DWORD size1 = 0, size2 = 0;
		const int sampleSize = m_format.channels * sizeof(short);

		if(doIsPlaying())
		{
			doUpdateCurrentPosition();

			int lockSize = getCurrentPlayPosition() - m_lastWritePos;
			if(lockSize < 0) lockSize += m_bufferSize;

			if(lockSize)
			{
				SAFE_CALL( m_buffer->Lock(m_lastWritePos * sampleSize,
										lockSize * sampleSize,
										&data1, &size1, &data2, &size2,
										0) );

				m_lastWritePos += lockSize;
				if(m_lastWritePos >= m_bufferSize) m_lastWritePos -= m_bufferSize;
			}
		}
		else
		{
			SAFE_CALL( m_buffer->Lock(0, 0, &data1, &size1, &data2, &size2, DSBLOCK_ENTIREBUFFER) );
			m_lastWritePos = 0;
		}

		Locker result(this);
		result.data1 = reinterpret_cast<short *>(data1);
		result.data2 = reinterpret_cast<short *>(data2);
		result.size1 = size1 / sampleSize;
		result.size2 = size2 / sampleSize;

		return result;
	}
	else
	{
		return Locker();
	}
}

//=====================================================================================//
//                              void SoundBuffer::play()                               //
//=====================================================================================//
void SoundBuffer::play(bool loop)
{
	MUFFLE_SYNCHRONIZED_THIS;

	if(m_buffer)
	{
		SAFE_CALL( m_buffer->Play(0,0,loop ? DSBPLAY_LOOPING : 0) );
	}
}

//=====================================================================================//
//                              void SoundBuffer::stop()                               //
//=====================================================================================//
void SoundBuffer::stop()
{
	MUFFLE_SYNCHRONIZED_THIS;

	if(m_buffer)
	{
		SAFE_CALL( m_buffer->Stop() );
	}
}

//=====================================================================================//
//                            bool SoundBuffer::isPlaying()                            //
//=====================================================================================//
bool SoundBuffer::isPlaying()
{
	MUFFLE_SYNCHRONIZED_THIS;
	if(m_buffer) return doIsPlaying();
	return false;
}

//=====================================================================================//
//                           bool SoundBuffer::doIsPlaying()                           //
//=====================================================================================//
bool SoundBuffer::doIsPlaying()
{
	DWORD status = 0;
	SAFE_CALL( m_buffer->GetStatus(&status) );
	return (status & DSBSTATUS_PLAYING) != 0;
}

//=====================================================================================//
//                      void SoundBuffer::updateCurrentPosition()                      //
//=====================================================================================//
void SoundBuffer::updateCurrentPosition()
{
	MUFFLE_SYNCHRONIZED_THIS;
	if(m_buffer)
	{
		doUpdateCurrentPosition();
	}
}

//=====================================================================================//
//                          void SoundBuffer::unlockBuffer()                           //
//=====================================================================================//
void SoundBuffer::unlockBuffer(const LockData &ldata)
{
	MUFFLE_SYNCHRONIZED_THIS;

	if(m_buffer && (ldata.size1+ldata.size2 > 0))
	{
		const int sampleSize = m_format.channels * sizeof(short);

		LPVOID data1 = reinterpret_cast<void *>(ldata.data1);
		LPVOID data2 = reinterpret_cast<void *>(ldata.data2);
		DWORD size1 = ldata.size1 * sampleSize;
		DWORD size2 = ldata.size2 * sampleSize;

		if(data1 != 0 || data2 != 0)
		{
			SAFE_CALL( m_buffer->Unlock(data1, size1, data2, size2) );
		}
	}
}

//=====================================================================================//
//                            void SoundBuffer::setVolume()                            //
//=====================================================================================//
void SoundBuffer::setVolume(long decibel)
{
	MUFFLE_SYNCHRONIZED_THIS;
	if(m_buffer) m_buffer->SetVolume(decibel);
}

//=====================================================================================//
//                          long SoundBuffer::getFrequency()                           //
//=====================================================================================//
long SoundBuffer::getFrequency() const
{
	MUFFLE_SYNCHRONIZED_THIS;

	if(m_buffer)
	{
		DWORD sizeNeed = 0;
		std::vector< char, SafeAlloc<char> > data;
		SAFE_CALL(m_buffer->GetFormat(NULL,0,&sizeNeed));
		data.resize(sizeNeed);
		SAFE_CALL(m_buffer->GetFormat(reinterpret_cast<LPWAVEFORMATEX>(&data[0]),sizeNeed,&sizeNeed));
		return reinterpret_cast<LPWAVEFORMATEX>(&data[0])->nSamplesPerSec;
	}
	else
	{
		return m_format.rate;
	}
}

//=====================================================================================//
//                          void SoundBuffer::setFrequency()                           //
//=====================================================================================//
void SoundBuffer::setFrequency(long freq)
{
	MUFFLE_SYNCHRONIZED_THIS;
	if(m_buffer) m_buffer->SetFrequency(freq);
}

//=====================================================================================//
//                           void SoundBuffer::setPosition()                           //
//=====================================================================================//
void SoundBuffer::setPosition(const vec &pos)
{
	MUFFLE_SYNCHRONIZED_THIS;
	if(m_buffer3d)
	{
		m_buffer3d->SetPosition(pos.x,pos.y,-pos.z,DS3D_IMMEDIATE);
	}
}

//=====================================================================================//
//                           void SoundBuffer::setVelocity()                           //
//=====================================================================================//
void SoundBuffer::setVelocity(const vec &pos)
{
	MUFFLE_SYNCHRONIZED_THIS;
	if(m_buffer3d)
	{
		m_buffer3d->SetVelocity(pos.x,pos.y,pos.z,DS3D_IMMEDIATE);
	}
}

//=====================================================================================//
//                          void SoundBuffer::setDistances()                           //
//=====================================================================================//
void SoundBuffer::setDistances(float n, float f)
{
	MUFFLE_SYNCHRONIZED_THIS;
	if(m_buffer3d)
	{
		m_buffer3d->SetMinDistance(n,DS3D_IMMEDIATE);
		m_buffer3d->SetMaxDistance(f,DS3D_IMMEDIATE);
	}
}

//=====================================================================================//
//                          void SoundBuffer::setConeParams()                          //
//=====================================================================================//
void SoundBuffer::setConeParams(float min, float max, float outVol)
{
	MUFFLE_SYNCHRONIZED_THIS;
	if(m_buffer3d)
	{
		m_buffer3d->SetConeAngles(static_cast<DWORD>(min),static_cast<DWORD>(max),DS3D_IMMEDIATE);
		m_buffer3d->SetConeOutsideVolume(normToDecibel(outVol),DS3D_IMMEDIATE);
	}
}

//=====================================================================================//
//                       void SoundBuffer::setConeOrientation()                        //
//=====================================================================================//
void SoundBuffer::setConeOrientation(const vec &p)
{
	MUFFLE_SYNCHRONIZED_THIS;
	if(m_buffer3d)
	{
		m_buffer3d->SetConeOrientation(p.x, p.y, p.z, DS3D_IMMEDIATE);
	}
}


//=====================================================================================//
//                     void SoundBuffer::doUpdateCurrentPosition()                     //
//=====================================================================================//
void SoundBuffer::doUpdateCurrentPosition()
{
	DWORD playpos = 0, writepos = 0;

	const int sampleSize = m_format.channels * sizeof(short);
	SAFE_CALL( m_buffer->GetCurrentPosition(&playpos,&writepos) );
	playpos /= sampleSize;
	writepos /= sampleSize;

	m_currentPlayPosition = playpos;
	m_currentWritePosition = writepos;
}

//=====================================================================================//
//                      HSoundBuffer SoundBuffer::createBuffer()                       //
//=====================================================================================//
HSoundBuffer SoundBuffer::createBuffer(size_t size, const Format &fmt, bool is3d,
									   const EmitterInfo &einfo)
{
	float sizeInSeconds = static_cast<float>(size)/static_cast<float>(fmt.rate);
	bool dynamic = sizeInSeconds > MaxStaticBufferLengthInSeconds;
	size_t rsize = size;
	if(dynamic) rsize = static_cast<size_t>(MaxStaticBufferLengthInSeconds * fmt.rate);
	return HSoundBuffer::create(rsize,fmt,is3d,dynamic,einfo);
}

//=====================================================================================//
//                        long SoundBuffer::getInstanceCount()                         //
//=====================================================================================//
long SoundBuffer::getInstanceCount()
{
	return g_instanceCount;
}

}

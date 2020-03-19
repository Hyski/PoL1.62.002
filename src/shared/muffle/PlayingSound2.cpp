#include "precomp.h"

#include "Work2.h"
#include "Fader2.h"
#include "Worker2.h"
#include "Settings2.h"
#include "DecoderMgr2.h"
#include "DecodeBuffer2.h"
#include "PlayingSound2.h"
#include "PlayingSoundMgr2.h"

namespace Muffle2
{

//=====================================================================================//
//                            PlayingSound::PlayingSound()                             //
//=====================================================================================//
PlayingSound::PlayingSound(HDecoder decoder, HScript script, size_t startpos,
						   const EmitterInfo &einfo, bool muted)
:	m_script(script),
	m_wasPlaying(true),
	m_decodedSamples(0),
	m_playedSamples(0),
	m_oldPlayPos(0),
	m_decoderDataSampleCount(decoder->getSampleCount()),
	m_dynamic(false),
	m_decoder(decoder)
{
	m_buffer = SoundBuffer::createBuffer(m_decoderDataSampleCount, m_decoder->getFormat(),script->getInfo().is3d,einfo);
	if(m_buffer->is3d())
	{
		m_buffer->setDistances(script->getInfo().min, script->getInfo().max);
		m_buffer->setConeParams(script->getInfo().minAngle, script->getInfo().maxAngle, script->getInfo().outerVolume);
	}
	m_dynamic = m_buffer->isDynamic();
	m_decoder->seekSample(startpos);
	m_decodedSamples = startpos;
	m_playedSamples = startpos;
	decode(true);
	m_ctrl = BufferCtrlMgr::inst()->createBufferCtrl(einfo.ctrl,m_buffer);
	if(muted) setVolume(0.0f);
	m_buffer->play(m_dynamic || script->getInfo().looped);

	PlayingSoundMgr::inst()->addSound(HPlayingSound(this));
}

//=====================================================================================//
//                            PlayingSound::~PlayingSound()                            //
//=====================================================================================//
PlayingSound::~PlayingSound()
{
}

//=====================================================================================//
//                            void PlayingSound::setFader()                            //
//=====================================================================================//
void PlayingSound::setFader(HFader fader)
{
	MUFFLE_SYNCHRONIZED_THIS;
	m_fader = fader;
}

//=====================================================================================//
//                       WHFader PlayingSound::getFader() const                        //
//=====================================================================================//
WHFader PlayingSound::getFader() const
{
	return m_fader;
}

//=====================================================================================//
//                           bool PlayingSound::isPlaying()                            //
//=====================================================================================//
bool PlayingSound::isPlaying()
{
	MUFFLE_SYNCHRONIZED_THIS;
	if(!m_wasPlaying) return false;

	if(isPlaybackStopped())
	{
		onStop();
		m_wasPlaying = false;
	}
    
	return m_wasPlaying;
}

//=====================================================================================//
//                              void PlayingSound::stop()                              //
//=====================================================================================//
void PlayingSound::stop()
{
	MUFFLE_SYNCHRONIZED_THIS;
	long fade = Settings::SoundFadeTime;
	if(m_script->getInfo().fadeTime > 0.0f)
	{
		fade = static_cast<long>(m_script->getInfo().fadeTime * 1000.0f);
	}
	Fader::createFadeOut(this,fade);
}

//=====================================================================================//
//                            void PlayingSound::badStop()                             //
//=====================================================================================//
void PlayingSound::badStop()
{
	MUFFLE_SYNCHRONIZED_THIS;
	m_buffer->stop();
}

//=====================================================================================//
//                       bool PlayingSound::isPlaybackStopped()                        //
//=====================================================================================//
bool PlayingSound::isPlaybackStopped()
{
	return !m_buffer->isPlaying();
}

//=====================================================================================//
//                             void PlayingSound::decode()                             //
//=====================================================================================//
void PlayingSound::decode(bool force)
{
	MUFFLE_SYNCHRONIZED_THIS;

	if(!isPlaybackStopped() && !m_dynamic)
	{
		return;
	}

	const size_t sampleSize = m_buffer->getFormat().channels * sizeof(short);

	{
		Locker locker = m_buffer->lockBuffer();
		size_t dirtySamplesCount = locker.size1 + locker.size2;
		DecodeBuffer buf(locker, sampleSize);

		while(dirtySamplesCount > 0)
		{
			if(m_decoder)
			{
				long amount = std::min(dirtySamplesCount, m_decoderDataSampleCount - m_decodedSamples);
				m_decoder->decode(amount,&buf);
				dirtySamplesCount -= amount;
				m_decodedSamples += amount;

				if(m_decoder->getSampleCount() == m_decodedSamples)
				{
					if(m_script->getInfo().looped)
					{
						m_decoder->seekSample(0);
						m_decodedSamples = 0;
					}
					else
					{
						m_decoder.reset();
					}
				}
			}
			else
			{
				buf.zero(dirtySamplesCount);
				dirtySamplesCount = 0;
			}
		}
	}

	size_t newPlayPos = m_buffer->getCurrentPlayPosition();
	size_t playedSamples = newPlayPos - m_oldPlayPos;

	if(newPlayPos < m_oldPlayPos)
	{
		playedSamples += m_buffer->getSize();
	}

	m_playedSamples += playedSamples;
	m_oldPlayPos = newPlayPos;

	if(!m_script->getInfo().looped && m_playedSamples >= m_decoderDataSampleCount)
	{
		badStop();
	}
}

//=====================================================================================//
//                             void PlayingSound::onStop()                             //
//=====================================================================================//
void PlayingSound::onStop()
{
}

//=====================================================================================//
//                  HPlayingSound PlayingSound::createPlayingSound()                   //
//=====================================================================================//
HPlayingSound PlayingSound::createPlayingSound(HScript script, const std::string &sound,
											   size_t startpos, const EmitterInfo &einfo,
											   bool muted)
{
	HDecoder decoder = DecoderMgr::createDecoder(sound);
	HPlayingSound result;

	if(decoder)
	{
		result = createPlayingSound(decoder,script,sound,startpos,einfo,muted);
	}
	
	return result;
}

//=====================================================================================//
//                  HPlayingSound PlayingSound::createPlayingSound()                   //
//=====================================================================================//
HPlayingSound PlayingSound::createPlayingSound(HDecoder decoder, HScript script,
											   const std::string &sound, size_t startpos,
											   const EmitterInfo &einfo, bool muted)
{
	HPlayingSound result;
	startpos = std::min(startpos, decoder->getSampleCount());
	//
	if (script->getInfo().is3d && (decoder->getFormat().channels==2))
	{
		MLL_REPORT("Muffle","PlayingSound: попытка создания 3Д звука из стерео.",sound);
	}
	else
	{
		result = HPlayingSound::create(decoder,script,startpos,einfo,muted || script->getInfo().looped);
		if(script->getInfo().looped || startpos != 0)
		{
			long fade = Settings::SoundFadeTime;
			if(script->getInfo().fadeTime > 0.0f)
			{
				fade = static_cast<long>(script->getInfo().fadeTime * 1000.0f);
			}

			Fader::createFadeIn(result,fade);
		}
	}

	return result;
}

}


#include "precomp.h"
#include "Emitter2.h"
#include "MuffleImpl2.h"
#include "DecoderMgr2.h"

namespace Muffle2
{

static MuffleImpl *g_inst = 0;

//=====================================================================================//
//                              MuffleImpl::MuffleImpl()                               //
//=====================================================================================//
MuffleImpl::MuffleImpl(Muffle::SndServices *sndsvc)
:	m_svc(sndsvc),
	m_kernel(sndsvc)
{
	assert( g_inst == 0 );
	g_inst = this;
}

//=====================================================================================//
//                              MuffleImpl::~MuffleImpl()                              //
//=====================================================================================//
MuffleImpl::~MuffleImpl()
{
	assert( g_inst == this );
	g_inst = 0;
}

//=====================================================================================//
//                    Muffle::HEmitter MuffleImpl::createEmitter()                     //
//=====================================================================================//
Muffle::HEmitter MuffleImpl::createEmitter(Channel chan)
{
	return m_kernel.createEmitter(chan);
}

//=====================================================================================//
//                    Muffle::HEmitter MuffleImpl::createEmitter()                     //
//=====================================================================================//
Muffle::HEmitter MuffleImpl::createEmitter(Muffle::HScript script)
{
	return m_kernel.createEmitter(script,std::string());
}

//=====================================================================================//
//                    Muffle::HEmitter MuffleImpl::createEmitter()                     //
//=====================================================================================//
Muffle::HEmitter MuffleImpl::createEmitter(Muffle::HScript script, const std::string& waveFile)
{
	return m_kernel.createEmitter(script,waveFile);
}

//=====================================================================================//
//                            void MuffleImpl::emitSound()                             //
//=====================================================================================//
void MuffleImpl::emitSound(Muffle::HScript script, const std::string &sound_, float startSec)
{
	EmitterInfo einfo;
	std::string sound = sound_.empty() ? script->getInfo().sound : sound_;
	einfo.ctrl = BufferCtrlMgr::inst()->getBufferCtrl(script->getInfo().channel);
	HDecoder decoder = DecoderMgr::createDecoder(sound);

	if(decoder)
	{
		size_t start = static_cast<size_t>(startSec * decoder->getFormat().rate);
		PlayingSound::createPlayingSound(decoder, script, sound, start, einfo);
	}
}

//=====================================================================================//
//                            void MuffleImpl::emitSound()                             //
//=====================================================================================//
void MuffleImpl::emitSound(Muffle::HScript script, float startSec)
{
	EmitterInfo einfo;
	einfo.ctrl = BufferCtrlMgr::inst()->getBufferCtrl(script->getInfo().channel);
	HDecoder decoder = DecoderMgr::createDecoder(script->getInfo().sound);

	if(decoder)
	{
		size_t start = static_cast<size_t>(startSec * decoder->getFormat().rate);
		PlayingSound::createPlayingSound(decoder, script, script->getInfo().sound, start, einfo);
	}
}

//=====================================================================================//
//                       Muffle::HScript MuffleImpl::getScript()                       //
//=====================================================================================//
Muffle::HScript MuffleImpl::getScript(const std::string &scriptName)
{
	return m_kernel.getScript(scriptName);
}

//=====================================================================================//
//                      Muffle::HScript MuffleImpl::embedScript()                      //
//=====================================================================================//
Muffle::HScript MuffleImpl::embedScript(const Muffle::ScriptInfo &info)
{
	return m_kernel.embedScript(info);
}

//=====================================================================================//
//                            void MuffleImpl::setVolume()                             //
//=====================================================================================//
void MuffleImpl::setVolume(Channel chan, float volume)
{
	BufferCtrlMgr::inst()->getBufferCtrl(chan)->setVolume(volume);
}

//=====================================================================================//
//                           void MuffleImpl::setFrequency()                           //
//=====================================================================================//
void MuffleImpl::setFrequency(Channel chan, float factor)
{
	BufferCtrlMgr::inst()->getBufferCtrl(chan)->setFrequencyFactor(factor);
}

//=====================================================================================//
//                           void MuffleImpl::muteChannel()                            //
//=====================================================================================//
void MuffleImpl::muteChannel(Channel chan, bool m)
{
	BufferCtrlMgr::inst()->getBufferCtrl(chan)->mute(m);
}

//=====================================================================================//
//                            float MuffleImpl::getVolume()                            //
//=====================================================================================//
float MuffleImpl::getVolume(Channel chan) const
{
	return BufferCtrlMgr::inst()->getBufferCtrl(chan)->getVolume();
}

//=====================================================================================//
//                        float MuffleImpl::getSoundDuration()                         //
//=====================================================================================//
float MuffleImpl::getSoundDuration(const std::string &sound)
{
	HDecoder decoder = DecoderMgr::createDecoder(sound);
	if(decoder)
	{
		return static_cast<float>(decoder->getSampleCount()) / static_cast<float>(decoder->getFormat().rate);
	}
	return 0.0f;
}

//=====================================================================================//
//                            void MuffleImpl::setCamera()                             //
//=====================================================================================//
void MuffleImpl::setCamera(const vec &dir, const vec &up, const vec &right, const vec &origin)
{
	m_kernel.setCamera(dir,up,right,origin);
}

//=====================================================================================//
//                              void MuffleImpl::manage()                              //
//=====================================================================================//
void MuffleImpl::manage()
{
	{
		std::ostringstream sstr;
		sstr << "PlayingSound count: " << PlayingSoundMgr::inst()->getPlayingSoundCount();
		m_svc->dbg_printf(200,200,sstr.str().c_str());
	}
	{
		std::ostringstream sstr;
		sstr << "Synchronized count: " << Synchronized::getInstanceCount();
		m_svc->dbg_printf(200,220,sstr.str().c_str());
	}
	{
		std::ostringstream sstr;
		sstr << "Work count: " << Work::getInstanceCount();
		m_svc->dbg_printf(200,240,sstr.str().c_str());
	}
	{
		std::ostringstream sstr;
		sstr << "SoundBuffer count: " << SoundBuffer::getInstanceCount();
		m_svc->dbg_printf(200,260,sstr.str().c_str());
	}
	{
		std::ostringstream sstr;
		sstr << "BufferCtrl count: " << BufferCtrl::getInstanceCount();
		m_svc->dbg_printf(200,280,sstr.str().c_str());
	}
	{
		std::ostringstream sstr;
		sstr << "Decoder count: " << Decoder::getInstanceCount();
		m_svc->dbg_printf(200,300,sstr.str().c_str());
	}
	{
		std::ostringstream sstr;
		sstr << "Emitter count: " << Emitter::getInstanceCount();
		m_svc->dbg_printf(200,320,sstr.str().c_str());
	}
}

//=====================================================================================//
//                           MuffleImpl *MuffleImpl::inst()                            //
//=====================================================================================//
MuffleImpl *MuffleImpl::inst()
{
	return g_inst;
}

}


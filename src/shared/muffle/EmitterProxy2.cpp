#include "precomp.h"
#include "EmitterProxy2.h"

namespace Muffle2
{

//=====================================================================================//
//                            EmitterProxy::EmitterProxy()                             //
//=====================================================================================//
EmitterProxy::EmitterProxy(HScript script, const std::string &sound, HEmitterImpl em)
:	m_defaultScript(script),
	m_defaultSound(sound.empty() ? m_defaultScript->getInfo().sound : sound),
	m_emitter(em)
{
}

//=====================================================================================//
//                              void EmitterProxy::play()                              //
//=====================================================================================//
int EmitterProxy::play()
{
	return m_emitter->play(m_defaultScript,m_defaultSound,0);
}

//=====================================================================================//
//                             void EmitterProxy::play2()                              //
//=====================================================================================//
int EmitterProxy::play(HScript script, const std::string &sound, float startpos)
{
	return m_emitter->play(script,sound,0);
}

}

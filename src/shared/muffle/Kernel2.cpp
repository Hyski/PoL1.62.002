#include "precomp.h"
#include "Kernel2.h"
#include "Emitter2.h"
#include "EmitterProxy2.h"
#include "ThemeSessionProxy2.h"
#include <mll/algebra/matrix3.h>
#include <mll/algebra/quaternion.h>

namespace Muffle2
{

static Kernel *g_inst = 0;

//=====================================================================================//
//                                  Kernel::Kernel()                                   //
//=====================================================================================//
Kernel::Kernel(Muffle::SndServices *ss)
:	m_ss(ss),
	m_directSound(DSDEVID_DefaultPlayback,ss->window()),
	m_primaryBuffer(m_directSound),
	m_fileMgmt(Ptr<Muffle::FileMgmt>::create(ss->fs())),
	m_scriptMgr(ss),
	m_psmgr(HPlayingSoundMgr::create())
{
	assert( g_inst == 0 );
	g_inst = this;

	SAFE_CALL( m_primaryBuffer.getListener()->SetRolloffFactor(5.0f,DS3D_IMMEDIATE) );
}

//=====================================================================================//
//                                  Kernel::~Kernel()                                  //
//=====================================================================================//
Kernel::~Kernel()
{
	assert( g_inst == this );
	g_inst = 0;

	m_psmgr->finish();
	m_worker.finish();
}

//=====================================================================================//
//                    Muffle::HThemeSession Kernel::beginSession()                     //
//=====================================================================================//
Muffle::HThemeSession Kernel::beginSession()
{
	MUFFLE_SYNCHRONIZED_THIS;

	HThemeSession sess = m_themeMgr.createSession();
	Muffle::HThemeSession result(new ThemeSessionProxy(sess));
	return result;
}

//=====================================================================================//
//                         Muffle::HScript Kernel::getScript()                         //
//=====================================================================================//
Muffle::HScript Kernel::getScript(const std::string &name)
{
	MUFFLE_SYNCHRONIZED_THIS;

	Muffle::HScript script = m_scriptMgr.get(name);
	if(!script)
	{
		Muffle::ScriptInfo info;
		info.name = name;
		script = m_scriptMgr.embedScript(info);
		MLL_REPORT_ERROR("Muffle","Отсутствует звуковой скрипт",name);
	}

	return script;
}

//=====================================================================================//
//                        Muffle::HScript Kernel::embedScript()                        //
//=====================================================================================//
Muffle::HScript Kernel::embedScript(const Muffle::ScriptInfo &info)
{
	MUFFLE_SYNCHRONIZED_THIS;

	return m_scriptMgr.embedScript(info);
}

//=====================================================================================//
//                              void Kernel::setCamera()                               //
//=====================================================================================//
void Kernel::setCamera(const vec &dir, const vec &up, const vec &right, const vec &origin)
{
	MUFFLE_SYNCHRONIZED_THIS;

	IDirectSound3DListener8 *listener = m_primaryBuffer.getListener();
	vec ndir = vec(dir.x,dir.y,-dir.z);
	vec nup = vec(up.x,up.y,-up.z);
	vec norigin = vec(origin.x,origin.y,-origin.z);

	listener->SetPosition(norigin.x, norigin.y, norigin.z, DS3D_DEFERRED);
	listener->SetOrientation(ndir.x, ndir.y, ndir.z, nup.x, nup.y, nup.z, DS3D_DEFERRED);
	listener->CommitDeferredSettings();
}

//=====================================================================================//
//                      Muffle::HEmitter Kernel::createEmitter()                       //
//=====================================================================================//
Muffle::HEmitter Kernel::createEmitter(Muffle::ISound::Channel chan)
{
	MUFFLE_SYNCHRONIZED_THIS;

	HEmitterImpl em(HEmitterImpl::create(chan));
	Muffle::HEmitter result(new EmitterProxy(0,std::string(),em));
	return result;
}

//=====================================================================================//
//                      Muffle::HEmitter Kernel::createEmitter()                       //
//=====================================================================================//
Muffle::HEmitter Kernel::createEmitter(Muffle::HScript script, const std::string &file)
{
	MUFFLE_SYNCHRONIZED_THIS;

	Muffle::ISound::Channel chan = script->getInfo().channel;
	HEmitterImpl em(HEmitterImpl::create(chan));
	Muffle::HEmitter result(new EmitterProxy(script,file,em));
	return result;
}

//=====================================================================================//
//                               Kernel *Kernel::inst()                                //
//=====================================================================================//
Kernel *Kernel::inst()
{
	return g_inst;
}

}

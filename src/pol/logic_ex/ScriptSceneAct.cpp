#include "precomp.h"

#include "SavedData.h"
#include "ScriptSceneAct.h"
#include <undercover/Interface/MenuMgr.h>
#include <undercover/Interface/LoadingScreen.h>

namespace PoL
{

//=====================================================================================//
//                          ScriptSceneAct::ScriptSceneAct()                           //
//=====================================================================================//
ScriptSceneAct::ScriptSceneAct(HAct act)
:	m_started(false),
	m_ended(false),
	m_endable(act->isEndable()),
	m_act(act)
{
}

//=====================================================================================//
//                          ScriptSceneAct::~ScriptSceneAct()                          //
//=====================================================================================//
ScriptSceneAct::~ScriptSceneAct()
{
}

//=====================================================================================//
//                             void ScriptSceneAct::skip()                             //
//=====================================================================================//
void ScriptSceneAct::skip()
{
	assert( isStarted() );
	assert( !isEnded() );

	m_act->skip();

	m_started = false;
}

//=====================================================================================//
//                        bool ScriptSceneAct::canStart() const                        //
//=====================================================================================//
bool ScriptSceneAct::canStart() const
{
	Widget *ls = MenuMgr::Instance()->Child(LoadingScreen::m_pDialogName);
	if(ls && ls->IsVisible()) return false;
	return true;
}

//=====================================================================================//
//                           void ScriptSceneAct::doStart()                            //
//=====================================================================================//
void ScriptSceneAct::doStart()
{
	m_act->start();
	m_savedData.reset(new SavedData);
}

//=====================================================================================//
//                            void ScriptSceneAct::start()                             //
//=====================================================================================//
void ScriptSceneAct::start()
{
	assert( !isStarted() );
	m_started = true;

	if(canStart())
	{
		doStart();
	}
}

//=====================================================================================//
//                            void ScriptSceneAct::update()                            //
//=====================================================================================//
void ScriptSceneAct::update(float tau)
{
	assert( isStarted() );
	assert( !isEnded() );

	if(m_savedData.get())
	{
		if(!m_act->isEnded()) m_act->update(tau);
		if(m_act->isEnded())
		{
			m_ended = true;
			m_savedData.reset();
		}
	}
	else if(canStart())
	{
		doStart();
	}
}

}
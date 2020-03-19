#include "precomp.h"

#include "LooseGameAct.h"
#include <undercover/Game.h>
#include "UnlimitedMovesMgr.h"
#include <undercover/interface/MenuMgr.h>
#include <undercover/interface/GameScreen.h>
#include <undercover/interface/loadingscreen.h>

namespace PoL
{

//=====================================================================================//
//                            LooseGameAct::LooseGameAct()                             //
//=====================================================================================//
LooseGameAct::LooseGameAct()
:	m_started(false),
	m_ended(false)
{
}

//=====================================================================================//
//                             void LooseGameAct::start()                              //
//=====================================================================================//
void LooseGameAct::start()
{
	m_started = true;
}

//=====================================================================================//
//                             void LooseGameAct::update()                             //
//=====================================================================================//
void LooseGameAct::update(float)
{
	assert( !m_ended );
	assert( m_started );

	UnlimitedMovesMgr::instance()->looseGame();

	Widget *gms =::MenuMgr::Instance()->Child( ::GameScreen::m_pDialogName);
	Widget *hms = gms ? gms->Child( ::HiddenMovementScreen::m_pDialogName) : 0;

	::Game::SetGameLevelRender(true);

	if(hms)
	{
		hms->SetVisible(false);
		::DirtyLinks::EnableLevelAndEffectSound(true);
	}

	::MenuMgr::MouseCursor()->SetVisible(true);

	Forms::GetInst()->ShowOptionsDialog(Forms::ODM_LOADMENU);

	m_ended = true;
}

}

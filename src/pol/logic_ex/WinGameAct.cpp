#include "precomp.h"

#include "WinGameAct.h"
#include "UnlimitedMovesMgr.h"
#include <undercover/Interface/MenuMgr.h>
#include <undercover/Interface/MainMenu.h>

namespace PoL
{

//=====================================================================================//
//                              WinGameAct::WinGameAct()                               //
//=====================================================================================//
WinGameAct::WinGameAct()
:	m_started(false),
	m_ended(false),
	m_phase(0)
{
}

//=====================================================================================//
//                              void WinGameAct::start()                               //
//=====================================================================================//
void WinGameAct::start()
{
	m_started = true;
}

//=====================================================================================//
//                              void WinGameAct::update()                              //
//=====================================================================================//
void WinGameAct::update(float)
{
	assert( !m_ended );
	assert( m_started );

	switch(m_phase++)
	{
	case 0:
		Forms::GetInst()->ShowMainMenuForm();
		break;
	case 1:
		::MenuMgr::Instance()->ShowDialog(CreditsMenu::m_pDialogName);
		UnlimitedMovesMgr::instance()->looseGame();
		m_ended = true;
		break;
	}
}

}
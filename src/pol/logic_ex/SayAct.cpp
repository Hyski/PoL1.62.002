#include "precomp.h"

#include "SayAct.h"
#include "QuestContext.h"
#include "TaskDatabase.h"
#include <undercover/Interface/MenuMgr.h>
#include <undercover/Interface/GameScreen.h>
#include <undercover/Interface/LoadingScreen.h>
#include <undercover/Game.h>

namespace PoL
{

//=====================================================================================//
//                                  SayAct::SayAct()                                   //
//=====================================================================================//
SayAct::SayAct(const std::string &who, const std::string &strid)
:	m_started(false),
	m_ended(false),
	m_who(who),
	m_strid(strid)
{
}

//=====================================================================================//
//                               void PrintAct::start()                                //
//=====================================================================================//
void SayAct::start()
{
	m_started = true;
}

//=====================================================================================//
//                               void PrintAct::update()                               //
//=====================================================================================//
void SayAct::update(float)
{
	assert( m_started );
	assert( !m_ended );

	if(!m_strid.empty())
	{
		BaseEntity *entity = 0;

		if(!m_who.empty())
		{
			entity = getEntityByName(m_who);
		}
		else
		{
			entity = getEntityByName(QuestContext::getHolder());
		}

		if(entity)
		{
			const bool mcv = ::MenuMgr::MouseCursor()->IsVisible();
			const bool vis = entity->GetGraph()->IsVisible();

			Widget *gms =::MenuMgr::Instance()->Child( ::GameScreen::m_pDialogName);
			Widget *hms = gms ? gms->Child( ::HiddenMovementScreen::m_pDialogName) : 0;

			bool hiddenvisible = true;
			bool gamelevelrender = ::Game::RenderEnabled();

			::Game::SetGameLevelRender(true);

			if(hms)
			{
				hiddenvisible = hms->IsVisible();
				hms->SetVisible(false);
				::DirtyLinks::EnableLevelAndEffectSound(true);
			}

			if(!mcv) ::MenuMgr::MouseCursor()->SetVisible(true);
			if(!vis) entity->GetGraph()->Visible(true);
			::Forms::GetInst()->ShowTalkDialog(entity,locPhr(m_strid));
			if(!mcv) ::MenuMgr::MouseCursor()->SetVisible(mcv);
			if(!vis) entity->GetGraph()->Visible(vis);

			if(hms)
			{
				hms->SetVisible(hiddenvisible);
				::DirtyLinks::EnableLevelAndEffectSound(!hiddenvisible);
			}

			::Game::SetGameLevelRender(gamelevelrender);

			TaskDatabase::instance()->addToSaidList(entity->GetEID());
		}
	}

	m_ended = true;
}

}

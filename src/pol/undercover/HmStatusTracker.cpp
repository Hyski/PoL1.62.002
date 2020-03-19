#include "precomp.h"

#include "Game.h"
#include "HmStatusTracker.h"
#include "Interface/MenuMgr.h"
#include "Interface/GameScreen.h"
#include "Interface/LoadingScreen.h"

namespace PoL
{

//=====================================================================================//
//                         HmStatusTracker::HmStatusTracker()                          //
//=====================================================================================//
HmStatusTracker::HmStatusTracker()
:	m_left(-1),
	m_top(-1),
	m_right(-1),
	m_bottom(-1),
	m_oldvis(false)
{
	Widget *gms = MenuMgr::Instance()->Child(GameScreen::m_pDialogName);
	Widget *hms = gms ? gms->Child(HiddenMovementScreen::m_pDialogName) : 0;
	Widget *hmsb = hms ? hms->Child("background") : 0;
	Widget *hmsp = hmsb ? hmsb->Child("progress") : 0;

	m_oldvis = hms && hms->IsVisible();

	if(hmsp)
	{
		hmsp->GetLocation()->Region()->GetRect(&m_left,&m_top,&m_right,&m_bottom);
	}
}

//=====================================================================================//
//                       bool HmStatusTracker::needActivities()                        //
//=====================================================================================//
bool HmStatusTracker::needActivities()
{
	Widget *gms = MenuMgr::Instance()->Child(GameScreen::m_pDialogName);
	Widget *hms = gms ? gms->Child(HiddenMovementScreen::m_pDialogName) : 0;
	Widget *hmsb = hms ? hms->Child("background") : 0;
	Widget *hmsp = hmsb ? hmsb->Child("progress") : 0;

	bool needrender = !hms || !hms->IsVisible();

	if(needrender == m_oldvis)
	{
		m_oldvis = !needrender;
		needrender = true;
	}

	if(!needrender)
	{
		if(hmsp)
		{
			int nleft = -1, ntop = -1, nright = -1, nbottom = -1;
			hmsp->GetLocation()->Region()->GetRect(&nleft,&ntop,&nright,&nbottom);

			if(nleft != m_left || ntop != m_top || nright != m_right || nbottom != m_bottom)
			{
				m_left = nleft;
				m_top = ntop;
				m_right = nright;
				m_bottom = nbottom;
				needrender = true;
			}
		}
	}

	//needrender = needrender || Game::RenderEnabled();

	return needrender;
}

}

/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   13.03.2001

************************************************************************/
#include "Precomp.h"
#include "DescriptionScreen.h"
#include "InventoryScreen.h"
#include "OptionsScreen.h"
#include "LoadingScreen.h"
#include "JournalScreen.h"
#include "WidgetFactory.h"
#include "DialogScreen.h"
#include "TradeScreen.h"
#include "MouseCursor.h"
#include "GameScreen.h"
#include "MenuMgr.h"
#include "Screens.h"

Screens::Deleter Screens::m_Deleter;

Screens::Screens()
{
}

Screens::~Screens()
{
}

//	�������� ��������� � ������� �������
GameScreen*	Screens::Game(void)
{
	return (GameScreen *)WidgetFactory::Instance()->Assert(MenuMgr::Instance(),GameScreen::m_pDialogName);
}
//	�������� ��������� � ������� ������������
InventoryScreen* Screens::Inventory(void)
{
	return (InventoryScreen *) WidgetFactory::Instance()->Assert(WidgetFactory::Instance()->Assert(MenuMgr::Instance(),GameScreen::m_pDialogName),InventoryScreen::m_pDialogName);
}
//	�������� ��������� � ������� ��������
TradeScreen* Screens::Trade(void)
{
	return (TradeScreen *) WidgetFactory::Instance()->Assert(WidgetFactory::Instance()->Assert(MenuMgr::Instance(),GameScreen::m_pDialogName),TradeScreen::m_pDialogName);
}
//	�������� ��������� � ������� �������
JournalScreen* Screens::Journal(void)
{
	return (JournalScreen *) WidgetFactory::Instance()->Assert(WidgetFactory::Instance()->Assert(MenuMgr::Instance(),GameScreen::m_pDialogName),JournalScreen::m_pDialogName);
}
//	�������� ��������� � ������� �������
DialogScreen* Screens::Dialog(void)
{
	return (DialogScreen *) WidgetFactory::Instance()->Assert(WidgetFactory::Instance()->Assert(MenuMgr::Instance(),GameScreen::m_pDialogName),DialogScreen::m_pDialogName);
}
//	�������� ��������� � ������� �������� �����
HeroInfoScreen* Screens::HeroInfo(void)
{
	return (HeroInfoScreen *) WidgetFactory::Instance()->Assert(WidgetFactory::Instance()->Assert(MenuMgr::Instance(),GameScreen::m_pDialogName),HeroInfoScreen::m_pDialogName);
}
//	�������� ��������� � ������� ����� ������ ����
OptionsScreen* Screens::Options(void)
{
	return (OptionsScreen *) WidgetFactory::Instance()->Assert(WidgetFactory::Instance()->Assert(MenuMgr::Instance(),GameScreen::m_pDialogName),OptionsScreen::m_pDialogName);
}
//	�������� ��������� � ������� ������� �����������
HiddenMovementScreen* Screens::HiddenMovement(void)
{
	return (HiddenMovementScreen *) WidgetFactory::Instance()->Assert(WidgetFactory::Instance()->Assert(MenuMgr::Instance(),GameScreen::m_pDialogName),HiddenMovementScreen::m_pDialogName);
}
//	�������� ��������� � ������� ������������ ������
PanicScreen* Screens::Panic(void)
{
	return (PanicScreen *) WidgetFactory::Instance()->Assert(WidgetFactory::Instance()->Assert(MenuMgr::Instance(),GameScreen::m_pDialogName),PanicScreen::m_pDialogName);
}
//	�������� ��������� � ������� �������� ��������
DescriptionScreen* Screens::Description(void)
{
	return (DescriptionScreen *) WidgetFactory::Instance()->Assert(WidgetFactory::Instance()->Assert(MenuMgr::Instance(),GameScreen::m_pDialogName),DescriptionScreen::m_pDialogName);
}
//	�������� ��������� � ������� ���������
HelperScreen* Screens::Helper(void)
{
	return (HelperScreen *) WidgetFactory::Instance()->Assert(WidgetFactory::Instance()->Assert(MenuMgr::Instance(),GameScreen::m_pDialogName),HelperScreen::m_pDialogName);
}
//	�������� ��������� � ������� ��������� �����
FlagDescScreen* Screens::FlagDesc(void)
{
	return (FlagDescScreen *) WidgetFactory::Instance()->Assert(WidgetFactory::Instance()->Assert(MenuMgr::Instance(),GameScreen::m_pDialogName),FlagDescScreen::m_pDialogName);
}

//	�������� ��������� � ������� �������
Widget*	Screens::HasGame(void)
{
	return MenuMgr::Instance()->Child(GameScreen::m_pDialogName);
}
//	�������� ��������� � ������� ������������
Widget* Screens::HasInventory(void)
{
	if(Widget *game = HasGame())
	{
		return game->Child(InventoryScreen::m_pDialogName);
	}

	return 0;
}
//	�������� ��������� � ������� ��������
Widget* Screens::HasTrade(void)
{
	if(Widget *game = HasGame())
	{
		return game->Child(TradeScreen::m_pDialogName);
	}

	return 0;
}
//	�������� ��������� � ������� �������
Widget* Screens::HasJournal(void)
{
	if(Widget *game = HasGame())
	{
		return game->Child(JournalScreen::m_pDialogName);
	}

	return 0;
}
//	�������� ��������� � ������� �������
Widget* Screens::HasDialog(void)
{
	if(Widget *game = HasGame())
	{
		return game->Child(DialogScreen::m_pDialogName);
	}

	return 0;
}
//	�������� ��������� � ������� �������� �����
Widget* Screens::HasHeroInfo(void)
{
	if(Widget *game = HasGame())
	{
		return game->Child(HeroInfoScreen::m_pDialogName);
	}

	return 0;
}
//	�������� ��������� � ������� ����� ������ ����
Widget* Screens::HasOptions(void)
{
	if(Widget *game = HasGame())
	{
		return game->Child(OptionsScreen::m_pDialogName);
	}

	return 0;
}
//	�������� ��������� � ������� ������� �����������
Widget* Screens::HasHiddenMovement(void)
{
	if(Widget *game = HasGame())
	{
		return game->Child(HiddenMovementScreen::m_pDialogName);
	}

	return 0;
}
//	�������� ��������� � ������� ������������ ������
Widget* Screens::HasPanic(void)
{
	if(Widget *game = HasGame())
	{
		return game->Child(PanicScreen::m_pDialogName);
	}

	return 0;
}
//	�������� ��������� � ������� �������� ��������
Widget* Screens::HasDescription(void)
{
	if(Widget *game = HasGame())
	{
		return game->Child(DescriptionScreen::m_pDialogName);
	}

	return 0;
}
//	�������� ��������� � ������� ���������
Widget* Screens::HasHelper(void)
{
	if(Widget *game = HasGame())
	{
		return game->Child(HelperScreen::m_pDialogName);
	}

	return 0;
}
//	�������� ��������� � ������� ��������� �����
Widget* Screens::HasFlagDesc(void)
{
	if(Widget *game = HasGame())
	{
		return game->Child(FlagDescScreen::m_pDialogName);
	}

	return 0;
}

//	�������������� ������������ �����
void Screens::Activate(SCREEN_ID id)
{
	GameScreen* pGameScreen = static_cast<GameScreen*>(WidgetFactory::Instance()->Assert(MenuMgr::Instance(),GameScreen::m_pDialogName));

	switch(id)
	{
	case SID_GAME:
		//	���������� ������
		MouseCursor::SetVisible(true);
		pGameScreen->UnloadOptionsScreen();
		pGameScreen->ShowDialog(0);
		break;
	case SID_INVENTORY: 
		pGameScreen->ShowDialog(InventoryScreen::m_pDialogName);
		break;
	case SID_TRADE: 
		pGameScreen->ShowDialog(TradeScreen::m_pDialogName);
		break;
	case SID_JOURNAL: 
		pGameScreen->ShowDialog(JournalScreen::m_pDialogName);
		break;
	case SID_DIALOG: 
		pGameScreen->ShowDialog(DialogScreen::m_pDialogName);
		break;
	case SID_HEROINFO: 
		pGameScreen->ShowDialog(HeroInfoScreen::m_pDialogName);
		break;
	case SID_HELPER: 
		pGameScreen->ShowDialog(HelperScreen::m_pDialogName);
		break;
	case SID_OPTIONS:
		//	������ �������� ����� �����
		pGameScreen->LoadOptionsScreen();
		pGameScreen->ShowDialog(OptionsScreen::m_pDialogName);
		break;
	case SID_HIDDENMOVEMENT: 
		//	������ ������
		MouseCursor::SetVisible(false);
		pGameScreen->ShowDialog(HiddenMovementScreen::m_pDialogName);
		break;
	case SID_PANIC: 
		//	������ ������
		MouseCursor::SetVisible(false);
		pGameScreen->ShowDialog(PanicScreen::m_pDialogName);
		break;
	case SID_DESCRIPTION: 
		pGameScreen->ShowDialog(DescriptionScreen::m_pDialogName);
		break;
	}
}
//	����� � ������� ����
void Screens::ExitToMainMenu(void)
{
	Game()->ExitToMainMenu();
}
//	��������� ������� ������ �����
bool Screens::IsOptionsPresent(void)
{
	return Game()->Child(OptionsScreen::m_pDialogName)?true:false;
}


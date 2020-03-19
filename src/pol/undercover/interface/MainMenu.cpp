/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   02.03.2001

************************************************************************/
#include "Precomp.h"
#include "MainMenu.h"

#include <common/DataMgr/TxtFile.h>
#include <common/CmdLine/CmdLine.h>
#include <common/FontMgr/Font.h>
#include <common/Shell/Shell.h>
#include <common/UI/Button.h>
#include <undercover/options/Options.h>
#include <Muffle/ISound.h>
#include "OptionsScreen.h"
#include "WidgetFactory.h"
#include "MouseCursor.h"
#include "DXImageMgr.h"
#include "Interface.h"
#include "TextBox.h"
#include "MenuMgr.h"
#include "Text.h"
#include "../PlayedVideoMgr.h"
#include <common/D3DApp/Input/Input.h>

//	��� ��������� ��������
#include <logic2/logictypes.h>
#include <logic2/DirtyLinks.h>

//********************************************************************//
//*******************      class MainMenu     ************************//
const char* MainMenu::m_pDialogName = "MainMenu";
MainMenu::MainMenu() : Dialog(m_pDialogName,m_pDialogName)
{
	Focus()->Set(WidgetFocus::FF_KEYBOARD,Child("game"));
	m_MenuEvent = AE_NONE;

#ifdef DEMO_VERSION

	WidgetFactory::Instance()->Assert(this,"cinematic")->SetVisible(false);
	WidgetFactory::Instance()->Assert(this,"credits")->SetVisible(false);

#endif	//	DEMO_VERSION

}

MainMenu::~MainMenu()
{
}

void MainMenu::OnButton(Button* pButton)
{
	if(!strcmp(pButton->Name(),"game"))
	{

#if !defined(DEMO_VERSION) && !defined(ML_ONLINE_DISTRIBUTION)
		if(!CmdLine::IsKey("-master") && !CheckCDVolume()) return;
#endif

		m_MenuEvent = AE_GAME;
	}
	else
		if(!strcmp(pButton->Name(),"options")) m_MenuEvent = AE_OPTIONS;
		else
			if(!strcmp(pButton->Name(),"cinematic")) m_MenuEvent = AE_CINEMATICS;
			else
				if(!strcmp(pButton->Name(),"credits")) m_MenuEvent = AE_CREDITS;
				else
					if(!strcmp(pButton->Name(),"exit")) m_MenuEvent = AE_EXIT;
	if(Parent()) Parent()->OnChange(this);
}
//	�������� ����� CD
bool MainMenu::CheckCDVolume(void)
{
	char file_system_name_buffer[MAX_PATH];
	char volume_name_buffer[MAX_PATH];
	DWORD maximum_component_length;
	DWORD volume_serial_number;
	DWORD file_system_flags;
	UINT drive_type;
	bool complete = false;

	std::string mark;

	mark += "P";
	mark += "O";
	mark += "L";
	mark += "_";
	mark += "C";
	mark += "E";

	std::transform(mark.begin(),mark.end(),mark.begin(),std::tolower);

	do
	{
		for(int i=3;i<27;i++)
		{
			std::ostringstream path_str;
			path_str << (char)('a'+i) << ":\\";
			drive_type = GetDriveType(path_str.str().c_str());

			if(drive_type == DRIVE_CDROM)
			{
				if(GetVolumeInformation(path_str.str().c_str(),
					volume_name_buffer,MAX_PATH,
					&volume_serial_number,&maximum_component_length,
					&file_system_flags,
					file_system_name_buffer,MAX_PATH))
				{
					std::string volume_name = volume_name_buffer;
					std::transform(volume_name.begin(),volume_name.end(),volume_name.begin(),std::tolower);
					if(mark == volume_name)
					{
						complete = true;
						break;
					}
				}
			}
		}
	}
	while(!complete && MenuMgr::Instance()->QuestionBox(MenuMgr::Instance()->GetStrResourse("mainmenu_insert_cd").c_str()));

	return complete;
}

/////////////////////////////////////////////////////////////////////////
////////////////////////    class VideoMenuImp    ///////////////////////
/////////////////////////////////////////////////////////////////////////
class VideoMenuImp : public Dialog
{
public:

	/////////////////////////////////////////////////////////////////////////
	//////////////////////////    class Observer    /////////////////////////
	/////////////////////////////////////////////////////////////////////////
	class Observer
	{
	public:
		virtual ~Observer() {}
		virtual void OnClose(void) = 0;
	};
	/////////////////////////////////////////////////////////////////////////

public:
	static const char* m_pDialogName;
	std::vector<PoL::VideoEntry> m_videos;
private:
	enum {MAX_SLOT = 8};
private:
	Observer* m_Observer;
	Text* m_Slots[MAX_SLOT];
	Widget* m_Selection;
	int m_SelPos;
public:
	VideoMenuImp();
	~VideoMenuImp();
public:
	//	���������� ����������
	void SetObserver(Observer* ob) {m_Observer = ob;}
private:
	//	����������� �� ������
	void OnButton(Button* pButton);
	//	�������� �������� ������
	const char* GetFilmDesc(unsigned int index);
	//	��������� ��������� �����
	bool ProcessMouseInput(VMouseState* pMS);
	//	��������� ������������� �����
	bool ProcessKeyboardInput(VKeyboardState* pKS);
	//	��������� ��������� ��� �����
	void SetSelectionToMouse(void);
	//	�������� ��������
	void UpdateSelection(void);
	//	���������� �������� �����
	void TryShowFilm(void);
	//	����������� �� ��������� ��������� �������
	void OnSystemChange(void) {Widget::OnSystemChange();}
};

const char* VideoMenuImp::m_pDialogName = "VideoMenuImp";

VideoMenuImp::VideoMenuImp() : Dialog(m_pDialogName,m_pDialogName),m_Observer(0),m_SelPos(0)
{
	char buff[20];
	int left,top,right,bottom;

	m_videos = PoL::PlayedVideoMgr::getPlayedVideos();

	VRegion slot_rgn = *WidgetFactory::Instance()->Assert(this,"slot")->GetLocation()->Region();
	for(int i=0;i<MAX_SLOT;i++)
	{
		Widget::Insert(m_Slots[i] = new Text(itoa(i,buff,10)));
		slot_rgn.GetRect(&left,&top,&right,&bottom);
		m_Slots[i]->SetFont("msg_s",0x69B1AF);
		m_Slots[i]->SetRegion(left+5,top,right-3,bottom);
		m_Slots[i]->Align(LogicalText::T_VCENTER);
		if(m_videos.size() > i)
		{
			m_Slots[i]->SetText(DirtyLinks::GetStrRes(m_videos[i].m_description).c_str());
		}
		//if(CheckEpisode(episode,i)) m_Slots[i]->SetText(GetFilmDesc(i));
		slot_rgn.Transform(1,0,0,0,1,5+slot_rgn.Height());
	}
	//	���������
	m_Selection = WidgetFactory::Instance()->Assert(this,"selection");
	BringToTop(m_Selection->Name());
	//	���������� ������������� ������
	WidgetFactory::Instance()->Assert(this,"ok")->Focus()->Enable(WidgetFocus::FF_KEYBOARD,false);
	WidgetFactory::Instance()->Assert(this,"cancel")->Focus()->Enable(WidgetFocus::FF_KEYBOARD,false);
}

VideoMenuImp::~VideoMenuImp()
{
}
//	����������� �� ������
void VideoMenuImp::OnButton(Button* pButton)
{
	if(!strcmp(pButton->Name(),"ok"))
	{
		TryShowFilm();
		return;
	}
	if(!strcmp(pButton->Name(),"cancel"))
	{
		if(m_Observer) m_Observer->OnClose();
		return;
	}
}
////	�������� �������� ������
//const char* VideoMenuImp::GetFilmDesc(unsigned int index)
//{
//	static std::string value;
//	value = DirtyLinks::GetStrRes(m_FilmDescId[index][0]).c_str();
//	return value.c_str();
//}
//	��������� ����������� ������� �������
//bool VideoMenuImp::CheckEpisode(EPISODE episode,unsigned int index)
//{
//	switch(episode)
//	{
//	case E_NONE:
//		return (index<3);
//	case E_FIRST:
//		return (index<4);
//	case E_SECOND:
//		return (index<5);
//	case E_THIRD:
//		return (index<6);
//	case E_FOURTH:
//		return (index<7);
//	}
//
//	return true;
//}
//	��������� ��������� �����
bool VideoMenuImp::ProcessMouseInput(VMouseState* pMS)
{
	if(pMS->LButtonFront)
	{
		SetSelectionToMouse();
	}
	if(pMS->LDblClick)
	{
		TryShowFilm();
	}

	return true;
}
//	��������� ������������� �����
bool VideoMenuImp::ProcessKeyboardInput(VKeyboardState* pKS)
{
	if(pKS->Back[DIK_UP])
	{
		if(m_SelPos>0)
		{
			m_SelPos--;
			UpdateSelection();
		}
		return false;
	}
	if(pKS->Back[DIK_DOWN])
	{
		if(m_SelPos<(MAX_SLOT-1))
		{
			m_SelPos++;
			UpdateSelection();
		}
		return false;
	}
	if(pKS->Back[DIK_ESCAPE])
	{
		if(m_Observer) m_Observer->OnClose();
		return false;
	}
	if(pKS->Back[DIK_RETURN] || pKS->Back[DIK_NUMPADENTER])
	{
		TryShowFilm();
		return false;
	}

	return true;
}
//	��������� ��������� ��� �����
void VideoMenuImp::SetSelectionToMouse(void)
{
	int x = Input::MouseState().x-GetLocation()->OriginX()-GetLocation()->Region()->X();
	int y = Input::MouseState().y-GetLocation()->OriginY()-GetLocation()->Region()->Y();

	for(int i=0;i<MAX_SLOT;i++)
	{
		if(m_Slots[i]->GetLocation()->Region()->PtInRegion(x,y))
		{
			m_SelPos = i;
			UpdateSelection();
			break;
		}
	}
}
//	�������� ��������
void VideoMenuImp::UpdateSelection(void)
{
	int left, top, right, bottom;
	m_Slots[m_SelPos]->GetLocation()->Region()->GetRect(&left,&top,&right,&bottom);
	VRegion region(left-5,top,right+3,bottom);
	m_Selection->GetLocation()->Create(region);

	//m_Selection->GetLocation()->Create(*m_Slots[m_SelPos]->GetLocation()->Region());
}
//	���������� �������� �����
void VideoMenuImp::TryShowFilm(void)
{
	//if(CheckEpisode(m_Episode,m_SelPos))
	if(m_SelPos < m_videos.size())
	{
//		ISound::instance()->muteChannel(ISound::cMaster,true);
		Interface::Instance()->PlayBink(m_videos[m_SelPos].m_bik.c_str());
//		ISound::instance()->muteChannel(ISound::cMaster,false);
	}
}

/////////////////////////////////////////////////////////////////////////
///////////////////    class VideoMenu::VMIObserver    //////////////////
/////////////////////////////////////////////////////////////////////////
class VideoMenu::VMIObserver : public VideoMenuImp::Observer
{
private:
	VideoMenu* m_VideoMenu;
public:
	VMIObserver(VideoMenu* vm) : m_VideoMenu(vm) {}
	~VMIObserver() {}
public:
	//	����������� � �������� ����
	void OnClose(void)
	{
		m_VideoMenu->Parent()->OnChange(m_VideoMenu);
	}
};

//********************************************************************//
//*******************      class VideoMenu     ************************//
const char* VideoMenu::m_pDialogName = "VideoMenu";
VideoMenu::VideoMenu() : Dialog(m_pDialogName,m_pDialogName),m_Imp(0),m_VMIObserver(0)
{
	//	��������� ���������� ���������� ��������� ����
	Insert(m_Imp = new VideoMenuImp,L_CENTER);
	m_Imp->SetVisible(true);
	CheckConformMode();
	//	������� ����������� �� �����������
	m_VMIObserver = new VMIObserver(this);
	m_Imp->SetObserver(m_VMIObserver);
	//	��������� ������������� ������
	Focus()->Set(WidgetFocus::FF_KEYBOARD,m_Imp);
}

VideoMenu::~VideoMenu()
{
	delete m_VMIObserver;
}

//unsigned int VideoMenu::GetEpisodeAdvance(void) const
//{
//	return std::max(0l,static_cast<long>(std::min(static_cast<long>(VideoMenuImp::E_FINAL),static_cast<long>((256-Options::GetInt("game.advance"))))));
//}
//********************************************************************//
//*******************      class GameMenu     ************************//
const char* GameMenu::m_pDialogName = "GameMenu";
GameMenu::GameMenu() : Dialog(m_pDialogName,m_pDialogName)
{
	Focus()->Set(WidgetFocus::FF_KEYBOARD,Child("new"));
	m_MenuEvent = AE_NONE;
}

GameMenu::~GameMenu()
{
}

void GameMenu::OnButton(Button* pButton)
{
	if(!strcmp(pButton->Name(),"new")) m_MenuEvent = AE_NEW;
	else
		if(!strcmp(pButton->Name(),"load")) m_MenuEvent = AE_LOAD;
		else
			if(!strcmp(pButton->Name(),"multiplayer")) m_MenuEvent = AE_MULTIPLAYER;
			else
				if(!strcmp(pButton->Name(),"training")) m_MenuEvent = AE_TRAINING;
				else
					if(!strcmp(pButton->Name(),"back")) m_MenuEvent = AE_BACK;
	if(Parent()) Parent()->OnChange(this);
}

//********************************************************************//
//*******************    class OptionsMenu    ************************//
class OptionsMenuController : public OptionsScreenController
{
private:
	Widget* m_Owner;
public:
	OptionsMenuController(Widget* owner) : m_Owner(owner) {}
	void OnBack(void) {if(m_Owner->Parent()) m_Owner->Parent()->OnChange(m_Owner);}
};

const char* OptionsMenu::m_pDialogName = "OptionsMenu";
OptionsMenu::OptionsMenu() : Dialog(m_pDialogName,m_pDialogName)
{
	m_Controller = new OptionsMenuController(this);
	//	--------------------------------------------
	Insert(m_Options = new OptionsScreen(),L_CENTER);
	m_Options->SetMode(OptionsScreen::M_MAINMENU);
	m_Options->SetController(m_Controller);
	m_Options->SetVisible(true);
	//	--------------------------------------------
	AlignWidget(m_Options);
	CheckConformMode();

#ifdef DEMO_VERSION

	WidgetFactory::Instance()->Assert(this,"cinematics")->SetVisible(false);
	WidgetFactory::Instance()->Assert(this,"credits")->SetVisible(false);

#endif	//	DEMO_VERSION

}

OptionsMenu::~OptionsMenu()
{
	delete m_Controller;
}
//	���������� ��������� �������
void OptionsMenu::SetVisible(bool bVisible)
{
	if(bVisible) m_Options->Reset();
	Dialog::SetVisible(bVisible);
}

//********************************************************************//
//	LoadingMenu
class LoadingMenuController : public OptionsScreenController
{
private:
	Widget* m_Owner;
public:
	LoadingMenuController(Widget* owner) : m_Owner(owner) {}
	void OnBack(void) {if(m_Owner->Parent()) m_Owner->Parent()->OnChange(m_Owner);}
};

const char* LoadingMenu::m_pDialogName = "LoadingMenu";
LoadingMenu::LoadingMenu() : Dialog(m_pDialogName,m_pDialogName)
{
	m_Controller = new LoadingMenuController(this);
	//	--------------------------------------------
	Insert(m_Options = new OptionsScreen(),L_CENTER);
	m_Options->SetMode(OptionsScreen::M_LOADINGMENU);
	m_Options->SetController(m_Controller);
	m_Options->SetVisible(true);
	Focus()->Set(WidgetFocus::FF_KEYBOARD,m_Options);
	//	--------------------------------------------
	AlignWidget(m_Options);
	CheckConformMode();
}

LoadingMenu::~LoadingMenu()
{
	delete m_Controller;
}
//***************************************************************//
//	QuestionBox
const char* QuestionBox::m_pDialogName = "QuestionBox";
QuestionBox::QuestionBox() : Dialog(m_pDialogName,m_pDialogName)
{
}

QuestionBox::~QuestionBox()
{
}

int QuestionBox::DoModal(const char* text)
{
	if(Parent())
	{
		std::vector<Widget*> widgets;
		std::vector<bool> states;
		//	���������� ������� ������
		Widget* prev_focus = Parent()->Focus()->Get(WidgetFocus::FF_KEYBOARD);
		//	���������� ������ ��������� 
		for(Widget::Iterator it=Parent()->Begin();it!=Parent()->End();it++)
		{
			if(strcmp(it->Name(),Name()))
			{
				widgets.push_back(&(*it));
				states.push_back(it->IsEnable());
				it->Enable(false);
			}
		}
		//	��������� ������
		MouseCursor::SetCursor(MouseCursor::CT_NORMAL);
		Input::Mouse()->UnlockPosition();
		//	���������� ����������� ���������
		MenuMgr::Instance()->Tip()->Activate(false);
		//	��������� ���� ��� ������
		MenuMgr::Instance()->LockInput(true);
		//	-------------------------------------------------------------------------------------
		const int x = (Parent()->GetLocation()->Region()->Width()-GetLocation()->Region()->Width())/2;
		const int y = (Parent()->GetLocation()->Region()->Height()-GetLocation()->Region()->Height())/2;
		MoveTo(x,y);
		//	-------------------------------------------------------------------------------------
		static_cast<Text*>(WidgetFactory::Instance()->Assert(this,"text"))->SetText(text);
		Focus()->Set(WidgetFocus::FF_KEYBOARD,Child("yes"));
		//	-------------------------------------------------------------------------------------
		SetVisible(true);
		//	-------------------------------------------------------------------------------------
		Focus()->Enable(WidgetFocus::FF_MOUSE,true);
		//	-------------------------------------------------------------------------------------
		m_Result = -1;
		//	-------------------------------------------------------------------------------------
		Shell::Instance()->DoModal(this);
		//	-------------------------------------------------------------------------------------
		Focus()->Enable(WidgetFocus::FF_MOUSE,false);
		//	-------------------------------------------------------------------------------------
		SetVisible(false);
		//	������������ ���� ��� ������
		MenuMgr::Instance()->LockInput(false);
		//	��������������� ���������
		for(unsigned int i=0;i<widgets.size();i++)
			widgets[i]->Enable(states[i]);
		//	�������������� �����
		Parent()->Focus()->Set(WidgetFocus::FF_KEYBOARD,prev_focus);
	}

	return m_Result;
}

void QuestionBox::OnButton(Button* pButton)
{
	if(!strcmp(pButton->Name(),"yes")) m_Result = 1;
	if(!strcmp(pButton->Name(),"no")) m_Result = 0;
	exit(); //Shell::Instance()->Exit();
}

//	���������� �����-�� ��������
bool QuestionBox::Tick(void)
{
	DebugInfo::Add(600,0,"QuestionBox");

	if(Input::KeyFront(DIK_Y)) m_Result = 1;
	if(Input::KeyFront(DIK_N)) m_Result = 0;
	if(m_Result != -1)
	{
		exit(); //Shell::Instance()->Exit();
		return false;
	}
	
	return true;
}
//	����������� �� ��������� ��������� �������
void QuestionBox::OnSystemChange(void)
{
	Widget::OnSystemChange();
}

/////////////////////////////////////////////////////////////////////////
////////////////////////    class CreditsMenu    ////////////////////////
/////////////////////////////////////////////////////////////////////////
const char* CreditsMenu::m_pDialogName = "CreditsMenu";
const char* CreditsMenu::m_TilesScript = "Scripts/Interface/CreditsTiles.txt";
const char* CreditsMenu::m_CreditsScript = "Scripts/Credits.txt";
const float CreditsMenu::m_Speed = 20.0f;
CreditsMenu::CreditsMenu() :  Dialog(m_pDialogName,m_pDialogName),m_Seconds(Timer::GetSeconds()+1),
							  m_CurTile(0),m_AmbientSound(0),m_EndOfShow(false)
{
	TxtFile tiles_script(DataMgr::Load(m_TilesScript));
	TxtFile credits_script(DataMgr::Load(m_CreditsScript));

	DataMgr::Release(m_TilesScript);
	DataMgr::Release(m_CreditsScript);

	//	������� ��������
	for(int i=0;i<tiles_script.SizeY();++i)
	{
		if(tiles_script(i,0).size())
			m_Tiles.push_back(DXImageMgr::Instance()->CreateImage(tiles_script(i,0).c_str()));
	}

	m_Content = static_cast<TextBox*>(WidgetFactory::Instance()->Assert(this,"content"));
	m_FontHeight = m_Content->GetFont()->Height();

	const int lines = (MenuMgr::m_MainMenuResY/m_FontHeight)+1;
	std::string empty_lines(lines,'\n');

	m_Part1 = empty_lines+credits_script(0,0)+empty_lines;
	m_Part2 = credits_script(1,0);

	m_Content->SetText(m_Part1.c_str());

	const int max_height = m_Content->GetLogicalText().Height();
	m_TileChangingTime = max_height/(m_Tiles.size()+1); 

	//	������������� ������ ��������
	if(GetImage()) GetImage()->Release();
	if(m_Tiles.size()) SetImage(m_Tiles[0]);
}

//=====================================================================================//
//                             CreditsMenu::~CreditsMenu()                             //
//=====================================================================================//
CreditsMenu::~CreditsMenu()
{
	SetImage(0);
	m_AmbientSound.reset();
	for(int i=0;i<m_Tiles.size();i++) m_Tiles[i]->Release();
}
//	���������� �����-�� ��������
bool CreditsMenu::Tick(void)
{
	if(IsVisible())
	{
		if(!CheckExit())
		{
			if(!m_EndOfShow)
			{
				if(!m_Content->IsScrollEnd())
				{
					const int upper_border = (Timer::GetSeconds()-m_Seconds)*m_Speed;
					m_Content->ScrollTo(-upper_border);
					if(upper_border>m_TileChangingTime*(m_CurTile+1))
					{
						SetImage(m_Tiles[++m_CurTile]);
					}
				}
				else
				{
					m_Content->Align(LogicalText::T_HCENTER|LogicalText::T_BOTTOM);
					m_Content->Scroll(TextBox::SD_BEGIN);
					m_Content->SetText(m_Part2.c_str());
					m_EndOfShow = true;
				}
			}
		}
	}

	return false;
}

//=====================================================================================//
//                           void CreditsMenu::SetVisible()                            //
//=====================================================================================//
//	���������� ��������� �������
void CreditsMenu::SetVisible(bool bVisible)
{
	Dialog::SetVisible(bVisible);

	if(bVisible)
	{
		m_AmbientSound = Muffle::ISound::instance()->beginSession();
		m_AmbientSound->changeTheme("CREDITS");

		m_Content->Align(LogicalText::T_HCENTER|LogicalText::T_VCENTER);
		m_Content->SetText(m_Part1.c_str());
		m_Content->Scroll(TextBox::SD_BEGIN);
		SetImage(m_Tiles[m_CurTile = 0]);
		m_Seconds = Timer::GetSeconds();
		MouseCursor::SetVisible(false);
		m_EndOfShow = false;
		Input::Update();
	}
	else
	{
		m_AmbientSound.reset();
		MouseCursor::SetVisible(true);
	}
}
//	��������� �� �����
bool CreditsMenu::CheckExit(void)
{
	if(Input::MouseState().LButtonBack	||
	   Input::KeyBack(DIK_NUMPADENTER)	||
	   Input::KeyBack(DIK_RETURN)		||
	   Input::KeyBack(DIK_ESCAPE)		|| 
	   Input::KeyBack(DIK_SPACE))
	{
		Parent()->OnChange(this);
		return true;
	}

	return false;
}
//	����������� �� ��������� ��������� �������
void CreditsMenu::OnSystemChange(void)
{
	for(int i=0;i<m_Tiles.size();++i)
		m_Tiles[i]->Reload();

	Dialog::OnSystemChange();
}

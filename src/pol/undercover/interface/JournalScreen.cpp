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

#include <common/DataMgr/TxtFile.h>
#include <undercover/gamelevel/GameLevel.h>
#include <common/UI/Static.h>
#include <common/UI/Button.h>
#include "WidgetFactory.h"
#include "DialogScreen.h"
#include "SlotsEngine.h"
#include "DXImageMgr.h"
#include "MenuMgr.h"
#include "MultiEdit.h"
#include <undercover/World.h>
#include "TextBox.h"
#include "Screens.h"
#include "Edit.h"
#include "Text.h"
#include "JournalScreen.h"
#include <common/D3DApp/Input/Input.h>

const char* g_NewsPictures = "pictures/interface/JournalScreen/News/";
const char* g_TechPictures = "pictures/interface/Banners/Tech/";
const char* g_ForcesPictures = "pictures/interface/Banners/Forces/";
const char* g_PeoplePictures = "pictures/interface/Banners/People/";
//const char* g_LocationPictures = "pictures/interface/Banners/Location";

//*********************************************************************//
//	class JournalScreen::HotLineControl
class JournalScreen::HotLineControl
{
private:
	static const char* m_ScriptPath;
	JournalScreen* m_Journal;
private:
	struct Block
	{
		std::map<std::string,Block> m_Level;		//	������� ����������� <caption,element>
		std::string m_Content;						//	���������� (�����)
	};
	//	����� ������ ��� ��������� ����������
	std::map<std::string,Block> m_Blocks;
public:
	HotLineControl(JournalScreen* journal) : m_Journal(journal)
	{
		TxtFile script(DataMgr::Load(m_ScriptPath));
		DataMgr::Release(m_ScriptPath);

		for(int i=1;i<script.SizeY();i++)
		{
			for(int j=1;j<script.SizeX(i);j++)
			{
				InsertBlock(m_Blocks[script(i,0)],script(0,j).c_str(),script(i,j).c_str());
			}
		}
	}
	virtual ~HotLineControl() {}
public:
	//	�������� ������� �����
	void Check(const char* object,const char* caption)
	{
		std::map<std::string,Block>::iterator i = m_Blocks.find(object);
		std::map<std::string,Block>::iterator j;
		std::string content;
		int offset = 0;

		if(i != m_Blocks.end())
		{
			while(const char* dot = strchr(caption+offset,'.'))
			{
				const int fore_dot_len = strlen(caption)-strlen(dot);
				char* buffer = new char[fore_dot_len+1];
				strncpy(buffer,caption,fore_dot_len);
				buffer[fore_dot_len] = 0;
				offset += fore_dot_len+1;
				j = i->second.m_Level.find(buffer);
				delete[] buffer;
				if(j == i->second.m_Level.end()) break;
				i = j;
			}
			j = i->second.m_Level.find(caption+offset);
			if(i == m_Blocks.find(object))
			{
				if(j == i->second.m_Level.end()) j = i->second.m_Level.find("default");
			}
			else
			{
				if(j == i->second.m_Level.end()) j = i;
			}
			if(j != i->second.m_Level.end()) content = j->second.m_Content;
		}
		m_Journal->SetHotLineText(content.c_str());
	}
private:
	//	�������� ���������� � ����
	void InsertBlock(Block& block,const char* caption,const char* info)
	{
		if(strlen(info))
		{
			const char* dot = strchr(caption,'.');
			
			if(dot)
			{//	���������� ���������
				const int fore_dot_len = strlen(caption)-strlen(dot);
				char* buffer = new char[fore_dot_len+1];
				strncpy(buffer,caption,fore_dot_len);
				buffer[fore_dot_len] = 0;
				InsertBlock(block.m_Level[buffer],caption+fore_dot_len+1,info);
				delete[] buffer;
			}
			else
			{//	�������� ����������
				block.m_Level[caption].m_Content = info;
			}
		}
	}
};

const char* JournalScreen::HotLineControl::m_ScriptPath = "scripts/interface/HotLine.txt";

//*********************************************************************//
//	class JournalScreen
const char* JournalScreen::m_pDialogName = "JournalScreen";
JournalScreen::JournalScreen() : Dialog(m_pDialogName,m_pDialogName),m_Controller(0),
								 m_News(0),m_Database(0),m_Journal(0),m_Navigation(0),
								 m_ActiveSheet(0),m_HotLine(0)
{
	//	������� �������� ������� ���������
	m_HotLine = new HotLineControl(this);
	m_HLWidget = static_cast<Text*>(WidgetFactory::Instance()->Assert(this,"hot_line"));
	//	�������������� ������ ������
	m_Buttons[B_BACK]	= static_cast<Button*>(WidgetFactory::Instance()->Assert(this,"power"));
	m_Buttons[B_UP]		= static_cast<Button*>(WidgetFactory::Instance()->Assert(this,"up"));
	m_Buttons[B_DOWN]	= static_cast<Button*>(WidgetFactory::Instance()->Assert(this,"down"));
	m_Buttons[B_LEFT]	= static_cast<Button*>(WidgetFactory::Instance()->Assert(this,"left"));
	m_Buttons[B_RIGHT]	= static_cast<Button*>(WidgetFactory::Instance()->Assert(this,"right"));
	m_Buttons[B_BACK]->NoFocus(true);
	m_Buttons[B_UP]->NoFocus(true);
	m_Buttons[B_DOWN]->NoFocus(true);
	m_Buttons[B_LEFT]->NoFocus(true);
	m_Buttons[B_RIGHT]->NoFocus(true);
	//	�������������� ������ ������ �� ����������
	m_UpdateButtons[UB_DATABASE].m_Static	= static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"database_update"));
	m_UpdateButtons[UB_JOURNAL].m_Static	= static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"journal_update"));
	m_UpdateButtons[UB_NAVIGATION].m_Static	= static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"navigation_update"));
	//	�������������� ������ ��������
	m_Sheets[S_DATABASE].m_Static	= static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"database"));
	m_Sheets[S_JOURNAL].m_Static	= static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"journal"));
	m_Sheets[S_NAVIGATION].m_Static	= static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"navigation"));
	//	������� ���������� ��������
	Insert(m_News		= new NewsSheet(this),L_CENTER);
	Insert(m_Database	= new DatabaseSheet(this),L_CENTER);
	Insert(m_Journal	= new JournalSheet(this),L_CENTER);
	Insert(m_Navigation = new NavigationSheet(this),L_CENTER);
	//	���������� ����� � ���������
	(m_ActiveSheet = m_Database)->SetVisible(true);
}

JournalScreen::~JournalScreen()
{
	delete m_HotLine;
}
//	��������� ��������� �����
bool JournalScreen::ProcessMouseInput(VMouseState* pMS)
{
	std::string caption = m_ActiveSheet->CheckHotLine();
	Widget* from_point = FromPoint(pMS->x,pMS->y);

	if(from_point)
	{
		std::string object_name;

		if(from_point->Parent()!=this)
		{
			object_name = m_ActiveSheet->Name();
			object_name += ".";
			object_name += m_ActiveSheet->FirstChildFromPoint(pMS->x,pMS->y)->Name();		
		}
		else
		{
			object_name += from_point->Name();
		}
		m_HotLine->Check(object_name.c_str(),caption.c_str());
	}
	else
	{
		SetHotLineText("");
	}

	return true;
}
//	�������������� ������������ �����
void JournalScreen::Activate(SCREEN_ID id)
{
	//	����� ���������� �������� �����
	if(m_ActiveSheet) m_ActiveSheet->SetVisible(false);
	//	�������� �����
	switch(id)
	{
	case SID_NEWS:
		(m_ActiveSheet = m_News)->SetVisible(true);
		break;
	case SID_DATABASE:
		(m_ActiveSheet = m_Database)->SetVisible(true);
		break;
	case SID_JOURNAL:
		(m_ActiveSheet = m_Journal)->SetVisible(true);
		break;
	case SID_NAVIGATION:
		(m_ActiveSheet = m_Navigation)->SetVisible(true);
		break;
	}
}
//	�������� ��������� � ������� ��������
NewsSheet* JournalScreen::News(void)
{
	return m_News;
}
//	�������� ��������� � ������� ������������
DatabaseSheet* JournalScreen::Database(void)
{
	return m_Database;
}
//	�������� ��������� � ������� ��������
JournalSheet* JournalScreen::Journal(void)
{
	return m_Journal;
}
//	�������� ��������� � ������� ���������
NavigationSheet* JournalScreen::Navigation(void)
{
	return m_Navigation;
}
//	����������� �� ��������� ��������� �������
void JournalScreen::OnSystemChange(void)
{
	Widget::OnSystemChange();
}
//	����������� �� ��������� ��������� ������
void JournalScreen::OnButton(Button *pButton)
{
	if(m_Controller)
	{
		if(pButton == m_Buttons[B_BACK])
		{
			m_Controller->OnBack();
			return;
		}
	}
	if(m_ActiveSheet)
	{
		for(int i=0;i<MAX_BUTTON;i++)
		{
			if(m_Buttons[i] == pButton)
			{
				if(m_ActiveSheet->GetController())
					m_ActiveSheet->GetController()->OnButtonClick(static_cast<BUTTON>(i));

				return;
			}
		}
	}
}

//	���������� ����������
void JournalScreen::SetController(Controller* controller)
{
	m_Controller = controller;
}
//	������������ ������ ����������
void JournalScreen::ActivateUpdateButton(UPDATE_BUTTON id,ACTIVATE_MODE mode)
{
	switch(mode)
	{
	case AM_ACTIVATE:
		if(m_UpdateButtons[id].m_Fixed)
		{
			m_UpdateButtons[id].m_Static->SetState(0);
			m_UpdateButtons[id].m_Fixed = false;
		}
		break;
	case AM_DEACTIVATE:
		if(!m_UpdateButtons[id].m_Fixed)
		{
			m_UpdateButtons[id].m_Static->SetState(2);
			m_UpdateButtons[id].m_Fixed = true;
		}
		break;
	}
}
//	���������� ������ ������������ ��������
void JournalScreen::IlluminateSheet(SHEET id)
{
	for(int i=0;i<MAX_SHEET;i++)
	{
		m_Sheets[i].m_Static->SetState(0);
		m_Sheets[i].m_Fixed = false;
	}
	m_Sheets[id].m_Static->SetState(1);
	m_Sheets[id].m_Fixed = true;
}

void JournalScreen::OnStatic(Static *pStatic)
{
	for(int sheet=0;sheet<MAX_SHEET;sheet++)
	{
		if(m_Sheets[sheet].m_Static == pStatic)
		{
			switch(pStatic->LastEvent())
			{
			case Static::LE_MOUSEMOVE:
				if(!m_Sheets[sheet].m_Fixed) pStatic->SetState(1);
				break;
			case Static::LE_MOUSELOST:
				if(!m_Sheets[sheet].m_Fixed) pStatic->SetState(0);
				break;
			case Static::LE_LBUTTONDOWN:
				pStatic->SetState(2);
				break;
			case Static::LE_LBUTTONUP:
				if(!m_Sheets[sheet].m_Fixed)
				{
					for(int i=0;i<MAX_SHEET;i++)
					{
						m_Sheets[i].m_Static->SetState(0);
						m_Sheets[i].m_Fixed = false;
					}
					m_Sheets[sheet].m_Static->SetState(1);
					m_Sheets[sheet].m_Fixed = true;
					if(m_Controller) m_Controller->OnSheetClick(static_cast<SHEET>(sheet));
				}
				else
				{
					m_Sheets[sheet].m_Static->SetState(1);
					m_Sheets[sheet].m_Fixed = false;
					if(m_Controller) m_Controller->OnSheetClick(S_NEWS);
				}
				break;
			}
			return;
		}
	}
	for(int ub=0;ub<MAX_UPDATE_BUTTON;ub++)
	{
		if(m_UpdateButtons[ub].m_Static == pStatic)
		{
			switch(pStatic->LastEvent())
			{
			case Static::LE_MOUSEMOVE:
				if(!m_UpdateButtons[ub].m_Fixed) pStatic->SetState(1);
				break;
			case Static::LE_MOUSELOST:
				if(!m_UpdateButtons[ub].m_Fixed) pStatic->SetState(0);
				break;
			case Static::LE_LBUTTONDOWN:
				pStatic->SetState(2);
				break;
			case Static::LE_LBUTTONUP:
				if(!m_UpdateButtons[ub].m_Fixed)
				{
					m_UpdateButtons[ub].m_Static->SetState(1);
					if(m_Controller) m_Controller->OnUpdateClick(static_cast<UPDATE_BUTTON>(ub));
				}
				break;
			}
			return;
		}
	}
}
//	��������� ������������� �����
bool JournalScreen::ProcessKeyboardInput(VKeyboardState* pKS)
{
	const bool is_escape = Input::KeyBack(DIK_ESCAPE|Input::RESET);
	const bool is_j = Input::KeyBack(DIK_J|(!m_Journal->IsEdit()?Input::RESET:0));

	if(is_escape||is_j)
	{
		if(m_Journal->IsEdit())
		{
			if(is_escape) m_Journal->ContinueEdit(JournalSheet::CM_CANCEL);
		}
		else
		{
			if(m_Controller) m_Controller->OnBack();
		}
	}

	return true;
}
//	���������/��������� ������ �� ��������� ����������
void JournalScreen::EnableControls(bool enable)
{
	int i;
		
	for(i=0;i<MAX_BUTTON;i++)
		m_Buttons[i]->Enable(enable);
	for(i=0;i<MAX_UPDATE_BUTTON;i++)
		m_UpdateButtons[i].m_Static->Enable(enable);
	for(i=0;i<MAX_SHEET;i++)
		m_Sheets[i].m_Static->Enable(enable);
}
//	���������� ��������� �������
void JournalScreen::SetVisible(bool bVisible)
{
	//	���������� �������� � ������� ��������
	if(bVisible)
	{
		for(int i=0;i<MAX_SHEET;i++)
		{
			m_Sheets[i].m_Static->SetState(0);
			m_Sheets[i].m_Static->Enable(true);
			m_Sheets[i].m_Fixed = false;
		}
	}
	//	�������� ���������� ������
	Dialog::SetVisible(bVisible);
}
//	���������� ����� ������� ���������
void JournalScreen::SetHotLineText(const char* text)
{
	if(m_HLWidget->GetText() != text)
	{
		m_HLWidget->SetText(text);
	}
}
//*********************************************************************//
//	class NewsSheet
const char* NewsSheet::m_pDialogName = "NewsSheet";
NewsSheet::NewsSheet(JournalScreen* js) : Sheet(js,m_pDialogName,m_pDialogName),m_Controller(0)
{
	//	��������� ����
	m_Channel = static_cast<Text*>(WidgetFactory::Instance()->Assert(this,"channel"));
	m_Content = static_cast<TextBox*>(WidgetFactory::Instance()->Assert(this,"content"));
	//	��������
	m_ImagePlace = *WidgetFactory::Instance()->Assert(this,"image_place")->GetLocation()->Region();
	Widget::Insert(m_Image = new Widget("image"));
}

NewsSheet::~NewsSheet()
{
}

void NewsSheet::SetController(JournalScreen::SheetController* controller)
{
	m_Controller = controller;
}

void NewsSheet::SetTraits(const Traits* t)
{
	bool visibility = t?true:false;

	if(visibility)
	{
		//	��������� ����
		m_Channel->SetText(t->m_Channel);
		m_Content->SetText(t->m_Content);
		m_Content->Scroll(TextBox::SD_BEGIN);
		//	��������
		int width,height;
		if(m_Image->GetImage())
			DXImageMgr::Instance()->Release(static_cast<DXImage*>(m_Image->GetImage()));
		m_Image->SetImage(DXImageMgr::Instance()->CreateImage(std::string(std::string(g_NewsPictures)+std::string(t->m_Image)+".tga").c_str()));
		DXImageMgr::Instance()->GetImageSize(static_cast<DXImage*>(m_Image->GetImage()),&width,&height);
		int left = m_ImagePlace.X()+(m_ImagePlace.Width()-width)/2;
		int top	 = m_ImagePlace.Y()+(m_ImagePlace.Height()-height)/2;
		m_Image->GetLocation()->Create(left,top,left+width,top+height);
	}
	m_Channel->SetVisible(visibility);
	m_Content->SetVisible(visibility);
	m_Image->SetVisible(visibility);
}

JournalScreen::SheetController* NewsSheet::GetController(void)
{
	return m_Controller;
}

std::string NewsSheet::CheckHotLine(void)
{
	return "news";
}

//*********************************************************************//
//	class DatabaseSheet
const char* DatabaseSheet::m_pDialogName = "DatabaseSheet";
DatabaseSheet::DatabaseSheet(JournalScreen* js) : Sheet(js,m_pDialogName,m_pDialogName),m_Controller(0)
{
	//	��������� ������ ��������
	m_Sheets[S_OUTFIT].m_Static	= static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"outfit"));
	m_Sheets[S_TECH].m_Static	= static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"tech"));
	m_Sheets[S_FORCES].m_Static	= static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"forces"));
	m_Sheets[S_PEOPLE].m_Static	= static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"people"));
	//	�������� ���������
	m_LeftColumn	= static_cast<Text*>(WidgetFactory::Instance()->Assert(this,"left"));
	m_RightColumn	= static_cast<Text*>(WidgetFactory::Instance()->Assert(this,"right"));
	m_ItemName		= static_cast<Text*>(WidgetFactory::Instance()->Assert(this,"name"));
	m_Text			= static_cast<TextBox*>(WidgetFactory::Instance()->Assert(this,"text"));
	//	��������� ������ ��� ����������� ��������
	Widget::Insert(m_Image = new Widget("image"));
}

DatabaseSheet::~DatabaseSheet()
{
}

JournalScreen::SheetController* DatabaseSheet::GetController(void)
{
	return m_Controller;
}

void DatabaseSheet::SetController(Controller* controller)
{
	m_Controller = controller;
}
//	�������� ������� �����
std::string DatabaseSheet::CheckHotLine(void)
{
	int sheet=0;
	for(;sheet<MAX_SHEET;sheet++)
	{
		if(m_Sheets[sheet].m_Static->IsEnable() && m_Sheets[sheet].m_Fixed) 
			break;
	}

	switch(sheet)
	{
	case S_OUTFIT:
		return "database.outfit";
	case S_TECH:
		return "database.tech";
	case S_FORCES:
		return "database.forces";
	case S_PEOPLE:
		return "database.people";
	default:
		return "database";
	}
}
//	���������� ������ ������������ ��������
void DatabaseSheet::IlluminateSheet(SHEET id)
{
	for(int i=0;i<MAX_SHEET;i++)
	{
		if(m_Sheets[i].m_Static->IsEnable())
		{
			m_Sheets[i].m_Static->SetState(0);
			m_Sheets[i].m_Fixed = false;
		}
	}
	m_Sheets[id].m_Static->SetState(1);
	m_Sheets[id].m_Fixed = true;
}
//	���������/��������� ������ � ��������
void DatabaseSheet::EnableSheet(SHEET id,bool enable)
{
	if(m_Sheets[id].m_Static->IsEnable() != enable)
	{
		m_Sheets[id].m_Static->Enable(enable);
		if(enable) m_Sheets[id].m_Static->SetState(0);
		else	   m_Sheets[id].m_Static->SetState(2);
	}
}
//	���������� ������������ �������
void DatabaseSheet::SetSectionName(const char* name)
{
	static_cast<Text*>(WidgetFactory::Instance()->Assert(this,"section"))->SetText(name);
}
//	����������� �� ��������� ��������� ��������
void DatabaseSheet::OnStatic(Static *pStatic)
{
	for(int sheet=0;sheet<MAX_SHEET;sheet++)
	{
		if(m_Sheets[sheet].m_Static == pStatic)
		{
			switch(pStatic->LastEvent())
			{
			case Static::LE_MOUSEMOVE:
				if(!m_Sheets[sheet].m_Fixed) pStatic->SetState(1);
				break;
			case Static::LE_MOUSELOST:
				if(!m_Sheets[sheet].m_Fixed) pStatic->SetState(0);
				break;
			case Static::LE_LBUTTONDOWN:
				if(!m_Sheets[sheet].m_Fixed) pStatic->SetState(2);
				break;
			case Static::LE_LBUTTONUP:
				if(!m_Sheets[sheet].m_Fixed)
				{
					for(int i=0;i<MAX_SHEET;i++)
					{
						if(m_Sheets[i].m_Static->IsEnable())
						{
							m_Sheets[i].m_Static->SetState(0);
							m_Sheets[i].m_Fixed = false;
						}
					}
					m_Sheets[sheet].m_Static->SetState(1);
					m_Sheets[sheet].m_Fixed = true;
					if(m_Controller) m_Controller->OnSheetClick(static_cast<SHEET>(sheet));
				}
				break;
			}
			return;
		}
	}
}
//	���������� ���������� ��� ������
void DatabaseSheet::SetItemInfo(const ItemInfo* ii,SHEET id)
{
	bool visibility = ii?true:false;

	if(visibility)
	{
		m_LeftColumn->SetText(ii->m_LeftColumn);
		m_RightColumn->SetText(ii->m_RightColumn);
		m_ItemName->SetText(ii->m_Name);
		m_Text->SetText(ii->m_Text);
		m_Text->Scroll(TextBox::SD_BEGIN);
		//	������������� ��������
		int width,height;
		const VRegion* image_place = WidgetFactory::Instance()->Assert(this,"image_place")->GetLocation()->Region();
		if(m_Image->GetImage())
			DXImageMgr::Instance()->Release(static_cast<DXImage*>(m_Image->GetImage()));
		m_Image->SetImage(DXImageMgr::Instance()->CreateImage(std::string(std::string(GetPathForResource(id))+std::string(ii->m_Image)+".tga").c_str()));
		DXImageMgr::Instance()->GetImageSize(static_cast<DXImage*>(m_Image->GetImage()),&width,&height);
		int left = image_place->X()+(image_place->Width()-width)/2;
		int top	 = image_place->Y()+(image_place->Height()-height)/2;
		m_Image->GetLocation()->Create(left,top,left+width,top+height);
	}
	//	������������� ���������
	m_LeftColumn->SetVisible(visibility);
	m_RightColumn->SetVisible(visibility);
	m_ItemName->SetVisible(visibility);
	m_Image->SetVisible(visibility);
	m_Text->SetVisible(visibility);
}
//	�������� ������� � ���������� ��� ������ ��������
const char* DatabaseSheet::GetPathForResource(SHEET id) const
{
	switch(id)
	{
	case S_OUTFIT:
		return g_InventoryPictures;
	case S_TECH:
		return g_TechPictures;
	case S_FORCES:
		return g_ForcesPictures;
	case S_PEOPLE:
		return g_PeoplePictures;
	}

	return "";
}

//*********************************************************************//
//	class JournalSheet
const char* JournalSheet::m_pDialogName = "JournalSheet";
JournalSheet::JournalSheet(JournalScreen* js) : Sheet(js,m_pDialogName,m_pDialogName),m_Controller(0)
{
	//	��������� ������ ��������
	m_Sheets[S_NEWTHEME].m_Static	= static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"newtheme"));
	m_Sheets[S_DELTHEME].m_Static	= static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"deltheme"));
	m_Sheets[S_NEWENTRY].m_Static	= static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"newentry"));
	m_Sheets[S_DELENTRY].m_Static	= static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"delentry"));
	//	�������� �������������� ��������
	m_ThemeName = static_cast<Text*>(WidgetFactory::Instance()->Assert(this,"theme_name"));
	m_ThemeEdit = static_cast<Edit*>(WidgetFactory::Instance()->Assert(this,"theme_edit"));
	m_Content = static_cast<TextBox*>(WidgetFactory::Instance()->Assert(this,"content"));
	m_ContentEdit = static_cast<MultiEdit*>(WidgetFactory::Instance()->Assert(this,"content_edit"));
	//	�������� ������ ����������� ������
	m_SButtons[SB_OK].m_Static		= static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"ok"));
	m_SButtons[SB_CANCEL].m_Static	= static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"cancel"));
	//	�������� �������� ��������������
	m_ThemeEdit->SetVisible(false);
	m_ContentEdit->SetVisible(false);
	for(int i=0;i<MAX_SPECIAL_BUTTON;i++)
		m_SButtons[i].m_Static->SetVisible(false);
}

JournalSheet::~JournalSheet()
{
}

JournalScreen::SheetController* JournalSheet::GetController(void)
{
	return m_Controller;
}

void JournalSheet::SetController(Controller* controller)
{
	m_Controller = controller;
}

//	����������� �� ��������� ��������� ��������
void JournalSheet::OnStatic(Static *pStatic)
{
	for(int sheet=0;sheet<MAX_SHEET;sheet++)
	{
		if(m_Sheets[sheet].m_Static == pStatic)
		{
			switch(pStatic->LastEvent())
			{
			case Static::LE_MOUSEMOVE:
				if(!m_Sheets[sheet].m_Fixed) pStatic->SetState(1);
				break;
			case Static::LE_MOUSELOST:
				if(!m_Sheets[sheet].m_Fixed) pStatic->SetState(0);
				break;
			case Static::LE_LBUTTONDOWN:
				pStatic->SetState(2);
				break;
			case Static::LE_LBUTTONUP:
				if(!m_Sheets[sheet].m_Fixed)
				{
					for(int i=0;i<MAX_SHEET;i++)
					{
						if(m_Sheets[i].m_Static->IsEnable())
						{
							m_Sheets[i].m_Static->SetState(0);
							m_Sheets[i].m_Fixed = false;
						}
					}
					m_Sheets[sheet].m_Static->SetState(1);
					m_Sheets[sheet].m_Fixed = true;
					if(m_Controller) m_Controller->OnSheetClick(static_cast<SHEET>(sheet));
				}
				else
				{
					m_Sheets[sheet].m_Static->SetState(1);
					m_Sheets[sheet].m_Fixed = false;
				}
				break;
			}
			return;
		}
	}
	for(int sb=0;sb<MAX_SPECIAL_BUTTON;sb++)
	{
		if(m_SButtons[sb].m_Static == pStatic)
		{
			switch(pStatic->LastEvent())
			{
			case Static::LE_MOUSEMOVE:
				pStatic->SetState(1);
				break;
			case Static::LE_MOUSELOST:
				pStatic->SetState(0);
				break;
			case Static::LE_LBUTTONDOWN:
				pStatic->SetState(2);
				break;
			case Static::LE_LBUTTONUP:
				pStatic->SetState(0);
				if(sb == SB_OK)
				{
					ContinueEdit(CM_OK);
					break;
				}
				if(sb == SB_CANCEL)
				{
					ContinueEdit(CM_CANCEL);
					break;
				}
				break;
			}
			return;
		}
	}
}
//	����� ��������� � ������
void JournalSheet::UnSelectSheet(SHEET id)
{
	if(m_Sheets[id].m_Enable)
	{
		m_Sheets[id].m_Static->SetState(0);
		m_Sheets[id].m_Fixed = false;
	}
}
//	���������� ��������� ����
void JournalSheet::SetTextField(const char* text,TEXT_FIELD tf)
{
	switch(tf)
	{
	case TF_THEME:
		m_ThemeName->SetText(text);
		break;
	case TF_THEME_COUNT:
		static_cast<Text*>(WidgetFactory::Instance()->Assert(this,"theme_number"))->SetText(text);
		break;
	case TF_ENTRY_COUNT:
		static_cast<Text*>(WidgetFactory::Instance()->Assert(this,"entry_number"))->SetText(text);
		break;
	case TF_CONTENT:
		m_Content->SetText(text);
		break;
	}
}
//	������ �������� ���� ��� ������
//enum SECTION {S_THEME,S_ENTRY};
void JournalSheet::EditSection(SECTION section,const char* text)
{
	switch(section)
	{
	case S_THEME:
		EnableControls(false);
		m_ThemeName->SetVisible(false);
		m_ThemeEdit->SetVisible(true);
		m_ThemeEdit->SetText("");
		Focus()->Set(WidgetFocus::FF_KEYBOARD,m_ThemeEdit);
		break;
	case S_ENTRY:
		EnableControls(false);
		m_Content->SetVisible(false);
		m_ContentEdit->SetVisible(true);
		m_ContentEdit->Clear();
		for(int i=0;i<MAX_SPECIAL_BUTTON;i++)
			m_SButtons[i].m_Static->SetVisible(true);
		Focus()->Set(WidgetFocus::FF_KEYBOARD,m_ContentEdit);
		break;
	}
}
//	������ ���������� ����� ��������������
const char* JournalSheet::GetSectionText(SECTION section) const
{
	switch(section)
	{
	case S_THEME:
		return m_ThemeEdit->GetText().c_str();
	case S_ENTRY:
		return m_ContentEdit->GetText().c_str();
	}

	return "";
}
//	���������/��������� ������ �� ��������� ����������
void JournalSheet::EnableControls(bool enable)
{
	//	��������� ��������
	for(int sheet=0;sheet<MAX_SHEET;sheet++)
		if(m_Sheets[sheet].m_Enable) m_Sheets[sheet].m_Static->Enable(enable);
	//	��������� ������������ ��������
	m_JournalScreen->EnableControls(enable);
}
//	���������� ����� ��������������
bool JournalSheet::IsEdit(void) const
{
	return m_ThemeEdit->IsVisible() || m_ContentEdit->IsVisible();
}
//	�������� ��������������
void JournalSheet::ContinueEdit(CONTINUE_MODE cm)
{
	if(m_ThemeEdit->IsVisible())
	{
		m_ThemeName->SetVisible(true);
		m_ThemeEdit->SetVisible(false);
		UnSelectSheet(S_NEWTHEME);
		if((cm==CM_OK) && m_Controller)
			m_Controller->OnSheetClick(S_SAVE_THEME);
	}
	if(m_ContentEdit->IsVisible())
	{
		m_Content->SetVisible(true);
		m_ContentEdit->SetVisible(false);
		UnSelectSheet(S_NEWENTRY);
		for(int i=0;i<MAX_SPECIAL_BUTTON;i++)
			m_SButtons[i].m_Static->SetVisible(false);
		if((cm==CM_OK) && m_Controller)
			m_Controller->OnSheetClick(S_SAVE_ENTRY);
	}

	EnableControls(true);
}
//	����������� �� ��������� ���������
void JournalSheet::OnChange(Widget* pChild)
{
	if(pChild == m_ThemeEdit)
	{
		ContinueEdit(CM_OK);
		return;
	}
	Dialog::OnChange(pChild);
}
//	���������/��������� ������ � ��������
void JournalSheet::EnableSheet(SHEET id,bool enable)
{
	if(m_Sheets[id].m_Enable != enable)
	{
		m_Sheets[id].m_Enable = enable;
		m_Sheets[id].m_Static->Enable(enable);
		if(enable) m_Sheets[id].m_Static->SetState(0);
		else	   m_Sheets[id].m_Static->SetState(2);
	}
}
//	�������� ������� �����
std::string JournalSheet::CheckHotLine(void)
{
	int sheet=0;
	for(;sheet<MAX_SHEET;sheet++)
	{
		if(m_Sheets[sheet].m_Static->IsEnable() && m_Sheets[sheet].m_Fixed) 
			break;
	}

	switch(sheet)
	{
	case S_NEWTHEME:
		return "journal.newtheme";
	case S_DELTHEME:
		return "journal.deltheme";
	case S_NEWENTRY:
		return "journal.newentry";
	case S_DELENTRY:
		return "journal.delentry";
	default:
		return "journal";
	}
}

//*********************************************************************//
//	class NavigationSheet::Flag
class NavigationSheet::Flag : public Widget
{
public:
	static const char* m_WidgetType;
	static const char* m_ImagePath;
private:
	Static* m_Static;
	std::string m_Description;
	std::string m_Hint;
	std::string m_Image;
public:
	Flag(const char* color,const char* center_xy,const char* hint,
		 const char* image,const char* desc) : Widget(GenerateName().c_str()),
		 m_Description(desc),m_Hint(hint),m_Image(image)
	{
		std::string color_image = m_ImagePath;
		color_image += color;
		int width,height;
		int x,y;

		//	����������� ���� �� ��������
		m_Image  = "pictures/interface/JournalScreen/Levels/Places/";
		m_Image += image;
		m_Image += ".tga";

		//	������������� ��������
		Insert(m_Static = new Static("static"));
		m_Static->AddImage(DXImageMgr::Instance()->CreateImage(std::string(color_image + "_n.tga").c_str()));
		m_Static->AddImage(DXImageMgr::Instance()->CreateImage(std::string(color_image + "_s.tga").c_str()));
		m_Static->AddImage(DXImageMgr::Instance()->CreateImage(std::string(color_image + "_p.tga").c_str()));
		m_Static->SetState(0);
		DXImageMgr::Instance()->GetImageSize(static_cast<DXImage*>(m_Static->GetImage(0)),&width,&height);
		m_Static->GetLocation()->Create(0,0,width,height);

		if(sscanf(center_xy,"%d,%d",&x,&y) == 2)
		{
			x -= (width>>1);
			y -= (height>>1);
			GetLocation()->Create(x,y,x+width,y+height);
		}
		//	������������� ���������
		m_Tip.m_Text = hint;
	}
	virtual ~Flag()
	{
	}
private:
	//	����������� �� ��������� ���� �� ��������� ���������
	void OnChange(Widget* pChild)
	{
		switch(m_Static->LastEvent())
		{
			case Static::LE_MOUSEMOVE:
				m_Static->SetState(1);
				break;
			case Static::LE_MOUSELOST:
				m_Static->SetState(0);
				break;
			case Static::LE_LBUTTONUP:
				{
					FlagDescScreen::Traits t;

					t.m_Image = m_Image.c_str();
					t.m_Text = m_Description.c_str();

					int x = Screens::Instance()->Journal()->GetLocation()->Region()->X();
					int y = Screens::Instance()->Journal()->GetLocation()->Region()->Y();

					Screens::Instance()->FlagDesc()->MoveTo(x+238,y+124);
					Screens::Instance()->FlagDesc()->DoModal(t);
				}
				break;
		}
	}
	//	��������� ����������� ����� �����
	static std::string GenerateName(void)
	{
		static int counter;
		static char buffer[50];

		sprintf(buffer,"flag%d",counter++);

		return std::string(buffer);
	}
public:
	const char* Type(void) const;
};

const char* NavigationSheet::Flag::m_WidgetType = "flag";
const char* NavigationSheet::Flag::m_ImagePath = "pictures/interface/JournalScreen/Levels/Flags/";

inline const char* NavigationSheet::Flag::Type(void) const
{
	return m_WidgetType;
}

/////////////////////////////////////////////////////////////////////////
///////////////////////    class RuntimeDialog    ///////////////////////
/////////////////////////////////////////////////////////////////////////
//	Naughty: 14.05.02 for bug fixing
/////////////////////////////////////////////////////////////////////////
class RuntimeDialog : public Dialog
{
public:
	RuntimeDialog(const char *pName,const char *pScriptName) : Dialog(pName,pScriptName) {}
	void OnSystemChange(void) { Widget::OnSystemChange(); }
};

//*********************************************************************//
//	class NavigationSheet
const char* NavigationSheet::m_pDialogName = "NavigationSheet";
const char* NavigationSheet::m_LevelImagePath = "pictures/interface/JournalScreen/Levels/";
const char* NavigationSheet::m_FlagsScript = "scripts/interface/Flags.txt";
const char* NavigationSheet::m_EpisodeMapDialogName = "episode_map";
const char* NavigationSheet::m_EpisodeMapScriptName = "MapEpisode";
NavigationSheet::NavigationSheet(JournalScreen* js) : Sheet(js,m_pDialogName,m_pDialogName),m_Controller(0),
													  m_EpisodeMap(0)
{
	//	��������� ������ ��������
	m_Sheets[S_LOCALMAP].m_Static	= static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"localmap"));
	m_Sheets[S_CITYMAP].m_Static	= static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"citymap"));
	//	�������� ������ �� �������� ����������
	m_LevelName = static_cast<Text*>(WidgetFactory::Instance()->Assert(this,"name"));
	m_LevelImage = WidgetFactory::Instance()->Assert(this,"level_map");
	m_LevelImage->Enable(true);
	//	������������ �������� ����������� ����� ������
	m_Sheets[S_LOCALMAP].m_Static->SetState(1);
	m_Sheets[S_LOCALMAP].m_Fixed = true;
}

NavigationSheet::~NavigationSheet()
{
}

JournalScreen::SheetController* NavigationSheet::GetController(void)
{
	return m_Controller;
}

void NavigationSheet::SetController(Controller* controller)
{
	m_Controller = controller;
}
//	���������� ������ ������������ ��������
void NavigationSheet::IlluminateSheet(SHEET id)
{
	for(int i=0;i<MAX_SHEET;i++)
	{
		m_Sheets[i].m_Static->SetState(0);
		m_Sheets[i].m_Fixed = false;
	}
	m_Sheets[id].m_Static->SetState(1);
	m_Sheets[id].m_Fixed = true;
}
//	����������� �� ��������� ��������� ��������
void NavigationSheet::OnStatic(Static *pStatic)
{
	for(int sheet=0;sheet<MAX_SHEET;sheet++)
	{
		if(m_Sheets[sheet].m_Static == pStatic)
		{
			switch(pStatic->LastEvent())
			{
			case Static::LE_MOUSEMOVE:
				if(!m_Sheets[sheet].m_Fixed) pStatic->SetState(1);
				break;
			case Static::LE_MOUSELOST:
				if(!m_Sheets[sheet].m_Fixed) pStatic->SetState(0);
				break;
			case Static::LE_LBUTTONDOWN:
				pStatic->SetState(2);
				break;
			case Static::LE_LBUTTONUP:
				if(!m_Sheets[sheet].m_Fixed)
				{
					for(int i=0;i<MAX_SHEET;i++)
					{
						m_Sheets[i].m_Static->SetState(0);
						m_Sheets[i].m_Fixed = false;
					}
					m_Sheets[sheet].m_Static->SetState(1);
					m_Sheets[sheet].m_Fixed = true;
					if(m_Controller) m_Controller->OnSheetClick(static_cast<SHEET>(sheet));
				}
				else
				{
					m_Sheets[sheet].m_Static->SetState(1);
					m_Sheets[sheet].m_Fixed = false;
				}
				break;
			}
			return;
		}
	}
}
//	���������� ��������� �������
void NavigationSheet::SetVisible(bool bVisible)
{
	//	������� ����� ������
	if(bVisible) CreateLevelMap(IWorld::Get()->GetLevel()->GetRawLevelName().c_str());
	//	�������� ���������� ������
	Dialog::SetVisible(bVisible);
}

//	������� ����� ������
void NavigationSheet::CreateLevelMap(const char* level)
{
	if(m_CurrentLevel != level)
	{
		m_CurrentLevel = level;
		//	������������� �������� ������
		if(m_LevelImage->GetImage()) m_LevelImage->GetImage()->Release();
		
		std::string level_image = m_LevelImagePath;
		level_image += level;
		level_image += ".tga";
		
		m_LevelImage->SetImage(DXImageMgr::Instance()->CreateImage(level_image.c_str()));
		//	���������� ��� ���������� ����� (�.� ��� �������� ������� m_ImageLevel)
		std::vector<Widget*> mortals;
		for(Widget::Iterator it=m_LevelImage->Begin();it!=m_LevelImage->End();it++)
			if(!strcmp(it->Type(),Flag::m_WidgetType)) mortals.push_back(it.operator->());
		for(std::vector<Widget*>::iterator iv = mortals.begin();iv!=mortals.end();iv++)
				m_LevelImage->Delete((*iv)->Name());
		//	��������� ���������� � ������
		TxtFile script(DataMgr::Load(m_FlagsScript));
		DataMgr::Release(m_FlagsScript);
		unsigned int i;

		if(script.FindInCol(level,&i,0))
		{
			for(;i<script.SizeY();i++)
			{
				if(script(i,0) != level) break;
				m_LevelImage->Insert(new Flag(script(i,1).c_str(),
											  script(i,2).c_str(),
											  script(i,3).c_str(),
											  script(i,4).c_str(),
											  script(i,5).c_str()));
			}
		}
	}
}

//=====================================================================================//
//                          void NavigationSheet::ShowSheet()                          //
//=====================================================================================//
//	�������� �� ��� ���� ��������
void NavigationSheet::ShowSheet(SHEET id)
{
	switch(id)
	{
	case S_LOCALMAP:
		//	������� ����� ���� ��� ��� �� ���� �������
		CreateLevelMap(IWorld::Get()->GetLevel()->GetRawLevelName().c_str());
		//	������������� ��� ������
		m_LevelName->SetText(IWorld::Get()->GetLevelDesc().c_str());
		//	������ ������� ��������
		m_LevelImage->SetVisible(true);
		//	������ ��������� �������� ����� �������
		m_EpisodeMap->SetVisible(false);
		break;
	case S_CITYMAP:
		//	������������� ������������ �������
		m_LevelName->SetText(GetEpisodeName(m_Episode).c_str());
		//	������ ������� �������� ����� �������
		m_EpisodeMap->SetVisible(true);
		//	������ ��������� �������� ������
		m_LevelImage->SetVisible(false);
		break;
	}
}
//	���������� ������� ������ (����������� ������������ ������)
void NavigationSheet::SetEpisode(unsigned int episode)
{
	char buff[50];

	m_Episode = episode;

	Delete(m_EpisodeMapDialogName);
	sprintf(buff,"%s%d",m_EpisodeMapScriptName,episode+1);
	Insert(m_EpisodeMap = new RuntimeDialog(m_EpisodeMapDialogName,buff),Dialog::L_LEFT_TOP);
	for(Widget::Iterator i=m_EpisodeMap->Begin();i!=m_EpisodeMap->End();i++)
		i->m_Tip.m_Activated = false;
	ClearEpisodeMap();
	m_EpisodeMap->SetVisible(false);
}
//	�������� ������� �����
void NavigationSheet::ShowEpisodePart(const char* level, TYPE type)
{
	if(m_EpisodeMap && level)
	{
		Static* part = static_cast<Static*>(m_EpisodeMap->Child(level));
		if(part)
		{
			part->SetVisible(true);
			part->SetState(type);
			part->m_Tip.m_Text = IWorld::Get()->GetLevelDesc();
		}
		WidgetFactory::Instance()->Assert(m_EpisodeMap,"background")->SetVisible(true);
	}
}
//	�������� ��� ����� �������
void NavigationSheet::ClearEpisodeMap(void)
{
    if(m_EpisodeMap)
    {
	    for(Widget::Iterator i=m_EpisodeMap->Begin();i!=m_EpisodeMap->End();i++)
		    i->SetVisible(false);
    }
}
//	�������� ������� �����
std::string NavigationSheet::CheckHotLine(void)
{
	int sheet=0;
	for(;sheet<MAX_SHEET;sheet++)
	{
		if(m_Sheets[sheet].m_Static->IsEnable() && m_Sheets[sheet].m_Fixed) 
			break;
	}
	switch(sheet)
	{
	case S_CITYMAP:
		return "navigation.citymap";
	case S_LOCALMAP:
		return "navigation.localmap";
	default:
		return "navigation";
	}
}
//	��������� ��������� �����
bool NavigationSheet::ProcessMouseInput(VMouseState* pMS)
{
	//	���������� ��� ��������� ������
	Widget* flag = FirstChildFromPoint(pMS->x,pMS->y); 

	if(flag && !strcmp(flag->Name(),"level_map"))
	{
		flag = flag->FirstChildFromPoint(pMS->x,pMS->y); 
		if(flag)
		{
			m_JournalScreen->SetHotLineText(flag->m_Tip.m_Text.c_str());
			return true;
		}
	}
	if(flag && !strcmp(flag->Name(),"episode_map"))
	{
		flag = flag->FirstChildFromPoint(pMS->x,pMS->y); 
		if(flag->m_Tip.m_Text.size())
		{
			m_JournalScreen->SetHotLineText(flag->m_Tip.m_Text.c_str());
		}
	}

	return true;
}
//	�������� ������������ ������� �� ������ �������
std::string NavigationSheet::GetEpisodeName(unsigned int episode) const
{
	switch(episode)
	{
	case 0: return MenuMgr::Instance()->GetStrResourse("js_episode0");
	case 1: return MenuMgr::Instance()->GetStrResourse("js_episode1");
	case 2: return MenuMgr::Instance()->GetStrResourse("js_episode2");
	case 3: return MenuMgr::Instance()->GetStrResourse("js_episode3");
	}

	return "Unknown episode";
}
//	����������� �� ��������� ��������� �������
void NavigationSheet::OnSystemChange(void)
{
	Widget::OnSystemChange();
}

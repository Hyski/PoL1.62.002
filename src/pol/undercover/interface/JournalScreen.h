/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   13.03.2001

************************************************************************/
#ifndef _JOURNAL_SCREEN_H_
#define _JOURNAL_SCREEN_H_

#include "Dialog.h"

class NavigationSheet;
class DatabaseSheet;
class JournalSheet;
class NewsSheet;
class MultiEdit;
class TextBox;
class Text;
class Edit;

extern const char* g_TechPictures;
extern const char* g_ForcesPictures;
extern const char* g_PeoplePictures;
extern const char* g_LocationPictures;

class JournalScreen : public Dialog
{
public:
	class SheetController;
	class HotLineControl;
	class Controller;
	class Sheet;
	//	������������ ��������
	enum SCREEN_ID
	{
		SID_NONE,
		SID_NEWS,						//	�������
		SID_DATABASE,					//	���� ������
		SID_JOURNAL,					//	�������
		SID_NAVIGATION					//	���������
	};
public:
	static const char* m_pDialogName;
private:
	Controller*			m_Controller;
	HotLineControl*		m_HotLine;
	NewsSheet*			m_News;
	DatabaseSheet*		m_Database;
	JournalSheet*		m_Journal;
	NavigationSheet*	m_Navigation;
	Sheet*				m_ActiveSheet;
	Text*				m_HLWidget;
public:
	JournalScreen();
	virtual ~JournalScreen();
public:
	//	�������������� ������������ �����
	void Activate(SCREEN_ID id); 
	//	�������� ��������� � ������� ��������
	NewsSheet* News(void);
	//	�������� ��������� � ������� ������������
	DatabaseSheet* Database(void);
	//	�������� ��������� � ������� ��������
	JournalSheet* Journal(void);
	//	�������� ��������� � ������� ���������
	NavigationSheet* Navigation(void);
public:
	//	���������� ����������
	void SetController(Controller* controller);
	//	������ ������ 
	enum BUTTON
	{
		B_UP,
		B_DOWN,
		B_LEFT,
		B_RIGHT,
		B_BACK,
		MAX_BUTTON
	};
	//	������ ������ �� ����������
	enum UPDATE_BUTTON
	{
		UB_DATABASE,
		UB_JOURNAL,
		UB_NAVIGATION,
		MAX_UPDATE_BUTTON
	};
	//	������ ��������
	enum SHEET
	{
		S_DATABASE,					//	���� ������
		S_JOURNAL,					//	�������
		S_NAVIGATION,				//	���������
		MAX_SHEET,
		S_NEWS						//	�������
	};
	struct FixedStatic 
	{
		Static* m_Static;
		bool m_Fixed;
		FixedStatic() : m_Static(0),m_Fixed(false) {}
	};
	//	������������ ������ ����������
	enum ACTIVATE_MODE {AM_ACTIVATE,AM_DEACTIVATE};
	void ActivateUpdateButton(UPDATE_BUTTON id,ACTIVATE_MODE mode);
	//	���������� ������ ������������ ��������
	void IlluminateSheet(SHEET id);
	//	���������/��������� ������ �� ��������� ����������
	void EnableControls(bool enable);
	//	���������� ��������� �������
	void SetVisible(bool bVisible);
	//	���������� ����� ������� ���������
	void SetHotLineText(const char* text);
	//	������� ��������� HotLine
	HotLineControl* HotLine(void) {return m_HotLine;}
private:
	Button* m_Buttons[MAX_BUTTON];
	FixedStatic m_Sheets[MAX_SHEET];
	FixedStatic m_UpdateButtons[MAX_UPDATE_BUTTON];
private:
	//	����������� �� ��������� ��������� �������
	void OnSystemChange(void);
	//	����������� �� ��������� ��������� ������
	void OnButton(Button *pButton);
	//	����������� �� ��������� ��������� ��������
	void OnStatic(Static *pStatic);
	//	��������� ������������� �����
	bool ProcessKeyboardInput(VKeyboardState* pKS);
	//	��������� ��������� �����
	bool ProcessMouseInput(VMouseState* pMS);
};

//*********************************************************************//
//	class JournalScreen::Controller
class JournalScreen::Controller
{
public:
	virtual ~Controller() {}
	virtual void OnSheetClick(JournalScreen::SHEET id) = 0;
	virtual void OnUpdateClick(JournalScreen::UPDATE_BUTTON id) = 0;
	virtual void OnBack(void) = 0;
};

//*********************************************************************//
//	class JournalScreen::SheetController
class JournalScreen::SheetController
{
public:
	virtual ~SheetController() {}
	virtual void OnButtonClick(JournalScreen::BUTTON id) = 0;
};

//*********************************************************************//
//	class JournalScreen::Sheet
class JournalScreen::Sheet : public Dialog
{
protected:
	JournalScreen* m_JournalScreen;
public:
	Sheet(JournalScreen* js,const char *pName,const char *pScriptName) : Dialog(pName,pScriptName),m_JournalScreen(js) {}
	virtual ~Sheet() {}
public:
	virtual SheetController* GetController(void) = 0;
	virtual std::string CheckHotLine(void) = 0;
protected:
	//	����������� �� ��������� ��������� �������
	virtual void OnSystemChange(void) {Widget::OnSystemChange();}
	//	����������� � ������ ������ ����������
	virtual void OnKeyboardLost(void) {}
};

//*********************************************************************//
//	class NewsSheet
class NewsSheet : public JournalScreen::Sheet
{
public:
	static const char* m_pDialogName;
	struct Traits
	{
		const char* m_Image;		//	��������� ��� ��������
		const char* m_Channel;		//	������������ ������
		const char* m_Content;		//	���������� ��������
	};
private:
	JournalScreen::SheetController* m_Controller;
	VRegion m_ImagePlace;
	Widget* m_Image;
	Text* m_Channel;
	TextBox* m_Content;
public:
	NewsSheet(JournalScreen* js);
	virtual ~NewsSheet();
public:
	void SetController(JournalScreen::SheetController* controller);
	void SetTraits(const Traits* t);
private:
	JournalScreen::SheetController* GetController(void);
	std::string CheckHotLine(void);
};

//*********************************************************************//
//	class DatabaseSheet
class DatabaseSheet : public JournalScreen::Sheet
{
public:
	class Controller;
	enum SHEET
	{
		S_OUTFIT,
		S_TECH,
		S_FORCES,
		S_PEOPLE,
		MAX_SHEET
	};
	struct ItemInfo
	{
		const char* m_Image;
		const char* m_Name;
		const char* m_LeftColumn;
		const char* m_RightColumn;
		const char* m_Text;
	};
public:
	static const char* m_pDialogName;
private:
	Controller* m_Controller;
	Widget* m_Image;
	Text* m_LeftColumn;
	Text* m_RightColumn;
	Text* m_ItemName;
	TextBox* m_Text;
public:
	DatabaseSheet(JournalScreen* js);
	virtual ~DatabaseSheet();
public:
	void SetController(Controller* controller);
	//	���������� ������ ������������ ��������
	void IlluminateSheet(SHEET id);
	//	���������/��������� ������ � ��������
	void EnableSheet(SHEET id,bool enable);
	//	���������� ���������� ��� ������ (0 - ��� ��������,sheet - ��� ����������� �������� � ����������)
	void SetItemInfo(const ItemInfo* ii,SHEET id = S_OUTFIT);
	//	���������� ������������ �������
	void SetSectionName(const char* name);
private:
	JournalScreen::FixedStatic m_Sheets[MAX_SHEET];
private:
	JournalScreen::SheetController* GetController(void);
	//	�������� ������� � ���������� ��� ������ ��������
	const char* GetPathForResource(SHEET id) const;
	//	����������� �� ��������� ��������� ��������
	void OnStatic(Static *pStatic);
	//	�������� ������� �����
	std::string CheckHotLine(void);
};

//*********************************************************************//
//	class DatabaseSheet::Controller
class DatabaseSheet::Controller : public JournalScreen::SheetController
{
public:
	virtual ~Controller() {}
	virtual void OnSheetClick(DatabaseSheet::SHEET id) = 0;
};

//*********************************************************************//
//	class JournalSheet
class JournalSheet : public JournalScreen::Sheet
{
public:
	class Controller;
	enum SHEET
	{
		S_NEWTHEME,
		S_DELTHEME,
		S_NEWENTRY,
		S_DELENTRY,
		MAX_SHEET,
		S_SAVE_THEME,
		S_SAVE_ENTRY
	};
public:
	static const char* m_pDialogName;
private:
	Controller* m_Controller;
	Text* m_ThemeName;
	Edit* m_ThemeEdit;
	TextBox* m_Content;
	MultiEdit* m_ContentEdit;
public:
	JournalSheet(JournalScreen* js);
	virtual ~JournalSheet();
public:
	//	���������� ����������
	void SetController(Controller* controller);
	//	���������� ��������� ����
	enum TEXT_FIELD {TF_THEME,TF_THEME_COUNT,TF_ENTRY_COUNT,TF_CONTENT};
	void SetTextField(const char* text,TEXT_FIELD tf);
	//	������ �������� ���� ��� ������
	enum SECTION {S_THEME,S_ENTRY};
	void EditSection(SECTION section,const char* text);
	//	������ ���������� ����� ��������������
	const char* GetSectionText(SECTION section) const; 
	//	���������� ����� ��������������
	bool IsEdit(void) const;
	//	�������� ��������������
	enum CONTINUE_MODE {CM_OK,CM_CANCEL};
	void ContinueEdit(CONTINUE_MODE cm);
	//	����� ��������� � ������
	void UnSelectSheet(SHEET id);
	//	���������/��������� ������ � ��������
	void EnableSheet(SHEET id,bool enable);
private:
	struct FixedStaticEx : public JournalScreen::FixedStatic
	{
		bool m_Enable;	//	���� ���� �������� ��� ������ �� �����/�����
		FixedStaticEx() : FixedStatic(),m_Enable(true) {}
	};
	FixedStaticEx m_Sheets[MAX_SHEET];
	enum SPECIAL_BUTTON {SB_OK,SB_CANCEL,MAX_SPECIAL_BUTTON};
	JournalScreen::FixedStatic m_SButtons[MAX_SPECIAL_BUTTON];
private:
	JournalScreen::SheetController* GetController(void);
	//	����������� �� ��������� ��������� ��������
	void OnStatic(Static *pStatic);
	//	����������� �� ��������� ���������
	void OnChange(Widget* pChild);
	//	���������/��������� ������ �� ��������� ����������
	void EnableControls(bool enable);
	//	�������� ������� �����
	std::string CheckHotLine(void);
};

//*********************************************************************//
//	class JournalSheet::Controller
class JournalSheet::Controller : public JournalScreen::SheetController
{
public:
	virtual ~Controller() {}
	virtual void OnSheetClick(SHEET id) = 0;
};

//*********************************************************************//
//	class NavigationSheet
class NavigationSheet : public JournalScreen::Sheet
{
	class Flag;
public:
	class Controller;
	enum SHEET
	{
		S_LOCALMAP,
		S_CITYMAP,
		MAX_SHEET
	};
public:
	static const char* m_pDialogName;
	static const char* m_LevelImagePath;
	static const char* m_FlagsScript;
	static const char* m_EpisodeMapDialogName;
	static const char* m_EpisodeMapScriptName;
private:
	Controller* m_Controller;
	Text* m_LevelName;
	Widget* m_LevelImage;
	std::string m_CurrentLevel;
	Dialog* m_EpisodeMap;
	unsigned int m_Episode;
public:
	NavigationSheet(JournalScreen* js);
	virtual ~NavigationSheet();
public:
	void SetController(Controller* controller);
	//	���������� ������ ������������ ��������
	void IlluminateSheet(SHEET id);
	//	�������� �� ��� ���� ��������
	void ShowSheet(SHEET id);
	//	���������� ������� ������ (����������� ������������ ������)
	void SetEpisode(unsigned int episode);
	//	�������� ������� �����
	enum TYPE {T_NORMAL,T_SELECTED};
	void ShowEpisodePart(const char* level,TYPE type);
	//	�������� ��� ����� �������
	void ClearEpisodeMap(void);
private:
	JournalScreen::FixedStatic m_Sheets[MAX_SHEET];
private:
	JournalScreen::SheetController* GetController(void);
	//	����������� �� ��������� ��������� ��������
	void OnStatic(Static *pStatic);
	//	������� ����� ������
	void CreateLevelMap(const char* level);
	//	���������� ��������� �������
	void SetVisible(bool bVisible);
	//	�������� ������� �����
	std::string CheckHotLine(void);
	//	��������� ��������� �����
	bool ProcessMouseInput(VMouseState* pMS);
	//	�������� ������������ ������� �� ������ �������
	std::string NavigationSheet::GetEpisodeName(unsigned int episode) const;
	//	����������� �� ��������� ��������� �������
	void OnSystemChange(void);
};

//*********************************************************************//
//	class NavigationSheet::Controller
class NavigationSheet::Controller : public JournalScreen::SheetController
{
public:
	virtual ~Controller() {}
	virtual void OnSheetClick(SHEET id) = 0;
};

#endif	//_JOURNAL_SCREEN_H_
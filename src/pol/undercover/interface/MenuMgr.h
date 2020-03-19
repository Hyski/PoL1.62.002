/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   02.03.2001

************************************************************************/
#ifndef _MENU_MGR_H_
#define _MENU_MGR_H_

#include <Common/UI/Widget.h>
#include "../ModMgr.h"
#include <Muffle/ISound.h>

class ISndThemeSession;
class LoadingScreen;
class OptionsMenu;
class LoadingMenu;
class GameScreen;
class MainMenu;
class GameMenu;
class Dialog;
class DXView;
class Text;

class MenuMgr : public Widget
{
public:
	class ic_State;
	class ic_MouseCursor;
	class ic_LoadLevelModifier;
	class ic_LoadSaveGameModifier;
private:
	class ic_Tip;
	class ic_InterfaceTip;
private:
	struct Deleter 
	{
	public:
		MenuMgr *m_pInstance;
	public:
		Deleter(){m_pInstance = 0;}
		~Deleter(){if(m_pInstance) delete m_pInstance;}
	};
	friend Deleter;
	static Deleter m_Deleter;
private:
	DXView *m_pDXView;						//	������ ����������� ��������
	int m_bLockInput;						//	���� ���������� ����� 
	ic_Tip* m_Tip;							//	����������� ���������
	ic_InterfaceTip* m_InterfaceTip;		//	���������� ����������� ����������
	ic_State* m_State;						//	����� ������ ����������
	//	������������
	ic_LoadLevelModifier* m_LoadLevelModifier;			//	�� �������� ������
	ic_LoadSaveGameModifier* m_LoadSaveGameModifier;	//	�� �������� ����������� ����
	//	����� ��� ������������ � ������� ����
	Widget::Player::Sample* m_RainSound;
	Muffle::HThemeSession m_AmbientSound;
	//	�����-���� ������ ����������
	Text* m_Version;
	//	������� ���
	const Mod m_currentMod;			
	//	�������� ������
	static ic_MouseCursor* m_MouseCursor;
public:
	static const int m_MainMenuResX;
	static const int m_MainMenuResY;
	static const std::string &GetStartLevel();
private:
	MenuMgr();
	virtual ~MenuMgr();
public:
	//	��������� �������� ����
	void Render(void);
	void Draw(WidgetView* /*pView*/){}
	//	��������� �����
	void OnMouse(void);
	void OnKeyboard(void);
	//	��������� ������� �� �������� ����
	void OnChange(Widget* pChild);
	//	������������ �� ����� ����� ������
	void OnChangeVideoMode(void);
	//	��������� ����� �����
	bool Tick(void);
public:
	//	�������� ������
	void LoadLevel(const char* name);
	//	���������� �������c ���� �������� ������
	void UpdateLoadingProgressBar(float value);
	//	���������� ��������� �������c ���� �������� ������
	void SetProgressBarTitle(const char* title);
	//	��������/������� ������ ����������� ����������
	void MakeSaveLoad(GenericStorage& store);
	//	������� �� ����� MessageBox
	int QuestionBox(const char* text);
	//	����� �� ������� �� ����� MessageBox
	bool IsQBFree(void);
	//	������ ��������� �� Tip'����
	ic_Tip* Tip(void);
	//	������������� ���� �� ����������
	void LockInput(bool lock);
	//	������������ �� ���� �� ����������
	bool IsLockInput(void);
	//	�������� �����������, ��� ������ ����� ����������
	bool IsCameraMoveable(void);
	//	������� ����� �� ���������
	void ExitFromProgram(void);
	//	������������ ���� ��������
	void Release(void);
public:
	//	�������� �� ��� ���� ����
	void ShowDialog(const char *pName);
	//	����� �� ����
	void ExitFromGame(void);
	//	�������� ��������� ������
	std::string GetStrResourse(const std::string& name);
	//	�������� ������� ����������� ������ ����
	Text* Version(void) const;
public:
	ic_LoadLevelModifier* LoadLevelModifier(void);
	ic_LoadSaveGameModifier* LoadSaveGameModifier(void);
	//	�������� ��� ������������
	void ResetModifiers(void);
	//	�������� �������� ������
	static ic_MouseCursor* MouseCursor(void) {return m_MouseCursor;}
	//	�������� ��������� �� ������ ��������������� ��������
	WidgetView* GetDXView(void) {return reinterpret_cast<WidgetView*>(m_pDXView);}
	//	�������� ����� ���� 
	Widget::Player::Sample* RainSound(void) {return m_RainSound;}
	void PlayAmbientSound(void);
	void StopAmbientSound(void);
public:
	//	���������� � ������
	bool IsLevelKeyPresent(void) const;
private:
	void OnConsole(Widget* pChild);
public:
	//	�������� ����������� ����� ������ �������� ��������� ����
	enum CHECK_VIDEO_MODE {CVM_FOR_MAINMENU,CVM_FOR_CFG};
	static void CheckVideoMode(CHECK_VIDEO_MODE cvm);
public:
	static MenuMgr *Instance(void);
};

inline MenuMgr* MenuMgr::Instance(void)
{
	if(!m_Deleter.m_pInstance) m_Deleter.m_pInstance = new MenuMgr();
	return m_Deleter.m_pInstance;
}
//	������������ �� ���� �� ����������
inline bool MenuMgr::IsLockInput(void)
{
	return m_bLockInput > 0;
}
//	������ ��������� �� Tip'����
inline MenuMgr::ic_Tip* MenuMgr::Tip(void)
{
	return m_Tip;
}
//	������� ��������� �� LoadLevelModifier
inline MenuMgr::ic_LoadLevelModifier* MenuMgr::LoadLevelModifier(void)
{
	return m_LoadLevelModifier;
}
//	������� ��������� �� LoadSaveGameModifier
inline MenuMgr::ic_LoadSaveGameModifier* MenuMgr::LoadSaveGameModifier(void)
{
	return m_LoadSaveGameModifier;
}
//	�������� ������� ����������� ������ ����
inline Text* MenuMgr::Version(void) const
{
	return m_Version;
}

//***************************************************************//
//	class MenuMgr::ic_Tip 
class MenuMgr::ic_Tip
{
private:
	static const int m_TieX;
	static const int m_TieY;
	Text* m_Text;
public:
	ic_Tip(Widget* pParent);
	virtual ~ic_Tip();
public:
	void Activate(bool on);
	void SetColor(unsigned int color);
	void MoveTo(int x,int y);
	void SetText(const char* text);
};

//***************************************************************//
//	class MenuMgr::ic_State 
class MenuMgr::ic_State
{
public:
	enum MODE {M_EXTERNAL,M_INTERNAL};
protected:
	MenuMgr* m_MenuMgr;
	bool m_Destroy;								//	���� ���������������
public:
	ic_State(MenuMgr* mm) : m_MenuMgr(mm),m_Destroy(false) {}
	virtual ~ic_State() {}
public:
	//	�������� ���������� �� �������� ����
	virtual bool OnChange(Widget* pChild) = 0;
	//	�������� ���������� 
	virtual void OnTick(void) = 0;
	//	���������� �������� ����
	virtual void UpdateLoadingProgressBar(float value) = 0;
	//	���������� ��������� �������c ���� �������� ������
	virtual void SetProgressBarTitle(const char* title) = 0;
	//	���������� ����� ������ ����
	virtual MenuMgr::ic_State::MODE Mode(void) = 0;
	//	��������� �������
	virtual void LoadLevel(const char* name) = 0;
public:
	bool IsDestroy(void) {return m_Destroy;}
};

//***************************************************************//
//	class MenuMgr::ic_LoadLevelModifier 
class MenuMgr::ic_LoadLevelModifier
{
private:
	std::string m_LevelName;
	bool m_Available;								//	���� �����������
public:
	ic_LoadLevelModifier() : m_Available(false) {}
	virtual ~ic_LoadLevelModifier() {}
public:
	void SetLevelName(const char* level_name) {m_LevelName = level_name;}
	void SetAvailable(bool available) {m_Available = available;}
	const std::string& LevelName(void) const {return m_LevelName;}
	bool IsAvailable(void) const {return m_Available;}
};

//***************************************************************//
//	class MenuMgr::ic_LoadSaveGameModifier 
class MenuMgr::ic_LoadSaveGameModifier
{
public:
	enum AVAILABLE {A_NONE,A_LOAD,A_SAVE};
private:
	std::string m_SaveGameName;
	std::string m_SaveGameDesc;
	AVAILABLE m_Available;								//	���� �������
public:
	ic_LoadSaveGameModifier() : m_Available(A_NONE) {}
	virtual ~ic_LoadSaveGameModifier() {}
public:
	void SetSaveGameName(const char* name) {m_SaveGameName = name;}
	void SetSaveGameDesc(const char* desc) {m_SaveGameDesc = desc;}
	void SetAvailable(AVAILABLE available) {m_Available = available;}
	const std::string& SaveGameName(void) const {return m_SaveGameName;}
	const std::string& SaveGameDesc(void) const {return m_SaveGameDesc;}
	AVAILABLE Available(void) const {return m_Available;}
};

//***************************************************************//
//	class MenuMgr::ic_MouseCursor
class MenuMgr::ic_MouseCursor
{
	class Params;
public:
	enum CURSOR_TYPE
	{
		CT_NORMAL,
		CT_CANMOVE,
		CT_CANNOTMOVE,
		CT_CANATTACK,
		CT_CANNOTATTACK,
		CT_CANUSE,
		CT_SELECT,
		CT_UNLOAD,
		CT_SHIPMENT,
		CT_SPECIAL,
		CT_PICKUP,
		CT_SWAP,
		MAX_CURSORS,

		CT_MEDIC = CT_PICKUP
	};

	struct TextField
	{
		const char* m_Text;
		unsigned int m_Color;
		TextField() : m_Text(0),m_Color(0) {}
		TextField(const char* text,unsigned int color) : m_Text(text),m_Color(color) {}
	};
	enum {MAX_TEXT_FIELDS = 4};
private:
	struct CursorInfo
	{
		std::string m_ImageName;		//	��� ��������
		int m_Width;					//	������ 
		int m_Height;					//	������
		int m_LinkX;					//	����� �������� 
		int m_LinkY;					//	����� ��������
		ipnt2_t m_TextXY[MAX_TEXT_FIELDS];			//	����� �������� ��� ������
	};
private:
	static const char* m_CursorsPath;
	LPDIRECTDRAWSURFACE7 m_Surface;
	CursorInfo m_Cursors[MAX_CURSORS];
	RECT m_rcBorder;
	RECT m_rcCursor;
	int m_CursorX;
	int m_CursorY;
	bool m_Visible;
//	CURSOR_TYPE m_Type;
	Text* m_TextFields[MAX_TEXT_FIELDS];
	ipnt2_t m_TextFieldSize;
	std::string m_Font;
	enum {P_OLD,P_NEW,P_MAX};
	Params* m_Params[P_MAX];
public:
	ic_MouseCursor();
	virtual ~ic_MouseCursor();
public:
	//	���������� ������� �������
	void SetPos(int x,int y);
	//	���������� ������� �������� �������
	void SetBorder(int left,int top,int right,int bottom);
	//	���������� ��� ������� � ��������� ����
	void SetCursor(CURSOR_TYPE type,
				   const TextField *t1 = 0,
				   const TextField *t2 = 0,
				   const TextField *t3 = 0,
				   const TextField *t4 = 0);
	//	����������� � ����� ����� ������
	void OnChangeVideoMode(void);
	//	���������� ������
	void Render(void);
	//	��������� �������
	void SetVisible(bool visible);

	bool IsVisible() const { return m_Visible; }
private:
	void TextMoveTo(Widget* widget,int x,int y);
};

#endif	//_MENU_MGR_H_
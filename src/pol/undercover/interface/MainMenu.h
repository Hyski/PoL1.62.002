/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   02.03.2001

************************************************************************/
#ifndef _MAIN_MENU_H_
#define _MAIN_MENU_H_

#include "Dialog.h"
#include <common/Shell/Shell.h>
#include <Muffle/ISound.h>

class ISndThemeSession;
class TextBox;

//********************************************************************//
//*******************      class MainMenu     ************************//
class MainMenu : public Dialog
{
public:
	static const char* m_pDialogName;
public:
	enum ACTIVE_EVENT
	{
		AE_NONE,
		AE_GAME,
		AE_OPTIONS,
		AE_CINEMATICS,
		AE_CREDITS,
		AE_EXIT
	};
private:
	ACTIVE_EVENT m_MenuEvent;
public:
	MainMenu();
	virtual ~MainMenu();
public:
	MainMenu::ACTIVE_EVENT GetEvent(void){return m_MenuEvent;}
private:
	void OnButton(Button* pButton);
	//	�������� ����� CD
	bool CheckCDVolume(void);
};

//********************************************************************//
//*******************      class GameMenu     ************************//
class GameMenu : public Dialog
{
public:
	static const char* m_pDialogName;
public:
	enum ACTIVE_EVENT
	{
		AE_NONE,
		AE_NEW,
		AE_LOAD,
		AE_MULTIPLAYER,
		AE_TRAINING,
		AE_BACK
	};
private:
	ACTIVE_EVENT m_MenuEvent;
public:
	GameMenu();
	~GameMenu();
public:
	GameMenu::ACTIVE_EVENT GetEvent(void){return m_MenuEvent;}
protected:
	void OnButton(Button* pButton);
};

//********************************************************************//
//******************      class VideoMenu     ************************//
class VideoMenuImp;
class VideoMenu : public Dialog
{
private:
	class VMIObserver;
public:
	static const char* m_pDialogName;
private:
	VMIObserver* m_VMIObserver;
	VideoMenuImp* m_Imp;
public:
	VideoMenu();
	~VideoMenu();
private:
	unsigned int GetEpisodeAdvance(void) const;
};

/////////////////////////////////////////////////////////////////////////
//////////////////////////    class Credits    //////////////////////////
/////////////////////////////////////////////////////////////////////////
class CreditsMenu : public Dialog
{
public:
	static const char* m_pDialogName;
	static const char* m_TilesScript;
	static const char* m_CreditsScript;
	static const float m_Speed;							//	�������� �������� ������
private:
	std::vector<WidgetImage*> m_Tiles;		//	������ ��������		
	Muffle::HThemeSession m_AmbientSound;	//	����
	unsigned int m_FontHeight;				//	������ ������
	std::string m_Part1;					
	std::string m_Part2;
	TextBox* m_Content;						//	����� �����������
	bool m_EndOfShow;
	float m_TileChangingTime;				//
	float m_Seconds;
	int m_CurTile;
public:
	CreditsMenu();
	~CreditsMenu();
private:
	//	���������� �����-�� ��������
	bool Tick(void);
	//	���������� ��������� �������
	void SetVisible(bool bVisible);
	//	����������� �� ��������� ��������� �������
	void OnSystemChange(void);
	//	��������� �� �����
	bool CheckExit(void);

};

//********************************************************************//
//*******************    class OptionsMenu    ************************//
class OptionsScreen;
class OptionsMenuController;
class OptionsMenu : public Dialog
{
private:
	OptionsScreen* m_Options;
	OptionsMenuController* m_Controller;
public:
	static const char* m_pDialogName;
public:
	OptionsMenu();
	virtual ~OptionsMenu();
public:
	//	���������� ��������� �������
	void SetVisible(bool bVisible);
};

//********************************************************************//
//	LoadingMenu 
class LoadingMenuController;
class LoadingMenu : public Dialog
{
private:
	OptionsScreen* m_Options;
	LoadingMenuController* m_Controller;
public:
	static const char* m_pDialogName;
public:
	LoadingMenu();
	virtual ~LoadingMenu();
};

//***************************************************************//
//	QuestionBox 
class QuestionBox : public Dialog, private ModalLoop
{
public:
	static const char* m_pDialogName;
private:
	int m_Result;
public:
	QuestionBox();
	virtual ~QuestionBox();
public:
	int DoModal(const char* text);
private:
	void OnButton(Button* pButton);
	//	����������� �� ��������� ��������� �������
	void OnSystemChange(void);
	//	���������� �����-�� ��������
	bool Tick(void);
};

#endif	//_MAIN_MENU_H_
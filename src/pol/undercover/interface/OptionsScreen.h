/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   22.03.2001

************************************************************************/
#ifndef _OPTIONS_SCREEN_H_
#define _OPTIONS_SCREEN_H_

#include "Dialog.h"

class OptionsScreenController;
class OptionsWrapper;
class Slide;
class Text;
class Edit;

class OptionsScreen : public Dialog
{
public:
	static const char* m_pDialogName;
public:
	class Video;
	class Audio;
	class System;
	class Camera;
	class SaveLoad;
	class TipOfTheDay;
	//	������ ����
	enum MODE 
	{
		M_MAINMENU,
		M_GAMEMENU,
		M_LOADINGMENU
	};
	//	����������� ��� ��������
	enum PARENT_NOTIFY
	{
		PN_NONE,
		PN_EXIT,
		PN_SAVELOAD
	};
private:
	Video* m_Video;
	Audio* m_Audio;
	System* m_System;
	Camera* m_Camera;
	TipOfTheDay* m_TipOfTheDay;
	SaveLoad* m_SaveLoad;
	OptionsScreenController* m_Controller;
	OptionsWrapper* m_Updater;
private:
	enum BUTTON
	{
		B_EXIT,
		B_BACK,
		B_SAVE,
		B_LOAD,
		B_AUDIO,
		B_VIDEO,
		B_SYSTEM,
		B_CAMERA,
		B_OK,
		B_CANCEL,
		MAX_BUTTON
	};
	//	������ � ����
	Button* m_Buttons[MAX_BUTTON];
	//	������� ����� ������ ����
	MODE m_Mode;
	//	������� ������������ �����������
	PARENT_NOTIFY m_ParentNotify;
private:
	Widget* m_pCurrentlyVisible;
public:
	OptionsScreen();
	virtual ~OptionsScreen();
public:
	void SetController(OptionsScreenController* controller);
public:
	//	����������� �� ��������� ��������� �������
	void OnSystemChange(void);
	//	��������� ������������� �����
	bool ProcessKeyboardInput(VKeyboardState* pKS);
	//	����������� �� ��������� ��������� ��������� �������
	void OnChange(Widget* pChild);
	//	����������� �� ��������� ��������� ������
	void OnButton(Button *pButton);
	//	�������� �����
	void Reset(void);
	//	���������� ����� ������ ����
	void SetMode(MODE mode);
	//	�������� ����������� �����������
	PARENT_NOTIFY GetParentNotify(void) {return m_ParentNotify;}
	//	������� ������ ��� ����� ��� �����
	static std::string GetFullFileNameOfSave(const char* save_file_name);
	//	������� ������� ��� �������� ������ ����������
	static void MakeSavesDir(void);
private:
	//	������������ �������� � ������ ����
	void OrginizeSaveLoad(void);
	//	�������� �� ���������� ����
	bool OptionsScreen::CheckResaving(void);
};
//***********************************************************************//
//****************** class OptionsScreen::Video *************************//
class OptionsScreen::Video : public Dialog
{
private:
	OptionsWrapper* m_Updater;
private:
	enum SLIDER {SHADOWS,EFFECTS,GAMMA,MAX_SLIDER};
	enum SWITCH {MARKS,LIGHTS,ENVIRONMENT,MAX_SWITCH};
private:
	Slide* m_Sliders[MAX_SLIDER];
	Static* m_Switches[MAX_SWITCH];
public:
	static const char* m_pDialogName;
public:
	Video(OptionsWrapper* updater);
	virtual ~Video();
public:
	//	����� ���� ����������
	void Reset(void);
	//	����������� �� ��������� ��������� �������
	void OnSystemChange(void);
	//	����������� 
	void OnKeyboardLost(void){}
	void OnStatic(Static *pStatic);
	void OnChange(Widget* pChild);
};

//***********************************************************************//
//****************** class OptionsScreen::Audio *************************//
class OptionsScreen::Audio : public Dialog
{
private:
	OptionsWrapper* m_Updater;
private:
	enum SLIDER {VOICES,AMBIENT,EFFECTS,MUSIC,MAX_SLIDER};
private:
	Slide* m_Sliders[MAX_SLIDER];
public:
	static const char* m_pDialogName;
public:
	Audio(OptionsWrapper* updater);
	virtual ~Audio();
public:
	//	����� ���� ����������
	void Reset(void);
	//	����������� �� ��������� ��������� �������
	void OnSystemChange(void);
	//	����������� 
	void OnKeyboardLost(void){}
	void OnChange(Widget* pChild);
};

//***********************************************************************//
//****************** class OptionsScreen::System *************************//
class OptionsScreen::System : public Dialog
{
private:
	OptionsWrapper* m_Updater;
public:
	static const char* m_pDialogName;
private:
	enum SLIDER {ANIMA_SPEED,CAMERA_SPEED,MOUSE_SENSITIVITY,MAX_SLIDER};
	enum SWITCH {PATH,HEXGRID,MAX_SWITCH};
private:
	Slide* m_Sliders[MAX_SLIDER];
	Static* m_Switches[MAX_SWITCH];
public:
	System(OptionsWrapper* updater);
	virtual ~System();
public:
	//	����� ���� ����������
	void Reset(void);
	//	����������� �� ��������� ��������� �������
	void OnSystemChange(void);
	//	����������� 
	void OnKeyboardLost(void){}
	void OnStatic(Static *pStatic);
	void OnChange(Widget* pChild);
};

//***********************************************************************//
//****************** class OptionsScreen::Camera ************************//
class OptionsScreen::Camera : public Dialog
{
private:
	OptionsWrapper* m_Updater;
public:
	static const char* m_pDialogName;
public:
	enum SWITCH 
	{
		ENEMY_MOVEMENT,		ENEMY_FIRE,
		NEUTRAL_MOVEMENT,	NEUTRAL_FIRE,
		ALLY_MOVEMENT,		ALLY_FIRE,
		CIVILIAN_MOVEMENT,	CIVILIAN_FIRE,
		VEHICLE_MOVEMENT,	VEHICLE_FIRE,
		MAX_SWITCH
	};
private:
	Static* m_Switches[MAX_SWITCH];
public:
	Camera(OptionsWrapper* updater);
	virtual ~Camera();
public:
	//	����� ���� ����������
	void Reset(void);
	//	����������� �� ��������� ��������� �������
	void OnSystemChange(void);
	//	����������� 
	void OnKeyboardLost(void){}
	void OnStatic(Static *pStatic);
	void OnChange(Widget* pChild);
private:
	SWITCH GetStaticId(Static* s);
};

//***********************************************************************//
//	class OptionsScreen::TipOfTheDay
class OptionsScreen::TipOfTheDay : public Dialog
{
public:
	static const char* m_pDialogName;
private:
	static const char* m_pTipsScript;
public:
	TipOfTheDay();
	virtual ~TipOfTheDay();
public:
	//	����������� �� ��������� ��������� �������
	void OnSystemChange(void);
	//	����������� 
	void OnKeyboardLost(void){}
	//	���������� ��������� �������
	void SetVisible(bool bVisible);
};

//***********************************************************************//
//****************** class OptionsScreen::Audio *************************//
class OptionsScreen::SaveLoad : public Dialog
{
public:
	class Strategy;
public:
	static const char* m_SavesPath;
	static const char* m_SavesName;
	static const char* m_SavesExt;
	static const char* m_DashName;
	static const int m_MaxLength; 
	static const int m_MaxSlot;
private:
	Text* m_Slots[8];
	Text* m_Times[8];
	Widget* m_Selection;
	Edit* m_Edit;
	int m_SelPos;
	int m_HidedSlot;
public:
	static const char* m_pDialogName;
public:
	enum MODE {M_NONE,M_SAVE,M_LOAD};
private:
	MODE m_Mode;
	Strategy* m_Strategy;
public:	
	SaveLoad();
	virtual ~SaveLoad();
public:
	//	����������� �� ��������� ��������� �������
	void OnSystemChange(void);
	//	����������� 
	void OnKeyboardLost(void){}
	//	���������� ����� ������ ����
	void SetMode(MODE mode);
	//	�������� ����� ������ ����
	OptionsScreen::SaveLoad::MODE GetMode(void) const;
	//	�������� ��� ����� ��� ��������
	bool GetSelection(std::string& file_name,std::string& file_desc);
	//	��������� ��������� ��� �����
	void SetSelectionToMouse(void);
	//	�������� ������ �������������� ��� ������
	void EnableEdit(bool enable);
	//	������� ��������
	void MakeLoad(void);
	//	���������� ������
	void OnCanceled(void);
	//	�������� ������ ���
	void FillSlots(void);
	//	��������� ���������� �� ����
	bool IsResavingSelection(void);
	//	��������� �� �������� �� ������� ���� autosave ��� quicksave
	bool IsCurrentSlotAvailable(void);
	//	��������� �������� � ��� ��� ���������� ������� ����/����
	void Report(void);
	//	������� ������ ��� ����� ��� �����
	static std::string GetFullFileNameOfSave(const char* save_file_name);
private:
	bool GetSaveDesc(const char* file_name,std::string& desc);
	void UpdateSelection(void);
	//	��������� ��������� �����
	bool ProcessMouseInput(VMouseState* pMS);
	//	��������� ������������� �����
	bool ProcessKeyboardInput(VKeyboardState* pKS);
	//	��������� ��������� �� �������� ��������
	void OnChange(Widget* pChild);
};

//	�������� ����� ������ ����
inline OptionsScreen::SaveLoad::MODE OptionsScreen::SaveLoad::GetMode(void) const
{
	return m_Mode;
}

class OptionsScreen::SaveLoad::Strategy
{
protected:
	SaveLoad* m_Owner;
public:
	Strategy(SaveLoad* owner) : m_Owner(owner) {}
	virtual ~Strategy(){}
	virtual void OnClick(void) = 0;
	virtual void OnDblClk(void) = 0;
	virtual void OnReturn(void) = 0;
	virtual void OnUpDown(void) = 0;
};

//***********************************************************************//
//****************** class OptionsScreen::Audio *************************//
class OptionsScreenController
{
public:
	virtual ~OptionsScreenController(){}
	virtual void OnBack(void) = 0;
};

//***********************************************************************//
//******************   class OptionsUpdater     *************************//
class OptionsUpdater
{
public:
	virtual ~OptionsUpdater(){}
	virtual void SetMarksOnWalls(bool value) = 0;
	virtual void SetDynamicLights(bool value) = 0;
	virtual void SetEnvironment(bool value) = 0;
	virtual void SetPath(bool value) = 0;
	virtual void SetHexGrid(bool value) = 0;
	virtual void SetShadowsQ(float value) = 0;
	virtual void SetEffectsQ(float value) = 0;
	virtual void SetGamma(float value) = 0;
	virtual void SetVoicesVolume(float value) = 0;
	virtual void SetAmbientVolume(float value) = 0;
	virtual void SetEffectsVolume(float value) = 0;
	virtual void SetMusicVolume(float value) = 0;
	virtual void SetAnimaSpeed(float value) = 0;
	virtual void SetCameraSpeed(float value) = 0;
	virtual void SetMouseSensitivity(float value) = 0;
	//	����� ����� ��� HiddenMovement
	virtual void SetEnemyMovement(bool value) = 0;
	virtual void SetNeutralMovement(bool value) = 0;
	virtual void SetAllyMovement(bool value) = 0;
	virtual void SetCivilianMovement(bool value) = 0;
	virtual void SetVehicleMovement(bool value) = 0;
	virtual void SetEnemyFire(bool value) = 0;
	virtual void SetNeutralFire(bool value) = 0;
	virtual void SetAllyFire(bool value) = 0;
	virtual void SetCivilianFire(bool value) = 0;
	virtual void SetVehicleFire(bool value) = 0;
public:
	virtual bool GetPath(void) = 0;
	virtual bool GetHexGrid(void) = 0;
};

/*
class OptionsUpdater
{
public:
	enum TYPE
	{
		T_MARKS_ON_WALLS,
		T_DYNAMIC_LIGHT,
		T_ENVIRONMENT,
		T_PATH,
		T_HEX_GRID,
		T_SHADOW_Q,
		T_EFFECTS_Q,
		T_GAMMA,
		T_VOICES_VOLUME,
		T_AMBIENT_VOLUME,
		T_EFFECTS_VOLUME,
		T_MUSIC_VOLUME,
		T_ANIMA_SPEED,
		T_CAMERA_SPEED,
		T_MOUSE_SENSITIVITY
	};
public:
	virtual ~OptionsUpdater() {}
	virtual float GetValue(TYPE type) = 0;
	virtual void  SetValue(TYPE type,float value) = 0;
};
*/

#endif	//_OPTIONS_SCREEN_H_
/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   12.03.2001

************************************************************************/
#ifndef _INVENTORY_SCREEN_H_
#define _INVENTORY_SCREEN_H_

#include "Dialog.h"

//#define _DEBUG_SLOTS_ENGINE

class InventoryScreenController;
class MMgrController;
class SlotScroller;
class SlotsEngine;
class BaseThing;
class SESlot;
class Text;

//=====================================================================================//
//                        class InventoryScreen : public Dialog                        //
//=====================================================================================//
class InventoryScreen : public Dialog
{
//	class DSController;
public:
	static const char* m_pDialogName;
	static const char* m_PortraitPath;
	static const char* m_PortraitExt;
private:
	InventoryScreenController* m_Controller;
	MMgrController* m_MMgrController;
//	DSController* m_DSController;
	SlotScroller* m_GroundScroller;
	SlotsEngine* m_SlotsEngine;
public:
	InventoryScreen();
	virtual ~InventoryScreen();
public:
//****************** ������� ��������� � ������� *********************//
	//	���������� ���������� ������ ��� ����������
	void SetController(InventoryScreenController* controller);
	InventoryScreenController *GetController() const { return m_Controller; }
//	*******************************************************************
//	������ ���������� ���������� ****
	enum BUTTON 
	{
		B_DISMISS,
		B_UNLOAD,
		B_BACK,
		B_INFO,
		B_PREV_HERO,
		B_NEXT_HERO,
		MAX_BUTTON
	};
	// ��������/������ ������
	void EnableButton(BUTTON id,bool enable);
	//	*******************************************************************
	enum SCROLLER
	{
		S_GND_TO_LEFT,
		S_GND_TO_RIGHT,
		MAX_SCROLLER
	};
	//	*******************************************************************
	enum LEVEL_UP
	{
		LU_STRENGTH,
		LU_DEXTERITY,
		LU_REACTION,
		LU_ACCURACY,
		LU_WISDOM,
		LU_MECHANICS,
		LU_SIGHT,
		LU_HEALTH,
		MAX_LEVEL_UP
	};
	enum LEVEL_UP_SIGN
	{
		LUS_PLUS,					//	������ ����
		LUS_MINUS					//	������ �����
	};
	void ShowLevelUp(bool show);
	void ShowLevelUp(LEVEL_UP index,LEVEL_UP_SIGN sign,bool show);
	//********* ������� � ���� *********************//
	struct HeroTraits
	{
		std::string m_NextLevel;
		int m_Experience;
		int m_Level;
		int m_LevelUpPoints;
		int m_Strength;
		int m_Dexterity;
		int m_Reaction;
		int m_Accuracy;
		int m_Wisdom;
		int m_Mechanics;
		int m_Sight;
		int m_ShockRes;
		int m_FireRes;
		int m_ElectricRes;
		int m_MoveCurrent;
		int m_MoveMax;
		int m_HealthMax;
		int m_MoralCurrent;
		int m_MoralMax;
		int m_Money;
		int m_WeightCurrent;
		int m_WeightMax;

		int m_MaxStrength;
		int m_MaxDexterity;
		int m_MaxReaction;
		int m_MaxAccuracy;
		int m_MaxWisdom;
		int m_MaxMechanics;
		int m_MaxSight;
		int m_MaxHealth;
		
		std::string m_Rank;
	};
	//	���������� ��������� ��������	
	void SetHeroTraits(const HeroTraits& t);
//	*******************************************************************
	struct PortraitTraits
	{
		const char* m_portrait_name;		//	��������� ��� �����
		const char* m_hero_name;			//	�� ��������� ��� �����
	};
	void SetPortraitTraits(const PortraitTraits& t);
//	*******************************************************************
	enum SLOT
	{
		S_GROUND = 0,
		S_BACKPACK = 1,
		S_KNEE_LEFT = 2,
		S_KNEE_RIGHT = 3,
		S_HANDS = 4,
		S_BODY = 5,
		S_IMPLANT = 6,
		S_HEAD = 7,
		MAX_SLOT = 8,
		S_NONE = 9
	};
public:
	//	�������� ��������� �� �������� ������ �� �������
	SlotsEngine* GetSlotsEngine(void) const {return m_SlotsEngine;}
	//	�������������/�������������� ������
	void BlockButtons(bool block);
private:
	//	������� ����� ��� ��������� ������
	void CreateSlots(void);
	//	������ ������
	Button* m_Buttons[MAX_BUTTON];
	//	������ ����������
	Static* m_Scrollers[MAX_SCROLLER];
	//	������ LevelUp'��
	struct LevelUpSigns
	{
		Button* m_Plus;
		Button* m_Minus;
		Static* m_Place;
	} m_LevelUps[MAX_LEVEL_UP];
	//	*******************************************************************
private:
	//	����������� �� ��������� ��������� �������
	void OnSystemChange(void);
	//	���������� �����-�� ��������
	bool Tick(void);

#ifdef _DEBUG_SLOTS_ENGINE
	//	����������� ������� � ���� �������� ��������
	void Render(WidgetView* pView);
#endif
	//	��������� �� ������
	void OnButton(Button *pButton);
	//	��������� �� ��������
	void OnStatic(Static* pStatic);
};

class SEItem;
class BaseThing;
class HumanEntity;

//**********************************************************************//
//*********	       class InventoryScreenController         *************//
class InventoryScreenController
{
public:
	virtual ~InventoryScreenController(){};
public:
	virtual void OnButtonClick(InventoryScreen::BUTTON id) = 0;
	virtual void OnLevelUpClick(InventoryScreen::LEVEL_UP id,InventoryScreen::LEVEL_UP_SIGN sign) = 0;
	virtual void OnLevelUpDescription(InventoryScreen::LEVEL_UP id) = 0;
	virtual void UpdateAllContent(HumanEntity *) = 0;
	virtual HumanEntity *GetCurrentHuman() = 0;
	virtual void GiveItemTo(HumanEntity *, SEItem *) = 0;
};

/////////////////////////////////////////////////////////////////////////
////////////////////////    class SlotScroller    ///////////////////////
/////////////////////////////////////////////////////////////////////////
class SlotScroller
{
	static const float m_Pause;
	static const float m_Speed;
public:
	enum DIRECTION {D_LEFT,D_RIGHT,D_UP,D_DOWN};
private:
	SESlot* m_Slot;
	float m_Start;
	float m_Seconds;
public:
	SlotScroller(SESlot* slot);
	~SlotScroller();
public:
	void Start(DIRECTION dir);
	void Scroll(DIRECTION dir);
private:
	void Translate(DIRECTION dir,int* dx,int* dy);
};

#endif	//_INVENTORY_SCREEN_H_
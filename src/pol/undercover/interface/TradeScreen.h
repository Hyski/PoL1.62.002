/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   13.03.2001

************************************************************************/
#ifndef _TRADE_SCREEN_H_
#define _TRADE_SCREEN_H_

#include "Dialog.h"

class TradeScreenController;
class CommodityItem;
class SlotScroller;
class SlotsEngine;

class TradeScreen : public Dialog
{
public:
	static const char* m_pDialogName;
	static const char* m_PortraitPath;
	static const char* m_PortraitExt;
private:
	TradeScreenController* m_Controller;
	SlotsEngine* m_SlotsEngine;
	CommodityItem* m_CommodityItem;
	SlotScroller* m_GroundScroller;
	SlotScroller* m_TradeScroller;
public:
	TradeScreen();
	virtual ~TradeScreen();
public:
	//	����������� �� ��������� ��������� �������
	void OnSystemChange(void);
private:
	void OnChange(Widget* pChild);
	void OnButton(Button *pButton);
	void OnStatic(Static *pStatic);
public:
//****************** ������� ��������� � ������� *********************//
	//	���������� ���������� ������ ��� ����������
	void SetController(TradeScreenController* controller);
	//	�������� ��������� �� �������� ������ �� �������
	SlotsEngine* GetSlotsEngine(void) const {return m_SlotsEngine;}
//  **** ������ ���������� ���������� ****
	enum BUTTON 
	{
		B_UNLOAD,
		B_BACK,
		B_PREV_HERO,
		B_NEXT_HERO,
		B_PREV_THING,
		B_NEXT_THING,
		B_BUY,
		B_SELL,
		MAX_BUTTON
	};
	//	��������/������ ������
	void EnableButton(BUTTON id,bool enable);
	enum SCROLLER
	{
		S_TRADE_TO_UP,
		S_TRADE_TO_DOWN,
		S_GND_TO_LEFT,
		S_GND_TO_RIGHT,
		MAX_SCROLLER
	};
	enum SHEET
	{
		S_GUNS,
		S_AMMO,
		S_EQUIP,
		S_GREN,
		S_ARMOR,
		MAX_SHEET
	};
	//	��������/������ ��������
	void EnableSheet(SHEET id,bool enable);
	enum SHEET_STATE {SS_NORMAL,SS_PUSHED};
	void SetSheetState(SHEET sheet,SHEET_STATE ss);
	enum SLOT
	{
		S_TRADE,
		S_GROUND,
		S_BACKPACK,
		S_KNEE_LEFT,
		S_KNEE_RIGHT,
		S_HANDS,
		S_BODY,
		S_IMPLANT,
		S_HEAD,
		MAX_SLOT,
		S_NONE
	};
	//	��������� ������� �������� �������� �� �������
	enum COMMODITY_STATE
	{
		CS_NORMAL		=    0,
		CS_HIGHLIGHTED	= 1<<1,
		CS_GRAYED		= 1<<2,
		CS_NORANK		= 1<<3
	};
	struct CommodityTraits
	{
		std::string m_Name;						//	��� ��������
		std::string m_SystemName;				//	����/������������� ��������
		std::string m_Text;						//	��������� ���� �� ��������
		std::string m_Cost;						//	����
		std::string m_BuyRank;					//	����������� ������
		unsigned int m_State;					//	��������� - (������������ ��� ���)
		int m_Quantity;							//	���-�� ��������� � ��������
	};
	void SetCommodityTraits(const CommodityTraits& ct);
	//	�������� ��������
	struct TraderTraits
	{
		const char* m_Portrait;
		const char* m_Description;
	};
	void SetTraderTraits(const TraderTraits& t);
	//	�������� ���������
	struct HeroTraits
	{
		int m_WeightCurrent;
		int m_WeightMax;
	};
	//	���������� ��������� ��������	
	void SetHeroTraits(const HeroTraits& t);
	//	��������� ������� � �������
	enum TEXT
	{	
		T_HERO_NAME,
		T_HERO_MONEY,
		T_SALE_COST,
		T_HERO_RANK
	};
	void SetText(TEXT id,const char* value);
private:
	//	������� ����� ��� ��������� ������
	void CreateSlots(void);
	//	������ ������
	Button* m_Buttons[MAX_BUTTON];
	//	������ ����������
	Static* m_Scrollers[MAX_SCROLLER];
	//	������ ��������
	Static* m_Sheets[MAX_SHEET];
#ifdef _DEBUG_SLOTS_ENGINE
	//	����������� ������� � ���� �������� ��������
	void Render(WidgetView* pView);
#endif
};

//**********************************************************************//
//	class TradeScreenController
class TradeScreenController
{
public:
	enum HANGING_MODE {HM_START,HM_FINISH};
	virtual ~TradeScreenController(){};
public:
	virtual void OnCommodityClick(void) = 0;
	virtual void OnCommodityHanging(HANGING_MODE hm) = 0;
	virtual void OnButtonClick(TradeScreen::BUTTON id) = 0;
	virtual void OnSheetClick(TradeScreen::SHEET id) = 0;
};

#endif	//_TRADE_SCREEN_H_
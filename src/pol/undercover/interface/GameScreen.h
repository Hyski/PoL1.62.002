/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   07.03.2001

************************************************************************/
#ifndef _GAME_SCREEN_H_
#define _GAME_SCREEN_H_

#include "Dialog.h"

class GameScreenController;
class VehicleEntity;
class HumanEntity;
class WeaponIcon;
class TextBox;
class Marker;
class Static;
class Text;

//=====================================================================================//
//                          class GameScreen : public Dialog                           //
//=====================================================================================//
class GameScreen : public Dialog
{
public:
	class EnemyIconMgr;
	class HackVehicleMgr;
	class PersonIconMgr;
public:
	static const char* m_pDialogName;
	static const char* m_WeaponIconName;
private:
	class MessageWindow;
	MessageWindow* m_pMsgWindow;
	class VehicleMonitor;
	struct VM
	{
		VehicleMonitor* m_pMonitor;
		bool m_bVisible;
	} m_pVehicleMonitors[6];
	GameScreenController *m_pController;
	WeaponIcon* m_WeaponIcon;
	bool m_IsCameraMoveable;				//	����, ��������� � ����������� ����������� ������
	bool m_bUnloadOptionsScreen;
	//	�������� ������ ������
	EnemyIconMgr* m_EnemyIconMgr;
	//	�������� ������ �������� �������
	HackVehicleMgr* m_HackVehicleMgr;
	//	�������� ������ �� ���������
	PersonIconMgr* m_PersonIconMgr;
	//	������� �������� ������
	Widget* m_CurrentActiveDilog;
public:
	GameScreen();
	virtual ~GameScreen();

	virtual bool ProcessMouseInput(VMouseState *pMS);
public:
	//	���������� �����-�� ��������
	bool Tick(void);
public:
	//	������� �������� ���������� ���� (0 - ������ ��� ����)
	void ShowDialog(const char *pName);
	//	����������� �� ��������� ��������� �������
	void OnSystemChange(void);
	//	��������� ���� �����
	void LoadOptionsScreen(void);
	//	��������� ���� �����
	void UnloadOptionsScreen(void);
	//	��������/�������� �������� ����������
	void ShowControls(bool bShow);
	//	�������� �����������, ��� ������ ����� ����������
	bool IsCameraMoveable(void);
	//	�������� ��������� �� ���� ���������
	MessageWindow* MsgWindow(void);
	//	����� � ������� ����
	void ExitToMainMenu(void);
	//	��������/��������� ���� "��������������" � "������� ����������" (���� ����� ��� return false)
	enum SPECIAL_SAVELOAD_TYPE {/*SST_AUTOSAVE,*/SST_QUICKSAVE};
	enum SPECIAL_SAVELOAD_MODE {SSM_LOAD,SSM_SAVE};
	bool MakeSpecialSaveLoad(SPECIAL_SAVELOAD_TYPE type, SPECIAL_SAVELOAD_MODE mode);
public:
	//	��� ������ �� ����������� �������
	void OpenScriptSceneScreen(void);
	void CloseScriptSceneScreen(void);
public:
/****************** ������� ��������� � ������� *********************/
	//	���������� ���������� ������ ��� ����������
	void SetController(GameScreenController *pController);
	GameScreenController *GetController() const { return m_pController; }

	//	**** ������ ������������ � ������ ****
	enum JOURNAL_BUTTON_STATE {JBS_NORMAL,JBS_SELECTED,JBS_BLINK};
	void SetJournalButtonState(JOURNAL_BUTTON_STATE jbs);
	//  **** ������� ������ ****
	enum BUTTON
	{
		B_RELOAD,
		B_UP_ARROW,
		B_DOWN_ARROW,
		MAX_BUTTON
	};
	// ������������ ������ � ��������� Enable/Disable
	void EnableButtonState(BUTTON id,bool bEnable);
	//  **** ������ ���������� ���������� ****
	enum CHECKBOX {CB_RUN,
				   CB_SIT,
				   CB_SPECIAL,
				   CB_USE,
				   CB_EYES,
				   CB_ROOF,
				   CB_JOURNAL,
				   CB_ENDTURN,
				   CB_MENU,
				   CB_FOS,
				   CB_AUTOSHOT,
				   CB_SNAPSHOT,
				   CB_AIMSHOT,
				   CB_BACKPACK,
				   MAX_CHECKBOX};
	enum CHECKBOX_STATE {CBS_ON=1,CBS_OFF=0};
	enum BN_CLICK {BNC_NONE,BNC_LCLICK,BNC_RCLICK,BNC_LDBLCLICK,BNC_RDBLCLICK};
	// ������������ ������ � �������� ���������
	void SetCBState(CHECKBOX id,CHECKBOX_STATE state);
	// �������� ��������� ������	
	CHECKBOX_STATE GetCBState(CHECKBOX id);
	// ������������ ������ � ��������� Enable/Disable
	void EnableCBState(CHECKBOX id,bool bEnable);
	//	����� ��������� ��� ��������� ������
	PersonIconMgr* GetPersonIconMgr(void) const {return m_PersonIconMgr;}
	//  **** ������ ��� ��������� �������� ****
	struct BodyIconTraits
	{
		enum BODY_PARTS_STATE {BPS_NONE,BPS_GREEN,BPS_BLUE,BPS_YELLOW,BPS_RED};
		enum DAMAGE_TYPE {DT_ELECTRIC	= 0x01,
						  DT_SHOCK		= 0x02,
					      DT_FLAME		= 0x04};
		//	-------------------------------------------------------
		BODY_PARTS_STATE m_Head;	//	��������� ��� ������
		BODY_PARTS_STATE m_Body;	//	��������� ��� ������
		BODY_PARTS_STATE m_Hands;	//	��������� ��� ���
		BODY_PARTS_STATE m_Legs;	//	��������� ��� ���
		unsigned int m_Damages;		//	����� �� ���������
	};
	void SetBodyIcon(const BodyIconTraits& bit);
	//	��������� ��� ������������ �������� ��� ���������� ������������� ��������
	struct ShootMovePoints
	{//	������ ������ ������������� �����������, 0 - ������������ ��������
		const char* m_AimMP;	unsigned int m_AimColor;
		const char* m_SnapMP;	unsigned int m_SnapColor;
		const char* m_AutoMP;	unsigned int m_AutoColor;
	};
	void SetShootMovePoints(const ShootMovePoints& smp);
	//  **** ������ ��� ������� ****
	struct VehicleIconTraits
	{
		enum SEL_TYPE {ST_NONE,ST_NORMAL};
		enum VEHICLE_ICON_FLAG {VIF_INMOVEMENT = 0x01};
		//	-------------------------------------------------------
		unsigned int m_Flags;					//	��������� ������ (�����) VEHICLE_ICON_FLAG
		bool m_Visible;							//	�����/�� �����
		int m_MaxCrew;							//	������������ ���-�� �������
		int m_NumCrew;							//	������� ���-�� �������
		int m_MovePoints;						//	���������
		int m_HitPoints;						//	���-�� ����� � �������
		unsigned int m_Value;					//	�������� ���������
		SEL_TYPE m_Selection;					//	��� ���������
	};
	void SetVehicleIcon(unsigned int icon,const VehicleIconTraits& vit);
	//  **** ������ ��� ������****
	//	���������� �������� ��� ������ (0 - �� ���������� ������)
	struct WeaponIconTraits
	{
		const char* m_ImageName;				//	��������� ��� �������� �� ������
		const char* m_AmmoQuantity;				//	���-�� �������� � ������
		const char* m_AmmoDesc;					//	������ �������� ��������
		WeaponIconTraits() : m_ImageName(0),m_AmmoQuantity(0),m_AmmoDesc(0) {}
		WeaponIconTraits(const char* image_name,const char* ammo_quantity,const char* ammo_desc) : m_ImageName(image_name),m_AmmoQuantity(ammo_quantity),m_AmmoDesc(ammo_desc) {}
	};
	//	���������� �������� ��� ������ (0 - �� ���������� ������)
	void SetWeaponIcon(const WeaponIconTraits* traits);
	bool IsCursorOnWeaponIcon(void);
	//	**** ��������� �������� ��������� ������ ****
	EnemyIconMgr* GetEnemyIconMgr(void) const {return m_EnemyIconMgr;}
	int GetEnemyIconBottom(void);
	int GetEnemyIconTop(void);
	void OnEnemyIconClick(Marker* marker);
	//	**** ��������� �������� �������� ������� ****
	HackVehicleMgr* GetHackVehicleMgr(void) const {return m_HackVehicleMgr;}
	int GetHackVehicleIconLeft(void);
	int GetHackVehicleIconRight(void);
	void OnHackVehicleIconClick(VehicleEntity* ve,GameScreen::BN_CLICK click_type);
//****************************************************************************//
private:
	struct DoubleCheckBox
	{//	���������� ��� ������������� � ���������� ���-�������
		Static* m_VisualArea;	//	�� ��� ����� �� ������, �����������
		Static* m_HotArea;		//	�������, ���� ���� ������� ������
	};
	DoubleCheckBox m_CheckBoxes[MAX_CHECKBOX];
	Static* m_Buttons[MAX_BUTTON];
	enum MP_TEXT_FIELD {MTF_AIM,MTF_SNAP,MTF_AUTO,MAX_MTF};
	Text* m_MPTextFields[MAX_MTF];
public:
	void OnChange(Widget* pChild);
	void OnStatic(Static *pStatic);
};

inline GameScreen::MessageWindow* GameScreen::MsgWindow(void)
{
	return m_pMsgWindow;
}

//**********************************************************************//
//*********	              class MessageWindow              *************//
class GameScreen::MessageWindow : public Dialog
{
private:

	class StringBuffer;				//	�����, ������������ �� �������� �����

private:
	float m_iMaxHeight;				//	����������� ������ ����
	float m_iMinHeight;				//	����������� ������ ����
	float m_iCurHeight;				//	������� ��������� ����
	float m_iAimHeight;				//	������ ������� ���������� �������
	const float m_MovingStep;
private:
	std::auto_ptr<StringBuffer> m_StringBuffer;		//	����� �����
	TextBox *m_MessageBox;							//	���� � �������
	int m_iMessageBoxHeight;						//	������ ���� � �������
public:
	MessageWindow();
	virtual ~MessageWindow();
public:
	//	����������� �� ��������� ��������� �������
	void CheckConformMode(void);
public:
	void OnStatic(Static *pStatic);
public:
	//	��������� ��������� ����
	void IncMsgWindow(void);
	//	��������� ��������� ����
	void DecMsgWindow(void);
	//	������ - ���� ��������� ��� ���
	bool IsPulled(void);
	//	�������� ����� � ����
	void AddText(const char* pText);
	//	�������� ������
	void Clear(void);

	virtual void SetVisible(bool v);
protected:
	//	������� ���� ���� ���������
	bool Tick(void);
};

//	������ - ���� ��������� ��� ���
inline bool GameScreen::MessageWindow::IsPulled(void)
{
	return !(m_iAimHeight == m_iMinHeight);
}

//**********************************************************************//
//*********	              class VehicleMonitor             *************//
class GameScreen::VehicleMonitor : public Dialog
{
private:
	static const char* m_pScriptName;				//	��� �������
	enum {MAX_PLACES = 6};
	Static* m_Places[MAX_PLACES];
	static const int m_PlacesX;
	static const int m_PlacesY;
	static const int m_PlacesWidth;
	static const int m_PlacesHeight;
public:
	VehicleMonitor(const char *pName);
	virtual ~VehicleMonitor();
public:
	//	����������� �� ��������� ��������� �������
	void OnSystemChange(void);
	//	��������� �� �������
	void OnStatic(Static *pStatic);
public:
	void SetMaxCrew(int value);
	void SetNumCrew(int value);
	void SetMovePoints(int value);
	void SetHealth(int value);
	void EnableInMovement(bool bEnable);
public:
	virtual const char* Type(void) const {return "vehiclemonitor";};
};

//**********************************************************************//
//*********	       class GameScreenController              *************//
class GameScreenController
{
public:
	virtual ~GameScreenController(){};
public:
	// ������� �� ���� ����, ���������� ������� ��������� ������
	virtual void OnCheckBoxClick(GameScreen::CHECKBOX id) = 0;
	// ������� �� ���� ����, ���������� ������� ��������� ������
	virtual void OnButtonClick(GameScreen::BUTTON id) = 0;
	// ������� �� ���� ���� �� ������ ���������
	virtual void OnIconClick(unsigned int id,GameScreen::BN_CLICK click_type) = 0;
	// ������� �� ���� ���� �� ������ ������
	virtual void OnWeaponIconClick(void) = 0;
	// ������� �� ���� ���� �� ������ �������
	virtual void OnVehicleIconClick(unsigned int icon_number,GameScreen::BN_CLICK click_type) = 0;
	// ������� �� ���� ���� �� ������ �����
	virtual void OnEnemyIconClick(Marker* marker) = 0;
	// ������� �� ���� ���� �� ������ �����
	virtual void OnHackVehicleClick(VehicleEntity* ve,GameScreen::BN_CLICK click_type) = 0;
};

//**********************************************************************
//	class GameScreen::EnemyIconMgr
class GameScreen::EnemyIconMgr
{
public:
	class EnemyIcon;
	class EnemyIconExtended;
	class EnemyIconAsWidget;
private:
	static int m_Bottom;							//	������ ������� (�� ����������)
	static int m_Top;								//	������� ������� 
private:
	std::list<EnemyIconExtended*> m_Icons;
	GameScreen* m_Owner;								//	����� GameScreen
public:
	EnemyIconMgr(GameScreen* owner);
	virtual ~EnemyIconMgr();
public:
	void Insert(EnemyIcon& ei);
	void Remove(Marker* marker);
	void Clear(void);
	EnemyIcon* ItemAt(const Marker* marker);
	EnemyIcon* ItemAt(unsigned int number);
	//	���������� ����� ��� ���� ������ ������
	void SetFlags(unsigned int flags);
	//	�������� ������������ ������
	void UpdateLocation(void);
private:
	ipnt2_t GetNextLocation(unsigned int size);
};

//**********************************************************************
//	class GameScreen::EnemyIconMgr::EnemyIcon
class GameScreen::EnemyIconMgr::EnemyIcon
{
public:
	enum FLAGS
	{
		F_NONE		= 0,
		F_SELECTED	= 1<<0
	};
protected:
	std::string m_Tip;
	Marker* m_Marker;
	unsigned int m_Flags;
public:
	//	����������� �����������
	EnemyIcon(const EnemyIcon& ei);
	//	����������� �������������
	EnemyIcon(const std::string& tip = "",Marker* marker = 0,unsigned int flags = F_NONE);
	//	����������
	virtual ~EnemyIcon();
public:
	std::string GetTip(void) const;
	unsigned int GetFlags(void) const;
	virtual void SetFlags(unsigned int flags);
	Marker* GetMarker(void) const;
};

//**********************************************************************
//	class GameScreen::EnemyIconMgr::EnemyIconExtended
class GameScreen::EnemyIconMgr::EnemyIconExtended : public GameScreen::EnemyIconMgr::EnemyIcon
{
private:
	GameScreen* m_Owner;					//	������������ ������
	EnemyIconAsWidget* m_Icon;				//	������, ������������ ������
public:
	EnemyIconExtended(GameScreen* owner,const GameScreen::EnemyIconMgr::EnemyIcon& ei);
	virtual ~EnemyIconExtended();
public:
	void SetFlags(unsigned int flags);
	void MoveTo(const ipnt2_t& pt);
	void SetNumber(int number);
public:
	//	����������� �� ������
	void OnClick(void);
};

//**********************************************************************
//	class GameScreen::EnemyIconMgr::EnemyIconAsWidget
class GameScreen::EnemyIconMgr::EnemyIconAsWidget : public Widget
{
public:
	static const char* m_EnemyIconName;
	static const char* m_EnemyIconSelectedName;
	static const char* m_NameDesc;
private:
	static int m_Width;
	static int m_Height;
private:
	Static* m_Icon;							//	������, ������������ ������
	Text* m_Number;							//	��������� ���� ��� ����������� ������ ������
	bool m_Selected;
	EnemyIconExtended* m_Link;				//	����� ������� � �������
public:
	EnemyIconAsWidget(EnemyIconExtended* link);
	virtual ~EnemyIconAsWidget();
public:
	//	���������� ����� ������
	void SetNumber(const char* number);
	//	����������/����� ���������
	void SetSelection(bool enable);
	//	������� ������� ������
	static int Width(void) {return m_Width;}
	static int Height(void) {return m_Height;}
private:
	std::string GenerateName(void);
	//	��������� �� �������� ����
	void OnChange(Widget* pChild);
};

//**********************************************************************
//	class GameScreen::HackVehicleMgr
class GameScreen::HackVehicleMgr
{
public:
	class Icon;
	class IconExtended;
	class IconAsWidget;
private:
	static int m_Right;							//	������� �������
	static int m_Left;							//	������ ������� (�� ����������)
private:
	std::list<IconExtended*> m_Icons;
	GameScreen* m_Owner;								//	����� GameScreen
public:
	HackVehicleMgr(GameScreen* owner);
	virtual ~HackVehicleMgr();
public:
	void Insert(Icon& icon);
	void Remove(VehicleEntity* ve);
	void Clear(void);
	Icon* ItemAt(const VehicleEntity* ve);
	Icon* ItemAt(unsigned int number);
	//	���������� ����� ��� ���� ������ ������
	void SetFlags(unsigned int flags);
	//	�������� ������������ ������
	void UpdateLocation(void);
private:
	ipnt2_t GetNextLocation(unsigned int size);
};


//**********************************************************************
//	class GameScreen::HackVehicleMgr::Icon
class GameScreen::HackVehicleMgr::Icon
{
public:
	enum FLAGS
	{
		F_NONE		= 0,
		F_SELECTED	= 1<<0
	};
protected:
	std::string m_Tip;
	VehicleEntity* m_VehicleEntity;
	unsigned int m_Flags;
public:
	//	����������� �����������
	Icon(const Icon& icon) : m_Tip(icon.m_Tip),m_VehicleEntity(icon.m_VehicleEntity),m_Flags(icon.m_Flags) {}
	//	����������� �������������
	Icon(const std::string& tip = "",VehicleEntity* ve = 0,unsigned int flags = F_NONE) : m_Tip(tip),m_VehicleEntity(ve),m_Flags(flags) {}
	//	����������
	virtual ~Icon() {}
public:
	std::string GetTip(void) const {return m_Tip;}
	unsigned int GetFlags(void) const {return m_Flags;}
	virtual void SetFlags(unsigned int flags) {m_Flags = flags;}
	VehicleEntity* GetVehicleEntity(void) const {return m_VehicleEntity;}
};


//**********************************************************************
//	class GameScreen::HackVehicleMgr::IconExtended
class GameScreen::HackVehicleMgr::IconExtended : public GameScreen::HackVehicleMgr::Icon
{
private:
	GameScreen* m_Owner;					//	������������ ������
	IconAsWidget* m_Icon;					//	������, ������������ ������
public:
	IconExtended(GameScreen* owner,const GameScreen::HackVehicleMgr::Icon& icon);
	virtual ~IconExtended();
public:
	void SetFlags(unsigned int flags);
	void MoveTo(const ipnt2_t& pt);
	void SetNumber(int number);
public:
	//	����������� �� ������
	void OnClick(GameScreen::BN_CLICK btn_clk);
};


//**********************************************************************
//	class GameScreen::HackVehicleMgr::IconAsWidget
class GameScreen::HackVehicleMgr::IconAsWidget : public Widget
{
public:
	static const char* m_IconName;
	static const char* m_IconSelectedName;
	static const char* m_NameDesc;
private:
	static int m_Width;
	static int m_Height;
private:
	Static* m_Icon;							//	������, ������������ ������
	Text* m_Number;							//	��������� ���� ��� ����������� ������ ������
	bool m_Selected;
	IconExtended* m_Link;					//	����� ������� � �������
	int m_lastHealth;				//	��������
	int m_lastMovepnts;			//	���� ����
public:
	IconAsWidget(IconExtended* link);
	virtual ~IconAsWidget();
public:
	//	���������� ����� ������
	void SetNumber(const char* number);
	//	����������/����� ���������
	void SetSelection(bool enable);
	//	������� ������� ������
	static int Width(void) {return m_Width;}
	static int Height(void) {return m_Height;}
private:
	std::string GenerateName(void);
	//	��������� �� �������� ����
	void OnChange(Widget* pChild);
	//	������� ����������� ���������
	std::string AssemblyTip(int h,int mp);
	//	���������� �������� �� ��������� ���������
	bool Tick(void);
};

//**********************************************************************
//	class GameScreen::PersonIconMgr
class GameScreen::PersonIconMgr
{
public:
	class Icon;
	class Monitor;
	//	�������� ��������
	struct MonitorTraits
	{
		enum FLAGS 
		{	
			F_HEALTH		= 1<<0,
			F_PORTRAIT		= 1<<1,
			F_SELECTION		= 1<<2,
			F_ICONFLAGS		= 1<<3,
			F_MOVEPOINTS	= 1<<4,
			F_HEARTBEAT		= 1<<5,
			F_MAXHEALTH		= 1<<6,
			F_MAXMOVEPOINTS	= 1<<7
		};
		unsigned int m_Flags;		//	����� ����� ������� ������� ���������
		//	��� ���������
		enum SEL_TYPE {ST_NONE,ST_NORMAL,ST_DEAD};
		SEL_TYPE m_Selection;		//	��� ���������
		//	��� �����������
		enum HEARTBEAT {HB_NONE,HB_CALM,HB_EXITED,HB_WOUND,HB_DEAD};
		HEARTBEAT m_Heartbeat;		//	��� �����������
		//	��������� ������ (�����)
		enum ICON_FLAG {IF_INVEHICLE  = 0x01,
						IF_LEVELUP    = 0x02,
						IF_INMOVEMENT = 0x04,
						IF_INPANIC	  = 0x08
					   };
		unsigned int m_StateFlags;	//	��������� ������ (�����)
		int m_MovePoints;			//	���-�� ����������
		int m_Health;				//	������� ��������
		int m_MaxMovePoints;			//	���-�� ����������
		int m_MaxHealth;				//	������� ��������
		std::string m_Portrait;		//	��� �������� �� ������� ���������
		MonitorTraits() : m_Flags(0) {}
		MonitorTraits(const MonitorTraits& t) : m_Flags(t.m_Flags),m_Selection(t.m_Selection),
			m_Health(t.m_Health),m_MaxHealth(t.m_MaxHealth),
			m_Portrait(t.m_Portrait),m_Heartbeat(t.m_Heartbeat),
			m_StateFlags(t.m_StateFlags),
			m_MovePoints(t.m_MovePoints), m_MaxMovePoints(t.m_MaxMovePoints) {}
	};
public:
	static unsigned int m_MaxMonitors;					//	������������ ���-�� ���������
	enum REMOVE_TYPE {RT_QUICK,RT_SLOW};
private:
	GameScreen* m_Owner;								//	����� GameScreen
	std::list<Monitor*> m_Monitors;						//	�������� ����������
	std::list<Monitor*> m_MonitorViews;					//	�������� ���������� �� ������
	std::list<Icon*> m_Icons;
public:
	PersonIconMgr(GameScreen* owner);
	virtual ~PersonIconMgr();
public:
	//	�������� ������ (������ �����������)
	void InsertIcon(const Icon& icon);
	//	�������� �������� ������ �� �������������� HumanEntity
	Icon* IconAt(const HumanEntity* he);
	//	�������� �������� ������ �� ����������� ������
	Icon* IconAt(unsigned int number);
	//	������� ������  (������ �����������)
	void RemoveIcon(const HumanEntity* he,REMOVE_TYPE rt);
	//	������� ��� ������ 
	void RemoveAll(void);
public:
	//	���
	void Tick(void);
	//	�������������� ���������� ���������
	void SwapMonitors(void);
};

//**********************************************************************
//	class GameScreen::PersonIconMgr::Icon
class GameScreen::PersonIconMgr::Icon
{
private:
	friend class PersonIconMgr;

	HumanEntity* m_HumanEntity;					//	�������� �������������
	Monitor* m_Monitor;							//	������� ��������� � �������
	MonitorTraits m_MonitorState;				//	������������ ��������� ��������
public:
	Icon(HumanEntity* he = 0);
	Icon(const Icon& icon,GameScreen::PersonIconMgr::Monitor* monitor);
	virtual ~Icon();
public:
	HumanEntity* GetHumanEntity(void) const {return m_HumanEntity;}
	Monitor* GetMonitor(void) const {return m_Monitor;}
	//	���������� ��������� ��������
	void SetHealth(int health);
	void SetMaxHealth(int h);
	void SetMovePoints(int mp);
	void SetMaxMovePoints(int mp);
	void SetHeartbeat(MonitorTraits::HEARTBEAT hb);
	void SetStateFlags(unsigned int flags);
	void SetPortrait(const char* name,MonitorTraits::SEL_TYPE selection);
	//	�������� ���������� � �����
	void Tick(void);
public:
	//	������� ������ � ���������� ������ this � ������
	void Remove(REMOVE_TYPE rt);
};

//**********************************************************************//
//	class GameScreen::PersonIconMgr::Monitor
class GameScreen::PersonIconMgr::Monitor : public Dialog
{
public:
	enum COMMAND {C_OPEN,C_QUICK_CLOSE,C_SLOW_CLOSE,C_CHANGE_STATE};
	//**********************************************************************//
	//	class Strategy
	class Strategy
	{
	protected:
		GameScreen::PersonIconMgr::Monitor* m_Controller;
	public:
		Strategy(GameScreen::PersonIconMgr::Monitor* controller) : m_Controller(controller) {}
		virtual ~Strategy() {}
		virtual void Tick(void) = 0;
		virtual COMMAND Type(void) = 0;
	};
	class OpenStrategy;
	class QuickCloseStrategy;
	class SlowCloseStrategy;
private:
	static const char* m_ScriptName;				//	������������ �������
public:
	//static const float m_MaxWidth;					//	������������ ������ ��������
	static const float m_MinWidth;					//	����������� ������ ��������
	static const float m_MovingStep;				//	��� ��������
	static const float m_LevelUpFlipTime;			//	������� ������� LevelUp'�
private:
	bool m_IsLevelUp;								//	���� ��������� ����� ��'�
	float m_LevelUpSeconds;
private:
	Text* m_MovePoints;
	Text* m_Health;
	Text* m_MaxMovePoints;
	Text* m_MaxHealth;
	std::map<std::string,WidgetImage*> m_IconImages;
private:
	bool m_Busy;									//	������� ������������
	bool m_Closed;
private:
	struct Order
	{
		COMMAND m_Command;							//	��������
		MonitorTraits m_Traits;						//	��������
		Order(COMMAND command,const MonitorTraits& traits):m_Command(command),m_Traits(traits){}
	};
	std::deque<Order> m_Commands;					//	������� ����������� ����� 
	Strategy* m_Strategy;							//	������� ��������
	PersonIconMgr* m_Controller;
public:
	Monitor(const char* name,PersonIconMgr* controller);
	virtual ~Monitor();
public:
	//	������������
	void ToOrder(COMMAND command);
	void ToOrder(COMMAND command,const MonitorTraits& traits);
	//	���������� ��������� ��������
	void OnBusy(bool busy);
	//	������� �����?
	bool IsBusy(void) {return m_Busy;}
	//	������� ������?
	bool IsClosed(void) {return m_Closed;}
	//	����������� � ���,��� �������� ���������
	void OnStrategyFinish(void);
	//	�������� ������� 
	void Clear(void);
private:
	//	������� ���� ���� ���������
	bool Tick(void);
	//	����������� �� ��������� ��������� �������
	void OnSystemChange(void);
	//	�����-������
	void OnStatic(Static *pStatic);
	//	������� ��������� ������
	void UpdateState(const MonitorTraits& traits);
public:
	virtual const char* Type(void) const {return "personmonitor";};
};

#endif	//_GAME_SCREEN_H_
/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   13.03.2001

************************************************************************/
#ifndef _SCREENS_H_
#define _SCREENS_H_

class Widget;
class HiddenMovementScreen;
class DescriptionScreen;
class InventoryScreen;
class HeroInfoScreen;
class FlagDescScreen;
class JournalScreen;
class OptionsScreen;
class OptionsScreen;
class DialogScreen;
class HelperScreen;
class TradeScreen;
class PanicScreen;
class GameScreen;

class Screens
{
public:
	enum SCREEN_ID
	{
		SID_GAME,
		SID_INVENTORY,
		SID_TRADE,
		SID_JOURNAL,
		SID_DIALOG,
		SID_HEROINFO,
		SID_HELPER,
		SID_OPTIONS,
		SID_HIDDENMOVEMENT,
		SID_PANIC,
		SID_DESCRIPTION
	};
private:
	struct Deleter 
	{
	public:
		Screens *m_pInstance;
	public:
		Deleter(){m_pInstance = 0;}
		~Deleter(){if(m_pInstance) delete m_pInstance;}
	};
	friend Deleter;
	static Deleter m_Deleter;
private:
	Screens();
	virtual ~Screens();
public:
	//	�������� ��������� � ������� �������
	GameScreen*	Game(void);
	//	�������� ��������� � ������� ������������
	InventoryScreen* Inventory(void);
	//	�������� ��������� � ������� ��������
	TradeScreen* Trade(void);
	//	�������� ��������� � ������� �������
	JournalScreen* Journal(void);
	//	�������� ��������� � ������� ���������� �������
	DialogScreen* Dialog(void);
	//	�������� ��������� � ������� �������� �����
	HeroInfoScreen* HeroInfo(void);
	//	�������� ��������� � ������� ����� ������ ����
	OptionsScreen* Options(void);
	//	�������� ��������� � ������� ������� �����������
	HiddenMovementScreen* HiddenMovement(void);
	//	�������� ��������� � ������� ������������ ������
	PanicScreen* Panic(void);
	//	�������� ��������� � ������� �������� ��������
	DescriptionScreen* Description(void);
	//	�������� ��������� � ������� ���������
	HelperScreen* Helper(void);
	//	�������� ��������� � ������� ��������� �����
	FlagDescScreen* FlagDesc(void);

	//	�������� ��������� � ������� �������
	Widget*	HasGame(void);
	//	�������� ��������� � ������� ������������
	Widget* HasInventory(void);
	//	�������� ��������� � ������� ��������
	Widget* HasTrade(void);
	//	�������� ��������� � ������� �������
	Widget* HasJournal(void);
	//	�������� ��������� � ������� ���������� �������
	Widget* HasDialog(void);
	//	�������� ��������� � ������� �������� �����
	Widget* HasHeroInfo(void);
	//	�������� ��������� � ������� ����� ������ ����
	Widget* HasOptions(void);
	//	�������� ��������� � ������� ������� �����������
	Widget* HasHiddenMovement(void);
	//	�������� ��������� � ������� ������������ ������
	Widget* HasPanic(void);
	//	�������� ��������� � ������� �������� ��������
	Widget* HasDescription(void);
	//	�������� ��������� � ������� ���������
	Widget* HasHelper(void);
	//	�������� ��������� � ������� ��������� �����
	Widget* HasFlagDesc(void);

public:
	//	�������������� ������������ �����
	void Activate(SCREEN_ID id);
	//	����� � ������� ����
	void ExitToMainMenu(void);
	//	��������� ������� ������ �����
	bool IsOptionsPresent(void);
public:
	static Screens *Instance(void);
};

inline Screens* Screens::Instance(void)
{
	if(!m_Deleter.m_pInstance) m_Deleter.m_pInstance = new Screens();
	return m_Deleter.m_pInstance;
}


#endif	//_SCREENS_H_
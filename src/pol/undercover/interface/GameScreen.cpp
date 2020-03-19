/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   07.03.2001

************************************************************************/
#include "Precomp.h"
#include <common/UI/Static.h>
#include <common/FontMgr/Font.h>
#include "Text.h"
#include "TextBox.h"
#include "WidgetFactory.h"
#include "DXImageMgr.h"
#include "InventoryScreen.h"
#include "TradeScreen.h"
#include "JournalScreen.h"
#include "DialogScreen.h"
#include "LoadingScreen.h"
#include "OptionsScreen.h"
#include "DescriptionScreen.h"
#include "SlotsEngine.h"
#include "WidgetSoundMgr.h"
#include "MenuMgr.h"
#include "GameScreen.h"
#include "Screens.h"

//	��� ������������� ������
#include <logic2/logictypes.h>
#include <logic2/entity.h>
#include <logic2/thing.h>
#include <logic2/aiutils.h>
#include <logic2/graphic.h>
#include <logic_ex/ActMgr.h>
#include <common/GraphPipe/GraphPipe.h>
#include <common/D3DApp/Input/Input.h>

//**********************************************************************
//	class WeaponIcon
class WeaponIcon : public Widget
{
private:
	Text* m_Text;
	bool m_IsNotVisible;			//	�������������� ����
public:
	WeaponIcon() : Widget("weapon_icon_item")
	{
		Insert(m_Text = new Text("text"));
		m_Text->Align(LogicalText::T_BOTTOM|LogicalText::T_RIGHT);
		m_Text->SetFont("oskar",0x00dd00);
		m_Text->Enable(false);
	}
	virtual ~WeaponIcon(){};
public:
	void SetText(const char* text)
	{
		m_Text->SetText(text);
		m_Text->SetRegion(0,
						  GetLocation()->Region()->Height()/2,
						  GetLocation()->Region()->Width(),
						  GetLocation()->Region()->Height());
	}
	void SetNotVisible(bool value) {m_IsNotVisible = value;};
	bool IsNotVisible(void) const {return m_IsNotVisible;};
};


//**********************************************************************
//	class GameScreen
//	�������� ��� ������ ���������
const char* g_pIconNone = "_gray";
const char* g_pIconDied = "_dead";

const char* GameScreen::m_pDialogName = "GameScreen";
const char* GameScreen::m_WeaponIconName = "WeaponIcon";
#pragma warning(push)
#pragma warning(disable:4355)
GameScreen::GameScreen() : Dialog(m_pDialogName,m_pDialogName),m_pController(0),
						   m_bUnloadOptionsScreen(false),m_EnemyIconMgr(new EnemyIconMgr(this)),
						   m_HackVehicleMgr(new HackVehicleMgr(this)),
						   m_PersonIconMgr(new PersonIconMgr(this)),
						   m_IsCameraMoveable(true),m_CurrentActiveDilog(0)
{
	char pBuff[50];
	int i;

	//	��������� ������ �� ������
	Insert(m_WeaponIcon = new WeaponIcon(),L_LEFT_BOTTOM);
	//	��������� ���� ���������
	Insert(m_pMsgWindow = new MessageWindow(),L_RIGHT_BOTTOM);
	BringToBottom(m_pMsgWindow->Name());
	//	��������� �������� �� �������
	for(i=0;i<6;i++)
	{
		sprintf(pBuff,"vm%d",i);
		Insert(m_pVehicleMonitors[i].m_pMonitor = new VehicleMonitor(pBuff),L_RIGHT_TOP);
		m_pVehicleMonitors[i].m_pMonitor->Move(0,(m_pVehicleMonitors[i].m_pMonitor->GetLocation()->Region()->Height()+2)*i);
		m_pVehicleMonitors[i].m_bVisible = false;
	}
	//	��������� �������������� ������� ������
	Insert(new InventoryScreen(),L_CENTER);
	Insert(new TradeScreen(),L_CENTER);
	Insert(new JournalScreen(),L_CENTER);
	Insert(new DialogScreen(),L_CENTER);
	Insert(new HeroInfoScreen(),L_CENTER);
	Insert(new HelperScreen(),L_CENTER);
	Insert(new FlagDescScreen(),L_CENTER);
	Insert(new HiddenMovementScreen(),L_CENTER);
	Insert(new PanicScreen(),L_CENTER);
	Insert(new DescriptionScreen(),L_CENTER);
	//	�������������� ������ ���-������
	m_CheckBoxes[CB_RUN].m_VisualArea	= (Static *)WidgetFactory::Instance()->Assert(this,"run");
	m_CheckBoxes[CB_RUN].m_HotArea		= (Static *)WidgetFactory::Instance()->Assert(this,"run_hot");
	m_CheckBoxes[CB_SIT].m_VisualArea	= (Static *)WidgetFactory::Instance()->Assert(this,"sit");
	m_CheckBoxes[CB_SIT].m_HotArea		= (Static *)WidgetFactory::Instance()->Assert(this,"sit_hot");
	m_CheckBoxes[CB_SPECIAL].m_VisualArea	= (Static *)WidgetFactory::Instance()->Assert(this,"special");
	m_CheckBoxes[CB_SPECIAL].m_HotArea		= (Static *)WidgetFactory::Instance()->Assert(this,"special_hot");
	m_CheckBoxes[CB_USE].m_VisualArea	= (Static *)WidgetFactory::Instance()->Assert(this,"use");
	m_CheckBoxes[CB_USE].m_HotArea		= (Static *)WidgetFactory::Instance()->Assert(this,"use_hot");
	m_CheckBoxes[CB_EYES].m_VisualArea	= (Static *)WidgetFactory::Instance()->Assert(this,"eyes");
	m_CheckBoxes[CB_EYES].m_HotArea		= (Static *)WidgetFactory::Instance()->Assert(this,"eyes");
	m_CheckBoxes[CB_ROOF].m_VisualArea	= (Static *)WidgetFactory::Instance()->Assert(this,"roof");
	m_CheckBoxes[CB_ROOF].m_HotArea		= (Static *)WidgetFactory::Instance()->Assert(this,"roof");
	m_CheckBoxes[CB_JOURNAL].m_VisualArea	= (Static *)WidgetFactory::Instance()->Assert(this,"journal");
	m_CheckBoxes[CB_JOURNAL].m_HotArea		= (Static *)WidgetFactory::Instance()->Assert(this,"journal");
	m_CheckBoxes[CB_ENDTURN].m_VisualArea	= (Static *)WidgetFactory::Instance()->Assert(this,"endturn");
	m_CheckBoxes[CB_ENDTURN].m_HotArea		= (Static *)WidgetFactory::Instance()->Assert(this,"endturn_hot");
	m_CheckBoxes[CB_MENU].m_VisualArea	= (Static *)WidgetFactory::Instance()->Assert(this,"menu");
	m_CheckBoxes[CB_MENU].m_HotArea		= (Static *)WidgetFactory::Instance()->Assert(this,"menu_hot");
	m_CheckBoxes[CB_FOS].m_VisualArea	= (Static *)WidgetFactory::Instance()->Assert(this,"visibility");
	m_CheckBoxes[CB_FOS].m_HotArea		= (Static *)WidgetFactory::Instance()->Assert(this,"visibility");
	m_CheckBoxes[CB_AUTOSHOT].m_VisualArea	= (Static *)WidgetFactory::Instance()->Assert(this,"autoshot");
	m_CheckBoxes[CB_AUTOSHOT].m_HotArea		= (Static *)WidgetFactory::Instance()->Assert(this,"autoshot");
	m_CheckBoxes[CB_SNAPSHOT].m_VisualArea	= (Static *)WidgetFactory::Instance()->Assert(this,"snapshot");
	m_CheckBoxes[CB_SNAPSHOT].m_HotArea		= (Static *)WidgetFactory::Instance()->Assert(this,"snapshot");
	m_CheckBoxes[CB_AIMSHOT].m_VisualArea	= (Static *)WidgetFactory::Instance()->Assert(this,"aimshot");
	m_CheckBoxes[CB_AIMSHOT].m_HotArea		= (Static *)WidgetFactory::Instance()->Assert(this,"aimshot");
	m_CheckBoxes[CB_BACKPACK].m_VisualArea	= (Static *)WidgetFactory::Instance()->Assert(this,"backpack");
	m_CheckBoxes[CB_BACKPACK].m_HotArea		= (Static *)WidgetFactory::Instance()->Assert(this,"backpack");
	m_CheckBoxes[CB_RUN].m_HotArea->Enable(false);
	m_CheckBoxes[CB_SIT].m_HotArea->Enable(false);
	m_CheckBoxes[CB_SPECIAL].m_HotArea->Enable(false);
	m_CheckBoxes[CB_USE].m_HotArea->Enable(false);
	//	�������������� ������ ������ (������ ����� ��� ���������: normal, pressed, grayed)
	m_Buttons[B_RELOAD] = static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"reload"));
	m_Buttons[B_UP_ARROW] = static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"up_arrow"));
	m_Buttons[B_DOWN_ARROW] = static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"down_arrow"));
	//	----------------------------------------
	m_MPTextFields[MTF_AIM] = static_cast<Text*>(WidgetFactory::Instance()->Assert(this,"aimshot_mp"));
	m_MPTextFields[MTF_SNAP] = static_cast<Text*>(WidgetFactory::Instance()->Assert(this,"snapshot_mp"));
	m_MPTextFields[MTF_AUTO] = static_cast<Text*>(WidgetFactory::Instance()->Assert(this,"autoshot_mp"));
	//	----------------------------------------
	//	�������� �� �������������
	WidgetFactory::Instance()->Assert(this,"head");
	WidgetFactory::Instance()->Assert(this,"body");
	WidgetFactory::Instance()->Assert(this,"hands");	
	WidgetFactory::Instance()->Assert(this,"legs");	
	WidgetFactory::Instance()->Assert(this,"electric");
	WidgetFactory::Instance()->Assert(this,"shock");
	WidgetFactory::Instance()->Assert(this,"flame");	
	//	��������� ������� ����������
	for(Widget::Iterator wi=Begin();wi!=End();++wi)
		AlignWidget(&(*wi));
	CheckConformMode(); 
	//	��������� ������ �����
	m_EnemyIconMgr->UpdateLocation();
	m_HackVehicleMgr->UpdateLocation();
}
#pragma warning(pop)

GameScreen::~GameScreen()
{
	//	���������� �������� ������ �����
	delete m_EnemyIconMgr;
	//	���������� �������� ������ �������� �������
	delete m_HackVehicleMgr;
	//	���������� �������� ������ �� ���������
	delete m_PersonIconMgr;
}

//=====================================================================================//
//                            static bool HasItemOnCursor()                            //
//=====================================================================================//
static bool HasItemOnCursor()
{
	return Screens::Instance()->Inventory()->GetSlotsEngine()->MigrationMgr()->GetItem() != 0;
}

//=====================================================================================//
//                              static bool IsOnGround()                               //
//=====================================================================================//
static bool IsOnGround(SEItem *item)
{
    if(item == 0 || item->SlotId() == SEItem::m_SlotIdIsAbsent)
	{
        return false;
	}

	return item->SlotId() == InventoryScreen::S_GROUND;
}

namespace GameScreenDetail
{

struct GiveItemsParams
{
	float m_maxDistance;

	GiveItemsParams()
	:	m_maxDistance(5.0f)
	{
		VFile *file = DataMgr::Load("scripts/giveit.txt");
		if(file && file->Size())
		{
			const std::string data(file->Data(),file->Data()+file->Size());
			std::string temp;
			std::istringstream sstr(data);
			sstr >> temp >> m_maxDistance;
		}
		if(file) DataMgr::Release("scripts/giveit.txt");
	}
};

}

using namespace GameScreenDetail;

//=====================================================================================//
//                        bool GameScreen::ProcessMouseInput()                         //
//=====================================================================================//
bool GameScreen::ProcessMouseInput(VMouseState *pMS)
{
	static const GiveItemsParams gi_params;

	if(Focus()->Get(WidgetFocus::FF_KEYBOARD) == WidgetFactory::Instance()->Assert(this,InventoryScreen::m_pDialogName))
	{
		if(GetController() && pMS->LButtonBack)
		{
			PersonIconMgr *pim = GetPersonIconMgr();

			for(unsigned int i = 0; i < 8; i++)
			{
				PersonIconMgr::Icon *icon = pim->IconAt(i);
				if(icon && icon->GetMonitor()->GetLocation()->Region()->PtInRegion(pMS->x,pMS->y))
				{
					HumanEntity *me = Screens::Instance()->Inventory()->GetController()->GetCurrentHuman();
					HumanEntity *he = icon->GetHumanEntity();

					if(!HasItemOnCursor())
					{
						GetController()->OnIconClick(i,BNC_LCLICK);
						Screens::Instance()->Inventory()->GetController()->UpdateAllContent(icon->GetHumanEntity());
					}
					else
					{
						if(he != me)
						{
							SEMigrationMgr *smgr = Screens::Instance()->Inventory()->GetSlotsEngine()->MigrationMgr();
							
							if((me->GetGraph()->GetPos3()-he->GetGraph()->GetPos3()).Length() < gi_params.m_maxDistance)
							{
								int movepnts = 0;

								BaseThing *thing = smgr->GetItem()->GetBaseThing();
								const ThingInfo* info = thing->GetInfo();
								if(IsOnGround(smgr->GetItem())) movepnts += AIUtils::CalcMps4ThingPickUp(thing);

								/*if(betwHumans) */movepnts += 2;	// ���������� ���������� �� �������� ��������

								if(me->GetEntityContext()->GetTraits()->GetMovepnts() >= movepnts || UnlimitedMoves())
								{
									Screens::Instance()->Inventory()->GetController()->GiveItemTo(he,smgr->GetItem());
								}
							}
						}
					}
				}
			}
		}
	}

	Dialog::ProcessMouseInput(pMS);
	return true;
}

//	���������� �����-�� ��������
bool GameScreen::Tick(void)
{
	Camera *Cam=IWorld::Get()->GetPipe()->GetCam();
	if(Cam->GetMoveType() != cmtPersonBone)
	{
		Screens::Instance()->Game()->SetCBState(CB_EYES, CBS_OFF);
	}
	else
	{
		Screens::Instance()->Game()->SetCBState(CB_EYES, CBS_ON);
	}

	if(m_bUnloadOptionsScreen)
	{
		Delete(OptionsScreen::m_pDialogName);
		m_bUnloadOptionsScreen = false;
		return false;
	}
	//	��������� ����������
	if(Input::KeyFront(DIK_F10) && !PoL::ActMgr::isPlayingScene())
	{//	������� �����  �� ����
		if(MenuMgr::Instance()->IsQBFree())
		{
			if(MenuMgr::Instance()->QuestionBox(MenuMgr::Instance()->GetStrResourse("mm_quit").c_str()))
			{
				//	Napalm ������, ��� ������ ��� �� �����
				//MakeSpecialSaveLoad(SST_AUTOSAVE,SSM_SAVE);
				MenuMgr::Instance()->ExitFromProgram();
			}
		}
	}
	//	�������� ���������� ��������� ������ ����������
	m_PersonIconMgr->Tick();

	return true;
}
//	��������� ���� �����
void GameScreen::LoadOptionsScreen(void)
{
	//	������� ����� �����
	Insert(new OptionsScreen(),Dialog::L_CENTER);
	//	����������� ��� ������� ����������
	AlignWidget(Child(OptionsScreen::m_pDialogName));
	//	������������� ����� ������ �����
	static_cast<OptionsScreen*>(Child(OptionsScreen::m_pDialogName))->SetMode(OptionsScreen::M_GAMEMENU);
}
//	��������� ���� �����
void GameScreen::UnloadOptionsScreen(void)
{
	m_bUnloadOptionsScreen = true;
}
//	������� �������� ���������� ���� (0 - ������ ��� ����)
void GameScreen::ShowDialog(const char *pName)
{
	if(!pName)
	{
		if(Focus()->Get(WidgetFocus::FF_KEYBOARD))
			Focus()->Get(WidgetFocus::FF_KEYBOARD)->SetVisible(false);
		for(Widget::Iterator it=Begin();it!=End();it++)
			it->Focus()->Enable(WidgetFocus::FF_MOUSE,true);
		//	�������������� ��������
		if(m_CurrentActiveDilog) { m_CurrentActiveDilog->SetVisible(false); m_CurrentActiveDilog = 0; }
		//	�������� ���� �������������� ������
		m_IsCameraMoveable = true;
	}
	else
	{
		Widget* prev_dialog = Focus()->Get(WidgetFocus::FF_KEYBOARD);

		if(Focus()->Get(WidgetFocus::FF_KEYBOARD) != WidgetFactory::Instance()->Assert(this,pName))
		{
			for(Widget::Iterator it=Begin();it!=End();it++)
			{
				//	�������� �� ����� �������� �����
				if(strcmp(it->Name(),HeroInfoScreen::m_pDialogName))
					it->Focus()->Enable(WidgetFocus::FF_MOUSE,false);
			}
			Child(pName)->SetVisible(true);
			Child(pName)->Focus()->Enable(WidgetFocus::FF_MOUSE,true);
			Focus()->Set(WidgetFocus::FF_KEYBOARD,Child(pName));
			//	�������������� ��������
			m_CurrentActiveDilog = Child(pName);
		}
		//	�������� �� ����� �������� 
		if(!strcmp(pName,DescriptionScreen::m_pDialogName))
		{
			if(prev_dialog) prev_dialog->SetVisible(true);
			BringToTop(pName);
		}
		//	�������� ���� �������������� ������
		m_IsCameraMoveable = false;
	}
}
//	�������� �����������, ��� ������ ����� ����������
bool GameScreen::IsCameraMoveable(void)
{
	Widget* child = FirstChildFromPoint(Input::MouseState().x,Input::MouseState().y);

	if(child)
	{
		if((child == MsgWindow()) ||
		   (!strcmp(child->Name(),"weapon_icon")))
		{
			return false;
		}
	}

	return m_IsCameraMoveable;
}
//	��������/�������� �������� ����������
void GameScreen::ShowControls(bool bShow)
{
	for(Widget::Iterator it=Begin();it!=End();it++)
	{
		if(strcmp(it->Type(),"dialog")!=0)
			it->SetVisible(bShow);
	}
	MsgWindow()->SetVisible(bShow);
	if(bShow)
	{
		m_WeaponIcon->SetVisible(!m_WeaponIcon->IsNotVisible());
		for(int i=0;i<6;i++)
			m_pVehicleMonitors[i].m_pMonitor->SetVisible(m_pVehicleMonitors[i].m_bVisible);
	}
}
//	����� � ������� ����
void GameScreen::ExitToMainMenu(void)
{
	Parent()->OnChange(this);
}

void GameScreen::OnChange(Widget* pChild)
{
	if(pChild == Child(OptionsScreen::m_pDialogName))
	{//	OptionsScreen ���� ������ ������ Exit ��� ������������ ������/������
		switch(static_cast<OptionsScreen*>(pChild)->GetParentNotify())
		{
		case OptionsScreen::PN_EXIT:
			//	��������� ���� � AUTOSAVE
			//	Napalm ������, ��� ������ ��� �� �����
			//MakeSpecialSaveLoad(SST_AUTOSAVE,SSM_SAVE);
		case OptionsScreen::PN_SAVELOAD:
			//	������� �� ����
			ExitToMainMenu();
			break;
		}

		return;
	}
	if(m_pController)
	{
		if(!strcmp(pChild->Type(),"personmonitor"))
		{
			for(int i=0; i<8; i++)
			{
				if(m_PersonIconMgr->IconAt((int)i))
				{
					if(reinterpret_cast<Widget*>(m_PersonIconMgr->IconAt((int)i)->GetMonitor()) == pChild)
					{
						GameScreen::BN_CLICK clk = GameScreen::BNC_NONE;
						
						if(Input::MouseState().LDblClick)
							clk = GameScreen::BNC_LDBLCLICK;
						else
						{
							if(Input::MouseState().LButtonBack)
								clk = GameScreen::BNC_LCLICK;
						}
						if(clk != GameScreen::BNC_NONE)
							m_pController->OnIconClick(i,clk);
						else
						{
							if(Input::MouseState().RDblClick)
								clk = GameScreen::BNC_RDBLCLICK;
							else
							{
								if(Input::MouseState().RButtonBack)
									clk = GameScreen::BNC_RCLICK;
							}
							m_pController->OnIconClick(i,clk);
						}
						return;
					}
				}
			}
		}
		if(!strcmp(pChild->Type(),"vehiclemonitor"))
		{
			for(int i=0;i<6;i++)
			{
				if(m_pVehicleMonitors[i].m_pMonitor == pChild)
				{
					GameScreen::BN_CLICK clk = GameScreen::BNC_NONE;

					if(Input::MouseState().LDblClick)
						clk = GameScreen::BNC_LDBLCLICK;
					else
					{
						if(Input::MouseState().LButtonBack)
							clk = GameScreen::BNC_LCLICK;
					}
					if(clk != GameScreen::BNC_NONE)
						m_pController->OnVehicleIconClick(i,clk);
					else
					{
						if(Input::MouseState().RDblClick)
							clk = GameScreen::BNC_RDBLCLICK;
						else
						{
							if(Input::MouseState().RButtonBack)
								clk = GameScreen::BNC_RCLICK;
						}
						m_pController->OnVehicleIconClick(i,clk);
					}
					return;
				}
			}
		}
		else
		{//	�����
			Dialog::OnChange(pChild);
		}
	}
}

void GameScreen::OnStatic(Static *pStatic)
{
	int index;

	for(index=0;index<MAX_CHECKBOX;index++)
	{
		if(m_CheckBoxes[index].m_HotArea == pStatic)
			break;
	}
	if(index != MAX_CHECKBOX)
	{
		if(pStatic->LastEvent() == Static::LE_LBUTTONDOWN)
			m_pController->OnCheckBoxClick(static_cast<CHECKBOX>(index));
		return;
	}
	for(index=0;index<MAX_BUTTON;index++)
	{
		if(m_Buttons[index] == pStatic)
		{
			switch(pStatic->LastEvent())
			{
			case Static::LE_LBUTTONPUSHED:
				if(pStatic->GetState()==0) pStatic->SetState(1);
				break;
			case Static::LE_MOUSELOST:
				pStatic->SetState(0);
				break;
			case Static::LE_LBUTTONDOWN:
				pStatic->SetState(1);
				break;
			case Static::LE_LBUTTONUP:
				pStatic->SetState(0);
				if(m_pController) m_pController->OnButtonClick((BUTTON)index);
				break;
			}

			return;
		}
	}
	if(!strcmp(pStatic->Name(),"message_up"))
	{
		switch(pStatic->LastEvent())
		{
		case Static::LE_LBUTTONDOWN:
			((Static *)WidgetFactory::Instance()->Assert(this,"message_control"))->SetState(2);
			m_pMsgWindow->IncMsgWindow();
			break;
		case Static::LE_LBUTTONUP:
		case Static::LE_MOUSELOST:
			((Static *)WidgetFactory::Instance()->Assert(this,"message_control"))->SetState(0);
			break;
		}
		return;
	}
	if(!strcmp(pStatic->Name(),"message_down"))
	{
		switch(pStatic->LastEvent())
		{
		case Static::LE_LBUTTONDOWN:
			((Static *)WidgetFactory::Instance()->Assert(this,"message_control"))->SetState(1);
			m_pMsgWindow->DecMsgWindow();
			break;
		case Static::LE_LBUTTONUP:
		case Static::LE_MOUSELOST:
			((Static *)WidgetFactory::Instance()->Assert(this,"message_control"))->SetState(0);
			break;
		}
		return;
	}
	if(!strcmp(pStatic->Name(),"weapon_icon"))
	{
		if(!m_WeaponIcon->IsNotVisible() && 
		  (pStatic->LastEvent() == Static::LE_LBUTTONDOWN || 
		   pStatic->LastEvent() == Static::LE_RBUTTONDOWN))
		{
			if(m_pController) m_pController->OnWeaponIconClick();
		}

		return;
	}
}

/****************** ������� ��������� � ������� *********************/
//	���������� ��������� ��� ������ ����� � ������
void GameScreen::SetJournalButtonState(JOURNAL_BUTTON_STATE jbs)
{
	m_CheckBoxes[CB_JOURNAL].m_VisualArea->SetState(jbs);
}
// ������������ ������ � �������� ���������
void GameScreen::SetCBState(CHECKBOX id,CHECKBOX_STATE state)
{
	if(m_CheckBoxes[id].m_HotArea->IsEnable())
		m_CheckBoxes[id].m_VisualArea->SetState(state);
}
// �������� ��������� ������	
GameScreen::CHECKBOX_STATE GameScreen::GetCBState(CHECKBOX id)
{
	return (CHECKBOX_STATE)m_CheckBoxes[id].m_VisualArea->GetState();
}
// ������������ ������ � ��������� Enable/Disable
void GameScreen::EnableCBState(CHECKBOX id,bool bEnable)
{
	if(!bEnable) m_CheckBoxes[id].m_VisualArea->SetState(2);
	else m_CheckBoxes[id].m_VisualArea->SetState(0);
	m_CheckBoxes[id].m_HotArea->Enable(bEnable);
}
// ������������ ������ � ��������� Enable/Disable
void GameScreen::EnableButtonState(BUTTON id,bool bEnable)
{
	if(!bEnable) m_Buttons[id]->SetState(2);
	else m_Buttons[id]->SetState(0);
	m_Buttons[id]->Enable(bEnable);
}
//	���������� ��������� ��� ������ ����������� ��������� ���������
void GameScreen::SetBodyIcon(const BodyIconTraits& bit)
{
	((Static *)Child("head"))->SetState(bit.m_Head);	
	((Static *)Child("body"))->SetState(bit.m_Body);	
	((Static *)Child("hands"))->SetState(bit.m_Hands);	
	((Static *)Child("legs"))->SetState(bit.m_Legs);	
	//	������ ���������
	static_cast<Static *>(Child("electric"))->SetState((bit.m_Damages&BodyIconTraits::DT_ELECTRIC)?1:0);	
	static_cast<Static *>(Child("shock"))->SetState((bit.m_Damages&BodyIconTraits::DT_SHOCK)?1:0);	
	static_cast<Static *>(Child("flame"))->SetState((bit.m_Damages&BodyIconTraits::DT_FLAME)?1:0);	
}
//	���������� ��������� ��� ������ �������
void GameScreen::SetVehicleIcon(unsigned int icon,const VehicleIconTraits& vit)
{
	//	�����/�� �����
	m_pVehicleMonitors[icon].m_pMonitor->SetVisible(vit.m_Visible);
	m_pVehicleMonitors[icon].m_bVisible = vit.m_Visible;
	//	��� ���������
	m_pVehicleMonitors[icon].m_pMonitor->SetMaxCrew(vit.m_MaxCrew);
	m_pVehicleMonitors[icon].m_pMonitor->SetNumCrew(vit.m_NumCrew);
	m_pVehicleMonitors[icon].m_pMonitor->SetMovePoints(vit.m_MovePoints);
	m_pVehicleMonitors[icon].m_pMonitor->SetHealth(vit.m_HitPoints);
	//	��������� ������ (�����) VEHICLE_ICON_FLAG
	m_pVehicleMonitors[icon].m_pMonitor->EnableInMovement(vit.m_Flags&VehicleIconTraits::VIF_INMOVEMENT);
	//	��� ���������
	((Static *)WidgetFactory::Instance()->Assert(m_pVehicleMonitors[icon].m_pMonitor,"icon"))->SetState(vit.m_Selection);
	//	��������� ������ �����
	m_EnemyIconMgr->UpdateLocation();

}
//	���������� �������� ��� ������ (0 - �� ���������� ������)
void GameScreen::SetWeaponIcon(const WeaponIconTraits* traits)
{
	Text* ammo_desc = static_cast<Text*>(WidgetFactory::Instance()->Assert(this,"ammo_desc"));

	if(traits)
	{
		if(traits->m_ImageName && strlen(traits->m_ImageName)/* && WidgetFactory::Instance()->Assert(this,"metal")->IsVisible()*/)
		{
			Widget *metal = WidgetFactory::Instance()->Assert(this,"metal");
			std::string image_name = std::string(g_InventoryPictures)+std::string(traits->m_ImageName)+".tga";

			if(metal->IsVisible())
			{
				m_WeaponIcon->SetVisible(true);
				m_WeaponIcon->SetNotVisible(false);
			}

			if(!m_WeaponIcon->GetImage() || !(image_name == static_cast<DXImage*>(m_WeaponIcon->GetImage())->m_Resource))
			{
				int left = 0, top = 0, right = 0, bottom = 0;
				int width = 0, height = 0;
				int x = 0, y = 0;
				
				//	������������� ��������
				if(m_WeaponIcon->GetImage()) m_WeaponIcon->GetImage()->Release();
				m_WeaponIcon->SetImage(DXImageMgr::Instance()->CreateImage(image_name.c_str()));
				DXImageMgr::Instance()->GetImageSize(static_cast<DXImage*>(m_WeaponIcon->GetImage()),&width,&height);
				WidgetFactory::Instance()->Assert(this,"weapon_icon")->GetLocation()->Region()->GetRect(&left,&top,&right,&bottom);
				x = left + ((right-left)-width)/2;
				y = top + ((bottom-top)-height)/2;
				m_WeaponIcon->GetLocation()->Create(x,y,x+width,y+height);
			}
			BringToBottom(m_WeaponIcon->Name());
			BringToBottom(WidgetFactory::Instance()->Assert(this,"metal")->Name());
			BringToBottom(m_pMsgWindow->Name());
		}
		m_WeaponIcon->SetText(traits->m_AmmoQuantity);
		ammo_desc->SetText(traits->m_AmmoDesc);
	}
	else
	{
		m_WeaponIcon->SetVisible(false);
		m_WeaponIcon->SetNotVisible(true);
		ammo_desc->SetText("");
	}
}
//	���������, ����������� ������ �� ������ ������
bool GameScreen::IsCursorOnWeaponIcon(void)
{
	Widget* child = FirstChildFromPoint(Input::MouseState().x, Input::MouseState().y);
	
	return (child && !strcmp(child->Name(),"weapon_icon"));
}
//	���������� �������� ��������� ����� ��� ����������� ����������
void GameScreen::SetShootMovePoints(const ShootMovePoints& smp)
{
	m_MPTextFields[MTF_AIM]->SetFont(m_MPTextFields[MTF_AIM]->GetFontName(),smp.m_AimColor);
	m_MPTextFields[MTF_SNAP]->SetFont(m_MPTextFields[MTF_SNAP]->GetFontName(),smp.m_SnapColor);
	m_MPTextFields[MTF_AUTO]->SetFont(m_MPTextFields[MTF_AUTO]->GetFontName(),smp.m_AutoColor);
	m_MPTextFields[MTF_AIM]->SetText(smp.m_AimMP);
	m_MPTextFields[MTF_SNAP]->SetText(smp.m_SnapMP);
	m_MPTextFields[MTF_AUTO]->SetText(smp.m_AutoMP);
}
//	���������� ���������� ������ ��� ����������
void GameScreen::SetController(GameScreenController *pController)
{
	m_pController = pController;
}
//	��� ������ �� ����������� �������
void GameScreen::OpenScriptSceneScreen(void)
{
	if(!Child(ScriptSceneScreen::m_pDialogName))
	{
		Insert(new ScriptSceneScreen(),L_CENTER);
	}
}

void GameScreen::CloseScriptSceneScreen(void)
{
	Delete(ScriptSceneScreen::m_pDialogName);
}

int GameScreen::GetEnemyIconBottom(void)
{
	return GetLocation()->Region()->Height()-170;
}

int GameScreen::GetEnemyIconTop(void)
{
	int i=0;
	for(;i<5;i++)
	{
		if(!m_pVehicleMonitors[i].m_bVisible)
			break;
	}

	return std::min(350,m_pVehicleMonitors[i].m_pMonitor->GetLocation()->Region()->Y());
}
//	����������� �� ��������� ��������� �������
void GameScreen::OnSystemChange(void)
{
	Dialog::OnSystemChange();
	//	��������� ������ �����
	m_EnemyIconMgr->UpdateLocation();
	m_HackVehicleMgr->UpdateLocation();
}

void GameScreen::OnEnemyIconClick(Marker* marker)
{
	if(m_pController)
	{
		m_pController->OnEnemyIconClick(marker);
	}
}

int GameScreen::GetHackVehicleIconRight(void)
{
	return GetLocation()->Region()->Width()-m_pVehicleMonitors[0].m_pMonitor->GetLocation()->Region()->Width()-50;
}

void GameScreen::OnHackVehicleIconClick(VehicleEntity* ve,GameScreen::BN_CLICK click_type)
{
	if(m_pController)
	{
		m_pController->OnHackVehicleClick(ve,click_type);
	}
}

//=====================================================================================//
//                           static time_t GetSaveFileTime()                           //
//=====================================================================================//
static time_t GetSaveFileTime(const std::string &name)
{
	time_t result = 0;
	_finddata_t buf;
	long hFind = _findfirst(name.c_str(), &buf);
	if (hFind != -1L)
	{
		result = buf.time_write;
		_findclose(hFind);
	}
	return result;
}

//	��������/��������� ���� "��������������" � "������� ����������" (���� ����� ��� return false)
bool GameScreen::MakeSpecialSaveLoad(SPECIAL_SAVELOAD_TYPE type,SPECIAL_SAVELOAD_MODE mode)
{
	std::string file_name;
	std::string file_desc;

	bool save = (mode == SSM_SAVE);

	switch(type)
	{
	//case SST_AUTOSAVE:
	//	file_name = OptionsScreen::GetFullFileNameOfSave("save0");
	//	file_desc = MenuMgr::Instance()->GetStrResourse("os_autosave");
	//	break;
	case SST_QUICKSAVE:
		{
			std::string file1 = OptionsScreen::GetFullFileNameOfSave("save0");
			std::string file2 = OptionsScreen::GetFullFileNameOfSave("save1");

			time_t time1 = GetSaveFileTime(file1);
			time_t time2 = GetSaveFileTime(file2);

			file_name = ((time1 <= time2) == save ? file1 : file2);

			//file_name = OptionsScreen::GetFullFileNameOfSave("save1");
			file_desc = MenuMgr::Instance()->GetStrResourse("os_quicksave");
			break;
		}
	}

	switch(mode)
	{
	case SSM_SAVE:
		{
			//	�������� �����������
			MenuMgr::Instance()->LoadSaveGameModifier()->SetSaveGameName(file_name.c_str());
			MenuMgr::Instance()->LoadSaveGameModifier()->SetSaveGameDesc(file_desc.c_str());
			MenuMgr::Instance()->LoadSaveGameModifier()->SetAvailable(MenuMgr::ic_LoadSaveGameModifier::A_SAVE);
			Parent()->OnChange(this);
			
			return true;
		}
	case SSM_LOAD:
		{
			//	�������� �� ������������� �����
			VFile* test_file = DataMgr::Load(file_name.c_str());
			bool result = test_file->Size() != 0;
			
			DataMgr::Release(file_name.c_str());
			if(result)
			{
				//	�������� �����������
				MenuMgr::Instance()->LoadSaveGameModifier()->SetSaveGameName(file_name.c_str());
				MenuMgr::Instance()->LoadSaveGameModifier()->SetSaveGameDesc(file_desc.c_str());
				MenuMgr::Instance()->LoadSaveGameModifier()->SetAvailable(MenuMgr::ic_LoadSaveGameModifier::A_LOAD);
				Parent()->OnChange(this);
			}
			
			return result;
		}
	}

	return false;
}

//=====================================================================================//
//                    class GameScreen::MessageWindow::StringBuffer                    //
//=====================================================================================//
class GameScreen::MessageWindow::StringBuffer
{
private:

	typedef std::list<unsigned int> lengthes_t;
	lengthes_t m_lengthes;							//	����� �����

	unsigned int m_capacity;						//	������������ ���-�� �����, �������� � ������
	std::string m_content;

public:
	StringBuffer(unsigned int capacity) : m_capacity(capacity) {}
	~StringBuffer() {}

public:
	//	�������� ������� 
	void addString(const char* str);
	//	�������� ������� �����
	const char* getContent(void) const;
	//	�������� �����
	void clear(void);

};

//=====================================================================================//
//              void GameScreen::MessageWindow::StringBuffer::addString()              //
//=====================================================================================//
// �������� ������� 
void GameScreen::MessageWindow::StringBuffer::addString(const char* str)
{
	//	��������� �� ������������
	if(m_lengthes.size() == m_capacity)
	{
		const lengthes_t::iterator it = m_lengthes.begin();
		m_content.erase(0,*it);
		m_lengthes.erase(it);
	}

	//	��������� ������
	int count = m_content.size();
	m_content.append(str);
	m_content.push_back('\n');
	count = m_content.size() - count;
	m_lengthes.push_back(count);
}

//=====================================================================================//
//       const char* GameScreen::MessageWindow::StringBuffer::getContent() const       //
//=====================================================================================//
//	�������� ������� �����
const char* GameScreen::MessageWindow::StringBuffer::getContent(void) const
{
	return m_content.c_str();
}
//	�������� �����
void GameScreen::MessageWindow::StringBuffer::clear(void)
{
	m_content.clear();
	m_lengthes.clear();
}

//**********************************************************************//
//*********	              class MessageWindow              *************//	
GameScreen::MessageWindow::MessageWindow() : Dialog("MessageWindow","MessageWindow"),
											 m_MovingStep(150),
											 m_StringBuffer(new StringBuffer(300)),
											 m_MessageBox(0)
{
	SetVisible(true);
	Focus()->Enable(WidgetFocus::FF_KEYBOARD,false);
	//	����������� ��������� �������/���������
	m_iMaxHeight = GetLocation()->Region()->Y();
	m_iMinHeight = m_iMaxHeight+GetLocation()->Region()->Height();
	m_iCurHeight = m_iMaxHeight;
	m_iAimHeight = m_iMinHeight;
	//	----------------------------------------
	CheckConformMode(); 
	//	----------------------------------------
	m_MessageBox = (TextBox *)WidgetFactory::Instance()->Assert(this,"message_box");
	m_iMessageBoxHeight = m_MessageBox->GetLocation()->Region()->Height();
}

GameScreen::MessageWindow::~MessageWindow()
{
}

//=====================================================================================//
//                    void GameScreen::MessageWindow::SetVisible()                     //
//=====================================================================================//
void GameScreen::MessageWindow::SetVisible(bool v)
{
	Dialog::SetVisible(v);
	if(m_MessageBox) m_MessageBox->SetVisible(v);
}

//	������� ����, ���� ���������
bool GameScreen::MessageWindow::Tick(void)
{
	static float seconds = Timer::GetSeconds();
	float step;

	if(m_iCurHeight != m_iAimHeight)
	{
		step = (Timer::GetSeconds()-seconds)*m_MovingStep;
		if(m_iCurHeight > m_iAimHeight)
		{//	��������� ����
			step = -std::min(m_iCurHeight-m_iAimHeight,step);
		}
		else
		{//	�������� ����
			step = std::min(m_iAimHeight-m_iCurHeight,step);
		}
		m_iCurHeight += step;
		MoveTo(GetLocation()->Region()->X(),m_iCurHeight);
		//	�������� ������� message_box'�
		int old_height = m_MessageBox->GetLocation()->Region()->Height();
		m_MessageBox->SetRegion(m_MessageBox->GetLocation()->Region()->X(),
								m_MessageBox->GetLocation()->Region()->Y(),
								m_MessageBox->GetLocation()->Region()->X()+m_MessageBox->GetLocation()->Region()->Width(),
								m_MessageBox->GetLocation()->Region()->Y()+std::max(static_cast<int>(m_MessageBox->GetFont()->Height()),m_iMessageBoxHeight-static_cast<int>(m_iCurHeight-m_iMaxHeight)));
		if(m_MessageBox->IsScrollEnd())
			m_MessageBox->Scroll(TextBox::SD_END);
		else
			m_MessageBox->Scroll((int)m_MessageBox->GetLocation()->Region()->Height()-old_height);
	}
	seconds = Timer::GetSeconds();

	return true;
}

void GameScreen::MessageWindow::OnStatic(Static *pStatic)
{
	if(!strcmp(pStatic->Name(),"message_scroll_up"))
	{
		switch(pStatic->LastEvent())
		{
		case Static::LE_LBUTTONDOWN:
			((Static *)WidgetFactory::Instance()->Assert(this,"message_scroll"))->SetState(2);
			m_MessageBox->Scroll(TextBox::SD_UP,true);
			break;
		case Static::LE_LBUTTONPUSHED:
			m_MessageBox->Scroll(TextBox::SD_UP,true);
			break;
		case Static::LE_LBUTTONUP:
		case Static::LE_MOUSELOST:
			((Static *)WidgetFactory::Instance()->Assert(this,"message_scroll"))->SetState(0);
			break;
		}
		return;
	}
	if(!strcmp(pStatic->Name(),"message_scroll_down"))
	{
		switch(pStatic->LastEvent())
		{
		case Static::LE_LBUTTONDOWN:
			((Static *)WidgetFactory::Instance()->Assert(this,"message_scroll"))->SetState(1);
			m_MessageBox->Scroll(TextBox::SD_DOWN,true);
			break;
		case Static::LE_LBUTTONPUSHED:
			m_MessageBox->Scroll(TextBox::SD_DOWN,true);
			break;
		case Static::LE_LBUTTONUP:
		case Static::LE_MOUSELOST:
			((Static *)WidgetFactory::Instance()->Assert(this,"message_scroll"))->SetState(0);
			break;
		}
		return;
	}
}

void GameScreen::MessageWindow::CheckConformMode(void)
{
	int dy = D3DKernel::ResY() - m_ResY;

	m_ResY = D3DKernel::ResY();
	//	����������� ��������� �������/���������
	m_iMaxHeight += dy;
	m_iMinHeight += dy;
	m_iCurHeight += dy;
	m_iAimHeight += dy;
}

//=====================================================================================//
//                   void GameScreen::MessageWindow::IncMsgWindow()                    //
//=====================================================================================//
//	��������� ��������� ����
void GameScreen::MessageWindow::IncMsgWindow(void)
{
	m_iAimHeight = std::max(m_iMaxHeight,m_iAimHeight-(m_iMinHeight-m_iMaxHeight)/2);
}

//=====================================================================================//
//                   void GameScreen::MessageWindow::DecMsgWindow()                    //
//=====================================================================================//
//	��������� ��������� ����
void GameScreen::MessageWindow::DecMsgWindow(void)
{
	m_iAimHeight = std::min(m_iMinHeight,m_iAimHeight+(m_iMinHeight-m_iMaxHeight)/2);
}

//=====================================================================================//
//                      void GameScreen::MessageWindow::AddText()                      //
//=====================================================================================//
//	�������� ����� � ����
void GameScreen::MessageWindow::AddText(const char* pText)
{
	m_StringBuffer->addString(pText);
	m_MessageBox->SetText(m_StringBuffer->getContent());
	m_MessageBox->Scroll(TextBox::SD_END);
}
//	�������� ������
void GameScreen::MessageWindow::Clear(void)
{
	m_StringBuffer->clear();
	m_MessageBox->SetText("");
}

//**********************************************************************//
//*********	              class VehicleMonitor             *************//
const char* GameScreen::VehicleMonitor::m_pScriptName = "VehicleMonitor";
const int GameScreen::VehicleMonitor::m_PlacesX = 5;
const int GameScreen::VehicleMonitor::m_PlacesY = 18;
const int GameScreen::VehicleMonitor::m_PlacesWidth = 7;
const int GameScreen::VehicleMonitor::m_PlacesHeight = 11;
GameScreen::VehicleMonitor::VehicleMonitor(const char *pName) : Dialog(pName,m_pScriptName)
{
	Focus()->Enable(WidgetFocus::FF_KEYBOARD,false);
	//	--------------------------------------------
	//	������� ������ ����������
	char pBuff[50];
	for(int i=0;i<MAX_PLACES;i++)
	{
		sprintf(pBuff,"place%d",i);
		Widget::Insert(m_Places[i] = new Static(pBuff));
		int left = m_PlacesX+(m_PlacesWidth+1)*(i&1);
		int top  = m_PlacesY+(m_PlacesHeight+1)*(i/2);
		m_Places[i]->GetLocation()->Create(left,top,left+m_PlacesWidth,top+m_PlacesHeight);
		m_Places[i]->AddImage(DXImageMgr::Instance()->CreateImage("pictures/interface/GameScreen/place_empty.tga"));
		m_Places[i]->AddImage(DXImageMgr::Instance()->CreateImage("pictures/interface/GameScreen/place.tga"));
		m_Places[i]->SetVisible(false);
	}
}

GameScreen::VehicleMonitor::~VehicleMonitor()
{
}

void GameScreen::VehicleMonitor::OnSystemChange(void)
{
	Widget::OnSystemChange();
}
//	��������� �� �������
void GameScreen::VehicleMonitor::OnStatic(Static *pStatic)
{
	if(Parent() && !strcmp("icon",pStatic->Name()) && (pStatic->LastEvent() != Static::LE_NONE))
		Parent()->OnChange(this);
}

void GameScreen::VehicleMonitor::SetMaxCrew(int value)
{
	value = std::min(value,(int)MAX_PLACES);
	for(int i=0;i<MAX_PLACES;i++)
	{
		if(i<value)
		{
			m_Places[i]->SetState(0);
			m_Places[i]->SetVisible(true);
		}
		else
		{
			m_Places[i]->SetVisible(false);
		}
	}
}

void GameScreen::VehicleMonitor::SetNumCrew(int value)
{
	value = std::min(value,(int)MAX_PLACES);
	for(int i=0;i<value;i++)
		m_Places[i]->SetState(1);
}

void GameScreen::VehicleMonitor::SetMovePoints(int value)
{
	static char pBuff[20];

	itoa(value,pBuff,10);
	((Text* )WidgetFactory::Instance()->Assert(this,"movepoints"))->SetText(pBuff);
}

void GameScreen::VehicleMonitor::SetHealth(int value)
{
	static char pBuff[20];

	itoa(value,pBuff,10);
	((Text* )WidgetFactory::Instance()->Assert(this,"health"))->SetText(pBuff);
}

void GameScreen::VehicleMonitor::EnableInMovement(bool /*bEnable*/)
{
//	WidgetFactory::Instance()->Assert(this,"inmovement")->SetVisible(bEnable);
}

//**********************************************************************//
//	class GameScreen::EnemyIconMgr
int GameScreen::EnemyIconMgr::m_Bottom = 0;							
int GameScreen::EnemyIconMgr::m_Top = 0;							
GameScreen::EnemyIconMgr::EnemyIconMgr(GameScreen* owner) : m_Owner(owner)
{
}

GameScreen::EnemyIconMgr::~EnemyIconMgr()
{
	if(m_Icons.size()) Clear();
}

void GameScreen::EnemyIconMgr::Insert(EnemyIcon& ei)
{
	EnemyIconExtended* eie = new EnemyIconExtended(m_Owner,ei);

	eie->MoveTo(GetNextLocation(m_Icons.size()));
	eie->SetNumber(m_Icons.size()+1);

	m_Icons.push_back(eie);
}

void GameScreen::EnemyIconMgr::Remove(Marker* marker)
{
	for(std::list<EnemyIconExtended*>::iterator i=m_Icons.begin();i!=m_Icons.end();i++)
	{
		if((*i)->GetMarker() == marker)
		{
			//	������� ����
			delete (*i);
			m_Icons.erase(i);
			//	��������� ������� ������ ������
			UpdateLocation();
			break;
		}
	}
}

void GameScreen::EnemyIconMgr::Clear(void)
{
	for(std::list<EnemyIconExtended*>::iterator i=m_Icons.begin();i!=m_Icons.end();i++)
		delete (*i);
	m_Icons.clear();
}

GameScreen::EnemyIconMgr::EnemyIcon* GameScreen::EnemyIconMgr::ItemAt(const Marker* marker)
{
	for(std::list<EnemyIconExtended*>::iterator i=m_Icons.begin();i!=m_Icons.end();i++)
	{
		if((*i)->GetMarker() == marker)
		{
			return (*i);
		}
	}

	return 0;
}

GameScreen::EnemyIconMgr::EnemyIcon* GameScreen::EnemyIconMgr::ItemAt(unsigned int number)
{
	std::list<EnemyIconExtended*>::iterator i = m_Icons.begin();
	
	while((i!=m_Icons.end()) && number)
	{
		i++;
		number--;
	}
	if(i!=m_Icons.end()) return (*i);

	return 0;
}

ipnt2_t GameScreen::EnemyIconMgr::GetNextLocation(unsigned int size)
{
	const int width = GameScreen::EnemyIconMgr::EnemyIconAsWidget::Width();
	const int height = GameScreen::EnemyIconMgr::EnemyIconAsWidget::Height();

	const int in_column = (m_Bottom-m_Top)/height;
	const int width_factor = size/in_column;
	const int height_factor = size-(width_factor*in_column);

	return ipnt2_t(m_Owner->GetLocation()->Region()->Width()-(width*(width_factor+1)),m_Bottom-height*(height_factor+1));
}

void GameScreen::EnemyIconMgr::UpdateLocation(void)
{
	int size = 0;

	m_Bottom = m_Owner->GetEnemyIconBottom();
	m_Top	 = m_Owner->GetEnemyIconTop();
	for(std::list<EnemyIconExtended*>::iterator i=m_Icons.begin();i!=m_Icons.end();i++,size++)
	{
		(*i)->MoveTo(GetNextLocation(size));
		(*i)->SetNumber(size+1);
	}
}
//	���������� ����� ��� ���� ������ ������
void GameScreen::EnemyIconMgr::SetFlags(unsigned int flags)
{
	for(std::list<EnemyIconExtended*>::iterator i=m_Icons.begin();i!=m_Icons.end();i++)
		(*i)->SetFlags(flags);
}

//**********************************************************************//
//	class GameScreen::EnemyIconMgr::EnemyIcon
GameScreen::EnemyIconMgr::EnemyIcon::EnemyIcon(const EnemyIcon& ei) : m_Tip(ei.m_Tip),m_Marker(ei.m_Marker),m_Flags(ei.m_Flags)
{
}

GameScreen::EnemyIconMgr::EnemyIcon::EnemyIcon(const std::string& tip,Marker* marker,unsigned int flags) : m_Tip(tip),m_Marker(marker),m_Flags(flags)
{
}

GameScreen::EnemyIconMgr::EnemyIcon::~EnemyIcon()
{
}

std::string GameScreen::EnemyIconMgr::EnemyIcon::GetTip(void) const
{
	return m_Tip;
}

unsigned int GameScreen::EnemyIconMgr::EnemyIcon::GetFlags(void) const
{
	return m_Flags;
}

void GameScreen::EnemyIconMgr::EnemyIcon::SetFlags(unsigned int flags)
{
	m_Flags = flags;
}

Marker* GameScreen::EnemyIconMgr::EnemyIcon::GetMarker(void) const
{
	return m_Marker;
}

//**********************************************************************
//	class GameScreen::EnemyIconMgr::EnemyIconExtended
GameScreen::EnemyIconMgr::EnemyIconExtended::EnemyIconExtended(GameScreen* owner,const GameScreen::EnemyIconMgr::EnemyIcon& ei) : GameScreen::EnemyIconMgr::EnemyIcon(ei),m_Owner(owner)
{
	m_Owner->Insert(m_Icon = new EnemyIconAsWidget(this),L_RIGHT_BOTTOM);
	m_Owner->BringToBottom(m_Icon->Name());
	SetFlags(m_Flags);
}

GameScreen::EnemyIconMgr::EnemyIconExtended::~EnemyIconExtended()
{
	m_Owner->Delete(m_Icon->Name());
}

void GameScreen::EnemyIconMgr::EnemyIconExtended::MoveTo(const ipnt2_t& pt)
{
	m_Icon->MoveTo(pt.x,pt.y);
}

void GameScreen::EnemyIconMgr::EnemyIconExtended::SetNumber(int number)
{
	static char buff[20];

	sprintf(buff,"%d",number);
	m_Icon->SetNumber(buff);
}

void GameScreen::EnemyIconMgr::EnemyIconExtended::SetFlags(unsigned int flags)
{
	m_Flags = flags;
	//	-----------------------------------------------------------------------
	(m_Flags&F_SELECTED)?m_Icon->SetSelection(true):m_Icon->SetSelection(false);
}
//	����������� �� ������
void GameScreen::EnemyIconMgr::EnemyIconExtended::OnClick(void)
{
	m_Owner->OnEnemyIconClick(m_Marker);
}

//**********************************************************************
//	class GameScreen::EnemyIconMgr::EnemyIconAsWidget
const char* GameScreen::EnemyIconMgr::EnemyIconAsWidget::m_EnemyIconName = "pictures/interface/gamescreen/enemy_icon.tga";
const char* GameScreen::EnemyIconMgr::EnemyIconAsWidget::m_EnemyIconSelectedName = "pictures/interface/gamescreen/enemy_icon_selected.tga";
const char* GameScreen::EnemyIconMgr::EnemyIconAsWidget::m_NameDesc = "enemy_icon_";
int GameScreen::EnemyIconMgr::EnemyIconAsWidget::m_Width = 32;
int GameScreen::EnemyIconMgr::EnemyIconAsWidget::m_Height = 32;
GameScreen::EnemyIconMgr::EnemyIconAsWidget::EnemyIconAsWidget(EnemyIconExtended* link) : Widget(GenerateName().c_str()),m_Selected(false),m_Link(link)
{
	Insert(m_Icon = new Static("icon"));
	m_Icon->AddImage(DXImageMgr::Instance()->CreateImage(m_EnemyIconName));
	m_Icon->AddImage(DXImageMgr::Instance()->CreateImage(m_EnemyIconSelectedName));
	m_Icon->SetState(0);
	//	������ ������� ��������
	DXImageMgr::Instance()->GetImageSize(static_cast<DXImage*>(m_Icon->GetImage(0)),&m_Width,&m_Height);
	//	������� ��������� ����
	Insert(m_Number = new Text("number"));
	m_Number->SetRegion(0,0,m_Width,m_Height);
	m_Number->SetFont("oskar",0xffff00);
	m_Number->Align(LogicalText::T_HCENTER|LogicalText::T_VCENTER);
	m_Number->SetText("X");
	//	������������� ����������� ���������
	m_Number->m_Tip.m_Text = m_Link->GetTip();
	m_Number->m_Tip.m_Activated = true;
	m_Number->Enable(false);
	//	������������� �������	
	m_Icon->GetLocation()->Create(0,0,m_Width,m_Height);
	GetLocation()->Create(0,0,m_Width,m_Height);
}

GameScreen::EnemyIconMgr::EnemyIconAsWidget::~EnemyIconAsWidget(void)
{
}

std::string GameScreen::EnemyIconMgr::EnemyIconAsWidget::GenerateName(void)
{
	static int counter = 0;
	static char buff[50];

	sprintf(buff,"%s%ld",m_NameDesc,counter++);

	return std::string(buff);
}
//	����������/����� ���������
void GameScreen::EnemyIconMgr::EnemyIconAsWidget::SetSelection(bool enable)
{
	m_Icon->SetState(enable?1:0);
}
//	���������� ����� ������
void GameScreen::EnemyIconMgr::EnemyIconAsWidget::SetNumber(const char* number)
{
	m_Number->SetText(number);
}
//	��������� �� �������� ����
void GameScreen::EnemyIconMgr::EnemyIconAsWidget::OnChange(Widget* pChild)
{
	if(static_cast<Static*>(pChild)->LastEvent() == Static::LE_LDBLCLICK)
	{
		m_Link->OnClick();
	}
}

//**********************************************************************//
//	class GameScreen::HackVehicleMgr
int GameScreen::HackVehicleMgr::m_Left = 0;							
int GameScreen::HackVehicleMgr::m_Right = 0;							
GameScreen::HackVehicleMgr::HackVehicleMgr(GameScreen* owner) : m_Owner(owner)
{
}

GameScreen::HackVehicleMgr::~HackVehicleMgr()
{
	if(m_Icons.size()) Clear();
}

void GameScreen::HackVehicleMgr::Insert(Icon& icon)
{
	IconExtended* ie = new IconExtended(m_Owner,icon);

	ie->MoveTo(GetNextLocation(m_Icons.size()));
	ie->SetNumber(m_Icons.size()+1);

	m_Icons.push_back(ie);
}

void GameScreen::HackVehicleMgr::Remove(VehicleEntity* ve)
{
	for(std::list<IconExtended*>::iterator i=m_Icons.begin();i!=m_Icons.end();i++)
	{
		if((*i)->GetVehicleEntity() == ve)
		{
			//	������� ����
			delete (*i);
			m_Icons.erase(i);
			//	��������� ������� ������ ������
			UpdateLocation();
			break;
		}
	}
}

void GameScreen::HackVehicleMgr::Clear(void)
{
	for(std::list<IconExtended*>::iterator i=m_Icons.begin();i!=m_Icons.end();i++)
		delete (*i);
	m_Icons.clear();
}

GameScreen::HackVehicleMgr::Icon* GameScreen::HackVehicleMgr::ItemAt(const VehicleEntity* ve)
{
	for(std::list<IconExtended*>::iterator i=m_Icons.begin();i!=m_Icons.end();i++)
	{
		if((*i)->GetVehicleEntity() == ve)
		{
			return (*i);
		}
	}

	return 0;
}

GameScreen::HackVehicleMgr::Icon* GameScreen::HackVehicleMgr::ItemAt(unsigned int number)
{
	std::list<IconExtended*>::iterator i = m_Icons.begin();
	
	while((i!=m_Icons.end()) && number)
	{
		i++;
		number--;
	}
	if(i!=m_Icons.end()) return (*i);

	return 0;
}

ipnt2_t GameScreen::HackVehicleMgr::GetNextLocation(unsigned int size)
{
	const int width = GameScreen::HackVehicleMgr::IconAsWidget::Width();
	const int height = GameScreen::HackVehicleMgr::IconAsWidget::Height();

	const int in_row = (m_Right-m_Left)/width;
	const int height_factor = size/in_row; 
	const int width_factor = size-(height_factor*in_row);

	return ipnt2_t(m_Left+width*width_factor,height*height_factor);
}

void GameScreen::HackVehicleMgr::UpdateLocation(void)
{
	int size = 0;

	m_Right = m_Owner->GetHackVehicleIconRight();
	m_Left	 = m_Owner->GetHackVehicleIconLeft();
	for(std::list<IconExtended*>::iterator i=m_Icons.begin();i!=m_Icons.end();i++,size++)
	{
		(*i)->MoveTo(GetNextLocation(size));
		(*i)->SetNumber(size+1);
	}
}

//	���������� ����� ��� ���� ������ ������
void GameScreen::HackVehicleMgr::SetFlags(unsigned int flags)
{
	for(std::list<IconExtended*>::iterator i=m_Icons.begin();i!=m_Icons.end();i++)
		(*i)->SetFlags(flags);
}

//**********************************************************************
//	class GameScreen::HackVehicleMgr::IconExtended
GameScreen::HackVehicleMgr::IconExtended::IconExtended(GameScreen* owner,const GameScreen::HackVehicleMgr::Icon& icon) : GameScreen::HackVehicleMgr::Icon(icon),m_Owner(owner)
{
	m_Owner->Insert(m_Icon = new IconAsWidget(this),L_RIGHT_BOTTOM);
	m_Owner->BringToBottom(m_Icon->Name());
	SetFlags(m_Flags);
}

GameScreen::HackVehicleMgr::IconExtended::~IconExtended()
{
	m_Owner->Delete(m_Icon->Name());
}

void GameScreen::HackVehicleMgr::IconExtended::MoveTo(const ipnt2_t& pt)
{
	m_Icon->MoveTo(pt.x,pt.y);
}

void GameScreen::HackVehicleMgr::IconExtended::SetNumber(int number)
{
	static char buff[20];

	sprintf(buff,"%d",number);
	m_Icon->SetNumber(buff);
}

void GameScreen::HackVehicleMgr::IconExtended::SetFlags(unsigned int flags)
{
	m_Flags = flags;
	//	-----------------------------------------------------------------------
	(m_Flags&F_SELECTED)?m_Icon->SetSelection(true):m_Icon->SetSelection(false);
}
//	����������� �� ������
void GameScreen::HackVehicleMgr::IconExtended::OnClick(GameScreen::BN_CLICK btn_clk)
{
	m_Owner->OnHackVehicleIconClick(m_VehicleEntity,btn_clk);
}


//**********************************************************************
//	class GameScreen::HackVehicleMgr::IconAsWidget
const char* GameScreen::HackVehicleMgr::IconAsWidget::m_IconName = "pictures/interface/gamescreen/hv_icon.tga";
const char* GameScreen::HackVehicleMgr::IconAsWidget::m_IconSelectedName = "pictures/interface/gamescreen/hv_icon_selected.tga";
const char* GameScreen::HackVehicleMgr::IconAsWidget::m_NameDesc = "hv_icon_";
int GameScreen::HackVehicleMgr::IconAsWidget::m_Width = 32;
int GameScreen::HackVehicleMgr::IconAsWidget::m_Height = 32;
GameScreen::HackVehicleMgr::IconAsWidget::IconAsWidget(IconExtended* link)
:	Widget(GenerateName().c_str()),
	m_Selected(false),
	m_Link(link),
	m_lastHealth(0),
	m_lastMovepnts(0)
{
	Insert(m_Icon = new Static("icon"));
	m_Icon->AddImage(DXImageMgr::Instance()->CreateImage(m_IconName));
	m_Icon->AddImage(DXImageMgr::Instance()->CreateImage(m_IconSelectedName));
	m_Icon->SetState(0);
	//	������ ������� ��������
	DXImageMgr::Instance()->GetImageSize(static_cast<DXImage*>(m_Icon->GetImage(0)),&m_Width,&m_Height);
	//	������� ��������� ����
	Insert(m_Number = new Text("number"));
	m_Number->SetRegion(0,0,m_Width,m_Height);
	m_Number->SetFont("oskar",0xffff00);
	m_Number->Align(LogicalText::T_HCENTER|LogicalText::T_VCENTER);
	m_Number->SetText("X");
	//	������������� ����������� ���������
	m_Number->m_Tip.m_Text = AssemblyTip(m_Link->GetVehicleEntity()->GetEntityContext()->GetHealth(),
								         m_Link->GetVehicleEntity()->GetEntityContext()->GetMovepnts());
	m_Number->m_Tip.m_Activated = true;
	m_Number->Enable(false);
	//	������������� �������	
	m_Icon->GetLocation()->Create(0,0,m_Width,m_Height);
	GetLocation()->Create(0,0,m_Width,m_Height);
}

GameScreen::HackVehicleMgr::IconAsWidget::~IconAsWidget(void)
{
}

std::string GameScreen::HackVehicleMgr::IconAsWidget::GenerateName(void)
{
	static int counter = 0;
	static char buff[50];

	sprintf(buff,"%s%ld",m_NameDesc,counter++);

	return std::string(buff);
}
//	����������/����� ���������
void GameScreen::HackVehicleMgr::IconAsWidget::SetSelection(bool enable)
{
	m_Icon->SetState(enable?1:0);
}
//	���������� ����� ������
void GameScreen::HackVehicleMgr::IconAsWidget::SetNumber(const char* number)
{
	m_Number->SetText(number);
}
//	��������� �� �������� ����
void GameScreen::HackVehicleMgr::IconAsWidget::OnChange(Widget* pChild)
{
	GameScreen::BN_CLICK btn_clk = BNC_NONE;

	switch(static_cast<Static*>(pChild)->LastEvent())
	{
	case Static::LE_LBUTTONDOWN:
		btn_clk = GameScreen::BNC_LCLICK;
		break;
	case Static::LE_RBUTTONDOWN:
		btn_clk = GameScreen::BNC_RCLICK;
		break;
	case Static::LE_LDBLCLICK:
		btn_clk = GameScreen::BNC_LDBLCLICK;
		break;
	case Static::LE_RDBLCLICK:
		btn_clk = GameScreen::BNC_RDBLCLICK;
		break;
	}
	m_Link->OnClick(btn_clk);
}
//	���������� �������� �� ��������� ���������
bool GameScreen::HackVehicleMgr::IconAsWidget::Tick(void)
{
	if(m_lastHealth != m_Link->GetVehicleEntity()->GetEntityContext()->GetHealth() ||
	   m_lastMovepnts != m_Link->GetVehicleEntity()->GetEntityContext()->GetMovepnts())
	{
		m_Number->m_Tip.m_Text = AssemblyTip(m_Link->GetVehicleEntity()->GetEntityContext()->GetHealth(),
										     m_Link->GetVehicleEntity()->GetEntityContext()->GetMovepnts());
	}

	return false;
}
//	������� ����������� ���������
std::string GameScreen::HackVehicleMgr::IconAsWidget::AssemblyTip(int h,int mp)
{
	m_lastHealth = h;				//	��������
	m_lastMovepnts = mp;			//	���� ����
	return mlprintf("%s (%d/%d)",m_Link->GetTip().c_str(),m_lastHealth,m_lastMovepnts);
}


//**********************************************************************//
//	class GameScreen::PersonIconMgr::Monitor::OpenStrategy
class GameScreen::PersonIconMgr::Monitor::OpenStrategy : public Strategy
{
private:
	float m_Seconds;
	float m_Position;
	float m_Aim;
public:
	OpenStrategy(GameScreen::PersonIconMgr::Monitor* controller) : Strategy(controller),m_Seconds(0) {}
	virtual ~OpenStrategy(){}
private:
	void Tick(void)
	{
		float step;

		if(!m_Seconds)
		{
			m_Seconds = Timer::GetSeconds();
			m_Position = m_Controller->m_MinWidth;
			m_Aim = m_Controller->GetImage()->GetWidth() - 1;// m_Controller->m_MaxWidth;
		}

		step = (Timer::GetSeconds()-m_Seconds)*m_Controller->m_MovingStep;
		if(m_Position < m_Aim)
		{//	��������� ����
			step = std::min(m_Aim-m_Position,step);
			m_Position += step;
			m_Controller->MoveTo(m_Position-m_Controller->GetLocation()->Region()->Width(),m_Controller->GetLocation()->Region()->Y());
		}
		else
		{
			m_Controller->OnStrategyFinish();
		}
	}
	COMMAND Type(void) {return C_OPEN;}
};
//**********************************************************************//
//	class GameScreen::PersonIconMgr::Monitor::QuickCloseStrategy
class GameScreen::PersonIconMgr::Monitor::QuickCloseStrategy : public Strategy
{
private:
	float m_Seconds;
	float m_Position;
	float m_Aim;
public:
	QuickCloseStrategy(GameScreen::PersonIconMgr::Monitor* controller) : Strategy(controller),m_Seconds(0)
	{

	}
	virtual ~QuickCloseStrategy() {}
private:
	void Tick(void)
	{
		float step;

		if(!m_Seconds)
		{
			m_Seconds = Timer::GetSeconds();
			m_Position = m_Controller->GetImage()->GetWidth() - 1;// m_Controller->m_MaxWidth;
			m_Aim = m_Controller->m_MinWidth;
		}

		step = (Timer::GetSeconds()-m_Seconds)*m_Controller->m_MovingStep;
		if(m_Position > m_Aim)
		{//	��������� ����
			step = -std::min(m_Position-m_Aim,step);
			m_Position += step;
			m_Controller->MoveTo(m_Position-m_Controller->GetLocation()->Region()->Width(),m_Controller->GetLocation()->Region()->Y());
		}
		else
		{
			m_Controller->OnStrategyFinish();
		}
	}
	COMMAND Type(void) {return C_QUICK_CLOSE;}
};
//**********************************************************************//
//	class GameScreen::PersonIconMgr::Monitor::SlowCloseStrategy
class GameScreen::PersonIconMgr::Monitor::SlowCloseStrategy : public Strategy
{
private:
	const float m_WaitTime;
	float m_Seconds;
	float m_Position;
	float m_Aim;
	bool m_Wait;
public:
	SlowCloseStrategy(GameScreen::PersonIconMgr::Monitor* controller) : Strategy(controller),m_Seconds(0),m_WaitTime(4.5f),m_Wait(true) {}
	virtual ~SlowCloseStrategy() {}
private:
	void Tick(void)
	{
		float step;

		if(!m_Seconds)
		{
			m_Seconds = Timer::GetSeconds();
			m_Position = m_Controller->GetImage()->GetWidth() - 1;// m_Controller->m_MaxWidth;
			m_Aim = m_Controller->m_MinWidth;
		}
		if(m_Wait)
		{
			if((Timer::GetSeconds()-m_Seconds)>m_WaitTime)
			{
				m_Seconds = Timer::GetSeconds();
				m_Wait = false;
			}
		}
		else
		{
			step = (Timer::GetSeconds()-m_Seconds)*m_Controller->m_MovingStep;
			if(m_Position > m_Aim)
			{//	��������� ����
				step = -std::min(m_Position-m_Aim,step);
				m_Position += step;
				m_Controller->MoveTo(m_Position-m_Controller->GetLocation()->Region()->Width(),m_Controller->GetLocation()->Region()->Y());
			}
			else
			{
				m_Controller->OnStrategyFinish();
			}
		}
	}
	COMMAND Type(void) {return C_SLOW_CLOSE;}
};

//**********************************************************************
//	class GameScreen::PersonIconMgr
unsigned int GameScreen::PersonIconMgr::m_MaxMonitors = 8;
GameScreen::PersonIconMgr::PersonIconMgr(GameScreen* owner) : m_Owner(owner)
{
	char buff[20];

	//	��������� ������ �� ����������
	for(unsigned int i=0;i<m_MaxMonitors;i++)
	{
		Monitor* monitor;

		sprintf(buff,"pm%d",i);
		m_Monitors.push_back(monitor = new Monitor(buff,this));
		m_MonitorViews.push_back(monitor);
		m_Owner->Insert(monitor,L_LEFT_TOP);
		monitor->Move(0,(monitor->GetLocation()->Region()->Height()+2)*i);
	}
}

GameScreen::PersonIconMgr::~PersonIconMgr()
{
	for(std::list<Icon*>::iterator i=m_Icons.begin();i!=m_Icons.end();i++)
		delete *i;
}

void GameScreen::PersonIconMgr::InsertIcon(const Icon& icon)
{
	//	��������� �� ������������� ���������� �����
	if(m_Icons.size() == m_MaxMonitors) throw CASUS("��� ���������� ����� ��� ����� ������ ���������.");
	for(std::list<Monitor*>::iterator m=m_Monitors.begin();m!=m_Monitors.end();m++)
	{
		if(!(*m)->IsBusy())
		{
			m_Icons.push_back(new Icon(icon,(*m)));
			break;
		}
	}
}

GameScreen::PersonIconMgr::Icon* GameScreen::PersonIconMgr::IconAt(const HumanEntity* he)
{
	for(std::list<Icon*>::iterator i=m_Icons.begin();i!=m_Icons.end();i++)
	{
		if((*i)->GetHumanEntity() == he) return *i;
	}

	return 0;
}

GameScreen::PersonIconMgr::Icon* GameScreen::PersonIconMgr::IconAt(unsigned int number)
{
	for(std::list<Icon*>::iterator i=m_Icons.begin();i!=m_Icons.end();i++)
	{
		if(!number)	return *i;
		number--;
	}

	return 0;
}

void GameScreen::PersonIconMgr::RemoveIcon(const HumanEntity* he,REMOVE_TYPE rt)
{
	for(std::list<Icon*>::iterator i=m_Icons.begin();i!=m_Icons.end();i++)
	{
		if((*i)->GetHumanEntity() == he) 
		{
			//	������� ������
			(*i)->Remove(rt);
			m_Icons.erase(i);
			//	����������������� ��������
			for(std::list<Monitor*>::iterator m=m_Monitors.begin();m!=m_Monitors.end();m++)
			{
				if(!(*m)->IsBusy())
				{//	����� ������ �� ������� �������
					//	�������� ��� � ����� ������
					Monitor* monitor = *m;
					m_Monitors.erase(m);
					m_Monitors.push_back(monitor);
					break;
				}
			}
			break;
		}
	}
}
//	������� ��� ������ 
void GameScreen::PersonIconMgr::RemoveAll(void)
{
	int j = 0;

	//	������� ��� ������
	for(std::list<Icon*>::iterator i=m_Icons.begin();i!=m_Icons.end();i++)
		delete (*i);
	m_Icons.clear();
	m_Monitors.clear();
	//	�������� � ������������ ��� ��������� �� ���������
	for(std::list<Monitor*>::iterator m=m_MonitorViews.begin();m!=m_MonitorViews.end();m++,j++)
	{
		m_Monitors.push_back(*m);
		(*m)->Clear();
	}
}
//	���
void GameScreen::PersonIconMgr::Tick(void)
{
	static bool was_unlim = UnlimitedMoves();
	const bool unlim = UnlimitedMoves();

	if(unlim != was_unlim)
	{
		was_unlim = unlim;

		for(std::list<Icon*>::iterator i=m_Icons.begin();i!=m_Icons.end();i++)
		{
			(*i)->m_MonitorState.m_Flags |= MonitorTraits::F_MAXMOVEPOINTS|MonitorTraits::F_MOVEPOINTS;
		}
	}

	for(std::list<Icon*>::iterator i=m_Icons.begin();i!=m_Icons.end();i++)
		(*i)->Tick();
}
//	�������������� ���������� ���������
void GameScreen::PersonIconMgr::SwapMonitors(void)
{	
	std::list<Monitor*>::iterator m;
	int i = 0;

	//	����������������� ��������
	for(m=m_MonitorViews.begin();m!=m_MonitorViews.end();m++)
	{
		if((*m)->IsClosed())
		{//	����� ������ �� ������� �������
			//	�������� ��� � ����� ������
			Monitor* monitor = *m;
			m_MonitorViews.erase(m);
			m_MonitorViews.push_back(monitor);
			break;
		}
	}
	//	������������� ����� ���������� ��� ���������
	for(m=m_MonitorViews.begin();m!=m_MonitorViews.end();m++,i++)
	{
		(*m)->MoveTo((*m)->GetLocation()->Region()->X(),((*m)->GetLocation()->Region()->Height()+2)*i);
	}
}

//**********************************************************************
//	class GameScreen::PersonIconMgr::Icon
GameScreen::PersonIconMgr::Icon::Icon(HumanEntity* he) : m_HumanEntity(he),m_Monitor(0)
{
}

GameScreen::PersonIconMgr::Icon::Icon(const Icon& icon,GameScreen::PersonIconMgr::Monitor* monitor) : m_Monitor(monitor),
								 m_HumanEntity(icon.m_HumanEntity),m_MonitorState(icon.m_MonitorState)
{
	//	�������� �������
	m_Monitor->OnBusy(true);
	//	������ ��������� ���������
	m_Monitor->ToOrder(Monitor::C_CHANGE_STATE,m_MonitorState);
	//	��������� �������
	m_Monitor->ToOrder(Monitor::C_OPEN);
}

GameScreen::PersonIconMgr::Icon::~Icon()
{
}
//	���������� ��������
void GameScreen::PersonIconMgr::Icon::SetHealth(int health)
{
	m_MonitorState.m_Flags |= MonitorTraits::F_HEALTH;
	m_MonitorState.m_Health = health;
}

void GameScreen::PersonIconMgr::Icon::SetMaxHealth(int health)
{
	m_MonitorState.m_Flags |= MonitorTraits::F_MAXHEALTH;
	m_MonitorState.m_MaxHealth = health;
}

void GameScreen::PersonIconMgr::Icon::SetMaxMovePoints(int pt)
{
	m_MonitorState.m_Flags |= MonitorTraits::F_MAXMOVEPOINTS;
	m_MonitorState.m_MaxMovePoints = pt;
}

//	���������� ������� � ��� ��������� ���������
void GameScreen::PersonIconMgr::Icon::SetPortrait(const char* name,MonitorTraits::SEL_TYPE selection)
{
	m_MonitorState.m_Flags |= MonitorTraits::F_PORTRAIT;
	m_MonitorState.m_Flags |= MonitorTraits::F_SELECTION;
	m_MonitorState.m_Portrait = name;
	m_MonitorState.m_Selection = selection;
}

void GameScreen::PersonIconMgr::Icon::SetMovePoints(int mp)
{
	m_MonitorState.m_Flags |= MonitorTraits::F_MOVEPOINTS;
	m_MonitorState.m_MovePoints = mp;
}

void GameScreen::PersonIconMgr::Icon::SetHeartbeat(MonitorTraits::HEARTBEAT hb)
{
	m_MonitorState.m_Flags |= MonitorTraits::F_HEARTBEAT;
	m_MonitorState.m_Heartbeat = hb;
}

void GameScreen::PersonIconMgr::Icon::SetStateFlags(unsigned int flags)
{
	m_MonitorState.m_Flags |= MonitorTraits::F_ICONFLAGS;
	m_MonitorState.m_StateFlags = flags;
}

//	�������� ���������� � �����
void GameScreen::PersonIconMgr::Icon::Tick(void)
{
	if(m_MonitorState.m_Flags && m_Monitor)
	{
		m_Monitor->ToOrder(Monitor::C_CHANGE_STATE,m_MonitorState);
		m_MonitorState.m_Flags = 0;
	}
}
//	������� ������ � ���������� ������ this � ������
void GameScreen::PersonIconMgr::Icon::Remove(REMOVE_TYPE rt)
{
	if(m_Monitor)
	{	
		if(m_MonitorState.m_Flags || (m_MonitorState.m_StateFlags&MonitorTraits::IF_LEVELUP))
		{
			m_MonitorState.m_StateFlags &= ~MonitorTraits::IF_LEVELUP;
			m_MonitorState.m_Flags |= MonitorTraits::F_ICONFLAGS;
			m_Monitor->ToOrder(Monitor::C_CHANGE_STATE,m_MonitorState);
		}
		switch(rt)
		{
		case RT_QUICK:
			m_Monitor->ToOrder(Monitor::C_QUICK_CLOSE);
			break;
		case RT_SLOW:
			m_Monitor->ToOrder(Monitor::C_SLOW_CLOSE);
			break;
		}

		m_Monitor->OnBusy(false);
	}

	delete this;
}
//**********************************************************************//
//	class GameScreen::PersonIconMgr::Monitor
const char* GameScreen::PersonIconMgr::Monitor::m_ScriptName = "PersonMonitor";
//const float GameScreen::PersonIconMgr::Monitor::m_MaxWidth = 108;//81;
const float GameScreen::PersonIconMgr::Monitor::m_MinWidth = 11;
const float GameScreen::PersonIconMgr::Monitor::m_MovingStep = 100;
const float GameScreen::PersonIconMgr::Monitor::m_LevelUpFlipTime = 0.5f;

//=====================================================================================//
//                    GameScreen::PersonIconMgr::Monitor::Monitor()                    //
//=====================================================================================//
GameScreen::PersonIconMgr::Monitor::Monitor(const char* name,PersonIconMgr* controller)
:	Dialog(name,m_ScriptName),
	m_Controller(controller),
	m_Strategy(0),
	m_Busy(false),
	m_Closed(true),
	m_IsLevelUp(false)
{
	//	��������� �������� ������
	Focus()->Enable(WidgetFocus::FF_KEYBOARD,false);
	//	�������� 
	m_MovePoints = static_cast<Text*>(WidgetFactory::Instance()->Assert(this,"movepoints"));
	m_Health = static_cast<Text*>(WidgetFactory::Instance()->Assert(this,"health"));
	m_MaxMovePoints = static_cast<Text*>(WidgetFactory::Instance()->Assert(this,"maxmovepoints"));
	m_MaxHealth = static_cast<Text*>(WidgetFactory::Instance()->Assert(this,"maxhealth"));
	//	�����������
	MoveTo(m_MinWidth-GetLocation()->Region()->Width(),GetLocation()->Region()->Y());
	//	���������� ����������� ��������
	WidgetFactory::Instance()->Assert(this,"levelup")->SetVisible(false);
}

GameScreen::PersonIconMgr::Monitor::~Monitor()
{
	//	������� ���������� ���������
	if(m_Strategy) delete m_Strategy;
	//	������� ����������� ��������
	WidgetFactory::Instance()->Assert(this,"icon")->SetImage(0);
	for(std::map<std::string,WidgetImage*>::iterator i=m_IconImages.begin();i!=m_IconImages.end();i++)
		i->second->Release();
}
//	������������
void GameScreen::PersonIconMgr::Monitor::ToOrder(COMMAND command)
{
	m_Commands.push_back(Order(command,MonitorTraits()));
}
//	������������
void GameScreen::PersonIconMgr::Monitor::ToOrder(COMMAND command,const MonitorTraits& traits)
{
	m_Commands.push_back(Order(command,traits));
}

//	������� ���� ���� ���������
bool GameScreen::PersonIconMgr::Monitor::Tick(void)
{
	if(!m_Strategy && m_Commands.size())
	{
		switch(m_Commands[0].m_Command)
		{
		case C_OPEN:
			m_Closed = false;
			m_Strategy = new OpenStrategy(this);
			break;
		case C_QUICK_CLOSE:
			m_Strategy = new QuickCloseStrategy(this);
			break;
		case C_SLOW_CLOSE:
			m_Strategy = new SlowCloseStrategy(this);
			break;
		case C_CHANGE_STATE:
			UpdateState(m_Commands[0].m_Traits);
			break;
		}
		//	������� �������
		m_Commands.pop_front();
	}
	
	if(m_Strategy) m_Strategy->Tick();
	//	���������������� LevelUp'�
	if(m_IsLevelUp)
	{
		if((Timer::GetSeconds()-m_LevelUpSeconds)>m_LevelUpFlipTime)
		{
			Child("levelup")->SetVisible(!WidgetFactory::Instance()->Assert(this,"levelup")->IsVisible());
			m_LevelUpSeconds = Timer::GetSeconds();
		}
	}

	return false;
}
//	����������� � ���,��� �������� ���������
void GameScreen::PersonIconMgr::Monitor::OnStrategyFinish(void)
{
	switch(m_Strategy->Type())
	{
	case C_QUICK_CLOSE:
	case C_SLOW_CLOSE:
		m_Closed = true;
		m_Controller->SwapMonitors();
		break;
	}
	//	������� ���������
	delete m_Strategy;
	m_Strategy = 0;
}
//	�������� ���������
void GameScreen::PersonIconMgr::Monitor::UpdateState(const MonitorTraits& traits)
{
	const bool unlim = UnlimitedMoves();
	static char pBuff[20];

	//	������� ��������
	if(traits.m_Flags&MonitorTraits::F_HEALTH)
	{
		itoa(traits.m_Health,pBuff,10);
		m_Health->SetText(pBuff);
	}
	if(traits.m_Flags&MonitorTraits::F_MAXHEALTH)
	{
		itoa(traits.m_MaxHealth,pBuff,10);
		m_MaxHealth->SetText(pBuff);
	}
	//	���-�� ����������
	if(traits.m_Flags&MonitorTraits::F_MOVEPOINTS)
	{
		if(unlim)
		{
			m_MovePoints->SetText("--");
		}
		else
		{
			itoa(traits.m_MovePoints,pBuff,10);
			m_MovePoints->SetText(pBuff);
		}
	}
	if(traits.m_Flags&MonitorTraits::F_MAXMOVEPOINTS)
	{
		if(unlim)
		{
			m_MaxMovePoints->SetText("--");
		}
		else
		{
			itoa(traits.m_MaxMovePoints,pBuff,10);
			m_MaxMovePoints->SetText(pBuff);
		}
	}
	//	��� ������������
	if(traits.m_Flags&MonitorTraits::F_HEARTBEAT)
	{
		static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"heartbeat"))->SetState(traits.m_Heartbeat);
	}
	//	���������� ������ ��������� � ���������
	if(traits.m_Flags&MonitorTraits::F_PORTRAIT)
	{
		std::string portrait_ex = traits.m_Portrait;

		//	��������� ��������� ���������
		switch(traits.m_Selection)
		{
		case MonitorTraits::ST_NONE:
			portrait_ex += g_pIconNone;
			break;
		case MonitorTraits::ST_DEAD:
			portrait_ex += g_pIconDied;
			break;
		case MonitorTraits::ST_NORMAL:
			break;
		}
		const std::string icon_name = std::string("pictures/interface/portraits/")+std::string(portrait_ex)+".tga";
		std::map<std::string,WidgetImage*>::iterator i = m_IconImages.find(icon_name);
		
		if(i != m_IconImages.end())
		{
			WidgetFactory::Instance()->Assert(this,"icon")->SetImage(i->second);
		}
		else
		{
			m_IconImages[icon_name] = DXImageMgr::Instance()->CreateImage(icon_name.c_str());
			i = m_IconImages.find(icon_name);
		}
		WidgetFactory::Instance()->Assert(this,"icon")->SetImage(i->second);
	}
	//	��������� ����� ���������
	if(traits.m_Flags&MonitorTraits::F_ICONFLAGS)
	{
		//	������� � �������
		WidgetFactory::Instance()->Assert(this,"invehicle")->SetVisible(traits.m_StateFlags&MonitorTraits::IF_INVEHICLE);
		//	������� ������� �����
		if((m_IsLevelUp = (traits.m_StateFlags&MonitorTraits::IF_LEVELUP)) != 0)
		{
			WidgetFactory::Instance()->Assert(this,"levelup")->SetVisible(true);
			m_LevelUpSeconds = Timer::GetSeconds();
		}
		else
			WidgetFactory::Instance()->Assert(this,"levelup")->SetVisible(false);
		//	������� � ��������
		WidgetFactory::Instance()->Assert(this,"inmovement")->SetVisible(traits.m_StateFlags&MonitorTraits::IF_INMOVEMENT);
		//	������� � ������
		WidgetFactory::Instance()->Assert(this,"inpanic")->SetVisible(traits.m_StateFlags&MonitorTraits::IF_INPANIC);
	}
}
//	���������� ��������� ��������
void GameScreen::PersonIconMgr::Monitor::OnBusy(bool busy)
{
	m_Busy = busy;
}

void GameScreen::PersonIconMgr::Monitor::OnSystemChange(void)
{
	for(std::map<std::string,WidgetImage*>::iterator i = m_IconImages.begin();i!=m_IconImages.end();i++)
		i->second->Reload();
	//	-----------------------------------------
	Widget::OnSystemChange();
}
//	�������� ������� 
void GameScreen::PersonIconMgr::Monitor::Clear(void)
{
	//	���������� ��������� ���������
	if(m_Strategy)
	{
		delete m_Strategy;
		m_Strategy = 0;
	}
	//	������� ������ ������
	m_Commands.clear();
	//	��������� ������
	MoveTo(m_MinWidth-GetLocation()->Region()->Width(),GetLocation()->Region()->Y());
	m_Closed = true;
	m_Busy = false;
}
//	��������� �� �������
void GameScreen::PersonIconMgr::Monitor::OnStatic(Static *pStatic)
{
	if(Parent() && !strcmp("icon",pStatic->Name()) && (pStatic->LastEvent() != Static::LE_NONE))
		Parent()->OnChange(this);
}

//*****************************************************************************************
//	class GameScreen
int GameScreen::GetHackVehicleIconLeft(void)
{
	return Child("pm0")->GetImage()->GetWidth() + 50;
	//return PersonIconMgr::Monitor::m_MaxWidth+50;
}

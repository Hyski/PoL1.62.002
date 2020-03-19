#include "logicdefs.h"

#include <common/D3DApp/Input/Input.h>

#include <undercover/VolumeHelper.h>
#include <undercover/interface/screens.h>
#include <undercover/interface/console.h>
#include <undercover/interface/interface.h>
#include <undercover/Interface/loadingscreen.h>
#include <undercover/interface/dialogscreen.h>
#include <undercover/interface/optionsscreen.h>
#include <undercover/interface/DescriptionScreen.h>
#include <Muffle/ISound.h>

#include "form.h"
#include "thing.h"
#include "phrase.h"
#include "entity.h"
#include "aiutils.h"
#include "sndutils.h"
#include "cameraman.h"
#include "dirtylinks.h"
#include "cheatparser.h"
#include "gameobserver.h" 
//#include "phrasemanager.h"

#include "GameFormImp.h"

//================================================================

namespace{
  
    class ProgressBarImp : public AbstractProgressBar{
    public:

        ProgressBarImp(float from, float to) 
        {
            Interface::Instance()->ProBar()->SetRange(from, to);
        }
    
        //установить движок 
        void SetPos(float value, const std::string& text)
        {
            Interface::Instance()->ProBar()->SetPercent(value);
            if(text.size()) Interface::Instance()->ProBar()->SetTitle(text.c_str());
        }
    };
};

//================================================================

//
// экран HiddenMovement
//
class HMForm : public BaseForm{
public:

    HMForm() : m_update_time(0), m_update_delta(3.0f){}

    void Init(const ini_s& ini)
    {
        const hm_ini_s& hini = static_cast<const hm_ini_s&>(ini);

        if(hini.m_player == PT_ENEMY) Screens::Instance()->HiddenMovement()->SetBackground(HiddenMovementScreen::B_ENEMY);
        if(hini.m_player == PT_CIVILIAN) Screens::Instance()->HiddenMovement()->SetBackground(HiddenMovementScreen::B_CIVILIAN);
    }
    
    void Show()
    {   
        Enable(true);
        m_update_time = 0;

        Screens::Instance()->Activate(Screens::SID_HIDDENMOVEMENT);
    }
    
    //обработка ввода
    void HandleInput(state_type* st)
    {
        if(m_update_time > Timer::GetSeconds())
            return;

        m_update_time = Timer::GetSeconds() + m_update_delta;

        Enable(m_fenable);
    }

    //включить/выключить экран
    void Enable(bool flag)
    {       
        DirtyLinks::EnableLevelRender(!flag);
        DirtyLinks::EnableLevelAndEffectSound(!flag);

        Screens::Instance()->HiddenMovement()->SetVisible(flag);
    }

    //установить запрос на вкл/выкл экрана
    void SetEnableReq(bool flag)
    {
        m_fenable = flag;

        //запрос на выключение экрана обработать немедленно
        if(!m_fenable) m_update_time = 0;
    }

    //сохранить/загрузить настройки меню
    void MakeSaveLoad(SavSlot& slot) {}
    
private:

    float m_update_time;
    float m_update_delta;

    bool  m_fenable;    
};

//================================================================

namespace{

    //
    // реализация устройства камеры
    //
    class CameraDeviceImp : public CameraDevice{
    public:

        //видима ли точка в камере
        bool IsInCone(const point3& pnt3) const
        { return DirtyLinks::IsInCameraCone(pnt3); }

        //переместить камеру
        void Move(const point3& pnt, float time)
        { DirtyLinks::MoveCamera(pnt, time); }

        //снимать (спрятать экран HM, показать уровень)
        void DoFilm(bool flag, BaseEntity* entity)
        {
            Forms::GetInst()->GetHMForm()->SetEnableReq(!flag);
        }    
    };
}

CameraDevice* CameraDevice::GetInst()
{
    static CameraDeviceImp imp;
    return &imp;
}

//================================================================

FormFactory* FormFactory::GetInst()
{
    static FormFactory imp;
    return &imp;
}

AbstractProgressBar* FormFactory::CreateLoadBar(float from, float to)
{
    return new ProgressBarImp(from, to);
}

//================================================================

bool AbstractScroller::CanScroll() const
{
    AbstractScroller* This = const_cast<AbstractScroller*>(this);

    if(!This->IsEnd()){
        This->Next();        
        bool fhave_next = !This->IsEnd();        
        This->Prev();

        if(fhave_next) return true;
    }

    if(!This->IsREnd()){
        This->Prev();
        bool fhave_prev = !This->IsREnd();
        This->Next();

        if(fhave_prev) return true;
    }
    
    return false;
}

//================================================================

//
// окно консоли
//
class ConsoleForm : private Console::Controller{
public:

    ConsoleForm()  { Console::AddController(this);    }
    ~ConsoleForm() { Console::RemoveController(this); }

private:

    void Activate(void)
    {
#ifdef _HOME_VERSION

        CheatServer server;
        CheatParser parser(&server);

        std::string script = Console::GetCommandLine();

        if(!parser.Parse(script)){

            Console::AddString(script.c_str());

            script.clear();
            for(int k = 0; k < parser.GetLastPos(); k++){
                script += ' ';
            }

            script += '^';            

            Console::AddString(script.c_str());
        } 
#endif
    }
};

//================================================================

//
// базовый класс диалога
//
class BaseDialog{
public:

    virtual ~BaseDialog(){}
    
    //обработка ввода
    virtual void HandleInput() = 0;
    //диалог активен?
    virtual bool IsActive() const = 0; 
};

//================================================================

namespace{

    class DescDlg : public BaseDialog, public DescriptionScreenController{
    public:
        
        static DescDlg* GetInst()
        {
            static DescDlg imp;
            return &imp;
        }
     
        bool IsActive() const { return m_factive; }
        
        void Init(BaseThing* thing) 
        { 
            m_factive = true;
            
            DescriptionScreen::ItemInfo info;
            
            const ThingInfo* thing_info = thing->GetInfo();
            
            info.m_Name  = thing_info->GetName().c_str();
            info.m_Text  = thing_info->GetDesc().c_str();
            info.m_Image = thing_info->GetShader().c_str();
            
            ThingInfo::traits_string traits; 
            
            thing_info->GetTraitsString(&traits);
            
            info.m_LeftColumn  = traits.m_left_col.c_str();
            info.m_RightColumn = traits.m_right_col.c_str();

			

            
            Screens::Instance()->Activate(Screens::SID_DESCRIPTION);
            Screens::Instance()->Description()->SetItemInfo(info);
            
            Screens::Instance()->Description()->SetController(this);

            ThingHandbook::GetInst()->Push(thing->GetInfo());
        }

        void Init(const std::string& text) 
        { 
            m_factive = true;
            
            DescriptionScreen::ItemInfo info;

            info.m_Name  = 0;
            info.m_Image = 0;
            info.m_Text  = text.c_str();
            
            Screens::Instance()->Activate(Screens::SID_DESCRIPTION);
            Screens::Instance()->Description()->SetItemInfo(info);
            Screens::Instance()->Description()->SetController(this);
        }
        
        void HandleInput() { if(Input::KeyBack(DIK_ESCAPE)) OnClose(); }
        
    private:
        
        DescDlg(){}
        
        void OnClose() { m_factive = false; }
        
    private:
        
        bool m_factive;  
    };
}

//================================================================

namespace{

    class OptionsDlg : public BaseDialog, private OptionsScreenController{
    public:
        
        static OptionsDlg* GetInst()
        {
            static OptionsDlg imp;
            return &imp;
        }

        bool IsActive() const { return m_factive; }

        void Init(Forms::options_mode mode)
        {
            m_mode = mode;

            m_factive = true;
            Screens::Instance()->Activate(Screens::SID_OPTIONS);
            Screens::Instance()->Options()->SetController(this);

            if(m_mode == Forms::ODM_GAMEMENU)
                Screens::Instance()->Options()->SetMode(OptionsScreen::M_GAMEMENU);
            else
                Screens::Instance()->Options()->SetMode(OptionsScreen::M_LOADINGMENU);
        }
        
        void HandleInput() 
        { 
            if(Input::KeyBack(DIK_ESCAPE)) OnBack();
        }
        
    private:

        OptionsDlg(){}        

        void OnBack(void)
        {
            m_factive = false;
            if(m_mode != Forms::ODM_GAMEMENU) Forms::GetInst()->ShowMainMenuForm();
        }    
        
    private:
            
        bool m_factive;
        Forms::options_mode m_mode;
    };
}

//=======================================================================================

Forms* Forms::GetInst()
{
    static Forms imp;
    return &imp;
}

//=====================================================================================//
//                                   Forms::Forms()                                    //
//=====================================================================================//
Forms::Forms()
:	m_dialog(0),
	m_console(new ConsoleForm),
	m_current(0)
{
}

//=====================================================================================//
//                                   Forms::~Forms()                                   //
//=====================================================================================//
Forms::~Forms()
{
}

//=====================================================================================//
//                                 void Forms::Reset()                                 //
//=====================================================================================//
void Forms::Reset(reset_type type)
{
    if(type == RT_SHUT)
	{
		m_hm_form.reset();
		m_shop_form.reset();
		m_game_form.reset();
		m_panic_form.reset();
		m_journal_form.reset();
		m_inventory_form.reset();
    }
	else
	{
		//изначально текущим явл. игровое меню
		m_dialog = 0;
		ShowGameForm();
	}
}

//=====================================================================================//
//                             HMForm* Forms::GetHMForm()                              //
//=====================================================================================//
HMForm* Forms::GetHMForm()
{
	if(!m_hm_form.get()) m_hm_form.reset(new HMForm);
    return m_hm_form.get();
}

//=====================================================================================//
//                           BaseForm* Forms::GetPanicForm()                           //
//=====================================================================================//
BaseForm* Forms::GetPanicForm()
{
	if(!m_panic_form.get()) m_panic_form.reset(FormFactory::GetInst()->CreatePanicForm());
    return m_panic_form.get();
}

//=====================================================================================//
//                           BaseForm* Forms::GetShopForm()                            //
//=====================================================================================//
BaseForm* Forms::GetShopForm()
{ 
    if(!m_shop_form.get()) m_shop_form.reset(FormFactory::GetInst()->CreateShopForm());
    return m_shop_form.get();
}

//=====================================================================================//
//                           bool Forms::HasGameForm() const                           //
//=====================================================================================//
bool Forms::HasGameForm() const
{
	return m_game_form.get() != 0;
}

//=====================================================================================//
//                           BaseForm* Forms::GetGameForm()                            //
//=====================================================================================//
GameFormImp *Forms::GetGameForm()
{
	if(!m_game_form.get()) m_game_form.reset(FormFactory::GetInst()->CreateGameForm());
    return m_game_form.get();
}

//=====================================================================================//
//                        JournalForm* Forms::GetJournalForm()                         //
//=====================================================================================//
JournalForm* Forms::GetJournalForm()
{
	if(!m_journal_form.get()) m_journal_form.reset(FormFactory::GetInst()->CreateJournalForm());
    return m_journal_form.get();
}

//=====================================================================================//
//                         BaseForm* Forms::GetInventoryForm()                         //
//=====================================================================================//
BaseForm* Forms::GetInventoryForm()
{ 
    if(!m_inventory_form.get()) m_inventory_form.reset(FormFactory::GetInst()->CreateInventoryForm());
    return m_inventory_form.get();
}

//=====================================================================================//
//                             void Forms::ShowGameForm()                              //
//=====================================================================================//
void Forms::ShowGameForm(bool focus_camera)
{
    m_current = GetGameForm();
    GetGameForm()->Show();

    DirtyLinks::EnableLevelRender(true);
    DirtyLinks::EnableCameraControl(true);
    DirtyLinks::EnableLevelAndEffectSound(true);

    GameObserver::menu_info info(GameObserver::MT_GAME);
    GameEvMessenger::GetInst()->Notify(GameObserver::EV_MENU, &info);
}

//=====================================================================================//
//                            void Forms::ShowJournalForm()                            //
//=====================================================================================//
void Forms::ShowJournalForm()
{
    m_current = GetJournalForm();
    GetJournalForm()->Show();

    DirtyLinks::EnableCameraControl(false);

    GameObserver::menu_info info(GameObserver::MT_JOURNAL);
    GameEvMessenger::GetInst()->Notify(GameObserver::EV_MENU, &info);
}

//=====================================================================================//
//                             void Forms::ShowShopForm()                              //
//=====================================================================================//
void Forms::ShowShopForm(HumanEntity* hero, TraderEntity* trader, AbstractGround* ground,
						 AbstractScroller* scroller, AbstractBin* bin)
{
    m_current = GetShopForm();

    GetShopForm()->Init(BaseForm::shop_ini_s(hero, trader, bin, ground, scroller));
    GetShopForm()->Show();

    DirtyLinks::EnableCameraControl(false);

    GameObserver::menu_info info(GameObserver::MT_TRADE);
    GameEvMessenger::GetInst()->Notify(GameObserver::EV_MENU, &info);
}

//=====================================================================================//
//                           void Forms::ShowInventoryForm()                           //
//=====================================================================================//
void Forms::ShowInventoryForm(HumanEntity* human, AbstractGround* ground,
							  AbstractScroller* scroller, AbstractBin* bin)
{
    m_current = GetInventoryForm();

    GetInventoryForm()->Init(BaseForm::inv_ini_s(human, scroller, ground, bin));
    GetInventoryForm()->Show();

    DirtyLinks::EnableCameraControl(false);

    GameObserver::menu_info info(GameObserver::MT_INVENTORY);
    GameEvMessenger::GetInst()->Notify(GameObserver::EV_MENU, &info);
}
  
//=====================================================================================//
//                            void Forms::ShowDescDialog()                             //
//=====================================================================================//
void Forms::ShowDescDialog(BaseThing* thing)
{
    m_dialog = DescDlg::GetInst();
    DescDlg::GetInst()->Init(thing);
}

//=====================================================================================//
//                            void Forms::ShowDescDialog()                             //
//=====================================================================================//
void Forms::ShowDescDialog(const std::string& text)
{
    m_dialog = DescDlg::GetInst();
    DescDlg::GetInst()->Init(text);
}

//=====================================================================================//
//                           void Forms::ShowOptionsDialog()                           //
//=====================================================================================//
void Forms::ShowOptionsDialog(options_mode mode)
{
    m_dialog = OptionsDlg::GetInst();
    OptionsDlg::GetInst()->Init(mode);
}

//=====================================================================================//
//                            void Forms::ShowTalkDialog()                             //
//=====================================================================================//
void Forms::ShowTalkDialog(BaseEntity* entity, const phrase_s& phrase)
{
    std::ostringstream ostr;
    ostr << entity->GetInfo()->GetName() << ": " << phrase.m_text;
    
    DialogScreen::DialogTraits traits;

    traits.m_Speech       = phrase.m_sound;    
    traits.m_Text         = ostr.str();
    traits.m_PortraitName = entity->GetInfo()->GetRID();
	traits.m_Volume       = 2.0f;

	const float vol = Muffle::ISound::instance()->getVolume(Muffle::ISound::cMaster);

	PoL::VolumeHelper::setMasterVolume(0.5f*vol);
	//Muffle::ISound::instance()->asetVolume(Muffle::ISound::cMaster,0.5f*vol);
    Cameraman::GetInst()->FocusEntity(entity);    
    Screens::Instance()->Dialog()->DoModal(traits);
	PoL::VolumeHelper::setMasterVolume(vol);
	//Muffle::ISound::instance()->asetVolume(Muffle::ISound::cMaster,vol);
}

//=====================================================================================//
//                             void Forms::ShowHeroInfo()                              //
//=====================================================================================//
void Forms::ShowHeroInfo(HumanEntity* hero)
{
    HeroInfoScreen::HeroTraits traits;

    traits.m_Text         = hero->GetInfo()->GetDesc();
    traits.m_PortraitName = hero->GetInfo()->GetRID();

    Screens::Instance()->HeroInfo()->DoModal(traits);
}

//=====================================================================================//
//                              void Forms::HandleInput()                              //
//=====================================================================================//
void Forms::HandleInput(state_type* st)
{
    if(m_dialog){

        if(st) *st = ST_INCOMPLETE;

        if(!m_dialog->IsActive()){
            m_dialog = 0;
            if(m_current) m_current->Show();
            return;
        }

        m_dialog->HandleInput();
        return;
    }

    if(m_current) m_current->HandleInput(st);
}

//=====================================================================================//
//                            bool Forms::ShowMessageBox()                             //
//=====================================================================================//
bool Forms::ShowMessageBox(const std::string& text)
{   
    return Interface::Instance()->QuestionBox(text.c_str()) != 0;    
}

//=====================================================================================//
//                              void Forms::ShowHMForm()                               //
//=====================================================================================//
void Forms::ShowHMForm(player_type player)
{
    m_current = GetHMForm();

    GetHMForm()->Show();
    GetHMForm()->Init(BaseForm::hm_ini_s(player));

    DirtyLinks::EnableCameraControl(false);

    GameObserver::menu_info info(GameObserver::MT_HIDDEN);
    GameEvMessenger::GetInst()->Notify(GameObserver::EV_MENU, &info);
}

//=====================================================================================//
//                            void Forms::UpdateHMProress()                            //
//=====================================================================================//
void Forms::UpdateHMProress(float val)
{
    //запросы подобного рода имеют смысл только при активном экране HM
    if(m_current == GetHMForm()) Screens::Instance()->HiddenMovement()->Update(val);
}

//=====================================================================================//
//                              bool Forms::IsHMVisible()                              //
//=====================================================================================//
bool Forms::IsHMVisible()
{
	return Screens::Instance()->HiddenMovement()->IsVisible();
}

//=====================================================================================//
//                           void Forms::ShowMainMenuForm()                            //
//=====================================================================================//
void Forms::ShowMainMenuForm()
{ 
    Screens::Instance()->ExitToMainMenu();
}

//=====================================================================================//
//                               void Forms::ShowBink()                                //
//=====================================================================================//
void Forms::ShowBink(const rid_t& rid)
{
    Interface::Instance()->PlayBink(rid.c_str());
}

//=====================================================================================//
//                             void Forms::ShowPanicForm()                             //
//=====================================================================================//
void Forms::ShowPanicForm(HumanEntity* panicer)
{
	std::string context;
    m_current = GetPanicForm();

    GetHMForm()->Enable(false);

	switch(panicer->GetEntityContext()->GetPanicType())
	{
	case HPT_USUAL:
		context = DirtyLinks::GetStrRes("aiu_usual_panic");
		break;
	case HPT_SHOCK:
		context = DirtyLinks::GetStrRes("aiu_shock_panic");
		break;
	case HPT_BERSERK:
		context = DirtyLinks::GetStrRes("aiu_berserk_panic");
		break;
	}

	Screens::Instance()->Panic()->SetContent(mlprintf(context.c_str(),panicer->GetInfo()->GetName().c_str()).c_str());
    GetPanicForm()->Show();
    
    DirtyLinks::EnableCameraControl(false);
}

//=====================================================================================//
//                             void Forms::MakeSaveLoad()                              //
//=====================================================================================//
void Forms::MakeSaveLoad(SavSlot& slot)
{
    GetHMForm()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_forms_hidden_movement"));
    GetShopForm()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_forms_shop"));

    if(!slot.IsSaving())
	{
        m_dialog = 0;
        ShowGameForm();
    }

    GetGameForm()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_forms_game"));
    GetPanicForm()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_forms_panic"));
    GetJournalForm()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_forms_journal"));
    GetInventoryForm()->MakeSaveLoad(SavSlot(slot.GetStore(), "logic_forms_inventory")); 
}

//=====================================================================================//
//                              void Forms::ShowHelper()                               //
//=====================================================================================//
void Forms::ShowHelper(helper_s& helper)
{
    HelperScreen::Traits traits;

    traits.m_Image = helper.m_image;
    traits.m_Text  = helper.m_message;

    traits.m_ShowThisMessage = helper.m_fenable_cur;
    traits.m_ShowHelper      = helper.m_fenable_all;  

    Screens::Instance()->Helper()->DoModal(traits);  

    helper.m_fenable_all = traits.m_ShowHelper;  
    helper.m_fenable_cur = traits.m_ShowThisMessage;
}



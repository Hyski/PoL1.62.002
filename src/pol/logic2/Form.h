//
// ����� ������� ���� 
//

#ifndef _PUNCH_FORM_H_
#define _PUNCH_FORM_H_

struct phrase_s;
class BaseThing;

class BaseEntity;
class HumanEntity;
class TraderEntity;

class HMForm;
class BaseDialog;
class ConsoleForm;
class GameFormImp;

//
// ���������� ����� ��� ���������� "���������" �����
//
class AbstractGround{
public:

    virtual ~AbstractGround(){}

    //����� �� �������� ������� � ���������
    virtual bool CanInsert(HumanEntity* actor, BaseThing* thing, std::string* reason) const = 0;

    //�������� ������� � ���������
    virtual void Insert(HumanEntity* actor, BaseThing* thing) = 0;
    //������� ������� �� ����������
    virtual void Remove(HumanEntity* actor, BaseThing* thing) = 0;

    enum sound_event{
        SE_DRAG,    
        SE_DROP,
    };

    //��������� ������������ �����
    virtual void PlaySound(sound_event sound) = 0;

    //
    // ����������� �������� �� ���������
    // 
    class Iterator{
    public:
        
        virtual ~Iterator(){}

        //������� �� �������� ������� 
        virtual void Next() = 0;
        //������� �� ������ �������
        virtual void First() = 0;

        //�������� ������ � ��������
        virtual BaseThing* Get() = 0;
        //������������ ��� �� ���������?
        virtual bool IsNotDone() const = 0;
    };

    //�������� ����������� ��������
    virtual Iterator* CreateIterator(HumanEntity* human) = 0;
};

//
// ���������� ��������� �� ��������
//
class AbstractScroller{
public:

    virtual ~AbstractScroller(){}

    //==========================================================
    
    //
    // ������� ��� �������� � �������� �����������
    // 

    //������� � ������ ���������
    virtual void Prev() = 0;
    //��� ����� ������������������?
    virtual bool IsREnd() const = 0;
    
    //���������� �������� �� ����� ������������������
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // ����:
    //      human == 0 �������� ��������. � ����� ����������.
    //      human != 0 �������� ��������. �� human c ����� ����
    //
    virtual void Last(HumanEntity* human = 0) = 0;

    //==========================================================

    //
    // ������� ��� �������� � ������ �����������
    // 
    
    //������� � ��������� ���������
    virtual void Next() = 0;
    //��� ������ ������������������?
    virtual bool IsEnd() const = 0;

    //���������� ������� � ������ ������������������
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // ����:
    //      human == 0 �������� ��������. � ������ ����������.
    //      human != 0 �������� ��������. �� human c ������ ����
    //
    virtual void First(HumanEntity* human = 0) = 0;

    //==========================================================

    //� ������������������ ������ ���� �������?
    bool CanScroll() const;

    //�������� ������ �� ������� ��-� ������������������
    virtual HumanEntity* Get() = 0;
};

//
// ���������� ������� - � ��� �������� �� ������ ��������
//
class AbstractBin{
public:

    virtual ~AbstractBin(){}

    //�������� ������� � �������
    virtual void Insert(HumanEntity* actor, BaseThing* thing) = 0;
};

//
// ���������� progress_bar
//
class AbstractProgressBar{
public:

    virtual ~AbstractProgressBar(){}

    //���������� ������ + ��������� �����
    virtual void SetPos(float value, const std::string& text = std::string()) = 0;
};

//
// ������� ����� ��� ���� ���� � ����
//
class BaseForm{
public:

    virtual ~BaseForm(){}

    struct ini_s {};

    //��� ������������� ��������
    struct shop_ini_s : public ini_s{
      HumanEntity*      m_human;
      TraderEntity*     m_trader;
      AbstractBin*      m_bin;
      AbstractGround*   m_ground;
      AbstractScroller* m_scroller;

      shop_ini_s(HumanEntity* human, TraderEntity* trader, AbstractBin* bin,
                 AbstractGround* ground, AbstractScroller* scroller) :
        m_human(human), m_ground(ground), m_trader(trader), m_bin(bin), m_scroller(scroller){}
    };

    //�������������� ���� ��������
    struct inv_ini_s : public ini_s{       
       HumanEntity*      m_human;
       AbstractBin*      m_bin;
       AbstractGround*   m_ground;
       AbstractScroller* m_scroller;

       inv_ini_s(HumanEntity* human, AbstractScroller* scroller,
                 AbstractGround* ground, AbstractBin* bin): 
         m_human(human), m_scroller(scroller), m_ground(ground), m_bin(bin){}
    };

    //������������� ���� ������� �����
    struct hm_ini_s : public ini_s{
        player_type m_player;
        hm_ini_s(player_type player) : m_player(player) {}
    };

    //������� ���� �������
    virtual void Show() = 0;
    //��������� �����
    virtual void HandleInput(state_type* st) = 0;
    //����������/��������
    virtual void MakeSaveLoad(SavSlot& slot) = 0;
    //������������� ����
    virtual void Init(const ini_s& ptr = ini_s()) = 0;
};

//
// ���� �������
//
class JournalForm : public BaseForm{
public:

    //���� �� ����� ���������� � ����
    virtual bool IsUpdated() const = 0;
};

//
// ������� ����
//
class FormFactory
{
public:

    static FormFactory* GetInst();

    //������� ���� ��������
    BaseForm* CreateShopForm();
    //������� ���� ������
    BaseForm* CreatePanicForm();
    //������� ���� �������� ���������
    BaseForm* CreateInventoryForm();

    //������� ������� ���� 
    GameFormImp* CreateGameForm();
    //������� ���� �������
    JournalForm* CreateJournalForm();

    //������� progress bar
    AbstractProgressBar* CreateLoadBar(float from, float to);

private:

    FormFactory(){}
};

//
// ������� ������� ���� 
//
class Forms
{
public:
    ~Forms();

    //singleton
    static Forms* GetInst();

    enum reset_type {
        RT_SHUT,
        RT_NEXT_LEVEL
    };
   
    //����������������� ����
    void Reset(reset_type type);
    //���������/������������ ����. ����
    void MakeSaveLoad(SavSlot& st);

	bool HasGameForm() const;

    //�������� ������ �� ����� ����
    HMForm* GetHMForm();
    BaseForm* GetShopForm();
    GameFormImp* GetGameForm();
    BaseForm* GetPanicForm();
    BaseForm* GetInventoryForm();
    JournalForm* GetJournalForm();

    //���������� ������������ ���� � ������� ����
    void HandleInput(state_type* st);

    //������� � ������
    void ShowJournalForm();
    //������� � �������� ������� ����
    void ShowMainMenuForm();
    //�������� ����� ������
    void ShowPanicForm(HumanEntity* panicer);
    //�������� ����� ������� �����������
    void ShowHMForm(player_type player);
    //���������� ����������� � ���������
    void ShowHeroInfo(HumanEntity* hero);
    //������� � ������� ����
    void ShowGameForm(bool focus_camera = false);
    //������� � ���� ��������
    void ShowInventoryForm(HumanEntity* human, AbstractGround* ground, AbstractScroller* scroller, AbstractBin* bin);
    //������� � �������
    void ShowShopForm(HumanEntity* hero, TraderEntity* trader, AbstractGround* ground, AbstractScroller* scroller, AbstractBin* bin);

    enum options_mode{
        ODM_GAMEMENU,    //����������� ���� �����
        ODM_LOADMENU,    //����� ��������
    };
    
    //������� � ������ ������� �����
    void ShowOptionsDialog(options_mode mode);

    //������� � ������ �������� ��������
    void ShowDescDialog(BaseThing* thing);
    //������� � ������ ��������
    void ShowDescDialog(const std::string& desc);

    //�������� ������ ���������
    void ShowTalkDialog(BaseEntity* entity, const phrase_s& phrase);

    struct helper_s{
        std::string m_image;
        std::string m_message;

        bool m_fenable_cur;
        bool m_fenable_all;

        helper_s(std::string im, std::string msg, bool fcur, bool fall) : 
            m_image(im), m_message(msg), m_fenable_cur(fcur), m_fenable_all(fall){}
    };

    //�������� ������ ���������
    void ShowHelper(helper_s& helper);

    //�������� bink
    void ShowBink(const rid_t& rid);

    //�������� message box
    bool ShowMessageBox(const std::string& text);
    //���������� progress bar � ������ hidden movement
    void UpdateHMProress(float val);
	bool IsHMVisible();
   
protected:

    Forms();

private:

    BaseForm*    m_current;
    BaseDialog*  m_dialog;

	std::auto_ptr<HMForm> m_hm_form;
    std::auto_ptr<BaseForm> m_shop_form;
    std::auto_ptr<GameFormImp> m_game_form;
    std::auto_ptr<BaseForm> m_panic_form;
    std::auto_ptr<BaseForm> m_inventory_form; 

	std::auto_ptr<ConsoleForm> m_console;
    std::auto_ptr<JournalForm> m_journal_form;
};

#endif // _PUNCH_FORM_H_
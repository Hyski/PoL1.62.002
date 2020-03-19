//
// ���������� ���� ��������
// 
#include "logicdefs.h"

#include <undercover/interface/screens.h>

#include "thing.h"
#include "entity.h"
#include "aiutils.h"
#include "RankTable.h"
#include "xlsreader.h"
#include "dirtylinks.h"
#include "dragdropmanager.h"
#include "InventoryFormImp3.h"
#include <common/D3DApp/Input/Input.h>
#include <common/Utils/VFileWpr.h>

//=======================================================================

BaseForm* FormFactory::CreateInventoryForm()
{
    return new InventoryFormImp3();
}

//=======================================================================

namespace{
    
    const char* traits_desc_xls = "scripts/logic/traits_desc.txt";

    //�������� ������ � ���-��� ����� �� ��������� ������
    std::string MakeNextLevelStr(int points)
    {
        if(points < 0) return DirtyLinks::GetStrRes("if_max_next_level");

        std::ostringstream ostr;
        ostr << points;
        return ostr.str();
    }
}

//=======================================================================

namespace{

    //
    // ����� ��� ��������� �������� ���-� ���������
    //
    class TraitDescReader{
    public:

        TraitDescReader() : m_last_trait(InventoryScreen::MAX_LEVEL_UP)
        {
             TxtFilePtr txt(traits_desc_xls);
             txt.ReadColumns(m_columns, m_columns + MAX_COLUMNS);
        }

        const std::string& Read(InventoryScreen::LEVEL_UP trait)
        {
            //���� ��� ������
            if(m_last_trait == trait) return m_last_desc;

            TxtFilePtr txt(traits_desc_xls);

            std::string str, trait_str = Trait2Str(trait);

            //������ ������ ������
            for(int line = 0; line < txt->SizeY(); line++)
			{
                txt->GetCell(line, m_columns[CT_SYS_NAME].m_index, str);

                if(trait_str == str)
				{
					m_last_desc = PoL::getLocStr("traits_desc." + trait_str + ".desc");
                    m_last_trait = trait;
                    //txt->GetCell(line, m_columns[CT_DESC].m_index, m_last_desc);
                    return m_last_desc;
                }
            }

            std::ostringstream ostr;

            ostr << "TraitDescReader: ��� �������� ��������!" << std::endl;
            ostr << "��������:\t" << trait_str << std::endl;
            ostr << "xls:\t" << traits_desc_xls;

            m_last_desc = ostr.str();
            m_last_trait = InventoryScreen::MAX_LEVEL_UP;
            
            return m_last_desc;
        }

    private:

        std::string Trait2Str(InventoryScreen::LEVEL_UP trait) const
        {
            switch(trait){
            case InventoryScreen::LU_SIGHT: return "sight";
            case InventoryScreen::LU_WISDOM: return "intelligence";
            case InventoryScreen::LU_HEALTH: return "health";
            case InventoryScreen::LU_REACTION: return "reaction";
            case InventoryScreen::LU_ACCURACY: return "accuracy";
            case InventoryScreen::LU_STRENGTH: return "strength";
            case InventoryScreen::LU_DEXTERITY: return "dexterity";
            case InventoryScreen::LU_MECHANICS: return "mechanics";
            }

            return "unknown";
        }

    private:

        enum column_type{
            CT_SYS_NAME,
            //CT_DESC,

            MAX_COLUMNS,
        };

        std::string               m_last_desc;
        InventoryScreen::LEVEL_UP m_last_trait;
        
        static column m_columns[MAX_COLUMNS];          
    };

    column TraitDescReader::m_columns[] = 
    {
        column("sys_name", CT_SYS_NAME),
        //column("desc",     CT_DESC),
    };
}

//=======================================================================

namespace{

    class StandartLeveluper : public  LevelupManager,
                              private EntityObserver
    {
    public:

        StandartLeveluper() : m_hero(0) { memset(m_addition, 0, sizeof(m_addition)); }

        ~StandartLeveluper(){ if(m_hero) m_hero->Detach(this); }

        void SetActor(HumanEntity* human)
        {
            if(m_hero)
			{
                m_hero->Detach(this);
                ApplyLevelups();
            }

            //�������� ��� levelup'�
            memset(m_addition, 0, sizeof(m_addition));

            if(m_hero = human)
			{
                Update(0,0,0);

                m_hero->Attach(this, EV_FOS_CHANGE);
                m_hero->Attach(this, EV_WEIGHT_CHANGE);
                m_hero->Attach(this, EV_WISDOM_CHANGE);
                m_hero->Attach(this, EV_MORALE_CHANGE);
                m_hero->Attach(this, EV_HEALTH_CHANGE);
                m_hero->Attach(this, EV_MAXHEALTH_CHANGE);
                m_hero->Attach(this, EV_LEVELUP_CHANGE);
                m_hero->Attach(this, EV_ACCURACY_CHANGE);
                m_hero->Attach(this, EV_REACTION_CHANGE);
                m_hero->Attach(this, EV_STRENGTH_CHANGE);
                m_hero->Attach(this, EV_DEXTERITY_CHANGE);
                m_hero->Attach(this, EV_MECHANICS_CHANGE);
                m_hero->Attach(this, EV_EXPERIENCE_CHANGE);
                m_hero->Attach(this, EV_MAXMOVEPNTS_CHANGE);
            }
        }

        //������ ���-�� ����� levelup'a
        int GetPoints() const
        {
            return m_hero->GetEntityContext()->GetTraits()->GetLevelupPoints() - GetSum();
        }

        //������ �������� ���������
        int GetTrait(InventoryScreen::LEVEL_UP trait) const
        {
            HumanContext::Traits* hum = m_hero->GetEntityContext()->GetTraits();

            switch(trait){
            case InventoryScreen::LU_STRENGTH:
                 return hum->GetStrength() + m_addition[trait];

            case InventoryScreen::LU_DEXTERITY:
                 return hum->GetDexterity() + m_addition[trait];

            case InventoryScreen::LU_REACTION:
                 return hum->GetReaction() + m_addition[trait];

            case InventoryScreen::LU_ACCURACY:
                 return hum->GetAccuracy() + m_addition[trait];

            case InventoryScreen::LU_WISDOM:
                 return hum->GetWisdom() + m_addition[trait];

            case InventoryScreen::LU_MECHANICS:
                 return hum->GetMechanics() + m_addition[trait];

            case InventoryScreen::LU_SIGHT:
                 return hum->GetFrontRadius() + m_addition[trait];

            case InventoryScreen::LU_HEALTH:
                 return hum->GetHealth() + m_addition[trait];
            }

            return 0;
        }

        int GetLimit(InventoryScreen::LEVEL_UP trait) const
        {
            HumanContext::Limits* hum = m_hero->GetEntityContext()->GetLimits();

            switch(trait){
            case InventoryScreen::LU_STRENGTH:
                 return hum->GetStrength() + m_addition[trait];

            case InventoryScreen::LU_DEXTERITY:
                 return hum->GetDexterity() + m_addition[trait];

            case InventoryScreen::LU_REACTION:
                 return hum->GetReaction() + m_addition[trait];

            case InventoryScreen::LU_ACCURACY:
                 return hum->GetAccuracy() + m_addition[trait];

            case InventoryScreen::LU_WISDOM:
                 return hum->GetWisdom() + m_addition[trait];

            case InventoryScreen::LU_MECHANICS:
                 return hum->GetMechanics() + m_addition[trait];

            case InventoryScreen::LU_SIGHT:
                 return hum->GetFrontRadius() + m_addition[trait];

            case InventoryScreen::LU_HEALTH:
                 return hum->GetHealth() + m_addition[trait];
            }

            return 0;
        }
        
        //���������� ����������/���������� ����������
        void Inc(InventoryScreen::LEVEL_UP trait)
        {
            m_addition[trait]++;
            m_hero->Notify(EntityObserver::EV_LEVELUP_CHANGE);
        }

        void Dec(InventoryScreen::LEVEL_UP trait)
        {
            m_addition[trait]--;
            m_hero->Notify(EntityObserver::EV_LEVELUP_CHANGE);
        }

    private:
        void Update(subject_t subj, event_t event, info_t info)
        {
            InventoryScreen* screen = Screens::Instance()->Inventory();

            bool fhave_points = GetPoints() > 0;

            //��������� ������
            for(int k = 0; k < InventoryScreen::MAX_LEVEL_UP; k++)
			{                                
                InventoryScreen::LEVEL_UP levelup = static_cast<InventoryScreen::LEVEL_UP>(k);
                screen->ShowLevelUp(levelup, InventoryScreen::LUS_MINUS, m_addition[levelup] != 0);
                screen->ShowLevelUp(levelup, InventoryScreen::LUS_PLUS, fhave_points && CanInc(levelup));
            }
        }

        void ApplyLevelups()
        {
            HumanContext::Traits* traits = m_hero->GetEntityContext()->GetTraits();
            HumanContext::Limits* limits = m_hero->GetEntityContext()->GetLimits();
			const bool needChangeHealth = limits->GetHealth() == traits->GetHealth();

            limits->SetHealth(GetLimit(InventoryScreen::LU_HEALTH));
            limits->SetWisdom(GetLimit(InventoryScreen::LU_WISDOM));
            limits->SetReaction(GetLimit(InventoryScreen::LU_REACTION));
            limits->SetAccuracy(GetLimit(InventoryScreen::LU_ACCURACY));
            limits->SetStrength(GetLimit(InventoryScreen::LU_STRENGTH));
            limits->SetFrontRadius(GetLimit(InventoryScreen::LU_SIGHT));
            limits->SetDexterity(GetLimit(InventoryScreen::LU_DEXTERITY));
            limits->SetMechanics(GetLimit(InventoryScreen::LU_MECHANICS));

            if(needChangeHealth) traits->AddHealth(m_addition[InventoryScreen::LU_HEALTH]);
            traits->AddFrontRadius(m_addition[InventoryScreen::LU_SIGHT]);
            traits->AddReaction(m_addition[InventoryScreen::LU_REACTION]);
            traits->AddAccuracy(m_addition[InventoryScreen::LU_ACCURACY]);
            traits->AddStrength(m_addition[InventoryScreen::LU_STRENGTH]);
            traits->AddDexterity(m_addition[InventoryScreen::LU_DEXTERITY]);
            traits->AddMechanics(m_addition[InventoryScreen::LU_MECHANICS]);
            traits->AddWisdom(m_addition[InventoryScreen::LU_WISDOM]);

            traits->AddLevelupPoints(- GetSum());
        }

        int GetSum() const
        {
            int sum = 0;
            for(int k = 0; k < InventoryScreen::MAX_LEVEL_UP; sum += m_addition[k++]); 
            return sum;
        }

        bool IsSightValid() const
        {
            return GetLimit(InventoryScreen::LU_SIGHT) < MAX_FRONT_RADIUS;;
        }

        bool IsDexterityValid() const
        {
			return GetLimit(InventoryScreen::LU_DEXTERITY) < PoL::Inv::DexterityMax;
        }
        
        bool IsStrengthAndDexterityValid() const
        {
            return      GetLimit(InventoryScreen::LU_STRENGTH)
                    +   GetLimit(InventoryScreen::LU_DEXTERITY)
					<   PoL::Inv::StrengthPlusDexterityMax;
        }
        
        bool IsStrengthAndWisdomValid() const
        {
            return      GetLimit(InventoryScreen::LU_WISDOM)
                    +   GetLimit(InventoryScreen::LU_STRENGTH)
					<   PoL::Inv::StrengthPlusWisdomMax;
        }
        
        bool IsStrengthAndAccuracyValid() const
        {
            return      GetLimit(InventoryScreen::LU_STRENGTH)
                    +   GetLimit(InventoryScreen::LU_ACCURACY)
					<   PoL::Inv::StrengthPlusAccuracyMax;
        }
        
        bool IsAccuracyAndMechanicsValid() const
        {
            return      GetLimit(InventoryScreen::LU_ACCURACY)
                    +   GetLimit(InventoryScreen::LU_MECHANICS)
					<   PoL::Inv::AccuracyPlusMechanicsMax;
        }

        bool CanInc(InventoryScreen::LEVEL_UP trait) const
        {
            switch(trait){
            case InventoryScreen::LU_STRENGTH:
                return  IsStrengthAndWisdomValid()
                    &&  IsStrengthAndAccuracyValid()
                    &&  IsStrengthAndDexterityValid();

            case InventoryScreen::LU_DEXTERITY:
                return   IsDexterityValid()
                    &&   IsStrengthAndDexterityValid();

            case InventoryScreen::LU_ACCURACY:
                return  IsStrengthAndAccuracyValid()
                    &&  IsAccuracyAndMechanicsValid();

            case InventoryScreen::LU_WISDOM:
                return IsStrengthAndWisdomValid();

            case InventoryScreen::LU_MECHANICS:
                return IsAccuracyAndMechanicsValid();

            case InventoryScreen::LU_SIGHT:
                return IsSightValid();

            case InventoryScreen::LU_HEALTH:
            case InventoryScreen::LU_REACTION:
                return true;
            }

            return false;
        }

    private:

        HumanEntity* m_hero;

        int m_addition[InventoryScreen::MAX_LEVEL_UP];
    };
}

//=======================================================================

InventoryFormImp3::InventoryFormImp3() :
    m_hero(0),m_leveluper(new StandartLeveluper()),
    m_drag_drop(new DragDropManager(Screens::Instance()->Inventory()->GetSlotsEngine()))
{
    Screens::Instance()->Inventory()->SetController(this);

    //������� ����� � �������� Drag&Drop'a
    m_drag_drop->Insert(InventoryScreen::S_BODY, SlotFactory::Create(SlotStrategy::ST_BODY));
	m_drag_drop->Insert(InventoryScreen::S_HEAD, SlotFactory::Create(SlotStrategy::ST_HEAD));
    m_drag_drop->Insert(InventoryScreen::S_GROUND, SlotFactory::Create(SlotStrategy::ST_GROUND));
	m_drag_drop->Insert(InventoryScreen::S_IMPLANT, SlotFactory::Create(SlotStrategy::ST_IMPLANTS)); 
	m_drag_drop->Insert(InventoryScreen::S_BACKPACK, SlotFactory::Create(SlotStrategy::ST_BACKPACK));
	m_drag_drop->Insert(InventoryScreen::S_KNEE_LEFT, SlotFactory::Create(SlotStrategy::ST_LKNEE));
	m_drag_drop->Insert(InventoryScreen::S_KNEE_RIGHT, SlotFactory::Create(SlotStrategy::ST_RKNEE));

    m_unloader = new UnloadingDecorator(SlotFactory::Create(SlotStrategy::ST_HANDS));

    m_drag_drop->Insert(InventoryScreen::S_HANDS, m_unloader);
}

InventoryFormImp3::~InventoryFormImp3()
{
    delete m_drag_drop;
    delete m_leveluper;

    //Screens::Instance()->Inventory()->SetController(0);
}

void InventoryFormImp3::Show()
{
    Screens::Instance()->Activate(Screens::SID_INVENTORY);
}

void InventoryFormImp3::HandleInput(state_type* state)
{
    if(Input::KeyBack(DIK_ESCAPE)) HandleExitFormReq();
    if(Input::KeyBack(DIK_I)) HandleExitFormReq();
}

//=====================================================================================//
//                        void InventoryFormImp3::GiveItemTo()                         //
//=====================================================================================//
void InventoryFormImp3::GiveItemTo(HumanEntity *e, SEItem *t)
{
	m_drag_drop->DropItem(e);
}

//=====================================================================================//
//                           void InventoryFormImp3::Init()                            //
//=====================================================================================//
void InventoryFormImp3::Init(const ini_s& ini)
{
    const inv_ini_s& inv_ini = static_cast<const inv_ini_s&>(ini);

    m_ground = inv_ini.m_ground;
    m_scroller = inv_ini.m_scroller;

    m_scroller->First(inv_ini.m_human);

    m_drag_drop->SetBin(inv_ini.m_bin);
    m_drag_drop->SetGround(inv_ini.m_ground);

    HandleSelectHeroReq(m_scroller->Get());
}

//=====================================================================================//
//                     void InventoryFormImp3::HandleExitFormReq()                     //
//=====================================================================================//
void InventoryFormImp3::HandleExitFormReq()
{
    if(m_drag_drop->CancelDrop())
        return;

    HandleSelectHeroReq(0);
    Forms::GetInst()->ShowGameForm();    
}

//=====================================================================================//
//                     void InventoryFormImp3::HandleNextHeroReq()                     //
//=====================================================================================//
void InventoryFormImp3::HandleNextHeroReq()
{
    m_scroller->Next();

    if(m_scroller->IsEnd()) m_scroller->First();
    
    HandleSelectHeroReq(m_scroller->Get());
}

//=====================================================================================//
//                     void InventoryFormImp3::HandlePrevHeroReq()                     //
//=====================================================================================//
void InventoryFormImp3::HandlePrevHeroReq()
{
    m_scroller->Prev();

    if(m_scroller->IsREnd()) m_scroller->Last();
    
    HandleSelectHeroReq(m_scroller->Get());
}

//=====================================================================================//
//                       void InventoryFormImp3::OnButtonClick()                       //
//=====================================================================================//
void InventoryFormImp3::OnButtonClick(InventoryScreen::BUTTON button)
{
    switch(button){
    case InventoryScreen::B_INFO:
        Forms::GetInst()->ShowHeroInfo(m_hero);
        break;

    case InventoryScreen::B_UNLOAD:
        m_unloader->Unload();
        break;
        
    case InventoryScreen::B_PREV_HERO:
        HandlePrevHeroReq();
        break;
        
    case InventoryScreen::B_NEXT_HERO:
        HandleNextHeroReq();
        break;
        
    case InventoryScreen::B_BACK:
        HandleExitFormReq();
        break;
        
    case InventoryScreen::B_DISMISS:
        HandleDismissHero();
        break;
    }
}

//=====================================================================================//
//                     void InventoryFormImp3::HandleDismissHero()                     //
//=====================================================================================//
void InventoryFormImp3::HandleDismissHero()
{
    std::string message = mlprintf( DirtyLinks::GetStrRes("if_can_dismiss").c_str(),
                                    m_hero->GetInfo()->GetName().c_str());

    if(m_scroller->CanScroll() && Forms::GetInst()->ShowMessageBox(message)){

        //������� ������� � ����� 
        m_drag_drop->CancelDrop();

        BaseEntity* hero4dismiss = m_hero;

        //�������� � ���������  �����
        HandleNextHeroReq();
        
        //������ �����������
        hero4dismiss->GetEntityContext()->SetAIModel(DirtyLinks::GetStrRes("igo_dismissed_hero_ai"));
        AIUtils::ChangeEntityPlayer(hero4dismiss, PT_CIVILIAN);

        //�������� ������� ��� ������������ ������
        hero4dismiss->Notify(EntityObserver::EV_TEAMMATE_DISMISS);

        EntityPool::iterator itor = EntityPool::GetInst()->begin(ET_HUMAN, PT_PLAYER);
        while(itor != EntityPool::GetInst()->end()){
            itor->Notify(EntityObserver::EV_TEAMMATE_LEAVE);
            ++itor;
        }

        EnableDismissAndScrollHero();
    }    
}

//=====================================================================================//
//                void InventoryFormImp3::EnableDismissAndScrollHero()                 //
//=====================================================================================//
void InventoryFormImp3::EnableDismissAndScrollHero()
{
    InventoryScreen* screen = Screens::Instance()->Inventory();

    bool fcan_scroll = m_scroller->CanScroll();
    
    screen->EnableButton(InventoryScreen::B_PREV_HERO, fcan_scroll);
    screen->EnableButton(InventoryScreen::B_NEXT_HERO, fcan_scroll);
    
    screen->EnableButton(InventoryScreen::B_DISMISS, fcan_scroll && m_hero->GetInfo()->CanDismiss());// && !m_hero->GetInfo()->IsHacker());
}

//=====================================================================================//
//                      void InventoryFormImp3::OnLevelUpClick()                       //
//=====================================================================================//
void InventoryFormImp3::OnLevelUpClick(InventoryScreen::LEVEL_UP id,InventoryScreen::LEVEL_UP_SIGN sign)
{   
    if(m_hero == 0) return;

    switch(sign){
    case InventoryScreen::LUS_PLUS:
        m_leveluper->Inc(id);
        break;

    case InventoryScreen::LUS_MINUS:
        m_leveluper->Dec(id);
        break;
    }
}

//=====================================================================================//
//                   void InventoryFormImp3::OnLevelUpDescription()                    //
//=====================================================================================//
void InventoryFormImp3::OnLevelUpDescription(InventoryScreen::LEVEL_UP id)
{
    static TraitDescReader reader;
    Forms::GetInst()->ShowDescDialog(reader.Read(id));
}

//=====================================================================================//
//                    void InventoryFormImp3::HandleSelectHeroReq()                    //
//=====================================================================================//
void InventoryFormImp3::HandleSelectHeroReq(HumanEntity* hero)
{
    if(m_hero) m_hero->Detach(this);

    m_drag_drop->SetActor(m_hero = hero);
    m_leveluper->SetActor(m_hero);

    if(m_hero == 0) return;

    EnableDismissAndScrollHero();

	m_hero->Attach(this, EV_RANK_UP);
    m_hero->Attach(this, EV_TAKE_MONEY);
    m_hero->Attach(this, EV_FOS_CHANGE);
    m_hero->Attach(this, EV_WEIGHT_CHANGE);
    m_hero->Attach(this, EV_WISDOM_CHANGE);
    m_hero->Attach(this, EV_MORALE_CHANGE);
    m_hero->Attach(this, EV_HEALTH_CHANGE);
    m_hero->Attach(this, EV_MAXHEALTH_CHANGE);
    m_hero->Attach(this, EV_LEVELUP_CHANGE);
    m_hero->Attach(this, EV_MOVEPNTS_CHANGE);
    m_hero->Attach(this, EV_MAXMOVEPNTS_CHANGE);
    m_hero->Attach(this, EV_ACCURACY_CHANGE);
    m_hero->Attach(this, EV_REACTION_CHANGE);
    m_hero->Attach(this, EV_STRENGTH_CHANGE);
    m_hero->Attach(this, EV_DEXTERITY_CHANGE);
    m_hero->Attach(this, EV_MECHANICS_CHANGE);
    m_hero->Attach(this, EV_EXPERIENCE_CHANGE);
    m_hero->Attach(this, EV_WEAPON_STATE_CHANGE);

    InventoryScreen* screen = Screens::Instance()->Inventory();
   
    InventoryScreen::PortraitTraits traits;
    
    //��������� ������� ���������
    traits.m_hero_name     = m_hero->GetInfo()->GetName().c_str();
    traits.m_portrait_name = m_hero->GetInfo()->GetRID().c_str();
    
    screen->SetPortraitTraits(traits);

    //���������� ��������� ��������
    Update(m_hero, 0, 0);
}

//=====================================================================================//
//                          void InventoryFormImp3::Update()                           //
//=====================================================================================//
void InventoryFormImp3::Update(BaseEntity* entity, event_t event, info_t info)
{
    InventoryScreen::HeroTraits hero_traits;

    HumanContext* context = m_hero->GetEntityContext();

	hero_traits.m_Rank			= PoL::RankTable::instance()->getLocName(context->GetTraits()->GetRank());

	hero_traits.m_ElectricRes   = std::min(context->GetTraits()->GetElectricRes(),100);
    hero_traits.m_Experience    = context->GetTraits()->GetExperience();
	hero_traits.m_FireRes       = std::min(context->GetTraits()->GetFlameRes(),100);
    hero_traits.m_Level         = context->GetTraits()->GetLevel() + 1;

    hero_traits.m_LevelUpPoints = m_leveluper->GetPoints();

    hero_traits.m_Money         = MoneyMgr::GetInst()->GetMoney();
    hero_traits.m_MoralCurrent  = context->GetTraits()->GetMorale();
    hero_traits.m_MoralMax      = context->GetLimits()->GetMorale();
    
    hero_traits.m_NextLevel     = MakeNextLevelStr(LevelupMgr::GetInst()->GetLevelPoints(context->GetTraits()->GetLevel() + 1)/* - context->GetTraits()->GetExperience()*/);

	hero_traits.m_ShockRes      = std::min(context->GetTraits()->GetShockRes(),100);
    hero_traits.m_WeightCurrent = context->GetTraits()->GetWeight();
    hero_traits.m_WeightMax     = context->GetLimits()->GetWeight();

	// ����
    hero_traits.m_Strength      = m_leveluper->GetTrait(InventoryScreen::LU_STRENGTH);
	hero_traits.m_MaxStrength   = m_leveluper->GetLimit(InventoryScreen::LU_STRENGTH) + context->GetTraits()->GetConstStrength();

	// ��������
    hero_traits.m_Dexterity     = m_leveluper->GetTrait(InventoryScreen::LU_DEXTERITY);
	hero_traits.m_MaxDexterity  = m_leveluper->GetLimit(InventoryScreen::LU_DEXTERITY);

	// �������
    hero_traits.m_Reaction      = m_leveluper->GetTrait(InventoryScreen::LU_REACTION);
	hero_traits.m_MaxReaction	= m_leveluper->GetLimit(InventoryScreen::LU_REACTION) + context->GetTraits()->GetConstReaction();

	// ��������
    hero_traits.m_Accuracy      = m_leveluper->GetTrait(InventoryScreen::LU_ACCURACY);
	hero_traits.m_MaxAccuracy	= m_leveluper->GetLimit(InventoryScreen::LU_ACCURACY) + context->GetTraits()->GetConstAccuracy();

	// ���������
    hero_traits.m_Wisdom        = m_leveluper->GetTrait(InventoryScreen::LU_WISDOM);
	hero_traits.m_MaxWisdom		= m_leveluper->GetLimit(InventoryScreen::LU_WISDOM) + context->GetTraits()->GetConstWisdom();

	// ��������
    hero_traits.m_Mechanics     = m_leveluper->GetTrait(InventoryScreen::LU_MECHANICS);
	hero_traits.m_MaxMechanics  = m_leveluper->GetLimit(InventoryScreen::LU_MECHANICS);

	// ������
    hero_traits.m_Sight         = m_leveluper->GetTrait(InventoryScreen::LU_SIGHT);
	hero_traits.m_MaxSight		= m_leveluper->GetLimit(InventoryScreen::LU_SIGHT) + context->GetTraits()->GetConstFrontRadius();

	// ���� ����
    hero_traits.m_MoveCurrent   = context->GetTraits()->GetMovepnts();
	hero_traits.m_MoveMax       = PoL::Inv::MpsForTurnStrengthFactor*hero_traits.m_MaxStrength
								- PoL::Inv::MpsForTurnWeightFactor*context->GetTraits()->GetWeight();

	if(context->GetTraits()->GetHealth() == context->GetLimits()->GetHealth())
	{
	    hero_traits.m_HealthMax     = m_leveluper->GetLimit(InventoryScreen::LU_HEALTH);
	}
	else
	{
	    hero_traits.m_HealthMax     = context->GetTraits()->GetHealth();
	}

	hero_traits.m_MaxHealth		= m_leveluper->GetLimit(InventoryScreen::LU_HEALTH);
    InventoryScreen* screen = Screens::Instance()->Inventory();
    
    screen->SetHeroTraits(hero_traits); 

    HumanContext::iterator itor = context->begin(HPK_HANDS, TT_WEAPON);
    screen->EnableButton(InventoryScreen::B_UNLOAD, itor != context->end() && static_cast<WeaponThing*>(&*itor)->GetAmmo());
}

//=====================================================================================//
//                     void InventoryFormImp3::UpdateAllContent()                      //
//=====================================================================================//
void InventoryFormImp3::UpdateAllContent(HumanEntity *e)
{
	m_scroller->First(e);
	HandleSelectHeroReq(e);
}

//=====================================================================================//
//                       void InventoryFormImp3::MakeSaveLoad()                        //
//=====================================================================================//
void InventoryFormImp3::MakeSaveLoad(SavSlot& slot)
{
}

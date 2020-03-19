#include "logicdefs.h"

#include <undercover/interface/screens.h>

#include "thing.h"
#include "entity.h"
#include "aiutils.h"
#include "sndutils.h"
#include "ranktable.h"
#include "dirtylinks.h"
#include "shopformimp.h"
#include "dragdropmanager.h"
#include <common/D3DApp/Input/Input.h>

//===============================================================


BaseForm* FormFactory::CreateShopForm()
{
    return new ShopFormImp();
}

//===============================================================

namespace{

    //�������� ������ �� ����������
    ShopFormImp* GetShopFormImp()
    {
        return static_cast<ShopFormImp*>(Forms::GetInst()->GetShopForm());
    }
    
    //��������� �������� ��� ��������
    std::string GetTraderDesc(TraderEntity* trader)
    {
        std::ostringstream ostr;
        
        ostr << trader->GetInfo()->GetName() << ':' << std::endl;
        ostr << trader->GetInfo()->GetDesc() << std::endl;
        
        return ostr.str();
    }

    //������� �������������� �������� � ������
    std::string Int2Str(int value)
    {
        std::ostringstream ostr;
        ostr << value;
        return ostr.str();
    }
}

//===============================================================

namespace{

static bool IsEnoughRank(BaseThing *thing)
{
	HumanEntity *mainHero = 0;

	EntityPool::iterator i = EntityPool::GetInst()->begin(ET_HUMAN,PT_PLAYER);
	for(; i != EntityPool::GetInst()->end(); ++i)
	{
		if(i->Cast2Human() && i->GetInfo()->GetRID() == "player")
		{
			mainHero = i->Cast2Human();
			break;
		}
	}

	int rank = thing->GetInfo()->GetRank();
	return rank <= (mainHero ? mainHero->GetEntityContext()->GetRank() : 0xFFFF);
}

    //
    // ���������� ������� ������
    //
    class UsualSheet : public CommoditySheet
	{
    public:
        UsualSheet(TradeScreen::SHEET sheet, unsigned mask) : 
            m_mask(mask), m_sheet(sheet) {}

        void Show(HumanEntity* hero, BaseThing* active)
        {
            TradeScreen::CommodityTraits traits;
            TraderEntity* trader = GetShopFormImp()->GetTrader();

            std::string tmp;

            if(m_commodity)
			{
                //traits.m_Cost = trader->GetInfo()->GetSellPrice(m_commodity);
                traits.m_Name = m_commodity->GetInfo()->GetName();
                traits.m_Quantity = trader->GetEntityContext()->GetCount(m_commodity);
                traits.m_SystemName = m_commodity->GetInfo()->GetShader();

                tmp = AIUtils::GetThingNumber(m_commodity);
                traits.m_Text = tmp;

                traits.m_State = 0;

                //������� ����� ��������������� ���������?
                if(!ThingDelivery::IsSuitable(hero, m_commodity))
				{
                    traits.m_State |= TradeScreen::CS_GRAYED;
				}

				if(!IsEnoughRank(m_commodity))
				{
					traits.m_State |= TradeScreen::CS_NORANK;
				}
                
                //������� ����� ������������ ������� ��������� � ������?
                if(active && ThingDelivery::IsSuitable(m_commodity, active))
				{
                    traits.m_State |= TradeScreen::CS_HIGHLIGHTED;
				}

				traits.m_Cost = boost::lexical_cast<std::string>(trader->GetInfo()->GetSellPrice(m_commodity));
				traits.m_BuyRank = PoL::RankTable::instance()->getLocName(m_commodity->GetMinRank());
            }
			else
			{
                traits.m_State = 0;
                traits.m_Quantity = 0;
                traits.m_Name.clear();
                traits.m_Text.clear();
                traits.m_SystemName.clear();
				traits.m_Cost.clear();
				traits.m_BuyRank.clear();
            }

            Screens::Instance()->Trade()->SetCommodityTraits(traits);
        }

        void Init()
        {
            m_commodity = 0;
            Next();
        }
        
        void Next()
        {
            TraderContext* context = GetShopFormImp()->GetTrader()->GetEntityContext();

            if(m_commodity){
    
                TraderContext::iterator itor = GetUpperBound(context->begin(m_mask), context->end(), m_commodity);

                if(itor != context->end()){
                    m_commodity = &*itor;
                    return;
                }
            }
            
            TraderContext::iterator itor = context->begin(m_mask);
            m_commodity = itor != context->end() ? &*itor : 0;

            Screens::Instance()->Trade()->EnableSheet(m_sheet, m_commodity != 0);
        }

        void Prev()
        {
            TraderContext* context = GetShopFormImp()->GetTrader()->GetEntityContext();

            if(m_commodity){

                TraderContext::iterator bound = GetLowerBound(context->begin(m_mask), context->end(), m_commodity),
                                        itor  = GetPrevious(context->begin(m_mask), bound);

                if(itor != bound){
                    m_commodity = &*itor;
                    return;
                }
            }

            TraderContext::iterator itor = GetPrevious(context->begin(m_mask), context->end());
            m_commodity = itor != context->end() ? &*itor : 0;

            Screens::Instance()->Trade()->EnableSheet(m_sheet, m_commodity != 0);
        }

        BaseThing* SellThing()
        { 
            BaseThing* thing4sale = m_commodity;

            if(thing4sale){

                TraderEntity* trader = GetShopFormImp()->GetTrader();
                TraderContext* context = trader->GetEntityContext();

				if(!IsEnoughRank(thing4sale))
				{
                    DirtyLinks::Print( DirtyLinks::GetStrRes("no_rank"));
                    SndPlayer::GetInst()->Play(SndUtils::Snd2Str(SndUtils::SND_LOGIC_DENIED));
					return 0;
				}

                //������ �� ������: ������ ������ ������� ���� �� ������� �����
                if(MoneyMgr::GetInst()->GetMoney() < trader->GetInfo()->GetSellPrice(thing4sale)){
                    DirtyLinks::Print( DirtyLinks::GetStrRes("no_money"));
                    SndPlayer::GetInst()->Play(SndUtils::Snd2Str(SndUtils::SND_LOGIC_DENIED));
                    return 0;
                }

                context->RemoveThing(thing4sale);

                //���������� � ��������
                int sum = trader->GetInfo()->GetSellPrice(thing4sale);
                MoneyMgr::GetInst()->SetMoney(MoneyMgr::GetInst()->GetMoney() - sum);                

                TraderContext::iterator itor = GetLowerBound(context->begin(m_mask), context->end(), thing4sale);

                if(itor != context->end())
                    m_commodity = &*itor;
                else
                    Next();
            }
         
            return thing4sale;
        }

        BaseThing* GetThing()
        {
            return m_commodity;
        }

    private:

        TraderContext::iterator GetLowerBound(TraderContext::iterator first, TraderContext::iterator last, BaseThing* thing)
        {
            TraderContext::less_thing less;

            while(first != last){

                if(!less(&*first, thing) && !less(thing, &*first))
                    return first;

                ++first;
            }

            return first;
        }

        TraderContext::iterator GetUpperBound(TraderContext::iterator first, TraderContext::iterator last, BaseThing* thing)
        {
            TraderContext::less_thing less;

            while(first != last){

                if(less(thing, &*first))
                    return first;

                ++first;
            }

            return first;
        }

        TraderContext::iterator GetPrevious(TraderContext::iterator first, TraderContext::iterator last)
        {
            TraderContext::iterator prev = last;
            while(first != last) prev = first++;
            return prev;
        }
        
    private: 

        unsigned    m_mask;
        BaseThing*  m_commodity;

        TradeScreen::SHEET m_sheet;
    };
}

//===============================================================

class MarketSlotDecorator : public SlotDecorator{
public:
    
    MarketSlotDecorator(SlotStrategy* strategy) :
      m_user(0), SlotDecorator(strategy) {}      
      
    void Fill(SESlot* slot, SlotEnv* env)
    {
        SlotDecorator::Fill(m_slot = slot, m_env = env);

        SEItem* mouse = m_env->GetEngine()->MigrationMgr()->GetItem();
        Illuminate(SlotStrategy::ST_NONE, mouse ? mouse->GetBaseThing() : 0);

        if(m_user == 0) m_user = m_env->GetActor();
    }

    void Clear() { }

    //������������� ��� ����� � ����
    void Init()
    {
      m_slot->UnScroll();
    }

    //������� ��� ������ �� ����
    void Shut()
    {
        SESlot::Iterator itor = m_slot->Begin();

        while(itor != m_slot->End()){
            m_env->GetGround()->Insert(m_user, itor->GetBaseThing());
            ++itor;
        }

        m_slot->Clear();
        m_user = 0;
    }

    void Illuminate(SlotStrategy::slot_type slot, BaseThing* thing)
    {
        SlotDecorator::Illuminate(slot, thing);
        GetShopFormImp()->HandleIlluminateSheetReq(thing);
    }
      
    bool DoDrag(SEItem* item)
    {
        bool ret = SlotDecorator::DoDrag(item);
        UpdateTotalSum();
        return ret;
    }
      
    bool DoDrop(SEItem* item, const ipnt2_t& to)
    {
        bool ret = SlotDecorator::DoDrop(item, to);
        UpdateTotalSum();
        return ret;
    }
      
    void SellThings(TraderEntity* trader)
    {
        SESlot::Iterator itor = m_slot->Begin();
        
        //���������� � ��������
        MoneyMgr::GetInst()->SetMoney(MoneyMgr::GetInst()->GetMoney() + CalcTotalSum());
        
        //��������� ���� ������� ���������
        if(itor != m_slot->End()) SndPlayer::GetInst()->Play(SndUtils::Snd2Str(SndUtils::SND_SELL_TO_TRADER));

        //������� �������� � ��������
        while(itor != m_slot->End()){
            
            SEItem* item = &*(itor++);
            
            m_slot->Remove(item);
            
            trader->GetEntityContext()->InsertThing(item->GetBaseThing());
            
            delete item;              
        } 
        
        UpdateTotalSum();
    }
      
    void BuyThing(BaseThing* thing)
    {
        //���� �������� ������� � ����
        SEItem* item = m_env->CreateItem(thing);
        
        if(!m_slot->CanInsert(item)) throw CASUS("MarketSlotDecorator: � ����-�� �� ����������!");
        
        //���������� �������
        m_env->SetState(item);          
        //�������� � ����
        m_slot->Insert(item);
        
        UpdateTotalSum();
    }
    
private:
    
    void UpdateTotalSum()
    {
        Screens::Instance()->Trade()->SetText(TradeScreen::T_SALE_COST, Int2Str(CalcTotalSum()).c_str());
    }
    
    int CalcTotalSum()
    {
        int sum = 0;
        
        SESlot::Iterator itor = m_slot->Begin();
        
        while(itor != m_slot->End()){
            sum += GetShopFormImp()->GetTrader()->GetInfo()->GetBuyPrice(itor->GetBaseThing());
            ++itor;
        } 
        
        return sum;
    }
    
private:

    SlotEnv* m_env;
    SESlot*  m_slot;

    HumanEntity* m_user;
};

//===============================================================

ShopFormImp::ShopFormImp() :
    m_hero(0), m_cur_sheet(TradeScreen::S_GUNS), m_drag_drop(new DragDropManager(Screens::Instance()->Trade()->GetSlotsEngine()))
{
    Screens::Instance()->Trade()->SetController(this);

    memset(m_sheet, 0, sizeof(m_sheet));

    //��������� �������
    m_sheet[TradeScreen::S_AMMO].reset(new UsualSheet(TradeScreen::S_AMMO, TT_AMMO));
    m_sheet[TradeScreen::S_GUNS].reset(new UsualSheet(TradeScreen::S_GUNS, TT_WEAPON));
    m_sheet[TradeScreen::S_GREN].reset(new UsualSheet(TradeScreen::S_GREN, TT_GRENADE));
    m_sheet[TradeScreen::S_ARMOR].reset(new UsualSheet(TradeScreen::S_ARMOR, TT_ARMOUR));
    m_sheet[TradeScreen::S_EQUIP].reset(new UsualSheet(TradeScreen::S_EQUIP, TT_IMPLANT|TT_MEDIKIT|TT_MONEY|TT_KEY|TT_CAMERA|TT_SHIELD|TT_SCANNER));

    //������� ����� � �������� Drag&Drop'a

    m_market_slot = new MarketSlotDecorator(SlotFactory::Create(SlotStrategy::ST_MARKET));
    m_drag_drop->Insert(TradeScreen::S_TRADE, m_market_slot);

    m_unloader = new UnloadingDecorator(SlotFactory::Create(SlotStrategy::ST_HANDS));
	m_drag_drop->Insert(TradeScreen::S_HANDS, m_unloader);

    m_drag_drop->Insert(TradeScreen::S_BODY, SlotFactory::Create(SlotStrategy::ST_BODY));
	m_drag_drop->Insert(TradeScreen::S_HEAD, SlotFactory::Create(SlotStrategy::ST_HEAD));
    m_drag_drop->Insert(TradeScreen::S_GROUND, SlotFactory::Create(SlotStrategy::ST_GROUND));
	m_drag_drop->Insert(TradeScreen::S_IMPLANT, SlotFactory::Create(SlotStrategy::ST_IMPLANTS)); 
	m_drag_drop->Insert(TradeScreen::S_BACKPACK, SlotFactory::Create(SlotStrategy::ST_BACKPACK));
	m_drag_drop->Insert(TradeScreen::S_KNEE_LEFT, SlotFactory::Create(SlotStrategy::ST_LKNEE));
	m_drag_drop->Insert(TradeScreen::S_KNEE_RIGHT, SlotFactory::Create(SlotStrategy::ST_RKNEE));
}

//=====================================================================================//
//                             ShopFormImp::~ShopFormImp()                             //
//=====================================================================================//
ShopFormImp::~ShopFormImp()
{
    //Screens::Instance()->Trade()->SetController(0);
}

void ShopFormImp::Show()
{
    Screens::Instance()->Activate(Screens::SID_TRADE);
}

void ShopFormImp::ShowFirstActiveSheet()
{
    TradeScreen* screen = Screens::Instance()->Trade();

    //������� ������� �������
    for(int k = 0; k < TradeScreen::MAX_SHEET; k++){        

        if(m_sheet[k]->GetThing()){
            
            screen->EnableButton(TradeScreen::B_BUY, true);
            screen->EnableButton(TradeScreen::B_PREV_THING, true);
            screen->EnableButton(TradeScreen::B_NEXT_THING, true);

            OnSheetClick(static_cast<TradeScreen::SHEET>(k));
            return;
        }
    }

    //��� ����� ��� ����������, ��� ��� ������
    m_sheet[0]->Show(m_hero);

    m_cur_sheet = TradeScreen::MAX_SHEET;

    screen->EnableButton(TradeScreen::B_BUY,false);
    screen->EnableButton(TradeScreen::B_PREV_THING,false);
    screen->EnableButton(TradeScreen::B_NEXT_THING,false);
}

void ShopFormImp::HandleInput(state_type* state)
{
    if(Input::KeyBack(DIK_ESCAPE)) HandleExitFormReq();

    /*
    if(Input::KeyBack(DIK_P)) HandlePrevHeroReq();
    if(Input::KeyBack(DIK_N)) HandleNextHeroReq();
    */
}

void ShopFormImp::Init(const ini_s& ini)
{
    const shop_ini_s& sini = static_cast<const shop_ini_s&>(ini);

    m_trader = sini.m_trader;
    m_ground = sini.m_ground;
    m_scroller = sini.m_scroller;

    //��������� ��������� ��������
    TradeScreen::TraderTraits traits;

    std::string desc = GetTraderDesc(m_trader);

    traits.m_Description = desc.c_str();
    traits.m_Portrait    = m_trader->GetInfo()->GetRID().c_str();

    TradeScreen* screen = Screens::Instance()->Trade();

    screen->SetTraderTraits(traits);
    
    //��������� ������ � �������������� ���������
    screen->SetSheetState(TradeScreen::S_AMMO, TradeScreen::SS_NORMAL);
    screen->SetSheetState(TradeScreen::S_GUNS, TradeScreen::SS_NORMAL);
    screen->SetSheetState(TradeScreen::S_GREN, TradeScreen::SS_NORMAL);
    screen->SetSheetState(TradeScreen::S_ARMOR, TradeScreen::SS_NORMAL);
    screen->SetSheetState(TradeScreen::S_EQUIP, TradeScreen::SS_NORMAL);

    //����������������� �������
    for(int k = 0; k < TradeScreen::MAX_SHEET; m_sheet[k++]->Init());

    //��������� ���� �����
	int herorank = sini.m_human->Cast2Human()->GetEntityContext()->GetRank();
    screen->SetText(TradeScreen::T_SALE_COST, Int2Str(0).c_str());
    screen->SetText(TradeScreen::T_HERO_MONEY, Int2Str(MoneyMgr::GetInst()->GetMoney()).c_str());
	screen->SetText(TradeScreen::T_HERO_RANK, PoL::RankTable::instance()->getLocName(herorank).c_str());

    //���� ������ �� �������� �������� ������ �������
    screen->EnableButton(TradeScreen::B_PREV_HERO, m_scroller->CanScroll());    
    screen->EnableButton(TradeScreen::B_NEXT_HERO, m_scroller->CanScroll());    

    m_drag_drop->SetBin(sini.m_bin);
    m_drag_drop->SetGround(sini.m_ground);

    //������� ������ �����
    m_scroller->First(sini.m_human);
    HandleSelectHeroReq(m_scroller->Get());

    m_market_slot->Init();
    
    //������� ������� �������
    ShowFirstActiveSheet();   
}

void ShopFormImp::OnButtonClick(TradeScreen::BUTTON button)
{
    switch(button){
    case TradeScreen::B_PREV_HERO:
        HandlePrevHeroReq();
        break;

    case TradeScreen::B_NEXT_HERO:
        HandleNextHeroReq();
        break;

    case TradeScreen::B_UNLOAD:
        m_unloader->Unload();
        break;        

    case TradeScreen::B_BACK:
        HandleExitFormReq();
        break;

    case TradeScreen::B_PREV_THING:
        HandlePrevCommodityReq();
        break;

    case TradeScreen::B_NEXT_THING:
        HandleNextCommodityReq();
        break;

    case TradeScreen::B_BUY:
        HandleBuyReq();
        break;

    case TradeScreen::B_SELL:
        HandleSellReq();
        break;
    }
}

void ShopFormImp::OnCommodityClick()
{
    if(BaseThing* thing = m_sheet[m_cur_sheet]->GetThing())
        Forms::GetInst()->ShowDescDialog(thing);
}

void ShopFormImp::OnSheetClick(TradeScreen::SHEET sheet)
{
    m_cur_sheet = sheet;

    m_sheet[m_cur_sheet]->Show(m_hero);
    Screens::Instance()->Trade()->SetSheetState(m_cur_sheet, TradeScreen::SS_PUSHED);
}

void ShopFormImp::HandlePrevCommodityReq()
{
    m_sheet[m_cur_sheet]->Prev();
    m_sheet[m_cur_sheet]->Show(m_hero);
}

void ShopFormImp::HandleNextCommodityReq()
{
    m_sheet[m_cur_sheet]->Next();
    m_sheet[m_cur_sheet]->Show(m_hero);
}

void ShopFormImp::HandleExitFormReq()
{
    if(m_drag_drop->CancelDrop())
        return;

    m_market_slot->Shut();

    HandleSelectHeroReq(0);
    Forms::GetInst()->ShowGameForm();
}

void ShopFormImp::HandleSelectHeroReq(HumanEntity* hero)
{   
    if(m_hero) m_hero->Detach(this);

    m_drag_drop->SetActor(m_hero = hero);

    if(m_hero == 0) return;

    m_hero->Attach(this, EV_TAKE_MONEY);
    m_hero->Attach(this, EV_WEIGHT_CHANGE);
    m_hero->Attach(this, EV_WEAPON_STATE_CHANGE);

    //��� �����
    Screens::Instance()->Trade()->SetText(TradeScreen::T_HERO_NAME, m_hero->GetInfo()->GetName().c_str());

    Update(m_hero, 0, 0);

	int herorank = m_hero->GetEntityContext()->GetRank();
	Screens::Instance()->Trade()->SetText(TradeScreen::T_HERO_RANK, PoL::RankTable::instance()->getLocName(herorank).c_str());

    if(m_cur_sheet == TradeScreen::MAX_SHEET)
	{
        ShowFirstActiveSheet();
	}
    else
	{
        m_sheet[m_cur_sheet]->Show(m_hero);
	}
}

void ShopFormImp::Update(BaseEntity* entity, event_t event, info_t)
{
    TradeScreen::HeroTraits hero_traits;
    HumanContext* context = m_hero->GetEntityContext();
    
    //�������� �������� �����
    hero_traits.m_WeightMax     = context->GetLimits()->GetWeight(); 
    hero_traits.m_WeightCurrent = context->GetTraits()->GetWeight();

    TradeScreen* screen = Screens::Instance()->Trade();

    //��������� �������� �����
    screen->SetHeroTraits(hero_traits);

    //������ �������� ������
    HumanContext::iterator itor = context->begin(HPK_HANDS, TT_WEAPON);
    screen->EnableButton(TradeScreen::B_UNLOAD, itor != context->end() && static_cast<WeaponThing*>(&*itor)->GetAmmo());    
}

TraderEntity* ShopFormImp::GetTrader()
{
    return m_trader;
}

void ShopFormImp::HandleSellReq()
{
    m_market_slot->SellThings(m_trader);

    for(int k = 0;  k < TradeScreen::MAX_SHEET; k++)
	{
        m_sheet[k]->Next();    
        m_sheet[k]->Prev();
    }

    if(m_cur_sheet == TradeScreen::MAX_SHEET)
	{
        ShowFirstActiveSheet();
	}
    else
	{
        m_sheet[m_cur_sheet]->Show(m_hero);
	}
    
    Screens::Instance()->Trade()->SetText(TradeScreen::T_HERO_MONEY, Int2Str(MoneyMgr::GetInst()->GetMoney()).c_str());
}

void ShopFormImp::HandleBuyReq()
{
    if(BaseThing* thing = m_sheet[m_cur_sheet]->SellThing())
	{
        m_market_slot->BuyThing(thing);

        if(m_sheet[m_cur_sheet]->GetThing())
            m_sheet[m_cur_sheet]->Show(m_hero);
        else
            ShowFirstActiveSheet();

        //��������� ���� �������� ������� ��������
        SndPlayer::GetInst()->Play(SndUtils::Snd2Str(SndUtils::SND_BUY_FROM_TRADER_SUCCEEDED));
        Screens::Instance()->Trade()->SetText(TradeScreen::T_HERO_MONEY, Int2Str(MoneyMgr::GetInst()->GetMoney()).c_str());
        return;
    }

    //��������� ���� �� �������� ������� ��������
    SndPlayer::GetInst()->Play(SndUtils::Snd2Str(SndUtils::SND_BUY_FROM_TRADER_FAILED));
}

void ShopFormImp::HandleIlluminateSheetReq(BaseThing* active)
{
    if(m_hero && m_cur_sheet != TradeScreen::MAX_SHEET)
        m_sheet[m_cur_sheet]->Show(m_hero, active);
}

void ShopFormImp::OnCommodityHanging(HANGING_MODE hm)
{
    m_drag_drop->Illuminate(SlotStrategy::ST_NONE, hm == HM_START ? m_sheet[m_cur_sheet]->GetThing() : 0);
}

void ShopFormImp::HandleNextHeroReq()
{
    m_scroller->Next();
    
    if(m_scroller->IsEnd()) m_scroller->First();
    
    HandleSelectHeroReq(m_scroller->Get());
}

void ShopFormImp::HandlePrevHeroReq()
{
    m_scroller->Prev();

    if(m_scroller->IsREnd()) m_scroller->Last();
    
    HandleSelectHeroReq(m_scroller->Get());
}

void ShopFormImp::MakeSaveLoad(SavSlot& slot)
{
}




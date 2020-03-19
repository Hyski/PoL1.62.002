//
// ���������� ���� ��������
//

#ifndef _PUNCH_SHOPFORMIMP_H_
#define _PUNCH_SHOPFORMIMP_H_

#include "form.h"
#include "entityaux.h"

#include <undercover/interface/tradescreen.h>

class BaseThing;
class HumanEntity;
class TraderEntity;
class DragDropManager;
class UnloadingDecorator;
class MarketSlotDecorator;

//
// ���������� ������� �� ������� � ������� ��������
//
class CommoditySheet{
public:

    virtual ~CommoditySheet(){}

    //������������� ����� ������ � ����
    virtual void Init() = 0;
    //�������� ������� ��������� �������
    virtual void Show(HumanEntity* hero, BaseThing* active = 0) = 0;

    //������� � ��������� �������� 
    virtual void Next() = 0;
    //������� � ����������� ��������
    virtual void Prev() = 0;  
    
    //������� ����� �������
    virtual BaseThing* SellThing() = 0;
    //������� ������ �� �������
    virtual BaseThing* GetThing() = 0;
};

//
// ���������� ���� ��������
//
class ShopFormImp : public  BaseForm,
                    private EntityObserver,
                    private TradeScreenController
{
public:

    ShopFormImp();
    ~ShopFormImp();

    //������ �������� ��������
    TraderEntity* GetTrader();

    //�������� ����
    void Show();
    //������������� ����
    void Init(const ini_s& ini);  
    //���������/��������� ��������� ����
    void MakeSaveLoad(SavSlot& slot);
    //��������� ������������� �����
    void HandleInput(state_type* state);
    //����������� � ����� ��������� ��������
    void HandleIlluminateSheetReq(BaseThing* active);

private:

    //��������� ������� ��������
    void HandleBuyReq();
    //��������� ������� ���������
    void HandleSellReq();
    //��������� �������� ����
    void HandleExitFormReq();
    //������� � ��������� �����
    void HandleNextHeroReq();
    //������� � ����������� �����
    void HandlePrevHeroReq();
    //��������� �������� � ����������� ��������
    void HandlePrevCommodityReq();
    //��������� �������� � ��������� ��������
    void HandleNextCommodityReq();
    //��������� ��������� ���������
    void HandleSelectHeroReq(HumanEntity* hero);

    //��������� ������� ������ ������ ���� �� ��������
	void OnCommodityClick();
    //��������� ��������� ����� ��� ��������� ��������
    void OnCommodityHanging(HANGING_MODE hm);
    //��������� �������� �� ������ ���
    void OnSheetClick(TradeScreen::SHEET id);
    //��������� ������� ������
	void OnButtonClick(TradeScreen::BUTTON id);

    //���������� ������ �������� �������
    void ShowFirstActiveSheet();

    //���������� ���������� ���������
    void Update(BaseEntity* entity, event_t event, info_t);
private:

    HumanEntity*  m_hero;
    TraderEntity* m_trader;

    AbstractGround*   m_ground;
    AbstractScroller* m_scroller;
	std::auto_ptr<DragDropManager> m_drag_drop; 

    UnloadingDecorator*  m_unloader; 
    MarketSlotDecorator* m_market_slot;

    TradeScreen::SHEET m_cur_sheet;
	std::auto_ptr<CommoditySheet> m_sheet[TradeScreen::MAX_SHEET];
};

#endif // _PUNCH_SHOPFORMIMP_H_

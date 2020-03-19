//
// ���������� ���� �������� iteration 3
//

#ifndef _PUNCH_INVENTORYFORM3_H_
#define _PUNCH_INVENTORYFORM3_H_

#include "form.h"
#include "entityaux.h"
#include <undercover/interface/inventoryscreen.h>

class HumanEntity;
class DragDropManager;
class UnloadingDecorator;

//
// ���������� ��������� LevelUp'a
//
class LevelupManager{
public:

    virtual ~LevelupManager(){}

    //���������� �����
    virtual void SetActor(HumanEntity* human) = 0;

    //������ ���-�� ����� levelup'a
    virtual int GetPoints() const = 0;

    //������ �������� ���������
    virtual int GetTrait(InventoryScreen::LEVEL_UP trait) const = 0;
    virtual int GetLimit(InventoryScreen::LEVEL_UP trait) const = 0;

    //���������� ����������/���������� ����������
    virtual void Inc(InventoryScreen::LEVEL_UP trait) = 0;
    virtual void Dec(InventoryScreen::LEVEL_UP trait) = 0;
};

//
// ���������� ����
//
class InventoryFormImp3 : public  BaseForm,
                          private EntityObserver, 
                          private InventoryScreenController
{
public:

    InventoryFormImp3();
    ~InventoryFormImp3();

    //�������� ����
    void Show();
    //������������� ����� ������� ����
    void Init(const ini_s& ini);
    //���������/��������� ��������� ����
    void MakeSaveLoad(SavSlot& slot);
    //��������� ������������� �����
    void HandleInput(state_type* state);

private:

    //������� ����� 
    void HandleDismissHero();
    //����� �� ����
    void HandleExitFormReq();
    //������� � ��������� �����
    void HandleNextHeroReq();
    //������� � ������ �����
    void HandlePrevHeroReq();
    //��������� ��������� � ����
    void HandleSelectHeroReq(HumanEntity* hero);

    //enable/disable ������ ���������/��������� ����������
    void EnableDismissAndScrollHero();

    //��������� ������� ������
    void OnButtonClick(InventoryScreen::BUTTON id);
    //��������� �������� levelup'a
    void OnLevelUpDescription(InventoryScreen::LEVEL_UP id);
    //��������� levelup'a
	void OnLevelUpClick(InventoryScreen::LEVEL_UP id, InventoryScreen::LEVEL_UP_SIGN sign);

    //����������� �� ��������� ���������� ��������
    void Update(BaseEntity *entity, event_t event, info_t info);
	void UpdateAllContent(HumanEntity *e);
	HumanEntity *GetCurrentHuman() { return m_hero; }
	void GiveItemTo(HumanEntity *, SEItem *);

private:

    HumanEntity*      m_hero;
    AbstractGround*   m_ground;
    AbstractScroller* m_scroller;
    DragDropManager*  m_drag_drop; 
    LevelupManager*   m_leveluper;

    UnloadingDecorator* m_unloader;
};

#endif //_PUNCH_INVENTORYFORM3_H_
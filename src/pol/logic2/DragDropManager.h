//
// ������ ��� ������ � ���������� drag & drop'a
//

#ifndef _PUNCH_DRAGDROPMANAGER_H_
#define _PUNCH_DRAGDROPMANAGER_H_

#include "entityaux.h"
#include <undercover/interface/SlotsEngine.h>

class SlotEnv;
class HumanEntity;
class SlotsEngine;
class AbstractBin;
class AbstractGround;

typedef unsigned slid_t; //������������� �����

//
// ��������� �� ����
//
class SlotStrategy
{
public:
    virtual ~SlotStrategy();

    enum slot_type
	{
        ST_NONE,
        ST_HEAD,
        ST_BODY,
        ST_HANDS,
        ST_LKNEE,
        ST_RKNEE,
        ST_GROUND,
        ST_MARKET,
        ST_IMPLANTS,
        ST_BACKPACK,
    };

    //������ ��� �����
    virtual slot_type GetType() const = 0;

    //�������� ����� (�����. ��������� ������� ���������)
    virtual void Clear() = 0;
    //�������� ����� (�����. ��������� ����)
    virtual void Fill(SESlot* slot, SlotEnv* env) = 0;
    //���������� �������� � ����� 
    virtual void Illuminate(slot_type slot, BaseThing* thing) = 0;

    //��������� ����������� �������� 
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // �������:
    //          true  - ����������� ����
    //          false - ����������� �� ����
    //
    virtual bool DoDrag(SEItem* item) = 0;

    //��������� �������� ��������
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // �������:
    //          true  - �������� ����
    //          false - �������� �� ����
    //
    virtual bool DoDrop(SEItem *item, const ipnt2_t& to) = 0;

	virtual bool DoDrop() { return false; }
};

//
// ��������� ��� ��������� �����
//
class SlotDecorator : public SlotStrategy
{
public:
    SlotDecorator(SlotStrategy* stratetgy);
    ~SlotDecorator();

    //������ ��� �����
    slot_type GetType() const;

    //������ ��� �����
    void Clear();
    //�������� ����� (�����. ��������� ����)
    void Fill(SESlot* slot, SlotEnv* env);
    //���������� �������� � ����� 
    void Illuminate(slot_type slot, BaseThing* thing);

    //��������� ����������� �������� 
    bool DoDrag(SEItem* item);
    //��������� �������� ��������
    bool DoDrop(SEItem* item, const ipnt2_t& to);
	bool DoDrop();

private:

    SlotStrategy* m_strategy;
};

//
// ��������� ����������� ������ � ����� ���������
//
class UnloadingDecorator : public SlotDecorator
{
public:
    UnloadingDecorator(SlotStrategy* strategy);

    //�������� ����� (�����. ��������� ����)
    void Fill(SESlot* slot, SlotEnv* env);
    //��������� ������ 
    void Unload();

private:

    SlotEnv* m_env;
};

//
// �������� ��� ��������� ������
//
class SlotEnv
{
public:
    virtual ~SlotEnv() {}

    //�������� ������ �� ��������� �����
    virtual HumanEntity *GetActor() const = 0;
	//�������� ������ �� �����, ������� ���� �������
	virtual HumanEntity *GetGiver() const = 0;
    //�������� ������ � ���������� ������
    virtual SlotsEngine *GetEngine() const = 0;
    //�������� ������ �� �����
    virtual AbstractGround *GetGround() const = 0;

    //
    //������� ��� ������ � ������
    //

    //���������� ����� �� item'e
    virtual void UpdateText(SEItem* item) = 0;
    //�������� ����������� ��������
    virtual void DiscardItem(SEItem* item) = 0;
    //������� item ��� �����
    virtual SEItem* CreateItem(BaseThing* thing) = 0;
    //���������� ��������� ��������
    virtual void SetState(SEItem* item, BaseThing* thing = 0) = 0;

    //������ ������ �������
    virtual SlotStrategy::slot_type GetLastOwner(SEItem* item) const = 0;
};

//=====================================================================================//
//                                  class SlotFactory                                  //
//=====================================================================================//
// �������� ��������� ��� ������
class SlotFactory
{
public:
    //������ ���� ���������������� ����
    static SlotStrategy* Create(SlotStrategy::slot_type type);
};

//
// �������� Drag & Drop'a
//
class DragDropManager : private SlotEnv,
                        private SEController,
                        private EntityObserver
{
public:

    DragDropManager(SlotsEngine* engine);
    ~DragDropManager();

    //���������� ������ ���� 
    void Insert(slid_t slot, SlotStrategy* strategy);

    //���������� ������ �� ���������� �������
    void SetBin(AbstractBin* bin);
    //���������� ��������� ���������
    void SetActor(HumanEntity* human);
    //���������� ������ �� ���������� �����
    void SetGround(AbstractGround* ground);

    //�������� ����� �����������
    bool CancelDrop();
	//������� ������� � ������ (���� ������ � ������, ����� �� �����)
	void DropItem(HumanEntity *);

    //���������� ���
    void Illuminate(SlotStrategy::slot_type slot, BaseThing* thing);

private:

    //===============================================
    
    //�������� ������ �� ��������� �����
    HumanEntity* GetActor() const;
	//�������� ������ �� �����, ������� ���� �������
	HumanEntity *GetGiver() const;
    //�������� ������ � ���������� ������
    SlotsEngine* GetEngine() const;
    //�������� ������ �� �����
    AbstractGround* GetGround() const;
    
    //���������� ����� �� item'e
    void UpdateText(SEItem* item);
    //�������� ����������� ��������
    void DiscardItem(SEItem* item);
	//�������� ��� DropItem
	void DropItemToBackpack(SEItem *item, HumanEntity *e);
    //������� item ��� �����
    SEItem* CreateItem(BaseThing* thing);
    //���������� ��������� ��������
    void SetState(SEItem* item, BaseThing* thing);
    //������ ������ �������
    SlotStrategy::slot_type GetLastOwner(SEItem* item) const;

    //===============================================

    //��������� ������� ������ ������ ����
    void OnItemClick(SEItem* item);
    //��������� �����������
    void OnDrag(SESlot* slot, SEItem* item);
    //���������� ���������
  	void OnLighting(SEItem* item,LIGHTING_MODE lm);
    //��������� ��������
	void OnDrop(SESlot* slot, SEItem* item, const ipnt2_t& pos);

    //===============================================

    //��������� ��������� ��������� ��������
    void Update(BaseEntity* entity, event_t event, info_t info);

private:

    HumanEntity *m_actor;
	HumanEntity *m_giver;
    SlotsEngine *m_engine;

    AbstractBin *m_bin;
    AbstractGround *m_ground;    

    typedef std::vector<SlotStrategy*> slot_vec_t;
    slot_vec_t m_slots;
};

#endif // _PUNCH_DRAGDROPMANAGER_H_


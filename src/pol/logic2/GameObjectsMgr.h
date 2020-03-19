#pragma once

//=====================================================================================//
//                                class GameObjectsMgr                                 //
//=====================================================================================//
// менеджер отслеживающий объекты на кот был Use
class GameObjectsMgr
{
public:
    static GameObjectsMgr* GetInst();

    void Reset();

    //сохранить/восстановить объекты на уровне
    void MakeSaveLoad(SavSlot& st);

    //был ли объект использован
    bool IsUsed(const rid_t& rid) const;
    //был ли объект уничтожен
    bool IsDestroyed(const rid_t& rid) const;

    //пометить объект как ипользованный
    void MarkAsUsed(const rid_t& rid);
    //пометить оъект как убитый
    void MarkAsDestroyed(const rid_t& rid);

private:
    GameObjectsMgr(){}

private:
    rid_set_t m_used;
    rid_set_t m_destroyed;
};

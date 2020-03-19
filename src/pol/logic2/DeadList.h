#pragma once

class BaseEntity;
class SpawnTag;

//=====================================================================================//
//                                   class DeadList                                    //
//=====================================================================================//
// класс - список убитых существ
class DeadList
{
public:
    //получить список убитых людей
    static DeadList* GetHeroesList();
    //получить список убитых торговцев
    static DeadList* GetTradersList();
    //получить список убитой техники
    static DeadList* GetVehiclesList();
    //получить список убитых для респауна
    static DeadList* GetRespawnerDeadList();

    //инициализация/деинициализация
    virtual void Init() = 0;
    virtual void Shut() = 0;

    //добавить существо в список
    virtual void Insert(BaseEntity* entity) = 0;
    //сохранить/загрузить данные
    virtual void MakeSaveLoad(SavSlot& slot) = 0;

    //узнвть сколько таких существ
    virtual int GetKilledCount(const SpawnTag& tag) const = 0;
};

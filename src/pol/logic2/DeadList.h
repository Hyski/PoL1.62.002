#pragma once

class BaseEntity;
class SpawnTag;

//=====================================================================================//
//                                   class DeadList                                    //
//=====================================================================================//
// ����� - ������ ������ �������
class DeadList
{
public:
    //�������� ������ ������ �����
    static DeadList* GetHeroesList();
    //�������� ������ ������ ���������
    static DeadList* GetTradersList();
    //�������� ������ ������ �������
    static DeadList* GetVehiclesList();
    //�������� ������ ������ ��� ��������
    static DeadList* GetRespawnerDeadList();

    //�������������/���������������
    virtual void Init() = 0;
    virtual void Shut() = 0;

    //�������� �������� � ������
    virtual void Insert(BaseEntity* entity) = 0;
    //���������/��������� ������
    virtual void MakeSaveLoad(SavSlot& slot) = 0;

    //������ ������� ����� �������
    virtual int GetKilledCount(const SpawnTag& tag) const = 0;
};

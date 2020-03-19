#pragma once

//=====================================================================================//
//                                class GameObjectsMgr                                 //
//=====================================================================================//
// �������� ������������� ������� �� ��� ��� Use
class GameObjectsMgr
{
public:
    static GameObjectsMgr* GetInst();

    void Reset();

    //���������/������������ ������� �� ������
    void MakeSaveLoad(SavSlot& st);

    //��� �� ������ �����������
    bool IsUsed(const rid_t& rid) const;
    //��� �� ������ ���������
    bool IsDestroyed(const rid_t& rid) const;

    //�������� ������ ��� �������������
    void MarkAsUsed(const rid_t& rid);
    //�������� ����� ��� ������
    void MarkAsDestroyed(const rid_t& rid);

private:
    GameObjectsMgr(){}

private:
    rid_set_t m_used;
    rid_set_t m_destroyed;
};

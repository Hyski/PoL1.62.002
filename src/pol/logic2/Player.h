//
// ��� ��� ���������� ��� ���������� ������
//

#ifndef _PUNCH_PLAYER_H_
#define _PUNCH_PLAYER_H_

class BasePlayer;

//=====================================================================================//
//                                 class PlayerFactory                                 //
//=====================================================================================//
// ������� �������
class PlayerFactory
{
public:
    static PlayerFactory* GetInst();

    //������� ������ - ��������
    BasePlayer* CreateHumanPlayer();
    //������� ������ 
    BasePlayer* CreateEnemyPlayer();
    //������� ��������
    BasePlayer* CreateCivilianPlayer();

private:

    PlayerFactory(){}
};

//=====================================================================================//
//                                  class PlayerPool                                   //
//=====================================================================================//
// ������� ����� ��� ��������� �������
class PlayerPool
{
public:
    ~PlayerPool();

    //singleton
    static PlayerPool* GetInst();

    //�������� ���� �������
    void Reset();
    
    //���������� ��� � ����
    void Think();
    //��������� �������
    void MakeSaveLoad(SavSlot& st);
    //�������� ������ � ������ �������
    void Insert(BasePlayer* player);

protected:
    PlayerPool();

private:
    unsigned int m_current;            //������� �����
    BasePlayer *m_players[MAX_TEAMS];
};

#endif // _PUNCH_PLAYER_H_
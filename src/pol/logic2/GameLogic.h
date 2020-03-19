//
//��� ���� ����� ������ ��������. ����������
//

#ifndef _PUNCH_GAMELOGIC_H_
#define _PUNCH_GAMELOGIC_H_

class GenericStorage;

//
// ��������� AI ����
//
class GameLogic
{
public:
    //�������������/���������������
    static void Init();
    static void Shut();

    //������ ������� ����
    static void BegNewGame();               
    //������ ������� ����
    static void BegNetGame();

    //��������� ���� - ����
    static void Draw();
    //AI ������ � ���� tick'�
    static void Think();

    //��������/���������� ����
    static void MakeSaveLoad(GenericStorage& st);  
};

#endif // _PUNCH_GAMELOGIC_H_
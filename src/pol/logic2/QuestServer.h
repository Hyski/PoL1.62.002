#ifndef _QUESTSERVER_H_
#define _QUESTSERVER_H_

class BaseEntity;

#error QuestServer is obsolete

//=====================================================================================//
//                                  class QuestServer                                  //
//=====================================================================================//
// �������� ��� ������ � quest'���
class QuestServer
{
public:

    virtual ~QuestServer() {}

    //singleton
    static QuestServer* GetInst();

    enum sheme_type{
        ST_START_NEW_GAME,   
        ST_CHANGE_EPISODE,
    };

    //�������� �������
    virtual void Shut() = 0;
    //������������� �������
    virtual void Init(sheme_type sheme, int episode) = 0;

    //��������/���������� quest'��
    virtual void MakeSaveLoad(SavSlot& slot) = 0;

    //����� �������� ��� use �� ��������
    virtual void HandleUseEntity(BaseEntity* user, BaseEntity* used) = 0;
    //������� ������ �����
    virtual void HandleFirstPhrase(BaseEntity* actor, BaseEntity* entity) = 0;
    //��������� �������� ��������
    virtual void HandleKillEntity(BaseEntity* killer, BaseEntity* victim) = 0;

    virtual void TestSmth() = 0;
};

#endif _QUESTSERVER_H_

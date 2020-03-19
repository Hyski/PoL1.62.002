//
// �������� ��� ������������ ���������� ����
//

#ifndef _PUNCH_SSCENE_H_
#define _PUNCH_SSCENE_H_

class Activity;

//
// ����� ��� ���������� � ������������ ���������� ����
//
class SSceneMgr
{
public:
    virtual ~SSceneMgr() {}

    static SSceneMgr* GetInst();

    //�������������/��������������
    virtual void Init() = 0;
    virtual void Shut() = 0;

    //�������� ���������� ����� ��� ������������
    //virtual Activity* CreateScriptScene() = 0;
    //��������� ����������� �����
    virtual void MakeSaveLoad(SavSlot& slot) = 0;

    //���� �� ��������� ���������� �����
    //virtual bool IsScriptSceneFinished(const rid_t& rid) const = 0;
};

#endif // _PUNCH_SSCENE_H_
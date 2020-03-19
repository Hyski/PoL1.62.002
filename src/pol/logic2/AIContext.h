//
// ������ AI ��������� � ������ ���������
//

#ifndef _PUNCH_AICONTEXT_H_
#define _PUNCH_AICONTEXT_H_

//
// �������� AI ��������� � ���������
//
class AIContext{
public:

    DCTOR_ABS_DEF(AIContext)

    virtual ~AIContext(){}

    //������� ����������/��������
    virtual void MakeSaveLoad(SavSlot& slot) = 0;
    //������ ���������� � ������� ������� ��������
    virtual const std::string& getSubteam() const = 0;
};

//
// ������� ��������� �������� AI
//
class AIContextFactory{
public:

    //singleton
    static AIContextFactory* GetInst();

    enum ai_type{
        AI_ENEMY,       //�������� ��� �����
        AI_CIVILIAN,    //�������� ��� �����������
    };

    //������� �������� AI ����� ����
    AIContext* Create(ai_type type, const std::string& subteam);

private:

    AIContextFactory(){}
};

#endif // _PUNCH_AICONTEXT_H_
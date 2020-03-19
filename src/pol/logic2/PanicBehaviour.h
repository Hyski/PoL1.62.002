//
// ������ ��������� �������� � ������
//

#ifndef _PUNCH_PANICBEHAVIOUR_H
#define _PUNCH_PANICBEHAVIOUR_H

class HumanEntity;

//
// ���������� ��������� � ������
//
class PanicBehaviour{
public:

    virtual ~PanicBehaviour() {}

    //������������� ������ ���������
    virtual void Init(HumanEntity* human) = 0;

    // ��������� ������ ��� ��������
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // �������:  
    //           true   -  ��������� ��������� ����� 
    //           false  -  ������ ��������� ��������� ���������
    //
    // ��������: 
    //           �� ��������� ������ ���-�� movepoints �.�. == 0
    //
    virtual bool Panic(HumanEntity* human) = 0;
};

//
// ������� ������� ��������� � ������
//
class PanicBehaviourFactory
{
public:
    //������� ������� ���������
    static PanicBehaviour* GetShockBehaviour();
    //������� ������� ������
    static PanicBehaviour* GetUsualBehaviour();
    //������� ������ ��������
    static PanicBehaviour* GetBerserkBehaviour();
};

#endif // _PUNCH_PANICBEHAVIOUR_H
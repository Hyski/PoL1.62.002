#pragma once

class HumanEntity;

//=====================================================================================//
//                                 class ThrowManager                                  //
//=====================================================================================//
// �������� �������� ���������
class ThrowManager
{
public:
    virtual ~ThrowManager(){}

    //��������� ����� ��������
    virtual bool Run() = 0;
    //������� ������� �� �������
    virtual void Init(HumanEntity* human, const point3& to) = 0;
};

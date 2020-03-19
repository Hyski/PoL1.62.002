#pragma once

#include "BaseBox.h"

//=====================================================================================//
//                           class DataBox : public BaseBox                            //
//=====================================================================================//
// ���� - ��������� ����������
class DataBox : public BaseBox
{
public:
    DataBox(const rid_t& rid = rid_t());
    ~DataBox();
    
    DCTOR_DEF(DataBox)

    //������� ��������� �� ���������
    DataBox* Cast2Info();
    //������� ���������� ��� ����� �����
    void Accept(BoxVisitor& visitor);   
    //����������/��������
    void MakeSaveLoad(SavSlot& slot);

    int GetExperience() const;

    const rid_t& GetScriptRID() const;
};

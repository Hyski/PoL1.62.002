#pragma once

class BoxVisitor;
class DataBox;
class BoxInfo;
class StoreBox;

//=====================================================================================//
//                                    class BaseBox                                    //
//=====================================================================================//
// ������� ���� �� ������
class BaseBox
{
public:
    BaseBox(const rid_t& rid);
    virtual ~BaseBox();

    DCTOR_ABS_DEF(BaseBox)

    //�������� ��������� id �����
    const rid_t& GetRID() const;

    //�������� ��������� �� �����. ��������
    virtual DataBox* Cast2Info();
    virtual StoreBox* Cast2Store();
    
    //������� ���������� ��� ����� �����
    virtual void Accept(BoxVisitor& visitor) = 0;   
    //����������/��������
    virtual void MakeSaveLoad(SavSlot& slot) = 0;

protected:

    const BoxInfo* GetInfo() const;
    
private:

    BoxInfo* m_info;
};

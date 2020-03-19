#pragma once

class DataBox;
class StoreBox;

//=====================================================================================//
//                                  class BoxVisitor                                   //
//=====================================================================================//
// ���������� ��� �����
class BoxVisitor
{
public:
    virtual ~BoxVisitor() {}

    //�������� ���� � �������
    virtual void Visit(DataBox* box) = 0;
    //�������� ���� � ����������
    virtual void Visit(StoreBox* box) = 0;
};

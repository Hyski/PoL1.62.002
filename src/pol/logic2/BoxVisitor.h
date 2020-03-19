#pragma once

class DataBox;
class StoreBox;

//=====================================================================================//
//                                  class BoxVisitor                                   //
//=====================================================================================//
// посетитель для ящика
class BoxVisitor
{
public:
    virtual ~BoxVisitor() {}

    //посетить ящик с данными
    virtual void Visit(DataBox* box) = 0;
    //посетить ящик с предметами
    virtual void Visit(StoreBox* box) = 0;
};

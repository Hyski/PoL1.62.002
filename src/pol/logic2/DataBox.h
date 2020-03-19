#pragma once

#include "BaseBox.h"

//=====================================================================================//
//                           class DataBox : public BaseBox                            //
//=====================================================================================//
// ящик - хранилище информации
class DataBox : public BaseBox
{
public:
    DataBox(const rid_t& rid = rid_t());
    ~DataBox();
    
    DCTOR_DEF(DataBox)

    //вернуть указатель на интерфейс
    DataBox* Cast2Info();
    //принять посетителя для соотв ящика
    void Accept(BoxVisitor& visitor);   
    //сохранение/загрузка
    void MakeSaveLoad(SavSlot& slot);

    int GetExperience() const;

    const rid_t& GetScriptRID() const;
};

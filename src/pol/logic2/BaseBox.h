#pragma once

class BoxVisitor;
class DataBox;
class BoxInfo;
class StoreBox;

//=====================================================================================//
//                                    class BaseBox                                    //
//=====================================================================================//
// базовый ящик на уровне
class BaseBox
{
public:
    BaseBox(const rid_t& rid);
    virtual ~BaseBox();

    DCTOR_ABS_DEF(BaseBox)

    //получить строковое id ящика
    const rid_t& GetRID() const;

    //получить указатель на соотв. инерфейс
    virtual DataBox* Cast2Info();
    virtual StoreBox* Cast2Store();
    
    //принять посетителя для соотв ящика
    virtual void Accept(BoxVisitor& visitor) = 0;   
    //сохранение/загрузка
    virtual void MakeSaveLoad(SavSlot& slot) = 0;

protected:

    const BoxInfo* GetInfo() const;
    
private:

    BoxInfo* m_info;
};

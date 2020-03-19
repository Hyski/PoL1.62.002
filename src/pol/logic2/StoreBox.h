#pragma once

#include "BaseBox.h"

//=====================================================================================//
//                           class StoreBox : public BaseBox                           //
//=====================================================================================//
// интерфейс ящика для хранения предметов
class StoreBox : public BaseBox
{
public:
    StoreBox(const rid_t& rid = rid_t());
    ~StoreBox();

    DCTOR_DEF(StoreBox)

    //вернуть указатель на интерфейс
    StoreBox* Cast2Store();
    //принять посетителя для соотв ящика
    void Accept(BoxVisitor& visitor);   
    //сохранение/загрузка
    void MakeSaveLoad(SavSlot& slot);

    class Iterator;
    typedef Iterator iterator;
    
    iterator end();
    iterator begin(unsigned type = TT_ALL_ITEMS); 

    tid_t Insert(BaseThing* thing);
    //void Remove(iterator&  itor);
    //void Remove(BaseThing* thing);

    int GetRespawn() const;   
    int GetExperience() const;

    float GetWeight() const;

    const rid_t& GetScriptRID() const;

private:
    friend class Iterator;

    typedef std::vector<BaseThing*> pack_t;
    pack_t  m_pack;
};

//=====================================================================================//
//                              class StoreBox::Iterator                               //
//=====================================================================================//
// итератор для прохода по предметам в ящике
class StoreBox::Iterator
{
public:

    Iterator(StoreBox::pack_t* pack = 0, int first = 0, unsigned mask = 0);

    //итерация
    Iterator& operator ++();
    Iterator  operator ++(int)
    { Iterator tmp = *this; operator++(); return tmp;}
    
    //операторы для удобства работы
    BaseThing* operator ->(){return (*m_pack)[m_first];}
    BaseThing& operator * (){return *(*m_pack)[m_first];}
    
    //сравнение на !=
    friend bool operator != (const Iterator& i1, const Iterator& i2)
    { return i1.m_first != i2.m_first; }

private:

    bool IsSuitable(const BaseThing* thing) const;

private:

    size_t    m_first;       
    unsigned  m_mask; 
    
    StoreBox::pack_t* m_pack;
};


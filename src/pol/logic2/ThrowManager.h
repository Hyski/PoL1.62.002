#pragma once

class HumanEntity;

//=====================================================================================//
//                                 class ThrowManager                                  //
//=====================================================================================//
// менеджер бросания предметов
class ThrowManager
{
public:
    virtual ~ThrowManager(){}

    //проиграть полет предмета
    virtual bool Run() = 0;
    //бросить предмет на уровень
    virtual void Init(HumanEntity* human, const point3& to) = 0;
};

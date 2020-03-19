//
// модели поведения человека в панике
//

#ifndef _PUNCH_PANICBEHAVIOUR_H
#define _PUNCH_PANICBEHAVIOUR_H

class HumanEntity;

//
// абстракция поведения в панике
//
class PanicBehaviour{
public:

    virtual ~PanicBehaviour() {}

    //инициализация модели поведения
    virtual void Init(HumanEntity* human) = 0;

    // проиграть панику для человека
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // возврат:  
    //           true   -  необходим повторный вызов 
    //           false  -  модель поведения проиграна полностью
    //
    // ВНИМАНИЕ: 
    //           по окончанию паники кол-во movepoints д.б. == 0
    //
    virtual bool Panic(HumanEntity* human) = 0;
};

//
// фабрика моделей поведения в панике
//
class PanicBehaviourFactory
{
public:
    //создать шоковое поведение
    static PanicBehaviour* GetShockBehaviour();
    //создать обычную панику
    static PanicBehaviour* GetUsualBehaviour();
    //создать панику берсерка
    static PanicBehaviour* GetBerserkBehaviour();
};

#endif // _PUNCH_PANICBEHAVIOUR_H
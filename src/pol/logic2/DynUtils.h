//
// утилиты для динамического создания объектов
//

#ifndef _PUNCH_DYNUTILS_H_
#define _PUNCH_DYNUTILS_H_

namespace DynUtils{

    //
    //динамический конструктор
    //
    class DynCtor{
    public:
        
        //фукция создания объекта
        typedef void* (*new_func_t)();
        
        DynCtor(new_func_t func, const char* cls);
        
        //найти конструктор по строковому идентификатору класса
        static DynCtor* FindCtor(const char* cls);
        
        //узнать строковый идентификатор класса
        const char* GetClass() const { return m_class; }  
        //создать объект класса
        void* Create(){return m_func();}
        
    private:
        
        new_func_t  m_func;
        const char* m_class;        
	};
    
    //
    // утилиты сист. сохранения\загрузки объектов из save'a
    //
    
    //сохранение конструктора
    template<class Obj>
    inline SavSlotBase& SaveObj(SavSlotBase& slot, const Obj* obj)
    {
        slot << std::string(obj->GetDynCtor()->GetClass());
        return slot;  
    }
    
    //получить указатель на сохраненный конструктор
    template<class Obj>
    inline SavSlotBase& LoadObj(SavSlotBase& slot, Obj*& obj)
    {
        std::string class_str;
        slot >> class_str;
        
        DynCtor* ctor = DynCtor::FindCtor(class_str.c_str());
        obj = static_cast<Obj*>(ctor->Create());
        
        return slot;
    }
}

//
// из имени класса генерируется его строковый идентификатор
//
#define CLASS_STR(class_name)  #class_name

//
// опред абстракного динамич конструктора
//
#define DCTOR_ABS_DEF(class_name)						\
	virtual DynUtils::DynCtor* GetDynCtor() const = 0;

//
// определяет динамич конструктор для данного класса
//
#define DCTOR_DEF(class_name)							\
	private:											\
		static void* CreateObj();						\
		static DynUtils::DynCtor m_dctor;				\
	public:												\
		virtual DynUtils::DynCtor* GetDynCtor() const;

//
// реализует динамич конструктор класса
//
#define DCTOR_IMP(class_name)																\
	void* class_name::CreateObj() { return new class_name; }								\
	DynUtils::DynCtor class_name::m_dctor(class_name::CreateObj, CLASS_STR(class_name));	\
	DynUtils::DynCtor* class_name::GetDynCtor() const { return &class_name::m_dctor; }

#define DCTOR_IMP_RENAMED(class_name,renamed)												\
	void* class_name::CreateObj() { return new class_name; }								\
	DynUtils::DynCtor class_name::m_dctor(class_name::CreateObj, CLASS_STR(renamed));		\
	DynUtils::DynCtor* class_name::GetDynCtor() const { return &class_name::m_dctor; }

#endif // _PUNCH_DYNUTILS_H_
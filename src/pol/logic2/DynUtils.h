//
// ������� ��� ������������� �������� ��������
//

#ifndef _PUNCH_DYNUTILS_H_
#define _PUNCH_DYNUTILS_H_

namespace DynUtils{

    //
    //������������ �����������
    //
    class DynCtor{
    public:
        
        //������ �������� �������
        typedef void* (*new_func_t)();
        
        DynCtor(new_func_t func, const char* cls);
        
        //����� ����������� �� ���������� �������������� ������
        static DynCtor* FindCtor(const char* cls);
        
        //������ ��������� ������������� ������
        const char* GetClass() const { return m_class; }  
        //������� ������ ������
        void* Create(){return m_func();}
        
    private:
        
        new_func_t  m_func;
        const char* m_class;        
	};
    
    //
    // ������� ����. ����������\�������� �������� �� save'a
    //
    
    //���������� ������������
    template<class Obj>
    inline SavSlotBase& SaveObj(SavSlotBase& slot, const Obj* obj)
    {
        slot << std::string(obj->GetDynCtor()->GetClass());
        return slot;  
    }
    
    //�������� ��������� �� ����������� �����������
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
// �� ����� ������ ������������ ��� ��������� �������������
//
#define CLASS_STR(class_name)  #class_name

//
// ����� ����������� ������� ������������
//
#define DCTOR_ABS_DEF(class_name)						\
	virtual DynUtils::DynCtor* GetDynCtor() const = 0;

//
// ���������� ������� ����������� ��� ������� ������
//
#define DCTOR_DEF(class_name)							\
	private:											\
		static void* CreateObj();						\
		static DynUtils::DynCtor m_dctor;				\
	public:												\
		virtual DynUtils::DynCtor* GetDynCtor() const;

//
// ��������� ������� ����������� ������
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
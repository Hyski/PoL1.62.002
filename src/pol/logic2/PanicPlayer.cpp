#include "logicdefs.h"

#include "Entity.h"
#include "entityaux.h"
#include "DirtyLinks.h"
#include "PanicPlayer.h"
#include "PanicBehaviour.h"

namespace PanicPlayerDetails
{

//=====================================================================================//
//                    class PanicPlayerImp : private EntityObserver                    //
//=====================================================================================//
// алгоритм подготовки к ходу
class PanicPlayerImp : private EntityObserver
{
    HumanEntity*    m_actor;
	PanicBehaviour *m_panic[MAX_PANICS];

public:
    PanicPlayerImp() : m_actor(0)
    {
		m_panic[HPT_NONE] = 0;
        m_panic[HPT_USUAL] = PanicBehaviourFactory::GetUsualBehaviour();
        m_panic[HPT_SHOCK] = PanicBehaviourFactory::GetShockBehaviour();
        m_panic[HPT_BERSERK] = PanicBehaviourFactory::GetBerserkBehaviour();
    }

    void Init(BaseEntity* entity)
    {
        if(m_actor = entity ? entity->Cast2Human() : 0)
		{
            m_actor->Attach(this, EV_PREPARE_DEATH);
			const human_panic_type panicType = m_actor->GetEntityContext()->SelectPanicType();
			PanicBehaviour * const panic = m_panic[panicType];

            if(panic) panic->Init(m_actor);                                
            
            PrintPanicMsg(m_actor, m_actor->GetEntityContext()->GetPanicType());                
        }
    }

    bool Execute()
    {
        if(m_actor)
		{
            PanicBehaviour * const behaviour = m_panic[m_actor->GetEntityContext()->GetPanicType()];

            if(behaviour && behaviour->Panic(m_actor))
			{
				return true;
			}
            
            m_actor->Detach(this);
            m_actor = 0;
        }
        
        return false;
    }

	bool CanExecute()
	{
		if(!m_actor) return false;
		PanicBehaviour *const behaviour = m_panic[m_actor->GetEntityContext()->GetPanicType()];
		return behaviour != 0;
	}

private:
    void Update(BaseEntity* entity, event_t event, info_t info)
    {
        m_actor->Detach(this);
        m_actor = 0;
    }

    void PrintPanicMsg(HumanEntity* human, human_panic_type type)
    {
        rid_t str_rid;

        if(type == HPT_USUAL)
		{
            str_rid = "aiu_usual_panic";
		}
        else if(type == HPT_SHOCK)
		{
            str_rid = "aiu_shock_panic";
		}
        else if(type == HPT_BERSERK)
		{
            str_rid = "aiu_berserk_panic";
		}
        else
		{
            return;
		}

        DirtyLinks::Print(mlprintf(DirtyLinks::GetStrRes(str_rid).c_str(), human->GetInfo()->GetName().c_str()));
    }
};

//=====================================================================================//
//                         PanicPlayerImp *getPanicPlayerImp()                         //
//=====================================================================================//
PanicPlayerImp *getPanicPlayerImp()
{
	static PanicPlayerImp inst;
	return &inst;
}

}

using namespace PanicPlayerDetails;

//=====================================================================================//
//                              void PanicPlayer::Init()                               //
//=====================================================================================//
void PanicPlayer::Init(BaseEntity *entity)
{
	getPanicPlayerImp()->Init(entity);
}

//=====================================================================================//
//                             bool PanicPlayer::Execute()                             //
//=====================================================================================//
bool PanicPlayer::Execute()
{
	return getPanicPlayerImp()->Execute();
}

//=====================================================================================//
//                           bool PanicPlayer::CanExecute()                            //
//=====================================================================================//
bool PanicPlayer::CanExecute()
{
	return getPanicPlayerImp()->CanExecute();
}

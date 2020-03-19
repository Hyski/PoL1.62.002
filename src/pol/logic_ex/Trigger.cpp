#include "precomp.h"

#include "Act.h"
#include "ActMgr.h"
#include "Trigger.h"
#include "Condition.h"
#include "ActTemplate.h"

namespace PoL
{

//=====================================================================================//
//                                 Trigger::Trigger()                                  //
//=====================================================================================//
Trigger::Trigger(Id_t id, HCondition cond, HActTemplate act, bool onetime)
:	Persistent(id),
	m_cond(cond),
	m_act(act),
	m_onetime(onetime),
	m_triggered(false)
{
	if(m_cond) m_conn = m_cond->addFn(boost::bind(onConditionChanged,this,_1));
}

//=====================================================================================//
//                                 Trigger::~Trigger()                                 //
//=====================================================================================//
Trigger::~Trigger()
{
}

//=====================================================================================//
//                             void Trigger::store() const                             //
//=====================================================================================//
void Trigger::store(mll::io::obinstream &out) const
{
	out << asInt(m_triggered);
}

//=====================================================================================//
//                               void Trigger::restore()                               //
//=====================================================================================//
void Trigger::restore(mll::io::ibinstream &in)
{
	in >> asInt(m_triggered);
}

//=====================================================================================//
//                                void Trigger::reset()                                //
//=====================================================================================//
void Trigger::reset()
{
	m_triggered = false;
}

//=====================================================================================//
//                         void Trigger::onConditionChanged()                          //
//=====================================================================================//
void Trigger::onConditionChanged(bool v)
{
	if(v && (!m_triggered || !m_onetime))
	{
		m_triggered = true;
		ACT_LOG("INFO: trigger [" << getName() << "] is performing its' act\n");
		ActMgr::start(m_act->createAct());
	}
}

}
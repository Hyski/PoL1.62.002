#include "precomp.h"
#include "OnSmbUseSignal.h"

namespace PoL
{

//=====================================================================================//
//                          OnTaskDbSignal::OnTaskDbSignal()                           //
//=====================================================================================//
OnSmbUseSignal::OnSmbUseSignal(Id_t id, const std::string &obj, const std::string &subj)
:	Signal(id),
	m_obj(obj),
	m_subj(subj)
{
	m_sigConn = TaskDatabase::instance()->attach(boost::bind(onSignal,this),m_subj,TaskDatabase::mtItemUsed);
}

//=====================================================================================//
//                         void OnSmbUseSignal::setPriority()                          //
//=====================================================================================//
void OnSmbUseSignal::setPriority(int p)
{
	m_sigConn.setPriority(p);
}

//=====================================================================================//
//                           void OnSmbUseSignal::doEnable()                           //
//=====================================================================================//
void OnSmbUseSignal::doEnable(bool e)
{
	m_sigConn.enable(e);
}

//=====================================================================================//
//                           void OnSmbUseSignal::onSignal()                           //
//=====================================================================================//
void OnSmbUseSignal::onSignal()
{
	if(TaskDatabase::instance()->getMessageInProcess().m_object == m_obj)
	{
		notify();
	}
}

}
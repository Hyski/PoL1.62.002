#include "precomp.h"
#include "OnTaskDbSignal.h"

namespace PoL
{

//=====================================================================================//
//                          OnTaskDbSignal::OnTaskDbSignal()                           //
//=====================================================================================//
OnTaskDbSignal::OnTaskDbSignal(Id_t id, TaskDatabase::MessageType mt)
:	Signal(id),
	m_mt(mt)
{
	m_sigConn = TaskDatabase::instance()->attach(boost::bind(onSignal,this),m_mt);
}

//=====================================================================================//
//                          OnTaskDbSignal::OnTaskDbSignal()                           //
//=====================================================================================//
OnTaskDbSignal::OnTaskDbSignal(Id_t id, TaskDatabase::MessageType mt,
							   const std::string &subj)
:	Signal(id),
	m_subj(subj),
	m_mt(mt)
{
	m_sigConn = TaskDatabase::instance()->attach(boost::bind(onSignal,this),m_subj,m_mt);
}

//=====================================================================================//
//                         void OnTaskDbSignal::setPriority()                          //
//=====================================================================================//
void OnTaskDbSignal::setPriority(int p)
{
	m_sigConn.setPriority(p);
}

//=====================================================================================//
//                           void OnTaskDbSignal::doEnable()                           //
//=====================================================================================//
void OnTaskDbSignal::doEnable(bool e)
{
	m_sigConn.enable(e);
}

//=====================================================================================//
//                           void OnTaskDbSignal::onSignal()                           //
//=====================================================================================//
void OnTaskDbSignal::onSignal()
{
	notify();
}

}
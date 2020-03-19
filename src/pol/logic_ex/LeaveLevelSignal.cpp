#include "precomp.h"
#include "LeaveLevelSignal.h"

namespace PoL
{

//=====================================================================================//
//                        LeaveLevelSignal::LeaveLevelSignal()                         //
//=====================================================================================//
LeaveLevelSignal::LeaveLevelSignal(Id_t id, const std::string &from)
:	Signal(id),
	m_from(from)
{
	m_sigConn = TaskDatabase::instance()->attach(boost::bind(onSignal,this),from,TaskDatabase::mtLeaveLevel);
}

//=====================================================================================//
//                        LeaveLevelSignal::LeaveLevelSignal()                         //
//=====================================================================================//
LeaveLevelSignal::LeaveLevelSignal(Id_t id, const std::string &from, const std::string &to)
:	Signal(id),
	m_from(from),
	m_to(to)
{
	m_sigConn = TaskDatabase::instance()->attach(boost::bind(onSignal,this),from,TaskDatabase::mtLeaveLevel);
}

//=====================================================================================//
//                          void LeaveLevelSignal::doEnable()                          //
//=====================================================================================//
void LeaveLevelSignal::doEnable(bool b)
{
	m_sigConn.enable(b);
}

//=====================================================================================//
//                        void LeaveLevelSignal::setPriority()                         //
//=====================================================================================//
void LeaveLevelSignal::setPriority(int p)
{
	m_sigConn.setPriority(p);
}

//=====================================================================================//
//                          void LeaveLevelSignal::onSignal()                          //
//=====================================================================================//
void LeaveLevelSignal::onSignal()
{
	if(m_to.empty() || m_to == TaskDatabase::instance()->getMessageInProcess().m_subject)
	{
		notify();
	}
}

}

#include "precomp.h"
#include "JournalAct.h"
#include "JournalMgr.h"

namespace PoL
{

//=====================================================================================//
//                              JournalAct::JournalAct()                               //
//=====================================================================================//
JournalAct::JournalAct(const std::string &jid, const std::string &hdr,
					   const std::string &strid, bool add)
:	m_jid(jid),
	m_hdr(hdr),
	m_strid(strid),
	m_add(add),
	m_started(false),
	m_ended(false)
{
}

//=====================================================================================//
//                              void JournalAct::start()                               //
//=====================================================================================//
void JournalAct::start()
{
	m_started = true;
}

//=====================================================================================//
//                              void JournalAct::update()                              //
//=====================================================================================//
void JournalAct::update(float)
{
	assert( m_started );
	assert( !m_ended );

	if(m_add)
	{
		addRecord();
	}
	else
	{
		removeRecord();
	}

	m_ended = true;
}

//=====================================================================================//
//                            void JournalAct::addRecord()                             //
//=====================================================================================//
void JournalAct::addRecord()
{
	JournalMgr::instance()->addRecord(m_jid,locStr(m_hdr),locStr(m_strid));
}

//=====================================================================================//
//                           void JournalAct::removeRecord()                           //
//=====================================================================================//
void JournalAct::removeRecord()
{
	JournalMgr::instance()->removeRecord(m_jid);
}

}

#include "precomp.h"
#include "JournalMgr.h"
#include <Logic2/AIUtils.h>

namespace PoL
{

static JournalMgr *g_inst = 0;

//=====================================================================================//
//                              JournalMgr::JournalMgr()                               //
//=====================================================================================//
JournalMgr::JournalMgr()
{
	assert( g_inst == 0 );
	g_inst = this;
}

//=====================================================================================//
//                              JournalMgr::~JournalMgr()                              //
//=====================================================================================//
JournalMgr::~JournalMgr()
{
	assert( g_inst == this );
	g_inst = 0;
}

//=====================================================================================//
//                         JournalMgr *JournalMgr::instance()                          //
//=====================================================================================//
JournalMgr *JournalMgr::instance()
{
	return g_inst;
}

//=====================================================================================//
//                            void JournalMgr::addRecord()                             //
//=====================================================================================//
void JournalMgr::addRecord(const std::string &id, const std::string &hdr, const std::string &text)
{
	if(m_records.find(id) != m_records.end()) return;

	// добавим тему
	::DiaryManager::Record hdrrec(hdr, ::DiaryManager::ROOT_KEY, ::DiaryManager::Record::F_NEW);
	::DiaryManager::key_t hdrkey = ::DiaryManager::GetInst()->Insert(hdrrec);

	// добавим запись в тему
	::DiaryManager::Record textrec(text, hdrkey, ::DiaryManager::Record::F_NEW);
	::DiaryManager::key_t textkey = ::DiaryManager::GetInst()->Insert(textrec);

	JournalRecord rec = {hdrkey, textkey, hdr, text};
	m_records.insert(std::make_pair(id,rec));
}

//=====================================================================================//
//                           void JournalMgr::removeRecord()                           //
//=====================================================================================//
void JournalMgr::removeRecord(const std::string &id)
{
	Records_t::iterator i = m_records.find(id);

	if(i != m_records.end())
	{
		::DiaryManager::GetInst()->Delete(i->second.m_textkey);
		::DiaryManager::GetInst()->Delete(i->second.m_headerkey);
		m_records.erase(i);
	}
}

//=====================================================================================//
//                           void JournalMgr::store() const                            //
//=====================================================================================//
void JournalMgr::store(mll::io::obinstream &out) const
{
	out << m_records.size();

	for(Records_t::const_iterator i = m_records.begin(); i != m_records.end(); ++i)
	{
		out << i->first << i->second.m_headerkey << i->second.m_textkey
			<< i->second.m_header << i->second.m_text;
	}
}

//=====================================================================================//
//                             void JournalMgr::restore()                              //
//=====================================================================================//
void JournalMgr::restore(mll::io::ibinstream &in)
{
	assert( m_records.empty() );

	Records_t::size_type count;
	in >> count;

	for(Records_t::size_type i = 0; i < count; ++i)
	{
		std::string id;
		JournalRecord rec;

		in  >> id >> rec.m_headerkey >> rec.m_textkey
			>> rec.m_header >> rec.m_text;

		m_records.insert(std::make_pair(id,rec));
	}
}

//=====================================================================================//
//                              void JournalMgr::reset()                               //
//=====================================================================================//
void JournalMgr::reset()
{
	while(!m_records.empty())
	{
		removeRecord(m_records.begin()->first);
	}
}

}

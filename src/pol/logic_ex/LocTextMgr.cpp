#include "precomp.h"
#include "LocTextMgr.h"

namespace PoL
{

static LocTextMgr *g_inst = 0;

//=====================================================================================//
//                              LocTextMgr::LocTextMgr()                               //
//=====================================================================================//
LocTextMgr::LocTextMgr()
{
	assert( g_inst == 0 );
	g_inst = this;

	init();
}

//=====================================================================================//
//                              LocTextMgr::~LocTextMgr()                              //
//=====================================================================================//
LocTextMgr::~LocTextMgr()
{
	assert( g_inst == this );
	g_inst = 0;
}

//=====================================================================================//
//                         LocTextMgr *LocTextMgr::instance()                          //
//=====================================================================================//
LocTextMgr *LocTextMgr::instance()
{
	return g_inst;
}

//=====================================================================================//
//                               void LocTextMgr::init()                               //
//=====================================================================================//
void LocTextMgr::init()
{
	using mll::utils::table;

	VFileWpr file("scripts/pol/locale.txt");
	if(file.get() && file->Size())
	{
		VFileBuf buf(file.get());
		std::istream stream(&buf);

		table tbl;
		stream >> tbl;

		table::row_type hdr = tbl.row(0);
		table::col_type sysid = tbl.col(std::find(hdr.begin(),hdr.end(),"sys_id"));
		table::col_type text = tbl.col(std::find(hdr.begin(),hdr.end(),"text"));
		table::col_type sound = tbl.col(std::find(hdr.begin(),hdr.end(),"sound"));

		for(int i = 1; i < tbl.height(); ++i)
		{
			std::string id = sysid[i];
			std::transform(id.begin(),id.end(),id.begin(),std::tolower);
			std::string snd = sound[i];
			std::transform(snd.begin(),snd.end(),snd.begin(),std::tolower);
			if(snd.find("sounds/") != 0) snd.insert(0,"sounds/");
			m_phrases.insert(std::make_pair(id,::phrase_s(text[i],snd)));
		}
	}
}

//=====================================================================================//
//                      std::string LocTextMgr::getString() const                      //
//=====================================================================================//
std::string LocTextMgr::getString(const std::string &id) const
{
	Phrases_t::const_iterator i = m_phrases.find(id);
	if(i == m_phrases.end()) return "No such string: " + id;
	return i->second.m_text;
}

//=====================================================================================//
//                      ::phrase_s LocTextMgr::getPhrase() const                       //
//=====================================================================================//
::phrase_s LocTextMgr::getPhrase(const std::string &id) const
{
	Phrases_t::const_iterator i = m_phrases.find(id);
	if(i == m_phrases.end()) return ::phrase_s("No such phrase: " + id,std::string());
	return i->second;
}

}

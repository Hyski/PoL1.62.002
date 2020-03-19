#include "precomp.h"

#include "LocRes.h"

namespace PoL
{

static LocRes *g_instance = 0;

//=====================================================================================//
//                                  LocRes::LocRes()                                   //
//=====================================================================================//
LocRes::LocRes()
{
	assert( g_instance == 0 );
	g_instance = this;
	using mll::utils::table;

	VFileWpr file("scripts/pol/locale2.txt");
	VFileBuf buf(file.get());
	std::istream stream(&buf);

	table tbl;
	stream >> tbl;

	table::row_type hdr = tbl.row(0);
	table::col_type sysid = tbl.col(std::find(hdr.begin(),hdr.end(),"sys_id"));
	table::col_type text = tbl.col(std::find(hdr.begin(),hdr.end(),"text"));

	for(int i = 1; i < tbl.height(); ++i)
	{
		std::string id = sysid[i];
		std::transform(id.begin(),id.end(),id.begin(),std::tolower);
		m_strTable.insert(std::make_pair(id,text[i]));
	}
}

//=====================================================================================//
//                                  LocRes::~LocRes()                                  //
//=====================================================================================//
LocRes::~LocRes()
{
	assert( g_instance == this );
	g_instance = 0;
}

//=====================================================================================//
//                    const std::string &LocRes::getLocStr() const                     //
//=====================================================================================//
const std::string &LocRes::getLocStr(const std::string &name) const
{
	StrTable_t::const_iterator i = m_strTable.find(name);
	if(i == m_strTable.end()) return m_emptyString;
	return i->second;
}

//=====================================================================================//
//                          const LocRes &LocRes::instance()                           //
//=====================================================================================//
const LocRes &LocRes::instance()
{
	static LocRes locres;
	return *g_instance;
}

}

#include "logicdefs.h"
#include "RankTable.h"
#include <boost/lexical_cast.hpp>
#include "../Common/Utils/VFileWpr.h"

namespace PoL
{

//=====================================================================================//
//                               RankTable::RankTable()                                //
//=====================================================================================//
RankTable::RankTable()
{
	using mll::utils::table;

	PoL::VFileWpr file("scripts/ranks.txt");
	PoL::VFileBuf buf(file.get());
	std::istream stream(&buf);

	table tbl;
	stream >> tbl;

	table::row_type hdr = tbl.row(0);
	table::col_type sysid = tbl.col(std::find(hdr.begin(),hdr.end(),"sys_name"));
	table::col_type locname = tbl.col(std::find(hdr.begin(),hdr.end(),"loc_name"));

	for(int i = 1; i < tbl.height(); ++i)
	{
		int id = boost::lexical_cast<int>(sysid[i]);
		std::string locn = locname[i];
		if(m_ranks.size() <= id)
		{
			m_ranks.resize(id+1);
		}
		m_ranks[id] = locn;
	}
}

//=====================================================================================//
//                          RankTable *RankTable::instance()                           //
//=====================================================================================//
RankTable *RankTable::instance()
{
	static RankTable inst;
	return &inst;
}

}
#pragma once

//=====================================================================================//
//                                  class InfoReader                                   //
//=====================================================================================//
template<typename T>
class InfoReader
{
public:
	typedef mll::utils::table table;

private:
	static table m_table;
	static table::row_type m_hdr;

public:
	static const table &GetTbl()
	{
		if(!m_table.width() || !m_table.height())
		{
			PoL::VFileWpr wrapper(T::GetTableName());
			PoL::VFileBuf buf(wrapper.get());
			std::istream in(&buf);
			in >> m_table;

			m_hdr = m_table.row(0);
			T::ReadColumns();
		}

		return m_table;
	}

	static table::col_type GetCol(const std::string &c)
	{
		return GetTbl().col(std::find(m_hdr.begin(),m_hdr.end(),c));
	}

	static const table::row_type &GetHdr()
	{
		return m_hdr;
	}

	static bool Init()
	{
		return &GetTbl() != 0;
	}
};

template<typename T>
mll::utils::table InfoReader<T>::m_table;

template<typename T>
mll::utils::table::row_type InfoReader<T>::m_hdr;

#define POL_TABLE_REQUIRED(id,name)																	\
	id = GetCol(name);																				\
	if(!id)																							\
	{																								\
		errout << "Отсутствует столбец \"" << name << "\" в таблице [" << GetTableName() << "]\n";	\
	}

#define POL_TABLE_OPTIONAL(id,name)																\
	id = GetCol(name);

#define POL_GET_CLASS(d)	d
#define POL_MAKE_DEFINITION(id,name,req,d)		mll::utils::table::col_type POL_GET_CLASS##d :: id;
#define POL_MAKE_DECLARATION(id,name,req,d)		static table::col_type id;
#define POL_MAKE_INITIALIZATION(id,name,req,d)	POL_TABLE_##req(id,name);

#define POL_EMPTY()

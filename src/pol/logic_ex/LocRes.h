#pragma once

#ifndef LOGIC_EX_LOCRES_H_INCLUDED
#define LOGIC_EX_LOCRES_H_INCLUDED

namespace PoL
{

//=====================================================================================//
//                                    class LocRes                                     //
//=====================================================================================//
class LocRes
{
	typedef std::hash_map<std::string,std::string> StrTable_t;

	StrTable_t m_strTable;
	std::string m_emptyString;

public:
	LocRes();
	~LocRes();

	/// Возвращает локализованную строку
	const std::string &getLocStr(const std::string &name) const;

	static const LocRes &instance();
};

//=====================================================================================//
//                        inline const std::string &getLocStr()                        //
//=====================================================================================//
inline const std::string &getLocStr(const std::string &name)
{
	return LocRes::instance().getLocStr(name);
}

}

#endif
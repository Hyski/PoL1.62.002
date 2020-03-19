#include "precomp.h"
#include "LevelCase.h"
#include "ParseUtils.h"

//=====================================================================================//
//                          void LevelCase::ReadStorageInfo()                          //
//=====================================================================================//
void LevelCase::ReadStorageInfo(const std::string &props)
{
	m_isStorage	= true;
	m_exp		= ParseUtils::GetInt(props,"experience(");
	m_respawn	= ParseUtils::GetInt(props,"respawn(");
	m_setName	= ParseUtils::GetStr(props,"items(");
	m_weight	= ParseUtils::GetInt(props,"weight(");
	m_wisdom	= ParseUtils::GetInt(props,"wisdom(");
}

//=====================================================================================//
//                         void LevelCase::ReadDatabaseInfo()                          //
//=====================================================================================//
void LevelCase::ReadDatabaseInfo(const std::string &info)
{
	m_db		= true;
	m_exp		= ParseUtils::GetInt(info,"experience(");
	m_setName	= ParseUtils::GetStr(info,"items(");
	m_wisdom	= ParseUtils::GetInt(info,"wisdom(");
}
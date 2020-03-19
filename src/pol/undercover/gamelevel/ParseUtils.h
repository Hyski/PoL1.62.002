#pragma once

//=====================================================================================//
//                                  class ParseUtils                                   //
//=====================================================================================//
class ParseUtils
{
public:
	static int GetInt(const std::string &, const std::string &);
	static float GetFloat(const std::string &, const std::string &);
	static std::string GetStr(const std::string &, const std::string &);
};
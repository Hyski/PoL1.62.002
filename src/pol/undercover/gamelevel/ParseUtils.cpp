#include "precomp.h"
#include "ParseUtils.h"

//=====================================================================================//
//                              int ParseUtils::GetInt()                               //
//=====================================================================================//
int ParseUtils::GetInt(const std::string &str, const std::string &hdr)
{
	std::string::size_type i, npos=std::string::npos;
	i=str.find(hdr);
	if(i == npos) return 0;

	assert( str[i+hdr.size()] == '(' || str[i+hdr.size()] == '=' );
	std::istringstream sstr(str.substr(i+hdr.size()+1));
	sstr >> std::ws;

	int num;
	sstr >> num;

	assert( sstr.good() || sstr.eof() );

	return num;
}

//=====================================================================================//
//                            float ParseUtils::GetFloat()                             //
//=====================================================================================//
float ParseUtils::GetFloat(const std::string &str, const std::string &hdr)
{
	std::string::size_type i, npos=std::string::npos;
	i=str.find(hdr);
	if(i == npos) return 0;

	assert( str[i+hdr.size()] == '(' );
	std::istringstream sstr(str.substr(i+hdr.size()+1));
	sstr >> std::ws;

	float num;
	sstr >> num;

	assert( sstr.good() );

	return num;
}


//=====================================================================================//
//                          std::string ParseUtils::GetStr()                           //
//=====================================================================================//
std::string ParseUtils::GetStr(const std::string &str, const std::string &hdr)
{
	std::string::size_type i,j,npos=std::string::npos;
	static const char CLOSING_SYMB = ')';

	i=str.find(hdr);
	if(i == std::string::npos) return "";

	i += 1;
	j=str.find(CLOSING_SYMB,i);

	return i==npos?"":str.substr(i+hdr.size(),j-i-hdr.size());
}
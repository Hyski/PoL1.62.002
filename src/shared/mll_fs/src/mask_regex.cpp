

#include "precomp.h"
#include <boost/regex.hpp>
#include <mll/fs/mask_regex.h>

#define allowed "[a-zA-Z0-9_\\`\\~\\!\\@\\#\\$\\%\\^\\&\\.{}\\(\\) -]"


using namespace mll::fs;

//=====================================================================================//
//                              mask_regex::mask_regex()                               //
//=====================================================================================//
mask_regex::mask_regex(const std::string &mask)
{
	using boost::regex;
	using boost::regex_replace;

	const char *expression =	"([\\[\\]\\\\+\\{\\}\\|\\^\\.\\-\\$])" "|"
								"(\\*+)" "|"
								"(\\?)";

	const char *format =		"(?1\\\\$0)"
								"(?2"allowed"*)"
								"(?3"allowed")";
	regex re(expression);

	result_mask = regex_replace(mask,re,format,boost::format_all);	
}


//=====================================================================================//
//                             bool mask_regex::is_valid()                             //
//=====================================================================================//
bool mask_regex::is_valid(const std::string &name)
{	
	//	modified by Naughty: не компилялось под интеловским компилятором
	//	return boost::regex_match(name, result_mask);
	return boost::regex_match(name, boost::regex(result_mask));
}

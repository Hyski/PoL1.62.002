#include "precomp.h"

using namespace mll::io;

/*=====================================================================================*\
 *                                      printf()                                       * 
\*=====================================================================================*/
std::string mll::io::printf(const char *str, ...)
{
	va_list pm_list;

	va_start(pm_list,str);
	std::string result = vprintf(str,pm_list);
	va_end(pm_list);

	return result;
}

/*=====================================================================================*\
 *                                      vprintf()                                      * 
\*=====================================================================================*/
std::string mll::io::vprintf(const char *str, va_list vl)
{
	int buffer_length = 64;
	char *buffer = new char[buffer_length];

	while(_vsnprintf(buffer,buffer_length, str, vl)<0)
	{
		delete [] buffer;
		buffer_length *= 2;
		buffer = new char[buffer_length];
	}
	
	std::string result(buffer);
	delete [] buffer;

	return result;
}
#include "precomp.h"
#include "dump.h"
#include <windows.h>
#include "handler.h"
#include <io.h>
#include <mll/debug/callstack.h>

#include <direct.h>

namespace
{

//=====================================================================================//
//               static std::pair<const char *, const char *> getInfo()                //
//=====================================================================================//
std::pair<const char *, const char *> getInfo(unsigned code)
{
	switch(code)
	{
		case EXCEPTION_ACCESS_VIOLATION:
			return std::pair<const char *,const char *>("EXCEPTION_ACCESS_VIOLATION","The thread tried to read from or write to a virtual address for which it does not have the appropriate access.");
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
			return std::pair<const char *,const char *>("EXCEPTION_ARRAY_BOUNDS_EXCEEDED","The thread tried to access an array element that is out of bounds and the underlying hardware supports bounds checking.");
		case EXCEPTION_BREAKPOINT:
			return std::pair<const char *,const char *>("EXCEPTION_BREAKPOINT","A breakpoint was encountered.");
		case EXCEPTION_DATATYPE_MISALIGNMENT:
			return std::pair<const char *,const char *>("EXCEPTION_DATATYPE_MISALIGNMENT","The thread tried to read or write data that is misaligned on hardware that does not provide alignment. For example, 16-bit values must be aligned on 2-byte boundaries; 32-bit values on 4-byte boundaries, and so on.");
		case EXCEPTION_FLT_DENORMAL_OPERAND:
			return std::pair<const char *,const char *>("EXCEPTION_FLT_DENORMAL_OPERAND","One of the operands in a floating-point operation is denormal. A denormal value is one that is too small to represent as a standard floating-point value.");
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:
			return std::pair<const char *,const char *>("EXCEPTION_FLT_DIVIDE_BY_ZERO","The thread tried to divide a floating-point value by a floating-point divisor of zero.");
		case EXCEPTION_FLT_INEXACT_RESULT:
			return std::pair<const char *,const char *>("EXCEPTION_FLT_INEXACT_RESULT","The result of a floating-point operation cannot be represented exactly as a decimal fraction.");
		case EXCEPTION_FLT_INVALID_OPERATION:
			return std::pair<const char *,const char *>("EXCEPTION_FLT_INVALID_OPERATION","This exception represents any floating-point exception not included in this list.");
		case EXCEPTION_FLT_OVERFLOW:
			return std::pair<const char *,const char *>("EXCEPTION_FLT_OVERFLOW","The exponent of a floating-point operation is greater than the magnitude allowed by the corresponding type.");
		case EXCEPTION_FLT_STACK_CHECK:
			return std::pair<const char *,const char *>("EXCEPTION_FLT_STACK_CHECK","The stack overflowed or underflowed as the result of a floating-point operation.");
		case EXCEPTION_FLT_UNDERFLOW:
			return std::pair<const char *,const char *>("EXCEPTION_FLT_UNDERFLOW","The exponent of a floating-point operation is less than the magnitude allowed by the corresponding type.");
		case EXCEPTION_ILLEGAL_INSTRUCTION:
			return std::pair<const char *,const char *>("EXCEPTION_ILLEGAL_INSTRUCTION","The thread tried to execute an invalid instruction.");
		case EXCEPTION_IN_PAGE_ERROR:
			return std::pair<const char *,const char *>("EXCEPTION_IN_PAGE_ERROR","The thread tried to access a page that was not present, and the system was unable to load the page. For example, this exception might occur if a network connection is lost while running a program over the network.");
		case EXCEPTION_INT_DIVIDE_BY_ZERO:
			return std::pair<const char *,const char *>("EXCEPTION_INT_DIVIDE_BY_ZERO","The thread tried to divide an integer value by an integer divisor of zero.");
		case EXCEPTION_INT_OVERFLOW:
			return std::pair<const char *,const char *>("EXCEPTION_INT_OVERFLOW","The result of an integer operation caused a carry out of the most significant bit of the result.");
		case EXCEPTION_INVALID_DISPOSITION:
			return std::pair<const char *,const char *>("EXCEPTION_INVALID_DISPOSITION","An exception handler returned an invalid disposition to the exception dispatcher. Programmers using a high-level language such as C should never encounter this exception.");
		case EXCEPTION_NONCONTINUABLE_EXCEPTION:
			return std::pair<const char *,const char *>("EXCEPTION_NONCONTINUABLE_EXCEPTION","The thread tried to continue execution after a noncontinuable exception occurred.");
		case EXCEPTION_PRIV_INSTRUCTION:
			return std::pair<const char *,const char *>("EXCEPTION_PRIV_INSTRUCTION","The thread tried to execute an instruction whose operation is not allowed in the current machine mode.");
		case EXCEPTION_SINGLE_STEP:
			return std::pair<const char *,const char *>("EXCEPTION_SINGLE_STEP","A trace trap or other single-instruction mechanism signaled that one instruction has been executed.");
		case EXCEPTION_STACK_OVERFLOW:
			return std::pair<const char *,const char *>("EXCEPTION_STACK_OVERFLOW","The thread used up its stack.");
		default:
			return std::pair<const char *,const char *>("UNKNOWN","Unknown exception");
	}
}

}

//=====================================================================================//
//                           static bool fileAlreadyExists()                           //
//=====================================================================================//
static bool fileAlreadyExists(const std::string &name)
{
	_finddata_t fd;
	intptr_t h = _findfirst(name.c_str(),&fd);
	if(h != -1)
	{
		_findclose(h);
		return true;
	}
	return false;
}

//=====================================================================================//
//                            static std::string genName()                             //
//=====================================================================================//
static std::string genName(const std::string &prefix, int index)
{
	time_t msec;
	time(&msec);
	const unsigned buffer_size = 128;
	char dumpname[buffer_size];
	strftime(dumpname,buffer_size,"_%Y_%m_%d_%H.%M.%S",localtime(&msec));
	std::ostringstream sstr;
	sstr << "logs/FATAL/" << prefix << dumpname;
	if(index) sstr << "_" << index;
	sstr << ".log";
	return sstr.str();
}

//=====================================================================================//
//                        static std::string getCrashLogName()                         //
//=====================================================================================//
static std::string getCrashLogName(const std::string &name)
{
	int counter = 0;
	std::string result = genName(name,counter++);

	while(fileAlreadyExists(result))
	{
		result = genName(name,counter++);
	}

	return result;
}

//=====================================================================================//
//                                   void makeDump()                                   //
//=====================================================================================//
std::string makeDump(unsigned code, _EXCEPTION_POINTERS *ptrs, const std::string &name)
{
	_mkdir("logs");
	_mkdir("logs/FATAL");

	std::string logname = getCrashLogName(name);

	std::ofstream dump(logname.c_str());
	dump << "Code: " << getInfo(code).first << "\n";
	dump << "Description: " << getInfo(code).second << "\n";

	dump << "\n";
	dump << "Stack trace:\n";
	stackTrace(dump,ptrs);

	dump << "\n\n";

	mll::debug::crash_info::flush_infos(dump);
	return logname;
}
#include "precomp.h"
#include "dump.h"
#include <mll/debug/callstack.h>
#include <boost/format.hpp>

#pragma warning(push)
#pragma warning(disable:4073)
#pragma init_seg(lib)
#pragma warning(pop)

namespace mll
{

namespace debug
{

static std::string g_exception_message	= "Critical exception has been occured.\n"
										  "See %1% for details.";
static std::string g_error_caption		= "Error";

//=====================================================================================//
//                 void MLL_EXPORT mll::debug::set_exception_message()                 //
//=====================================================================================//
void MLL_EXPORT set_exception_message(const std::string &s)
{
	g_exception_message = s;
}

//=====================================================================================//
//                   void MLL_EXPORT mll::debug::set_error_caption()                   //
//=====================================================================================//
void MLL_EXPORT set_error_caption(const std::string &s)
{
	g_error_caption = s;
}

static bool g_genLogOnBreakPoint = false;

//=====================================================================================//
//                    void MLL_EXPORT mll::debug::make_crash_log()                     //
//=====================================================================================//
void MLL_EXPORT make_crash_log()
{
	g_genLogOnBreakPoint = true;
	__debugbreak();
}

//=====================================================================================//
//                                   int SEHFilter()                                   //
//=====================================================================================//
LONG CALLBACK SEHFilter(/*unsigned code, */EXCEPTION_POINTERS *ptrs)
{
	if(ptrs->ExceptionRecord->ExceptionCode == EXCEPTION_BREAKPOINT)
	{
		if(g_genLogOnBreakPoint)
		{
			g_genLogOnBreakPoint = false;
			makeDump(ptrs->ExceptionRecord->ExceptionCode,ptrs,"crash_c");
		}

		return EXCEPTION_CONTINUE_EXECUTION;
	}
	else
	{
		std::string logname = makeDump(ptrs->ExceptionRecord->ExceptionCode,ptrs,"crash");
		boost::format message(g_exception_message);
		MessageBox(GetForegroundWindow(), (message % logname).str().c_str(),
										  g_error_caption.c_str(), MB_OK);
		_exit(1);
		return EXCEPTION_EXECUTE_HANDLER;
	}
}

//=====================================================================================//
//               _exception_handler_initor::_exception_handler_initor()                //
//=====================================================================================//
_exception_handler_initor::_exception_handler_initor()
:	m_oldFilter(SetUnhandledExceptionFilter(SEHFilter))
{
}

//=====================================================================================//
//               _exception_handler_initor::~_exception_handler_initor()               //
//=====================================================================================//
_exception_handler_initor::~_exception_handler_initor()
{
	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)m_oldFilter);
}

//=====================================================================================//
//                    void _exception_handler_initor::dummy_func()                     //
//=====================================================================================//
void _exception_handler_initor::dummy_func()
{
}

_exception_handler_initor MLL_EXPORT __MiST_land_Exception_Initor;

}

}
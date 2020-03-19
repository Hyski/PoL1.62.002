#include "precomp.h"

#include "CallStackUnroller.h"
#include "SymEngine.h"

#include "dbghelp.h"
#include <stdlib.h>
#include <tlhelp32.h>
#include <tchar.h>

//=====================================================================================//
//                    LONG __stdcall CrashHandlerExceptionFilter ()                    //
//=====================================================================================//
LONG __stdcall CrashHandlerExceptionFilter (EXCEPTION_POINTERS* pExPtrs)
{
    LONG lRet = EXCEPTION_CONTINUE_SEARCH ;

    // If the exception is an EXCEPTION_STACK_OVERFLOW, there isn't much
    // you can do because the stack is blown. If you try to do anything,
    // the odds are great that you'll just double-fault and bomb right
    // out of your exception filter. Although I don't recommend doing so,
    // you could play some games with the stack register and
    // manipulate it so that you could regain enough space to run these
    // functions. Of course, if you did change the stack register, you'd
    // have problems walking the stack.
    // I take the safe route and make some calls to OutputDebugString here.
    // I still might double-fault, but because OutputDebugString does very
    // little on the stack (something like 8-16 bytes), it's worth a
    // shot. You can have your users download Mark Russinovich's
    // DebugView/Enterprise Edition (www.sysinternals.com) so they can
    // at least tell you what they see.
    // The only problem is that I can't even be sure there's enough
    // room on the stack to convert the instruction pointer.
    // Fortunately, EXCEPTION_STACK_OVERFLOW doesn't happen very often.

    // Note that I still call your crash handler. I'm doing the logging
    // work here in case the blown stack kills your crash handler.
    if ( EXCEPTION_STACK_OVERFLOW ==
                              pExPtrs->ExceptionRecord->ExceptionCode )
    {
        OutputDebugString("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n") ;
        OutputDebugString("EXCEPTION_STACK_OVERFLOW occurred\n") ;
        OutputDebugString("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n") ;
    }

    __try
    {
        // The symbol engine has to be initialized here so that
        // I can look up the base module information for the
        // crash address as well as get the symbol engine
        // ready.
		for(CallStackUnroller::start(pExPtrs); !CallStackUnroller::isDone(); CallStackUnroller::next())
		{
			printf("%s\n", CallStackUnroller::item());
		}

		CallStackUnroller::finish();

/*        InitSymEng();

        const char * szBuff = GetFirstStackTraceString ( GSTSO_PARAMS | GSTSO_MODULE | GSTSO_SYMBOL | GSTSO_SRCLINE  ,
                                                         pExPtrs  ) ;
        do
        {
            printf ( "%s\n" , szBuff ) ;
            fflush ( stdout ) ;
            szBuff = GetNextStackTraceString ( GSTSO_PARAMS | GSTSO_MODULE | GSTSO_SYMBOL | GSTSO_SRCLINE , pExPtrs ) ;
        }
        while ( NULL != szBuff ) ;

        CleanupSymEng();*/
    }
    __except ( EXCEPTION_EXECUTE_HANDLER )
    {
        lRet = EXCEPTION_CONTINUE_SEARCH ;
    }
    return ( lRet ) ;
}

struct EnumInfo
{
	std::string m_module;
	std::ostream *m_out;
};

//=====================================================================================//
//                           BOOL CALLBACK enumSymbolsProc()                           //
//=====================================================================================//
BOOL CALLBACK enumSymbolsProc(SYMBOL_INFO *sinfo, ULONG ssize, EnumInfo *info)
{
	std::ostringstream nfo;

	if(sinfo->Flags & IMAGEHLP_SYMBOL_FUNCTION)
	{
		nfo << "The symbol is a function.\n";
	}

	if(sinfo->Flags & IMAGEHLP_SYMBOL_INFO_CONSTANT)
	{
		nfo << "The symbol is a constant.\n";
	}

	if(sinfo->Flags & IMAGEHLP_SYMBOL_INFO_FRAMERELATIVE)
	{
		nfo << "Offsets are frame relative.\n";
	}

	if(sinfo->Flags & IMAGEHLP_SYMBOL_INFO_LOCAL)
	{
		nfo << "The symbol is a local variable.\n";
	}

	if(sinfo->Flags & IMAGEHLP_SYMBOL_INFO_PARAMETER)
	{
		nfo << "The symbol is a parameter.\n";
	}

	if(sinfo->Flags & IMAGEHLP_SYMBOL_INFO_REGISTER)
	{
		nfo << "The symbol is a register. The Register member is used.\n";
	}

	if(sinfo->Flags & IMAGEHLP_SYMBOL_INFO_REGRELATIVE)
	{
		nfo << "Offsets are register relative.\n";
	}

//	SymGetTypeInfo((HANDLE)GetCurrentProcessId(),

	*info->m_out << nfo.str();
	*info->m_out << sinfo->TypeIndex << " " << /*"!" << info->m_module << " " <<*/ sinfo->Name;
	if(sinfo->Flags & IMAGEHLP_SYMBOL_INFO_VALUEPRESENT)
	{
		*info->m_out << "=" << sinfo->Value;
	}
	*info->m_out << "\n";
	return true;
}

//=====================================================================================//
//                            BOOL CALLBACK enumSymbols64()                            //
//=====================================================================================//
BOOL CALLBACK enumSymbols64(PTSTR name, DWORD64 address, ULONG size, EnumInfo *info)
{
/*	union
	{
		char buffer[sizeof(SYMBOL_INFO)+256];
		SYMBOL_INFO sinfo;
	};

	sinfo.SizeOfStruct = sizeof(SYMBOL_INFO)+256;

	SymFromName((HANDLE)GetCurrentProcessId(),name,&sinfo);*/
	*info->m_out << std::hex << address << " " << name << "\n";
	return true;
}

//=====================================================================================//
//                           BOOL CALLBACK enumModulesProc()                           //
//=====================================================================================//
BOOL CALLBACK enumModulesProc(PSTR moduleName, DWORD64 baseOfDll, std::ostream *out)
{
	EnumInfo info;
	info.m_module = moduleName;
	info.m_out = out;

	if(!SymEngine::EnumSymbols
		(
			(void*)GetCurrentProcessId(),
			0/*baseOfDll*/,
			NULL,
			(PSYM_ENUMERATESYMBOLS_CALLBACK)enumSymbolsProc,
			&info
		))
	{
		*out << "Cannot enumerate symbols\n";
	}
	return true;
}

//=====================================================================================//
//                                  void stackTrace()                                  //
//=====================================================================================//
void stackTrace(std::ostream &out, EXCEPTION_POINTERS *ptrs)
{
	volatile int myLocalSymbol = 0;

	CallStackUnroller::start(ptrs);
	const char * szBuff = CallStackUnroller::item();

	IMAGEHLP_STACK_FRAME frame;
    ZeroMemory(&frame, sizeof(IMAGEHLP_STACK_FRAME));

    frame.InstructionOffset = CallStackUnroller::currentFrame()->AddrPC.Offset;
    frame.StackOffset = CallStackUnroller::currentFrame()->AddrStack.Offset;
    frame.FrameOffset = CallStackUnroller::currentFrame()->AddrFrame.Offset;
	frame.ReturnOffset = CallStackUnroller::currentFrame()->AddrReturn.Offset;
	{for(unsigned i = 0; i < 4; ++i) frame.Params[i] = CallStackUnroller::currentFrame()->Params[i];}

    if(szBuff) do
    {
        out << szBuff << "\n" << std::flush;
        szBuff = CallStackUnroller::item();
		CallStackUnroller::next();
    }
    while ( !CallStackUnroller::isDone() );

//	out << "\n";

/*	if(SymEngine::canSetContext())
	{
		if(!SymEngine::SetContext((HANDLE)GetCurrentProcessId(),&frame,NULL))
		{
			out << "Cannot set context due to " << GetLastError() << "\n";
		}

		{
			EnumInfo info;
			info.m_module = "stack_trace.exe";
			info.m_out = &out;

			if(!SymEngine::EnumSymbols
				(
					(void*)GetCurrentProcessId(),
					0,
					NULL,
					(PSYM_ENUMERATESYMBOLS_CALLBACK)enumSymbolsProc,
					&info
				))
			{
				out << "Cannot enumerate symbols\n";
			}
		}
	}*/

	CallStackUnroller::finish();
}
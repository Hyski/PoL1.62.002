#if !defined(__SYM_ENGINE_H_INCLUDED_5992692519605117__)
#define __SYM_ENGINE_H_INCLUDED_5992692519605117__

#include <dbghelp.h>

enum
{
	GSTSO_PARAMS    =0x01,
	GSTSO_MODULE    =0x02,
	GSTSO_SYMBOL    =0x04,
	GSTSO_SRCLINE   =0x08,
};

//=====================================================================================//
//                                   class SymEngine                                   //
//=====================================================================================//
class SymEngine
{
public:
	static void init();
	static void shut();
	static bool good();

	static BOOL StackWalk	(
						DWORD MachineType, 
						HANDLE hProcess, 
						HANDLE hThread, 
						LPSTACKFRAME StackFrame, 
						PVOID ContextRecord, 
						PREAD_PROCESS_MEMORY_ROUTINE ReadMemoryRoutine,  
						PFUNCTION_TABLE_ACCESS_ROUTINE FunctionTableAccessRoutine,
						PGET_MODULE_BASE_ROUTINE GetModuleBaseRoutine, 
						PTRANSLATE_ADDRESS_ROUTINE TranslateAddress 
							);

	static bool canSetContext();

	static BOOL SetContext	(
						HANDLE hProcess,
						PIMAGEHLP_STACK_FRAME StackFrame,
						PIMAGEHLP_CONTEXT Context
							);

	static BOOL EnumSymbols	(
						HANDLE  hProcess,
						ULONG64  BaseOfDll,
						PCSTR  Mask,
						PSYM_ENUMERATESYMBOLS_CALLBACK  EnumSymbolsCallback,
						PVOID  UserContext
							);

	static BOOL Cleanup(HANDLE);

	static BOOL Initialize	(
						HANDLE hProcess,     
						PSTR UserSearchPath,  
						BOOL fInvadeProcess  
							);

	static DWORD LoadModule	(
						HANDLE hProcess,  
						HANDLE hFile,     
						PSTR ImageName,  
						PSTR ModuleName, 
						DWORD BaseOfDll,  
						DWORD SizeOfDll   
							);

	static BOOL GetLineFromAddr	(
							HANDLE hProcess,
							DWORD dwAddr,
							PDWORD pdwDisplacement,
							PIMAGEHLP_LINE Line
								);

	static BOOL GetSymFromAddr	(
							HANDLE hProcess,             
							DWORD Address,               
							PDWORD Displacement,       
							PIMAGEHLP_SYMBOL Symbol  
								);

	static DWORD GetModuleBase(HANDLE hProcess, DWORD dwAddr);

	static const char *getFirstStackTraceString(DWORD, EXCEPTION_POINTERS *);
	static const char *getNextStackTraceString(DWORD, EXCEPTION_POINTERS *);
	static const STACKFRAME *getCurrentFrame();
};

#endif // !defined(__SYM_ENGINE_H_INCLUDED_5992692519605117__)
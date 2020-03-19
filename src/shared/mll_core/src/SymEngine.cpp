#include "precomp.h"
#include "SymEngine.h"
#include <tlhelp32.h>
#include <tchar.h>

#pragma comment(lib,"user32.lib")

namespace
{
typedef DWORD (__stdcall *SymGetOptions_t)(VOID);
typedef DWORD (__stdcall *SymSetOptions_t)(DWORD);
typedef BOOL (__stdcall *SymCleanup_t)(HANDLE);
typedef BOOL (__stdcall *SymSetContext_t)(HANDLE,PIMAGEHLP_STACK_FRAME,PIMAGEHLP_CONTEXT);
typedef BOOL (__stdcall *SymInitialize_t)(HANDLE,PSTR,BOOL);
typedef DWORD (__stdcall *SymLoadModule_t)(HANDLE,HANDLE,PSTR,PSTR,DWORD,DWORD);
typedef BOOL (__stdcall *SymEnumSymbols_t)(HANDLE,ULONG64,PCSTR,PSYM_ENUMERATESYMBOLS_CALLBACK,PVOID);
typedef PVOID (__stdcall *SymFunctionTableAccess_t)(HANDLE,DWORD);
typedef DWORD (__stdcall *SymGetModuleBase_t)(HANDLE,DWORD);
typedef BOOL (__stdcall *SymGetLineFromAddr_t)(HANDLE,DWORD,PDWORD,PIMAGEHLP_LINE);
typedef BOOL (__stdcall *SymGetSymFromAddr_t)(HANDLE,DWORD,PDWORD,PIMAGEHLP_SYMBOL);
typedef BOOL (__stdcall *StackWalk_t)(DWORD,HANDLE,HANDLE,LPSTACKFRAME,PVOID,
									  PREAD_PROCESS_MEMORY_ROUTINE,PFUNCTION_TABLE_ACCESS_ROUTINE,
									  PGET_MODULE_BASE_ROUTINE,PTRANSLATE_ADDRESS_ROUTINE);


#define	SYMBOLS_LIST						\
	MUST_SYMBOL(SymGetOptions);				\
	MUST_SYMBOL(SymSetOptions);				\
	MUST_SYMBOL(SymCleanup);				\
	MUST_SYMBOL(SymInitialize);				\
	MUST_SYMBOL(SymLoadModule);				\
	MUST_SYMBOL(SymGetModuleBase);			\
	MUST_SYMBOL(SymFunctionTableAccess);	\
	MUST_SYMBOL(StackWalk);					\
	MUST_SYMBOL(SymGetLineFromAddr);		\
	MUST_SYMBOL(SymGetSymFromAddr);			\
	MAY_SYMBOL(SymSetContext);				\
	MAY_SYMBOL(SymEnumSymbols);

#	define MUST_SYMBOL(S) S##_t g_##S = 0
#	define MAY_SYMBOL(S) S##_t g_##S = 0
	SYMBOLS_LIST;
#	undef MUST_SYMBOL
#	undef MAY_SYMBOL

int g_bSymEngInit = 0;
HMODULE g_dbgHelp = 0;

STACKFRAME g_stFrame;
const unsigned MAX_SYM_SIZE = 256;
const unsigned BUFF_SIZE = 1024;
const unsigned SYM_BUFF_SIZE = 512;

TCHAR g_szBuff[BUFF_SIZE];
BYTE g_stSymbol[SYM_BUFF_SIZE];
IMAGEHLP_LINE g_stLine;

HMODULE *g_ahMod = 0;
unsigned g_uiModCount = 0;

// Indicates that the version information is valid.
BOOL g_bHasVersion = FALSE ;
// Indicates NT or 95/98.
BOOL g_bIsNT = TRUE ;

//=====================================================================================//
//                                     BOOL IsNT()                                     //
//=====================================================================================//
BOOL IsNT()
{
    if ( TRUE == g_bHasVersion )
    {
        return ( TRUE == g_bIsNT ) ;
    }

    OSVERSIONINFO stOSVI ;

    memset ( &stOSVI , NULL , sizeof ( OSVERSIONINFO ) ) ;
    stOSVI.dwOSVersionInfoSize = sizeof ( OSVERSIONINFO ) ;

    BOOL bRet = GetVersionEx ( &stOSVI ) ;
    if ( FALSE == bRet )
    {
        return ( FALSE ) ;
    }

    // Check the version and call the appropriate thing.
    if ( VER_PLATFORM_WIN32_NT == stOSVI.dwPlatformId )
    {
        g_bIsNT = TRUE ;
    }
    else
    {
        g_bIsNT = FALSE ;
    }
    g_bHasVersion = TRUE ;
    return ( TRUE == g_bIsNT ) ;
}

/*//////////////////////////////////////////////////////////////////////
                                Typedefs
//////////////////////////////////////////////////////////////////////*/
// The typedefs for the PSAPI.DLL functions used by this module.
typedef BOOL (WINAPI *ENUMPROCESSMODULES) ( HANDLE    hProcess   ,
                                            HMODULE * lphModule  ,
                                            DWORD     cb         ,
                                            LPDWORD   lpcbNeeded  ) ;

typedef DWORD (WINAPI *GETMODULEBASENAME) ( HANDLE  hProcess   ,
                                            HMODULE hModule    ,
                                            LPTSTR  lpBaseName ,
                                            DWORD   nSize       ) ;

typedef DWORD (WINAPI *GETMODULEFILENAMEEX) ( HANDLE  hProcess   ,
                                              HMODULE hModule    ,
                                              LPTSTR  lpFilename ,
                                              DWORD   nSize       ) ;


/*//////////////////////////////////////////////////////////////////////
                            File Static Data
//////////////////////////////////////////////////////////////////////*/
// Has the function stuff here been initialized?  This is only to be
// used by the InitPSAPI function and nothing else.
BOOL g_bInitialized = FALSE ;
// The pointer to EnumProcessModules.
ENUMPROCESSMODULES g_pEnumProcessModules = NULL ;
// The pointer to GetModuleBaseName.
GETMODULEBASENAME g_pGetModuleBaseName = NULL ;
// The pointer to GetModuleFileNameEx.
GETMODULEFILENAMEEX g_pGetModuleFileNameEx = NULL ;

//=====================================================================================//
//                                  BOOL InitPSAPI()                                   //
//=====================================================================================//
BOOL InitPSAPI(void)
{
	static bool g_bInitialized = false;
    if ( TRUE == g_bInitialized )
    {
        return ( TRUE ) ;
    }

    // Load up PSAPI.DLL.
    HINSTANCE hInst = LoadLibraryA ( "PSAPI.DLL" ) ;
    if ( NULL == hInst )
    {
        return ( FALSE ) ;
    }

    // Now do the GetProcAddress stuff.
    g_pEnumProcessModules =
           (ENUMPROCESSMODULES)GetProcAddress ( hInst ,
                                                "EnumProcessModules" ) ;
    if ( NULL == g_pEnumProcessModules )
    {
        return ( FALSE ) ;
    }

    g_pGetModuleBaseName =
        (GETMODULEBASENAME)GetProcAddress ( hInst ,
                                            "GetModuleBaseNameA" ) ;
    if ( NULL == g_pGetModuleBaseName )
    {
        return ( FALSE ) ;
    }

    g_pGetModuleFileNameEx =
        (GETMODULEFILENAMEEX)GetProcAddress ( hInst ,
                                              "GetModuleFileNameExA" ) ;
    if ( NULL == g_pGetModuleFileNameEx )
    {
        return ( FALSE ) ;
    }

    // All OK, Jumpmaster!
    g_bInitialized = TRUE ;
    return ( TRUE ) ;
}

//=====================================================================================//
//                             BOOL NT4GetLoadedModules()                              //
//=====================================================================================//
BOOL NT4GetLoadedModules(DWORD dwPID, UINT uiCount,
						 HMODULE *paModArray, LPDWORD pdwRealCount)
{

    // Initialize PSAPI.DLL, if needed.
    if ( FALSE == InitPSAPI ( ) )
    {
        SetLastErrorEx ( ERROR_DLL_INIT_FAILED , SLE_ERROR ) ;
        return ( FALSE ) ;
    }

    // Convert the process ID into a process handle.
    HANDLE hProc = OpenProcess ( PROCESS_QUERY_INFORMATION |
                                    PROCESS_VM_READ         ,
                                 FALSE                      ,
                                 dwPID                       ) ;
    if ( NULL == hProc )
    {
        return ( FALSE ) ;
    }

    // Now get the modules for the specified process.
    // Because of possible DLL unload order differences, make sure that
    // PSAPI.DLL is still loaded in case this function is called during
    // shutdown.
    if ( TRUE == IsBadCodePtr ( (FARPROC)g_pEnumProcessModules ) )
    {
        // Close the process handle used.
        CloseHandle ( hProc );

        SetLastErrorEx ( ERROR_INVALID_DLL , SLE_ERROR ) ;

        return ( FALSE ) ;
    }

    DWORD dwTotal = 0 ;
    BOOL bRet = g_pEnumProcessModules ( hProc                        ,
                                        paModArray                   ,
                                        uiCount * sizeof ( HMODULE ) ,
                                        &dwTotal                      );

    // Close the process handle used.
    CloseHandle ( hProc );

    // Convert the count from bytes to HMODULE values.
    *pdwRealCount = dwTotal / sizeof ( HMODULE ) ;

    // If bRet was FALSE, and the user was not just asking for the
    // total, there was a problem.
    if ( ( ( FALSE == bRet ) && ( uiCount > 0 ) ) || ( 0 == dwTotal ) )
    {
        return ( FALSE ) ;
    }

    // If the total returned in pdwRealCount is larger than the value in
    // uiCount, then return an error.  If uiCount is zero, then it is
    // not an error.
    if ( ( *pdwRealCount > uiCount ) && ( uiCount > 0 ) )
    {
        SetLastErrorEx ( ERROR_INSUFFICIENT_BUFFER , SLE_ERROR ) ;
        return ( FALSE ) ;
    }

    // All OK, Jumpmaster!
    SetLastError ( ERROR_SUCCESS ) ;
    return ( TRUE ) ;
}

/*//////////////////////////////////////////////////////////////////////
                                Typedefs
//////////////////////////////////////////////////////////////////////*/
// The typedefs for the TOOLHELP32.DLL functions used by this module.
// Type definitions for pointers to call tool help functions.
typedef BOOL (WINAPI *MODULEWALK) ( HANDLE          hSnapshot ,
                                    LPMODULEENTRY32 lpme       ) ;
typedef BOOL (WINAPI *THREADWALK) ( HANDLE          hSnapshot ,
                                    LPTHREADENTRY32 lpte       ) ;
typedef BOOL (WINAPI *PROCESSWALK) ( HANDLE           hSnapshot ,
                                     LPPROCESSENTRY32 lppe       ) ;
typedef HANDLE (WINAPI *CREATESNAPSHOT) ( DWORD dwFlags       ,
                                          DWORD th32ProcessID  ) ;


/*//////////////////////////////////////////////////////////////////////
                            File Static Data
//////////////////////////////////////////////////////////////////////*/
// Has the function stuff here been initialized?  This is only to be
// used by the InitPSAPI function and nothing else.
CREATESNAPSHOT g_pCreateToolhelp32Snapshot = NULL  ;
MODULEWALK     g_pModule32First            = NULL  ;
MODULEWALK     g_pModule32Next             = NULL  ;
PROCESSWALK    g_pProcess32First           = NULL  ;
PROCESSWALK    g_pProcess32Next            = NULL  ;
THREADWALK     g_pThread32First            = NULL  ;
THREADWALK     g_pThread32Next             = NULL  ;

/*----------------------------------------------------------------------
FUNCTION        :   InitTOOLHELP32
DISCUSSION      :
    Retrieves the function pointers needed to access the tool help
routines.  Since this code is supposed to work on NT4, it cannot link
to the non-existant addresses in KERNEL32.DLL.
    This is pretty much lifter right from the MSDN.
PARAMETERS      :
    None.
RETURNS         :
    TRUE  - Everything initialized properly.
    FALSE - There was a problem.
----------------------------------------------------------------------*/
//=====================================================================================//
//                                BOOL InitTOOLHELP32()                                //
//=====================================================================================//
BOOL InitTOOLHELP32(void)
{
	static bool g_bInitialized = false;
    if ( TRUE == g_bInitialized )
    {
        return ( TRUE ) ;
    }

    BOOL      bRet    = FALSE ;
    HINSTANCE hKernel = NULL  ;

    // Obtain the module handle of the kernel to retrieve addresses of
    // the tool helper functions.
    hKernel = GetModuleHandleA ( "KERNEL32.DLL" ) ;

    if ( NULL != hKernel )
    {
        g_pCreateToolhelp32Snapshot =
           (CREATESNAPSHOT)GetProcAddress ( hKernel ,
                                            "CreateToolhelp32Snapshot");
        g_pModule32First = (MODULEWALK)GetProcAddress (hKernel ,
                                                       "Module32First");
        g_pModule32Next = (MODULEWALK)GetProcAddress (hKernel        ,
                                                      "Module32Next"  );
        g_pProcess32First =
                (PROCESSWALK)GetProcAddress ( hKernel          ,
                                              "Process32First"  ) ;
        g_pProcess32Next =
                (PROCESSWALK)GetProcAddress ( hKernel         ,
                                              "Process32Next" ) ;
        g_pThread32First =
                (THREADWALK)GetProcAddress ( hKernel         ,
                                             "Thread32First"  ) ;
        g_pThread32Next =
                (THREADWALK)GetProcAddress ( hKernel        ,
                                             "Thread32Next"  ) ;
        // All addresses must be non-NULL to be successful.  If one of
        // these addresses is NULL, one of the needed lists cannot be
        // walked.

        bRet =  g_pModule32First            &&
                g_pModule32Next             &&
                g_pProcess32First           &&
                g_pProcess32Next            &&
                g_pThread32First            &&
                g_pThread32Next             &&
                g_pCreateToolhelp32Snapshot    ;
    }
    else
    {
        // Could not get the module handle of kernel.
        SetLastErrorEx ( ERROR_DLL_INIT_FAILED , SLE_ERROR ) ;
        bRet = FALSE ;
    }

    if ( TRUE == bRet )
    {
        // All OK, Jumpmaster!
        g_bInitialized = TRUE ;
    }
    return ( bRet ) ;
}

/*----------------------------------------------------------------------
FUNCTION        :   TLHELPGetLoadedModules
DISCUSSION      :
    The TOOLHELP32 specific version of GetLoadedModules.  This function
assumes that GetLoadedModules does the work to validate the parameters.
PARAMETERS      :
    dwPID        - The process ID to look into.
    uiCount      - The number of slots in the paModArray buffer.  If
                   this value is 0, then the return value will be TRUE
                   and pdwRealCount will hold the number of items
                   needed.
    paModArray   - The array to place the HMODULES into.  If this buffer
                   is too small to hold the result and uiCount is not
                   zero, then FALSE is returned, but pdwRealCount will
                   be the real number of items needed.
    pdwRealCount - The count of items needed in paModArray, if uiCount
                   is zero, or the real number of items in paModArray.
RETURNS         :
    FALSE - There was a problem, check GetLastError.
    TRUE  - The function succeeded.  See the parameter discussion for
            the output parameters.
----------------------------------------------------------------------*/
//=====================================================================================//
//                           BOOL TLHELPGetLoadedModules ()                            //
//=====================================================================================//
BOOL TLHELPGetLoadedModules (DWORD dwPID, UINT uiCount,
							 HMODULE *paModArray, LPDWORD pdwRealCount)
{

    // Always set pdwRealCount to a know value before anything else.
    *pdwRealCount = 0 ;

    if ( FALSE == InitTOOLHELP32 ( ) )
    {
        SetLastErrorEx ( ERROR_DLL_INIT_FAILED , SLE_ERROR ) ;
        return ( FALSE ) ;
    }

    // The snapshot handle.
    HANDLE        hModSnap     = NULL ;
    // The module structure.
    MODULEENTRY32 stME32              ;
    // A flag kept to report if the buffer was too small.
    BOOL          bBuffToSmall = FALSE ;


    // Get the snapshot for the specified process.
    hModSnap = g_pCreateToolhelp32Snapshot ( TH32CS_SNAPMODULE ,
                                             dwPID              ) ;
    if ( INVALID_HANDLE_VALUE == hModSnap )
    {
        return ( FALSE ) ;
    }

    memset ( &stME32 , NULL , sizeof ( MODULEENTRY32 ) ) ;
    stME32.dwSize = sizeof ( MODULEENTRY32 ) ;

    // Start getting the module values.
    if ( TRUE == g_pModule32First ( hModSnap , &stME32 ) )
    {
        do
        {
            // If uiCount is not zero, copy values.
            if ( 0 != uiCount )
            {
                // If the passed in buffer is to small, set the flag.
                // This is so we match the functionality of the NT4
                // version of this function which will return the
                // correct total needed.
                if ( ( TRUE == bBuffToSmall     ) ||
                     ( *pdwRealCount == uiCount )   )
                {
                    bBuffToSmall = TRUE ;
                    break ;
                }
                else
                {
                    // Copy this value in.
                    paModArray[ *pdwRealCount ] =
                                         (HINSTANCE)stME32.modBaseAddr ;
                }
            }
            // Bump up the real total count.
            *pdwRealCount += 1 ;
        }
        while ( ( TRUE == g_pModule32Next ( hModSnap , &stME32 ) ) ) ;
    }
    else
    {
        DWORD dwLastErr = GetLastError ( ) ;
        SetLastErrorEx ( dwLastErr , SLE_ERROR ) ;
        return ( FALSE ) ;
    }

    // Close the snapshot handle.
    CloseHandle ( hModSnap );

    // Check if the buffer was too small.
    if ( TRUE == bBuffToSmall )
    {
        SetLastErrorEx ( ERROR_INSUFFICIENT_BUFFER , SLE_ERROR ) ;
        return ( FALSE ) ;
    }

    // All OK, Jumpmaster!
    SetLastError ( ERROR_SUCCESS ) ;
    return ( TRUE ) ;
}

//=====================================================================================//
//                              BOOL GetLoadedModules ()                               //
//=====================================================================================//
BOOL GetLoadedModules (DWORD dwPID, UINT uiCount, 
					   HMODULE *paModArray, LPDWORD pdwRealCount)
{
    // Do the parameter checking for real.  Note that I only check the
    // memory in paModArray if uiCount is > 0.  The user can pass zero
    // in uiCount if they are just interested in the total to be
    // returned so they could dynamically allocate a buffer.
    if ( ( TRUE == IsBadWritePtr ( pdwRealCount , sizeof(UINT) ) )    ||
         ( ( uiCount > 0 ) &&
           ( TRUE == IsBadWritePtr ( paModArray ,
                                     uiCount * sizeof(HMODULE) ) ) )   )
    {
        SetLastErrorEx ( ERROR_INVALID_PARAMETER , SLE_ERROR ) ;
        return ( FALSE ) ;
    }

    // Figure out which OS we are on.
    OSVERSIONINFO stOSVI ;

    memset ( &stOSVI , NULL , sizeof ( OSVERSIONINFO ) ) ;
    stOSVI.dwOSVersionInfoSize = sizeof ( OSVERSIONINFO ) ;

    BOOL bRet = GetVersionEx ( &stOSVI ) ;
    if ( FALSE == bRet )
    {
        return ( FALSE ) ;
    }

    // Check the version and call the appropriate thing.
    if ( ( VER_PLATFORM_WIN32_NT == stOSVI.dwPlatformId ) &&
         ( 4 == stOSVI.dwMajorVersion                   )    )
    {
        // This is NT 4 so call its specific version in PSAPI.DLL
        return ( NT4GetLoadedModules ( dwPID        ,
                                       uiCount      ,
                                       paModArray   ,
                                       pdwRealCount  ) );
    }
    else
    {
        // Win9x and Win2K go through tool help.
        return ( TLHELPGetLoadedModules ( dwPID         ,
                                          uiCount       ,
                                          paModArray    ,
                                          pdwRealCount   ) ) ;
    }
}

//=====================================================================================//
//                              DWORD BSUSymInitialize ()                              //
//=====================================================================================//
DWORD BSUSymInitialize (DWORD dwPID, HANDLE hProcess, 
						PSTR UserSearchPath, BOOL fInvadeProcess)
{
    // If this is any flavor of NT or fInvadeProcess is FALSE, just call
    // SymInitialize itself
    if ( ( TRUE == IsNT ( ) ) || ( FALSE == fInvadeProcess ) )
    {
        return ( SymEngine::Initialize ( hProcess       ,
                                   UserSearchPath ,
                                   fInvadeProcess  ) ) ;
    }
    else
    {
        // This is Win9x and the user wants to invade!

        // The first step is to initialize the symbol engine.  If it
        // fails, there is not much I can do.
        BOOL bSymInit = SymEngine::Initialize ( hProcess       ,
                                          UserSearchPath ,
                                          fInvadeProcess  ) ;
        if ( FALSE == bSymInit )
        {
            return ( FALSE ) ;
        }

        DWORD dwCount ;
        // Find out how many modules there are.  This is a BSU function.
        if ( FALSE == GetLoadedModules ( dwPID    ,
                                         0        ,
                                         NULL     ,
                                         &dwCount  ) )
        {
            // Clean up the symbol engine and leave.
            SymEngine::Cleanup ( hProcess );
            return ( FALSE ) ;
        }
        // Allocate something big enough to hold the list.
        HMODULE * paMods = new HMODULE[ dwCount ] ;

        // Get the list for real.
        if ( FALSE == GetLoadedModules ( dwPID    ,
                                         dwCount  ,
                                         paMods   ,
                                         &dwCount  ) )
        {
            // Clean up the symbol engine and leave.
            SymEngine::Cleanup ( hProcess );
            // Free the memory that I allocated earlier.
            delete [] paMods ;
            return ( FALSE ) ;
        }
        // The module filename.
        TCHAR szModName [ MAX_PATH ] ;
        for ( UINT uiCurr = 0 ; uiCurr < dwCount ; uiCurr++ )
        {
            // Get the module's filename.
            if ( FALSE == GetModuleFileName ( paMods[ uiCurr ]     ,
                                              szModName            ,
                                              sizeof ( szModName )  ) )
            {
                // Clean up the symbol engine and leave.
                SymEngine::Cleanup ( hProcess );
                // Free the memory that I allocated earlier.
                delete [] paMods ;
                return ( FALSE ) ;
            }

            // In order to get the symbol engine to work outside a
            // debugger, it needs a handle to the image.  Yes, this
            // will leak but the OS will close it down when the process
            // ends.
            HANDLE hFile = CreateFile ( szModName       ,
                                        GENERIC_READ    ,
                                        FILE_SHARE_READ ,
                                        NULL            ,
                                        OPEN_EXISTING   ,
                                        0               ,
                                        0                ) ;

            // For whatever reason, SymLoadModule can return zero, but
            // it still loads the modules.  Sheez.
            if ( FALSE == SymEngine::LoadModule ( hProcess               ,
                                          hFile                  ,
                                          szModName              ,
                                          NULL                   ,
                                         (DWORD)paMods[ uiCurr ] ,
                                          0                       ) )
            {
                // Check the last error value.  If it is zero, then all
                // I can assume is that it worked.
                DWORD dwLastErr = GetLastError ( ) ;
                if ( ERROR_SUCCESS != dwLastErr )
                {
                    // Clean up the symbol engine and leave.
                    SymEngine::Cleanup ( hProcess );
                    // Free the memory that I allocated earlier.
                    delete [] paMods ;
                    return ( FALSE ) ;
                }
            }
        }
        delete [] paMods ;
    }
    return ( TRUE ) ;
}

//=====================================================================================//
//                           DWORD Win95GetModuleBaseName ()                           //
//=====================================================================================//
DWORD Win95GetModuleBaseName (HANDLE, HMODULE hModule, LPSTR lpBaseName, DWORD nSize)
{
    if ( TRUE == IsBadWritePtr ( lpBaseName , nSize ) )
    {
        SetLastError ( ERROR_INVALID_PARAMETER ) ;
        return ( 0 ) ;
    }

    // This could blow the stack...
    char szBuff[ MAX_PATH + 1 ] ;
    DWORD dwRet = GetModuleFileName ( hModule , szBuff , MAX_PATH ) ;
    if ( 0 == dwRet )
    {
        return ( 0 ) ;
    }

    // Find the last '\' mark.
    char * pStart = strrchr ( szBuff , '\\' ) ;
    int iMin ;
    if ( NULL != pStart )
    {
        // Move up one character.
        pStart++ ;
        //lint -e666
        iMin = std::min ( (int)nSize , (lstrlen ( pStart ) + 1) ) ;
        //lint +e666
        lstrcpyn ( lpBaseName , pStart , iMin ) ;
    }
    else
    {
        // Copy the szBuff buffer in.
        //lint -e666
        iMin = std::min ( (int)nSize , (lstrlen ( szBuff ) + 1) ) ;
        //lint +e666
        lstrcpyn ( lpBaseName , szBuff , iMin ) ;
    }
    // Always NULL terminate.
    lpBaseName[ iMin ] = '\0' ;
    return ( (DWORD)(iMin - 1) ) ;
}

//=====================================================================================//
//                    static DWORD __stdcall NTGetModuleBaseName ()                    //
//=====================================================================================//
DWORD NTGetModuleBaseName ( HANDLE  hProcess   ,
                                      HMODULE hModule    ,
                                      LPTSTR  lpBaseName ,
                                      DWORD   nSize       )
{
    // Initialize PSAPI.DLL, if needed.
    if ( FALSE == InitPSAPI ( ) )
    {
        SetLastErrorEx ( ERROR_DLL_INIT_FAILED , SLE_ERROR ) ;
        return ( FALSE ) ;
    }
    return ( g_pGetModuleBaseName ( hProcess    ,
                                    hModule     ,
                                    lpBaseName  ,
                                    nSize        ) ) ;
}

//=====================================================================================//
//                       DWORD __stdcall BSUGetModuleBaseName ()                       //
//=====================================================================================//
DWORD BSUGetModuleBaseName ( HANDLE  hProcess   ,
                                       HMODULE hModule    ,
                                       LPTSTR  lpBaseName ,
                                       DWORD   nSize       )
{
    if ( TRUE == IsNT ( ) )
    {
        // Call the NT version.  It is in NT4ProcessInfo because that is
        // where all the PSAPI wrappers are kept.
        return ( NTGetModuleBaseName ( hProcess     ,
                                       hModule      ,
                                       lpBaseName   ,
                                       nSize         ) ) ;
    }
    return ( Win95GetModuleBaseName ( hProcess     ,
                                      hModule      ,
                                      lpBaseName   ,
                                      nSize         ) ) ;

}

//=====================================================================================//
//                       BOOL __stdcall CH_ReadProcessMemory ()                        //
//=====================================================================================//
BOOL CALLBACK CH_ReadProcessMemory ( HANDLE                      ,
                            LPCVOID lpBaseAddress       ,
                            LPVOID  lpBuffer            ,
                            DWORD   nSize               ,
                            LPDWORD lpNumberOfBytesRead  )
{
    return ( ReadProcessMemory ( GetCurrentProcess ( ) ,
                                 lpBaseAddress         ,
                                 lpBuffer              ,
                                 nSize                 ,
                                 lpNumberOfBytesRead    ) ) ;
}

//=====================================================================================//
//                         BOOL InternalSymGetLineFromAddr ()                          //
//=====================================================================================//
BOOL InternalSymGetLineFromAddr( IN  HANDLE          hProcess        ,
                                 IN  DWORD           dwAddr          ,
                                 OUT PDWORD          pdwDisplacement ,
                                 OUT PIMAGEHLP_LINE  Line            )
{
#ifdef WORK_AROUND_SRCLINE_BUG

    // The problem is that the symbol engine finds only those source
    // line addresses (after the first lookup) that fall exactly on
    // a zero displacement. I'll walk backward 100 bytes to
    // find the line and return the proper displacement.
    DWORD dwTempDis = 0 ;
    while ( FALSE == SymEngine::GetLineFromAddr( hProcess        ,
                                          dwAddr -
                                           dwTempDis      ,
                                          pdwDisplacement ,
                                          Line             ) )
    {
        dwTempDis += 1 ;
        if ( 100 == dwTempDis )
        {
            return ( FALSE ) ;
        }
    }

    // I found the line, and the source line information is correct, so
    // change the displacement if I had to search backward to find
    // the source line.
    if ( 0 != dwTempDis )
    {
        *pdwDisplacement = dwTempDis ;
    }
    return ( TRUE ) ;

#else  // WORK_AROUND_SRCLINE_BUG
    return ( SymEngine::GetLineFromAddr ( hProcess         ,
                                  dwAddr           ,
                                  pdwDisplacement  ,
                                  Line              ) ) ;
#endif
}

//=====================================================================================//
//               static LPCTSTR __stdcall InternalGetStackTraceString ()               //
//=====================================================================================//
LPCTSTR InternalGetStackTraceString(DWORD dwOpts, EXCEPTION_POINTERS *pExPtrs)
{
    if ( TRUE == IsBadReadPtr ( pExPtrs                      ,
                                sizeof ( EXCEPTION_POINTERS ) ) )
    {
        return ( NULL ) ;
    }

    // The value that is returned
    LPCTSTR szRet ;
    // A temporary variable for all to use. This variable saves
    // stack space.
    DWORD dwTemp ;
    // The module base address. I look this up right after the stack
    // walk to ensure that the module is valid.
    DWORD dwModBase ;

    __try
    {
        // Initialize the symbol engine in case it isn't initialized.
		SymEngine::init();

        // Note:  If the source file and line number functions are used,
        //        StackWalk can cause an access violation.
        BOOL bSWRet = SymEngine::StackWalk (  IMAGE_FILE_MACHINE_I386           ,
											  (HANDLE)GetCurrentProcessId ( )   ,
											  GetCurrentThread ( )              ,
											  &g_stFrame                        ,
											  pExPtrs->ContextRecord            ,
											  (PREAD_PROCESS_MEMORY_ROUTINE)
														   CH_ReadProcessMemory ,
											  g_SymFunctionTableAccess          ,
											  g_SymGetModuleBase                ,
											  NULL                               ) ;
        if ( ( FALSE == bSWRet ) || ( 0 == g_stFrame.AddrFrame.Offset ))
        {
            szRet = NULL ;
            __leave ;
        }

        // Before I get too carried away and start calculating
        // everything, I need to double-check that the address returned
        // by StackWalk really exists. I've seen cases in which
        // StackWalk returns TRUE but the address doesn't belong to
        // a module in the process.
        dwModBase = SymEngine::GetModuleBase ( (HANDLE)GetCurrentProcessId ( ),
                                        g_stFrame.AddrPC.Offset       );
        if ( 0 == dwModBase )
        {
            szRet = NULL ;
            __leave ;
        }

        int iCurr = 0 ;

        // At a minimum, put in the address.
        iCurr += wsprintf ( g_szBuff + iCurr              ,
                            _T ( "%04X:%08X" )            ,
                            pExPtrs->ContextRecord->SegCs ,
                            g_stFrame.AddrPC.Offset        ) ;

        // Output the parameters?
        if ( GSTSO_PARAMS == ( dwOpts & GSTSO_PARAMS ) )
        {
            iCurr += wsprintf ( g_szBuff + iCurr          ,
                                _T ( " (0x%08X 0x%08X "\
                                      "0x%08X 0x%08X)"  ) ,
                                g_stFrame.Params[ 0 ]     ,
                                g_stFrame.Params[ 1 ]     ,
                                g_stFrame.Params[ 2 ]     ,
                                g_stFrame.Params[ 3 ]      ) ;
        }
        // Output the module name.
        if ( GSTSO_MODULE == ( dwOpts & GSTSO_MODULE ) )
        {
            iCurr += wsprintf ( g_szBuff + iCurr  , _T ( " " ) ) ;

            iCurr += BSUGetModuleBaseName ( GetCurrentProcess ( ) ,
                                            (HINSTANCE)dwModBase  ,
                                            g_szBuff + iCurr      ,
                                            BUFF_SIZE - iCurr      ) ;
        }

        DWORD dwDisp ;

        // Output the symbol name?
        if ( GSTSO_SYMBOL == ( dwOpts & GSTSO_SYMBOL ) )
        {

            // Start looking up the exception address.
            PIMAGEHLP_SYMBOL pSym = (PIMAGEHLP_SYMBOL)&g_stSymbol ;
            ZeroMemory ( pSym , SYM_BUFF_SIZE ) ;
            pSym->SizeOfStruct = sizeof ( IMAGEHLP_SYMBOL ) ;
            pSym->MaxNameLength = SYM_BUFF_SIZE -
                                  sizeof ( IMAGEHLP_SYMBOL ) ;

            if ( TRUE ==
				SymEngine::GetSymFromAddr ( (HANDLE)GetCurrentProcessId ( ) ,
                                      g_stFrame.AddrPC.Offset         ,
                                      &dwDisp                         ,
                                      pSym                            ))
            {
                iCurr += wsprintf ( g_szBuff + iCurr , _T ( ", " ) ) ;

                // Copy no more symbol information than there's room for.
                dwTemp = lstrlen ( pSym->Name ) ;
                if ( dwTemp > (DWORD)( BUFF_SIZE - iCurr -
                                     ( MAX_SYM_SIZE + 50 ) ) )
                {
                    lstrcpyn ( g_szBuff + iCurr      ,
                               pSym->Name            ,
                               BUFF_SIZE - iCurr - 1  ) ;
                    // Gotta leave now
                    szRet = g_szBuff ;
                    __leave ;
                }
                else
                {
                    if ( dwDisp > 0 )
                    {
                        iCurr += wsprintf ( g_szBuff + iCurr         ,
                                            _T( "%s()+%04d byte(s)") ,
                                            pSym->Name               ,
                                            dwDisp                   );
                    }
                    else
                    {
                        iCurr += wsprintf ( g_szBuff + iCurr ,
                                            _T ( "%s" )      ,
                                            pSym->Name        ) ;
                    }
                }
            }
            else
            {
                // If the symbol wasn't found, the source file and line
                // number won't be found either, so leave now.
                szRet = g_szBuff ;
                __leave ;
            }

        }

        // Output the source file and line number information?
        if ( GSTSO_SRCLINE == ( dwOpts & GSTSO_SRCLINE ) )
        {
            ZeroMemory ( &g_stLine , sizeof ( IMAGEHLP_LINE ) ) ;
            g_stLine.SizeOfStruct = sizeof ( IMAGEHLP_LINE ) ;

            if ( TRUE ==
                   InternalSymGetLineFromAddr ( (HANDLE)
                                                  GetCurrentProcessId(),
                                                g_stFrame.AddrPC.Offset,
                                                &dwDisp                ,
                                                &g_stLine             ))
            {
                iCurr += wsprintf ( g_szBuff + iCurr , _T ( ", " ) ) ;

                // Copy no more of the source file and line number
                // information than there's room for.
                dwTemp = lstrlen ( g_stLine.FileName ) ;
                if ( dwTemp > (DWORD)( BUFF_SIZE - iCurr -
                                       ( MAX_PATH + 50     ) ) )
                {
                    lstrcpyn ( g_szBuff + iCurr      ,
                               g_stLine.FileName     ,
                               BUFF_SIZE - iCurr - 1  ) ;
                    // Gotta leave now
                    szRet = g_szBuff ;
                    __leave ;
                }
                else
                {
                    if ( dwDisp > 0 )
                    {
                        iCurr += wsprintf(g_szBuff + iCurr             ,
                                       _T("%s, line %04d+%04d byte(s)"),
                                          g_stLine.FileName            ,
                                          g_stLine.LineNumber          ,
                                          dwDisp                     );
                    }
                    else
                    {
                        iCurr += wsprintf ( g_szBuff + iCurr       ,
                                            _T ( "%s, line %04d" ) ,
                                            g_stLine.FileName      ,
                                            g_stLine.LineNumber     ) ;
                    }
                }
            }
        }

        szRet = g_szBuff ;
    }
    __except ( EXCEPTION_EXECUTE_HANDLER )
    {
        szRet = NULL ;
    }
    return ( szRet ) ;
}

//=====================================================================================//
//                         LPCTSTR GetFirstStackTraceString()                          //
//=====================================================================================//
LPCTSTR GetFirstStackTraceString(DWORD dwOpts, EXCEPTION_POINTERS *pExPtrs)
{
    // All the error checking is in the InternalGetStackTraceString
    // function.

    // Initialize the STACKFRAME structure.
    ZeroMemory ( &g_stFrame , sizeof ( STACKFRAME ) ) ;

    g_stFrame.AddrPC.Offset       = pExPtrs->ContextRecord->Eip ;
    g_stFrame.AddrPC.Mode         = AddrModeFlat                ;
    g_stFrame.AddrStack.Offset    = pExPtrs->ContextRecord->Esp ;
    g_stFrame.AddrStack.Mode      = AddrModeFlat                ;
    g_stFrame.AddrFrame.Offset    = pExPtrs->ContextRecord->Ebp ;
    g_stFrame.AddrFrame.Mode      = AddrModeFlat                ;

    return ( InternalGetStackTraceString ( dwOpts , pExPtrs ) ) ;
}

//=====================================================================================//
//                          LPCTSTR GetNextStackTraceString()                          //
//=====================================================================================//
LPCTSTR GetNextStackTraceString(DWORD dwOpts, EXCEPTION_POINTERS *pExPtrs)
{
    // All error checking is in InternalGetStackTraceString.
    // Assume that GetFirstStackTraceString has already initialized the
    // stack frame information.
    return (InternalGetStackTraceString(dwOpts, pExPtrs)) ;
}

//=====================================================================================//
//                                 bool initDbgHelp()                                  //
//=====================================================================================//
bool initDbgHelp()
{
	g_dbgHelp = LoadLibrary("dbghelp.dll");
	if(g_dbgHelp == 0) return false;

#	define MUST_SYMBOL(S) { if((g_##S = (S##_t)GetProcAddress(g_dbgHelp,#S))==0) return false; }
#	define MAY_SYMBOL(S) { g_##S = (S##_t)GetProcAddress(g_dbgHelp,#S); }
	SYMBOLS_LIST;
#	undef MUST_SYMBOL
#	undef MAY_SYMBOL

	return true;
}

//=====================================================================================//
//                                 void shutDbgHelp()                                  //
//=====================================================================================//
void shutDbgHelp()
{
#	define MUST_SYMBOL(S) { g_##S = 0; }
#	define MAY_SYMBOL(S) { g_##S = 0; }
	SYMBOLS_LIST;
#	undef MUST_SYMBOL
#	undef MAY_SYMBOL

	FreeLibrary(g_dbgHelp);
}

}

//=====================================================================================//
//                               void SymEngine::init()                                //
//=====================================================================================//
void SymEngine::init()
{
    if(!g_bSymEngInit++)
    {
		if(!initDbgHelp()) return;

        // Set up the symbol engine.
        DWORD dwOpts = g_SymGetOptions();

        // Turn on line loading and deferred loading.
        g_SymSetOptions(dwOpts|SYMOPT_DEFERRED_LOADS|SYMOPT_LOAD_ANYTHING) ;

        // Force the invade process flag no matter what operating system
        // I'm on.
        HANDLE hPID = (HANDLE)GetCurrentProcessId() ;
		BSUSymInitialize((DWORD)hPID,hPID,NULL,TRUE);
    }
}

//=====================================================================================//
//                               void SymEngine::shut()                                //
//=====================================================================================//
void SymEngine::shut()
{
    if(!--g_bSymEngInit)
    {
        if(good())
		{
			g_SymCleanup((HANDLE)GetCurrentProcessId());
			shutDbgHelp();
		}
    }
}

//=====================================================================================//
//                               bool SymEngine::good()                                //
//=====================================================================================//
bool SymEngine::good()
{
	if(g_dbgHelp == 0) return false;
#	define MUST_SYMBOL(S) {if(g_##S == 0) return false;}
#	define MAY_SYMBOL(S) {}
	SYMBOLS_LIST;
#	undef MUST_SYMBOL
#	undef MAY_SYMBOL
	return true;
}

//=====================================================================================//
//                              BOOL SymEngine::Cleanup()                              //
//=====================================================================================//
BOOL SymEngine::Cleanup(HANDLE h)
{
	return g_SymCleanup(h);
}

BOOL SymEngine::StackWalk	(
						DWORD MachineType, 
						HANDLE hProcess, 
						HANDLE hThread, 
						LPSTACKFRAME StackFrame, 
						PVOID ContextRecord, 
						PREAD_PROCESS_MEMORY_ROUTINE ReadMemoryRoutine,  
						PFUNCTION_TABLE_ACCESS_ROUTINE FunctionTableAccessRoutine,
						PGET_MODULE_BASE_ROUTINE GetModuleBaseRoutine, 
						PTRANSLATE_ADDRESS_ROUTINE TranslateAddress 
							)
{
	return g_StackWalk(MachineType, 
				hProcess, 
				hThread, 
				StackFrame, 
				ContextRecord, 
				ReadMemoryRoutine,  
				FunctionTableAccessRoutine,
				GetModuleBaseRoutine, 
				TranslateAddress);

}

//=====================================================================================//
//                           bool SymEngine::canSetContext()                           //
//=====================================================================================//
bool SymEngine::canSetContext()
{
	return g_SymSetContext != 0;
}

//=====================================================================================//
//                            BOOL SymEngine::SetContext ()                            //
//=====================================================================================//
BOOL SymEngine::SetContext	(
				HANDLE hProcess,
				PIMAGEHLP_STACK_FRAME StackFrame,
				PIMAGEHLP_CONTEXT Context
					)
{
	return g_SymSetContext(
				hProcess,
				StackFrame,
				Context
				);
}

//=====================================================================================//
//                           BOOL SymEngine::EnumSymbols ()                            //
//=====================================================================================//
BOOL SymEngine::EnumSymbols	(
						HANDLE  hProcess,
						ULONG64  BaseOfDll,
						PCSTR  Mask,
						PSYM_ENUMERATESYMBOLS_CALLBACK  EnumSymbolsCallback,
						PVOID  UserContext
							)
{
	return g_SymEnumSymbols	(
						hProcess,
						BaseOfDll,
						Mask,
						EnumSymbolsCallback,
						UserContext
							);

}

//=====================================================================================//
//                            BOOL SymEngine::Initialize ()                            //
//=====================================================================================//
BOOL SymEngine::Initialize	(
				HANDLE hProcess,     
				PSTR UserSearchPath,  
				BOOL fInvadeProcess  
					)
{
	return g_SymInitialize(
				hProcess,     
				UserSearchPath,  
				fInvadeProcess
				);
}

//=====================================================================================//
//                            DWORD SymEngine::LoadModule()                            //
//=====================================================================================//
DWORD SymEngine::LoadModule(
				HANDLE hProcess,  
				HANDLE hFile,     
				PSTR ImageName,  
				PSTR ModuleName, 
				DWORD BaseOfDll,  
				DWORD SizeOfDll   
					)
{
	return g_SymLoadModule(
				hProcess,  
				hFile,     
				ImageName,  
				ModuleName, 
				BaseOfDll,  
				SizeOfDll
				);
}

//=====================================================================================//
//                          BOOL SymEngine::GetLineFromAddr()                          //
//=====================================================================================//
BOOL SymEngine::GetLineFromAddr(
					HANDLE hProcess,
					DWORD dwAddr,
					PDWORD pdwDisplacement,
					PIMAGEHLP_LINE Line
						)
{
	return g_SymGetLineFromAddr(
					hProcess,
					dwAddr,
					pdwDisplacement,
					Line
					);
}

//=====================================================================================//
//                          BOOL SymEngine::GetSymFromAddr()                           //
//=====================================================================================//
BOOL SymEngine::GetSymFromAddr(
					HANDLE hProcess,             
					DWORD Address,               
					PDWORD Displacement,       
					PIMAGEHLP_SYMBOL Symbol  
						)
{
	return g_SymGetSymFromAddr(
					hProcess,             
					Address,               
					Displacement,       
					Symbol
					);
}

//=====================================================================================//
//                          DWORD SymEngine::GetModuleBase()                           //
//=====================================================================================//
DWORD SymEngine::GetModuleBase(HANDLE hProcess, DWORD dwAddr)
{
	return g_SymGetModuleBase(hProcess, dwAddr);
}

//=====================================================================================//
//                  const char *SymEngine::getFirstStackTraceString()                  //
//=====================================================================================//
const char *SymEngine::getFirstStackTraceString(DWORD dwOpts, EXCEPTION_POINTERS *ptrs)
{
	return GetFirstStackTraceString(dwOpts,ptrs);
}

//=====================================================================================//
//                  const char *SymEngine::getNextStackTraceString()                   //
//=====================================================================================//
const char *SymEngine::getNextStackTraceString(DWORD dwOpts, EXCEPTION_POINTERS *ptrs)
{
	return GetNextStackTraceString(dwOpts,ptrs);
}

//=====================================================================================//
//                   const STACKFRAME *SymEngine::getCurrentFrame()                    //
//=====================================================================================//
const STACKFRAME *SymEngine::getCurrentFrame()
{
	return &g_stFrame;
}
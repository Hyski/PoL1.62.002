/***********************************************************************

                             Virtuality

                       Copyright by MiST land 2000

************************************************************************/
#include "precomp.h"
#include <common/shell/shell.h>
#include "globals.h"

#include <malloc.h>
#include <d3dxcore.h>
#include <mll/debug/callstack.h>
#include <mll/debug/reporter.h>

#include <logic2/logictypes.h>
#include <logic_ex/ActMgr.h>

//#ifdef _HOME_VERSION
//#	include <CppUnit/TextOutputter.h>
//#	include <CppUnit/Ui/Text/TestRunner.h>
//#	include <CppUnit/TestResultCollector.h>
//#endif

//#if !defined(_DEBUG)
//#	pragma comment(lib,"CryptMark.lib")
//#	include "CryptMark/CryptMark.h"
//#endif

namespace WinMainDetails
{

struct Cleaner
{
	~Cleaner()
	{
		PoL::ActMgr::cleanup();
	}
};

}

using namespace WinMainDetails;

//=====================================================================================//
//                              static bool HasDataDir()                               //
//=====================================================================================//
static bool HasDataDir()
{
	_finddata_t fd;
	intptr_t fh1 = _findfirst("Data",&fd);
	if(fh1 != -1) _findclose(fh1);
	intptr_t fh2 = _findfirst("Maps",&fd);
	if(fh2 != -1) _findclose(fh2);
	return fh1 != -1 || fh2 != -1;
}

//=====================================================================================//
//                          static void SetValidCurrentDir()                           //
//=====================================================================================//
static void SetValidCurrentDir()
{
	DWORD size = GetCurrentDirectory(0,NULL);
	std::vector<char> buf(size);
	GetCurrentDirectory(size,&buf[0]);
	std::string cdir(&buf[0]);

	while(!HasDataDir())
	{
		size_t pos = cdir.rfind('\\');
		if(pos == std::string::npos)
		{
			break;
		}
		else
		{
			cdir = cdir.substr(0,pos);
		}
        
		SetCurrentDirectory(cdir.c_str());
	}

}

#define AppGo GameAppGo
//#define AppGo TestAppGo

//=====================================================================================//
//                               static void TestAppGo()                               //
//=====================================================================================//
static void TestAppGo()
{
	SetValidCurrentDir();

	Cleaner cleaner;
	DataMgr::Initialize();
	PoL::ActMgr::reset();
}

//=====================================================================================//
//                                 static void AppGo()                                 //
//=====================================================================================//
static void GameAppGo()
{
	SetValidCurrentDir();

	Cleaner cleaner;

	bool changeFpuState = true
#ifdef _HOME_VERSION
		&& (std::string(GetCommandLine()).find("-fpre") == std::string::npos)
#endif
	;

	WORD wOldCW;
	BOOL bChangedFPCW;

	if(changeFpuState)
	{
		bChangedFPCW = MungeFPCW(&wOldCW);
	}

	D3DXInitialize();
	Shell().Go();
	D3DXUninitialize();

	if(changeFpuState && bChangedFPCW)
	{
		RestoreFPCW(wOldCW);
	}
}

//=====================================================================================//
//                                int WINAPI WinMain()                                 //
//=====================================================================================//
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)
{
#ifdef _HOME_VERSION
	if(std::string(lpCmdLine).find("-setaffmask") != std::string::npos)
	{
		SetProcessAffinityMask(GetCurrentProcess(),1);
	}
#else
	mll::debug::reporter::set_parameters("",false);
#endif

//#ifdef _HOME_VERSION
//	if(std::string(lpCmdLine).find("-nounit") == std::string::npos)
//	{
//		CppUnit::TestResultCollector coll;
//		std::ostringstream sstr;
//		sstr << "Unit test failed!\n";
//		CppUnit::TextUi::TestRunner runner(new CppUnit::TextOutputter(&coll,sstr));
//		if(!runner.run())
//		{
//			MessageBox(NULL,sstr.str().c_str(), "UnderCover", MB_OK|MB_ICONEXCLAMATION);
//			return 1;
//		}
//	}
//#endif

	if(IsDebuggerPresent())
	{
		AppGo();
	}
	else
	{
		try
		{
			AppGo();
		}
		catch(CasusImprovisus& ci)
		{
			MessageBox(0,
					ci.what(),
					"Power Of Law: Missions Exception",
					MB_OK|MB_ICONERROR);
		}
	}

	return 0;
}

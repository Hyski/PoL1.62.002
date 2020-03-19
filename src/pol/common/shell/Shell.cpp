/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description:
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)

************************************************************************/
#include "Precomp.h"
#include "..\D3DApp\GammaControl\GammaControl.h"
#include "..\GraphPipe\GraphPipe.h"
#include "..\TextureMgr\TextureMgr.h"
#include "..\DataMgr\DataMgr.h"
#include "..\FontMgr\FontMgr.h"
#include <undercover/Game.h>
#include <undercover/Options/Options.h>
#include <undercover/Options/PCOptions.h>
#include <undercover/resource.h>
#include <logic2/GameLogic.h>
#include "..\CmdLine\CmdLine.h"
#include <undercover/Options/XRegistry.h>
#include "../SurfaceMgr/SurfaceMgr.h"
#include "../BinkMgr/BinkMgr.h"
#include "../utils/profiler.h"
#include <undercover/ModMgr.h>
#include "Shell.h"
#include <mmsystem.h>
#include <undercover/IWorld.h>
#include <undercover/GameLevel/GameLevel.h>
#include <undercover/Skin/AnimaLibrary.h>
#include "../GraphPipe/statesmanager.h"
#include <undercover/HmStatusTracker.h>
#include <undercover/skin/skanim.h>

#include <undercover/interface/LoadingScreen.h>
#include <undercover/interface/GameScreen.h>
#include <undercover/interface/MenuMgr.h>

#include "../Utils/Dir.h"
#include <Muffle/ISound.h>
#include "../gsound/filesystem.h"
#include <logic2/Invariants.h>
#include "../D3DApp/Input/Input.h"

#include <logic2/logictypes.h>
#include <logic_ex/ActMgr.h>

#include <Bink.h>

//---------- ��� ���� ------------
//#define _DEBUG_SHELL
#ifdef _DEBUG_SHELL
CLog shell_log;
#define shell	shell_log["shell.log"]
#else
#define shell	/##/
#endif


namespace ShellDetails
{

static const std::string g_scriptsBasePath = "scripts\\sound\\";
static const std::string g_soundBasePath = "";

//=====================================================================================//
//                                 struct SndServices                                  //
//=====================================================================================//
class SndServicesImpl : public Muffle::SndServices
{
public:
	HWND m_hwnd;
	Muffle::ci_VFileSystem *m_fs;

	SndServicesImpl() : m_hwnd(NULL), m_fs(0) {}
	/// �������� ������� ���� �� �������� ��������, ����. "/scripts/sound/"
	virtual const std::string& getScriptBasePath() { return g_scriptsBasePath; }
	/// �������� ������� ���� �� ������, ����. "/sounds/"
	virtual const std::string& getSoundBasePath() { return g_soundBasePath; }
	///	������� ���������� ����������
	virtual void dbg_printf(short x,short y,const char *s) { DebugInfo::Add(x,y,"%s",s); }
	///	�������� �������� �������
	virtual Muffle::ci_VFileSystem* fs() { return m_fs; }
	///	�������� ���������� ����
	virtual HWND window() { return m_hwnd; }
};

static SndServicesImpl svc;

// ������������ ��������������� ��������
class DS : public DirectoryScanner
{
	bool OnItem(const Item &it)
	{
		if(it.Ext==".skel")
		{
			AnimaLibrary::GetInst()->GetSkAnimation(it.FullName);
		}
		else if(it.Ext==".skin")
		{
			SkSkin *s=AnimaLibrary::GetInst()->GetSkSkin(it.FullName);
			delete s;
		}
		AnimaLibrary::GetInst()->Clear();
		return true;
	};
};

//=====================================================================================//
//                              void ConvertAnimations()                               //
//=====================================================================================//
void ConvertAnimations(void)
{
	if(CmdLine::IsKey("-convert"))
	{
		DS d;
		d.SetPath("animations","*.skin;*.skel");
		throw CasusImprovisus("Animations converted.");
	}
}

//=====================================================================================//
//                                  void CheckActs()                                   //
//=====================================================================================//
void CheckActs()
{
	if(CmdLine::IsKey("-at"))
	{
		PoL::ActMgr::reset();
		throw CASUS("Acts checked.");
	}
}

}

using namespace ShellDetails;

int ShellFrameCounter = 0;

//=====================================================================================//
//                               class Shell::FPSometer                                //
//=====================================================================================//
class Shell::FPSometer
{
private:
	enum {NUMOFFRAME=25};
	float m_LastTime;
	float m_TimeQueue[NUMOFFRAME];
	int m_EndQueue;
public:
	FPSometer() : m_LastTime(0),m_EndQueue(0)
	{
		memset(m_TimeQueue,NUMOFFRAME*sizeof(float),0);
	}
	virtual ~FPSometer() {}
public:
	//	���������� ������� �����
	void SetTime(float seconds)
	{
		for(int i=0;i<NUMOFFRAME;i++) m_TimeQueue[i] = seconds-m_LastTime;
		m_LastTime = seconds;
	}
	//	�������� ����� ��� ��������
	void AddTime(float seconds)
	{
	    m_TimeQueue[m_EndQueue++] = seconds-m_LastTime;
		if(m_EndQueue == NUMOFFRAME) m_EndQueue = 0;
		m_LastTime = seconds;    
	}
	//	�������� ������� ���-�� ������ � �������
	float FPS(void) const
	{
		float sum = 0;
		for(int i=0;i<NUMOFFRAME;i++) sum += m_TimeQueue[i];

		return static_cast<float>(NUMOFFRAME)/sum;
	}
};

//=====================================================================================//
//                                  class MutexHandle                                  //
//=====================================================================================//
class MutexHandle
{
	HANDLE m_handle;

public:
	MutexHandle() : m_handle(CreateMutex(NULL,FALSE,"CheckExistInstanceForPowerOfLaw"))
	{
		if(m_handle == NULL)
		{
			throw CASUS("Unknown error occured: cannot create mutex object");
		}
	}

	~MutexHandle()
	{
		CloseHandle(m_handle);
	}

	HANDLE getHandle() const { return m_handle; }
};

//=====================================================================================//
//                               class SingleRunProcess                                //
//=====================================================================================//
class SingleRunProcess
{
	MutexHandle m_handle;	///< ������� ��� ������������ ���������� �������

public:
	SingleRunProcess()
	{
		DWORD result = WaitForSingleObject(m_handle.getHandle(),0);

		switch(result)
		{
			case WAIT_TIMEOUT:
				throw CASUS("You've already started a game instance");
			case WAIT_FAILED:
				throw CASUS("WaitForSingleObject failed to unknown reason");
		}
	}

	~SingleRunProcess()
	{
		ReleaseMutex(m_handle.getHandle());
	}
};

//=====================================================================================//
//                                 struct ShellPreInit                                 //
//=====================================================================================//
struct ShellPreInit
{
	ShellPreInit()
	{
		CodeProfiler::Init();

		//	������ ��������� ������
		CmdLine::Init();

		//	������������� �������
		Timer::Init();

		//	������ ����� ����� � �������
		Options::Init();

		//	��������������� ������������ ����������
		PCOptions::Register();

		//	������������� DataMgr
		InitDataMgr();

		// ��������� �������� �����
		CheckActs();

		// ��������� ��������������� ��������
		ConvertAnimations();

		//BinkSoundUseDirectSound(0);

		// ����������������� ������� ���������
		PoL::Inv::init();
	}

	~ShellPreInit()
	{
		// ���������� DataMgr
		DataMgr::Uninitialize();

		// ��������� �����
		Options::Close();
	}

	void InitDataMgr()
	{
		//	�������������� DataMgr
		DataMgr::Initialize();

		//	��������� ���� � �������� �� ���������
		DataMgr::ReadPacks2("Data", DataMgr::ppNormal);

		//	��������� ���� �� ���������� ����
		const std::string mod_name = Options::GetString("game.mod");
		if(!mod_name.empty())
		{
			//	��������� �� ������������ ���������� � ����
			if(Mod(mod_name).isValid())
			{
				DataMgr::ReadPacks2((ModMgr::m_modPath+mod_name).c_str(),DataMgr::ppMod);
			}
		}

		if(!Game::GetAddonName().empty())
		{
			DataMgr::ReadPacks2(Game::GetAddonName().c_str(),DataMgr::ppAddon);
		}

		//	��������� ���� �� ��������� ��������� � �������
		const char* path = Options::Registry()->Var("Packs Path").GetString().c_str();
		const char* next;
		char buff[MAX_PATH];

		while(*path)
		{
			int len = strlen(path);
			if(next = strchr(path,';')) len -= strlen(next);
			strncpy(buff,path,len);
			path += len;
			if(len && (buff[len-1] == '/') || (buff[len-1] == '\\')) len--;
			buff[len] = 0;
			DataMgr::ReadPacks2(buff, DataMgr::ppNormal);
			if(next) path++;
		}
	}
};

//=====================================================================================//
//                               struct Shell::Container                               //
//=====================================================================================//
struct Shell::Container : public ShellPreInit
{
	/// ����� ��������� ������ ���� ������� � �����
	SingleRunProcess m_srp;

	//	�������� FPS
	FPSometer m_FPSometer;
	//	������ ����������
	DDFrame m_Frame;
	//	������ ������ �������
	GraphPipe m_GraphPipe;

	Container() : m_Frame(m_AppName,IDI_MAIN_ICON) {}
};

//*********************************************************************//
// class Shell
Shell* Shell::m_Instance = 0;
const char* Shell::m_AppName = "Power Of Law: Missions";

#ifdef _HOME_VERSION
static bool g_homeNoSleep = false;
#endif

//=====================================================================================//
//                                   Shell::Shell()                                    //
//=====================================================================================//
Shell::Shell()
:	m_Container(new Container),
	m_Loop(PerformanceLoop),
	m_IsModalMode(false)
{
	//	��������� �� ��������� ������� ��� ������������
	m_Instance = this;

#ifdef _HOME_VERSION
	if(CmdLine::IsKey("-nosleep"))
	{
		g_homeNoSleep = true;
	}
#endif

	DDFrame::VideoMode vm = Frame()->CurrentVideoMode();
	if(!vm.IsValid())
	{
		if(Options::GetInt("system.video.windowed") && !CmdLine::IsKey("-fs"))
		{
			vm.m_Driver		= Options::GetInt("system.video.driver");
			vm.m_Device		= Options::GetInt("system.video.device");
			vm.m_Width		= 800;
			vm.m_Height		= 600;
			vm.m_Depth		= Options::GetInt("system.video.bpp");
			vm.m_Freq		= 0;
			vm.m_Mode		= DDFrame::VideoMode::M_WINDOWED;
		}
		else
		{
			vm.m_Driver		= Options::GetInt("system.video.driver");
			vm.m_Device		= Options::GetInt("system.video.device");
			vm.m_Width		= 800;
			vm.m_Height		= 600;
			vm.m_Depth		= Options::GetInt("system.video.bpp");
			vm.m_Freq		= Options::GetInt("system.video.freq");
			vm.m_Mode		= DDFrame::VideoMode::M_FULLSCREEN;
		}
	}

	Frame()->CreateHandle(vm.m_Mode);
	long r = BinkSoundUseDirectSound(0);
	//	---------------------------------------
	//	������������� ������ �����
	svc.m_hwnd = Frame()->Handle();
	cc_VFileSystem::initInstance();
	svc.m_fs = cc_VFileSystem::instance();
	Muffle::ISound::initInstance(&svc);
	shell("    ...sound init;\n");
	//	��������� ��������������� ��������� ���������� � ������������ ���������
	Game::PriorInterfaceInitialize();
	//	��������� ����� ������
	D3DKernel::SetTriple(Options::GetInt("system.video.triple"));
	//	������������� GammaControl'�
	GammaControl::Init(D3DKernel::GetPS());
	GammaControl::SetGamma(Options::GetFloat("system.video.gamma"));
	shell("    ....GammaControl::Init();\n");
	//	---------------------------------------
	//	������������� ����
	StatesManager::Clear();
	Game::Init(&m_Container->m_GraphPipe);
	shell("    ...init Game;\n");
	//	���������� ����
	m_Container->m_Frame.ShowWindow();

	shell("    ...show window;\n");
	//	��������� ���������������� ����
	Input::Mouse()->SetSensitivity(Options::GetFloat("system.input.sensitivity"));
	//	������������� �������
	TextureMgr::Instance()->Init(D3DKernel::GetD3DDevice(),
					 Options::GetInt("system.video.mipmap"),
					 Options::GetInt("system.video.texquality"),
					 Options::GetInt("system.video.usecompressing"),
					 false);
	shell("    ....TextureMgr::Instance()->Init(); \n");
	//	������������ GraphPipe
	m_Container->m_GraphPipe.Init(D3DKernel::GetD3D(),
								  D3DKernel::GetD3DDevice(),
								  D3DKernel::ResX(),
								  D3DKernel::ResY());

	shell("    ....m_pGraphPipe->Init();\n");
	//	--------------------------------------------
	m_Container->m_GraphPipe.RegisterShaderFile("shaders.sys/shaders.shader");
	//	--------------------------------------------
    //������������� AI ����
    GameLogic::Init();
    shell("    ... GameLogic init;\n");
	//	�������� ����������
	Game::CreateInterface();
    shell("    ... Game Interface init;\n");
	// ���������������� ����
	PoL::ActMgr::reset();
	//	������������� ����� FPS'������
	m_Container->m_FPSometer.SetTime(Timer::GetSeconds());
	//	������������� ����������
	m_Container->m_Frame.SetController(this);

	m_loops.push_back(this);

	shell("...finished;\n\n");
}

//=====================================================================================//
//                                   Shell::~Shell()                                   //
//=====================================================================================//
Shell::~Shell()
{
	shell("Shell:\n...start completion;\n");

	//	������������� ����������
	m_Container->m_Frame.SetController(0);
    //���������� AI 
    shell("    ...closing GameLogic;\n");
    GameLogic::Shut();
    shell("    ...closed GameLogic;\n");
	//	���������� ����� �� ����
	Options::Flush();
	shell("    ...flush options to disk;\n");
	Game::Close();
	shell("    ...close Game;\n");
	//	-------------------------------------
	GammaControl::Release();
	shell("    ...destroy Gamma Control;\n");
	//	-------------------------------------
	Muffle::ISound::shutInstance();
	cc_VFileSystem::shutInstance();
	shell("    ...Shutdown Sound;\n");
	//	������������ �������
	TextureMgr::Instance()->Release();
	shell("    ...release TextureMgr;\n");
	//	������� ������ �� ���������� - ��������� �� ������������
	m_Instance = 0;

	shell("...finished;\n");
}

//=====================================================================================//
//                                  void Shell::Go()                                   //
//=====================================================================================//
void Shell::Go(void)
{
	shell("Shell:\n...start the Process;\n");
	while(!needExit())
	{
		if(m_Loop == PerformanceLoop
#ifdef _HOME_VERSION
			|| CmdLine::IsKey("-noidle")
#endif
			)
		{
			normalThink();
			render();
			endFrame();
		}
		else if(m_Loop == IdleLoop)
		{
			idleThink();
		}
	}

	//while(!needExit()) (*this.*m_Loop)();
	shell("...finished;\n\n");
}

//static float getCurrentTime()
//{
//	LARGE_INTEGER time, freq;
//	QueryPerformanceCounter(&time);
//	QueryPerformanceFrequency(&freq);
//	return static_cast<float>(time.QuadPart)/static_cast<float>(freq.QuadPart);
//}

//struct PerfQuery
//{
//	std::string msg;
//	int delta;
//	float start;
//	PerfQuery(const std::string &s, int d) : delta(d), start(getCurrentTime()), msg(s) {}
//	~PerfQuery()
//	{
//		float t = getCurrentTime();
//		std::ostringstream sstr;
//		sstr << msg << " : " << t-start;
//		DebugInfo::Add(500,300+delta,const_cast<char *>(sstr.str().c_str()));
//	}
//};

//	���� ����������
//=====================================================================================//
//                            void Shell::PerformanceLoop()                            //
//=====================================================================================//
void Shell::PerformanceLoop(void)
{
	//	���������� �������
	Timer::Update();

	//	���������� �����
	Muffle::ISound::instance()->manage();

	Input::Update();
		
	//	������� ���� ����
	Game::Tick();

	//	��������� ����
	Game::Render();

	//	����� �����
	D3DKernel::UpdateFrame();

	//	������������ ������� ���������
	m_Container->m_Frame.Tick();
	//	�������� ��������� ����
	m_Container->m_Frame.CheckMouse();

	//	����������� �� fps'��
	if(m_Container->m_FPSometer.FPS()>100)
	{
		int sleep = (0.015f-(1.0f/m_Container->m_FPSometer.FPS()))*1000;
		if(sleep)
		{
			DebugInfo::Add(235,5,"Sleep(%d)",sleep);
			Sleep(sleep);
		}
	}

	//	�������� fps'�
	m_Container->m_FPSometer.AddTime(Timer::GetSeconds());
}

//=====================================================================================//
//                               void Shell::IdleLoop()                                //
//=====================================================================================//
//	���� ��������
void Shell::IdleLoop(void)
{
	//	������������ ������� ���������
	m_Container->m_Frame.Wait();
}

//=====================================================================================//
//                                void Shell::OnClose()                                //
//=====================================================================================//
//	������� [����� �� ����������]
void Shell::OnClose(void)
{
	CodeProfiler::Close();
	Exit();
}

//=====================================================================================//
//                              void Shell::OnActivate()                               //
//=====================================================================================//
void Shell::OnActivate(void)
{
	DDFrame::VideoMode vm = m_Container->m_Frame.CurrentVideoMode();

	m_Loop = PerformanceLoop;
	if(vm.m_Mode == DDFrame::VideoMode::M_FULLSCREEN)
	{
		m_Container->m_Frame.SetVideoMode(DDFrame::VideoMode());
		m_Container->m_Frame.SetVideoMode(vm);
//		m_Container->m_Frame.ShowWindow();
	}
	//	������ �������
	Timer::Resume();

	shell("   on activated;\n");
}

//=====================================================================================//
//                             void Shell::OnDeactivate()                              //
//=====================================================================================//
void Shell::OnDeactivate(void)
{
	m_Loop = IdleLoop;
	//	��������������� �������
	Timer::Suspend();
	shell("   on deactivated;\n");
}

//=====================================================================================//
//                        void Shell::OnStartChangeVideoMode()                         //
//=====================================================================================//
void Shell::OnStartChangeVideoMode(void)
{
	shell("   start change Video Mode;\n");

	//	��������������� �������
	Timer::Suspend();
	//	������������ �������� GraphPipe
	m_Container->m_GraphPipe.Release();
	//	���������� �������� 2D ������������
	SurfaceMgr::Instance()->Release();
	//	������������ ������
	BinkMgr::Instance()->Release();
	//	������������ �������
	FontMgr::Instance()->Release();
	//	������������ �������
	TextureMgr::Instance()->Release();
	//	������� �������
	FontMgr::Instance()->Release();
	//	������������ GammaControl'�
	GammaControl::Release();

	if(IWorld::Get() && IWorld::Get()->GetLevel())
	{
		IWorld::Get()->GetLevel()->OnStartChangeVideoMode();
	}
}

//=====================================================================================//
//                        void Shell::OnFinishChangeVideoMode()                        //
//=====================================================================================//
void Shell::OnFinishChangeVideoMode(void)
{
	//	���������� ����
	m_Container->m_Frame.ShowWindow();
	//	������������� �������
	TextureMgr::Instance()->Init(D3DKernel::GetD3DDevice(),
								 Options::GetInt("system.video.mipmap"),
								 Options::GetInt("system.video.texquality"),
								 Options::GetInt("system.video.usecompressing"),
								 false);
	shell("    ....TextureMgr::Instance()->Init(); \n");
	//	������������ GraphPipe
	m_Container->m_GraphPipe.Init(D3DKernel::GetD3D(),
								  D3DKernel::GetD3DDevice(),
								  D3DKernel::ResX(),
								  D3DKernel::ResY());
	shell("    ....GraphPipe->Init();\n");
	//	������������� ����
	Game::Tune();
	shell("    ....Game::Tune();\n");
	//	������������� �����-��������
	GammaControl::Init(D3DKernel::GetPS());
	shell("    ....GammaControl::Init();\n");

	if(IWorld::Get() && IWorld::Get()->GetLevel())
	{
		IWorld::Get()->GetLevel()->OnFinishChangeVideoMode();
	}

	//	������ �������
	Timer::Resume();

	shell("   finish change Video Mode;\n");
}

//=====================================================================================//
//                              float Shell::FPS() const                               //
//=====================================================================================//
//	�������� ���-�� ������ � �������
float Shell::FPS(void) const
{
	return m_Container->m_FPSometer.FPS();
}

//=====================================================================================//
//                            struct ModalLoopParentSetter                             //
//=====================================================================================//
struct ModalLoopParentSetter
{
	typedef std::list<ModalLoop*> Loops_t;
	Loops_t *m_loops;

	ModalLoopParentSetter(ModalLoop *l, Loops_t *loops)
	:	m_loops(loops)
	{
		l->noExit();
		l->setParent(m_loops->back());
		m_loops->push_back(l);
	}

	~ModalLoopParentSetter()
	{
		m_loops->back()->setParent(0);
		m_loops->pop_back();
	}
};

//=====================================================================================//
//                                void Shell::DoModal()                                //
//=====================================================================================//
//	����� � ��������� ����
void Shell::DoModal(ModalLoop *loop)
{
	if(std::find(m_loops.begin(),m_loops.end(),loop) != m_loops.end())
	{
		return;
	}

	ModalLoopParentSetter setter(loop,&m_loops);

	while(!loop->needExit())
	{
		if(m_Loop == PerformanceLoop)
		{
			loop->normalThink();
			loop->render();
			loop->endFrame();
		}
		else if(m_Loop == IdleLoop)
		{
			loop->idleThink();
		}
	}
	
	//if(m_IsModalMode) throw CASUS("������� ����� � ��������� ���� ������ ���.");
	//m_IsModalMode = true;
	//while(!m_Exit) (*this.*m_Loop)();
	//m_Exit = false;
	//m_IsModalMode = false;
}

//=====================================================================================//
//                             bool Shell::IsModal() const                             //
//=====================================================================================//
//	���������: �������� �� ��������� �����
bool Shell::IsModal(void) const
{
	return m_loops.size() > 1;
	//return m_IsModalMode;
}

//=====================================================================================//
//                               DDFrame* Shell::Frame()                               //
//=====================================================================================//
//	������� ��������� �� ������ ����������
DDFrame* Shell::Frame(void)
{
	return &m_Container->m_Frame;
}

//=====================================================================================//
//                       GraphPipe* Shell::GetGraphPipe() const                        //
//=====================================================================================//
//	�������� ��������� �� GraphPipe
GraphPipe* Shell::GetGraphPipe(void)
{
	return &m_Container->m_GraphPipe;
}


//=====================================================================================//
//                              void ModalLoop::render()                               //
//=====================================================================================//
void ModalLoop::render()
{
	static PoL::HmStatusTracker hst;

	if(hst.needActivities())
	{
		//	��������� ����
		Game::Render();
		//	����� �����
		D3DKernel::UpdateFrame();
	}
}

//=====================================================================================//
//                               void ModalLoop::think()                               //
//=====================================================================================//
void ModalLoop::think()
{
}

bool g_noSleep = false;

//=====================================================================================//
//                             void ModalLoop::endFrame()                              //
//=====================================================================================//
void ModalLoop::endFrame()
{
	//	������������ ������� ���������
	Shell::Instance()->m_Container->m_Frame.Tick();
	//	�������� ��������� ����
	Shell::Instance()->m_Container->m_Frame.CheckMouse();

	//	����������� �� fps'��
	if(
#ifdef _HOME_VERSION
		!g_homeNoSleep &&
#endif
		!g_noSleep && Shell::Instance()->m_Container->m_FPSometer.FPS() > 100)
	{
		int sleep = (0.015f-(1.0f/Shell::Instance()->m_Container->m_FPSometer.FPS()))*1000;
		if(sleep)
		{
			DebugInfo::Add(235,5,"Sleep(%d)",sleep);
			Sleep(sleep);
		}
	}

	//	�������� fps'�
	Shell::Instance()->m_Container->m_FPSometer.AddTime(Timer::GetSeconds());
}

//=====================================================================================//
//                            void ModalLoop::normalThink()                            //
//=====================================================================================//
void ModalLoop::normalThink()
{
	//	���������� �������
	Timer::Update();

	//	���������� �����
	Muffle::ISound::instance()->manage();

	Input::Update();
		
	//	������� ���� ����
	Game::Tick();
}

//=====================================================================================//
//                             void ModalLoop::idleThink()                             //
//=====================================================================================//
void ModalLoop::idleThink()
{
	//	������������ ������� ���������
	Shell::Instance()->m_Container->m_Frame.Wait();
}

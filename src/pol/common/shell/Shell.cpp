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

//---------- Лог файл ------------
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
	/// получить базовый путь до звуковых скриптов, напр. "/scripts/sound/"
	virtual const std::string& getScriptBasePath() { return g_scriptsBasePath; }
	/// получить базовый путь до звуков, напр. "/sounds/"
	virtual const std::string& getSoundBasePath() { return g_soundBasePath; }
	///	вывести отладочную информацию
	virtual void dbg_printf(short x,short y,const char *s) { DebugInfo::Add(x,y,"%s",s); }
	///	получить файловую систему
	virtual Muffle::ci_VFileSystem* fs() { return m_fs; }
	///	получить дескриптор окна
	virtual HWND window() { return m_hwnd; }
};

static SndServicesImpl svc;

// опциональное конвертирование анимаций
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
	//	установить текущее время
	void SetTime(float seconds)
	{
		for(int i=0;i<NUMOFFRAME;i++) m_TimeQueue[i] = seconds-m_LastTime;
		m_LastTime = seconds;
	}
	//	добавить время для подсчета
	void AddTime(float seconds)
	{
	    m_TimeQueue[m_EndQueue++] = seconds-m_LastTime;
		if(m_EndQueue == NUMOFFRAME) m_EndQueue = 0;
		m_LastTime = seconds;    
	}
	//	получить среднее кол-во кадров в секунду
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
	MutexHandle m_handle;	///< Мьютекс для отслеживания повторного запуска

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

		//	разбор командной строки
		CmdLine::Init();

		//	инициализация таймера
		Timer::Init();

		//	чтение файла опций и реестра
		Options::Init();

		//	регистрирование опциональных переменных
		PCOptions::Register();

		//	инициализация DataMgr
		InitDataMgr();

		// Выполнить проверку актов
		CheckActs();

		// Выполнить конвертирование анимаций
		ConvertAnimations();

		//BinkSoundUseDirectSound(0);

		// Проинициализируем игровые константы
		PoL::Inv::init();
	}

	~ShellPreInit()
	{
		// Уничтожаем DataMgr
		DataMgr::Uninitialize();

		// закрываем опции
		Options::Close();
	}

	void InitDataMgr()
	{
		//	инициализируем DataMgr
		DataMgr::Initialize();

		//	считываем паки в катологе по умолчанию
		DataMgr::ReadPacks2("Data", DataMgr::ppNormal);

		//	считываем паки из указанного мода
		const std::string mod_name = Options::GetString("game.mod");
		if(!mod_name.empty())
		{
			//	проверяем на правильность информации в моде
			if(Mod(mod_name).isValid())
			{
				DataMgr::ReadPacks2((ModMgr::m_modPath+mod_name).c_str(),DataMgr::ppMod);
			}
		}

		if(!Game::GetAddonName().empty())
		{
			DataMgr::ReadPacks2(Game::GetAddonName().c_str(),DataMgr::ppAddon);
		}

		//	считываем паки из каталогов указанных в реестре
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
	/// Можем запустить только один процесс с игрой
	SingleRunProcess m_srp;

	//	замерщик FPS
	FPSometer m_FPSometer;
	//	каркас приложения
	DDFrame m_Frame;
	//	модуль вывода графики
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
	//	указатель на экземпляр объекта уже действителен
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
	//	инициализация работы звука
	svc.m_hwnd = Frame()->Handle();
	cc_VFileSystem::initInstance();
	svc.m_fs = cc_VFileSystem::instance();
	Muffle::ISound::initInstance(&svc);
	shell("    ...sound init;\n");
	//	выполняем предварительную настройку интерфейса и проигрывание мультиков
	Game::PriorInterfaceInitialize();
	//	установка трипл буфера
	D3DKernel::SetTriple(Options::GetInt("system.video.triple"));
	//	инициализация GammaControl'а
	GammaControl::Init(D3DKernel::GetPS());
	GammaControl::SetGamma(Options::GetFloat("system.video.gamma"));
	shell("    ....GammaControl::Init();\n");
	//	---------------------------------------
	//	инициализация игры
	StatesManager::Clear();
	Game::Init(&m_Container->m_GraphPipe);
	shell("    ...init Game;\n");
	//	показываем окно
	m_Container->m_Frame.ShowWindow();

	shell("    ...show window;\n");
	//	установка чувствительности мыши
	Input::Mouse()->SetSensitivity(Options::GetFloat("system.input.sensitivity"));
	//	инициализация текстур
	TextureMgr::Instance()->Init(D3DKernel::GetD3DDevice(),
					 Options::GetInt("system.video.mipmap"),
					 Options::GetInt("system.video.texquality"),
					 Options::GetInt("system.video.usecompressing"),
					 false);
	shell("    ....TextureMgr::Instance()->Init(); \n");
	//	иницилизация GraphPipe
	m_Container->m_GraphPipe.Init(D3DKernel::GetD3D(),
								  D3DKernel::GetD3DDevice(),
								  D3DKernel::ResX(),
								  D3DKernel::ResY());

	shell("    ....m_pGraphPipe->Init();\n");
	//	--------------------------------------------
	m_Container->m_GraphPipe.RegisterShaderFile("shaders.sys/shaders.shader");
	//	--------------------------------------------
    //инициализация AI игры
    GameLogic::Init();
    shell("    ... GameLogic init;\n");
	//	создание интерфейса
	Game::CreateInterface();
    shell("    ... Game Interface init;\n");
	// Инициализировать акты
	PoL::ActMgr::reset();
	//	устанавливаем время FPS'ометра
	m_Container->m_FPSometer.SetTime(Timer::GetSeconds());
	//	устанавливаем контроллер
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

	//	устанавливаем контроллер
	m_Container->m_Frame.SetController(0);
    //разрущение AI 
    shell("    ...closing GameLogic;\n");
    GameLogic::Shut();
    shell("    ...closed GameLogic;\n");
	//	сбрасываем опции на диск
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
	//	освобождение текстур
	TextureMgr::Instance()->Release();
	shell("    ...release TextureMgr;\n");
	//	объекта больше не существует - указатель не действителен
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

//	цикл исполнения
//=====================================================================================//
//                            void Shell::PerformanceLoop()                            //
//=====================================================================================//
void Shell::PerformanceLoop(void)
{
	//	обновление таймера
	Timer::Update();

	//	обновление звука
	Muffle::ISound::instance()->manage();

	Input::Update();
		
	//	переход хода игре
	Game::Tick();

	//	отрисовка игры
	Game::Render();

	//	смена кадра
	D3DKernel::UpdateFrame();

	//	обрабатываем оконные сообщения
	m_Container->m_Frame.Tick();
	//	проверка положения мыши
	m_Container->m_Frame.CheckMouse();

	//	выравниваем по fps'ам
	if(m_Container->m_FPSometer.FPS()>100)
	{
		int sleep = (0.015f-(1.0f/m_Container->m_FPSometer.FPS()))*1000;
		if(sleep)
		{
			DebugInfo::Add(235,5,"Sleep(%d)",sleep);
			Sleep(sleep);
		}
	}

	//	замеряем fps'ы
	m_Container->m_FPSometer.AddTime(Timer::GetSeconds());
}

//=====================================================================================//
//                               void Shell::IdleLoop()                                //
//=====================================================================================//
//	цикл ожидания
void Shell::IdleLoop(void)
{
	//	обрабатываем оконные сообщения
	m_Container->m_Frame.Wait();
}

//=====================================================================================//
//                                void Shell::OnClose()                                //
//=====================================================================================//
//	событие [выход из приложения]
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
	//	запуск таймера
	Timer::Resume();

	shell("   on activated;\n");
}

//=====================================================================================//
//                             void Shell::OnDeactivate()                              //
//=====================================================================================//
void Shell::OnDeactivate(void)
{
	m_Loop = IdleLoop;
	//	приостановление таймера
	Timer::Suspend();
	shell("   on deactivated;\n");
}

//=====================================================================================//
//                        void Shell::OnStartChangeVideoMode()                         //
//=====================================================================================//
void Shell::OnStartChangeVideoMode(void)
{
	shell("   start change Video Mode;\n");

	//	приостановление таймера
	Timer::Suspend();
	//	освобождение объектов GraphPipe
	m_Container->m_GraphPipe.Release();
	//	уведомляем менеджер 2D поверхностей
	SurfaceMgr::Instance()->Release();
	//	освобождение бинков
	BinkMgr::Instance()->Release();
	//	освобождение шрифтов
	FontMgr::Instance()->Release();
	//	освобождение текстур
	TextureMgr::Instance()->Release();
	//	выгруза шрифтов
	FontMgr::Instance()->Release();
	//	освобождение GammaControl'а
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
	//	показываем окно
	m_Container->m_Frame.ShowWindow();
	//	инициализация текстур
	TextureMgr::Instance()->Init(D3DKernel::GetD3DDevice(),
								 Options::GetInt("system.video.mipmap"),
								 Options::GetInt("system.video.texquality"),
								 Options::GetInt("system.video.usecompressing"),
								 false);
	shell("    ....TextureMgr::Instance()->Init(); \n");
	//	иницилизация GraphPipe
	m_Container->m_GraphPipe.Init(D3DKernel::GetD3D(),
								  D3DKernel::GetD3DDevice(),
								  D3DKernel::ResX(),
								  D3DKernel::ResY());
	shell("    ....GraphPipe->Init();\n");
	//	инициализация игры
	Game::Tune();
	shell("    ....Game::Tune();\n");
	//	инициализация гамма-контрола
	GammaControl::Init(D3DKernel::GetPS());
	shell("    ....GammaControl::Init();\n");

	if(IWorld::Get() && IWorld::Get()->GetLevel())
	{
		IWorld::Get()->GetLevel()->OnFinishChangeVideoMode();
	}

	//	запуск таймера
	Timer::Resume();

	shell("   finish change Video Mode;\n");
}

//=====================================================================================//
//                              float Shell::FPS() const                               //
//=====================================================================================//
//	получить кол-во кадров в секунду
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
//	войти в модальный цикл
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
	
	//if(m_IsModalMode) throw CASUS("Попытка войти в модальный цикл второй раз.");
	//m_IsModalMode = true;
	//while(!m_Exit) (*this.*m_Loop)();
	//m_Exit = false;
	//m_IsModalMode = false;
}

//=====================================================================================//
//                             bool Shell::IsModal() const                             //
//=====================================================================================//
//	проверить: свободен ли модальный режим
bool Shell::IsModal(void) const
{
	return m_loops.size() > 1;
	//return m_IsModalMode;
}

//=====================================================================================//
//                               DDFrame* Shell::Frame()                               //
//=====================================================================================//
//	вернуть указатель на каркас приложения
DDFrame* Shell::Frame(void)
{
	return &m_Container->m_Frame;
}

//=====================================================================================//
//                       GraphPipe* Shell::GetGraphPipe() const                        //
//=====================================================================================//
//	получить указатель на GraphPipe
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
		//	отрисовка игры
		Game::Render();
		//	смена кадра
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
	//	обрабатываем оконные сообщения
	Shell::Instance()->m_Container->m_Frame.Tick();
	//	проверка положения мыши
	Shell::Instance()->m_Container->m_Frame.CheckMouse();

	//	выравниваем по fps'ам
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

	//	замеряем fps'ы
	Shell::Instance()->m_Container->m_FPSometer.AddTime(Timer::GetSeconds());
}

//=====================================================================================//
//                            void ModalLoop::normalThink()                            //
//=====================================================================================//
void ModalLoop::normalThink()
{
	//	обновление таймера
	Timer::Update();

	//	обновление звука
	Muffle::ISound::instance()->manage();

	Input::Update();
		
	//	переход хода игре
	Game::Tick();
}

//=====================================================================================//
//                             void ModalLoop::idleThink()                             //
//=====================================================================================//
void ModalLoop::idleThink()
{
	//	обрабатываем оконные сообщения
	Shell::Instance()->m_Container->m_Frame.Wait();
}

/***********************************************************************

Virtuality

Copyright by MiST land 2000

--------------------------------------------------------------------
Description:
--------------------------------------------------------------------
Author: Pavel A.Duvanov (Naughty)
Date:   12.04.2000

************************************************************************/
#include "Precomp.h"
#include "Options/Options.h"
#include <Common/GraphPipe/GraphPipe.h>
#include <Common/CmdLine/CmdLine.h>
#include <Common/GraphPipe/statesmanager.h>
#include "GameLevel/LongShot.h"
#include "Interface/Interface.h"
#include "VolumeHelper.h"

#include "gamelevel/grid.h"
#include "gamelevel/gamelevel.h"
#include "Common/3dEffects/EffectManager.h"
#include "Interface/ScreenShot.h"
#include "Interface/graphmgr.h"
#include "logic2/GameLogic.h"
#include "Skin/Shadow.h"
#include "Game.h"
#include "IWorld.h"
#include <Muffle/ISound.h>
#include "common/utils/profiler.h"
#include "interface/screens.h"
#include "interface/gamescreen.h"
#include <mmsystem.h>
#include <numeric>
#include "GameLevel/DynObjectPool.h"
#include <Common/D3DApp/Input/Input.h>

#ifdef __ML_PROJECT_ALREADY_CONFIGURED__
#undef __ML_PROJECT_ALREADY_CONFIGURED__
#endif

#include <logic2/logictypes.h>
#include <logic2/entity.h>
#include "ParadiseWorld.h"

#include <mll/utils/timer.h>

// testing ActMgr
#include "logic_ex/ActMgr.h"

//---------- Лог файл ------------
//#define _DEBUG_SHELL
#ifdef _DEBUG_SHELL
CLog game_log;
#define game	game_log["game.log"]
#else
#define game	/##/
#endif

#ifdef _HOME_VERSION
namespace PoL
{
void disappearEntity(BaseEntity *);
}
#endif

//	внешние переменные
bool _OldRender=true;

//#ifdef _HOME_VERSION
//#	define USING_VTUNE
//#endif

//	------------- Внутрение объекты ---------------


namespace Game	
{
	//	необходимо переинициализировать при смене Video Mode
	GraphPipe *m_pGraphPipe = 0;
	//	управление поведением уровня
	bool m_GameLevelRender = false;			//	флаг отрисовки уровня
	bool m_GameLevelUpdate = false;			//	флаг обновления уровня
}

namespace Game	
{
	std::string m_CurGameLevel;
	//	игровое время
	float m_fGameTime = 0;
}

//	--------------------- Функции -------------------------------------
namespace Game
{
	void InitScene(void);
	void UnloadGameLevel(void);

	static const std::string g_addonName = "polmis";

	const std::string &GetAddonName()
	{
		return g_addonName;
	}
}

static std::auto_ptr<IWorld> g_world;

//	--------------------- Функции -------------------------------------

///////////////////////////////////////////////////////////////////////
//	переинициализация объектов при смене видео режима
//=====================================================================================//
//                                  void Game::Tune()                                  //
//=====================================================================================//
void Game::Tune(void)
{
	if(Interface::IsInitialized()) Interface::Instance()->OnChangeVideoMode();

	//	инициализация сцены
	InitScene();

	if(IWorld::Get()->GetLevel())
	{
		IWorld::Get()->GetLevel()->LinkShaders(m_pGraphPipe);
	}

	game("--- GAME TUNE ---\n");
	//	инициализация теней
	Shadows::UpdateOnModeChange();
}

//=====================================================================================//
//                                  void Game::Init()                                  //
//=====================================================================================//
void Game::Init(GraphPipe *pGraphPipe)
{
	///Grom::testing
	//   QuestEngine::ChangeEpisode(0);
	///Grom::testing
	//--------------------------------------
	_OldRender = !Options::GetInt("system.video.tl_optimization");
	//--------------------------------------
	m_pGraphPipe = pGraphPipe;
	//--------------------------------------
	//Инициализация игрового мира
	g_world.reset(new ParadiseWorld(m_pGraphPipe));
	//IWorld::Get()->Init(m_pGraphPipe);

	point3 pos(m_pGraphPipe->GetCam()->GetPos());
	point3 front(m_pGraphPipe->GetCam()->GetFront());
	point3 up(m_pGraphPipe->GetCam()->GetUp());

	// --------------------------------------------------
	// инициализация теней
	Shadows::UpdateOnModeChange();
	//	//	инициализируем интерфейс
	//	Interface::Instance();
	// инициализация сцены
	InitScene();
	//   
	float vol=Options::GetInt("game.sound.effectsvol");
	PoL::VolumeHelper::setEffectsVolume(vol * 0.01f);
	//Muffle::ISound::instance()->asetVolume(Muffle::ISound::cEffects,vol*0.01f);
	vol=Options::GetInt("game.sound.ambientvol");
	PoL::VolumeHelper::setAmbientVolume(vol * 0.01f);
	//Muffle::ISound::instance()->asetVolume(Muffle::ISound::cAmbient,vol*0.01f);
	vol=Options::GetInt("game.sound.musicvol");
	PoL::VolumeHelper::setThemesVolume(vol * 0.01f);
	//Muffle::ISound::instance()->asetVolume(Muffle::ISound::cThemes,vol*0.01f);
	vol=Options::GetInt("game.sound.voicesvol");
	PoL::VolumeHelper::setSpeechVolume(vol * 0.01f);
	//Muffle::ISound::instance()->asetVolume(Muffle::ISound::cSpeech,vol*0.01f);
}

//=====================================================================================//
//                                 void Game::Close()                                  //
//=====================================================================================//
void Game::Close(void)
{
	//IWorld::Get()->Close();
	//	освобождаем все ресурсы, используемые интерфейсом
	Interface::Instance()->Release();	
	g_world.reset();
}


//=====================================================================================//
//                             bool Game::RenderEnabled()                              //
//=====================================================================================//
bool Game::RenderEnabled()
{
	return m_GameLevelRender;
}

//struct TimeDeltaQuery2
//{
//	unsigned int &target;
//	unsigned int begin;
//	TimeDeltaQuery2(unsigned int &t) : target(t), begin(timeGetTime()) {}
//	~TimeDeltaQuery2() { unsigned int end = timeGetTime(); target = end-begin; }
//};


//=====================================================================================//
//                                  void Game::Tick()                                  //
//=====================================================================================//
//	передача управления игре
void Game::Tick(void)
{
	static float tGLevel=Timer::GetSeconds();
	float tSub;

	//	-----------------------------------------------
	tSub = Timer::GetSeconds()-tGLevel;
	tGLevel = Timer::GetSeconds();

	//	-----------------------------------------------
	//	управление на интерфейс
	Interface::Instance()->Tick();

	if(m_GameLevelUpdate)
	{
		//	-----------------------------------------------
		IWorld::Get()->Tick();
		{
			CodeProfiler CP("level.tick");
			IWorld::Get()->GetLevel()->UpdateMarks(Timer::GetSeconds());
			IWorld::Get()->GetLevel()->UpdateObjects(Timer::GetSeconds());
			IWorld::Get()->GetCamera()->Update(Timer::GetSeconds());

			IWorld::Get()->GetEffects()->NextTick(Timer::GetSeconds());
			IWorld::Get()->GetEffects()->Update();
			//	-----------------------------------------------
			m_fGameTime += tSub;
		}
	}

	//ISound::instance()->manage();
}

//  Обработка Input'а
extern bool DoMulti,DoWire;
bool ProcessCamera=true;
float KEYSPEED = 1.0f;
float MOUSESPD = 1.0f;

namespace Shadows { extern bool g_useHT; }

//=====================================================================================//
//                              void Game::ProcessInput()                              //
//=====================================================================================//
void Game::ProcessInput(void)
{
	static bool firstTime = true;

	if(firstTime)
	{
		KEYSPEED = MOUSESPD = Options::GetFloat("game.camera_speed");
		firstTime = false;
	}

	GameLevel *GLevel=IWorld::Get()->GetLevel();

	static float f,n,fov; //информация о камере
	static float LastTime=0;
	float DifTime=Timer::GetSeconds()-LastTime;
	LastTime+=DifTime;
	//управление камерой 
	float dx,dy,dz,mx,my;
	dx=Input::MouseState().dx/D3DKernel::ResX();
	dy=Input::MouseState().dy/D3DKernel::ResY();
	dz=Input::MouseState().dz;
	mx=Input::MouseState().x;
	my=Input::MouseState().y;

	//проверка можно ли двигаться
	bool BlockKeys = !Interface::Instance()->IsCameraMoveable();
	bool _shift=Input::KeyState(DIK_RSHIFT)||Input::KeyState(DIK_LSHIFT);
	bool _rmb=Input::MouseState().RButtonState;
	bool _lmb=Input::MouseState().LButtonState;
	bool _mmb=Input::MouseState().MButtonState;
	bool _up=Input::KeyState(DIK_UPARROW);
	bool _down=Input::KeyState(DIK_DOWNARROW);
	bool _right=Input::KeyState(DIK_RIGHTARROW);
	bool _left=Input::KeyState(DIK_LEFTARROW);
	bool _ypressed = Input::KeyFront(DIK_Y);

#ifdef _HOME_VERSION
	if(Input::KeyFront(DIK_K))
	{
		Shadows::g_useHT = !Shadows::g_useHT;
		if(!Shadows::g_useHT)
		{
			Shadows::Sync::inst().wait();
		}
	}
#endif

	if(_rmb||_mmb)  Input::Mouse()->LockPosition();
	else            Input::Mouse()->UnlockPosition();

	if(ProcessCamera && !BlockKeys)
	{
		Camera *Cam=m_pGraphPipe->GetCam();
		//горизонтальное перемещение
		if(_mmb && (dx || dy))            Cam->Move(MOUSESPD/30*point3(200*dx,-200*dy,0));
		if(_right&&!_shift) Cam->Move(DifTime*KEYSPEED*point3(15,0,0));
		if(_left&&!_shift)  Cam->Move(DifTime*KEYSPEED*point3(-15,0,0));
		if(_up&&!_shift)    Cam->Move(DifTime*KEYSPEED*point3(0,15,0));
		if(_down&&!_shift)  Cam->Move(DifTime*KEYSPEED*point3(0,-15,0));
		if(mx<2)            Cam->Move(DifTime*MOUSESPD*point3(-10,0,0));
		if(my<2)            Cam->Move(DifTime*MOUSESPD*point3(0,10,0));
		if(mx>D3DKernel::ResX()-3) Cam->Move(DifTime*MOUSESPD*point3(10,0,0));
		if(my>D3DKernel::ResY()-3) Cam->Move(DifTime*MOUSESPD*point3(0,-10,0));

		//изменения угла к горизонту
		unsigned int cflag = _shift ? Camera::SELF : Camera::AIMSPOT;
		if(Input::KeyState(DIK_PGUP)) Cam->Rotate(DifTime*KEYSPEED,cflag|Camera::RIGHT);
		if(Input::KeyState(DIK_PGDN)) Cam->Rotate(-DifTime*KEYSPEED,cflag|Camera::RIGHT);
		//вращение камеры
		if(_right&&_shift) Cam->Rotate(DifTime*KEYSPEED*2,cflag|Camera::Z);
		if(_left&&_shift) Cam->Rotate(-DifTime*KEYSPEED*2,cflag|Camera::Z);
		//два предыдущих вместе
		if(_rmb && !_lmb && !_mmb)
		{
			if(Cam->GetMoveType() == cmtFree)
			{
				if(dx) Cam->Rotate(dx*MOUSESPD,cflag|Camera::Z);
				if(dy) Cam->Rotate(dy*MOUSESPD,cflag|Camera::RIGHT);
			}
			else if(Cam->GetMoveType() == cmtPersonBone)
			{
				if(dx) Cam->Rotate(dx*MOUSESPD, Camera::Z | Camera::SELF);
				if(dy) Cam->Rotate(dy*MOUSESPD, Camera::RIGHT | Camera::SELF);
			}
		}
		//приближение/удаление
		if(_rmb&&_lmb && (dx || dy))     Cam->Move(MOUSESPD/30*point3(0,0,-100*(dx+dy)));
		if(_up&&_shift)    Cam->Move(DifTime*KEYSPEED*point3(0,0,15));
		if(_down&&_shift)  Cam->Move(DifTime*KEYSPEED*point3(0,0,-15));
		if(dz!=0.f) 			 Cam->Move(DifTime*MOUSESPD*point3(0,0,dz*0.2));

		if(_ypressed)
		{
			if(Cam->GetMoveType() != cmtPersonBone)
			{
				EntityPool *pool = EntityPool::GetInst();
				for(EntityPool::Iterator i = pool->begin(ET_ALL_ENTS,PT_PLAYER1); i != pool->end(); ++i)
				{
					ActiveContext *ac = static_cast<ActiveContext*>(i->GetEntityContext());
					if(ac->IsSelected())
					{
						Cam->AttachToEntity(i->GetEID());
						break;
					}
				}
			}
			else
			{
				Cam->DetachFromEntity();
			}
		}
	}

#ifdef _HOME_VERSION
	if(Input::KeyFront(DIK_SCROLL))
	{
		if(!_shift)
		{
			DoMulti=!DoMulti;
		}
		else
		{
			DoWire=!DoWire;
		}
	}
#endif

	GameLogic::Think();
}

//static LARGE_INTEGER getCurrentTime()
//{
//	LARGE_INTEGER time, freq;
//	QueryPerformanceCounter(&time);
//	//QueryPerformanceFrequency(&freq);
//	return time;///static_cast<float>(freq.QuadPart);
//}

//	отрисовка всех графических объектов
void Shitit(int i);
void DrawIt();

//=====================================================================================//
//                                 void Game::Render()                                 //
//=====================================================================================//
void Game::Render(void)
{
	//	game("--- Game Render ---\n");
	CodeProfiler CP("game.render");
	//	чистим экранные буффера
	D3DKernel::GetD3DDevice()->Clear(0,NULL,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,0,1.0f,0);
	//D3DKernel::GetD3DDevice()->Clear(0,NULL,D3DCLEAR_ZBUFFER,0,1.0f,0);
	//	отрисовываем 3D элементы

	{
		static bool inhere = false;
		assert( inhere == false );
		inhere = true;
		D3DKernel::GetD3DDevice()->BeginScene();
		inhere = false;
	}

	m_pGraphPipe->StartFrame();
	if(m_GameLevelRender)
	{
		LongShot *longshot = IWorld::Get()->GetLevel()->GetLongShot();
		DynObjectPool *objpool = IWorld::Get()->GetLevel()->GetLevelObjects();

#ifdef _HOME_VERSION
		if(longshot) m_pGraphPipe->Chop(longshot,"Longshot");
#else
		if(longshot) m_pGraphPipe->Chop(longshot);
#endif

		IWorld::Get()->GetLevel()->Update(m_pGraphPipe);
		IWorld::Get()->GetLevel()->Draw(m_pGraphPipe,false);

		IWorld::Get()->GetLevel()->DrawMarks();
		// Отрисовка теней
		Shadows::DrawShadows(m_pGraphPipe);

		if(objpool) objpool->Draw(m_pGraphPipe,false);
		IWorld::Get()->Draw();
		GameLogic::Draw();

		IWorld::Get()->GetLevel()->Draw(m_pGraphPipe,true);
		if(objpool) objpool->Draw(m_pGraphPipe,true);

		//отрисовка эффектов
		IWorld::Get()->GetEffects()->DrawEffects();
		//Shitit(!Input::KeyState(DIK_SPACE));
	}

	//	game("     ....RenderGLevel\n");
	////	воспроизведение интерфейса
	StatesManager::SetTextureStageState(0,D3DTSS_TEXCOORDINDEX,0);
	Interface::Instance()->Render3D();

	//	камера
	if(m_pGraphPipe->GetCam()->IsViewChanged())
	{
		point3 pos(m_pGraphPipe->GetCam()->GetPos());
		point3 front(m_pGraphPipe->GetCam()->GetFront());
		point3 up(m_pGraphPipe->GetCam()->GetUp());
		point3 right(m_pGraphPipe->GetCam()->GetRight());
		Muffle::ISound::instance()->setCamera(front.as_mll_vector(),up.as_mll_vector(),right.as_mll_vector(),pos.as_mll_vector());
	}

	//fixme:   testing. don't delete this!(Grom)
	DrawIt();

	m_pGraphPipe->EndFrame();
	D3DKernel::GetD3DDevice()->EndScene();

	//	отрисовываем 2D элементы
	Interface::Instance()->Render2D();
}

//=====================================================================================//
//                                void SetupPixelFog()                                 //
//=====================================================================================//
void SetupPixelFog(DWORD dwColor, DWORD dwMode)
{
	float fStart = 20.f,    // for linear mode     
		fEnd   = 30.f,
		fDensity = 0.12;  // for exponential modes    
	// Enable fog blending.
	StatesManager::SetRenderState(D3DRENDERSTATE_FOGENABLE, TRUE); 
	// Set the fog color.
	StatesManager::SetRenderState(D3DRENDERSTATE_FOGCOLOR, dwColor);    
	// Set fog parameters.
	if(D3DFOG_LINEAR == dwMode)
	{
		StatesManager::SetRenderState(D3DRENDERSTATE_FOGTABLEMODE, dwMode);
		StatesManager::SetRenderState(D3DRENDERSTATE_FOGSTART, *(DWORD *)(&fStart));
		StatesManager::SetRenderState(D3DRENDERSTATE_FOGEND,   *(DWORD *)(&fEnd));
	}  
	else
	{
		StatesManager::SetRenderState(D3DRENDERSTATE_FOGSTART, *(DWORD *)(&fStart));
		StatesManager::SetRenderState(D3DRENDERSTATE_FOGEND,   *(DWORD *)(&fEnd));
		StatesManager::SetRenderState(D3DRENDERSTATE_FOGTABLEMODE, dwMode);
		StatesManager::SetRenderState(D3DRENDERSTATE_FOGDENSITY, *(DWORD *)(&fDensity));
	} 
}

//=====================================================================================//
//                               void Game::InitScene()                                //
//=====================================================================================//
void Game::InitScene(void)
{
	//SetupPixelFog(RGB_MAKE(92,93,103), D3DFOG_EXP);

	StatesManager::SetRenderState(D3DRENDERSTATE_COLORKEYENABLE,FALSE);
	StatesManager::SetRenderState(D3DRENDERSTATE_COLORKEYBLENDENABLE,FALSE);
	//StatesManager::SetRenderState(D3DRENDERSTATE_NORMALIZENORMALS    ,TRUE);
	StatesManager::SetRenderState(D3DRENDERSTATE_EXTENTS ,FALSE);
	StatesManager::SetRenderState(D3DRENDERSTATE_LIGHTING ,FALSE);

	for(int i=0;i<2;i++)
	{
		StatesManager::SetTextureStageState(i, D3DTSS_MINFILTER, D3DTFN_LINEAR);
		StatesManager::SetTextureStageState(i, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
		StatesManager::SetTextureStageState(i, D3DTSS_MIPFILTER, D3DTFP_LINEAR);

		StatesManager::SetTextureStageState(i, D3DTSS_COLORARG2, D3DTA_CURRENT );
		StatesManager::SetTextureStageState(i, D3DTSS_COLORARG1,D3DTA_TEXTURE );
		StatesManager::SetTextureStageState(i, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
		StatesManager::SetTextureStageState(i, D3DTSS_ALPHAARG1,D3DTA_TEXTURE );

		StatesManager::SetTextureStageState(i, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		StatesManager::SetTextureStageState(i, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
		//StatesManager::SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		//StatesManager::SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	}
	StatesManager::SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	StatesManager::SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	D3DMATERIAL7 d3dMaterial;
	ZeroMemory(&d3dMaterial, sizeof(d3dMaterial));
	d3dMaterial.dcvDiffuse.r = 0.f;
	d3dMaterial.dcvDiffuse.g = 0.f;
	d3dMaterial.dcvDiffuse.b = 0.f;
	d3dMaterial.dcvDiffuse.a = 0.f;
	d3dMaterial.dcvAmbient.r = 1;
	d3dMaterial.dcvAmbient.g = 1;
	d3dMaterial.dcvAmbient.b = 1;
	d3dMaterial.dcvSpecular.r = 0.f;
	d3dMaterial.dcvSpecular.g = 0.f;
	d3dMaterial.dcvSpecular.b = 0.f;
	d3dMaterial.dcvSpecular.b = 0.f;
	d3dMaterial.dvPower = 10.0f;     
	D3DKernel::GetD3DDevice()->SetMaterial(&d3dMaterial);    
	StatesManager::SetRenderState(D3DRENDERSTATE_AMBIENT, 0xffffffff);
	StatesManager::SetRenderState(D3DRENDERSTATE_LIGHTING,FALSE);
	StatesManager::SetRenderState( D3DRENDERSTATE_CULLMODE, D3DCULL_NONE );
	//StatesManager::SetRenderState( D3DRENDERSTATE_WRAP0 , D3DWRAPCOORD_0|D3DWRAPCOORD_1);
}

//*********************** Управление выводом уровня *******************************//
//=====================================================================================//
//                           void Game::SetGameLevelRender()                           //
//=====================================================================================//
void Game::SetGameLevelRender(bool render)
{
#if defined(USING_VTUNE)
	if(render)
	{
		g_vtune.resume();
	}
	else
	{
		g_vtune.pause();
	}
#endif
	m_GameLevelRender = render;
}

//=====================================================================================//
//                           void Game::SetGameLevelUpdate()                           //
//=====================================================================================//
void Game::SetGameLevelUpdate(bool update)
{
	m_GameLevelUpdate = update;
}


namespace DirtyLinks
{
	std::string GetStrRes(const std::string& rid); 
}

//=====================================================================================//
//                             void Game::LoadGameLevel()                              //
//=====================================================================================//
void Game::LoadGameLevel(const char *pLevelName)
{
	IWorld::Get()->ChangeLevel(pLevelName);
	//начнем игру заново

	GameLogic::BegNewGame();
	Interface::Instance()->ProBar()->SetTitle(DirtyLinks::GetStrRes("ld_levelstart").c_str());
	IWorld::Get()->GetLevel()->Start();
}

//=====================================================================================//
//                            void Game::UnloadGameLevel()                             //
//=====================================================================================//
void Game::UnloadGameLevel(void)
{
	SetGameLevelRender(false);
	SetGameLevelUpdate(false);
	//	выгрузим уровень
	IWorld::Get()->ChangeLevel("");
}

//******************** ПОЛУЧЕНИЕ СКРИНШОТА ********************************//
//=====================================================================================//
//                             void Game::MakeScreenShot()                             //
//=====================================================================================//
void Game::MakeScreenShot(void)
{
	static counter = 0;
	char lpBuff[50];

	sprintf(lpBuff,"scr%0.2d.bmp",counter++);
	GammaScreenShot(D3DKernel::GetPS(),lpBuff);
}

//************************************************************************//
//=====================================================================================//
//                                void Game::AdjustHW()                                //
//=====================================================================================//
void Game::AdjustHW(void)
{
	if(IWorld::Get()->GetLevel())
		IWorld::Get()->GetLevel()->LinkShaders(m_pGraphPipe);
}

//=====================================================================================//
//                           void Game::SetEffectsQuality()                            //
//=====================================================================================//
void Game::SetEffectsQuality(float q)
{
	IWorld::Get()->GetEffects()->SetEffectsQuality(q);
}

//=====================================================================================//
//                              void Game::MakeSaveLoad()                              //
//=====================================================================================//
void Game::MakeSaveLoad(const char* pFileName,bool bSave)
{
	StdFile  file(pFileName, bSave?"wb":"rb");

	if(file.IsOk())
	{
		GenericStorage  store(&file, bSave?GenericStorage::SM_SAVE:GenericStorage::SM_LOAD);

		if(!bSave) PoL::ActMgr::reset();

		Interface::Instance()->MakeSaveLoad(store);
		IWorld::Get()->MakeSaveLoad(store);
		GameLogic::MakeSaveLoad(store);
		//Shadows::OptionsChanged();
		if(!bSave)
		{
			IWorld::Get()->GetLevel()->Start();
		}

#ifdef _HOME_VERSION
		std::string val;
		if(CmdLine::IsKey("-disent",val))
		{
			typedef boost::char_separator<char> Sep_t;
			typedef boost::tokenizer<Sep_t> Tok_t;
			Sep_t sep(":");
			Tok_t tok(val,sep);

			std::vector<unsigned int> eids;

			for(Tok_t::iterator i = tok.begin(); i != tok.end(); ++i)
			{
				eids.push_back(boost::lexical_cast<unsigned int>(*i));
			}

			for(EntityPool::iterator i = EntityPool::GetInst()->begin(); i != EntityPool::GetInst()->end(); ++i)
			{
				if(std::find(eids.begin(),eids.end(),i->GetEID()) == eids.end())
				{
					PoL::disappearEntity(&*i);
				}
			}
		}

		if(CmdLine::IsKey("-disentn",val))
		{
			typedef boost::char_separator<char> Sep_t;
			typedef boost::tokenizer<Sep_t> Tok_t;
			Sep_t sep(":");
			Tok_t tok(val,sep);

			std::vector<std::string> eids;

			for(Tok_t::iterator i = tok.begin(); i != tok.end(); ++i)
			{
				eids.push_back(*i);
			}

			for(EntityPool::iterator i = EntityPool::GetInst()->begin(); i != EntityPool::GetInst()->end(); ++i)
			{
				if(std::find(eids.begin(),eids.end(),i->GetInfo()->GetRID()) != eids.end())
				{
					PoL::disappearEntity(&*i);
				}
			}
		}
#endif
	}
	else throw CASUS("MakeSaveLoad: указанный Файл невозможно использовать.");
}

//=====================================================================================//
//                        void Game::PriorInterfaceInitialize()                        //
//=====================================================================================//
//	предварительная настройка интерфейса
void Game::PriorInterfaceInitialize(void)
{
	Interface::PriorInitialize();
}

//=====================================================================================//
//                            void Game::CreateInterface()                             //
//=====================================================================================//
//	создание игрового интерфейса
void Game::CreateInterface(void)
{
	Interface::Instance()->RunMenuFlow();
}

//=====================================================================================//
//                                void Game::PlayBink()                                //
//=====================================================================================//
//	проигрывание мультика
void Game::PlayBink(const char *pFileName)
{
	Interface::Instance()->PlayBink(pFileName);
}

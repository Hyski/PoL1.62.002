/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   20.03.2001

************************************************************************/
#include "Precomp.h"
#include "LoadingScreen.h"

#include <common/DataMgr/DataMgr.h>
#include <common/Shell/Shell.h>
#include <common/UI/Static.h>
#include "WidgetFactory.h"
#include "MouseCursor.h"
#include "DXImageMgr.h"
#include "Interface.h"
#include "Text.h"

#include <mll/utils/vtune.h>

#ifdef _HOME_VERSION
#	define USING_VTUNE
#endif

extern bool g_noSleep;

//************************************************************************************//
//	class LoadingScreen::ImageOscillator
class LoadingScreen::ImageOscillator
{
private:
	static const char* m_LoadingPath;
	static const char* m_ImageNameTemplate;
	static const char* m_ProgressNameTemplate;
private:
	std::string m_ImageName;
	std::string m_ProgresName;
public:
	ImageOscillator()
	{
		std::vector<DataMgr::FileInfo> files;
		std::vector<std::string> image;
		std::vector<std::string> progress;

		DataMgr::Scan(m_LoadingPath,files);

		for(int i=(files.size()-1);i!=0;i--)
		{
			if(strstr(files[i].m_Name.c_str(),m_ImageNameTemplate))
			{
				image.push_back(m_LoadingPath+files[i].m_FullName);
				continue;
			}
			if(strstr(files[i].m_Name.c_str(),m_ProgressNameTemplate))
			{
				progress.push_back(m_LoadingPath+files[i].m_FullName);
			}
		}
		//	----------------------------------------------------------
		if(image.size()) m_ImageName = image[(image.size()==1)?0:(rand()%(image.size()-1))];
		if(progress.size()) m_ProgresName = progress[(progress.size()==1)?0:(rand()%(progress.size()-1))];
	}
	~ImageOscillator(){}
public:
	const std::string& GetImageName(void) {return m_ImageName;}
	const std::string& GetProgressName(void)  {return m_ProgresName;}
};

const char* LoadingScreen::ImageOscillator::m_LoadingPath = "pictures/interface/Loading/";
const char* LoadingScreen::ImageOscillator::m_ImageNameTemplate = "loading";
const char* LoadingScreen::ImageOscillator::m_ProgressNameTemplate = "progress";

//************************************************************************************//
//	class LoadingScreen
const char* LoadingScreen::m_pDialogName = "LoadingScreen";
LoadingScreen::LoadingScreen() : Dialog(m_pDialogName,m_pDialogName),m_Step(S_PREPARE)
{
	ImageOscillator io; 
	VFile *file = DataMgr::Load("scripts/progress.txt");

	int px = 235, py = 440, pxx = 615, pyy = 457;

	if(file && file->Size())
	{
		std::string data;
		data.resize(file->Size());
		std::copy_n(file->Data(),file->Size(),data.begin());
		std::istringstream sstr(data);
		sstr >> px >> py >> pxx >> pyy;
	}
	DataMgr::Release("scripts/progress.txt");

	Widget* pImage = WidgetFactory::Instance()->Assert(this,"background");
	if(pImage->GetImage()) pImage->GetImage()->Release();
	pImage->SetImage(DXImageMgr::Instance()->CreateImage(io.GetImageName().c_str()));
	pImage->Insert(m_Progress = new Widget("progress"));
	m_Progress->GetLocation()->Create(px,py,pxx,pyy);	//	235,439,615,456
	m_Progress->Insert(new Widget("image"));
	m_Progress->Child("image")->GetLocation()->Create(0,0,m_Progress->GetLocation()->Region()->Width(),m_Progress->GetLocation()->Region()->Height());
	m_Progress->Child("image")->SetImage(DXImageMgr::Instance()->CreateImage(io.GetProgressName().c_str()));
	m_ProgressWidth = m_Progress->GetLocation()->Region()->Width();
	//	������� ��������� ���� ��� ���������
	pImage->Insert(m_Title = new Text("title"));
	m_Title->Align(LogicalText::T_HCENTER|LogicalText::T_VCENTER);
	m_Title->SetFont("mila_s",0xffdd77);
	m_Title->SetText("");
	int left,top,right,bottom;
	m_Progress->GetLocation()->Region()->GetRect(&left,&top,&right,&bottom);
	m_Title->SetRegion(left,top,right,bottom);
}

LoadingScreen::~LoadingScreen()
{
	//	���������� ������
	MouseCursor::SetVisible(true);
}

//=====================================================================================//
//                          void LoadingScreen::SetVisible()                           //
//=====================================================================================//
void LoadingScreen::SetVisible(bool vis)
{
#ifdef USING_VTUNE
	if(vis)
	{
		mll::utils::vtune::resume();
	}
	else
	{
		mll::utils::vtune::pause();
	}
#endif
	Dialog::SetVisible(vis);
}

//=====================================================================================//
//                             bool LoadingScreen::Tick()                              //
//=====================================================================================//
bool LoadingScreen::Tick(void)
{
	switch(m_Step)
	{
	case S_PREPARE:
		//	��������� ������� ����������
		CheckConformMode();
		//	������������� �������� �� ����� �� ��������� �������� ���� � ����
		Update(0);
		//	��������� ���
		m_Step = S_READY_FOR_LOADING;
		break;
	case S_READY_FOR_LOADING:
		//	���������� �������� � ���������� ������� ������
		Parent()->OnChange(this);
		m_Step = S_LOADING_DONE;
		break;
	case S_LOADING_DONE:
		//	���������� ������
		MouseCursor::SetVisible(true);
		//	����������
		Parent()->OnChange(this);
		m_Step = S_QUITE;
		break;
	default:
		return true;
	}

	return false;
}
//	����������� �������
void LoadingScreen::Draw(WidgetView* pView)
{
	D3DKernel::GetD3DDevice()->Clear(0,NULL,D3DCLEAR_TARGET,0x00000000,1.0f,0);
	Dialog::Draw(pView);
}
//	���������� �������� ����
void LoadingScreen::Update(float value)
{
	static int left,top,right,bottom;

	D3DKernel::GetD3DDevice()->Clear(0,NULL,D3DCLEAR_TARGET,0x00000000,1.0f,0);
	m_Progress->GetLocation()->Region()->GetRect(&left,&top,&right,&bottom);
	right = left+(m_ProgressWidth*value);
	m_Progress->GetLocation()->Create(left,top,right,bottom);
	//	������ ������
	MouseCursor::SetVisible(false);
	//	������������ ���������
	Interface::Instance()->Render2D();
	DebugInfo::Render();
	//	���������� ������
	D3DKernel::UpdateFrame();
	//	��������� ��������� �� ����
	Frame()->Tick();
}
//	���������� ��������� �������� ����
void LoadingScreen::SetTitle(const char* title)
{
	//	������������� ��������� �������� ����
	m_Title->SetText(title);
	//	��������� �����
	D3DKernel::GetD3DDevice()->Clear(0,NULL,D3DCLEAR_TARGET,0x00000000,1.0f,0);
	//	������ ������
	MouseCursor::SetVisible(false);
	//	������������ ���������
	Interface::Instance()->Render2D();
	//	���������� ������
	D3DKernel::UpdateFrame();
}
//**********************************************************************//
//*********	          class HiddenMovementScreen           *************//
const char* HiddenMovementScreen::m_pDialogName = "HiddenMovementScreen";
bool HiddenMovementScreen::m_DebugMode = false;
HiddenMovementScreen::HiddenMovementScreen() : Dialog(m_pDialogName,m_pDialogName),m_ipb(0)
{
	m_ProgressImage = static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"progress"));
	Remove(m_ProgressImage->Name());
	//	�������� ��������� �� ������ ���
	m_Background = static_cast<Static*>(WidgetFactory::Instance()->Assert(this,"background"));
	//	�������� ��������� �� ��������
	m_Background->Insert(m_Progress = new Widget("progress"));
	m_Progress->GetLocation()->Create(*m_ProgressImage->GetLocation()->Region());
	m_Progress->Insert(m_ProgressImage);
	m_ProgressImage->GetLocation()->Create(0,0,m_Progress->GetLocation()->Region()->Width(),m_Progress->GetLocation()->Region()->Height());
	m_ProgressWidth = m_Progress->GetLocation()->Region()->Width();
	//	������������� ���-�� �������� �� 
	m_ipb = m_Background->GetStateSize()*0.5f;
}

HiddenMovementScreen::~HiddenMovementScreen()
{
}
//	����������� �� ��������� ��������� �������
void HiddenMovementScreen::OnSystemChange(void)
{
	Widget::OnSystemChange();
}
//	������� ��������
void HiddenMovementScreen::SetBackground(BELONG belong)
{
	switch(belong)
	{
	case B_CIVILIAN:
		m_Background->SetState(rand()%m_ipb);
		break;
	case B_ENEMY:
		m_Background->SetState(m_ipb+rand()%m_ipb);
		break;
	}
}
// �������� �������� ���
void HiddenMovementScreen::Update(float value)
{
	static int left,top,right,bottom;

	m_Progress->GetLocation()->Region()->GetRect(&left,&top,&right,&bottom);
	right = left+(m_ProgressWidth*value);
	m_Progress->GetLocation()->Create(left,top,right,bottom);
}
//	����������� ������� � ���� �������� ��������
void HiddenMovementScreen::Render(WidgetView* pView)
{
	if(IsVisible())
	{
		if(!m_DebugMode)
		{
			D3DKernel::GetD3DDevice()->Clear(0,NULL,D3DCLEAR_TARGET,0x00000000,1.0f,0);
			Dialog::Render(pView);
		}
		else
		{
			m_Progress->Render(pView);
		}
	}
}

//**********************************************************************//
//	class PanicScreen
const char* PanicScreen::m_pDialogName = "PanicScreen";
PanicScreen::PanicScreen() : Dialog(m_pDialogName,m_pDialogName)
{
	m_Content = static_cast<Text*>(WidgetFactory::Instance()->Assert(this,"content"));
}

PanicScreen::~PanicScreen()
{
}
//	���������� ����� ��������
void PanicScreen::SetContent(const char* text)
{
	m_Content->SetText(text);
}
//	����������� �� ��������� ��������� �������
void PanicScreen::OnSystemChange(void)
{
	Widget::OnSystemChange();
}

//**********************************************************************//
//	class ScriptSceneScreen
const char* ScriptSceneScreen::m_pDialogName = "ScriptSceneScreen";
ScriptSceneScreen::ScriptSceneScreen() : Dialog(m_pDialogName,m_pDialogName)
{
	GetLocation()->Create(0,0,D3DKernel::ResX(),D3DKernel::ResY());

	Insert(m_Top = new Curtain("top"),L_LEFT_TOP);
	Insert(m_Bottom = new Curtain("bottom"),L_LEFT_BOTTOM);

	m_Top->Typify(Curtain::T_TOP);
	m_Bottom->Typify(Curtain::T_BOTTOM);

	//	������� �������� ������
	MouseCursor::SetVisible(false);

	SetVisible(true);
}

ScriptSceneScreen::~ScriptSceneScreen()
{
	MouseCursor::SetVisible(true);
}
//	����������� �� ��������� ��������� �������
void ScriptSceneScreen::OnSystemChange(void)
{
	Widget::OnSystemChange();
}

//**********************************************************************//
//	class ScriptSceneScreen::Curtain
const float ScriptSceneScreen::Curtain::m_PerCent = 0.15f;
ScriptSceneScreen::Curtain::Curtain(const char* pName) : Widget(pName),m_Type(T_TOP)
{
}

ScriptSceneScreen::Curtain::~Curtain()
{
}
//	������������
void ScriptSceneScreen::Curtain::Typify(TYPE type)
{
	LPDIRECTDRAWSURFACE7 surface;
	
	if(GetImage()) DXImageMgr::Instance()->Release(static_cast<DXImage*>(GetImage()));
	if(Parent())
	{
		int width  = Parent()->GetLocation()->Region()->Width();
		int height = Parent()->GetLocation()->Region()->Height()*m_PerCent;

		SetImage(DXImageMgr::Instance()->CreateImage(width,height));
		
		if(GetImage() && (surface = ((DXImage *)GetImage())->m_pSurface))
		{
			ClearSurface(surface);
			
			switch(type)
			{
			case T_TOP:
				GetLocation()->Create(0,0,width,height);
				break;
			case T_BOTTOM:
				GetLocation()->Create(0,Parent()->GetLocation()->Region()->Height()-height,width,Parent()->GetLocation()->Region()->Height());
				break;
			}
		}
	}
}
//	��������� ��������� ���������
void ScriptSceneScreen::Curtain::OnStart(void)
{
	m_StartTime = Timer::GetSeconds();

	switch(m_Type)
	{
	case T_TOP:
		MoveTo(0,0);
		break;
	case T_BOTTOM:
		MoveTo(0,Parent()->GetLocation()->Region()->Height()-GetLocation()->Region()->Height());
		break;
	}
}
//	�������� ���������
void ScriptSceneScreen::Curtain::OnUpdate(void)
{
	switch(m_Type)
	{
	case T_TOP:
		break;
	case T_BOTTOM:
		break;
	}
}
//*********************************************************************************//
void ScriptSceneScreen::Curtain::ClearSurface(LPDIRECTDRAWSURFACE7 surface)
{
	DDSURFACEDESC2 ddsd;
	memset(&ddsd,0,sizeof(DDSURFACEDESC2));
	ddsd.dwSize = sizeof(DDSURFACEDESC2);
	surface->GetSurfaceDesc(&ddsd);
	DDBLTFX ddfx;
	memset(&ddfx,0,sizeof(DDBLTFX));
	ddfx.dwSize = sizeof(DDBLTFX);
	ddfx.dwFillColor = 0;
	//	������� �����������
	surface->Blt(0,0,0,DDBLT_COLORFILL,&ddfx);
}

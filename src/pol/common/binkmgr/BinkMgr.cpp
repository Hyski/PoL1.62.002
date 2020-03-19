/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   02.03.2001

************************************************************************/
#include "Precomp.h"
#include "../Shell/Shell.h"
#include "BinkSurface.h"
#include "BinkMgr.h"

//---------- ��� ���� ------------
#ifdef _DEBUG_GRAPHICS
CLog binkmgr_log;
#define binkmgr	binkmgr_log["binkmgr.log"]
#else
#define binkmgr	/##/
#endif
//--------------------------------

BinkMgr::Deleter BinkMgr::m_Deleter;

//=====================================================================================//
//                                 BinkMgr::BinkMgr()                                  //
//=====================================================================================//
BinkMgr::BinkMgr()
:	m_volume(1.0f)
{
	m_Updated = m_mBinkSurfaces.begin();
}

//=====================================================================================//
//                                 BinkMgr::~BinkMgr()                                 //
//=====================================================================================//
BinkMgr::~BinkMgr()
{
	Release();
}

//=====================================================================================//
//                       LPDIRECTDRAWSURFACE7 BinkMgr::Surface()                       //
//=====================================================================================//
// �������� ����������� �� �������������� �����
LPDIRECTDRAWSURFACE7 BinkMgr::Surface(const char *pName)
{
	static std::map<std::string,Reference*>::iterator i;

	i = m_mBinkSurfaces.find(pName);
	if(i == m_mBinkSurfaces.end())
	{
		binkmgr("Loading bink [%s] - ",pName);
		LoadBink(pName);
		i = m_mBinkSurfaces.find(pName);
	}
	binkmgr("Return bink [%s] - reference[%d];\n",pName,i->second->RefCount());
	m_Updated = m_mBinkSurfaces.begin();


	return i->second->AddRef()->Surface();
}

//=====================================================================================//
//                               void BinkMgr::Release()                               //
//=====================================================================================//
// ���������� ����������� � ������� ����� �� ��������������
void BinkMgr::Release(const char *pName)
{
	std::map<std::string,Reference*>::iterator i;

	i = m_mBinkSurfaces.find(pName);
	if(i!=m_mBinkSurfaces.end())
	{
		if(!i->second->Release())
		{
			binkmgr("Unload bink: [%s];\n",pName);
			delete i->second;
			m_mBinkSurfaces.erase(i);
		}
	}
	m_Updated = m_mBinkSurfaces.begin();
}

//=====================================================================================//
//                               void BinkMgr::Release()                               //
//=====================================================================================//
//	���������� ��� �����������
void BinkMgr::Release(void)
{
	binkmgr(" --- Releasing All Binks --- \n");
	for(std::map<std::string,Reference*>::iterator i = m_mBinkSurfaces.begin();i!=m_mBinkSurfaces.end();i++)
	{
		binkmgr("Unload bink: [%s];\n",i->first.c_str());
		delete i->second;
	}
	m_mBinkSurfaces.clear();
	m_Updated = m_mBinkSurfaces.begin();
}

//=====================================================================================//
//                               void BinkMgr::Update()                                //
//=====================================================================================//
//	������������� ��� �������� ����
void BinkMgr::Update(void)
{	
	float play_time = (1.0f/Shell::Instance()->FPS())*0.04f;
	float ftime;
	int num = 0;

	if(m_mBinkSurfaces.size())
	{
		Timer::Update();
		ftime = Timer::GetSeconds();
		do
		{
			if(m_Updated != m_mBinkSurfaces.end())
			{
				m_Updated->second->Value()->Update();
				m_Updated++;
			}
			else
			{
				m_Updated = m_mBinkSurfaces.begin();
				m_Updated->second->Value()->Update();
			}
			num++;
			if(num == m_mBinkSurfaces.size()) break;
			Timer::Update();
		} while((Timer::GetSeconds()-ftime)<play_time);
//		DebugInfo::Add(200,200,"play_time %0.6f %d",play_time,num);
	}
}

//=====================================================================================//
//                              void BinkMgr::SetVolume()                              //
//=====================================================================================//
void BinkMgr::SetVolume(float vol)
{
	m_volume = vol;

	for(Binks_t::iterator i = m_mBinkSurfaces.begin(); i != m_mBinkSurfaces.end(); ++i)
	{
		i->second->Value()->SetVolume(m_volume);
	}
}

//=====================================================================================//
//                               void BinkMgr::Update()                                //
//=====================================================================================//
void BinkMgr::Update(const char *pName)
{
	std::map<std::string,Reference*>::iterator i;

	i = m_mBinkSurfaces.find(pName);
	if(i != m_mBinkSurfaces.end())
	{
		i->second->Value()->Update();
	}
}

//=====================================================================================//
//                              void BinkMgr::LoadBink()                               //
//=====================================================================================//
void BinkMgr::LoadBink(const char *pName)
{
	BinkSurface *pBS;

	pBS = new BinkSurface();
	pBS->Create(D3DKernel::GetDD(),pName);
	pBS->SetVolume(m_volume);
	m_mBinkSurfaces.insert(std::map<std::string,Reference*>::value_type(pName,new Reference(pBS)));
	SetColorKey(pBS->Surface());
	binkmgr("- %s;\n",pBS->Surface()?"SUCCESS":"FAILED");
	Update(pName);
}

//=====================================================================================//
//                             void BinkMgr::SetColorKey()                             //
//=====================================================================================//
void BinkMgr::SetColorKey(LPDIRECTDRAWSURFACE7 surface)
{
	DDPIXELFORMAT ddpf;
	DDCOLORKEY ddcolorkey;

	if(surface)
	{
		memset(&ddpf,0,sizeof(DDPIXELFORMAT));
		ddpf.dwSize = sizeof(DDPIXELFORMAT);
		D3DKernel::GetPS()->GetPixelFormat(&ddpf);
		ddcolorkey.dwColorSpaceLowValue = ddpf.dwGBitMask|ddpf.dwBBitMask;
		ddcolorkey.dwColorSpaceHighValue = ddcolorkey.dwColorSpaceLowValue;
		surface->SetColorKey(DDCKEY_SRCBLT,&ddcolorkey);
	}
}

//***************************************************************//
//************** class SurfaceMgr::Reference ********************//
//=====================================================================================//
//                           BinkMgr::Reference::Reference()                           //
//=====================================================================================//
BinkMgr::Reference::Reference() : m_Value(0),m_Counter(0)
{
}

//=====================================================================================//
//                           BinkMgr::Reference::Reference()                           //
//=====================================================================================//
BinkMgr::Reference::Reference(BinkSurface* value)
{
	m_Value = value;
	m_Counter = 0;
}

//=====================================================================================//
//                          BinkMgr::Reference::~Reference()                           //
//=====================================================================================//
BinkMgr::Reference::~Reference()
{
	if(m_Value)
		delete m_Value;
}

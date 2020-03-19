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
#include "BinkSurface.h"

//=====================================================================================//
//                             BinkSurface::BinkSurface()                              //
//=====================================================================================//
BinkSurface::BinkSurface()
:	m_pSurface(0),
	m_hBink(0),
	m_paused(5)
{
}

//=====================================================================================//
//                             BinkSurface::~BinkSurface()                             //
//=====================================================================================//
BinkSurface::~BinkSurface()
{
	Release();
}

//=====================================================================================//
//                             void BinkSurface::Release()                             //
//=====================================================================================//
void BinkSurface::Release(void)
{
	if(m_pSurface)
	{
		m_pSurface->Release();
		m_pSurface = 0;
	}
	BinkClose(m_hBink);
	m_hBink = 0;
}

//=====================================================================================//
//                             bool BinkSurface::Create()                              //
//=====================================================================================//
bool BinkSurface::Create(LPDIRECTDRAW7 pDD,const char *pFileName)
{
	DDSURFACEDESC2 ddsd;

	//	����������� �������
	Release();
	//	��������� ����
	if(pDD && (m_hBink = BinkOpen(pFileName,BINKPRELOADALL)))
	{
		BinkPause(m_hBink,1);
		//	������ ��������� ����������� ��� �����
		memset(&ddsd,0,sizeof(ddsd));
		ddsd.dwSize  = sizeof(ddsd);
		ddsd.dwFlags = DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH; 
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN|DDSCAPS_SYSTEMMEMORY;//DDSCAPS_VIDEOMEMORY; 
		ddsd.dwWidth  = m_hBink->Width;
		ddsd.dwHeight = m_hBink->Height;
		//	������� �����������
		if(FAILED(pDD->CreateSurface(&ddsd,&m_pSurface,0)))
		{
			ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN|DDSCAPS_SYSTEMMEMORY; 
			pDD->CreateSurface(&ddsd,&m_pSurface,0);
		}
		if(m_pSurface)
		{
			return true;
		}
	}

	return false;
}

//=====================================================================================//
//                            void BinkSurface::SetVolume()                            //
//=====================================================================================//
void BinkSurface::SetVolume(float vol)
{
	if(m_hBink)
	{
		BinkSetVolume(m_hBink,0,static_cast<int>(32768.0f*vol));
	}
}

//=====================================================================================//
//                             void BinkSurface::Update()                              //
//=====================================================================================//
void BinkSurface::Update(void)
{
	if(m_hBink)
	{
		if(!--m_paused)
		{
			BinkPause(m_hBink,0);
		}

		if(!BinkWait(m_hBink))
		{
			BinkDoFrame(m_hBink);
			BinkNextFrame(m_hBink);
			Unpack();
		}
	}
}

//=====================================================================================//
//                             void BinkSurface::Unpack()                              //
//=====================================================================================//
void BinkSurface::Unpack(void)
{
	static DDSURFACEDESC2 ddsd;

	ddsd.dwSize = sizeof(DDSURFACEDESC2);

//Grom ��� ����
	//while(m_pSurface->Lock(0,&ddsd,0,0) == DDERR_WASSTILLDRAWING);
	HRESULT hResult;
	while( (hResult=m_pSurface->Lock(0,&ddsd,0,0)) == DDERR_WASSTILLDRAWING);

	if(FAILED(hResult))
	{//	�� ������� �������� - �������
		return ;
	}
//������ ���� �� ����

	BinkCopyToBuffer(m_hBink,
					 ddsd.lpSurface,
					 ddsd.lPitch,
					 ddsd.dwHeight,
					 0,0,
                     BinkDDSurfaceType(m_pSurface));

	m_pSurface->Unlock(0);
}

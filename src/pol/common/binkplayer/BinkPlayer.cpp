/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   20.02.2001

************************************************************************/
#include "Precomp.h"
#include "BinkPlayer.h"

BinkPlayer::Deleter BinkPlayer::m_Deleter;

//=====================================================================================//
//                              BinkPlayer::BinkPlayer()                               //
//=====================================================================================//
BinkPlayer::BinkPlayer()
:	m_hBink(0)
{
	//BinkSoundUseDirectSound(0);
}

//=====================================================================================//
//                              BinkPlayer::~BinkPlayer()                              //
//=====================================================================================//
BinkPlayer::~BinkPlayer()
{
	Close();
}

//=====================================================================================//
//                               bool BinkPlayer::Open()                               //
//=====================================================================================//
//	открыть бик файл
bool BinkPlayer::Open(const char *pFileName)
{
	static bool firstTime = true;
	if(firstTime)
	{
		//BinkSoundUseDirectSound(0);
		firstTime = false;
	}

	if((m_hBink = BinkOpen(pFileName,0)))
		return true;

	return false;
}

//=====================================================================================//
//                              void BinkPlayer::Close()                               //
//=====================================================================================//
//	закрыть бик файл
void BinkPlayer::Close(void)
{
	BinkClose(m_hBink);
	m_hBink = 0;
}

//=====================================================================================//
//                            void BinkPlayer::NextFrame()                             //
//=====================================================================================//
//	распаковать следующий фрейм
void BinkPlayer::NextFrame(void)
{
	BinkDoFrame(m_hBink);
	BinkNextFrame(m_hBink);
}

//=====================================================================================//
//                               bool BinkPlayer::Wait()                               //
//=====================================================================================//
//	синхронизация
bool BinkPlayer::Wait(void)
{
	return BinkWait(m_hBink) != 0;
}

//=====================================================================================//
//                              void BinkPlayer::Render()                              //
//=====================================================================================//
void BinkPlayer::Render(LPDIRECTDRAWSURFACE7 pBackSurface)
{
	static DDSURFACEDESC2 ddsd;

	ddsd.dwSize = sizeof(DDSURFACEDESC2);

	HRESULT hr;
	while((hr=pBackSurface->Lock(0,&ddsd,0,0)) == DDERR_WASSTILLDRAWING);
	if(FAILED(hr))
	{
		return;
	}

	BinkCopyToBuffer(m_hBink,
					 ddsd.lpSurface,
					 ddsd.lPitch,
					 ddsd.dwHeight,
					 (ddsd.dwWidth-m_hBink->Width)>>1,
					 (ddsd.dwHeight-m_hBink->Height)>>1,
                     BinkDDSurfaceType(pBackSurface));

	pBackSurface->Unlock(0);
}


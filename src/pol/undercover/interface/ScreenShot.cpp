/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description:
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)
	Date:   23.04.2000

************************************************************************/
#include "Precomp.h"
#include "ScreenShot.h"
#include <common/D3DApp/GammaControl/GammaControl.h>

bool ScreenShot(LPDIRECTDRAWSURFACE7 lpSurface,const char *pFileName)
{
	BITMAPFILEHEADER bmfh = {0x4d42,0,0,0,sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)};
	BITMAPINFOHEADER bmih = {sizeof(BITMAPINFOHEADER),0,0,1,24,0,0,0,0,0,0};
	DDSURFACEDESC2 ddsd;
	DWORD dwImageSize;
	DWORD dwStride;
	BYTE *lpImage = 0;
	DWORD mask;
	BYTE cRShiftRight = 0; 
	BYTE cGShiftRight = 0; 
	BYTE cBShiftRight = 0; 
	BYTE cRShiftLeft = 8; 
	BYTE cGShiftLeft = 8; 
	BYTE cBShiftLeft = 8; 
	FILE *stream;

	if((stream = fopen(pFileName,"wb"))!=0)
	{
		memset(&ddsd,0,sizeof(DDSURFACEDESC2));
		ddsd.dwSize		= sizeof(DDSURFACEDESC2);
		ddsd.dwFlags	= DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT|DDSD_LPSURFACE|DDSD_PITCH;
		//	��������� �����������
	    while(lpSurface->Lock(NULL,&ddsd,DDLOCK_WAIT|DDLOCK_READONLY,0) == DDERR_WASSTILLDRAWING);
		//	������ � ������
		bmih.biWidth = ddsd.dwWidth;
		bmih.biHeight = ddsd.dwHeight;
		//	������� ����� �� ������ ���� � ��������
		for(mask = ddsd.ddpfPixelFormat.dwRBitMask;mask && !(mask&1);mask>>=1)
			cRShiftRight ++;
		for(;mask;mask>>=1)
			cRShiftLeft--;
		for(mask = ddsd.ddpfPixelFormat.dwGBitMask;mask && !(mask&1);mask>>=1)
			cGShiftRight ++;
		for(;mask;mask>>=1)
			cGShiftLeft--;
		for(mask = ddsd.ddpfPixelFormat.dwBBitMask;mask && !(mask&1);mask>>=1)
			cBShiftRight ++;
		for(;mask;mask>>=1)
			cBShiftLeft--;


		//	������� ����� �������� ������
		dwStride = ((DWORD)bmih.biBitCount*bmih.biWidth)/32;
		if(((DWORD)bmih.biWidth * bmih.biBitCount) % 32)
		{
			dwStride++;
		}
		dwStride *= 4;
		dwImageSize = dwStride*bmih.biHeight;
		//	������� ����� ��������� �����
		bmfh.bfSize = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+dwImageSize;
		//	��������� ������ ��� ������� �����
		if((lpImage = new BYTE[dwImageSize]))
		{
			BYTE* pBytes = (BYTE*)ddsd.lpSurface;
			BYTE* pDstImage = lpImage;
			//	�������� ���� �� ����������� �� ��������� ����������
			for(DWORD y=0;y<bmih.biHeight;y++)
			{
					for(DWORD x=0;x<bmih.biWidth;x++)
					{
						DWORD dwPixel = 0;

						switch(ddsd.ddpfPixelFormat.dwRGBBitCount)
						{
						case 16:
							dwPixel = *((WORD*)(pBytes+(y*ddsd.lPitch))+x);
							break;
						case 24:
							dwPixel = ((DWORD)*((BYTE*)(pBytes+(y*ddsd.lPitch))+x*3));
							dwPixel += ((DWORD)*((BYTE*)(pBytes+(y*ddsd.lPitch))+x*3+1))<<8;
							dwPixel += ((DWORD)*((BYTE*)(pBytes+(y*ddsd.lPitch))+x*3+2))<<16;
							break;
						case 32:
							dwPixel = *((DWORD*)(pBytes+(y*ddsd.lPitch))+x);
							break;
						}
						//	��������� ������� ������
						BYTE r = (BYTE)((((dwPixel&ddsd.ddpfPixelFormat.dwRBitMask)>>cRShiftRight)<<cRShiftLeft));
						BYTE g = (BYTE)((((dwPixel&ddsd.ddpfPixelFormat.dwGBitMask)>>cGShiftRight)<<cGShiftLeft));
						BYTE b = (BYTE)((((dwPixel&ddsd.ddpfPixelFormat.dwBBitMask)>>cBShiftRight)<<cBShiftLeft));

						//	������� ���������� ����� � ��������� �������
						pDstImage[((bmih.biHeight-y-1)*dwStride)+(x*3)]   = b;
						pDstImage[((bmih.biHeight-y-1)*dwStride)+(x*3)+1] = g;
						pDstImage[((bmih.biHeight-y-1)*dwStride)+(x*3)+2] = r;
					}
				}
			//	���������� ���������� � ����
			fwrite(&bmfh,sizeof(BITMAPFILEHEADER),1,stream);
			fwrite(&bmih,sizeof(BITMAPINFOHEADER),1,stream);
			fwrite(lpImage,dwImageSize,1,stream);

			delete[]  lpImage;
		}

		//	������������ �����������
		lpSurface->Unlock(0);
		//  ��������� ����
		fclose(stream);
	}

	return false;
}

//*******************************************************************************************//

bool GammaScreenShot(LPDIRECTDRAWSURFACE7 lpSurface,const char *pFileName)
{
	BITMAPFILEHEADER bmfh = {0x4d42,0,0,0,sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)};
	BITMAPINFOHEADER bmih = {sizeof(BITMAPINFOHEADER),0,0,1,24,0,0,0,0,0,0};
	DDSURFACEDESC2 ddsd;
	DWORD dwImageSize;
	DWORD dwStride;
	BYTE *lpImage = 0;
	DWORD mask;
	BYTE cRShiftRight = 0; 
	BYTE cGShiftRight = 0; 
	BYTE cBShiftRight = 0; 
	BYTE cRShiftLeft = 8; 
	BYTE cGShiftLeft = 8; 
	BYTE cBShiftLeft = 8; 
	FILE *stream;

	if((stream = fopen(pFileName,"wb"))!=0)
	{
		memset(&ddsd,0,sizeof(DDSURFACEDESC2));
		ddsd.dwSize		= sizeof(DDSURFACEDESC2);
		ddsd.dwFlags	= DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT|DDSD_LPSURFACE|DDSD_PITCH;
		//	��������� �����������
	    while(lpSurface->Lock(NULL,&ddsd,DDLOCK_WAIT|DDLOCK_READONLY,0) == DDERR_WASSTILLDRAWING);
		//	������ � ������
		bmih.biWidth = ddsd.dwWidth;
		bmih.biHeight = ddsd.dwHeight;
		//	������� ����� �� ������ ���� � ��������
		for(mask = ddsd.ddpfPixelFormat.dwRBitMask;mask && !(mask&1);mask>>=1)
			cRShiftRight ++;
		for(;mask;mask>>=1)
			cRShiftLeft--;
		for(mask = ddsd.ddpfPixelFormat.dwGBitMask;mask && !(mask&1);mask>>=1)
			cGShiftRight ++;
		for(;mask;mask>>=1)
			cGShiftLeft--;
		for(mask = ddsd.ddpfPixelFormat.dwBBitMask;mask && !(mask&1);mask>>=1)
			cBShiftRight ++;
		for(;mask;mask>>=1)
			cBShiftLeft--;
		
		//	��������� �����
		DDGAMMARAMP m_GammaRamp = GammaControl::GammaRamp();
	
		//	������� ����� �������� ������
		dwStride = ((DWORD)bmih.biBitCount*bmih.biWidth)/32;
		if(((DWORD)bmih.biWidth * bmih.biBitCount) % 32)
		{
			dwStride++;
		}
		dwStride *= 4;
		dwImageSize = dwStride*bmih.biHeight;
		//	������� ����� ��������� �����
		bmfh.bfSize = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+dwImageSize;
		//	��������� ������ ��� ������� �����
		if((lpImage = new BYTE[dwImageSize]))
		{
			BYTE* pBytes = (BYTE*)ddsd.lpSurface;
			BYTE* pDstImage = lpImage;
			//	�������� ���� �� ����������� �� ��������� ����������
			for(DWORD y=0;y<bmih.biHeight;y++)
			{
					for(DWORD x=0;x<bmih.biWidth;x++)
					{
						DWORD dwPixel = 0;

						switch(ddsd.ddpfPixelFormat.dwRGBBitCount)
						{
						case 16:
							dwPixel = *((WORD*)(pBytes+(y*ddsd.lPitch))+x);
							break;
						case 24:
							dwPixel = ((DWORD)*((BYTE*)(pBytes+(y*ddsd.lPitch))+x*3));
							dwPixel += ((DWORD)*((BYTE*)(pBytes+(y*ddsd.lPitch))+x*3+1))<<8;
							dwPixel += ((DWORD)*((BYTE*)(pBytes+(y*ddsd.lPitch))+x*3+2))<<16;
							break;
						case 32:
							dwPixel = *((DWORD*)(pBytes+(y*ddsd.lPitch))+x);
							break;
						}
						//	��������� ������� ������
						BYTE r = (BYTE)((((dwPixel&ddsd.ddpfPixelFormat.dwRBitMask)>>cRShiftRight)<<cRShiftLeft));
						BYTE g = (BYTE)((((dwPixel&ddsd.ddpfPixelFormat.dwGBitMask)>>cGShiftRight)<<cGShiftLeft));
						BYTE b = (BYTE)((((dwPixel&ddsd.ddpfPixelFormat.dwBBitMask)>>cBShiftRight)<<cBShiftLeft));
						
						//	���������� �������������� �����
						r = m_GammaRamp.red[r]   >> 8;
						g = m_GammaRamp.green[g] >> 8;
						b = m_GammaRamp.blue[b]  >> 8;

						//	������� ���������� ����� � ��������� �������
						pDstImage[((bmih.biHeight-y-1)*dwStride)+(x*3)]   = b;
						pDstImage[((bmih.biHeight-y-1)*dwStride)+(x*3)+1] = g;
						pDstImage[((bmih.biHeight-y-1)*dwStride)+(x*3)+2] = r;
					}
				}
			//	���������� ���������� � ����
			fwrite(&bmfh,sizeof(BITMAPFILEHEADER),1,stream);
			fwrite(&bmih,sizeof(BITMAPINFOHEADER),1,stream);
			fwrite(lpImage,dwImageSize,1,stream);

			delete[]  lpImage;
		}

		//	������������ �����������
		lpSurface->Unlock(0);
		//  ��������� ����
		fclose(stream);
	}

	return false;
}

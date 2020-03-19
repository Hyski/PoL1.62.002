/***********************************************************************

Texture Manager Test

Copyright by MiST land 2001

------------------------------------------------------------------    
Description:
------------------------------------------------------------------    
Author: Pavel A.Duvanov
Date:   17.05.2001

************************************************************************/
#include "Precomp.h"
#include "../Image/Image.h"
#include "../Image/ImageFactory.h"
#include "DIBData.h"
#include "TextureFormatMgr.h"
#include "D3DTexture.h"
#include "../Image/DDS.h"
#include "d3dxcore.h"

//---------- ��� ���� ------------
#ifdef _HOME_VERSION
CLog textures_log;
#define textures	textures_log["textures.log"]
#else
#define textures	/##/
#endif
//--------------------------------
D3DTexture::Copy D3DTexture::m_Copy = D3DTexture::CopyImageToSurface;

//=====================================================================================//
//                              D3DTexture::D3DTexture()                               //
//=====================================================================================//
D3DTexture::D3DTexture()
:	m_Surface(0),
m_Width(0),
m_Height(0),
m_dynamic(false)
{
}

//=====================================================================================//
//                              D3DTexture::~D3DTexture()                              //
//=====================================================================================//
D3DTexture::~D3DTexture()
{
	if(m_Surface) Release();
}

//=====================================================================================//
//                              D3DTexture::D3DTexture()                               //
//=====================================================================================//
D3DTexture::D3DTexture(LPDIRECT3DDEVICE7 lpd3dDevice,
					   DIBData* pImage,			
					   int iMipMapLevel,				
					   unsigned int iQuality,			
					   bool bCompressed,			
					   bool bAlphaPremultiplied) : m_Surface(0),m_Width(0),m_Height(0), m_dynamic(false)
{
	if(iMipMapLevel == -2)
	{
		m_dynamic = true;
		iMipMapLevel = 0;
	}

	Create(lpd3dDevice,pImage,iMipMapLevel,iQuality,bCompressed,bAlphaPremultiplied);
}

//=====================================================================================//
//                              D3DTexture::D3DTexture()                               //
//=====================================================================================//
D3DTexture::D3DTexture(LPDIRECT3DDEVICE7 lpd3dDevice,
					   Image* pImage,			
					   int iMipMapLevel,				
					   unsigned int iQuality,			
					   bool bCompressed,			
					   bool bAlphaPremultiplied) : m_Surface(0),m_Width(0),m_Height(0), m_dynamic(false)
{
	if(iMipMapLevel == -2)
	{
		m_dynamic = true;
		iMipMapLevel = 0;
	}

	Create(lpd3dDevice,pImage,iMipMapLevel,iQuality,bCompressed,bAlphaPremultiplied);
}

//=====================================================================================//
//                              bool D3DTexture::Create()                              //
//=====================================================================================//
bool D3DTexture::Create(LPDIRECT3DDEVICE7 lpd3dDevice,
						DIBData* pImage,			
						int iMipMapLevel,
						unsigned int iQuality,
						bool bCompressed,
						bool bAlphaPremultiplied)
{
	if(lpd3dDevice && pImage && pImage->IsValid())
	{
		Image image;

		image.Create(pImage->Width(),pImage->Height(),&Image::ic_PixelFormat(pImage->Bpp()),0,pImage->Image());
		return Create(lpd3dDevice,&image,iMipMapLevel,iQuality,bCompressed,bAlphaPremultiplied);
	}

	return false;
}

//=====================================================================================//
//                              bool D3DTexture::Create()                              //
//=====================================================================================//
bool D3DTexture::Create(LPDIRECT3DDEVICE7 lpd3dDevice,
						Image* pImage,			
						int iMipMapLevel,
						unsigned int iQuality,
						bool bCompressed,
						bool bAlphaPremultiplied)
{
	DDSURFACEDESC2 ddsd;	//	�������� ��������

	textures("creating texture [%x]:>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>......\n",this);
	//	���� ����������� ��� ���������� - ����������� ��������� � ��� �������
	if(m_Surface) Release();
	//	�������������� ��������� � ����������
	memset(&ddsd,0,sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	//	��������� �� ���������������� ������
	if(lpd3dDevice && pImage && pImage->BitImage())
	{
		//	��������� ������� ������
		if(ChoosePixelFormat(pImage,&ddsd.ddpfPixelFormat,bCompressed,bAlphaPremultiplied))
		{
			bCompressed = static_cast<bool>(ddsd.ddpfPixelFormat.dwFourCC);
			//	���� ���������, ��������� �������� ��������
			CheckQuality(pImage,iQuality);
			//	���������� ������� �������� ��������
			if(PrepareSurfaceProperties(lpd3dDevice,&ddsd))
			{
				//	���� ��������� ������ ����������� ��������
				if(bCompressed) AlignSizeForCompressing();
				//	������������ ���������� � ����� ���-�� ��������
				CalculateMipMapLevel(pImage,&ddsd,iMipMapLevel);
				//	������� ����������� 
				if(MakeSurface(lpd3dDevice,&ddsd))
				{
					//	�������� ����������� � ����������� � ������� �������
					if(FlushImageToSurface(lpd3dDevice,&ddsd,pImage))
					{					
						textures("     SUCCESS;\n");

						return true;
					}
				}
			}
		}
	}

	if(m_Surface) Release();

	textures("     FAILED;\n");

	return false;
}

//=====================================================================================//
//                              bool D3DTexture::Create()                              //
//=====================================================================================//
bool D3DTexture::Create(LPDIRECT3DDEVICE7 lpd3dDevice,
						DDS *pImage,			
						int iMipMapLevel,
						unsigned int iQuality)
{
	assert( pImage != 0 );
	assert( lpd3dDevice != 0 );
	DDSURFACEDESC2 ddsd = pImage->GetDesc();	//	�������� ��������

	textures("creating texture [%x]:>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>......\n",this);

	//	���� ����������� ��� ���������� - ����������� ��������� � ��� �������
	if(m_Surface) Release();

	//	��������� ������� ������
	bool bCompressed = (ddsd.ddpfPixelFormat.dwFourCC != 0);

	//	���� ���������, ��������� �������� ��������
	m_Width = ddsd.dwWidth;
	m_Height = ddsd.dwHeight;

	for(; iQuality; --iQuality)
	{
		m_Width = (m_Width>>1) + (m_Width&1);
		m_Height = (m_Height>>1) + (m_Height&1);
	}

	//	���������� ������� �������� ��������
	if(PrepareSurfaceProperties(lpd3dDevice,&ddsd))
	{
		//	���� ��������� ������ ����������� ��������
		if(bCompressed) AlignSizeForCompressing();
		//	������������ ���������� � ����� ���-�� ��������
		CalculateMipMapLevel(0, &ddsd, iMipMapLevel);
		//	������� ����������� 
		//if(MakeSurface(lpd3dDevice,&ddsd))
		{
			DWORD flags = (iMipMapLevel == 0 ? D3DX_TEXTURE_NOMIPMAP : 0);
			DWORD mips = 0;
			DWORD width = m_Width, height = m_Height;
			D3DX_SURFACEFORMAT fmt = D3DXMakeSurfaceFormat(&ddsd.ddpfPixelFormat);
			HRESULT hres = D3DXCreateTexture(
				lpd3dDevice,
				&flags,
				&width,&height,
				&fmt,
				0,
				&m_Surface,
				&mips);
			assert( SUCCEEDED(hres) );
			m_Width = width;
			m_Height = height;
			hres = D3DXLoadTextureFromMemory(
				lpd3dDevice,
				m_Surface,
				D3DX_DEFAULT,
				const_cast<unsigned char *>(pImage->GetFile()->Data())+128,
				0,
				fmt,
				D3DX_DEFAULT,
				0,
				D3DX_FT_LINEAR);
			assert( SUCCEEDED(hres) );
			textures("     SUCCESS;\n");
			return true;
		}
	}

	if(m_Surface) Release();

	textures("     FAILED;\n");

	return false;
}

//=====================================================================================//
//                        bool D3DTexture::ChoosePixelFormat()                         //
//=====================================================================================//
//	��������� ������� ������ ������ �� ����������
bool D3DTexture::ChoosePixelFormat(Image* pImage,DDPIXELFORMAT* pDDPF,bool bCompressed,bool bAlphaPremultiplied)
{
	pDDPF->dwFourCC = 0;
	pDDPF->dwRGBBitCount = pImage->PixelFormat()->m_BitPerPixel;
	if(pImage->ColorMap())
	{
		switch(pImage->PixelFormat()->m_BitPerPixel)
		{
		case 1:
			pDDPF->dwFlags |= DDPF_PALETTEINDEXED1;
			break;
		case 2:
			pDDPF->dwFlags |= DDPF_PALETTEINDEXED2;
			break;
		case 4:
			pDDPF->dwFlags |= DDPF_PALETTEINDEXED4;
			break;
		case 8:
			pDDPF->dwFlags |= DDPF_PALETTEINDEXED8;
			break;
		}
	}
	else
	{
		pDDPF->dwRGBAlphaBitMask = pImage->PixelFormat()->m_ABitMask;
		pDDPF->dwRBitMask        = pImage->PixelFormat()->m_RBitMask;
		pDDPF->dwGBitMask        = pImage->PixelFormat()->m_GBitMask;
		pDDPF->dwBBitMask        = pImage->PixelFormat()->m_BBitMask;
	}
	if(bCompressed)
	{
		switch(pImage->PixelFormat()->m_BitPerPixel)
		{
		case 16:
			pDDPF->dwFourCC = MAKEFOURCC('D','X','T','1');
			break;
		default:
			pDDPF->dwFourCC = bAlphaPremultiplied?MAKEFOURCC('D','X','T','4'):MAKEFOURCC('D','X','T','5');
			break;
		}
	}

	return TextureFormatMgr::Instance()->ChoosePixelFormat(pDDPF);
}

//=====================================================================================//
//                           void D3DTexture::CheckQuality()                           //
//=====================================================================================//
void D3DTexture::CheckQuality(Image* pImage,unsigned int iQuality)
{
	textures("     check quality:\n");
	textures("	   quality level - %d;\n",iQuality);
	m_Width = pImage->Width();
	m_Height = pImage->Height();
	textures("	   inner size - [%d:%d];\n",m_Width,m_Height);
	for(int i=0;i<iQuality;i++)
	{
		if((pImage->Width()>1) && (pImage->Height()>1))
		{
			pImage->Quad();
		}
	}
	m_Width = pImage->Width();
	m_Height = pImage->Height();
	textures("	   outer size - [%d:%d];\n",m_Width,m_Height);
}

//=====================================================================================//
//                     void D3DTexture::AlignSizeForCompressing()                      //
//=====================================================================================//
void D3DTexture::AlignSizeForCompressing(void)
{
	textures("align size for compressing:\n    inner size [%d,%d]\n",m_Width,m_Height);
	//	���� ������� �������� � ������� �������
	if(m_Width%4) m_Width += 4-(m_Width%4);
	if(m_Height%4) m_Height += 4-(m_Height%4);
	textures("    ...multipled for 4 pixels: [%d]:[%d]\n",m_Width,m_Height);
}

//=====================================================================================//
//                     bool D3DTexture::PrepareSurfaceProperties()                     //
//=====================================================================================//
bool D3DTexture::PrepareSurfaceProperties(LPDIRECT3DDEVICE7 lpd3dDevice,LPDDSURFACEDESC2 lpDDSD)
{
	D3DDEVICEDESC7 ddDesc;

	textures("prepare surface properties:\n");
	if(FAILED(lpd3dDevice->GetCaps(&ddDesc)))
		return false;
	//	�������� ������ �������� �� ����������� ����������
	m_Width = std::min(static_cast<DWORD>(m_Width),ddDesc.dwMaxTextureWidth);
	m_Height = std::min(static_cast<DWORD>(m_Height),ddDesc.dwMaxTextureHeight);
	textures("    ...device max texture size: [%d]:[%d]\n",ddDesc.dwMaxTextureWidth,ddDesc.dwMaxTextureHeight);
	textures("    ...align texture size for max available: [%d]:[%d]\n",m_Width,m_Height);
	//	��������� ���������� ����������
	lpDDSD->dwFlags         = DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|
		DDSD_PIXELFORMAT|DDSD_TEXTURESTAGE;
	lpDDSD->ddsCaps.dwCaps  = DDSCAPS_TEXTURE;
	lpDDSD->dwWidth         = m_Width;
	lpDDSD->dwHeight        = m_Height;
	// Turn on D3DTexture management for hardware devices
	if(ddDesc.deviceGUID == IID_IDirect3DHALDevice )
	{
		lpDDSD->ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
		textures("    ...set DDSCAPS2_TEXTUREMANAGE flag;\n");
	}
	else
	{
		if(ddDesc.deviceGUID == IID_IDirect3DTnLHalDevice )
		{
			lpDDSD->ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
			textures("    ...set DDSCAPS2_TEXTUREMANAGE flag;\n");
		}
		else
		{
			lpDDSD->ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
			textures("    ...set DDSCAPS_SYSTEMMEMORY flag;\n");
		}
	}

	if(m_dynamic)
	{
		lpDDSD->ddsCaps.dwCaps |= DDSCAPS_WRITEONLY;
		lpDDSD->ddsCaps.dwCaps2 |= DDSCAPS2_HINTDYNAMIC;
		//lpDDSD->ddsCaps.dwCaps2 &= ~DDSCAPS2_TEXTUREMANAGE;
	}

	//	���� ��������� ������������� ������� ������� ������� ������ 2
	if(ddDesc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_POW2)
	{
		textures("    ...D3DPTEXTURECAPS_POW2 availble;\n");
		for(lpDDSD->dwWidth=1;m_Width>lpDDSD->dwWidth;lpDDSD->dwWidth<<=1);
		for(lpDDSD->dwHeight=1;m_Height>lpDDSD->dwHeight;lpDDSD->dwHeight<<=1);
		textures("    ...after align: [%d]:[%d]\n",lpDDSD->dwWidth,lpDDSD->dwHeight);
		//	�������� ������ �������� �� ����������� ����������
		lpDDSD->dwWidth = std::min(lpDDSD->dwWidth,ddDesc.dwMaxTextureWidth);
		lpDDSD->dwHeight = std::min(lpDDSD->dwHeight,ddDesc.dwMaxTextureHeight);
		textures("    ...align texture size for max available: [%d]:[%d]\n",lpDDSD->dwWidth,lpDDSD->dwHeight);
	}
	//	���� ��������� ������������� ������� ������ ����� ������
	if(ddDesc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_SQUAREONLY)
	{
		textures("    ...D3DPTEXTURECAPS_SQUAREONLY availble;\n");
		if(lpDDSD->dwWidth > lpDDSD->dwHeight)
		{
			lpDDSD->dwHeight = lpDDSD->dwWidth;
		}
		else
		{
			lpDDSD->dwWidth  = lpDDSD->dwHeight;
		}
		textures("    ...after align: [%d]:[%d]\n",lpDDSD->dwWidth,lpDDSD->dwHeight);
	}
	//	���������� ����� ������ ��������
	m_Width = lpDDSD->dwWidth;
	m_Height = lpDDSD->dwHeight;

	return true;
}

//=====================================================================================//
//                       void D3DTexture::CalculateMipMapLevel()                       //
//=====================================================================================//
void D3DTexture::CalculateMipMapLevel(Image *, LPDDSURFACEDESC2 lpDDSD, int iMipMapLevel)
{
	int media;
	int level;

	lpDDSD->dwWidth = m_Width;
	lpDDSD->dwHeight = m_Height;
	textures("CALCULATE MIPMAP LEVEL:\n     innner mipmap level - [%d]\n",iMipMapLevel);
	//	��� ������ ������ ��� ������
	media = std::min(m_Width,m_Height);
	for(level = 0;(1<<level)<media;level++);
	textures("     find mipmap level: [%d];\n",level);
	if(iMipMapLevel != -1) level = std::max(0,std::min(level,iMipMapLevel));
	textures("     FINAL mipmap level: [%d];\n",level);
	if(level)
	{
		lpDDSD->dwFlags |= DDSD_MIPMAPCOUNT;
		lpDDSD->ddsCaps.dwCaps  |= DDSCAPS_MIPMAP|DDSCAPS_COMPLEX;
		lpDDSD->dwMipMapCount   = level;
	}
}

//=====================================================================================//
//                           bool D3DTexture::MakeSurface()                            //
//=====================================================================================//
bool D3DTexture::MakeSurface(LPDIRECT3DDEVICE7 lpd3dDevice,LPDDSURFACEDESC2 lpDDSD)
{
	HRESULT hres = S_OK;
	CComPtr<IDirectDrawSurface7> lpddsRender;
	CComPtr<IDirectDraw7> lpDD;

	textures("    <-- create new surface -->\n");
	//	�������� �����������
	hres = lpd3dDevice->GetRenderTarget(&lpddsRender);
	assert( SUCCEEDED(hres) );
	hres = lpddsRender->GetDDInterface((VOID**)&lpDD);
	assert( SUCCEEDED(hres) );

	//	������� ����� ����������� ��� ��������
	if(FAILED(lpDD->CreateSurface(lpDDSD,&m_Surface,NULL)))
	{
		textures("    ...can't create new surface\n");
		return false;
	}

	textures("    ...new surface created;\n");
	return true;
}

//=====================================================================================//
//                       bool D3DTexture::FlushImageToSurface()                        //
//=====================================================================================//
bool D3DTexture::FlushImageToSurface(LPDIRECT3DDEVICE7 lpd3dDevice,LPDDSURFACEDESC2 lpDDSD,Image *pImage)
{
	CComPtr<IDirectDrawSurface7> lpddsRender;
	CComPtr<IDirectDraw7> lpDD;
	IDirectDrawSurface7 *lpddsDest = m_Surface;
	DDSURFACEDESC2		 ddsd;					//	������������ ��� ������ ������������

	//	�������� ��������� �� ������ DirectDraw
	lpd3dDevice->GetRenderTarget(&lpddsRender);
	lpddsRender->GetDDInterface((VOID**)&lpDD);
	lpddsRender.Release();

	//	��������� ��� ������������ ������� Copy
	m_Copy = (pImage->PixelFormat()->m_BitPerPixel == 32)?UseBlitting:UseStretching;

	//	���� ��������� ������� ������ ��������
	if(lpDDSD->ddpfPixelFormat.dwFourCC)
	{
		//	��������� ���������� ������� ������ ��� �� ������ ��������
		memcpy(&ddsd,lpDDSD,sizeof(DDSURFACEDESC2));
		textures("    ...using compressed textures;\n");
		if(!ChoosePixelFormat(pImage,&ddsd.ddpfPixelFormat,false,false))
		{
			textures("    ...can find suitable surface for using compressed textures;\n");
			return false;
		}
		//	������� ������ �����
		ddsd.dwFlags        &= ~DDSD_MIPMAPCOUNT;
		ddsd.ddsCaps.dwCaps &= ~(DDSCAPS_MIPMAP|DDSCAPS_COMPLEX);
		ddsd.dwMipMapCount   = 0;
	}

	//	������� �������
	int level = lpDDSD->dwMipMapCount?lpDDSD->dwMipMapCount:1;

	for(int i=0;i<level;i++)
	{
		textures("IMAGE SIZE: %dx%d\n",lpDDSD->dwWidth,lpDDSD->dwHeight);

		if(lpDDSD->ddpfPixelFormat.dwFourCC)
		{
			ddsd.dwWidth = lpDDSD->dwWidth;
			ddsd.dwHeight = lpDDSD->dwHeight;
			if(!UseCompressing(lpDD,&ddsd,pImage,lpddsDest,lpDDSD->dwWidth,lpDDSD->dwHeight))
			{
				m_Surface.Release();
				return false;
			}
		}
		else
		{
			if(!m_Copy(lpDD,lpDDSD,pImage,lpddsDest))
			{
				m_Surface.Release();
				return false;
			}

		}

		if(i == level-1) break;

		//	!!! � ����� ������� ��������???

		lpDDSD->dwWidth >>= 1;
		lpDDSD->dwHeight >>= 1;

		pImage->Quad();

		DDSCAPS2 ddsCaps;
		ddsCaps.dwCaps  = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP;
		ddsCaps.dwCaps2 = 0;
		ddsCaps.dwCaps3 = 0;
		ddsCaps.dwCaps4 = 0;

		if(SUCCEEDED(lpddsDest->GetAttachedSurface(&ddsCaps,&lpddsDest)))
		{
			lpddsDest->Release();
		}
	}

	return true;
}

//=====================================================================================//
//                          bool D3DTexture::UseCompressing()                          //
//=====================================================================================//
bool D3DTexture::UseCompressing(LPDIRECTDRAW7 lpDD,LPDDSURFACEDESC2 lpDDSD,Image *pImage,LPDIRECTDRAWSURFACE7 pSurface,DWORD /*dwDstWidth*/,DWORD /*dwDstHeight*/)
{
	CComPtr<IDirectDrawSurface7> lpTempSurface;

	// Create a new surface for the D3DTexture
	if(!FAILED(lpDD->CreateSurface(lpDDSD,&lpTempSurface,NULL)))
	{
		if(m_Copy(lpDD,lpDDSD,pImage,lpTempSurface))
		{
			//	���������� Flip
			HRESULT hres = pSurface->BltFast(0,0,lpTempSurface,0,DDBLTFAST_WAIT);
			assert( SUCCEEDED(hres) );
		}

		return true;
	}

	return false;
}

//=====================================================================================//
//                          bool D3DTexture::UseStretching()                           //
//=====================================================================================//
bool D3DTexture::UseStretching(LPDIRECTDRAW7 /*lpDD*/,LPDDSURFACEDESC2 lpDDSD,Image *pImage,LPDIRECTDRAWSURFACE7 pSurface)
{
	HDC hSurfaceDC;

	// Get a DC for the surface
	if(FAILED(pSurface->GetDC(&hSurfaceDC)))
	{
		textures("- FAILED: Can't get DC from lpSurface;\n");
		return FALSE;
	}
	//	�������� ��������
	textures("Attached D3DTexture: %dx%d\n",lpDDSD->dwWidth,lpDDSD->dwHeight);
	ImageFactory::StretchImage(hSurfaceDC,pImage,
		0,0,lpDDSD->dwWidth,lpDDSD->dwHeight,
		0,0,pImage->Width(),pImage->Height());
	pSurface->ReleaseDC(hSurfaceDC);

	return true;
}

//=====================================================================================//
//                             void D3DTexture::Release()                              //
//=====================================================================================//
void D3DTexture::Release(void)
{
	if(m_Surface)
	{
		m_Surface.Release();
		m_Surface = 0;
		m_Width = m_Height = 0;
		textures("[%x] released;\n",this);
	}
}

//=====================================================================================//
//                        bool D3DTexture::CopyImageToSurface()                        //
//=====================================================================================//
bool D3DTexture::CopyImageToSurface(LPDIRECTDRAW7 lpDD,LPDDSURFACEDESC2 lpDDSD,Image *pImage,LPDIRECTDRAWSURFACE7 pSurface)
{
	if(!UseBlitting(lpDD,lpDDSD,pImage,pSurface))
	{//	Using Stretch
		textures(" - UseBlitting FAILED\n");
		if(!UseStretching(lpDD,lpDDSD,pImage,pSurface))
		{
			textures("- FAILED: Can't Stretch Bitmap To Surface;\n");
			return false;
		}
	}

	return true;
}

//=====================================================================================//
//                           bool D3DTexture::UseBlitting()                            //
//=====================================================================================//
bool D3DTexture::UseBlitting(LPDIRECTDRAW7 lpDD,LPDDSURFACEDESC2 lpDDSD,Image *pImage,LPDIRECTDRAWSURFACE7 pSurface)
{
	DDSURFACEDESC2 ddsd;
	CComPtr<IDirectDrawSurface7> lpTempSurface;

	if((pImage->PixelFormat()->m_BitPerPixel == 32) && 
		(lpDDSD->ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS))
	{
		textures("-- UseBlitting;\n");
		// Setup the new surface desc
		ddsd.dwSize = sizeof(DDSURFACEDESC2);
		pSurface->GetSurfaceDesc(&ddsd);
		ddsd.dwFlags	     = DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT|DDSD_TEXTURESTAGE;
		ddsd.ddsCaps.dwCaps  = DDSCAPS_TEXTURE|DDSCAPS_SYSTEMMEMORY;
		ddsd.ddsCaps.dwCaps2 = 0L;
		ddsd.dwWidth         = pImage->Width();
		ddsd.dwHeight        = pImage->Height();
		// Create a new surface for the D3DTexture
		if(FAILED(lpDD->CreateSurface(&ddsd,&lpTempSurface,NULL)))
		{
			return false;
		}

		//	����� ��������� ��������
		HRESULT hResult;
		while((hResult = lpTempSurface->Lock(NULL,&ddsd,0,0)) == DDERR_WASSTILLDRAWING);

		if(FAILED(hResult))
		{//	�� ������� �������� - �������
			textures("!!! LOCK FAILED !!!");
			return false;
		}

		BYTE* pBytes = (BYTE*)ddsd.lpSurface;
		DWORD* lpImage = (DWORD *)pImage->BitImage();

		DWORD dwRMask = ddsd.ddpfPixelFormat.dwRBitMask;
		DWORD dwGMask = ddsd.ddpfPixelFormat.dwGBitMask;
		DWORD dwBMask = ddsd.ddpfPixelFormat.dwBBitMask;
		DWORD dwAMask = ddsd.ddpfPixelFormat.dwRGBAlphaBitMask;

		DWORD dwRShiftL = 8, dwRShiftR = 0;
		DWORD dwGShiftL = 8, dwGShiftR = 0;
		DWORD dwBShiftL = 8, dwBShiftR = 0;
		DWORD dwAShiftL = 8, dwAShiftR = 0;

		DWORD dwMask;
		for(dwMask=dwRMask;dwMask && !(dwMask&0x1);dwMask>>=1) dwRShiftR++;
		for(;dwMask;dwMask>>=1) dwRShiftL--;

		for(dwMask=dwGMask;dwMask && !(dwMask&0x1);dwMask>>=1) dwGShiftR++;
		for(;dwMask;dwMask>>=1) dwGShiftL--;

		for(dwMask=dwBMask;dwMask && !(dwMask&0x1);dwMask>>=1) dwBShiftR++;
		for(;dwMask;dwMask>>=1) dwBShiftL--;

		for(dwMask=dwAMask;dwMask && !(dwMask&0x1);dwMask>>=1) dwAShiftR++;
		for(;dwMask;dwMask>>=1) dwAShiftL--;

		for(int y=(ddsd.dwHeight-1);y>=0;y--)
		{
			DWORD* pDstData32 = (DWORD*)pBytes;
			WORD*  pDstData16 = (WORD*)pBytes;

			for(DWORD x=0;x<ddsd.dwWidth;x++)
			{
				DWORD dwPixel = lpImage[y*ddsd.dwWidth+x];

				BYTE r = (BYTE)((dwPixel>>16)&0x000000ff);
				BYTE g = (BYTE)((dwPixel>> 8)&0x000000ff);
				BYTE b = (BYTE)((dwPixel>> 0)&0x000000ff);
				BYTE a = (BYTE)((dwPixel>>24)&0x000000ff);

				DWORD dr = ((r>>(dwRShiftL))<<dwRShiftR)&dwRMask;
				DWORD dg = ((g>>(dwGShiftL))<<dwGShiftR)&dwGMask;
				DWORD db = ((b>>(dwBShiftL))<<dwBShiftR)&dwBMask;
				DWORD da = ((a>>(dwAShiftL))<<dwAShiftR)&dwAMask;

				if( 32 == ddsd.ddpfPixelFormat.dwRGBBitCount )
					pDstData32[x] = (DWORD)(dr+dg+db+da);
				else
					pDstData16[x] = (WORD)(dr+dg+db+da);
			}

			pBytes += ddsd.lPitch;
		}

		hResult = lpTempSurface->Unlock(0);
		assert( SUCCEEDED(hResult) );

		// Copy the temp surface to the real Texture surface
		hResult = pSurface->Blt(NULL,lpTempSurface,NULL,DDBLT_WAIT,NULL);
		assert( SUCCEEDED(hResult) );
		return true;
	}

	return false;
}


/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description:
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)

************************************************************************/
#include "precomp.h"
#include "D3DKernel.h"
#include <common/cmdline/cmdline.h>
#include <common/surfacemgr/surfacemgr.h>

//=====================================================================================//
//                              struct SelectZFormatInfo                               //
//=====================================================================================//
struct SelectZFormatInfo
{
	enum DepthStencil
	{
		DepthOnly		=	0,
		DepthAndStencil	=	1
	};

	DDPIXELFORMAT format;

	DepthStencil getDepthStencil() const
	{
		if((format.dwFlags&DDPF_ZBUFFER) && (format.dwFlags&DDPF_STENCILBUFFER))
		{
			return DepthAndStencil;
		}
		else
		{
			return DepthOnly;
		}
	}

	bool operator<(const SelectZFormatInfo &r) const
	{
		if(getDepthStencil() != r.getDepthStencil()) return getDepthStencil() < r.getDepthStencil();
		if(format.dwZBufferBitDepth != r.format.dwZBufferBitDepth) return format.dwZBufferBitDepth < r.format.dwZBufferBitDepth;
		return false;
	}
};

typedef std::vector<SelectZFormatInfo> ZFormats_t;

HRESULT CALLBACK EnumZBufferCallback(LPDDPIXELFORMAT lpDDPixFmt, LPVOID lpContext);

//	---------- Лог файл ------------
#ifdef _DEBUG_GRAPHICS
CLog d3dkernel_log;
#define d3dkernel	d3dkernel_log["d3dkernel.log"]
#else
#define d3dkernel	/##/
#endif
//	--------------------------------	

typedef void (*LPUPDATEFRAME)(void);

namespace D3DKernel
{
	CComPtr<IDirectDraw7>			m_lpDD;
	CComPtr<IDirectDrawSurface7>	m_lpddsPrimary;
	CComPtr<IDirectDrawSurface7>	m_lpddsBackBuffer;
	CComPtr<IDirectDrawSurface7>	m_lpddsZBuffer;
	CComPtr<IDirect3D7>				m_lpD3D;
	CComPtr<IDirect3DDevice7>		m_lpd3dDevice;
	HWND							m_hPrevWnd = NULL;

	//	-----------------------------------
	RECT							m_rcScreenRect = {0,0,0,0};
	SIZE							m_szSize = {0,0};
	BOOL							m_bTripleBuffering = false;

	//	-----------------------------------
	LPUPDATEFRAME m_lpUpdateFrame;
//	BOOL m_bFullScreen = FALSE;
	//	------------- ФУНКЦИИ ---------------------
	BOOL Init3DWindow(HWND hWnd,
					  GUID* lpDriverGUID,
					  GUID* lpDeviceGUID,
					  int width,
					  int height);
	BOOL Init3DFullScreen(HWND hWnd,
						  GUID* lpDriverGUID,
						  GUID* lpDeviceGUID,
						  int width,
						  int height,
						  int depth,
						  int freq);
	BOOL AddingZBuffer(GUID* lpDeviceGUID, unsigned int desiredDepth);
}

//	---------------- Инициализация --------------------

//=====================================================================================//
//                               BOOL D3DKernel::Init()                                //
//=====================================================================================//
BOOL D3DKernel::Init(HWND hWnd,
					 GUID* lpDriverGUID,
					 GUID* lpDeviceGUID,
					 int width,
					 int height,
					 int depth,
					 int freq,
					 bool bWindowed)
{
	Release();
	//	запоминаем новое окно
	m_hPrevWnd = hWnd;
	//	инициализируем режим
	d3dkernel("--- D3DKernel initialization ---\n");
//	d3dkernel("    ...driver guid: {%x-%x-%x-...}\n",lpDriverGUID->Data1,lpDriverGUID->Data2,lpDriverGUID->Data3);
	d3dkernel("    ...driver & device pointers to guid: (%x)-(%x)\n",lpDriverGUID,lpDeviceGUID);
//	d3dkernel("    ...device guid: {%x-%x-%x-...}\n",lpDeviceGUID->Data1,lpDeviceGUID->Data2,lpDeviceGUID->Data3);

	BOOL result = FALSE;
	if(bWindowed)
	{//	Windowed mode
		m_lpUpdateFrame = Blt;
		result = Init3DWindow(hWnd,lpDriverGUID,lpDeviceGUID,width,height);
		//if(result) m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_ZENABLE,D3DZB_USEW);
	}
	else
	{//	Fullscreen mode
		m_lpUpdateFrame = Flip;
		result = Init3DFullScreen(hWnd,lpDriverGUID,lpDeviceGUID,width,height,depth,freq);
		//if(result) m_lpd3dDevice->SetRenderState(D3DRENDERSTATE_ZENABLE,D3DZB_USEW);
	}

	HRESULT hres = S_OK;
	const char *picname = "pictures/interface/hourglass.tga";
	CComPtr<IDirectDrawSurface7> surf = SurfaceMgr::Instance()->Surface(picname);
	DDSURFACEDESC2 ddsd2;
	ddsd2.dwSize = sizeof(ddsd2);
	hres = surf->GetSurfaceDesc(&ddsd2);
	assert( SUCCEEDED(hres) );
	RECT rect = {0,0,ddsd2.dwWidth,ddsd2.dwHeight};
	hres = GetD3DDevice()->Clear(0,NULL,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,0,1.0f,0);
	assert( SUCCEEDED(hres) );
	hres = GetD3DDevice()->BeginScene();
	assert( SUCCEEDED(hres) );
	hres = GetBB()->BltFast((width - ddsd2.dwWidth)/2,(height - ddsd2.dwHeight)/2,surf,&rect,DDBLTFAST_WAIT);
	assert( SUCCEEDED(hres) );
	hres = GetD3DDevice()->EndScene();
	assert( SUCCEEDED(hres) );
	UpdateFrame();
	SurfaceMgr::Instance()->Release(picname);

	return result;
}

//=====================================================================================//
//                              void D3DKernel::Release()                              //
//=====================================================================================//
void D3DKernel::Release(void)
{
	//	необходимо переключить окно в нормальный режим если оно в fullscreen'е
	if(m_lpDD && IsFullScreen() && m_hPrevWnd)
	{
		if(!FAILED(m_lpDD->SetCooperativeLevel(m_hPrevWnd,DDSCL_NORMAL)))
		{
			m_hPrevWnd = NULL;
			d3dkernel("     RESTORE DDSCL_NORMAL COOPERATIVE LEVEL!!!\n");
		}
	}
	//	освобождаем ресурсы
	d3dkernel("--- Releasing 3D ---\n");
	m_lpd3dDevice.Release();
	d3dkernel("    ...Direct3D Device released;\n");
	m_lpD3D.Release();
	d3dkernel("    ...Direct3D Object released;\n");
	m_lpddsZBuffer.Release();
	d3dkernel("    ...Z-Buffer released;\n");
	m_lpddsBackBuffer.Release();
	d3dkernel("    ...Back Buffer released;\n");
	m_lpddsPrimary.Release();
	d3dkernel("    ...Primary Surface released;\n");
	m_lpDD.Release();
	d3dkernel("    ...DirectDraw Object released;\n");
}

//=====================================================================================//
//                            void D3DKernel::UpdateFrame()                            //
//=====================================================================================//
//	---------------- Переключение страниц ---------------------
void D3DKernel::UpdateFrame(void)
{
	m_lpUpdateFrame();
}

//=====================================================================================//
//                          void D3DKernel::RestoreSurfaces()                          //
//=====================================================================================//
void D3DKernel::RestoreSurfaces(void)
{
    // Check/restore the primary surface
    if(m_lpddsPrimary)
	{
        if(m_lpddsPrimary->IsLost())
            m_lpddsPrimary->Restore();
	}
    // Check/restore the back buffer
    if(m_lpddsBackBuffer)
	{
        if(m_lpddsBackBuffer->IsLost())
            m_lpddsBackBuffer->Restore();
	}
    // Check/restore the ZBuffer surface
    if(m_lpddsZBuffer)
	{
        if(m_lpddsZBuffer->IsLost())
            m_lpddsZBuffer->Restore();
	}
}

//=====================================================================================//
//                           void D3DKernel::MoveViewPort()                            //
//=====================================================================================//
void D3DKernel::MoveViewPort(int x,int y)
{
	m_rcScreenRect.right = x+(m_rcScreenRect.right-m_rcScreenRect.left); 
	m_rcScreenRect.bottom = y+(m_rcScreenRect.bottom-m_rcScreenRect.top); 
	m_rcScreenRect.left = x; 
	m_rcScreenRect.top = y; 
}

//=====================================================================================//
//                           BOOL D3DKernel::AddingZBuffer()                           //
//=====================================================================================//
BOOL D3DKernel::AddingZBuffer(GUID *lpDeviceGUID, unsigned int desiredDepth)
{
	HRESULT hres = S_OK;
	CComPtr<IDirectDrawSurface7> zbuf;
	DDSURFACEDESC2 ddsd;
	DDPIXELFORMAT ddpfZBuffer;
	
	d3dkernel("=--> CD3DWindow::AddingZBuffer enter;\n");
	ZFormats_t fmtvec;
	hres = m_lpD3D->EnumZBufferFormats(*lpDeviceGUID, EnumZBufferCallback,(VOID*)&fmtvec);
	assert( SUCCEEDED(hres) );

	size_t i = 0;
	for(i = 0; i < fmtvec.size(); ++i)
	{
		if(fmtvec[i].format.dwZBufferBitDepth == desiredDepth)
		{
			break;
		}
	}

	if(i == fmtvec.size())
	{
		return FALSE;
	}

	ddpfZBuffer = fmtvec[i].format;

	if(sizeof(DDPIXELFORMAT) != ddpfZBuffer.dwSize)
	{
        return FALSE;
	}

	memset(&ddsd,0,sizeof(DDSURFACEDESC2));
	ddsd.dwSize			=  sizeof(DDSURFACEDESC2);
    ddsd.dwFlags        = DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT|DDSD_PIXELFORMAT;
    ddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER;
	ddsd.dwWidth        = m_rcScreenRect.right - m_rcScreenRect.left;
	ddsd.dwHeight       = m_rcScreenRect.bottom - m_rcScreenRect.top;
    memcpy(&ddsd.ddpfPixelFormat,&ddpfZBuffer,sizeof(DDPIXELFORMAT));

	if(IsEqualGUID(*lpDeviceGUID,IID_IDirect3DHALDevice) || 
	   IsEqualGUID(*lpDeviceGUID,IID_IDirect3DTnLHalDevice))
	{
		ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
		d3dkernel("    ...Z-Buffer in DDSCAPS_VIDEOMEMORY;\n");
	}
	else
	{
		ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
		d3dkernel("    ...Z-Buffer in DDSCAPS_SYSTEMMEMORY;\n");
	}

    if(FAILED(m_lpDD->CreateSurface(&ddsd,&zbuf,NULL)))
	{
		return FALSE;
	}

	d3dkernel("    ...create Z-Buffer - [OK];\n");
    if(FAILED(m_lpddsBackBuffer->AddAttachedSurface(zbuf)))
	{
		return FALSE;
	}

	d3dkernel("    ...add Z-Buffer to Back Buffer - [OK];\n");
	m_lpddsZBuffer = zbuf;

	return TRUE;
}

//=====================================================================================//
//                       HRESULT CALLBACK EnumZBufferCallback()                        //
//=====================================================================================//
HRESULT CALLBACK EnumZBufferCallback(LPDDPIXELFORMAT lpDDPixFmt,LPVOID lpContext)
{
	ZFormats_t *fmtvec = static_cast<ZFormats_t*>(lpContext);
	SelectZFormatInfo info = {*lpDDPixFmt};
	ZFormats_t::iterator i = std::lower_bound(fmtvec->begin(),fmtvec->end(),info);
	fmtvec->insert(i,info);
	return D3DENUMRET_OK;

	//d3dkernel("- calling EnumZBufferCallback;\n");
	//if(lpDDPixFmt->dwFlags == DDPF_ZBUFFER && lpDDPixFmt->dwZBufferBitDepth == info->desiredDepth)
 //   {
 //       memcpy(info->format,lpDDPixFmt,sizeof(DDPIXELFORMAT)); 
 //       // Return with D3DENUMRET_CANCEL to end the search.
	//	d3dkernel("- return D3DENUMRET_CANCEL;\n");
 //       return D3DENUMRET_CANCEL;
 //   } 
 //   // Return with D3DENUMRET_OK to continue the search.
	//d3dkernel("- return D3DENUMRET_OK;\n");
 //   return D3DENUMRET_OK;
}

  

//=====================================================================================//
//                           BOOL D3DKernel::Init3DWindow()                            //
//=====================================================================================//
BOOL D3DKernel::Init3DWindow(HWND hWnd,
							 GUID* lpDriverGUID,
							 GUID* lpDeviceGUID,
							 int width,
							 int height)
{
	POINT pt;
	HRESULT hres = S_OK;
	DDSURFACEDESC2 ddsd;
	LPDIRECTDRAWCLIPPER pcClipper;
	d3dkernel("---> enter CInitD3DImWindowed::Init3D();\n");
	d3dkernel("--- Set Windowed Mode ---\n");
	d3dkernel("window size: %dx%d;\n",width,height);

	d3dkernel("--- Trying To Initialization ---\n");
	m_rcScreenRect.left = 0;
	m_rcScreenRect.top = 0;
	m_rcScreenRect.right = m_szSize.cx = width;
	m_rcScreenRect.bottom = m_szSize.cy = height;
	pt.x = m_rcScreenRect.left;
	pt.y = m_rcScreenRect.top;

	ClientToScreen(hWnd,&pt);
	m_rcScreenRect.left = pt.x;
	m_rcScreenRect.top = pt.y;
	pt.x = m_rcScreenRect.right;
	pt.y = m_rcScreenRect.bottom;

	ClientToScreen(hWnd,&pt);
	m_rcScreenRect.right = pt.x;
	m_rcScreenRect.bottom = pt.y;

	if(!FAILED(DirectDrawCreateEx(lpDriverGUID, (LPVOID*)&m_lpDD,IID_IDirectDraw7,NULL)))
	{
		d3dkernel("    ...create DirectDraw Object - [OK];\n");

		if(!FAILED(m_lpDD->SetCooperativeLevel(hWnd,DDSCL_NORMAL|DDSCL_FPUSETUP)))
		{
			d3dkernel("    ...set cooperative level ( DDSCL_NORMAL|DDSCL_FPUSETUP ) - [OK];\n");
			memset(&ddsd,0,sizeof(DDSURFACEDESC2));
			ddsd.dwSize = sizeof(DDSURFACEDESC2);
			ddsd.dwFlags        = DDSD_CAPS;
			ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
			// Create the primary surface.
			if(!FAILED(m_lpDD->CreateSurface(&ddsd,&m_lpddsPrimary,NULL)))
			{
				d3dkernel("    ...create Primary Surface - [OK];\n");

				DDPIXELFORMAT pixf;
				ZeroMemory(&pixf,sizeof(pixf));
				pixf.dwSize = sizeof(pixf);
				hres = m_lpddsPrimary->GetPixelFormat(&pixf);

				//	create back buffer
				ddsd.dwFlags        = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
				ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE;
				ddsd.dwWidth  = width;
				ddsd.dwHeight = height;
				
				if(!FAILED(m_lpDD->CreateSurface(&ddsd,&m_lpddsBackBuffer,NULL)))
				{
					d3dkernel("    ...create Back Buffer - [OK];\n");
					if(!FAILED(m_lpDD->CreateClipper(0,&pcClipper,NULL)))
					{
						d3dkernel("    ...create Clipper - [OK];\n");
						hres = pcClipper->SetHWnd(0,hWnd);
						assert( SUCCEEDED(hres) );
						hres = m_lpddsPrimary->SetClipper(pcClipper);
						assert( SUCCEEDED(hres) );
						pcClipper->Release();
						if(!FAILED(m_lpDD->QueryInterface(IID_IDirect3D7,(VOID**)&m_lpD3D)))
						{
							d3dkernel("    ...query interface ( IID_IDirect3D7 ) - [OK];\n");
							if(AddingZBuffer(lpDeviceGUID,pixf.dwRGBBitCount))
							{
								if(!FAILED(m_lpD3D->CreateDevice(*lpDeviceGUID,
									m_lpddsBackBuffer,
									&m_lpd3dDevice)))
								{
									d3dkernel("    ...create Direct3D Device - [OK];\n");
									// Create the viewport
									D3DVIEWPORT7 vp = {0,0,width,height,0.0f,1.0f};
									if(!FAILED(m_lpd3dDevice->SetViewport(&vp)))
									{
										d3dkernel("    ...create viewport - [OK];\n");
										return TRUE;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	d3dkernel("---> leave CInitD3DImWindowed::Init3D() - FALSE;\n");

	return FALSE;
}

//=====================================================================================//
//                         BOOL D3DKernel::Init3DFullScreen()                          //
//=====================================================================================//
BOOL D3DKernel::Init3DFullScreen(HWND hWnd,
								 GUID* lpDriverGUID,
								 GUID* lpDeviceGUID,
								 int width,
								 int height,
								 int depth,
								 int freq)
{
	DDSURFACEDESC2 ddsd;
	d3dkernel("---> enter CInitD3DImFullScreen::Init3D();\n");
	d3dkernel("--- Set Fullscreen Mode ---\n");
	d3dkernel("fullscreen mode: %dx%dx%d;\n",width,height,depth);

	d3dkernel("--- Trying To Initialization ---\n");
	m_rcScreenRect.left = 0;
	m_rcScreenRect.top = 0;
	m_rcScreenRect.right = m_szSize.cx = width;
	m_rcScreenRect.bottom = m_szSize.cy = height;
	d3dkernel("    - lpDriverGUID (%x) DeviceGUID (%x);\n",lpDriverGUID,*lpDeviceGUID);
	if(!FAILED(DirectDrawCreateEx(lpDriverGUID, (LPVOID*)&m_lpDD,IID_IDirectDraw7,NULL)))
	{
		d3dkernel("    ...create DirectDraw Object - [OK];\n");
	    // Query DirectDraw for access to Direct3D
		if(!FAILED(m_lpDD->QueryInterface(IID_IDirect3D7,(VOID**)&m_lpD3D)))
		{
			d3dkernel("    ...query interface ( IID_IDirect3D7 ) - [OK];\n");
			if(!FAILED(m_lpDD->SetCooperativeLevel(hWnd,DDSCL_FULLSCREEN|DDSCL_EXCLUSIVE|DDSCL_FPUSETUP)))
			{
				d3dkernel("    ...set cooperative level ( DDSCL_FULLSCREEN|DDSCL_EXCLUSIVE|DDSCL_FPUSETUP ) - [OK];\n");
				if(!FAILED(m_lpDD->SetDisplayMode(width,height,depth,freq,0L)))
				{
					ZeroMemory(&ddsd,sizeof(DDSURFACEDESC2));
					ddsd.dwSize = sizeof(DDSURFACEDESC2);
					ddsd.dwFlags           = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
					ddsd.ddsCaps.dwCaps    = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | 
											 DDSCAPS_COMPLEX | DDSCAPS_3DDEVICE;
					ddsd.ddsCaps.dwCaps2 = DDSCAPS2_HINTANTIALIASING; 
					ddsd.dwBackBufferCount = m_bTripleBuffering?2:1;

					// Create the primary surface.
					if(!FAILED(m_lpDD->CreateSurface(&ddsd,&m_lpddsPrimary,NULL)))
					{
						d3dkernel("    ...create Primary Surface - [OK];\n");
						// Get a ptr to the back buffer, which will be our render target
						DDSCAPS2 ddscaps = {DDSCAPS_BACKBUFFER,0,0,0};
						if(!FAILED(m_lpddsPrimary->GetAttachedSurface(&ddscaps,&m_lpddsBackBuffer)))
						{
							d3dkernel("    ...get attached surface: Back Buffer - [OK];\n");
							if(AddingZBuffer(lpDeviceGUID,depth))
							{
								if(!FAILED(m_lpD3D->CreateDevice(*lpDeviceGUID,
									m_lpddsBackBuffer,
									&m_lpd3dDevice)))
								{
									d3dkernel("    ...create Direct3D Device - [OK];\n");
									// Create the viewport
									D3DVIEWPORT7 vp = {0,0,width,height,0.0f,1.0f};
									if(!FAILED(m_lpd3dDevice->SetViewport(&vp)))
									{
										d3dkernel("    ...create viewport - [OK];\n");
										return TRUE;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	d3dkernel("---> enter CInitD3DImFullScreen::Init3D() - FALSE;\n");

	return FALSE;
}

//=====================================================================================//
//                                void D3DKernel::Blt()                                //
//=====================================================================================//
void D3DKernel::Blt(void)
{
	if(m_lpddsPrimary->Blt(&m_rcScreenRect,m_lpddsBackBuffer, 
                        NULL,DDBLT_WAIT,NULL)==DDERR_SURFACELOST)
	{
		d3dkernel("    ...DDERR_SURFACELOST!\n");
		RestoreSurfaces();
	}
}

//=====================================================================================//
//                               void D3DKernel::Flip()                                //
//=====================================================================================//
void D3DKernel::Flip(void)
{
	static const bool novsync = CmdLine::IsKey("-novsync");
	static const unsigned long flg = novsync ? DDFLIP_NOVSYNC : 0;
	
	if(m_lpddsPrimary->Flip(NULL,DDFLIP_WAIT|flg)==DDERR_SURFACELOST)
	{
		d3dkernel("    ...DDERR_SURFACELOST!\n");
		if(SUCCEEDED(m_lpDD->TestCooperativeLevel()))
		{
			RestoreSurfaces();
		}
	}
}

//=====================================================================================//
//                          LPDIRECTDRAW7 D3DKernel::GetDD()                           //
//=====================================================================================//
LPDIRECTDRAW7 D3DKernel::GetDD(void)
{
	return m_lpDD;
}

//=====================================================================================//
//                       LPDIRECTDRAWSURFACE7 D3DKernel::GetPS()                       //
//=====================================================================================//
LPDIRECTDRAWSURFACE7 D3DKernel::GetPS(void)
{
	return m_lpddsPrimary;
}

//=====================================================================================//
//                       LPDIRECTDRAWSURFACE7 D3DKernel::GetBB()                       //
//=====================================================================================//
LPDIRECTDRAWSURFACE7 D3DKernel::GetBB(void)
{
	return m_lpddsBackBuffer;
}

//=====================================================================================//
//                           LPDIRECT3D7 D3DKernel::GetD3D()                           //
//=====================================================================================//
LPDIRECT3D7 D3DKernel::GetD3D(void)
{
	return m_lpD3D;
}

//=====================================================================================//
//                     LPDIRECT3DDEVICE7 D3DKernel::GetD3DDevice()                     //
//=====================================================================================//
LPDIRECT3DDEVICE7 D3DKernel::GetD3DDevice(void)
{
	return m_lpd3dDevice;
}

//=====================================================================================//
//                                int D3DKernel::ResX()                                //
//=====================================================================================//
int D3DKernel::ResX(void)
{
	return m_szSize.cx;
}

//=====================================================================================//
//                                int D3DKernel::ResY()                                //
//=====================================================================================//
int D3DKernel::ResY(void)
{
	return m_szSize.cy;
}

//=====================================================================================//
//                           BOOL D3DKernel::IsFullScreen()                            //
//=====================================================================================//
BOOL D3DKernel::IsFullScreen(void)
{
	return (m_lpUpdateFrame == Flip);
}

//=====================================================================================//
//                             void D3DKernel::SetTriple()                             //
//=====================================================================================//
void D3DKernel::SetTriple(BOOL bTriple)
{
	m_bTripleBuffering = bTriple;
}

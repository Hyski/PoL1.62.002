/***********************************************************************

                           Texture Manager Test

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   17.05.2001

************************************************************************/
#ifndef _D3D_TEXTURE_H_
#define _D3D_TEXTURE_H_

class DDS;
class DIBData;
class Image;

//=====================================================================================//
//                                  class D3DTexture                                   //
//=====================================================================================//
class D3DTexture
{
	ATL::CComPtr<IDirectDrawSurface7> m_Surface;
	int m_Width;
	int m_Height;
	bool m_dynamic;

private:
	typedef bool (*Copy)(LPDIRECTDRAW7 lpDD,LPDDSURFACEDESC2 lpDDSD,Image *pImage,LPDIRECTDRAWSURFACE7 pSurface);
	static Copy m_Copy;

public:
	D3DTexture();
	D3DTexture(LPDIRECT3DDEVICE7 lpd3dDevice,
			   DIBData* pImage,			
			   int iMipMapLevel = -1,				//	-1 ��� mipmap'�, 0 - �� �����
			   unsigned int iQuality = 0,			//	�������� �������, 0 - ������ ��������			
			   bool bCompressed = false,			//	������������ ������ ��������
			   bool bAlphaPremultiplied = false);	//	��� ������������� ������ �������,
	D3DTexture(LPDIRECT3DDEVICE7 lpd3dDevice,
			   Image* pImage,			
			   int iMipMapLevel = -1,				//	-1 ��� mipmap'�, 0 - �� �����
			   unsigned int iQuality = 0,			//	�������� �������, 0 - ������ ��������			
			   bool bCompressed = false,			//	������������ ������ ��������
			   bool bAlphaPremultiplied = false);	//	��� ������������� ������ �������,
	virtual ~D3DTexture();
public:
	bool Create(LPDIRECT3DDEVICE7 lpd3dDevice,
				DIBData* pImage,			
				int iMipMapLevel = -1,				//	-1 ��� mipmap'�, 0 - �� �����
			    unsigned int iQuality = 0,			//	�������� �������, 0 - ������ ��������			
				bool bCompressed = false,			//	������������ ������ ��������
				bool bAlphaPremultiplied = false);	//	��� ������������� ������ �������,
	bool Create(LPDIRECT3DDEVICE7 lpd3dDevice,
				Image* pImage,			
				int iMipMapLevel = -1,				//	-1 ��� mipmap'�, 0 - �� �����
			    unsigned int iQuality = 0,			//	�������� �������, 0 - ������ ��������			
				bool bCompressed = false,			//	������������ ������ ��������
				bool bAlphaPremultiplied = false);	//	��� ������������� ������ �������,
	bool Create(LPDIRECT3DDEVICE7 lpd3dDevice,
				DDS *dds,
				int iMipMapLevel = -1,
				unsigned int iQuality = 0);
	void Release(void);
public:
	LPDIRECTDRAWSURFACE7 Surface(void) const;
	int Width(void) const;
	int Height(void) const;
private:
	bool ChoosePixelFormat(Image* pImage,DDPIXELFORMAT* pDDPF,bool bCompressed,bool bAlphaPremultiplied);
	void CheckQuality(Image* pImage,unsigned int iQuality);
	void AlignSizeForCompressing(void);
	bool PrepareSurfaceProperties(LPDIRECT3DDEVICE7 lpd3dDevice,LPDDSURFACEDESC2 lpDDSD);
	void CalculateMipMapLevel(Image* pImage,LPDDSURFACEDESC2 lpDDSD,int iMipMapLevel);
	bool MakeSurface(LPDIRECT3DDEVICE7 lpd3dDevice,LPDDSURFACEDESC2 lpDDSD);
	bool FlushImageToSurface(LPDIRECT3DDEVICE7 lpd3dDevice,LPDDSURFACEDESC2 lpDDSD,Image *pImage);
	bool UseCompressing(LPDIRECTDRAW7 lpDD,LPDDSURFACEDESC2 lpDDSD,Image *pImage,LPDIRECTDRAWSURFACE7 pSurface,DWORD dwDstWidth,DWORD dwDstHeight);
public:
	//	������� ��� ������ ������ 
	static bool CopyImageToSurface(LPDIRECTDRAW7 lpDD,LPDDSURFACEDESC2 lpDDSD,Image *pImage,LPDIRECTDRAWSURFACE7 pSurface);
	//	������������� ��������� �������� � ������������ �������� (�������� ������ ��� 32 ���)
	static bool UseBlitting(LPDIRECTDRAW7 lpDD,LPDDSURFACEDESC2 lpDDSD,Image *pImage,LPDIRECTDRAWSURFACE7 pSurface);
	//	������������� HDC ������������ (�� �������� � ����� �������)
	static bool UseStretching(LPDIRECTDRAW7 lpDD,LPDDSURFACEDESC2 lpDDSD,Image *pImage,LPDIRECTDRAWSURFACE7 pSurface);
};

inline LPDIRECTDRAWSURFACE7 D3DTexture::Surface(void) const
{
	return m_Surface;
}

inline int D3DTexture::Width(void) const
{
	return m_Width;
}

inline int D3DTexture::Height(void) const
{
	return m_Height;
}

#endif	//_D3D_TEXTURE_H_
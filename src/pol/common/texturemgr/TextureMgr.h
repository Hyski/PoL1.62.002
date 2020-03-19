/***********************************************************************

                             Texture Manager

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   16.05.2001

************************************************************************/
#ifndef _TEXTURE_MGR_H_
#define _TEXTURE_MGR_H_

class D3DTexture;
class DIBData;

//=====================================================================================//
//                                  class TextureMgr                                   //
//=====================================================================================//
class TextureMgr
{
	class Reference;
public:
	struct Description
	{
		int m_Width;
		int m_Height;
		int m_Bpp;
		int m_FourCC;
		int m_MipMapCount;
		std::string m_Name;
	};

private:
	struct Deleter 
	{
	public:
		TextureMgr *m_pInstance;
	public:
		Deleter():m_pInstance(0){}
		~Deleter(){if(m_pInstance) delete m_pInstance;}
	};
	friend Deleter;
	static Deleter m_Deleter;

private:
	enum IMAGE_TYPE {IT_BITMAP,IT_JPEG,IT_TARGA,IT_UNKNOWN};

private:
	typedef std::hash_map<std::string,Reference*> TexMap_t;

	LPDIRECT3DDEVICE7 m_lpd3dDevice;
	int m_DefaultMipMapLevel;
	int m_DefaultQuality;
	bool m_UseCompressing;
	bool m_UsePremultipliedAlpha;

	TexMap_t m_Textures;		//	����� ������ �� ��������

private:
	TextureMgr();
	~TextureMgr();

public:
	//	�������������
	void Init(LPDIRECT3DDEVICE7 lpd3dDevice,	//	D3DDevice ��� �������� �������
			  int iDefaultMipMapLevel,			//	������� �������� ��� ������� �� ���������
			  unsigned int iDefaultQuality,		//	�������� ��� ������� �� ���������
			  bool bCompressed,					//	������������ ����������������� ��������
			  bool bAlphaPremultiplied);		//	������������ premultiplied ����� ��� ����������������� �������
	//	�������� �������� � ���������� ��  � ������ (����������� ���-�� ������ �� ��������)
	LPDIRECTDRAWSURFACE7 Texture(const char *pTextureName);
	LPDIRECTDRAWSURFACE7 Texture(const char *pTextureName,
								 int iMipMapLevel,
								 unsigned int iQuality);
	//	�������� �������� (����������� ���-�� ������ �� ��������)
	LPDIRECTDRAWSURFACE7 CreateTexture(const char *pTextureName,
									   DIBData *pImage,
									   int iMipMapLevel,
									   unsigned int iQuality);
	//	������������ ��������, ��������� � ������������ ��������� (���������� ���-�� ���������� ������)
	int Release(const char* pTextureName);
	//	������������ ���� ��������
	void Release(void);

public:	//	������� ������������
	//	���-�� ����������� �������
	int Quantity(void) const;	
	//  �� ��� � ������� ������������
	int Bads(void);				
	//	���-�� ���������� ������
	int Memory(void);
	//	�������� ������ ���������� � ���������
	void Report(std::vector<Description>& info);

private:
	bool LoadImageToSurface(D3DTexture *texture, const char *pImageName,int iMipMapLevel,unsigned int iQuality);
	bool LoadDDSToSurface(D3DTexture *texture, VFile *image, const char *pImageName, int iMipMapLevel, unsigned int iQuality);
//	TextureMgr::Reference* LoadImageToSurface(const char* pImageName,int iMipMapLevel,unsigned int iQuality);
	TextureMgr::IMAGE_TYPE GetImageType(const char* pFileExtension);
public:
	static TextureMgr *Instance(void);
};

inline TextureMgr* TextureMgr::Instance(void)
{
	if(!m_Deleter.m_pInstance) m_Deleter.m_pInstance = new TextureMgr();
	return m_Deleter.m_pInstance;
}

//***************************************************************//
// class TextureMgr::Reference
class TextureMgr::Reference
{
	std::auto_ptr<D3DTexture> m_Value;
	int m_Counter;	

public:	
	Reference(D3DTexture* value);
	~Reference();

public:	
	D3DTexture* Get(void) { m_Counter++; return m_Value.get(); }
	int Release(void){if(m_Counter){m_Counter--;} return m_Counter;}
	int Counter(void) const {return m_Counter;}
	bool IsBad(void);
};

#endif	//_TEXTURE_MGR_H_
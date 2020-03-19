/***********************************************************************

                             Texture Manager

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   17.05.2001

************************************************************************/
#ifndef _TEXTURE_FORMAT_MGR_H_
#define _TEXTURE_FORMAT_MGR_H_

enum COMPRESSED_FORMAT
{
	CF_NONE,
	CF_DXT1,	// ������ ��� ARGB:1555 (��� ����� ��� 1 ��� �� �����)
	CF_DXT2,	// ������ ��� ARGB:4444 (����� ����� premultiplied) 
	CF_DXT3,	// ������ ��� ARGB:4444 (����� ����� non premultiplied)
	CF_DXT4,	// ������ ��� ARGB:8888 (����������������� ����� premultiplied)
	CF_DXT5		// ������ ��� ARGB:8888 (����������������� ����� non premultiplied) 
};

enum PALETTE_FORMAT
{
	PF_NONE = 0,
	PF_1BIT = 1,
	PF_2BIT = 2,
	PF_4BIT = 4,
	PF_8BIT = 8
};

struct PixelFormat
{
	DWORD m_AlphaBitCount;		//	���-�� ��� �� �����
	DWORD m_RedBitCount;		//	���-�� ��� �� �������
	DWORD m_GreenBitCount;		//	���-�� ��� �� �������
	DWORD m_BlueBitCount;		//	���-�� ��� �� �����
	DDPIXELFORMAT m_ddpf;		//	���������� ������ ������
	COMPRESSED_FORMAT m_cf;		//	��� ����������, ���� �����������
	PALETTE_FORMAT m_pf;		//	������ �������
	PixelFormat();
	PixelFormat(LPDDPIXELFORMAT pDDPF);
};

class TextureFormatMgr
{
private:
	struct Deleter 
	{
	public:
		TextureFormatMgr *m_pInstance;
	public:
		Deleter():m_pInstance(0){}
		~Deleter(){if(m_pInstance) delete m_pInstance;}
	};
	friend Deleter;
	static Deleter m_Deleter;
private:
	std::vector<PixelFormat> m_vPFs;
private:
	TextureFormatMgr();
	virtual ~TextureFormatMgr();
public:
	void Init(LPDIRECT3DDEVICE7 lpd3dDevice);
	void Release(void);
	//	�������� ������� ������ ������� � ���������
	bool ChoosePixelFormat(DDPIXELFORMAT* pDDPF);
public:
	static const char* GetFourCCName(DWORD dwFourCC);
	static COMPRESSED_FORMAT GetFourCCValue(DWORD dwFourCC);
	static PALETTE_FORMAT GetPaletteIndexed(DWORD dwFlags);
public:
	static TextureFormatMgr *Instance(void);
};

inline TextureFormatMgr* TextureFormatMgr::Instance(void)
{
	if(!m_Deleter.m_pInstance) m_Deleter.m_pInstance = new TextureFormatMgr();
	return m_Deleter.m_pInstance;
}

#endif	//_TEXTURE_FORMAT_MGR_H_
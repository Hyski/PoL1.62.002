/***********************************************************************

                             Texture Manager

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   17.05.2001

************************************************************************/
#ifndef _IMAGE_H_
#define _IMAGE_H_

//**********************************************************************//
//	class Image
class Image
{
public:
	class ic_PixelFormat;
	class ic_ColorMap;
protected:
	ic_PixelFormat* m_PixelFormat;
	ic_ColorMap* m_ColorMap;				//	�������
	unsigned char* m_BitImage;				//	������� �����
	int m_Size;								//	������ �������� ������ � ������
	int m_Stride;							//	���-�� ���� � ������
	int m_Width;							//	������ � ��������
	int m_Height;							//	������ � ��������
public:
	Image();
	Image(const Image& image);
	virtual ~Image();
public:
	//	�������� �����������
	const Image& operator=(const Image& image);
	//	������� ������ ����������� ��������� ������� � ������������ ������� �������� � ��������
	bool Create(int width,int height,const ic_PixelFormat* pf,const ic_ColorMap* color_map = 0,const unsigned char* bit_image = 0);
	//	��������� �������� ��������
	bool Quad(void);
	//	�������� ��� ������
	void Release(void);
protected:
	//	������������ ������� �������� ������
	void EstimateImageSize(void);
	//	��������� �������� �������� (��� �������) 
	void QuadPalette(const Image& src_image);
	//	��������� �������� �������� (��� ARGB) 
	void QuadTrueColor(const Image& src_image);
public:
	//	�������� ���������� � ������� ��������
	const ic_PixelFormat* PixelFormat(void) const;
	//	�������� ���������� � �������
	const ic_ColorMap* ColorMap(void) const;
	//	�������� ������ � �������� ������
	unsigned char* BitImage(void) const;
	//	�������� ������ �������� ������
	int Size(void) const;
	//	�������� ����� ������ � ������
	int Stride(void) const;
	//	�������� ������ ����������� � ��������
	int Width(void) const;
	//	�������� ������ ����������� � ��������
	int Height(void) const;
};

//	�������� ���������� � ������� ��������
inline const Image::ic_PixelFormat* Image::PixelFormat(void) const
{
	return m_PixelFormat;
}
//	�������� ���������� � �������
inline const Image::ic_ColorMap* Image::ColorMap(void) const
{
	return m_ColorMap;
}
//	�������� ������ � �������� ������
inline unsigned char* Image::BitImage(void) const
{
	return m_BitImage;
}
//	�������� ������ �������� ������
inline int Image::Size(void) const
{
	return m_Size;
}
//	�������� ����� ������ � ������
inline int Image::Stride(void) const
{
	return m_Stride;
}
//	�������� ������ ����������� � ��������
inline int Image::Width(void) const
{
	return m_Width;
}
//	�������� ������ ����������� � ��������
inline int Image::Height(void) const
{
	return m_Height;
}

//**********************************************************************
//	class Image::ic_PixelFormat
class Image::ic_PixelFormat
{
public:
	enum TYPE
	{
		T_ARGB888,
		T_ARGB444,
		T_ARGB555,
		T_RGB888,
		T_RGB565,
		T_RGB555,
		T_NONE
	};
public:
	unsigned int m_BitPerPixel;
	unsigned int m_ABitMask;
	unsigned int m_RBitMask;
	unsigned int m_GBitMask;
	unsigned int m_BBitMask;
public:
	ic_PixelFormat();
	ic_PixelFormat(const ic_PixelFormat& pf);
	ic_PixelFormat(unsigned int bpp);
	ic_PixelFormat(unsigned int bpp,TYPE type);
	ic_PixelFormat(unsigned int bpp,
				   unsigned int abitmask,
				   unsigned int rbitmask,
				   unsigned int gbitmask,
				   unsigned int bbitmask);
	virtual ~ic_PixelFormat();
public:
	//	�������� �����������
	const ic_PixelFormat& operator=(const ic_PixelFormat& pf);
	//	���������� ������� ������
	void Set(unsigned int bpp,TYPE type);
	//	�������� ������� ������
	TYPE Get(void) const;
	//	�������� ���-�� ��� � ����������
	int BitCount(unsigned int mask) const;
	//	�������� �������� ������ �� ����
	int GetShiftR(unsigned int mask) const;
};

//	�������� ���-�� ��� � ����������
inline int Image::ic_PixelFormat::BitCount(unsigned int mask) const
{
	int counter = 0;
	for(;mask && !(mask&0x1);mask>>=1);
	for(;mask;mask>>=1) counter++; 
	return counter;
}
//	�������� �������� ������ �� ����
inline int Image::ic_PixelFormat::GetShiftR(unsigned int mask) const
{
	int counter = 0;
	for(;mask && !(mask&0x1);mask>>=1) counter++;
	return counter;
}

inline bool operator == (const Image::ic_PixelFormat& a,const Image::ic_PixelFormat& b)
{
	return (a.m_BitPerPixel == b.m_BitPerPixel) &&
	       (a.m_ABitMask == b.m_ABitMask) &&
	       (a.m_RBitMask == b.m_RBitMask) &&
	       (a.m_GBitMask == b.m_GBitMask) &&
	       (a.m_BBitMask == b.m_BBitMask);
}
//**********************************************************************
//	class Image::ic_ColorMap
class Image::ic_ColorMap
{
public:
	Image::ic_PixelFormat* m_PixelFormat;	//	������ �������
	unsigned char* m_Map;					//	������� ��� ������������� ��������
	int m_Size;								//	������ ������ �������
public:
	ic_ColorMap();
	ic_ColorMap(const ic_ColorMap& cm);
	ic_ColorMap(const Image::ic_PixelFormat& pf,const unsigned char* data,int size);
	virtual ~ic_ColorMap();
public:
	//	�������� �����������
	const ic_ColorMap& operator=(const ic_ColorMap& cm);
	//	���������� �������
	void Set(const Image::ic_PixelFormat& pf,const unsigned char* data,int size);
public:
	//	�������� ������� ������
	const ic_PixelFormat* PixelFormat(void) const;
	//	�������� ����� ������
	const unsigned char* Map(void) const;
	//	�������� ������ ����� ������
	int Size(void) const;
};

//	�������� ������� ������
inline const Image::ic_PixelFormat* Image::ic_ColorMap::PixelFormat(void) const
{
	return m_PixelFormat;
}
//	�������� ����� ������
inline const unsigned char* Image::ic_ColorMap::Map(void) const
{
	return m_Map;
}
//	�������� ������ ����� ������
inline int Image::ic_ColorMap::Size(void) const
{
	return m_Size;
}

#endif	//_IMAGE_H_
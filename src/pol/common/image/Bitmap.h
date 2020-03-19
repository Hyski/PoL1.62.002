/***********************************************************************

                                Image Test

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   17.05.2001

************************************************************************/
#ifndef _BITMAP_H_
#define _BITMAP_H_

#include "Image.h"

#pragma pack(push,1)

class Bitmap : public Image
{
	struct BitmapFileHeader;
	struct BitmapInfoHeader;
public:
	Bitmap();
	Bitmap(const Bitmap& image);
	Bitmap(const Image& image);
	virtual ~Bitmap();
public:
	//	�������� �����������
	const Bitmap& operator=(const Image& image);
	//	��������� �������� �� �����
	bool Load(const char* file_name);
	//	��������� �������� �� ������
	bool Load(const unsigned char* data,int size);
	//	��������� �������� � ������� bmp
	bool Save(const char* file_name);
private:
	bool ReadUncompressBitmap(const BitmapInfoHeader* bmih,const unsigned char* data,int offbits);
	bool BitFieldBitmap(const BitmapInfoHeader* bmih,const unsigned char* data,int offbits);
	bool ReadRLE8Bitmap(const BitmapInfoHeader* bmih,const unsigned char* data,int offbits);
	bool ReadRLE4Bitmap(const BitmapInfoHeader* bmih,const unsigned char* data,int offbits);

};

struct Bitmap::BitmapFileHeader
{
    unsigned short m_Type; 
    unsigned int   m_Size; 
    unsigned short m_Reserved1; 
    unsigned short m_Reserved2; 
    unsigned int   m_OffBits; 
};

struct Bitmap::BitmapInfoHeader
{
    unsigned int   m_Size; 
    int			   m_Width; 
    int			   m_Height; 
    unsigned short m_Planes; 
    unsigned short m_BitCount;
    unsigned int   m_Compression; 
    unsigned int   m_SizeImage; 
    int			   m_XPelsPerMeter; 
    int			   m_YPelsPerMeter; 
    unsigned int   m_ClrUsed; 
    unsigned int   m_ClrImportant;
};

#pragma pack(pop)


#endif	//_BITMAP_H_
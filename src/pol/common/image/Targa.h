/***********************************************************************

                              Image Library

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   22.05.2001

************************************************************************/
#ifndef _TARGA_H_
#define _TARGA_H_

#include "Image.h"

#pragma pack(push,1)

class Targa : public Image
{
	struct TargaHeader;
public:
	Targa();
	Targa(const Targa& image);
	Targa(const Image& image);
	virtual ~Targa();
public:
	//	�������� �����������
	const Targa& operator=(const Image& image);
	//	��������� �������� �� �����
	bool Load(const char* file_name);
	//	��������� �������� �� ������
	bool Load(const unsigned char* data,int size);
private:
	bool ReadUncompressTarga(TargaHeader* tga,const unsigned char* data);
	void ReadUncompressData(const unsigned char* data,bool luc);
};

struct Targa::TargaHeader
{
	unsigned char  m_IDLength;		//	����� ��������������
	unsigned char  m_ColorMapType;	//	RGB or Palletize
	unsigned char  m_ImageType;		//	��� ��������
	unsigned short m_FirstEntry;	
	unsigned short m_ColorMapLen;	
	unsigned char  m_ColMapEntSize;		
	unsigned short m_XOrigin;
	unsigned short m_YOrigin;
	unsigned short m_ImageWidth;
	unsigned short m_ImageHeight;
	unsigned char  m_PixelDepth;
	unsigned char  m_ImageDescriptor;
};

#pragma pack(pop)

#endif	//_TARGA_H_
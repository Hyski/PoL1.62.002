/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   25.05.2001

************************************************************************/
#ifndef _JPEG_H_
#define _JPEG_H_

#include "Image.h"

#pragma pack(push,1)

class Jpeg : public Image
{
public:
	Jpeg();
	Jpeg(const Jpeg& image);
	Jpeg(const Image& image);
	virtual ~Jpeg();
public:
	//	�������� �����������
	const Jpeg& operator=(const Image& image);
	//	��������� �������� �� �����
	bool Load(const char* file_name);
	//	��������� �������� �� ������
	bool Load(const unsigned char* data,int size);
};

#pragma pack(pop)

#endif	//_JPEG_H_
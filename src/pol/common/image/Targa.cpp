/***********************************************************************

                              Image Library

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   22.05.2001

************************************************************************/
#include "Precomp.h"
#include "Targa.h"

Targa::Targa() : Image()
{
}

Targa::Targa(const Targa& image) : Image(image)
{
}

Targa::Targa(const Image& image) : Image(image)
{
}

Targa::~Targa()
{
}
//	�������� �����������
const Targa& Targa::operator=(const Image& image)
{
	(*this).Image::operator=(image);

	return (*this);
}
  
bool Targa::Load(const char* file_name)
{
	FILE* hFile;
	int length;
	unsigned char* buff;
	bool ok = false;

	if(file_name && (*file_name))
	{
		if(hFile = fopen(file_name,"rb"))
		{
			fseek(hFile,0,SEEK_END);
			length = ftell(hFile);
			fseek(hFile,0,SEEK_SET);
			buff = new unsigned char[length];
			fread(buff,length,1,hFile);
			fclose(hFile);
			ok = Load(buff,length);
			delete[] buff;
		}
	}

	return ok;
}

bool Targa::Load(const unsigned char* data,int /*size*/)
{
	TargaHeader tga;

	if(m_BitImage) Release();

	// ��������� ��������� TARGA_HEADER
	memcpy(&tga,data,sizeof(TargaHeader));
	//	������ � ������ �������� ������
	m_Width  = tga.m_ImageWidth;
	m_Height = tga.m_ImageHeight;
	// ��������� ������ �����������
	switch(tga.m_ImageType)
	{
	case 0:	//	��� �������� ������
		break;
	case 1:	//	�������� ����������� � ��������
	case 2: //	�������� ����������� ��� ��������
		return ReadUncompressTarga(&tga,data+sizeof(TargaHeader)+tga.m_IDLength);
	case 3: //	�������� �����-����� �����������
	case 9: //	RLE ��������� ����������� � ��������
	case 10: //	RLE ��������� ����������� ��� ��������
	case 11: //	������ �����-����� �����������
		break;
	}

	return false;
}

bool Targa::ReadUncompressTarga(TargaHeader* tga,const unsigned char* data)
{
	if(tga->m_ColorMapType)
	{//	���������� �������
		m_PixelFormat->Set(tga->m_PixelDepth,ic_PixelFormat::T_NONE);
		//	���������� �������
		ic_PixelFormat pf;
		switch(tga->m_ColMapEntSize)
		{
		case 16:
			pf.Set(tga->m_ColMapEntSize,ic_PixelFormat::T_RGB555);
			break;
		case 24:
			pf.Set(tga->m_ColMapEntSize,ic_PixelFormat::T_RGB888);
			break;
		case 32:
			pf.Set(tga->m_ColMapEntSize,ic_PixelFormat::T_ARGB888);
			break;
		}
		m_ColorMap = new ic_ColorMap(pf,data,tga->m_ColorMapLen*(pf.m_BitPerPixel/8));
		data += tga->m_ColorMapLen*(pf.m_BitPerPixel/8); 
	}
	else
	{//	���������� ������ �������
		switch(tga->m_PixelDepth)
		{
		case 16:
			m_PixelFormat->Set(tga->m_PixelDepth,ic_PixelFormat::T_RGB555);
			break;
		case 24:
			m_PixelFormat->Set(tga->m_PixelDepth,ic_PixelFormat::T_RGB888);
			break;
		case 32:
			m_PixelFormat->Set(tga->m_PixelDepth,ic_PixelFormat::T_ARGB888);
			break;
		}
	}
	//	������������ ������� � �������
	EstimateImageSize();
	if(m_Size)
	{
		//	�������� ������ ��� ������� �����
		m_BitImage = new unsigned char[m_Size];
		//	�������� ������
		ReadUncompressData(data,(tga->m_ImageDescriptor&32)>0);

		return true;
	}

	return false;
}

void Targa::ReadUncompressData(const unsigned char* data,bool luc)
{
	unsigned char* pByte = m_BitImage;
	unsigned int len = m_Width*(m_PixelFormat->m_BitPerPixel/8);
	int stride = luc?(-m_Stride):m_Stride;

	if(luc) pByte += m_Stride*(m_Height-1);
	for(int i=0;i<m_Height;i++)
	{
		memcpy(pByte,data,len);
		data  += len;
		pByte += stride;
	}
}


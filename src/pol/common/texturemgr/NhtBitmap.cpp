#include "Precomp.h"
#include "../Utils/VFile.h"
#include "NhtBitmap.h"

NhtBitmap::NhtBitmap() : DIBData()
{
	InZero();
}

NhtBitmap::NhtBitmap(const char *pFileName) : DIBData()
{
	InZero();
	Load(pFileName);
}

NhtBitmap::NhtBitmap(FILE* hFile) : DIBData()
{
	InZero();
	Load(hFile);
}

NhtBitmap::NhtBitmap(LPBYTE pData,DWORD dwSize) : DIBData()
{
	InZero();
	Load(pData,dwSize);
}

NhtBitmap::NhtBitmap(VFile *pVFile) : DIBData()
{
	InZero();
	Load(pVFile);
}

NhtBitmap::~NhtBitmap()
{
	Release();
}

BOOL NhtBitmap::Load(const char *pFileName)
{
	FILE* hFile;

	if((hFile = fopen(pFileName,"rb"))!=NULL)
	{
		if(Load(hFile))
		{
			fclose(hFile);
			return TRUE;
		}
		fclose(hFile);
	}

	return FALSE;
}

BOOL NhtBitmap::Load(FILE *hFile)
{
	LPBYTE pData = NULL;
	DWORD dwFileSize;

	//	�������� ����� �����
	fseek(hFile,0L,SEEK_END);
	dwFileSize = ftell(hFile);
	rewind(hFile);
	//	������������ ������
	pData = new BYTE[dwFileSize];
	if(pData)
	{
		//	��������� ������
		if(fread(pData,dwFileSize,1,hFile))
		{
			if(Load(pData,dwFileSize))		
			{
				delete[] pData;
				return TRUE;
			}
		}
		delete[] pData;
	}

	return FALSE;
}

BOOL NhtBitmap::Load(LPBYTE pData,DWORD dwSize)
{
	VFile vFile(pData,dwSize);

	return Load(&vFile);
}

BOOL NhtBitmap::Load(VFile *pVFile)
{
	BITMAPFILEHEADER bmfh;

	Release();
	//	���������� ��������� ����� �� ������
	pVFile->Rewind();
	// ��������� ��������� BITMAPFILEHEADER
	if(!pVFile->Read(&bmfh,sizeof(BITMAPFILEHEADER)))
		return FALSE;
	if(bmfh.bfType != 0x4d42)  // ������� BM
		return FALSE;
	// ������ ��������� BITMAPINFOHEADER � ������� ������
	m_dwBitmapInfoSize = bmfh.bfOffBits - sizeof(BITMAPFILEHEADER);
	// �������� ������ ��� ��������� BITMAPINFOHEADER � ������� ������
	m_lpBMIH = (LPBITMAPINFOHEADER) new char[m_dwBitmapInfoSize];
	if(!m_lpBMIH)
		return FALSE;
	// ��������� �� ����� ��������� BITMAPINFOHEADER � ������� ������
	if(!pVFile->Read(m_lpBMIH,m_dwBitmapInfoSize))
		return FALSE;
	// �������� �� ����������
	if(m_lpBMIH->biSize>16)
		if(m_lpBMIH->biCompression!=0)
			return FALSE;
	// ����������� �������
	ComputeMetrics();
	// �������� ������ ��� ������� �����
	m_lpImage = (LPBYTE) new char[m_dwSizeImage];
	if(!m_lpImage)
		return FALSE;
	// ��������� ������� ����� �� �����
	if(!pVFile->Read(m_lpImage,m_dwSizeImage))
		return FALSE;

	return m_bValid = TRUE;
}
/*
void NhtBitmap::ComputeMetrics(void)
{
	// ������� ���������� ������� ���� � ����������� ������� ����
	DWORD dwBytes = ((DWORD) m_lpBMIH->biWidth * m_lpBMIH->biBitCount) / 32;
	// ����������� ���� �� 4 �������� �����
	if(((DWORD) m_lpBMIH->biWidth * m_lpBMIH->biBitCount) % 32)
	{
		dwBytes++;
	}
	dwBytes *= 4;
	m_dwStride = dwBytes;
	m_dwSizeImage = dwBytes * m_lpBMIH->biHeight; 
	// ������������� ��������� �� ������� ������ �������
	m_lpColorTable = (LPBYTE) m_lpBMIH + m_lpBMIH->biSize;
}
*/

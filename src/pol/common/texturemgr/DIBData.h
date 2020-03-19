/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description:
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)
    Date:   16.05.2000

************************************************************************/
#ifndef _DIB_DATA_H_
#define _DIB_DATA_H_

class DIBData
{
protected:
	BOOL m_bValid;					//	������������� �� ��������
protected:	
	LPBITMAPINFOHEADER m_lpBMIH;	//	��������� �������� ������
	LPBYTE m_lpImage;				//	��������� �� ������ �������� ������ � ������
	LPVOID m_lpColorTable;			//	��������� �� ������� ������
	DWORD m_dwSizeImage;			//	������ �������� ������ � ������
	DWORD m_dwBitmapInfoSize;		//	������ BITMAPINFOHEADER + ColorTable
	DWORD m_dwStride;				//	���-�� ���� � ������
public:
	DIBData();
	DIBData(const DIBData& image);
	virtual ~DIBData();
public:	//	params
	inline BOOL IsValid(void);
	inline DWORD Width(void);
	inline DWORD Height(void);
	inline LPBYTE Image(void);
	inline DWORD Stride(void);
	inline DWORD ImageSize(void);
	inline DWORD Bpp(void);
public:	//	draw
	BOOL StretchDIBits(HDC hdc,int XDest,int YDest,
							   int nDestWidth,int nDestHeight,
							   int XSrc,int YSrc,
							   int nSrcWidth,int nSrcHeight,
							   DWORD dwRop = SRCCOPY);
	BOOL Draw(HDC hdc,int x=0,int y=0,DWORD dwRop=SRCCOPY);
public:	//	������ �����������
	//	�������� ���������� ������� ������ � ������
	BOOL Copy(DIBData* pImage);
	//	��������� �������� � ��� ���� �� ������ � ������
	BOOL Halve(void);
	//	�������� ������� ������
	BOOL Create(int width,int height,int bpp);
	//	�������� ������ �� �������� �������� ������
	BOOL Create(int width,int height,int bpp,LPBYTE pImage);
	//	�������� ������ � ����������� ������� �� ������� ������
	BOOL Create(DIBData* pImage,int width,int height);
	//	��������� �������� ������� � ����� (x,y) ������ ��� 24,32 ������ ��������
	DWORD GetPixel(unsigned int x,unsigned int y);	//	ARGB ������
	//	��������� �������� ������� � ����� (x,y) ������ ��� 24,32 ������ ��������
	void SetPixel(unsigned int x,unsigned int y,DWORD dwPixel);	//	ARGB ������
	//	������ �� ���� � ������� BMP
	virtual BOOL Save(const char *pFileName);
	//	������ � �������� ����
	virtual BOOL Save(FILE *f);
  //������ �� ��������� �����
	virtual BOOL Load(FILE *f);
protected:
	virtual void InZero(void);
	virtual void ComputeMetrics(void);
private:
	void StringHalve(LPBYTE pSrc1,LPBYTE pSrc2,LPBYTE pDest,DWORD dwSrcLen,DWORD dwStep,BOOL bWidthNoExactly);
public:
	virtual void Release(void);

};

BOOL DIBData::IsValid(void)
{
	return m_bValid;
}

DWORD DIBData::Width(void)
{
	if(m_lpBMIH)
		return m_lpBMIH->biWidth;
	return 0;
}

DWORD DIBData::Height(void)
{
	if(m_lpBMIH)
		return m_lpBMIH->biHeight;
	return 0;
}

LPBYTE DIBData::Image(void)
{
	return m_lpImage;
}

DWORD DIBData::Stride(void)
{
	return m_dwStride;
}

DWORD DIBData::ImageSize(void)
{
	return m_dwSizeImage;
}

DWORD DIBData::Bpp(void)
{
	if(m_lpBMIH)
		return m_lpBMIH->biBitCount;
	return 0;
}

#endif
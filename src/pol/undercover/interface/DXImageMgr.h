/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   02.03.2001

************************************************************************/
#ifndef _DX_IMAGE_MGR_H_
#define _DX_IMAGE_MGR_H_

#include <common/UI/Widget.h>

class DXImage;

class DXImageMgr
{
private:
	struct Deleter 
	{
	public:
		DXImageMgr *m_pInstance;
	public:
		Deleter(){m_pInstance = 0;}
		~Deleter(){if(m_pInstance) delete m_pInstance;}
	};
	friend Deleter;
	static Deleter m_Deleter;
private:
	DXImageMgr();
	virtual ~DXImageMgr();
public:
	//	������� ����� �� ����-�� (�������������� �����������)
	DXImage* CreateImage(const char *pName);
	//	������� ����� ��������� �������
	DXImage* CreateImage(unsigned int width,unsigned int height);
	//	������� ����� ������ � ������������ ������
	DXImage* CreateImage(const char* name,const DXImage* image);
	//	������ ������ ������
	void GetImageSize(DXImage* image,int* width,int* height);
	//	������, �������� �� ����� ����� ��� ���
	bool IsImageCreated(const char* name) const;
	//	������������� �����
	void ReloadImage(DXImage* image);
	//	���������� ����� 
	void Release(DXImage* image);
private:
	//	������� ����� �� �������� (tga,bmp,jpg)
	DXImage* CreatePicture(const char *pName);
	//	������� ����� �� ����� (bik)
	DXImage* CreateBink(const char *pName);
public:
	static DXImageMgr *Instance(void);
};

inline DXImageMgr* DXImageMgr::Instance(void)
{
	if(!m_Deleter.m_pInstance) m_Deleter.m_pInstance = new DXImageMgr();
	return m_Deleter.m_pInstance;
}

//***************************************************************//
//********************** class DXImage **************************//
class DXImage : public WidgetImage
{
public:
	enum TYPE
	{
		T_UNKNOWN,
		T_PICTURE,
		T_BINK
	};
public:
	std::string m_Resource;		//	��� ������� (�������, ����)
	TYPE m_Type;
	LPDIRECTDRAWSURFACE7 m_pSurface;

public:
	DXImage(LPDIRECTDRAWSURFACE7 surface,const char* resource,TYPE type);
	virtual ~DXImage();

public:
	void Reload(void);
	void Release(void);

	int GetWidth();
};

#endif	//_DX_IMAGE_MGR_H_
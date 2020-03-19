/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   27.02.2001

************************************************************************/
#ifndef _SURFACE_MGR_H_
#define _SURFACE_MGR_H_

class DDSurface;

class SurfaceMgr
{
private:
	struct Deleter 
	{
	public:
		SurfaceMgr *m_pInstance;
	public:
		Deleter(){m_pInstance = 0;}
		~Deleter(){if(m_pInstance) delete m_pInstance;}
	};
	friend Deleter;
	static Deleter m_Deleter;
private:
	enum IMAGE_TYPE {IT_BITMAP,IT_JPEG,IT_TARGA,IT_UNKNOWN};
private:
	class Reference;
	std::map<std::string,Reference*> m_mSurfaces;		//	����� ������������
private:
	SurfaceMgr();
	virtual ~SurfaceMgr();
public:
	//	�������� ����������� �� ��������������
	LPDIRECTDRAWSURFACE7 Surface(const char *pName);
	//	������� ����������� ��������� ������� � �������� �� �������������
	const char* CreateSurface(unsigned int width,unsigned int height);
	//	������� ����� ������ ����������� � �������� ���������������
	//	���� ������ ������������� ��� ������������ �� return 0
	LPDIRECTDRAWSURFACE7 CreateSurface(const char* name,LPDIRECTDRAWSURFACE7 surface);
	//	������, ������� �� ����������� � ����� ���������������
	bool IsSurfaceCreated(const char* name) const;
	//	������� ���-�� ������������ � ���������
	int Quantity(void) const;
	//	������� ������ ������, ���������� �������������
	int Memory(void);
	//	���������� ����������� �� ��������������
	void Release(const char *pName);
	//	���������� ��� �����������
	void Release(void);
private:
	void LoadImageToSurface(const char *pName);
	void SetColorKey(DDSurface *surface);
	IMAGE_TYPE GetImageType(const char* pFileExtension);
public:
	static SurfaceMgr *Instance(void);
};

inline SurfaceMgr* SurfaceMgr::Instance(void)
{
	if(!m_Deleter.m_pInstance) m_Deleter.m_pInstance = new SurfaceMgr();
	return m_Deleter.m_pInstance;
}

inline int SurfaceMgr::Quantity(void) const
{
	return m_mSurfaces.size();
}

//***************************************************************//
//************** class SurfaceMgr::Reference ********************//
class SurfaceMgr::Reference
{
private:
	DDSurface* m_Value;
	int m_Counter;	
public:	
	Reference();
	Reference(DDSurface* value);
	virtual	~Reference();
public:	
	DDSurface* Get(void){m_Counter++;return m_Value;}
	int Release(void){if(m_Counter){m_Counter--;} return m_Counter;}
	int Counter(void){return m_Counter;}
};

#endif	//_SURFACE_MGR_H_
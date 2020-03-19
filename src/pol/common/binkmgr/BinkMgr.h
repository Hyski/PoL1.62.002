/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   02.03.2001

************************************************************************/
#ifndef _BINK_MGR_H_
#define _BINK_MGR_H_

class BinkSurface;

//=====================================================================================//
//                                    class BinkMgr                                    //
//=====================================================================================//
class BinkMgr
{
	struct Deleter 
	{
	public:
		BinkMgr *m_pInstance;
	public:
		Deleter(){m_pInstance = 0;}
		~Deleter(){if(m_pInstance) delete m_pInstance;}
	};

	friend Deleter;
	static Deleter m_Deleter;

	float m_volume;

private:
	class Reference;
	typedef std::map<std::string,Reference*> Binks_t;
	Binks_t m_mBinkSurfaces;		//	����� ������������
	Binks_t::iterator m_Updated;

private:
	BinkMgr();
	virtual ~BinkMgr();

public:
	//	������������� ��� �������� ����
	void Update(void);
	//	���������� ����������� ����
	void Update(const char *pName);

public:	//	����������
	/// ���������� ��������� �������� ������
	void SetVolume(float vol);
	//	�������� ����������� �� �������������� �����
	LPDIRECTDRAWSURFACE7 Surface(const char *pName);
	//	������� ���-�� ������ � ���������
	inline int Quantity(void) const;
	//	���������� ����������� � ������� ����� �� ��������������
	void Release(const char *pName);
	//	���������� ��� �����������
	void Release(void);

private:
	void LoadBink(const char *pName);
	void SetColorKey(LPDIRECTDRAWSURFACE7 surface);

public:
	static BinkMgr *Instance(void);
};

inline BinkMgr *BinkMgr::Instance(void)
{
	if(!m_Deleter.m_pInstance) m_Deleter.m_pInstance = new BinkMgr();
	return m_Deleter.m_pInstance;
}

inline int BinkMgr::Quantity(void) const
{
	return m_mBinkSurfaces.size();
}

//***************************************************************//
//**************** class BinkMgr::Reference *********************//
class BinkMgr::Reference
{
private:
	BinkSurface* m_Value;
	int m_Counter;	
public:	
	Reference();
	Reference(BinkSurface* value);
	virtual	~Reference();
public:	
	BinkSurface* AddRef(void){m_Counter++;return m_Value;}
	BinkSurface* Value(void){return m_Value;}
	int Release(void){if(m_Counter){m_Counter--;} return m_Counter;}
	int RefCount(void){return m_Counter;}
};


#endif	//_BINK_MGR_H_
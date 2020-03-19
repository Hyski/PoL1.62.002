/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description: ������� ����� ��� ������������ ���� ��������� ����������.
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)
	Date:   22.05.2000

************************************************************************/
#ifndef _LAGOON_H_
#define _LAGOON_H_

//---------- ��� ���� ------------
#ifdef _DEBUG_INTERFACE
class CLog;
extern CLog lagoon_log;
#define lag	lagoon_log["lagoon.log"]
#else
#define lag	/##/
#endif
//--------------------------------

//------------- ���������� ZOrder -------------------
class Lagoon;

struct ZOrder
{
	Lagoon* pLagoon;
	ZOrder(Lagoon* pL)
	{
		pLagoon = pL;
	};
};
//---------------------------------------------------

class GraphPipe;

class Lagoon
{
protected:
	//	����������� ����������
	int m_x;
	int m_y;										//	������ ������� �����
	int m_width;	
	int m_height;									//	������� ������� �����	
	//	������ �����	
	HRGN m_hVirtualRegion;							//	����������� ������������ ������� �����
	HRGN m_hScreenRegion;							//	�������� ������������ ������� �����
protected:
	std::string m_sName;							//	���-������������� �������
	std::string m_sClass;							//	��� ���� �������
	std::map<std::string,Lagoon*> m_ChildMap;		//	����� �������� ��������
protected:
	//	�� ��� �������� � ������� ����� �� ����������
	bool m_bFocus;	//	���� ������� ������
public:
	bool m_bFocusValid;		//	���� �������� ����� m_bFocus
	bool m_bMouseCapture;	//	���� ����� ���� ������������ ������ ���
public:
	Lagoon *m_pParent;								//	������������ ������
	bool m_bScaleX;									//	��� ������������� ������������ X-����������
	bool m_bScaleY;									//	��� ������������� ������������ Y-����������
public:
	//	��� ��� �������� � z �����������
	float m_zBegin;		//	������ ����������� ���������� - ������� �������� Z
	float m_zEnd;		//	����� ����������� ����������
protected:
	//	������������ �������� �� z
	std::set<ZOrder> m_zBuffer;	
public:
	//	������ ��� ���������
	//point3 m_Vertex[4];
	//texcoord m_TexCoord[4];
	Primi m_Object;
public:
	std::string m_sShader;
	bool m_bVisible;			//	���������� ������ ������ ��� ���
	bool m_bEnable;				//	�������� �� ��� ������ ��� ���
public:
	Lagoon(const char *pName);
	virtual ~Lagoon();
public:
	//	������� ��� ������ � ��������� ������
	void AddChild(Lagoon* pLagoon);
	void DeleteChild(const char *pName);
	void ClearChildMap(void);
	void RefreshZ(void);
public:
	//	������� ��������� �������� (����������� ����������)
	virtual void SetRect(RECT *pRect);
	virtual void SetRect(int left,int top,int right,int bottom);
	virtual void SetRegion(HRGN hRegion);
	//	������� ��������� �������������� (����������� ����������)
	virtual void MoveTo(int x,int y);
	//	������� �������� ����������� ��������� � ��������
	virtual void TransformToScreen(void);
	//	������� �����������
	virtual void PaintRegion(HDC hdc);
	virtual void Render(GraphPipe *lpGraphPipe);
	virtual void SetVisible(const char *pChild,bool bVisible);
	virtual void SetVisible(bool bVisible);
	//	������� ���������� ������ ��� ����� �����������
	virtual void Tune(void);
	//	������� �������� ����������
	virtual void Tick(void);
	//	����������� ����
	virtual void MouseIn(void);
	virtual void MousePresent(void);
	virtual void MouseOut(void);
	virtual void MouseCaptureOut(void);
	//	����������� ������
	virtual void SetFocus(void);
	virtual void KillFocus(void);
	virtual void FocusPresent(void);
	//	�������� �� �������������� � ���������� ������
	virtual bool IsClass(const char *pClass);
	//	������ �������� �����������
	virtual void SetMessage(unsigned int iMsg,Lagoon *pLagoon);
protected:
	virtual void SetVertex(void);
public:
	//	�������������� �������
	inline const char* Name(void);
	inline bool IsFocus(void);
	//	���������� � �������� � �������������� (����������� ����������)
	inline int X(void);
	inline int Y(void);
	inline int Width(void);
	inline int Height(void);
	//	���������� � �������������� ����� ������� (�������� ����������)
	inline BOOL PtInLagoon(int x,int y);
	//	���������� �� ���� � ����� (m_bEnable == true && m_bVisible == true)
	Lagoon* GetLagoon(int x,int y,bool bAlways);
	Lagoon* GetLagoon(const char *pName);
	//	���������� �� ���� �� ������� �����
	Lagoon* GetFocus(void);
};

//********************* ���������� ZOrder *****************************//
inline bool operator < (const ZOrder& x, const ZOrder& y)
{
	if(x.pLagoon->m_zEnd > y.pLagoon->m_zEnd)
		return true;
	
	return false;
}
//---------------------------------------------------

//********************* �������������� ������� *****************************//
const char* Lagoon::Name(void)
{
	return m_sName.c_str();
}

bool Lagoon::IsFocus(void)
{
	return m_bFocus;
}

int Lagoon::X(void)
{
	return m_x;
}

int Lagoon::Y(void)
{
	return m_y;
}

int Lagoon::Width(void)
{
	return m_width;
}

int Lagoon::Height(void)
{
	return m_height;
}

//	x,y - �������� ����������
BOOL Lagoon::PtInLagoon(int x,int y)
{
	return PtInRegion(m_hScreenRegion,x,y);
}

inline Lagoon* Lagoon::GetLagoon(const char *pName)
{
	std::map<std::string,Lagoon*>::iterator il;

	il = m_ChildMap.find(pName);
	if(il != m_ChildMap.end())
		return il->second;

	return 0;
}

#endif	//_LAGOON_H_
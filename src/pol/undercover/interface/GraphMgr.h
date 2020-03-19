/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   23.05.2001

************************************************************************/
#ifndef _GRAPH_MGR_H_
#define _GRAPH_MGR_H_

#include <common/UI/Widget.h>

class GLayer;
class Text;

class GraphWindow : public Widget
{
public:
	static HPEN m_BasePen;
	static HBRUSH m_BaseBrush;
private:
	//	���� ���� ��� ������ ��������������� ����� �� �����
	bool m_MouseCapture;
	int m_xMouse,m_yMouse;
private:
	Text* m_minText;
	Text* m_maxText;
	//	��������������� �����
	GLayer* m_Layer;
	float m_Time;				//	���������� ������� �� ���������� �����
	float m_MinValue;
	float m_MaxValue;
private:
	std::map<float,float> m_Values;
public:
	GraphWindow(const char* name,int width,int height);
	virtual ~GraphWindow();
public:
	//	��������� � �������������
	void Calibrate(float time,float min_value,float max_value);
	void AddValue(float time,float value);
	void Clear(void);
	void AutoScale(void);
	void Show(void);
	void Hide(void);
private:
	//	��������� ��������� �����
	bool ProcessMouseInput(VMouseState* pMS);
	//	���������� �����-�� ��������
	bool Tick(void);
	//	����������� �� ��������� ������ ������ �������
	void OnSystemChange(void);
	//	���������� ������
	void Build(void);
public:
	const char* Type(void) const {return "graphwindow";};
};


class GraphMgr
{
	class Console;
private:
	//	��� ������ ��������� 
	static GraphMgr* m_Instance;
	//	���� ���������� ��������� � ����
	static bool m_IsAvailable;
	//	������������ ���� ��� ��������
	static Widget* m_Owner;
	//	��������� �������
	Console* m_Console;
public:
	GraphMgr();
	virtual ~GraphMgr();
public:	//	��������� ������������� ��������� ��������
	//	������� ��������
	static GraphMgr* SetAvailable(Widget* pOwner); 
	//	��������� ������������ �� ��������
	static bool IsAvailable(void);
	//	���������� ��������
	static void Release(void);
public:
	//	������� ���� ��� ����� �������
	static GraphWindow* BuildWindow(const char* name,int width,int height); 
	//	�������� ���� �� �����
	static GraphWindow* Window(const char* name);
	//	���������� ����
	static void DestroyWindow(const char* name);
};

inline bool GraphMgr::IsAvailable(void) 
{
	return m_IsAvailable;
}

#endif	//_GRAPH_MGR_H_
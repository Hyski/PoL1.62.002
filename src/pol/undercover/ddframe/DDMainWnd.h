/***********************************************************************

                                   Alfa

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   14.11.2001

************************************************************************/
#ifndef _DD_MAIN_WND_H_
#define _DD_MAIN_WND_H_

//*********************************************************************//
//	class DDMainWnd
class DDMainWnd
{
public:
	class Controller;
	enum MODE {M_NONE,M_WINDOWED,M_FULLSCREEN};
public:
	static const int m_MaxWidth;
	static const int m_MaxHeight;
private:
	Controller* m_Controller;
	std::string m_Caption;
	unsigned int m_Icon;
	bool m_Minimized;
	bool m_Activated;
	HWND m_Handle;
	SIZE m_Client;
	SIZE m_Screen;
	MODE m_Mode;
public:
	DDMainWnd(const char* caption,unsigned int icon);
	virtual ~DDMainWnd();
public:
	//	�������� ����
	bool Create(int width,int height,MODE mode);
	//	����������� ����
	void Destroy(void);
	//	�������� ����
	void Show(void);
	//	�������� ������� ����
	void Resize(int width,int height);
	//	����������� ���� � �����
	void MoveTo(int x,int y);
	//	��������� ������� ��������� ��� ��������
	void Tick(void);
	//	��������� ������� ��������� � ���������
	void Wait(void);
	//	���������� ����������
	void SetController(Controller* controller);
	//	�������� ������������� ����
	HWND Handle(void) const;
	//	����� ����������� ����
	MODE Mode(void) const;
	//	�������� ������� ����
	const SIZE& ClientSize(void) const; 
	const SIZE& ScreenSize(void) const; 
private:
	//	��������������� ������ ����
	void RegisterMainWndClass(void) const;
private:
	//	������������ ��������� ������
	static DDMainWnd* m_Instance;
	//	��������� ������� ���������
	friend LRESULT CALLBACK MainWndProc(HWND hWnd,UINT iMsg,WPARAM wParam,LPARAM lParam);
	LRESULT ProcessMessage(HWND hWnd,UINT iMsg,WPARAM wParam,LPARAM lParam);
	LRESULT	OnPaint(WPARAM wParam,LPARAM lParam);
	LRESULT	OnMove(WPARAM ,LPARAM lParam);
	LRESULT OnSysCommand(WPARAM wParam,LPARAM lParam);
	LRESULT OnSetCursor(WPARAM wParam,LPARAM lParam);
	LRESULT OnSize(WPARAM wParam,LPARAM lParam);
	LRESULT OnMouseMove(WPARAM wParam,LPARAM lParam);
	LRESULT OnActivate(WPARAM wParam,LPARAM lParam);
};

//	�������� ������������� ����
inline HWND DDMainWnd::Handle(void) const
{
	return m_Handle;
}
//	�������� ������ ���������� ������� ����
inline const SIZE& DDMainWnd::ClientSize(void) const
{
	return m_Client;
}
//	�������� ������ ������� ����
inline const SIZE& DDMainWnd::ScreenSize(void) const
{
	return m_Screen;
}
//	����� ����������� ����
inline DDMainWnd::MODE DDMainWnd::Mode(void) const
{
	return m_Mode;
}

//*********************************************************************//
//	class DDMainWnd::Controller
class DDMainWnd::Controller
{
public:
	virtual ~Controller() {}
	virtual void OnClose(void) = 0;
	virtual void OnPaint(void) = 0;
	virtual void OnMove(short x,short y) = 0;
	virtual void OnMouseMove(void) = 0;
	virtual void OnActivate(void) = 0;
	virtual void OnDeactivate(void) = 0;
	virtual void OnResize(int width,int height) = 0;
};

#endif	//_DD_MAIN_WND_H_
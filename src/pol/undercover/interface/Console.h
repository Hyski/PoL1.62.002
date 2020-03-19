/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description: ������� ������� ����� � ����� ������� ���� ��� 
	������ ���������� � ������ ����� ��� ����� ����������.
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   04.04.2001

************************************************************************/
#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#include "Dialog.h"

class Edit;
class TextBox;

class Console : public Dialog
{
public:
	class Controller;
	static const char* m_pDialogName;
	enum LAST_EVENT 
	{
		LE_NONE,				//	������
		LE_ACTIVATE				//	����� �������� ����� �����
	};
private:
	//	��� ������ ������� 
	static Console* m_Instance;
	//	���� ���������� ������� � ����
	static bool m_IsAvailable;
private:
	//	��������� ������� �� �������
	LAST_EVENT m_LastEvent;
	//	���� ����� ������
	Edit* m_CommandLine;
	//	���� ������ ������
	TextBox* m_Content;
	//	������ ������������
	std::vector<Controller*> m_Controllers;
	//	���� ���� ��� ������ ��������������� ����� �� �����
	bool m_MouseCapture;
	int m_xMouse,m_yMouse;
private:
	Console();
public:
	virtual ~Console();
public:	//	��������� ������������� �������
	//	������� �������
	static Console* SetAvailable(void); 
	//	��������� ������������ �� �������
	static bool IsAvailable(void);
public:	//	��������� ������������� �������
	//	�������� ������ � ���� ������ �������
	static void AddString(const char* string);
	//	�������� ������ ������ �� ���� ����� �������
	static const std::string& GetCommandLine(void);
	//	�������� ������ ������ �� ���� ����� ������� ��� ������ ����������
	static int GetCommandLine(std::vector<std::string>& argv);
	//	���������� ����� � ���� ����� �������
	static void SetCommandLine(const char* string);
	//	����������� �� ������� ����� �� �������
	static void AddController(Controller* controller);
	//	������ ���������� �� ������� ����� �� �������
	static void RemoveController(Controller* controller);
	//	��������� ������� �� �������
	static void Show(bool show);
	//	��������� �� ������� �� ������
	static bool IsOnScreen(void);
	//	�������� ��������� ������� �� �������
	static Console::LAST_EVENT LastEvent(void);
public:
	//	��������� ��������� �����
	bool ProcessMouseInput(VMouseState* pMS);
	//	��������� ������ ������������� �����
	bool ProcessKeyboardLost(void);
	//	������ ����� �� ����������
	void OnKeyboardLost(void);
	//	��������� ���������� ���������
	void OnSystemChange(void);
	//	��������� ������� �� �������� ����
	void OnChange(Widget* pChild);
	//	��������� ���������
	void SetVisible(bool bVisible);
	//	���������� �����-�� ��������
	bool Tick(void);
private:
	//	��������� ���������� ��������� � �������
	void Starting(void);
};

inline bool Console::IsAvailable(void) 
{
	return m_IsAvailable;
}

//***************************************************************************//
//********************       class Console::Controller       ****************//
class Console::Controller
{
public:
	Controller(){}
	virtual ~Controller(){}
public:
	virtual void Activate(void) = 0;
};



#endif	//_CONSOLE_H_
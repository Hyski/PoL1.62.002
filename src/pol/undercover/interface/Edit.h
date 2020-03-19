/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   26.03.2001

************************************************************************/
#ifndef _EDIT_H_
#define _EDIT_H_

#include "Text.h"

class Edit : public Text
{
public:
	class Controller;
private:
	class Caret;
	Caret* m_Caret;				//	�������
	Controller* m_Controller;
private:
	bool m_InsertMode;			//	��������/��������� ������� ��������
public:
	Edit(const char *pName);
	virtual ~Edit();
public:
	//	���������
	struct Traits
	{
		const char* m_CaretImageName;
	};
	//	������������������� ���������
	virtual void SetTraits(const Traits& t);
public:
	//	������ ������������ ���-�� ��������
	virtual void SetLength(int symbols);
	//	������ ������������� ������ ������
	virtual void SetRegion(int left,int top,int right,int bottom);
	//	���������� �����
	virtual void SetFont(const char* name,int color);
	//	����������� �����
	virtual void SetText(const char* text);
	//	��������� �����
	virtual void Align(int align);
	//	���������� ����� ����� ������
	virtual void SetInsertMode(bool insert_mode);
	//	����������� ������ � ������������ �������
	enum SEEK {S_BEGIN,S_CURRENT,S_END};
	void SeekCaret(int pos,SEEK seek);
	//	�������� ������� ������� � ��������
	unsigned int GetCaretPos(void);
	//	���������� ����������
	void SetController(Controller* controller);
private:
	virtual void OnSystemChange(void);
	//	��������� ��������� �����
	virtual bool ProcessMouseInput(VMouseState* pMS);
	//	��������� ������������� �����
	virtual bool ProcessKeyboardInput(VKeyboardState* pKS);
	//	��������� ������ ������������� �����
	virtual bool ProcessKeyboardLost(void);
private:
	void InsertCharToText(unsigned char ch,int pos);
	void DeleteCharFromText(int pos);
	void MoveCaret(int dx,int new_width);
	//	����������� ������� � �������� ������� � ���������
	void MoveCaretTo(int pos);
public:
	virtual const char* Type(void) const {return "edit";};
};

//***********************************************************************//
//**********************    class Edit::Caret     ***********************//
class Edit::Caret : public Widget
{
private:
	int m_SymMax;				//	����� � ��������
	int m_SymPos;				//	������� ������� � ��������
	int m_SpaceWidth;			//	������ �������
	bool m_bActivated;			//	���� ������������� �������
private:
	static const float m_CaretBlinkFreq;		//	������� ������� �������
	static float m_BlinkSeconds;				//	������� ��������� ������
	static const int m_CaretHeight;				//	������ ������� � ��������
public:
	Caret();
	virtual ~Caret();
public:
	//	���������� ������� �������
	void SetMovingLine(int top,int space_width);
	//	���������� ������������ ���-�� ��������
	void SetMaxSym(int symbols);
	//	����������� ������� �����
	bool Reverse(void);
	//	����������� ������� ������
	bool Forward(void);
	//	����������� �������
	void SetPos(unsigned int pos);
	//	���������� ����� ������ �������
	void SetNewWidth(int width);
	//	���������� ��������� �������
	void Activate(bool bActivate);
public:
	int Pos(void){return m_SymPos;}
	int SpaceWidth(void){return m_SpaceWidth;}
	bool IsActivated(void){return m_bActivated;}
public:
	//	������������ �������
	bool Tick(void);
};

//***********************************************************************//
//	class Edit::Controller
class Edit::Controller
{
public:
	virtual ~Controller() {}
	virtual void OnReturn(void) = 0;
	virtual void OnBackspace(void) = 0;
	virtual void OnDelete(void) = 0;
	virtual void OnLeft(void) = 0;
	virtual void OnRight(void) = 0;
};

#endif	//_EDIT_H_
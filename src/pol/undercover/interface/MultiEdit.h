/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   30.10.2001

************************************************************************/
#ifndef _MULTI_EDIT_H_
#define _MULTI_EDIT_H_

#include <common/UI/Widget.h>

class TextBox;
class Edit;

class MultiEdit : public Widget
{
	class EditController;
	class Service;
	friend EditController;
public:
	struct Traits
	{
		const char* m_CaretImageName;
	};
private:
	TextBox* m_TextBox;						//	����� ����
	Edit* m_Edit;							//	������ ��������������
	unsigned int m_EditOffset;				//	�������� ������ ��������������
	EditController* m_EController;
public:
	MultiEdit(const char* name);
	virtual ~MultiEdit();
public:
	//	���������� �������
	void SetRegion(int left,int top,int right,int bottom);
	//	���������� �����
	void SetFont(const char* name,int color);
	//	�������� �����
	void Clear(void);
	//	���������� ��������
	void SetTraits(const Traits& t);
	//	�������� �����
	const std::string& GetText(void) const;
private:
	void OnReturn(void);
	void OnBackspace(void);
	void OnDelete(void);
	void OnLeft(void);
	void OnRight(void);
private:
	//	���������� ������ ��������������
	void MoveEdit(int offset);
	//	��������� ��������� �����
	bool ProcessMouseInput(VMouseState* pMS);
	//	��������� ������������� �����
	bool ProcessKeyboardInput(VKeyboardState* pKS);
public:
	const char* Type(void) const {return "multiedit";}
};

#endif	//_MULTI_EDIT_H_
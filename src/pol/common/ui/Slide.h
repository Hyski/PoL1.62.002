/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   07.05.2001

************************************************************************/
#ifndef _SLIDE_H_
#define _SLIDE_H_

#include "Widget.h"

class Slide : public Widget
{
private:
	float m_Value;
	float m_Step;
	Widget* m_Image;
public:
	Slide(const char *pName);
	virtual ~Slide();
public:
	//	���������� �������� ������������� � �������
	void SetValue(float v);
	//	���������� ���
	void SetStep(float s);
	//	�������� ��������
	float Value(void) const;
public:
	//	���������� �������� �����������
	virtual void SetImage(WidgetImage* pImage);
	//	��������� ��������� �����
	virtual bool ProcessMouseInput(VMouseState* pMS);
	//	��������� ������������� �����
	virtual bool ProcessKeyboardInput(VKeyboardState* pKS);
private:
	void Update(void);
public:
	virtual const char* Type(void) const {return "slider";}
};

//	�������� ��������
inline float Slide::Value(void) const
{
	return m_Value;
}

#endif	//_SLIDE_H_
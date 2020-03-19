/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
        ������.
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   28.02.2001

************************************************************************/
#ifndef _BUTTON_H_
#define _BUTTON_H_

#include "Widget.h"

class Button : public Widget
{
public:
	enum BUTTON_STATE
	{
		BS_NORMAL,				//	����������, ������� ���������
		BS_PUSHED,				//	������� ���������
		BS_SELECTED,			//	������ ��� �������
		MAX_STATE
	};
private:
	BUTTON_STATE m_BS;
	WidgetImage* m_pImages[MAX_STATE];
	Player::Sample* m_Samples[MAX_STATE];
	bool m_FirstSelect;
	bool m_NoFocus;
public:
	Button(const char *pName);
	virtual ~Button();
public:
	void SetImage(BUTTON_STATE bs,WidgetImage *pImage);
	void SetSample(BUTTON_STATE bs,Player::Sample* sample);
	void SetState(BUTTON_STATE bs);
	BUTTON_STATE GetState(void);
	//	���������� ������������� ������
	void NoFocus(bool no_focus);
protected:
	//	��������� ��������� �����
	virtual bool ProcessMouseInput(VMouseState* pMS);
	//	��������� ������������� �����
	virtual bool ProcessKeyboardInput(VKeyboardState* pKS);
	//	��������� ������ �������� �����
	virtual bool ProcessMouseLost(void);
	//	��������� ������ ������������� �����
	virtual bool ProcessKeyboardLost(void);
	//	��������� ���������� ���������
	virtual void OnSystemChange(void);
private:
	//	������������ �����
	void PlayState(BUTTON_STATE bs);
public:
	virtual const char* Type(void) const {return "button";};
};

inline Button::BUTTON_STATE Button::GetState(void)
{
	return m_BS;
}


#endif	//_BUTTON_H_